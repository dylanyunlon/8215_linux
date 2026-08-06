#ifndef __DATA_ZONE_H__
#define __DATA_ZONE_H__

#include "x_typedef.h"

#define IMAGE_STRING_LEN            16  // chars
#define MAX_IMAGE_DESCRIPTORS         3   // per sector

typedef U8 USHORT;
typedef U32 DWORD;
typedef struct _EDBG_ADDR{
    DWORD dwIP;
    USHORT wMAC[3];
    USHORT wPort;
} EDBG_ADDR;

typedef struct _CHAININFO {
    DWORD   dwLoadAddress;          // Load address in SDRAM
    DWORD   dwFlashAddress;         // Start location on the NAND
    DWORD   dwLength;               // The length of the image
} CHAININFO, *PCHAININFO;

typedef  struct tagDISPLAYINFO{

    UINT16 u2Width;
    UINT16 u2Height;
    UINT16 u2BitDepth;
    UINT16 u2Reserved;
}DISPLAYINFO;


typedef struct _IMAGE_DESCRIPTOR_T 
{
    DWORD dwVersion;    
    DWORD dwImageType;      // IMAGE_TYPE_ flags    
    DWORD dwIdentity;
    UCHAR ucString[IMAGE_STRING_LEN];   // e.g: "PocketPC_2002"
    DWORD dwLoadAddress;    // Virtual address to load image (ImageStart)
    DWORD dwLoadPhyAddr;
    DWORD dwJumpAddress;    // Virtual address to jump (StartAddress/LaunchAddr)
    DWORD dwJumpPhyAddr;
    DWORD dwStartSector;   // Logical Sectors
    DWORD dwTtlSectors;    // Logical Sectors
    DWORD dwStoreOffset;
} IMAGE_DESCRIPTOR_T, *PIMAGE_DESCRIPTOR_T;


typedef DWORD BSP_ARGS;
typedef U32      ULONG;
typedef struct _BOOTCFG 
{
    ULONG       ImageIndex;
    ULONG       ConfigFlags;
    ULONG       BootDelay;
    EDBG_ADDR   EdbgAddr;
    ULONG       SubnetMask;
    DWORD      dwLogMask;
    BSP_ARGS    *pBSPArgs;
} BOOT_CFG, *PBOOT_CFG;


typedef struct _DataZone {
    DWORD           dwVersion; 
    DWORD           dwIdentity;
    DWORD           dwSignature;
    BOOT_CFG        BootCfg;

    // Array of Image Descriptors.
    IMAGE_DESCRIPTOR_T    id[MAX_IMAGE_DESCRIPTORS];

    CHAININFO           chainInfo;
    DWORD           dwOALLogMask;
    BOOL            fFormatFAT;
    BOOL            fForceFormatAfterDownload;
    BOOL                    fSysHiveClean;
    BOOL                    fUserHiveClean;
    DWORD            dwDatazoneCopySign;
    DISPLAYINFO     DisplayInfo;
    BYTE reserved[512 - 260];
} TDataZone, *PTDataZone;

#endif