/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Misc boot support
 */
#include <common.h>
#include <command.h>
#include <net.h>

#include <partition.h>
#include <mmc.h>
#include <nand.h>

/* Allow ports to override the default behavior */
__attribute__((weak))
unsigned long do_go_exec (ulong (*entry)(int, char *[]), int argc, char *argv[])
{
	return entry (argc, argv);
}

int do_go (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr, rc;
	int     rcode = 0;

	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}

	addr = simple_strtoul(argv[1], NULL, 16);

	printf ("## Starting application at 0x%08lX ...\n", addr);

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */
	rc = do_go_exec ((void *)addr, argc - 1, argv + 1);
	if (rc != 0) rcode = 1;

	printf ("## Application terminated, rc = 0x%lX\n", rc);
	return rcode;
}

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
	go, CONFIG_SYS_MAXARGS, 1,	do_go,
	"start application at address 'addr'",
	"addr [arg ...]\n    - start application at address 'addr'\n"
	"      passing 'arg' as arguments"
);

extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

U_BOOT_CMD(
	reset, 1, 0,	do_reset,
	"Perform RESET of the CPU",
	""
);
void do_reboot_recovery(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    // Check recovery mode
    int ret;
    // Use page-aligned buffer (NAND page size is typically 2KB)
    #define NAND_PAGE_SIZE 4096
    static unsigned char page_aligned_buffer[NAND_PAGE_SIZE] __attribute__((aligned(NAND_PAGE_SIZE)));
    struct bootloader_message *bcb = (struct bootloader_message *)page_aligned_buffer;
    printf("Starting recovery mode setup...\n");

    // Clear page-aligned buffer
    memset(page_aligned_buffer, 0xFF, NAND_PAGE_SIZE);
    // Read BCB data from datazone partition using partition name
    printf("Reading BCB data from datazone partition...\n");
    char addr_str[16];
    sprintf(addr_str, "0x%lx", (unsigned long)page_aligned_buffer);
    char *argv_read[] = {"nand", "read", addr_str, "datazone", "0x1000"};
    ret = do_nand(NULL, 0, 6, argv_read);
    if(ret != 0){
        printf("Warning: Failed to read BCB info, using default BCB settings\n");
    } else {
        printf("Successfully read BCB info\n");
    }
    // Modify BCB command to recovery mode
    memcpy(bcb->command, "boot-recovery", 14);
    memcpy(bcb->recovery, "recovery", 9);
#ifdef CONFIG_SECURITY_UPGRADE
    // If security upgrade is enabled, calculate and set checksum
    uint32_t chksum = calc_bcb_checksum(bcb);
    printf("bcb_checksum %x\n", chksum);
    put_bcb_checksum(bcb, chksum);
#endif

    printf("Setting up recovery environment,bcb.command: 0x%s, bcb_rexovery\n", bcb->command, bcb->recovery);

    // Erase datazone partition
    printf("Erasing first 4KB of datazone partition...\n");
    char *argv_erase[] = {"nand", "erase", "datazone", "0x1000"};
    ret = do_nand(NULL, 0, 5, argv_erase);
    if (ret != 0) {
        printf("Error: Failed to erase datazone partition\n");
        return;
    }

    // Write modified BCB data to datazone partition
    printf("Writing modified BCB data...\n");
    char *argv_write[] = {"nand", "write", addr_str, "datazone", "0x1000"};
    ret = do_nand(NULL, 0, 6, argv_write);
    if (ret != 0) {
        printf("Error: Failed to write BCB data\n");
        return;
    }

    printf("BCB data written successfully, now rebooting to recovery mode...\n");
    // Reboot system
    extern void _reset(char mode, const char *cmd);
    _reset(0, NULL);
    // Safety loop
    while(1);
}

U_BOOT_CMD(
	rebootrecovery, 1, 0,	do_reboot_recovery,
	"Perform RESET of the CPU",
	""
);