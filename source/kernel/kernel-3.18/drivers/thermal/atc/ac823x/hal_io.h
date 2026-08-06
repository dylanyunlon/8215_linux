/********************************************************************************************
 *     LEGAL DISCLAIMER 
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED 
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS 
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, 
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY 
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, 
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK 
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION 
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *     
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH 
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, 
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE 
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
 *     
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS 
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.  
 ************************************************************************************************/

/******************************************************************************
*[File]             hal_io.h
*[Version]          v0.1
*[Revision Date]    2010-01-04
*[Author]           Kenny Hsieh
*[Description]
*    source file for global varabile and function in av_d directory
*
*
******************************************************************************/

#ifndef HAL_IO_H
#define HAL_IO_H

#include "x_hal_io.h"



// field access macro-----------------------------------------------------------

/* field macros */
#define Fld(wid,shft,ac)    (((UINT32)wid<<16)|(shft<<8)|ac)
#define Fld_wid(fld)    (UINT8)((fld)>>16)
#define Fld_shft(fld)   (UINT8)((fld)>>8)
#define Fld_ac(fld)     (UINT8)(fld)

/* access method*/
#define AC_FULLB0       1
#define AC_FULLB1       2
#define AC_FULLB2       3
#define AC_FULLB3       4
#define AC_FULLW10      5
#define AC_FULLW21      6
#define AC_FULLW32      7
#define AC_FULLDW       8
#define AC_MSKB0        11
#define AC_MSKB1        12
#define AC_MSKB2        13
#define AC_MSKB3        14
#define AC_MSKW10       15
#define AC_MSKW21       16
#define AC_MSKW32       17
#define AC_MSKDW        18

/* Reg32 to Reg8*/
#define REGB0(reg)  (reg)
#define REGB1(reg)  ((reg)+1)
#define REGB2(reg)  ((reg)+2)
#define REGB3(reg)  ((reg)+3)
/* Reg32 to Reg16*/
#define REGW0(reg)  (reg)
#define REGW1(reg)  ((reg)+1)
#define REGW2(reg)  ((reg)+2)

/* --------FLD help macros, mask32 to mask8,mask16,maskalign ----------*/
/* mask32 -> mask8 */
#define MSKB0(msk)  (UINT8)(msk)
#define MSKB1(msk)  (UINT8)((msk)>>8)
#define MSKB2(msk)  (UINT8)((msk)>>16)
#define MSKB3(msk)  (UINT8)((msk)>>24)
/* mask32 -> mask16 */
#define MSKW0(msk)  (UINT16)(msk)
#define MSKW1(msk)  (UINT16)((msk)>>8)
#define MSKW2(msk)  (UINT16)((msk)>>16)
                        /* mask32 -> maskalign */
#define MSKAlignB(msk)  (((msk)&0xff)?(msk):(\
            ((msk)&0xff00)?((msk)>>8):(\
            ((msk)&0xff0000)?((msk)>>16):((msk)>>24)\
        )\
    ))

/* --------FLD help macros, mask32 to mask8,mask16,maskalign ----------*/
#define Fld2Msk32(fld)  /*lint -save -e504 */ (((UINT32)0xffffffff>>(32-Fld_wid(fld)))<<Fld_shft(fld)) /*lint -restore */
#define Fld2MskB0(fld)  MSKB0(Fld2Msk32(fld))
#define Fld2MskB1(fld)  MSKB1(Fld2Msk32(fld))
#define Fld2MskB2(fld)  MSKB2(Fld2Msk32(fld))
#define Fld2MskB3(fld)  MSKB3(Fld2Msk32(fld))
#define Fld2MskBX(fld,byte) ((UINT8)(Fld2Msk32(fld)>>((byte&3)*8)))

#define Fld2MskW0(fld)  MSKW0(Fld2Msk32(fld))
#define Fld2MskW1(fld)  MSKW1(Fld2Msk32(fld))
#define Fld2MskW2(fld)  MSKW2(Fld2Msk32(fld))
#define Fld2MskWX(fld,byte) ((UINT16)(Fld2Msk32(fld)>>((byte&3)*8)))

#define Fld2MskAlignB(fld)  MSKAlignB(Fld2Msk32(fld))
#define FldshftAlign(fld)   ((Fld_shft(fld)<8)?Fld_shft(fld):(\
            (Fld_shft(fld)<16)?(Fld_shft(fld)-8):(\
            (Fld_shft(fld)<24)?(Fld_shft(fld)-16):(Fld_shft(fld)-24)\
        )\
    ))
#define ValAlign2Fld(val,fld)   ((val)<<FldshftAlign(fld))

#if 0
UINT16 u2HdmiRxIO32Read2B(UINT32 reg32);
void vHdmiRxIO32Write1BMsk(UINT32 reg32, UINT32 val8, UINT8 msk8);
void vHdmiRxIO32Write2BMsk(UINT32 reg32, UINT32 val16, UINT16 msk16);
void vHdmiRxIO32Write4BMsk(UINT32 reg32, UINT32 val32, UINT32 msk32);
#endif

// zplee
#define u2IO32Read2B(reg32) (*(volatile UINT8 *)(reg32))  //u2HdmiRxIO32Read2B(reg32)  
#define vIO32Write1BMsk(reg32, val8, msk8) (*(volatile UINT32 *)(reg32)=(val8)) //vHdmiRxIO32Write1BMsk(reg32, val8, msk8)
#define vIO32Write2BMsk(reg32, val16, msk16) (*(volatile UINT32 *)(reg32)=(val16)) //vHdmiRxIO32Write2BMsk(reg32, val16, msk16)
#define vIO32Write4BMsk(reg32, val32, msk32) (*(volatile UINT32 *)(reg32)=(val32)) //vHdmiRxIO32Write4BMsk(reg32, val32, msk32)

#define u1IO32Read1B(reg32) (*(volatile UINT8 *)(reg32))

#define u4IO32Read4B(reg32) (*(volatile UINT32 *)(reg32))

#define vIO32Write1B(reg32, val8) vIO32Write1BMsk(reg32,val8,0xff)
#define vIO32Write2B(reg32, val16) vIO32Write2BMsk(reg32,val16,0xffff)
#define vIO32Write4B(reg32,val32) (*(volatile UINT32 *)(reg32)=(val32))


#define u1RegRead1B(reg32) 		u1IO32Read1B(reg32)
#define u4RegRead4B(reg32) 		u4IO32Read4B(reg32)
#define vRegWrite4B(reg32,val32)	vIO32Write4B(reg32,val32)
#define vRegWrite4B_S(reg16,bByte3,bByte2,bByte1,bByte0)	(*(volatile UINT32 *)((reg16))=(((bByte3)<<24)|((bByte2)<<16)|((bByte1)<<8)|(bByte0)))

#if 0
#define vWriteHdmiRX(dAddr, dVal)  IO_WRITE32(0,dAddr, dVal)//*(volatile DWRD *)(IO_BASE_ADDRESS + HDMI_RGB_REG_OFFSET + dAddr) = dVal
#define dReadHdmiRX(dAddr)            IO_READ32(0,dAddr)//*(volatile DWRD *)(IO_BASE_ADDRESS + HDMI_RGB_REG_OFFSET + dAddr)
#define vWriteHdmiRXMsk(dAddr, dVal, dMsk) vWriteHdmiRX((dAddr), (dReadHdmiRX(dAddr) & (~(dMsk))) | ((dVal) & (dMsk)))
#endif

#if 0
#define IO32ReadFld(reg32,fld)  /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
    (((Fld_ac(fld)>=AC_FULLB0)&&(Fld_ac(fld)<=AC_FULLB3))?u1IO32Read1B((reg32)+(Fld_ac(fld)-AC_FULLB0)):( \
    ((Fld_ac(fld)>=AC_FULLW10)&&(Fld_ac(fld)<=AC_FULLW32))?u2IO32Read2B((reg32)+(Fld_ac(fld)-AC_FULLW10)):( \
    (Fld_ac(fld)==AC_FULLDW)? u4IO32Read4B(reg32):( \
    ((Fld_ac(fld)>=AC_MSKB0)&&(Fld_ac(fld)<=AC_MSKB3))?(u1IO32Read1B((reg32)+(Fld_ac(fld)-AC_MSKB0))&Fld2MskBX(fld,(Fld_ac(fld)-AC_MSKB0))):( \
    ((Fld_ac(fld)>=AC_MSKW10)&&(Fld_ac(fld)<=AC_MSKW32))?(u2IO32Read2B((reg32)+(Fld_ac(fld)-AC_MSKW10))&Fld2MskWX(fld,(Fld_ac(fld)-AC_MSKW10))):( \
    (Fld_ac(fld)==AC_MSKDW)?(u4IO32Read4B(reg32)&Fld2Msk32(fld)):0 \
  ))))))  /*lint -restore */
#endif

#if 0
#define IO32ReadFldAlign(reg32,fld) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
    (((Fld_ac(fld)>=AC_FULLB0)&&(Fld_ac(fld)<=AC_FULLB3))?u1IO32Read1B((reg32)+(Fld_ac(fld)-AC_FULLB0)):( \
    ((Fld_ac(fld)>=AC_FULLW10)&&(Fld_ac(fld)<=AC_FULLW32))?u2IO32Read2B((reg32)+(Fld_ac(fld)-AC_FULLW10)):( \
    (Fld_ac(fld)==AC_FULLDW)? u4IO32Read4B(reg32):( \
    ((Fld_ac(fld)>=AC_MSKB0)&&(Fld_ac(fld)<=AC_MSKB3))?((u1IO32Read1B((reg32)+(Fld_ac(fld)-AC_MSKB0))&Fld2MskBX(fld,(Fld_ac(fld)-AC_MSKB0)))>>((Fld_shft(fld)-8*(Fld_ac(fld)-AC_MSKB0))&7)):( \
    ((Fld_ac(fld)>=AC_MSKW10)&&(Fld_ac(fld)<=AC_MSKW32))?((u2IO32Read2B((reg32)+(Fld_ac(fld)-AC_MSKW10))&Fld2MskWX(fld,(Fld_ac(fld)-AC_MSKW10)))>>((Fld_shft(fld)-8*(Fld_ac(fld)-AC_MSKW10))&15)):( \
    (Fld_ac(fld)==AC_MSKDW)?((u4IO32Read4B(reg32)&Fld2Msk32(fld))>>Fld_shft(fld)):0 \
  ))))))  /*lint -restore */
#endif

#define vIO32WriteFld(reg32,val,fld) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
    (((Fld_ac(fld)>=AC_FULLB0)&&(Fld_ac(fld)<=AC_FULLB3))? vIO32Write1B((reg32)+(Fld_ac(fld)-AC_FULLB0),(val)):( \
    ((Fld_ac(fld)>=AC_FULLW10)&&(Fld_ac(fld)<=AC_FULLW32))?vIO32Write2B((reg32)+(Fld_ac(fld)-AC_FULLW10),(val)):( \
    (Fld_ac(fld)==AC_FULLDW)?vIO32Write4B((reg32),(val)):( \
    ((Fld_ac(fld)>=AC_MSKB0)&&(Fld_ac(fld)<=AC_MSKB3))?vIO32Write1BMsk((reg32)+(Fld_ac(fld)-AC_MSKB0),(val),Fld2MskBX(fld,(Fld_ac(fld)-AC_MSKB0))):( \
    ((Fld_ac(fld)>=AC_MSKW10)&&(Fld_ac(fld)<=AC_MSKW32))?vIO32Write2BMsk((reg32)+(Fld_ac(fld)-AC_MSKW10),(val),Fld2MskWX(fld,(Fld_ac(fld)-AC_MSKW10))):( \
    (Fld_ac(fld)==AC_MSKDW)?vIO32Write4BMsk((reg32),(val),Fld2Msk32(fld)):0\
    ))))))  /*lint -restore */

#define vIO32WriteFldAlign(reg32,val,fld) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
    (((Fld_ac(fld)>=AC_FULLB0)&&(Fld_ac(fld)<=AC_FULLB3))?vIO32Write1B((reg32)+(Fld_ac(fld)-AC_FULLB0),(val)):( \
    ((Fld_ac(fld)>=AC_FULLW10)&&(Fld_ac(fld)<=AC_FULLW32))?vIO32Write2B((reg32)+(Fld_ac(fld)-AC_FULLW10),(val)):( \
    (Fld_ac(fld)==AC_FULLDW)?vIO32Write4B((reg32),(val)):( \
    ((Fld_ac(fld)>=AC_MSKB0)&&(Fld_ac(fld)<=AC_MSKB3))?vIO32Write1BMsk((reg32)+(Fld_ac(fld)-AC_MSKB0),ValAlign2Fld((val),fld),Fld2MskBX(fld,(Fld_ac(fld)-AC_MSKB0))):( \
    ((Fld_ac(fld)>=AC_MSKW10)&&(Fld_ac(fld)<=AC_MSKW32))?vIO32Write2BMsk((reg32)+(Fld_ac(fld)-AC_MSKW10),ValAlign2Fld((val),fld),Fld2MskWX(fld,(Fld_ac(fld)-AC_MSKW10))):( \
    (Fld_ac(fld)==AC_MSKDW)?vIO32Write4BMsk((reg32),((UINT32)(val)<<Fld_shft(fld)),Fld2Msk32(fld)):0\
    )))))) /*lint -restore */

#if 0
#define vIO32WriteFldMulti(reg32,list) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
{ \
	UINT16 upk;\
	enum {msk=(INT32)(list)}; \
	{UINT8 upk;\
	((UINT32)msk==0xff)?vIO32Write1B(reg32,(list)):(\
	((UINT32)msk==0xff00)?vIO32Write1B(reg32+1,(list)>>8):(\
	((UINT32)msk==0xff0000)?vIO32Write1B(reg32+2,(list)>>16):(\
	((UINT32)msk==0xff000000)?vIO32Write1B(reg32+3,(list)>>24):(\
	((UINT32)msk==0xffff)?vIO32Write2B(reg32,(list)):(\
	((UINT32)msk==0xffff00)?vIO32Write2B(reg32+1,(list)>>8):(\
	((UINT32)msk==0xffff0000)?vIO32Write2B(reg32+2,(list)>>16):(\
	((UINT32)msk==0xffffffff)?vIO32Write4B(reg32,(list)):(\
	(((UINT32)msk&0xff)&&(!((UINT32)msk&0xffffff00)))?vIO32Write1BMsk(reg32,(list),(UINT8)(UINT32)msk):(\
	(((UINT32)msk&0xff00)&&(!((UINT32)msk&0xffff00ff)))?vIO32Write1BMsk(reg32+1,(list)>>8,(UINT8)((UINT32)msk>>8)):(\
	(((UINT32)msk&0xff0000)&&(!((UINT32)msk&0xff00ffff)))?vIO32Write1BMsk(reg32+2,(list)>>16,(UINT8)((UINT32)msk>>16)):(\
	(((UINT32)msk&0xff000000)&&(!((UINT32)msk&0x00ffffff)))?vIO32Write1BMsk(reg32+3,(list)>>24,(UINT8)((UINT32)msk>>24)):(\
	(((UINT32)msk&0xffff)&&(!((UINT32)msk&0xffff0000)))?vIO32Write2BMsk(reg32,(list),(UINT16)(UINT32)msk):(\
	(((UINT32)msk&0xffff00)&&(!((UINT32)msk&0xff0000ff)))?vIO32Write2BMsk(reg32+1,(list)>>8,(UINT16)((UINT32)msk>>8)):(\
	(((UINT32)msk&0xffff0000)&&(!((UINT32)msk&0x0000ffff)))?vIO32Write2BMsk(reg32+2,(list)>>16,(UINT16)((UINT32)msk>>16)):(\
	((UINT32)msk)?vIO32Write4BMsk(reg32,(list),((UINT32)msk)):0\
	)))))))))))))));\
	}\
}/*lint -restore */
#endif

#if 0
#define vIO32WriteFldD4val(reg32,dval32,list) /*lint -save -e506 -e504 -e514 -e62 -e737 -e572 -e961 -e648 -e701 -e732 -e571 */ \
{ \
	UINT16 upk;\
	enum {msk=(INT32)(list)};\
	{UINT8 upk;\
	vIO32Write4B(reg32,((dval32)&~(UINT32)msk)|(list));\
	}\
}/*lint -restore */
#endif

#endif  // X_HAL_IO_H
