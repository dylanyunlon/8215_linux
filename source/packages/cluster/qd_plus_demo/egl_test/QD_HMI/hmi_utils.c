
/*****************************************************************************

File Name		 :	hmi_user_interface.c
Organization	:  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/
#include "hmi_all_struct_include.h"
#include "stdlib.h"
#include <time.h>
#include <stdarg.h>
#include "hmi_utils.h"


void HMI_Sprintf( HMI_CHAR_STR *OutText, const char *Text, ... )
{
    va_list AP;
    va_start( AP, Text );
#if HMI_FONT_CODE == HMI_FONT_CODE_UNICODE
    {
        int I, C;
        char *Head;
        uint16_t *Tail;
        int Length = vsprintf( ( char* )OutText, Text, AP );
        if ( Length )
        {
            Head = ( char* )OutText + Length;
            Tail = OutText + Length;
            for ( I = 0, C = Length + 1; I < C; I++ )
            {
                *Tail = *Head;
                Head--;
                Tail--;
            }
        }
    }
#else
    vsprintf( ( char* )OutText, Text, AP );
#endif
    va_end( AP );
}

UINT8 get_utf_byte_length(UINT8 *pUtfData)
{
	UINT8 length = 0;
	if(pUtfData != NULL)
	{
		while((*pUtfData != 0x00)&&(length <1000))
		{
			pUtfData++;
			length++;
		}
	}

	return length;
}

UINT8 get_unicode_byte_length(UINT16 *pUtfData)
{
	UINT8 length = 0;
	if(pUtfData != NULL)
	{
		while((*pUtfData != 0x00)&&(length <1000))
		{
			pUtfData++;
			length++;
		}
	}
	return length;
}

UINT8 MultiMedia_u8GetUtf8ByteSize(const UINT8 u8data)
{
	UINT8 bytesize;
	UINT8 utf8_data = u8data;
	if((utf8_data & 0x80) == 0x0)
		bytesize = 1;
	else if((utf8_data & 0xE0) == 0xc0)
		bytesize = 2;
	else if((utf8_data & 0xF0) == 0xE0)
		bytesize = 3;
	else if((utf8_data & 0xF8) == 0xF0)
		bytesize = 4;
	else if((utf8_data & 0xFC) == 0xF8)
		bytesize = 5;
	else if((utf8_data & 0xFE) == 0xFC)
		bytesize = 6;
	else
		bytesize = 0;
	return bytesize;
}

BOOLEAN MultiMedia_bUtfToUnic(UINT8 *pUtfData, UINT16 datalength, UINT16 *pOutput,UINT16 output_len)
{
	UINT8 b1	= 0u;
	UINT8 b2	= 0u;
	UINT8 b3	= 0u;
	UINT8 byteL	= 0u;
	UINT8 byteH	= 0u;
	UINT8 transResult	= 0u;
	UINT8 utfbytes		= 0u;
	UINT16 data_index	= 0u;
	UINT16 output_index	= 0u;
	//  UINT16 *pOutput =  pUnicData;    
	//  *pUnicData = 0;   
	
	if((pUtfData != NULL)&&(pOutput != NULL)&&(output_len > 0)&&(datalength > 0))
	{  
		output_len =output_len -1;/*the last char is 0*/
		while((data_index < datalength)&&(output_index < output_len))
		{
			transResult = 0;
			utfbytes = MultiMedia_u8GetUtf8ByteSize(pUtfData[data_index]);
			switch ( utfbytes )
			{
				case 1:
				{
					pOutput[output_index]     = (UINT16)(pUtfData[data_index]);
					data_index++;
					transResult = 1;
				}
				break;
					
				case 2:
				{
					if(data_index < datalength)
					{
						b1 = pUtfData[data_index];
						data_index++;
					}
					else
					{
						b1 = 0u;
					}
					
					if(data_index < datalength)
					{
						b2 = pUtfData[data_index];
						data_index++;
					}
					else
					{
						b2 = 0u;
					}
					//  if ( (b2 & 0xE0) != 0x80 )            
					//       return 0;                
					byteH = (b1 >> 2) & 0x07;
					byteL = (b1 << 6) + (b2 & 0x3F);
					
					pOutput[output_index] = (UINT16) byteH * 256 + byteL;
					
					transResult = 1;
				}            
					break;      
					
				case 3:
				{
					if(data_index < datalength)
					{
						b1 = pUtfData[data_index];
						data_index++;
					}
					else
					{
						b1 = 0u;
					}
					if(data_index < datalength)
					{
						b2 = pUtfData[data_index];
						data_index++;
					}
					else
					{
						b2 = 0u;
					}
					if(data_index < datalength)
					{
						b3 = pUtfData[data_index];
						data_index++;
					}
					else
					{
						b3 = 0u;
					}
					// if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) )                
					//    return 0;                
					byteH = (b1 << 4) + ((b2 >> 2) & 0x0F);
					byteL = (b2 << 6) + (b3 & 0x3F);
					if(output_index < output_len)
					{
						pOutput[output_index] = (UINT16) (byteH * 256 + byteL);
					}
					transResult = 1;
				}            
				break;
				case 4:
				{
					pOutput[output_index] = 0x0020;
					data_index+=4;
				}
				break;
				case 5:
				{
					pOutput[output_index] = 0x0020;
					data_index+=5;
				}
				break;
				case 6:
				{
					pOutput[output_index] = 0x0020;
					data_index+=6;
				}
				break;
				default:
					pOutput[0]=0;
					break;
			}
			output_index++;
		}
		pOutput[output_index]=0;
	}
	output_index > 0 ? (transResult = 1) : (transResult = 0);
	return transResult;
}

void Utf16ToUnicodeBuf(UINT8 *pUtfData, UINT16 datalength, UINT16 *pOutput, UINT16 output_len)
{
	UINT16 len;
	
	len = datalength;

	if (len > (output_len/2)) {
		len = (output_len/2);
	}
	//len = (len / 2) * 2;

	for (UINT16 i = 0; i < len; i++) {
		pOutput[i] = (UINT16)((((UINT16)pUtfData[i*2] << 8) & 0xff00) | (((UINT16)pUtfData[i*2+1]) & 0x00ff));
	}
}




UINT32 qd_txt_unicode_to_utf8(UINT32 letter_uni)
{
	uint32_t * res_p = NULL;
	uint8_t bytes[4];
	if(letter_uni < 128) 
		return letter_uni;

	if(letter_uni < 0x0800) {
		bytes[0] = ((letter_uni >> 6) & 0x1F) | 0xC0;
		bytes[1] = ((letter_uni >> 0) & 0x3F) | 0x80;
		bytes[2] = 0;
		bytes[3] = 0;
	}
	else if(letter_uni < 0x010000) {
		bytes[0] = ((letter_uni >> 12) & 0x0F) | 0xE0;
		bytes[1] = ((letter_uni >> 6) & 0x3F) | 0x80;
		bytes[2] = ((letter_uni >> 0) & 0x3F) | 0x80;
		bytes[3] = 0;
	}
	else if(letter_uni < 0x110000) {
		bytes[0] = ((letter_uni >> 18) & 0x07) | 0xF0;
		bytes[1] = ((letter_uni >> 12) & 0x3F) | 0x80;
		bytes[2] = ((letter_uni >> 6) & 0x3F) | 0x80;
		bytes[3] = ((letter_uni >> 0) & 0x3F) | 0x80;
	}

	res_p = (uint32_t *)bytes;
	return *res_p;
}







