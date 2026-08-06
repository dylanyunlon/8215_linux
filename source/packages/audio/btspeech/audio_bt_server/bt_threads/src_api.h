#ifndef _SRC_API_H
#define _SRC_API_H


#define SRC_TAP_LENGTH 74

typedef short Word16;

typedef struct
{
    Word16 src_buffer[SRC_TAP_LENGTH];
    Word16 src_ptr;

} SRC_struct;

void SRC_init(SRC_struct* SRC);
void SRC_downsample_x2(SRC_struct *src_struct, Word16 *in, Word16 *out);
void SRC_upsample_x2(SRC_struct *src_struct, Word16 *in, Word16 *out);

#endif