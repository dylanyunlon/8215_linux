#ifndef VIDEO_TEST_H
#define VIDEO_TEST_H
#include <OMX_Core.h>
#include <OMX_Types.h>
#include <OMX_Component.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string>

#define VIDEO_TEST_OUTPUT_BUFFER_CNT 5
#define OMX_CORE_CONTROL_CMDQ_SIZE 10
typedef struct omx_buf_queue
{
    OMX_BUFFERHEADERTYPE* m_q[OMX_CORE_CONTROL_CMDQ_SIZE];
    unsigned m_read;
    unsigned m_write;
    unsigned m_size;
    pthread_mutex_t m_lockq;
}omx_buf_queue;


/* Round up to next aligened value. Alignment must be power of 2. */
#define ROUND_UP(a, b)      (((a) + (b) - 1) & ~((b) - 1))

typedef OMX_ERRORTYPE (*omx_get_handle)(OMX_HANDLETYPE * handle,
    OMX_STRING name, OMX_PTR data, OMX_CALLBACKTYPE * callbacks);
typedef OMX_ERRORTYPE (*omx_free_handle)(OMX_HANDLETYPE handle);


typedef struct _VIDEO_INPUT_BUFFER {
    OMX_U8 *handle;
    OMX_BUFFERHEADERTYPE *omx_inbufheader;
    OMX_U32 buffer_size;
    OMX_U32 filled_len;
}VIDEO_INPUT_BUF;

typedef struct _VIDEO_OUTPUT_BUFFER {
    OMX_U8 *handle;
    OMX_BUFFERHEADERTYPE *omx_outbufheader;
    OMX_U32 buffer_size;
    OMX_U32 filled_len;
}VIDEO_OUTPUT_BUF;

typedef struct _OMXComponent {
    OMX_HANDLETYPE omx_handle;
    void *core_library;
    VIDEO_INPUT_BUF input;
    OMX_STATETYPE omx_state;
    bool infinite_loop;

    sem_t state_semaphore;
    // Input memory pointer
    OMX_BUFFERHEADERTYPE  *m_inp_mem_ptr;
    int m_inp_buf_count;
    unsigned int m_inp_buf_size;
    // Output memory pointer
    OMX_BUFFERHEADERTYPE  *m_out_mem_ptr;
    int m_out_buf_count;
    unsigned int m_out_buf_size;
    omx_buf_queue m_in_buf_q;
    omx_buf_queue m_out_buf_q;
    VIDEO_OUTPUT_BUF output[VIDEO_TEST_OUTPUT_BUFFER_CNT];

    /* OpenMAX core library functions, protected with LOCK */
    OMX_ERRORTYPE (*init) (void);
    OMX_ERRORTYPE (*deinit) (void);
    OMX_ERRORTYPE (*get_handle) (OMX_HANDLETYPE * handle,
        OMX_STRING name, OMX_PTR data, OMX_CALLBACKTYPE * callbacks);
    OMX_ERRORTYPE (*free_handle) (OMX_HANDLETYPE handle);
}OMXComponent;

extern "C"{
void _video_test_setproperty(int width, int height, const std::string &name);
bool _video_test_prepare(OMXComponent *component);
bool _video_test_init(OMXComponent *component);
void _video_test_deinit(OMXComponent *pOmxComponent);
void _video_test_stop(OMXComponent *pOmxComponent);
bool omx_buf_queue_pop_entry(omx_buf_queue *queue, OMX_U64 *id);
bool omx_buf_queue_insert(omx_buf_queue *queue, OMX_U64 id);
}
#endif
