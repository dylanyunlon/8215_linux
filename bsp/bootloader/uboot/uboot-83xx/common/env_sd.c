/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

extern unsigned long long g_envpartitionoffset;

env_t *env_ptr = NULL;
extern uchar default_environment[];

char * env_name_spec = "MTK_SD";
extern void flush_invalid_cache(unsigned int start, unsigned int size);

uchar env_get_char_spec (int index)
{
	return ( *((uchar *)(gd->env_addr + index)) );
}

/* read env from sd */
void env_relocate_spec (void)
{
    unsigned int ret;
	#if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT0)
	int dev_num = 0;
	#else if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
	int dev_num = 2;
	#endif

	struct mmc *mmc = find_mmc_device(dev_num);
    mmc_init(mmc);

    //ret = mmc_bread(dev_num, CONFIG_ENV_SD_OFFSET, CONFIG_ENV_PAGE_NUM, env_ptr);
	flush_invalid_cache(env_ptr, CONFIG_ENV_PAGE_NUM);
    ret = mmc_bread(dev_num, (unsigned int)(g_envpartitionoffset/512), CONFIG_ENV_PAGE_NUM, env_ptr);
    if (CONFIG_ENV_PAGE_NUM == ret)
	{
    	ret = crc32(0, env_ptr->data, ENV_SIZE);
		if(ret != env_ptr->crc)
        	set_default_env();
    }
	else
	{
       set_default_env();
    }
}

int saveenv(void)
{
    unsigned int ret;
	#if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT0)
	int dev_num = 0;
	#else if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
	int dev_num = 2;
	#endif

    printf("Enter SD saveenv\r\n");

    //ret = mmc_bwrite(dev_num, CONFIG_ENV_SD_OFFSET, CONFIG_ENV_PAGE_NUM, env_ptr);
    ret = mmc_bwrite(dev_num, (unsigned int)(g_envpartitionoffset/512), CONFIG_ENV_PAGE_NUM, env_ptr);

	if (CONFIG_ENV_PAGE_NUM != ret){
        return -1;
    }

    return 0;
}

/************************************************************************
 * Initialize Environment use
 *
 * We are still running from ROM, so data use is limited
 * Use a (moderately small) buffer on the stack
 */
int env_init(void)
{
    /* use default */
    //printf("Enter sd env init\r\n");
    gd->env_addr = (ulong) & default_environment[0];
    gd->env_valid = 1;

    return 0;

}
