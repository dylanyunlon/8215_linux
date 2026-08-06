#!/usr/bin/env python

#define REPLICATION_NUMBER  8
#static  char _szBLID1[12] = "BOOTLOADER!";
#static char _szBLNFIID2[8] = "NFIINFO";
#static char _szBLMSDCID2[8] = "MT3360A";

#typedef struct _NFIType
#{
#	UINT16   pageSize;   
#	UINT16   spareSize;
#	UINT16   addressCycle;   
#	UINT16   pageShift;
#} NFI_MENU;


#typedef struct _BOOTLHeader_
#{
#	char ID1[12];
#	char version[4];
#	UINT32 length;
#	UINT32 startAddr;
#	UINT32 checksum;
#	char ID2[8];
#	NFI_MENU  NFIinfo;
#	UINT16 pagesPerBlock;   
#	UINT16  totalBlocks;
#	UINT16  blockShift;
#	UINT16  linkAddr[6];   
#	UINT16  lastBlock;
#} BOOTL_HEADER;

import struct
import sys
import os

def calcchecksum(preloaderbuf):
        checksum=0
        for tmp in preloaderbuf:
		checksum=checksum^tmp
	
	return checksum

def get_bss_start_address(filename):
	mapfd = open(filename,'r')
	for eachLine in mapfd:
		eachLine = eachLine.strip('\n')
		symboladdress,symboltype,symbolname = eachLine.split(' ')
		if symbolname == '_bss_start' :
			mapfd.close()
			return int(symboladdress,16)
	return 0x0


def get_first_section_address(filename):
	mapfd = open(filename,'r')
	for eachLine in mapfd:
		eachLine = eachLine.strip('\n')
		symboladdress,symboltype,symbolname = eachLine.split(' ')
		if symbolname == '_end' :
			mapfd.close()
			return int(symboladdress,16)
	return 0x0


def build_header(checksum,preloaderlen):
	_szBLID1 = 'BOOTLOADER!'
	_szBLNFIID2= 'NFIINFO'
	_szBLMSDCID2= 'MT3360A'
	nfifmt='HHHH'
	headerfmt='12sIIII8s4HHHH6HH'
	ss = struct.pack(headerfmt,_szBLID1,0xCCCCCCCC,preloaderlen,0xCCCCCCCC,checksum,_szBLMSDCID2,0xCCCC,0xCCCC,0xCCCC,0xCCCC,0xCCCC,0xCCCC,0xCCCC,0xCCCC,0xCCCC,0xCCCC,0xCCCC,0xCCCC,0xCCCC,0xCCCC)
	return ss + ss + ss +ss +ss +ss +ss +ss


def allocat_buffer(size):
	i = 1
	buf= struct.pack('c',chr(0))
	while i < size:
		buf=buf+(chr(0))
		i=i+1
        return buf


def main(argv):
        
	i = 1
	while  i < len(argv):
		if argv[i] == '-map':
			mapfilename = argv[i+1]
			i=i + 2
		elif argv[i] == '-o':
			outfilename = argv[i+1]
			i=i + 2
		else:
			preloadername = argv[i]
			i=i+1;
	fd = open(preloadername,'rb')
	buf = fd.read()
	fd.close()
	
	preloaderlen = get_first_section_address(mapfilename) -0xF4000000
	pre_limit = get_bss_start_address(mapfilename) - 0xF4000000
        if pre_limit >= 0x7000:
		print("preloader is overflow size:", pre_limit)
		os.remove(preloadername)
		sys.exit(1)
	mychecksum = calcchecksum(struct.unpack(str(preloaderlen/4)+'I',buf[:preloaderlen]))
	
	header = build_header(mychecksum,preloaderlen)
	
	writefd = open(outfilename,'wb+')
        	
	writefd.write(header)
	writefd.write(buf)
	
	filelen = len(buf)
	rest = filelen%512
	if rest !=0 :
		print 'file not 512 align,need pad:%d' %(512 - rest)
		restbuf = allocat_buffer(512-rest)
		writefd.write(restbuf)

	writefd.close()


if __name__ == '__main__':
	main(sys.argv)
