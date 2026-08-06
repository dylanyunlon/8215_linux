/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef _ERR_NUM_H_
#define _ERR_NUM_H_

struct errnum_str {
	int errnum;
	const char *errstr;
};

#define UPDATED_SUCCESS               0
#define INSTALL_SUCCESS               0
#define UPDATED_ERR_PART_DAMAGED    (-1)
#define UPDATED_ERR_FILE_DEMAGED    (-2)
#define UPDATED_ERR_OUT_OF_MEMORY   (-3)
#define UPDATED_ERR_DISK_NOT_EXIST (-4)
#define UPDATED_ERR_FILE_NOT_EXIST (-5)
#define UPDATED_ERR_TABLE_BAD      (-6)
#define INSTALL_ERROR              (-7)

#define ESUCCESS					0 //success
#define EINVAL						1 //Invalid argument
#define ESYSCALL					2 // SystemCall fail, eg open() fail.
#define EDATAZONE					3 //DataZone Error
#define EBCB						4 //BCB Error
#define EPARTTBL					5 //Partition Table Error
#define EPARTITION					6 //Partition Error, eg can NOT lookup one parition in partitions by partname.
#define EPARTCHG					7 //Partition Changed
#define ENOMEM						8 //Out of memory
#define ENOFILE						9 //File NOT Exsit
#define EFILELEN					10 //File Length Error
#define EPTHREAD					11 //pthread related functions fail.
#define EBADMD5						12 //Bad MD5 Check
#define ERDBACKCHK					13 //Read back check fail
#define EMOUNT						14 //mount error
#define EBADVERN					15 //get version fail
#define EVERNOMATCH					16 //upgrade version not match eg app version not match core
#define ESPARSEFILE					17 //Sparse format file error
#define EEMMCERASE					18 //erease emmc fail
#define EEMMCSIZE					19 //get emmc total size fail
#define EXMLGET						20 //get xml name fail
#define ECMDLINE					21 //Get or parse cmdline error
#define EBADBOOTDEV					22 //bad boot device
#define EMETAZONE					23 //metazone related calls fail
#define EMCUUPG						24 //MCU Firmware upg fail
#define ENOMCUFILE					25 //No mcu file
#define ENOMCUIMG					26 //no mcu image in sdcard
#define ENANDINFO					27 //get nand info fail
#define ENANDRW						28 //nand_rw_start/nand_rw_end fail
#define ENANDWR						29 //write nand fail
#define ENANDRD						30 //read nand fail
#define ENANDPTNOALIGN				31 //partition table start address not align to nand page size
#define ENANDERASE					32 //erease nand fail
#define ENANDALIGN					33 //align check fail
#define ENANDRSVBLK					34 //reserved blk num error
#define EBLHEADER					35 //bootloader header fail
#define EUPGHDR						36 //verfiy upgrade head fail


const char *strerrnum(int errnum);


#endif /* _ERR_NUM_H_ */
