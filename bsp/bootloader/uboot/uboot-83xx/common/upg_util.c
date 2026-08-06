#include <common.h>
#include <stdarg.h>
//#include "upg_typedef.h"
#include <asm/arch/x_typedef.h>
#include "upg_util.h"
#include "upg_main.h"

INT32 UPG_LOG_LEVEL = UPG_LOG_INFO; //default log level

void UPG_LOG(int level, const char *fmt, ...)
{
	if (level <= UPG_LOG_LEVEL)
	{
		//printf(fmt);

		va_list args;
		int i;
		char printbuffer[CONFIG_SYS_PBSIZE];

		va_start (args, fmt);

		/* For this to work, printbuffer must be larger than
		 * anything we ever want to print.
		 */
		i = vsprintf (printbuffer, fmt, args);
		va_end (args);

		/* Print the string */
		puts (printbuffer);
	}
}

BOOL fgUint32MemCmp(UINT32 u4MemAddr1, UINT32 u4MemAddr2, UINT32 u4Size)
{
  UINT32 i = 0;
  while(i<u4Size)
  {
    if(*(UINT32*)(u4MemAddr1 + i) != *(UINT32*)((u4MemAddr2 + i)))
    {
      return FALSE;
    }
    i+=4;
  }
  return TRUE;
}


CHAR * catFilePathName(CHAR* szFullPath, CHAR* szPath, CHAR* szName) {
	BOOL bPathSlashEnd = FALSE;
	INT32 i4PathLen = 0;
	INT32 i4NameLen = 0;
	
	if (szName == NULL) {
		return NULL;
	}

	if (szPath != NULL) {	
		i4PathLen = strlen(szPath);
		i4NameLen = strlen(szName);

		CHAR* p;
		p = strrchr(szPath, '/');

		if (p == NULL) {
			bPathSlashEnd = FALSE;	
		} else if (strcmp(p, "/") == 0) {
			bPathSlashEnd = TRUE;
		} else {
			bPathSlashEnd = FALSE;
		}

		strcpy(szFullPath, szPath);

		if (!bPathSlashEnd) {
			strcat(szFullPath, "/");
		}

		strcat(szFullPath, szName);
	}else {
		strcpy(szFullPath, szName);
	}
	

	return szFullPath;
}

