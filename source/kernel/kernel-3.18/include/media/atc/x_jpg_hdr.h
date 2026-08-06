#ifndef __X_JPG_HDR_H__
#define __X_JPG_HDR_H__

#include "x_os.h"
#include "x_gfx.h"

#define MAX_JPEG_COMPONENT      3
#define D_MAX_NUM_APP0          2
#define D_MAX_NUM_SCAN          32

// JPEG marker codes 
typedef enum
{
    M_SOF0  = 0xffc0,
    M_SOF1  = 0xffc1,
    M_SOF2  = 0xffc2,
    M_SOF3  = 0xffc3,

    M_SOF5  = 0xffc5,
    M_SOF6  = 0xffc6,
    M_SOF7  = 0xffc7,

    M_JPG   = 0xffc8,
    M_SOF9  = 0xffc9,
    M_SOF10 = 0xffca,
    M_SOF11 = 0xffcb,

    M_SOF13 = 0xffcd,
    M_SOF14 = 0xffce,
    M_SOF15 = 0xffcf,

    M_DHT   = 0xffc4,

    M_DAC   = 0xffcc,

    M_RST0  = 0xffd0,
    M_RST1  = 0xffd1,
    M_RST2  = 0xffd2,
    M_RST3  = 0xffd3,
    M_RST4  = 0xffd4,
    M_RST5  = 0xffd5,
    M_RST6  = 0xffd6,
    M_RST7  = 0xffd7,

    M_SOI   = 0xffd8,
    M_EOI   = 0xffd9,
    M_SOS   = 0xffda,
    M_DQT   = 0xffdb,
    M_DNL   = 0xffdc,
    M_DRI   = 0xffdd,
    M_DHP   = 0xffde,
    M_EXP   = 0xffdf,

    M_APP0  = 0xffe0,
    M_APP1  = 0xffe1,
    M_APP2  = 0xffe2,
    M_APP3  = 0xffe3,
    M_APP4  = 0xffe4,
    M_APP5  = 0xffe5,
    M_APP6  = 0xffe6,
    M_APP7  = 0xffe7,
    M_APP8  = 0xffe8,
    M_APP9  = 0xffe9,
    M_APP10 = 0xffea,
    M_APP11 = 0xffeb,
    M_APP12 = 0xffec,
    M_APP13 = 0xffed,
    M_APP14 = 0xffee,
    M_APP15 = 0xffef,

    M_JPG0  = 0xfff0,
    M_JPG13 = 0xfffd,
    M_COM   = 0xfffe,

    M_TEM   = 0xff01,

    M_ERROR = 0x100
} JDEC_MARKER_CODE_T;

// jpeg format 
typedef enum
{
    E_JPG_UNKNOWN_FORMAT,
    E_JPG_BASELINE,
    E_JPG_EXTENDED_SEQUENTIAL_HUFFMAN,
    E_JPG_PROGRESSIVE_HUFFMAN,
    E_JPG_EXTENDED_SEQUENTIAL_ARITHMETIC,
    E_JPG_PROGRESSIVE_ARITHMETIC,
    E_JPG_LOSSLESS_HUFFMAN,
    E_JPG_DIFFERENTIAL_SEQ_HUFFMAN,
    E_JPG_DIFFERENTIAL_PROGRESSIVE_HUFFMAN,
    E_JPG_DIFFERENTIAL_LOSSLESS_HUFFMAN,
    E_JPG_RESERVED_FOR_EXTENSIONS,
    E_JPG_LOSSLESS_ARITHMETIC,
    E_JPG_DIFFERENTIAL_SEQUENTIAL_ARITHMETIC,
    E_JPG_DIFFERENTIAL_PROGRESSIVE_ARITHMETIC,
    E_JPG_UNSUPPORT_FORMAT
} JDEC_FORMAT_T;

// component info in SOF marker 
typedef struct _JDEC_SOF_COMP_T
{
    __u16       u2CompCofSize;  //component cofficient buffer width
    __u8        u1ComponentId;
    __u8        u1HSampFactor;
    __u8        u1VSampFactor;
    __u8        u1QuantTblNo;
    __u8        u1CompWidth;  //component width
} JDEC_SOF_COMP_T;

// SOF data 
typedef struct _JDEC_JFIF_SOF_T
{
    __u16                    u2ImageHeight;
    __u16                    u2ImageWidth;
    __u8                     au1MapId2Index[256];
    JDEC_FORMAT_T             eJpegFormat;
    JDEC_SOF_COMP_T           arSofComp[MAX_JPEG_COMPONENT];
    __u8                     u1DataPrecision;
    __u8                     u1NumComponents;
} JDEC_JFIF_SOF_T;

// raw de-huffman table 
typedef struct _JDEC_DHT_HUFF_TBL_T
{
    __u8 au1Bits[20];/*17==>20, just for align with 4 bytes, by daihua.hu*/
    __u8 au1HuffVal[256];
} JDEC_DHT_HUFF_TBL_T;

// DHT data 
typedef struct _JDEC_JFIF_DHT_T
{
    __u32                    u4NumDcTbl;
    __u32                    u4NumAcTbl;
    __u32                    u4DcTblLoaded;     //bit mask for loaded dc table 
    __u32                    u4AcTblLoaded;     //bit mask for loaded ac table 
    JDEC_DHT_HUFF_TBL_T       arDcTbl[4];
    JDEC_DHT_HUFF_TBL_T       arAcTbl[4];
} JDEC_JFIF_DHT_T;

// DQT data 
typedef struct _JDEC_JFIF_DQT_T
{
    /*although we leave 2bytes  64 space here,
    if q table precision is 8bits, we use only
    first half (1x64) of this table*/
    
    __u8                aau1Qtbl[4][128];
    bool                 afgPrec[4];
    __u8                u1NumQ;
} JDEC_JFIF_DQT_T;

// SOS data
typedef struct _JDEC_JFIF_SOS_T
{
    __u8                u1Ss, u1Se, u1Ah, u1Al;
    __u8                u1CompInScan;
    __u8                au1CompIdx[MAX_JPEG_COMPONENT];
    __u8                au1DcId[MAX_JPEG_COMPONENT];
    __u8                au1AcId[MAX_JPEG_COMPONENT];
} JDEC_JFIF_SOS_T;

// Jpeg Picture Info data
typedef struct _JDEC_PIC_INFO_T
{
    JDEC_JFIF_DQT_T     rQTblInfo;
    JDEC_JFIF_DHT_T     rHuffTblInfo;
    JDEC_JFIF_SOF_T     rSofInfo;
    JDEC_JFIF_SOS_T     rSosInfo;
    __u32              u4RestartInterval;
    __u32              u4MaxHFactor;           ///< Max H Factor
    __u32              u4MaxVFactor;           ///< Max V Factor
    bool                fgEOI;
} JDEC_PIC_INFO_T;


#endif /* __X_JPG_HDR_H__ */
