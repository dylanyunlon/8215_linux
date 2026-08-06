#ifndef _UPG_UTIL_H_
#define _UPG_UTIL_H_

void UPG_LOG(int level, const char *fmt, ...);

BOOL fgUint32MemCmp(UINT32 u4MemAddr1, UINT32 u4MemAddr2, UINT32 u4Size);
CHAR * catFilePathName(CHAR* szFullPath, CHAR* szPath, CHAR* szName);

#endif /* _UPG_UTIL_H_ */

