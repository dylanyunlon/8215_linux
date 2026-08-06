#include "recovery.h"
#include "err_num.h"


static struct errnum_str errnum_str[] = {
    {ESUCCESS, "Success"},
    {-EINVAL, "Invalid argument"},
    {-ESYSCALL,"SystemCall fail, eg open() fail"},
    {-EDATAZONE,"DataZone Error"},
    {-EBCB,"BCB Error"},
    {-EPARTTBL,"Partition Table Error"},
    {-EPARTITION,"Partition Error, eg can NOT lookup one parition in partitions by partname."},
    {-EPARTCHG,"Partition Changed"},
    {-ENOMEM,"Out of memory"},
    {-ENOFILE,"File NOT Exsit"},
    {-EFILELEN,"File Length Error"},
    {-EPTHREAD,"pthread related functions fail"},
    {-EBADMD5,"Bad MD5 Check"},
    {-ERDBACKCHK,"Read back check fail"},
    {-EMOUNT,"mount error"},
    {-EBADVERN,"get version fail"},
    {-EVERNOMATCH,"upgrade version not match eg app version not match core"},
    {-ESPARSEFILE,"Sparse format file error"},
    {-EEMMCERASE,"erease emmc fail"},
    {-EEMMCSIZE,"get emmc total size fail"},
    {-EXMLGET,"get xml name fail"},
    {-ECMDLINE,"Get or parse cmdline error"},
    {-EBADBOOTDEV,"bad boot device"},
    {-EMETAZONE,"metazone related calls fail"},
    {-EMCUUPG, "MCU Firmware upg fail"},
    {-ENOMCUFILE, "No mcu file"},
    {-ENOMCUIMG, "no mcu image in sdcard"},
    {-ENANDINFO, "get nand info fail "},
    {-ENANDRW, "nand_rw_start/nand_rw_end fail"},
    {-ENANDWR, "write nand fail"},
    {-ENANDRD, "read nand fail"},
    {-ENANDPTNOALIGN, "partition table start address not align to nand page size"},
    {-ENANDERASE, "erase nand fail"},
    {-ENANDALIGN, "align check fail"},
    {-ENANDRSVBLK, "reserved blk num error"},
    {-EBLHEADER, "bootloader header fail"},
    {-EUPGHDR, "verfiy upgrade head fail"},
};

const char *strerrnum(int errnum)
{
    int i = 0;

    for (i = 0; i < ARRAY_SIZE(errnum_str); i++) {
        if (errnum_str[i].errnum == errnum)
            return errnum_str[i].errstr;
    }

    return "Unknown err num.";
}

