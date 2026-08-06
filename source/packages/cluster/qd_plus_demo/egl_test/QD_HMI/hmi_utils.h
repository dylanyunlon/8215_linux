/*****************************************************************************

File Name        :  hmi_user_interface.h
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/

#ifndef _HMI_UTILS_H
#define _HMI_UTILS_H

extern void HMI_Sprintf( HMI_CHAR_STR *OutText, const char *Text, ... );
extern UINT8 get_utf_byte_length(UINT8 *pUtfData);
extern UINT8 get_unicode_byte_length(UINT16 *pUtfData);
extern BOOLEAN MultiMedia_bUtfToUnic(UINT8 *pUtfData, UINT16 datalength, UINT16 *pOutput,UINT16 output_len);
extern void Utf16ToUnicodeBuf(UINT8 *pUtfData, UINT16 datalength, UINT16 *pOutput, UINT16 output_len);
extern UINT32 qd_txt_unicode_to_utf8(UINT32 letter_uni);

#endif

