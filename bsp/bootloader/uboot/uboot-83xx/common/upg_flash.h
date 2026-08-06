#ifndef _UPG_FLASH_H_
#define _UPG_FLASH_H_

INT32 readDataFromNand(UINT32 dstaddr, UINT32 noffset, UINT32 size);

INT32 writeDataToNand(UINT32 * u4memStartAddr, UINT32 u4MemSize, UINT32 u4PartID, 
	UINT32 u4OffsetFromPart, UINT32 u4PartOffsetFromFlash, UINT32 u4PartSize);
	
#endif /*_UPG_FLASH_H_ */

