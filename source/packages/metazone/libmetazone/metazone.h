#ifndef _LIBMETAZONE_H_
#define _LIBMETAZONE_H_
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*__cplusplus*/
unsigned int MetaZone_Init(void);
unsigned int MetaZone_Deinit(void);
unsigned int MetaZone_Read(unsigned int u4Idx, unsigned int *pu4Data);
unsigned int MetaZone_Write(unsigned int u4Idx, unsigned int u4Data);
unsigned int MetaZone_ReadBinary(unsigned int u4Idx, char *pbData, unsigned int u4Size);
unsigned int MetaZone_WriteBinary(unsigned int u4Idx, char *pbData, unsigned int u4Size);
unsigned int MetaZone_ReadReserved(char *pbData, unsigned int u4Size);
unsigned int MetaZone_WriteReserved(char *pbData, unsigned int u4Size);
unsigned int MetaZone_Flush(int fgblock);
#ifdef __cplusplus
#if __cplusplus
}	// End extern "C"
#endif
#endif /*__cplusplus*/
#endif
