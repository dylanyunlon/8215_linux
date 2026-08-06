#include <stdio.h>
#include <linux/types.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include "load_data.h"
#include <syslog.h>

#define PRINT_INFO(format, ...)   fprintf(stderr, "[I][MM][DTDemo][%s:%d] " format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define PRINT_ERROR(format, ...)  fprintf(stderr, "[E][MM][DTDemo][%s:%d] " format"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

enum {
  LOADDATA_ST_IDLE,
  LOADDATA_ST_INITED,
  LOADDATA_ST_RUNING,
  LOADDATA_ST_STOPPING,
};

typedef struct {
  char *szfilename;
  __u64 file_sz;
  int fd;
  bool  need_stop;
  bool loop;
  ATCHWDATARECFUNC cb;
  pthread_t tid;
  __s32 state;
  __u64 read_fileofst;
  __u64 data_fileofst;
  __u8  *pu1Buf;
  __u32 bufofst;
  __u32 bufsz;
  void *pvarg;
} AtcLoadDataInst;

#define LOAD_DATA_MAX_BUF_SZ ((__u32)(1024 * 1024))
#define SEND_BY_NAL 1

void *OpenDataLoad(char *szfilename, bool loop)
{
  __u32 name_len = 0;
  AtcLoadDataInst *loaddatainst = NULL;
  struct stat statbuff;

  PRINT_INFO("[demo] %s line %d enter\r\n",
    __func__, __LINE__);
  if (NULL == szfilename) {
    PRINT_ERROR("[demo] %s line %d fail for invalid args\r\n",
      __func__, __LINE__);
    return NULL;
  }

  loaddatainst = (AtcLoadDataInst *)malloc(sizeof(AtcLoadDataInst));
  if (NULL == loaddatainst) {
    PRINT_ERROR("[demo] %s line %d fail for no memory\r\n",
      __func__, __LINE__);
    return NULL;
  }
  memset(loaddatainst, 0, sizeof(AtcLoadDataInst));
  loaddatainst->loop = loop;

  name_len = strlen(szfilename);

  name_len = ((name_len + 256 + 1) / 256) * 256;

  loaddatainst->file_sz = 0;
  loaddatainst->fd = -1;
  loaddatainst->cb = NULL;
  loaddatainst->pvarg = NULL;
  loaddatainst->need_stop = false;
  loaddatainst->pu1Buf = (__u8 *)malloc(LOAD_DATA_MAX_BUF_SZ * sizeof(__u8));
  if (NULL == loaddatainst->pu1Buf)
  {
    PRINT_ERROR("[demo] %s line %d fail for no memory\r\n",
      __func__, __LINE__);
    goto OPENFAILED;
  }
  loaddatainst->bufsz = LOAD_DATA_MAX_BUF_SZ * sizeof(__u8);
  loaddatainst->bufofst = 0;
  loaddatainst->read_fileofst = 0;
  loaddatainst->data_fileofst = 0;

  loaddatainst->szfilename = (char *)malloc(name_len * sizeof(char));

  memset(loaddatainst->szfilename, 0, name_len * sizeof(char));

  strcpy(loaddatainst->szfilename, szfilename);

  loaddatainst->fd = open(loaddatainst->szfilename, 0666);
  if (-1 == loaddatainst->fd)
  {
    PRINT_ERROR("[demo] %s line %d fail in open file, err: %d(%s)\r\n",
      __func__, __LINE__, errno, strerror(errno));
    goto OPENFAILED;
  }

  memset(&statbuff, 0, sizeof(statbuff));

  if (stat(loaddatainst->szfilename, &statbuff) < 0)
  {
      PRINT_ERROR("[demo] %s line %d fail in get file info, err: %d(%s)\r\n",
        __func__, __LINE__, errno, strerror(errno));
      goto OPENFAILED;
  }

  loaddatainst->file_sz = (__u64)statbuff.st_size;
  loaddatainst->state = LOADDATA_ST_IDLE;

  PRINT_INFO("[demo] %s line %d -- buffer(%p), bufsz(%d)\r\n",
    __func__, __LINE__, loaddatainst->pu1Buf,
    loaddatainst->bufsz);

  PRINT_INFO("[demo] %s line %d exit, return %p\r\n",
    __func__, __LINE__, loaddatainst);

  return loaddatainst;

OPENFAILED:
  if (loaddatainst->fd >= 0)
    close(loaddatainst->fd);
  if (NULL != loaddatainst->pu1Buf)
    free(loaddatainst->pu1Buf);
  if (NULL != loaddatainst->szfilename)
    free(loaddatainst->szfilename);
  if (NULL != loaddatainst)
    free(loaddatainst);
  return NULL;
}

bool getNextNALUnit(
  __u8 **_data, __u32 *_size,
  __u8 **nalStart, __u32 *nalSize,
  bool startCodeFollows)
{
    __u8 *data = *_data;
    __u32 size = *_size;

    *nalStart = NULL;
    *nalSize = 0;

    if (size == 0) {
        return false;
    }

    __u32 offset = 0;
    while ((offset < size) && data[offset] == 0x00) {
        ++offset;
    }

    if (offset == size) {
      return false;
    }

    // A valid startcode consists of at least two 0x00 bytes followed by 0x01.

    if (offset < 2 || data[offset] != 0x01) {
      return false;
    }

    ++offset;

    __u32 startOffset = offset;

    for (;;) {
        while (offset < size && data[offset] != 0x01) {
            ++offset;
        }

        if (offset == size) {
            if (startCodeFollows) {
                offset = size + 2;
                break;
            }

            return false;
        }

        if (data[offset - 1] == 0x00 && data[offset - 2] == 0x00) {
            break;
        }

        ++offset;
    }

    __u32 endOffset = offset - 2;
    while (endOffset > startOffset + 1 && data[endOffset - 1] == 0x00) {
        --endOffset;
    }

    *nalStart = &data[startOffset];
    *nalSize = endOffset - startOffset;

    *_data = &data[endOffset];
    *_size = size - endOffset;

    return true;
}

void * loaddata_readfile_proc(void * arg)
{
  AtcLoadDataInst *loaddatainst = (AtcLoadDataInst *)arg;
  int readsz = 0;
  if (NULL == loaddatainst) {
    PRINT_ERROR("[demo] %s line %d fail for invalid args\r\n",
      __func__, __LINE__);
    return NULL;
  }

  loaddatainst->bufofst = 0;

  while (!loaddatainst->need_stop) {
    if (LOADDATA_ST_STOPPING == loaddatainst->state) {
      PRINT_ERROR("[demo] %s line %d exit while for state is stopping\r\n",
        __func__, __LINE__);
      break;
    }

    if (loaddatainst->bufsz > loaddatainst->bufofst) {
      __u8 *pu1Readbuf = NULL;
      readsz = read(loaddatainst->fd, loaddatainst->pu1Buf + loaddatainst->bufofst,
        loaddatainst->bufsz - loaddatainst->bufofst);
      PRINT_INFO( "[demo] %s line %d -- read file(fileofst: 0x%llx, bufofst: 0x%08x, %p, sz: 0x%08x, readsz: %d(0x%08x))\r\n",
        __func__, __LINE__, loaddatainst->read_fileofst,
        loaddatainst->bufofst, loaddatainst->pu1Buf + loaddatainst->bufofst,
        loaddatainst->bufsz - loaddatainst->bufofst,
        readsz, readsz);
      pu1Readbuf = loaddatainst->pu1Buf + loaddatainst->bufofst;
      PRINT_INFO( "[demo] line %d -- [0x%02x][0x%02x][0x%02x][0x%02x][0x%02x][0x%02x])\r\n",
        __LINE__,
        pu1Readbuf[0], pu1Readbuf[1], pu1Readbuf[2],
        pu1Readbuf[3], pu1Readbuf[4], pu1Readbuf[5]);
      if (0 < readsz) {
        loaddatainst->read_fileofst += readsz;
      }
    } else {
      PRINT_ERROR("[demo] %s line %d fail for bufsz(0x%08x) <= offset(0x%08x)\r\n",
        __func__, __LINE__, loaddatainst->bufsz, loaddatainst->bufofst);
      break;
    }
#if SEND_BY_NAL
    if (0 < readsz) {
      __u8 *pu1data = loaddatainst->pu1Buf;
      __u8 *pu1nal = NULL;
      __u32 datasz = loaddatainst->bufofst + readsz;
      __u32 nalsize = 0;
      bool fgerr = false;
      while (getNextNALUnit(&pu1data, &datasz, &pu1nal, &nalsize, false) &&
        (nalsize > 0) && (pu1nal != NULL)) {
        if (pu1nal >= loaddatainst->pu1Buf + 3) {
          pu1nal -= 3;
          nalsize += 3;
          PRINT_INFO( "[demo] line %d -- atc_vdec_decode(%p, 0x%08x, fileofst: 0x%llx, [0x%02x][0x%02x][0x%02x][0x%02x] [0x%02x][0x%02x][0x%02x])\r\n",
            __LINE__, pu1nal, nalsize, loaddatainst->data_fileofst,
            pu1nal[0], pu1nal[1], pu1nal[2], pu1nal[3],
            pu1nal[nalsize - 2] , pu1nal[nalsize - 1], pu1nal[nalsize]);
          PRINT_INFO( "[demo] line %d -- pu1data: %p, datasz: 0x%08x)\r\n",
            __LINE__, pu1data, datasz);
          loaddatainst->cb(pu1nal, nalsize, loaddatainst->pvarg);
          loaddatainst->bufofst = 0;
          loaddatainst->data_fileofst += nalsize;
          pu1nal = NULL;
          nalsize = 0;
        } else {
          PRINT_ERROR("[demo] %s line %d error, pu1nal: %p(0x%02x), pu1Buf: %p, nalsize: 0x%08x, datasz: 0x%08x\r\n",
            __func__, __LINE__, pu1nal, pu1nal[0], loaddatainst->pu1Buf,
            nalsize, datasz);
          fgerr = true;
          break;
        }
      }
      if (fgerr) {
        break;
      }
      if ((datasz > 0) && (loaddatainst->pu1Buf <= pu1data) &&
         (pu1data + datasz <= loaddatainst->pu1Buf + loaddatainst->bufsz)) {
        PRINT_INFO( "[demo] %s line %d memmove(pu1Buf: %p, pu1data: %p, datasz: 0x%08x)\r\n",
          __func__, __LINE__, loaddatainst->pu1Buf, pu1data, datasz);
        memmove(loaddatainst->pu1Buf, pu1data, datasz);
        memset(loaddatainst->pu1Buf + datasz, 0, loaddatainst->bufsz - datasz);
        loaddatainst->bufofst = datasz;
      } else {
        PRINT_ERROR("[demo] %s line %d exit while, for error pu1data: %p, datasz: 0x%08x\r\n",
          __func__, __LINE__, pu1data, datasz);
        break;
      }
    } else {
      PRINT_ERROR("[demo] %s line %d exit while, readsz: %d(0x%08x)\r\n",
        __func__, __LINE__, readsz, readsz);
      if (loaddatainst->bufofst > 0) {
        PRINT_ERROR("[demo] line %d --  atc_vdec_decode(%p, 0x%08x, fileofst: 0x%llx)\r\n",
          __LINE__, loaddatainst->pu1Buf,
          loaddatainst->bufofst, loaddatainst->data_fileofst);
        loaddatainst->cb(loaddatainst->pu1Buf, loaddatainst->bufofst, loaddatainst->pvarg);
        loaddatainst->data_fileofst += loaddatainst->bufofst;
      }

      if (loaddatainst->loop) {
          off_t res = lseek (loaddatainst->fd, 0, SEEK_SET);
          if (res != 0)
            break;
      } else {
        loaddatainst->cb(loaddatainst->pu1Buf, 0, loaddatainst->pvarg);
        break;
      }
    }
#else
    if (0 < readsz) {
      PRINT_INFO("[demo] %s line %d -- atc_vdec_decode(%p, 0x%08x)\r\n",
        __func__, __LINE__, loaddatainst->pu1Buf, readsz);
      loaddatainst->cb(loaddatainst->pu1Buf, readsz, loaddatainst->pvarg);
    } else {
      PRINT_ERROR("[demo] %s line %d exit while, readsz: 0x%08x\r\n",
        __func__, __LINE__, readsz);
        loaddatainst->cb(loaddatainst->pu1Buf, readsz, loaddatainst->pvarg);
      break;
    }
#endif
  }
  PRINT_INFO( "[demo] %s line %d exit\r\n",
    __func__, __LINE__);
  return NULL;
}

bool StartDataLoad(void *inst)
{
  AtcLoadDataInst *loaddatainst = (AtcLoadDataInst *)inst;
  int err = 0;

  PRINT_INFO( "[demo] %s(%p) line %d enter\r\n",
    __func__, inst, __LINE__);

  if (NULL == loaddatainst) {
    PRINT_ERROR("[demo] %s line %d fail for invalid args\r\n",
      __func__, __LINE__);
    return false;
  }

  if (LOADDATA_ST_INITED != loaddatainst->state) {
    PRINT_ERROR("[demo] %s line %d fail for invalid state(%d) to start data load\r\n",
      __func__, __LINE__, loaddatainst->state);
    return false;
  }

  err = pthread_create(&(loaddatainst->tid),
    NULL, loaddata_readfile_proc, loaddatainst);
  if (err != 0) {
    PRINT_ERROR("[demo] %s line %d create thread fail because of %s\r\n",
      __func__, __LINE__, strerror(errno));
    return false;
  }

  loaddatainst->state = LOADDATA_ST_RUNING;
  PRINT_INFO("[demo] %s(%p) line %d success\r\n",
    __func__, inst, __LINE__);

  return true;
}

bool DataLoadRegDataReceive(void *inst, ATCHWDATARECFUNC cb, void *pvarg)
{
  AtcLoadDataInst *loaddatainst = (AtcLoadDataInst *)inst;

  PRINT_INFO("[demo] %s(%p) line %d enter\r\n",
    __func__, inst, __LINE__);

  if (NULL == loaddatainst) {
    PRINT_ERROR("[demo] %s line %d fail for invalid args\r\n",
      __func__, __LINE__);
    return false;
  }

  if (LOADDATA_ST_IDLE != loaddatainst->state) {
    PRINT_ERROR("[demo] %s line %d fail for invalid state(%d) to register data receive cb\r\n",
      __func__, __LINE__, loaddatainst->state);
    return false;
  }

  loaddatainst->cb = cb;
  loaddatainst->pvarg = pvarg;
  loaddatainst->state = LOADDATA_ST_INITED;

  PRINT_INFO("[demo] %s(%p) line %d success\r\n",
    __func__, inst, __LINE__);

  return true;
}

bool StopDataLoad(void *inst)
{
  AtcLoadDataInst *loaddatainst = (AtcLoadDataInst *)inst;

  PRINT_INFO("[demo] %s(%p) line %d enter\r\n",
    __func__, inst, __LINE__);

  if (NULL == loaddatainst) {
    PRINT_ERROR("[demo] %s line %d fail for invalid args\r\n",
      __func__, __LINE__);
    return false;
  }

  if (LOADDATA_ST_RUNING != loaddatainst->state) {
    PRINT_ERROR("[demo] %s line %d fail for invalid state(%d) to register data receive cb\r\n",
      __func__, __LINE__, loaddatainst->state);
    return true;
  }

  loaddatainst->need_stop = true;

  loaddatainst->state = LOADDATA_ST_STOPPING;

  PRINT_INFO("[demo] %s(%p) line %d exit\r\n",
    __func__, inst, __LINE__);
  return true;
}


bool CloseDataLoad(void *inst)
{
  AtcLoadDataInst *loaddatainst = (AtcLoadDataInst *)inst;

  PRINT_INFO("[demo] %s(%p) line %d enter\r\n",
    __func__, inst, __LINE__);
  if (NULL == loaddatainst) {
    PRINT_ERROR("[demo] %s line %d fail for invalid args\r\n",
      __func__, __LINE__);
    return false;
  }

  PRINT_INFO("[demo] %s(%p) line %d -- state: %d\r\n",
    __func__, inst, __LINE__, loaddatainst->state);
  if (LOADDATA_ST_RUNING == loaddatainst->state) {
    PRINT_INFO( "[demo] %s(%p) line %d --> StopDataLoad()\r\n",
      __func__, inst, __LINE__);
    StopDataLoad(loaddatainst);
  }

  if (LOADDATA_ST_STOPPING == loaddatainst->state) {
    PRINT_INFO("[demo] %s(%p) line %d --> pthread_join\r\n",
      __func__, inst, __LINE__);
    pthread_join(loaddatainst->tid, NULL);
    PRINT_INFO("[demo] %s(%p) line %d --> pthread_join OK\r\n",
      __func__, inst, __LINE__);
  }

  if (loaddatainst->fd >= 0)
    close(loaddatainst->fd);
  if (NULL != loaddatainst->pu1Buf)
    free(loaddatainst->pu1Buf);
  if (NULL != loaddatainst->szfilename)
    free(loaddatainst->szfilename);
  if (NULL != loaddatainst)
    free(loaddatainst);

  PRINT_INFO("[demo] %s(%p) line %d success\r\n",
    __func__, inst, __LINE__);
  return true;
}

