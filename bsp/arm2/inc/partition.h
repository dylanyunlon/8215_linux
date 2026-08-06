#ifndef _PRELOADER_PARTITION_H
#define _PRELOADER_PARTITION_H

#define NEW_PARTITION_DESIGN

#define DATAZONE_BCB_SIZE   4096
#define DATAZONE_PART_INFO_SIZE 4096
#define DATAZONE_PARTITION_OFFSET   4096 + DATAZONE_BCB_SIZE

typedef  struct partitionread{
	char      szPartName[20];
	char      szType[20];
	unsigned int u4Mount;
	unsigned long long  u8PartitionStartAddr;
	unsigned long long  u8PartitionSize;
	unsigned int  u4LastPartition;
#ifdef NEW_PARTITION_DESIGN
	char  szImageFileName[48];
#else
	char  szImageFileName[40];
#endif
	unsigned long long u8RealDataSize;
#ifdef NEW_PARTITION_DESIGN
	unsigned int u4Flag;
#endif
	struct partitionread *nextpartition;
}partitionread;

typedef  struct partitionhead{
	unsigned int blockcnt;
	unsigned int u4Version;
	unsigned int u4Signature;
	struct partitionread *nextpartition;
	char checksum[4];
}partitionhead;

#endif
