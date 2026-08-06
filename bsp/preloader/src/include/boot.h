#ifndef _PRELOADER_BOOT_h
#define _PRELOADER_BOOT_h
#define KERNEL_LOAD_ADDR 0x1008000
#define FDT_LOAD_ADDR    0x2000000
#define MACH_TYPE_AC83XX               5000

typedef enum upgrade_mode
{
	BOOT_NO_UPGRADE,
	BOOT_SD_UPGRADE,
	BOOT_RECOVERY_UPGRADE,
	BOOT_MAX_UPGRADE,
}upgrade_mode_t;

typedef enum dtb_
{
	STATUS_WAIT_LOAD,
	STATUS_LOAD_READY,
	STATUS_MODIFY_END,
}dtb_status_t;

typedef enum args
{
	ARGS_MODE,
	ARGS_LOGO,
	ARGS_ARM2,
	ARGS_DTB,
	ARGS_BOOT_MISC,
	ARGS_MAX,
}args_t;

typedef struct upgrade_arg
{
	unsigned int upgrade_mode;
	unsigned int logo_size;
	unsigned int arm2_size;
	unsigned int dtb_size;
	unsigned int version;
	unsigned int boot_misc_size;
	unsigned int metazone_size;
}upgrade_arg_t;

typedef struct args_to_arm2_t
{
	unsigned int jump_instr;
	unsigned char unused[0x400 - 4];
	unsigned long dram_size;
	unsigned int upgrade_mode;
	unsigned int dtb_status;
	unsigned int system_index;
#if ATC_AB_PARTITION_SUPPORT
	unsigned int ab_slot;
	unsigned int cache_rsv[3];
#else
	unsigned int cache_rsv[4];
#endif
	unsigned int arm_dtb_status;
}ARGS_TO_ARM2_P;	/*args to arm2 from uboot/lk*/

#define NORMAL_BOOT   0x33633363
#define QUICK_BOOT    0x6D617273
#define EMMC_BOOT     0x636D6D65
void set_upgrade_mode(upgrade_mode_t mode);
int get_upgrade_mode(void);
unsigned int boot_time_ms(void);
unsigned int u4ARM2Start(unsigned int addr);
void check_rsv(void);
int strncmp(const char *cs, const char *ct, int count);

extern unsigned  get_boot_type();

extern unsigned set_boot_type();

extern unsigned set_opwrsb_mode(unsigned type);

extern unsigned set_opwrsb_function(unsigned fun, unsigned value);


#define GPIO_POLARITY_LOW  0
#define GPIO_POLARITY_HIGH 1



#define GPIO_WAKEUP_STS    0
#define GPIO_WAKEUP_SRC    1
#define GPIO_PAD_IR        2


struct quickboot_param {
 
       unsigned int version;

       /*ddr calibration address */
       unsigned int ddr_cal_addr;
       /*os resume entry(physical address) */
       unsigned int nw_resume_entry;
#ifdef __AndroidM__
	   /* secure os resume entry(physical address) */
	   unsigned int sw_resume_entry;
 #endif
       /* wakeup source gpio & polarity config*/ 
       unsigned int wakeup_src_gpio;
       unsigned int wakeup_src_polarity;
       /*wakeup state gpio & polarity config*/
       unsigned int wakeup_sts_gpio;
       unsigned int wakeup_sts_polarity;
       
       /* delay time for system broad power off*/ 
       unsigned int power_off_delay;
       
       /* delay time for cpu reset*/
       unsigned int power_on_delay;

};


#endif
