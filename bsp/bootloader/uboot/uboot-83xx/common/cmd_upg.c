/**
 *Upgrade firmware entry. It responses for parse command from user input and check upgrade whether is
 * is success
 *
 */
#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <malloc.h>
//#include "upg_typedef.h"
#include <asm/arch/x_typedef.h>
#include "upg_main.h"
#include "upg_util.h"

#include <ac83xx_gpio.h>
#include <upg_config.h>

#ifdef CONFIG_CMD_UPG

#ifdef CONFIG_MMC
extern INT32 debugMMC(void);
extern INT32 MMC_card_exist(void);
#endif

//#if (CONFIG_COMMANDS & CFG_CMD_UPG)
/*
 * Subroutine:  do_upg
 *
 * Description:  Handler for 'upg' command..
 *
 * Inputs:   argv[1] contains the subcommand
 *
 * Return:    1  - command executed, repeatable
 *            0  - command executed but not repeatable, interrupted commands are
 *       always considered not repeatable
 *            -1 - not executed (unrecognized, bootd recursion or too many args)
 *           (If cmd is NULL or "" or longer than CFG_CBSIZE-1 it is
 *           considered unrecognized)
 *
 */
int do_upg(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    img_type_t imgType = IMG_TYPE_BE;       //Default is backend image
    upg_interface_t upgInterface = UPG_INTERFACE_USB;   //Default is upgrade from usb
    CHAR    *cmd1, *cmd2, *cmd3;
    CHAR    szError[100];
    INT32   i4Ret = -1;
    INT32   i4Ret1 = -1;
    INT32 upgBootPart = 0;
    INT32 upgBackupPart = 0;
    INT32   upg_part_type = UPG_PART_LIST;
    INT32   upg_part_id = -1;
    
    if (argc < 1) {
        goto usage;
    }

    if (argc == 1) {                        //upg
        imgType = IMG_TYPE_BE;
        upgInterface = UPG_INTERFACE_USB;
        upgBootPart = 0;
        upgBackupPart = 1;   
    } else {//argc > 1
        cmd1 = argv[1]; 
        if (strcmp(cmd1, "usb") == 0) {     //upg usb       
            if (argc == 2) {                //upg usb
                imgType = IMG_TYPE_BE;
                upgInterface = UPG_INTERFACE_USB;
                upgBootPart = 0;
                upgBackupPart = 1;      
            } else {//argc > 2
                cmd2 = argv[2];
                if (strcmp(cmd2, "be") == 0) {//upg usb be
                    if (argc == 3) {//upg usb be
                        imgType = IMG_TYPE_BE;
                        upgInterface = UPG_INTERFACE_USB;
                        upgBootPart = 0;
                        upgBackupPart = 1;  
                    } else {
                        cmd3 = argv[3];
                        if (strcmp(cmd3, "all") == 0) {//upg usb be all
                            imgType = IMG_TYPE_BE;
                            upgInterface = UPG_INTERFACE_USB;
                            upgBootPart = 1;
                            upgBackupPart = 1;
                        } else {
                            goto usage;
                        }
                    }               
                } else if (strcmp(cmd2, "fe") == 0) {//upg usb fe
                    if (argc > 3) {
                        goto usage;
                    }
                    imgType = IMG_TYPE_FE;
                    upgInterface = UPG_INTERFACE_USB;
                } else {
                    goto usage;
                }
            }           
        } 
#ifdef CONFIG_MMC        
      else if (strcmp(cmd1, "mmc") == 0) {     //upg mmc       
      
            printf ("do_upg : mmc interface\n");
    
            if (argc == 2) {                //upg mmc
                imgType = IMG_TYPE_BE;
                upgInterface = UPG_INTERFACE_MMC;
                upgBootPart = 0;
                upgBackupPart = 1;      
            } else {//argc > 2
                cmd2 = argv[2];
                if (strcmp(cmd2, "be") == 0) {//upg mmc be
                    if (argc == 3) {//upg mmc be
                        imgType = IMG_TYPE_BE;
                        upgInterface = UPG_INTERFACE_MMC;
                        upgBootPart = 0;
                        upgBackupPart = 1;  
                    } else {
                        cmd3 = argv[3];
                        if (strcmp(cmd3, "all") == 0) {//upg mmc be all
                            imgType = IMG_TYPE_BE;
                            upgInterface = UPG_INTERFACE_MMC;
                            upgBootPart = 1;
                            upgBackupPart = 1;
                        } else {
                            goto usage;
                        }
                    }               
                } else if (strcmp(cmd2, "fe") == 0) {//upg mmc fe
                    if (argc > 3) {
                        goto usage;
                    }
                    imgType = IMG_TYPE_FE;
                    upgInterface = UPG_INTERFACE_MMC;
                } else if (strcmp(cmd2, "debug") == 0) {//upg mmc debug
                    debugMMC();   // print out mmc debug commands
                    return 0;
                } else if (strcmp(cmd2, "insert") == 0) {//upg mmc insert
                    MMC_card_exist();   // print out mmc debug commands
                    return 0;                    
                } else {
                    goto usage;
                }
            }           
        } 
#endif      
		else if (strcmp(cmd1, "tftp")== 0) {//upg tftp...
         if (argc == 2) {                //upg tftp
                imgType = IMG_TYPE_BE;
                upgInterface = UPG_INTERFACE_TFTP;
                upgBootPart = 0;
                upgBackupPart = 1;      
            } else {//argc > 2
                cmd2 = argv[2];
                if (strcmp(cmd2, "be") == 0) {//upg tftp be
                    if (argc == 3) {//upg usb be
                        imgType = IMG_TYPE_BE;
                        upgInterface = UPG_INTERFACE_TFTP;
                        upgBootPart = 0;
                        upgBackupPart = 1;  
                    } else {
                        cmd3 = argv[3];
                        if (strcmp(cmd3, "all") == 0) {//upg tftp be all
                            imgType = IMG_TYPE_BE;
                            upgInterface = UPG_INTERFACE_TFTP;
                            upgBootPart = 1;
                            upgBackupPart = 1;
                        } else {
                            goto usage;
                        }
                    }               
                } else if (strcmp(cmd2, "fe") == 0) {//upg tftp fe
                    if (argc > 3) {
                        goto usage;
                    }
                    imgType = IMG_TYPE_FE;
                    upgInterface = UPG_INTERFACE_TFTP;
                } else {
                    goto usage;
                }
            }
            
        } else if (strcmp(cmd1, "log") == 0) {//upg log...
            if (argc > 3) {
                goto usage;
            }
            cmd2 = argv[2];
            if (strcmp(cmd2, "0") == 0) {
                UPG_LOG_LEVEL = UPG_LOG_ERROR;
            } else if (strcmp(cmd2, "1") == 0) {
                UPG_LOG_LEVEL = UPG_LOG_INFO;
            } else if (strcmp(cmd2, "2") == 0) {
                UPG_LOG_LEVEL = UPG_LOG_DEBUG;
            } else {
                printf ("Usage:\n%s\n", cmdtp->usage);
                return 1;
            }

            return 0;           
        } else if (strcmp(cmd1, "part") == 0) {//upg part...
            if (argc == 2) {                        //upg part
                imgType = IMG_TYPE_BE;
                upgInterface = UPG_INTERFACE_USB;
                upg_part_type = UPG_PART_LIST;  
            } else {//argc > 1
                cmd2 = argv[2]; 
                if (strcmp(cmd2, "list") == 0) {        //upg part list
                    if (argc > 3) {
                        goto usage;
                    }

                    imgType = IMG_TYPE_BE;
                    upgInterface = UPG_INTERFACE_USB;
                    upg_part_type = UPG_PART_LIST;          
                } else if (strcmp(cmd2, "reload") == 0) {       //upg part list
                    if (argc > 3) {
                        goto usage;
                    }

                    imgType = IMG_TYPE_BE;
                    upgInterface = UPG_INTERFACE_USB;
                    upg_part_type = UPG_PART_RELOAD;            
                } else if (strcmp(cmd2, "write") == 0) {        //upg part write
                    if (argc != 4) {
                        goto usage;
                    }
                    //upg part write id(01|02|03|...)
                    upg_part_id = (ulong)simple_strtoul(argv[3], NULL, 10);
                    if (upg_part_id <= 0) {
                        UPG_LOG(UPG_LOG_ERROR, "Invalid part id :%s\n", argv[3]);
                        goto usage;
                    }

                    imgType = IMG_TYPE_BE;
                    upgInterface = UPG_INTERFACE_USB;
                    upg_part_type = UPG_PART_WRITE;
                } else {
                    goto usage;
                }

            }

#if SUPPORT_UPG_FLASH_LED
            v_set_upg_status(UPG_START, 0);
#endif

            if (upg_part_type == UPG_PART_LIST) {
                UPG_LOG(UPG_LOG_INFO, "[UPG]List partition info table\n");
                i4Ret = listPITForBE(upgInterface);
                if (UPGR_OK == i4Ret) {
                    UPG_LOG(UPG_LOG_INFO, "[UPG]List partition info table successfully\n");
                    goto end_ok;
                } else {
                    UPG_LOG(UPG_LOG_ERROR, "[UPG]List partition info table failed\n");
                    goto end_ng;
                }
            } else if (upg_part_type == UPG_PART_RELOAD) {
                UPG_LOG(UPG_LOG_INFO, "[UPG]Reload Backend image\n");
                i4Ret = reloadImage(upgInterface,IMG_TYPE_BE, &upgBootPart, &upgBackupPart);
                if (UPGR_OK == i4Ret) {
                    UPG_LOG(UPG_LOG_ERROR, "[UPG]Reload Backend image successfully\n");
                    goto end_ok;
                } else {
                    UPG_LOG(UPG_LOG_ERROR, "[UPG]Reload Backend image failed\n");
                    goto end_ng;
                }
            } else if (upg_part_type = UPG_PART_WRITE) {
                UPG_LOG(UPG_LOG_INFO, "[UPG]Upgrade partially backend image partid=%d\n", upg_part_id);
                i4Ret = writePartBE(upgInterface, upg_part_id);
                if (UPGR_OK == i4Ret) {
                    UPG_LOG(UPG_LOG_ERROR, "[UPG]Upgrade partially backend image successfully\n");
                    goto end_ok;
                } else {
                    UPG_LOG(UPG_LOG_ERROR, "[UPG]Upgrade partially backend image failed\n");
                    goto end_ng;
                }
            } else {
                printf ("Usage:\n%s\n", cmdtp->usage);
                return -1;
            }
            
            
        } else {
            goto usage;
        }

    }

    if (upgInterface == UPG_INTERFACE_UNKNOWN || imgType == IMG_TYPE_UNKNOWN) {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return -1;                          //Execute failed
    }

    #if CFG_UPG_SUPPORT_JIGMODE
    vSetUpgStatus(TRUE);
    i4Ret = upgBEImage(upgInterface, upgBootPart, upgBackupPart);   //Upgrade backend image 
    if (UPGR_OK == i4Ret)
    {
      UPG_LOG(UPG_LOG_ERROR, "[UPG]Upgrade BE image successfully\n");
    }
    i4Ret1 = upgFEImage(upgInterface);   //upgrade frontend image
    if (UPGR_OK == i4Ret1)
    {
      UPG_LOG(UPG_LOG_ERROR, "[UPG]Upgrade FE image successfully\n");
    }
    
    if ( (UPGR_OK == i4Ret)||(UPGR_OK == i4Ret1)) {
		vSetUpgStatus(FALSE);
        UPG_LOG(UPG_LOG_ERROR, "[UPG]Flash Upgrade Finish \n");
        goto end_ok;
    } else {
        UPG_LOG(UPG_LOG_ERROR, "[UPG]Flash Upgrade Failed \n");
        goto end_ng;
    }
    #else
    if (imgType == IMG_TYPE_BE) {
        i4Ret = upgBEImage(upgInterface, upgBootPart, upgBackupPart);   //Upgrade backend image
    } else  if (imgType == IMG_TYPE_FE) {
        i4Ret = upgFEImage(upgInterface);   //upgrade frontend image
    }

    if (UPGR_OK == i4Ret) {
        UPG_LOG(UPG_LOG_ERROR, "[UPG]Upgrade image successfully\n");
        goto end_ok;
    } else {
        UPG_LOG(UPG_LOG_ERROR, "[UPG]Upgrade image failed\n");
        goto end_ng;
    }
    #endif

end_ok:
#if SUPPORT_UPG_FLASH_LED
	v_set_upg_status(UPG_END_OK, 0);
#endif
	return 0;

end_ng:
#if SUPPORT_UPG_FLASH_LED	
	v_set_upg_status(UPG_END_FAIL, 0);
#endif
	return 1;

usage:  
    printf("Usage:\n%s\n", cmdtp->usage);   
    return 1;   
}


U_BOOT_CMD(
    upg,    4,  1,  do_upg,
    "upg     - Upgrade image sub-system\n", //Simple help
    "                 - (Default)Upgrade backend image from usb(Not upgrade boot part, backup part)\n"
    "upg usb              - Upgrade backend image from usb(Not upgrade boot part, backup part)\n"
    "upg usb be           - Upgrade backend image from usb(Not upgrade boot part, backup part)\n"
    "upg usb be all       - Upgrade backend image from usb(Upgrade boot part, primary part, backup part)\n"
    "upg usb fe           - Upgrade frontend image from usb\n"
#ifdef CONFIG_MMC  
    "upg mmc              - Upgrade backend image from mmc(Not upgrade boot part, backup part)\n"
    "upg mmc be           - Upgrade backend image from mmc(Not upgrade boot part, backup part)\n"
    "upg mmc be all       - Upgrade backend image from mmc(Upgrade boot part, primary part, backup part)\n"
    "upg mmc fe           - Upgrade frontend image from mmc\n"
    "upg mmc debug      - mmc debug\n"
    "upg mmc insert      - mmc card detection\n"    
#endif
    "upg part             - (Default)List partition info table from backend image\n"
    "upg part list        - List partition info table from backend image\n"
    "upg part write id    - Upgrade partially backend image by partition id from usb\n"
    "upg part reload      - Reload backend image from usb\n"
    "upg log level        - Set log level(default is 0; 0:ERROR 1:INFO 2:DEBUG)\n" 
   
/*    
    "upg tftp       -Upgrade backend image from tftp server\n"
    "upg tftp be    -Upgrade backend image from tftp server\n"
    "upg tftp fe    -Upgrade frontend image from tftp server\n"
*/    
);


/*
int do_upg_part(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]){
    img_type_t imgType = IMG_TYPE_BE;       //Default is backend image
    upg_interface_t upgInterface = UPG_INTERFACE_USB;   //Default is upgrade from usb
    CHAR    *cmd1, *cmd2, *cmd3;
    INT32   upg_part_type = UPG_PART_LIST;
    INT32   upg_part_id = -1;
    
    CHAR    szError[100];
    INT32   u4Ret = -1;

    if (argc < 1) {
        goto usage;
    }

    if (argc == 1) {                        //upg_part
        imgType = IMG_TYPE_BE;
        upgInterface = UPG_INTERFACE_USB;
        upg_part_type = UPG_PART_LIST;  
    } else {//argc > 1
        cmd1 = argv[1]; 
        if (strcmp(cmd1, "list") == 0) {        //upg_part list
            if (argc > 2) {
                goto usage;
            }

            imgType = IMG_TYPE_BE;
            upgInterface = UPG_INTERFACE_USB;
            upg_part_type = UPG_PART_LIST;          
        } else if (strcmp(cmd1, "reload") == 0) {       //upg_part list
            if (argc > 2) {
                goto usage;
            }

            imgType = IMG_TYPE_BE;
            upgInterface = UPG_INTERFACE_USB;
            upg_part_type = UPG_PART_RELOAD;            
        } else if (strcmp(cmd1, "write") == 0) {        //upg_part write
            if (argc != 3) {
                goto usage;
            }
            //upg_part write id
            upg_part_id = (ulong)simple_strtoul(argv[2], NULL, 10);
            if (upg_part_id <= 0) {
                UPG_LOG(UPG_LOG_ERROR, "Invalid part id :%s\n", argv[2]);
                goto usage;
            }

            imgType = IMG_TYPE_BE;
            upgInterface = UPG_INTERFACE_USB;
            upg_part_type = UPG_PART_WRITE;
        } else {
            goto usage;
        }

    }

    if (upg_part_type == UPG_PART_LIST) {
        UPG_LOG(UPG_LOG_INFO, "[UPG]List partition info table\n");
        u4Ret = listPITForBE(upgInterface);
        if (UPGR_OK == u4Ret) {
            UPG_LOG(UPG_LOG_INFO, "[UPG]List partition info table successfully\n");
            return 0;
        } else {
            UPG_LOG(UPG_LOG_ERROR, "[UPG]List partition info table failed\n");
            return 1;
        }
    } else if (upg_part_type == UPG_PART_RELOAD) {
        UPG_LOG(UPG_LOG_INFO, "[UPG]Reload Backend image\n");
        u4Ret = reloadImage(upgInterface,IMG_TYPE_BE);
        if (UPGR_OK == u4Ret) {
            UPG_LOG(UPG_LOG_ERROR, "[UPG]Reload Backend image successfully\n");
            return 0;
        } else {
            UPG_LOG(UPG_LOG_ERROR, "[UPG]Reload Backend image failed\n");
            return 1;
        }
    } else if (upg_part_type = UPG_PART_WRITE) {
        UPG_LOG(UPG_LOG_INFO, "[UPG]Upgrade partially backend image partid=%d\n", upg_part_id);
        u4Ret = writePartBE(upgInterface, upg_part_id);
        if (UPGR_OK == u4Ret) {
            UPG_LOG(UPG_LOG_ERROR, "[UPG]Upgrade partially backend image successfully\n");
            return 0;
        } else {
            UPG_LOG(UPG_LOG_ERROR, "[UPG]Upgrade partially backend image failed\n");
            return 1;
        }
    } else {
        printf ("Usage:\n%s\n", cmdtp->usage);
        return -1;
    }

    

usage:  
    printf("Usage:\n%s\n", cmdtp->usage);   
    return 1;

}


U_BOOT_CMD(
    upg_part,   3,  1,  do_upg_part,
    "upg_part- Upgrade partially backend image from usb sub-system\n", //Simple help
    "         - (Default)List partition info table from backend image\n"
    "upg_part list     - List partition info table from backend image\n"
    "upg_part write id - Upgrade partially backend image by partition id from usb\n"
    "upg_part reload   - Reload backend image from usb\n"   
);

int do_upg_log(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]){
    CHAR    *cmd1;
    switch (argc) {
    case 1:
        printf ("Usage:\n%s\n", cmdtp->usage);
        break;
    case 2:
        cmd1 = argv[1];
        if (stricmp(cmd1, "0") == 0) {
            UPG_LOG_LEVEL = UPG_LOG_ERROR;
        } else if (stricmp(cmd1, "1") == 0) {
            UPG_LOG_LEVEL = UPG_LOG_INFO;
        } else if (stricmp(cmd1, "2") == 0) {
            UPG_LOG_LEVEL = UPG_LOG_DEBUG;
        } else {
            printf ("Usage:\n%s\n", cmdtp->usage);
            return 1;
        }
        break;
    default:
        break;
    }

    return 0;
}


U_BOOT_CMD(
    upg_log,    2,  1,  do_upg_log,
    "upg_log - Set upgrade log level\n", //Simple help
    "        - upg_log level  =>set log level(0:ERROR 1:INFO 2:DEBUG)\n"   
);

*/

#endif /* (CONFIG_CMD_UPG) */

