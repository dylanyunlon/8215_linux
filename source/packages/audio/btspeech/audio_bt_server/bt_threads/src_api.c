#include "src_api.h"
#define MIN_32 (Word32)0x80000000L
#define MAX_32 (Word32)0x7fffffffL
typedef long long Word64;
typedef int Word32;

Word16 SRC_Coef[SRC_TAP_LENGTH] =
{ -6, 8, 24, -49, -156, -117, 50, 90, -65,
-119, 68, 156, -67, -202, 61, 256, -47, -320,
23, 396, 14, -485, -67, 594, 143, -729, -255,
909, 426, -1168, -732, 1526, 1223, -2326, -2535, 5116,
14132, 14132, 5116, -2535, -2326, 1223, 1526, -732, -1168,
426, 909, -255, -729, 143, 594, -67, -485, 14,
396, 23, -320, -47, 256, 61, -202, -67, 156,
68, -119, -65, 90, 50, -117, -156, -49, 24,
8, -6 };

Word16 SRC_Coefx2[SRC_TAP_LENGTH] =
{ -12, 16, 48, -98, -312, -234, 100, 180, -130,
-238, 136, 312, -134, -404, 122, 512, -94, -640,
46, 792, 28, -970, -134, 1188, 286, -1458, -510,
1818, 852, -2336, -1464, 3052, 2446, -4652, -5070, 10232,
28264, 28264, 10232, -5070, -4652, 2446, 3052, -1464, -2336,
852, 1818, -510, -1458, 286, 1188, -134, -970, 28,
792, 46, -640, -94, 512, 122, -404, -134, 312,
136, -238, -130, 180, 100, -234, -312, -98, 48,
16, -12
};



static __inline Word32 sat_fr1x32(Word64 v)
{
	Word32 ret;
	ret = (Word32)v;
	if (v < MIN_32) ret = MIN_32;
	else if (v > MAX_32) ret = MAX_32;

	return ret;
}
static __inline Word32 add_fr1x32(Word32 v1, Word32 v2)
{
	return   sat_fr1x32(((Word64)v1 + v2));
}
static __inline Word16 extract_h(Word32 v1)
{
	return ((Word16)(v1 >> 16));
}
static __inline Word16 round_fr1x32(Word32 L_var1)
{
	Word16 var_out;
	Word32 L_rounded;
	L_rounded = add_fr1x32(L_var1, (Word32)0x00008000L);
	var_out = extract_h(L_rounded);

	return (var_out);
}

static __inline Word32 L_mult(Word16 in1, Word16 in2)
{
	Word32 ret;

	ret = (Word32)in1 * (Word32)in2;
	if (ret != (Word32)0x40000000L){
		ret *= 2;
	}
	else{
		ret = MAX_32;
		return (ret);
	}
	return (ret);

}

void SRC_init(SRC_struct* SRC)
{

	Word16 i;

	for (i = 0; i < SRC_TAP_LENGTH; i++)
		SRC->src_buffer[i] = 0;

	SRC->src_ptr = 0;
}

void SRC_downsample_x2(SRC_struct *src_struct, Word16 *in, Word16 *out)
{

	Word16 *ptr_sesd_input = in;
	Word16 j, k, l;
	Word64 FIR_temp;
	Word16 temp_buffer[320];
	Word16 *ptr_sesd_output = &temp_buffer[0];

	for (j = 0; j < 320; j += 2){
		// A. update FIR buffer
		src_struct->src_buffer[src_struct->src_ptr] = *ptr_sesd_input;
		ptr_sesd_input++;
		src_struct->src_ptr--;
		if (src_struct->src_ptr < 0)
			src_struct->src_ptr = SRC_TAP_LENGTH - 1;
		src_struct->src_buffer[src_struct->src_ptr] = *ptr_sesd_input;
		ptr_sesd_input++;
		// B. do Poly Phase FIR
		l = src_struct->src_ptr;
		FIR_temp = 0;
		for (k = 0; k< SRC_TAP_LENGTH; k++){
			FIR_temp += L_mult(SRC_Coef[k], src_struct->src_buffer[l]);
			l++;
			if (l >(SRC_TAP_LENGTH - 1))
				l = 0;
		}

		src_struct->src_ptr--;
		if (src_struct->src_ptr < 0)
			src_struct->src_ptr = SRC_TAP_LENGTH - 1;

		//Decade 6 dB, Restore it @ BKF
		*ptr_sesd_output = round_fr1x32(sat_fr1x32(FIR_temp));
		ptr_sesd_output++;
	}
	for (j = 0; j < 160; j++)
		out[j] = temp_buffer[j];
}


void SRC_upsample_x2(SRC_struct *src_struct, Word16 *in, Word16 *out)
{

	Word16 *ptr_sesd_input = in;
	Word16 j, k, l;
	Word64 FIR_temp, FIR_temp2;
	Word16 temp_buffer[320];
	Word16 *ptr_sesd_output = &temp_buffer[0];

	for (j = 0; j < 320; j += 2){
		// A. update FIR buffer
		src_struct->src_buffer[src_struct->src_ptr] = *ptr_sesd_input;
		ptr_sesd_input++;

		// B. do Poly Phase FIR
		l = src_struct->src_ptr;
		FIR_temp = 0;
		FIR_temp2 = 0;
		for (k = 0; k< SRC_TAP_LENGTH; k += 2){
			FIR_temp += L_mult(SRC_Coefx2[k], src_struct->src_buffer[l]);
			FIR_temp2 += L_mult(SRC_Coefx2[k + 1], src_struct->src_buffer[l]);
			l++;
			if (l >(SRC_TAP_LENGTH - 1)) l = 0;
		}

		src_struct->src_ptr--;
		if (src_struct->src_ptr < 0)
			src_struct->src_ptr = SRC_TAP_LENGTH - 1;

		//Decade 6 dB, Restore it @ BKF
		*ptr_sesd_output = round_fr1x32(sat_fr1x32(FIR_temp));
		ptr_sesd_output++;
		*ptr_sesd_output = round_fr1x32(sat_fr1x32(FIR_temp2));
		ptr_sesd_output++;
	}

	for (j = 0; j < 320; j++)
		out[j] = temp_buffer[j];
}

