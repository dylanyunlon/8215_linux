#ifndef _ATC_LOGO_RW_H_
#define _ATC_LOGO_RW_H_

typedef unsigned char byte;


#ifdef __cplusplus
extern "C" {
#endif


int atc_read_logo(byte *buf, int offset, int  size);
int atc_write_logo(byte *buf, int offset, int  size);


#ifdef __cplusplus
}
#endif

#endif /* _ATC_LOGO_RW_H_ */
