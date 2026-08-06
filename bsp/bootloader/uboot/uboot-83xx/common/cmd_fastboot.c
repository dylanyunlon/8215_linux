#include <common.h>
#include <command.h>

DECLARE_GLOBAL_DATA_PTR;

extern  void FastBoot(void);
extern int do_nand(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);
int write_nand_ex(uchar* buf,ulong length,char * partitionName,ulong offset,char* type,int end);
extern unsigned int u64_to_u32(unsigned long long  u64,unsigned int* uhigh,unsigned int* ulow);


int fb_parse_cmdline (char *line, char *argv[])
{
	int nargs = 0;

#ifdef DEBUG_PARSER
	printf ("parse_line: \"%s\"\n", line);
#endif
	while (nargs < CONFIG_SYS_MAXARGS) {

		/* skip any white space */
		while ((*line == ' ') || (*line == '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
#ifdef DEBUG_PARSER
		printf ("parse_line: nargs=%d\n", nargs);
#endif
			return (nargs);
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		while (*line && (*line != ' ') && (*line != '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
#ifdef DEBUG_PARSER
		printf ("parse_line: nargs=%d\n", nargs);
#endif
			return (nargs);
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf ("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

#ifdef DEBUG_PARSER
	printf ("parse_line: nargs=%d\n", nargs);
#endif
	return (nargs);
}
int erase_nand (char *partitionName)
{
	char *argv[4] = {"nand", "erase"};
	
	argv[2] = partitionName;
	return do_nand(NULL, 0, 3, argv);
}

int write_mmc(u32 write_buf_ptr, u64 write_address, u64 write_size)
{
       char buf1[16] = {0};
       char buf2[16] = {0};
	   char buf3[16] = {0};

	   #if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT0)
       char *argv[6] = {"mmc", "write", "0"};
	   #else if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT2)
	   char *argv[6] = {"mmc", "write", "2"};
	   #endif

       sprintf(buf1, "%x", write_buf_ptr);
       argv[3]= buf1;

	   sprintf(buf2, "%x", (unsigned int)(write_address / 512) ); 
       argv[4] = buf2;

	   // handle write size, in mmc block
       write_size = write_size - (write_size % 512) + ((write_size % 512) ? 512 : 0);
       sprintf(buf3, "%x", (unsigned int)(write_size/512));       
       argv[5] = buf3;

	   // do_mmcops, Parameter 'argv' : (2013-12-28)
		// 0 - mmc 
		// 1 - write
		// 2 - 0 --> mmc slot select
		// 3 - Write Buffer Pointer
		// 4 - Write Address (in block)
		// 5 - Write Buffer Size (in block)
       return do_mmcops(NULL, 0, 6, argv);    
}


int write_nand (u32 base_addr, char *partitionName, unsigned size)
{
	char buf1[10] = {0};
	char buf2[10] = {0};
	char *argv[6] = {"nand", "write"};

	sprintf(buf1, "%x", base_addr);
	argv[2] = buf1;
	argv[3] = partitionName;
	sprintf(buf2, "%x", size);
	argv[4] = buf2;

	// do_nand, Parameter 'argv' : (2013-11-08)
	// 0 - nand 
	// 1 - write
	// 2 - Write Buffer Pointer
	// 3 - Partition Name
	// 4 - Partition Offset (Optional)
	// 5 - Partition Size, Real Write Buffer size (Optional)
	return do_nand(NULL, 0, 4, argv);
}

int ubi_write_wrap (u32 base_addr, char *partitionName, unsigned size)
{
	char buf1[10] = {0};
	char buf2[10] = {0};
	char *argv[6] = {"ubi", "write"};
	cmd_tbl_t *cmdtp;
	int argc = 0;

	sprintf(buf1, "%x", base_addr);
	argv[2] = buf1;
	argv[3] = partitionName;
	sprintf(buf2, "%x", size);
	argv[4] = buf2;

	if ((cmdtp = find_cmd("ubi")) == NULL) {
		printf ("Unknown command '%s' - try 'help'\n", argv[0]);
		return -1;
	}
	if ((cmdtp->cmd) (cmdtp, 0, 5, argv) != 0) {
			return -1;
	}
	return 0;
}

int fb_write_internal_storage(char *partition_name, char *partition_type, u64 part_offset, u32 data_buf_ptr, unsigned size, int last_write)
{
	int retval = 0;

#ifndef CONFIG_BOOT_MMC
	// For Nand, new interface for support offset according to partition start address
	retval = write_nand_ex(data_buf_ptr, size, partition_name, part_offset, partition_type, last_write);
#else
	// For eMMC, in this case, 'part_offset' = partition_offset + write_offset (according to partition offset)
	//printf("---> fb_write_internal_storage: part_offset = 0x%08X, size = ox%08X <---\r\n", part_offset, size);
	retval = write_mmc(data_buf_ptr, part_offset, size);
#endif

	return retval;
}

#ifdef CONFIG_BOOT_MMC
int fb_write_emmc_storage(u64 addr, u32 data_buf_ptr, u32 size)
{
	int retval = 0;
	//unsigned int vallow,valhigh;

	//u64_to_u32(addr, &valhigh, &vallow);
	//printf("fb_write_emmc_storage: addr=0x%X%08X, size=0x%x\r\n", addr, size);
	retval = write_mmc(data_buf_ptr, addr, size);

	return retval;
}
#endif


#ifndef CONFIG_BOOT_MMC
int fb_format_usrdata_partition (char *partitionName, unsigned int partition_start_addr)
{
	char *argv[3] = {"nand", "format"};

	argv[2] = partitionName;
	return do_nand(NULL, 0, 3, argv);
}
#else
extern char *uitostr_hex(char *str,unsigned int u4);
int fb_format_usrdata_partition(char *partitionName, unsigned long long partition_start_addr)
{
	char buf1[16] = {0};
	char buf2[16] = {0};
	char buf3[16] = {0};
	char szVal[9] = {0};
	char *strval;
	 unsigned int vallow,valhigh;
	#if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT0)
	char *argv[6] = {"mmc", "format", "0"};
	#else if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT0)
	char *argv[6] = {"mmc", "format", "2"};
	#endif
	
	argv[3]= partitionName;

   
	u64_to_u32(partition_start_addr,&valhigh,&vallow);
	if (valhigh != 0)
	{
	   strval = uitostr_hex(szVal,valhigh);
	   strcat(buf2,strval);
	}
	strval = uitostr_hex(szVal,vallow);
	if (vallow==0 && valhigh !=0)
		strcat(buf2,"00000000");
	else
	    strcat(buf2,strval);
	//sprintf(buf2, "%x", partition_start_addr); //partition_start_addr
	argv[4] = buf2;

	sprintf(buf3, "%x", 0);       
	argv[5] = buf3;

	// do_mmcops, Parameter 'argv' : (2013-12-28)
	// 0 - mmc 
	// 1 - format
	// 2 - 0 --> mmc slot select
	// 3 - Partition Name
	// 4 - Partition Start Address (in byte)
	// 5 - Partition Size (in byte)
	return do_mmcops(NULL, 0, 6, argv); 
}

int fb_format_partition(char *partitionName, unsigned long long partition_start_addr,unsigned long long partition_size)
{
	char buf1[16] = {0};
	char buf2[16] = {0};
	char buf3[16] = {0};
	char szVal[20] = {0};
	char *strval;
	 unsigned int vallow,valhigh;
	#if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT0)
	char *argv[6] = {"mmc", "format", "0"};
	#else if (CONFIG_BOOT_SD_SLOT == MSDC_SLOT0)
	char *argv[6] = {"mmc", "format", "2"};
	#endif
	
	argv[3]= partitionName;

    printf("---> fb_format_partition partition_size: 0x%08X, 0x%08X <---\n", (u32)(partition_size >> 32), (u32)(partition_size & 0xFFFFFFFF));

    u64_to_u32(partition_start_addr,&valhigh,&vallow);
	if (valhigh != 0)
	{
	   strval = uitostr_hex(szVal,valhigh);
	   strcat(buf2,strval);
	}
	strval = uitostr_hex(szVal,vallow);
	if (vallow==0 && valhigh !=0)
		strcat(buf2,"00000000");
	else
	    strcat(buf2,strval);


	//sprintf(buf2, "%x", partition_start_addr); //partition_start_addr
	argv[4] = buf2;

    valhigh = 0;
	vallow = 0;
    u64_to_u32(partition_size,&valhigh,&vallow);
	if (valhigh != 0)
	{
	   strval = uitostr_hex(szVal,valhigh);
	   strcat(buf3,strval);
	}
	strval = uitostr_hex(szVal,vallow);
	if (vallow==0 && valhigh !=0)
		strcat(buf3,"00000000");
	else
	    strcat(buf3,strval);

	//sprintf(buf3, "%x", partition_size);       
	argv[5] = buf3;

	// do_mmcops, Parameter 'argv' : (2013-12-28)
	// 0 - mmc 
	// 1 - format
	// 2 - 0 --> mmc slot select
	// 3 - Partition Name
	// 4 - Partition Start Address (in byte)
	// 5 - Partition Size (in byte)
	return do_mmcops(NULL, 0, 6, argv); 
}

#endif

static void fb_process_macros (const char *input, char *output)
{
	char c, prev;
	const char *varname_start = NULL;
	int inputcnt = strlen (input);
	int outputcnt = CONFIG_SYS_CBSIZE;
	int state = 0;		/* 0 = waiting for '$'  */

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
#ifdef DEBUG_PARSER
	char *output_start = output;

	printf ("[PROCESS_MACROS] INPUT len %d: \"%s\"\n", strlen (input),
		input);
#endif

	prev = '\0';		/* previous character   */

	while (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		if (state != 3) {
			/* remove one level of escape characters */
			if ((c == '\\') && (prev != '\\')) {
				if (inputcnt-- == 0)
					break;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		case 0:	/* Waiting for (unescaped) $    */
			if ((c == '\'') && (prev != '\\')) {
				state = 3;
				break;
			}
			if ((c == '$') && (prev != '\\')) {
				state++;
			} else {
				*(output++) = c;
				outputcnt--;
			}
			break;
		case 1:	/* Waiting for (        */
			if (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} else {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				if (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			break;
		case 2:	/* Waiting for )        */
			if (c == ')' || c == '}') {
				int i;
				char envname[CONFIG_SYS_CBSIZE], *envval;
				int envcnt = input - varname_start - 1;	/* Varname # of chars */

				/* Get the varname */
				for (i = 0; i < envcnt; i++) {
					envname[i] = varname_start[i];
				}
				envname[i] = 0;

				/* Get its value */
				envval = getenv (envname);

				/* Copy into the line if it exists */
				if (envval != NULL)
					while ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			break;
		case 3:	/* Waiting for '        */
			if ((c == '\'') && (prev != '\\')) {
				state = 0;
			} else {
				*(output++) = c;
				outputcnt--;
			}
			break;
		}
		prev = c;
	}

	if (outputcnt)
		*output = 0;
	else
		*(output - 1) = 0;

#ifdef DEBUG_PARSER
	printf ("[PROCESS_MACROS] OUTPUT len %d: \"%s\"\n",
		strlen (output_start), output_start);
#endif
}


int oem_parse(char *cmdline)
{
     char *argv[CONFIG_SYS_MAXARGS + 1];	
	 int argc = 0;
	 int i = 0;
	 cmd_tbl_t *cmdtp;
	 char finaltoken[CONFIG_SYS_CBSIZE];

	printf("%s\n", cmdline);
	
	fb_process_macros(cmdline, finaltoken);

	printf("finaltoken = %s \r\n", finaltoken);

	argc = fb_parse_cmdline(finaltoken, argv);

	printf("argc = %d, after parse: \r\n",argc);
	for (i = 0; i < argc; i++)
	{
		printf("argv[%d] = %s\r\n", i, argv[i]);
	}
	
    if ((cmdtp = find_cmd(argv[0])) == NULL) 
	{
		printf ("Unknown command '%s' - try 'help'\n", argv[0]);
		return -1;
	}
	
    /* found - check max args */
	if (argc > cmdtp->maxargs) 
	{
		cmd_usage(cmdtp);
		return  -1 ;		
	}
	
	if ((cmdtp->cmd) (cmdtp, 0, argc, argv) != 0) 
	{
		return -1;
	}
		
	return 0;
}

int do_fastboot ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    printf("start fastboot\r\n");
	FastBoot();
	return 0;
}

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
	fastboot,	1,	1,	do_fastboot,
	"upgrade images",
	""
);

