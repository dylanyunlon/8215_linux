
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

typedef struct _CRC16 {
    UINT16 crc_dword;
    UINT16 crc_binary;
    UINT16 crc_reserved;
    UINT16 crc_header;
} CRCflag;

typedef struct _SlotHeader {
    UINT32 magic;        /* METAZONE_SLOT_MAGIC */
    UINT32 seq;          /* Sequence number for latest detection */
    UINT8  state;        /* 0x01: writing, 0xFF: valid */
    UINT8  reserved[3];
    CRCflag crc;         /* CRC of the metazone data */
} SlotHeader;

#define METAZONE_SIGNATURE 0xabcdef01
#define MTZ_SLOT_SIZE   0x10000  /* 64KB */
#define METAZONE_VERSION  0x00010000
#define METAZONE_SLOT_MAGIC 0x12345678U
#define MTZ_SLOT_HEADER_SIZE 512
#define MTZ_ACTIVE_BLOCKS   2  /* Use 2 blocks for cycling */
#define MZ_WR_IDX_START  0x10000

//index
#define MZ_UPGRADE_MODE_IDX (MZ_WR_IDX_START + 0x66)
UINT32 Metazone_Init(void);
UINT32  _MetaZone_Read(UINT32 u4Idx, UINT32 *pu4Data);

#endif //_METAZONE_INTER_H_

