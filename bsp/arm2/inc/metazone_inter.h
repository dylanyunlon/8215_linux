
#ifndef _METAZONE_INTER_H_
#define _METAZONE_INTER_H_

//----------------------------------------------------------

typedef struct _MetaZone {
    UINT32       dwVersion; 
    UINT32       dwSignature;
    UINT32       dwDataSize;         // Metazone size (in bytes) 
    UINT32       dwReserveSize;
    UINT32       dwValueOffset;
    UINT32       dwValueNum;
    UINT32       dwBinaryOffset;   //
    UINT32       dwBinaryNum;      // Number of binary data
    UINT32       dwBinaryItemSize; // Max bytes of one item of binary data.
} TMetaZone, *PTMetaZone;           // limited size is 3 page size, that might be 3*512 bytes

typedef struct _Flag{
	UINT32 write_before;
	UINT32 write_after;
}Wflag;

typedef struct _CRC16{
    unsigned short crc_dword;
	unsigned short crc_binary;
	unsigned short crc_reserved;
	unsigned short align;
}CRCflag;

typedef enum {
   BACKUP_NONE,
   BACKUP_MTZ1,
   BACKUP_MTZ2
}backup_mtz;

#define METAZONE_SIGNATURE 0xabcdef01
#define METAZONE_SIZE_MAX  0x10000
#define METAZONE_VERSION  0x00010000

#define ERROR_INAVLID_METAZONE_SIGNATURE 100L
#define ERROR_INAVLID_METAZONE_SIZE      101L

#define MZ_SUCCESS  0x00000000
#define MZ_FAILURE  0x80000000
#define MZ_WR_IDX_START  0x10000

#define  MZ_DISPLAY_BRIGHTNESS        (MZ_WR_IDX_START + 0x35)
#define  MZ_DISPLAY_CONTRAST        (MZ_WR_IDX_START + 0x36)
#define  MZ_DISPLAY_SATURATION        (MZ_WR_IDX_START + 0x37)
#define  MZ_DISPLAY_DITHER        (MZ_WR_IDX_START + 0x38)
#define  MZ_DISPLAY_BACKLIGHT        (MZ_WR_IDX_START + 0x39)
#define  MZ_DISPLAY_RESOLUTION    (MZ_WR_IDX_START + 0x40)
#define  MZ_DISPLAY_ROTATE    (MZ_WR_IDX_START + 0x41)


#define  MZ_DISPLAY_GAMMA        (0x1000c)


#define  MZ_RES_OFFSET_0    (0x0)
#define  MZ_RES_OFFSET_3K   (0x400*3)
#define  MZ_RES_OFFSET_7K   (0x400*7)

#define MTZ_NAME "metazone"
UINT32 Metazone_Init(void);
UINT32  _MetaZone_Read(UINT32 u4Idx, UINT32 *pu4Data);
UINT32  _MetaZone_ReadReserved(BYTE *pbData, UINT32 u4Size);
UINT32  _MetaZone_ReadBinary(UINT32 u4Idx, BYTE *pbData, UINT32 u4Size);
UINT32  _MetaZone_ReadReserved_Offset(BYTE *pbData,UINT32 offset, UINT32 u4Size);

UINT32  _MetaZone_Write(UINT32 u4Idx, UINT32 u4Data);
UINT32  _MetaZone_WriteReserved(BYTE *pbData, UINT32 u4Size);
UINT32  _MetaZone_WriteReserved_Offset(BYTE *pbData,UINT32 offset, UINT32 u4Size);
UINT32  _MetaZone_WriteBinary(UINT32 u4Idx, BYTE *pbData, UINT32 u4Size);

#endif //_METAZONE_INTER_H_


