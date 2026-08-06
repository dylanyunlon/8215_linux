#include "boot.h"
#include "x_printf.h"
#include "x_hal_io.h"
#include "reserved_memory.h"

//CAUTION:ALL this file func,should be use only after ddr init.
static upgrade_arg_t up_arg;
int strncmp(const char *cs, const char *ct, int count)
{
    signed char __res = 0;

    while (count) {
        if ((__res = *cs - *ct++) != 0 || !*cs++)
            break;
        count--;
    }
    return __res;
}

void check_rsv(void)
{
	RSV_MEM_HEADER *t = (RSV_MEM_HEADER *)CONFIG_ARGS_START;
    if (!strncmp(t->magic, "RSV", 3)) {	
        if (t->rsv_mem_num >= MAX_RSV_MEM_NUM) {
            Printf("RSV Memory number is overflow,please check number is %d!\n", t->rsv_mem_num);
            while(1);
        }
	} 
	else
	{
		Printf("rsv loader error ,please check\n");
		while(1);
	}
	return;
}

void set_upgrade_mode(upgrade_mode_t mode)
{
	if (mode >= BOOT_MAX_UPGRADE)
		return;
	up_arg.upgrade_mode = mode;
}

int get_upgrade_mode(void)
{
	if (up_arg.upgrade_mode >= BOOT_MAX_UPGRADE)
		return BOOT_NO_UPGRADE;
	return up_arg.upgrade_mode;
}

unsigned int u4ARM2Start(unsigned int addr)
{
	printf("Init ARM2!\r\n");
#if 0
	UINT32 tmp;
	tmp = HAL_READ32(0xF0038088);
    tmp |= 0x1;
    HAL_WRITE32(0xF0038088,tmp);
	tmp = HAL_READ32(0xF0000058);
    tmp |= 0x2000000;
    HAL_WRITE32(0xF0000058,tmp);
#endif
	HAL_WRITE32(0xF004501C,0x00000001);
	//WriteReg32(0xF0045020,ARM2_RESERVED_MEM_PA);
	HAL_WRITE32(0xF0045020, addr);
	HAL_WRITE32(0xF00381B8,0x00000003);

	return 0;
}

