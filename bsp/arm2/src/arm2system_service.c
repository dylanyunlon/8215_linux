/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */
#include "arm2system_service.h"
#include "dual_callback.h"
#include "dual_task.h"
#include "dual_hal.h"
#include "rtc_hw.h"
#include "x_bim.h"
#include "ac83xx_gpio_pinmux.h"
#include "ac83xx_pinmux_table.h"
#include "pinmux.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "libfdt.h"
#include "printf.h"
#include "partition.h"
#include "83xx.inc"
#include "armv6_mmu.h"

#define TAGS "ARM2SYSTEM"
#define ARM2SYSTEM_TEST
static volatile int heart_cnt = 0;
static char check_heart = 0;
extern int SetAbnormalFlag(int flag);
extern int SetRebootFlag(int flag);
extern int GetRebootFlag(void);
extern int GetCarplayStartFlag(void);
extern int SetCarplayStartFlag(int flag);
extern void SetArm1VsyncReady(int flag);
extern void SetEnableAwtkFlag(int enable);
static arm2system_service_manger_t sm = {0};
static SemaphoreHandle_t xSem = NULL;
UINT32 g_u4PrevTime = 0;
UINT32 g_frame = 0;
static char send_shutdown = 0;

void arm2system_service_gpioinit(int u4GPIOPinNum)
{
#ifdef CONFIG_ATC_PLATFORM_ac823x
	GPIO_Pull_UpDown(u4GPIOPinNum, PULLUP);
	ac823x_gpio_inout_sel_reg(u4GPIOPinNum, 0);
#else
	GPIO_MultiFun_Set(u4GPIOPinNum, PINMUX_LEVEL_GPIO_END_FLAG);
	// gpio_request(u4GPIOPinNum,"BackCar_Init_GPIO");
	gpio_direction_input(u4GPIOPinNum);
	// printk("GPIO %d Init Success!\r\n", u4GPIOPinNum);
#endif
}

int arm2system_service_getgpio_value(int u4GPIOPinNum)
{
	int u8Val = 0;
#ifdef CONFIG_ATC_PLATFORM_ac823x
	u8Val = ac823x_gpio_get_value_reg(u4GPIOPinNum);
#else
	u8Val = gpio_get_value(u4GPIOPinNum);
#endif

	return u8Val;
}

// 500ms: 100ms send from kernel will give 5 times.
unsigned int arm2system_service_checkheartbeat(void)
{
	UINT32 u4Curtime = 0;

	rtc_time_t tm;
	if (!check_heart)
		return 0;
	if (heart_cnt >= 2)
	{
		heart_cnt = 0;
	}
	else
	{
		check_heart = 0;
		SetArm1VsyncReady(0);
		SetCarplayStartFlag(0);
		SetAbnormalFlag(1);
		SetEnableAwtkFlag(1);
		pr_info("send signal to app\n");
	}

	return 0;
}
unsigned int arm2system_service_keep_heart(void)
{
	heart_cnt++;
}

unsigned int arm2system_service_statemach(void)
{
	return TASK_BUSY;
}

static void arm2system_service_wdt(void *parameters)
{
	(void)parameters;
	for (;;)
	{
		/* Example Task Code */
		vTaskDelay(3000); /* delay 100 ticks */
		arm2system_service_checkheartbeat();
	}
	vTaskDelete(NULL);
}

void arm2system_service_action(int status, int msg_type)
{
	if (0 <= status && status <= STATUS_MAX)
	{
		if (NULL != sm.handler_status[status])
			sm.handler_status[status](sm.args_status[status]);
	}

	if (0 <= msg_type && msg_type <= ARM2SYSTEM_SERVICE_MAX)
	{
		if (NULL != sm.handler_msg[msg_type])
			sm.handler_msg[msg_type](sm.args_msg[msg_type]);
	}
}

static void arm2system_service_key(void *parameters)
{
	(void)parameters;
	int pre_value = 0;
	int value = 0;
	value = arm2system_service_getgpio_value(131);
	if (value) {
		arm2system_service_action(STATUS_ANI_ON, -1);
	} else {
		SetEnableAwtkFlag(1);
	}
	pre_value = value;
	for (;;)
	{
		/* Example Task Code */

		value = arm2system_service_getgpio_value(131);
		vTaskDelay(50); /* delay 100 ticks */
		if (value != pre_value)
		{
			if (xSemaphoreTake(xSem, portMAX_DELAY) == pdFALSE)
			{
				pr_info("xSemaphoreTake failed\n");
				continue;
			}
			if (value) {
				SetEnableAwtkFlag(1);
				arm2system_service_action(STATUS_ANI_STOP, -1);
			}
			else {
				SetEnableAwtkFlag(0);
				arm2system_service_action(STATUS_ANI_ON, -1);
			}
			pre_value = value;
			pr_info("check key press:%d\n", value);
			xSemaphoreGive(xSem);
			// if (count > 10)
			//	arm2system_service_action(STATUS_ANI_END, -1);
		}
	}
	vTaskDelete(NULL);
}

int arm2system_service_ani_on(void *arg)
{
	pr_info("arm2system_service_ani_on\n");
	return 0;
}

int arm2system_service_ani_off(void *arg)
{
	pr_info("arm2system_service_ani_off\n");
	return 0;
}

int arm2system_service_ani_end(void *arg)
{
	// uart_send_ani_end();
	pr_info("arm2system_service_ani_end\n");
	return 0;
}

int arm2system_service_shutdown(void *arg)
{
	pr_info("arm2system_service_shutdown\n");
	if (xSemaphoreTake(xSem, portMAX_DELAY) == pdFALSE)
	{
		pr_info("ARM2SYSTEM_SERVICE_SHUTDOWN xSemaphoreTake failed\n");
	}
	send_shutdown = 1;
	HWSendMessage(MSG_COMBINE(MODULE_ARM2SYSTEMSERVICE, ARM2SYSTEM_SERVICE_SHUTDOWN), 0, 0, 0);
	return 0;
}

#if 1
unsigned int num_cpu()
{
	unsigned int cpu_num = 2;//default 2
	unsigned int cpu_mask = 0;
	unsigned int addr = 0xF0054664;
	cpu_mask = (unsigned int)(((*(volatile unsigned int*)addr) & 0x00000700) >> 8);

	if (cpu_mask == 0x0){
		cpu_num = 4;
	}else if (cpu_mask == 0x6){
		cpu_num = 2;
	}else{
		pr_info("BUG:CPU MASK error:0x%x\r\n",cpu_mask);
	}
	pr_info("cpu_mask:%d cpu_num:%d\r\n", cpu_mask, cpu_num);

	return cpu_num;
}

static int fdt_set_cpu_core_num(int cpu_core_num)
{
	int cpu_noffset;
	int ret;
	void *fdt = (void *)((ARM1PHY2ARM2UCV)FDT_LOAD_ADDR);

	pr_info("CPU CORE NUM: %d\r\n", cpu_core_num);
	if (cpu_core_num == 2){
		cpu_noffset = fdt_path_offset (fdt, "/cpus/cpu@2");
		ret = fdt_del_node(fdt, cpu_noffset);
		if (ret){
			pr_err("Remove dts /cpus/cpu@2 fail!\r\n");
		}
		cpu_noffset = fdt_path_offset (fdt, "/cpus/cpu@3");
		ret = fdt_del_node(fdt, cpu_noffset);
		if (ret){
			pr_err("Remove dts /cpus/cpu@3 fail!\r\n");
		}
	}

	return 0;
}
#endif

static void modify_loglevel(char *cmdline)
{
	char *valueops = NULL;
	char *endops = NULL;
	char *loglevel = '8';
#ifdef CONFIG_ATC_USER
	loglevel = '4';
#else
	loglevel = '8';
#endif
	if( NULL ==cmdline )
		return;
	char *loglevelops = strstr(cmdline, "loglevel=");
	if( loglevelops )
	{
		valueops = loglevelops + strlen("loglevel=");
		*valueops = loglevel;
	}
	return;
}
static void int_to_string(int num, char* str) {
	if (num == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}

	int i = 0;
	char temp[16];

	while (num > 0) {
		temp[i++] = (num % 10) + '0';
		num /= 10;
	}

	int j;
	for (j = 0; j < i; j++) {
		str[j] = temp[i - 1 - j];
	}
	str[j] = '\0';
}

static void ull_to_hex_string(unsigned long long num, char* str) {
	if (num == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}

	char hex_chars[] = "0123456789abcdef";
	int i = 0;
	char temp[20];

	while (num > 0) {
		temp[i++] = hex_chars[num % 16];
		num /= 16;
	}

	while (i < 8) {
		temp[i++] = '0';
	}

	int j;
	for (j = 0; j < i; j++) {
		str[j] = temp[i - 1 - j];
	}
	str[j] = '\0';
}

#define DATAZONE_DRAM_ADDR (CONFIG_DATAZONE_START)
partitionread *readpartitioninfofromflash()
{
	char *bufpartinfo;
	partitionhead *pparthead;
	partitionread *ppartread, *pprepartition, *pcurpartition;

#if ATC_BOOT_NAND
	ppartread = (partitionread *)((void *)((ARM1PHY2ARM2UCV)DATAZONE_DRAM_ADDR) + DATAZONE_PARTITION_OFFSET + 512);
#elif ATC_BOOT_EMMC
	ppartread = (partitionread *)((void *)((ARM1PHY2ARM2UCV)DATAZONE_DRAM_ADDR) + 512);
#else
	ppartread = (partitionread *)((void *)((ARM1PHY2ARM2UCV)DATAZONE_DRAM_ADDR) + DATAZONE_PARTITION_OFFSET + 512);
#endif

	pcurpartition = ppartread;
	pprepartition = pcurpartition;

	while (pcurpartition != NULL)
	{
		if (pcurpartition->u4LastPartition == 1)
		{
			pcurpartition->nextpartition = NULL;
			break;
		}
		else
		{
			pcurpartition = pcurpartition + 1;
			pprepartition->nextpartition = pcurpartition;
			pprepartition = pcurpartition;
		}
	}

	return ppartread;
}

void build_partition_info(char *ppartinfo, int bufsize)
{
	partitionread *ppartitionread, *p;
	char temp_str[128];
	int first_partition = 1;

	ppartitionread = readpartitioninfofromflash();
	p = ppartitionread;

	if (p == NULL) {
		Printf("ERR: Cannot find valid part Info\n");
		if (bufsize > 0) {
			ppartinfo[0] = '\0';
		}
		return;
	}

	ppartinfo[0] = '\0';

#if ATC_BOOT_NAND
	strncat(ppartinfo, " mtdparts=atcnand:", bufsize - strlen(ppartinfo) - 1);
#else
	strncat(ppartinfo, " parts=", bufsize - strlen(ppartinfo) - 1);
#endif

	while (p) {
		Printf("read partition info: %s addr:0x%x\n", p->szPartName, p->u8PartitionStartAddr);

		if (!first_partition) {
			strncat(ppartinfo, ",", bufsize - strlen(ppartinfo) - 1);
		}

		char size_str[16] = {0};
		if (p->u8PartitionSize >= 0x100000) {
			int_to_string((int)(p->u8PartitionSize / 0x100000), size_str);
			int len = strlen(size_str);
			if (len < 15) {
				size_str[len] = 'M';
				size_str[len + 1] = '\0';
			}
		} else if (p->u8PartitionSize >= 0x400) {
			int_to_string((int)(p->u8PartitionSize / 0x400), size_str);
			int len = strlen(size_str);
			if (len < 15) {
				size_str[len] = 'K';
				size_str[len + 1] = '\0';
			}
		} else {
			int_to_string((int)p->u8PartitionSize, size_str);
		}

		temp_str[0] = '\0';
		strncat(temp_str, size_str, sizeof(temp_str) - 1);
		strncat(temp_str, "@0x", sizeof(temp_str) - strlen(temp_str) - 1);

		char addr_str[16] = {0};
		ull_to_hex_string(p->u8PartitionStartAddr, addr_str);
		strncat(temp_str, addr_str, sizeof(temp_str) - strlen(temp_str) - 1);
		strncat(temp_str, "(", sizeof(temp_str) - strlen(temp_str) - 1);
		strncat(temp_str, p->szPartName, sizeof(temp_str) - strlen(temp_str) - 1);
		strncat(temp_str, ")", sizeof(temp_str) - strlen(temp_str) - 1);

#if ATC_BOOT_NAND
		// need not add mount flag.
#else
		if ((strncmp(p->szPartName, "system", 6) == 0) ||
			(strncmp(p->szPartName, "usrdata", 7) == 0) ||
			(strncmp(p->szPartName, "data", 4) == 0)) {
			strncat(temp_str, "1", sizeof(temp_str) - strlen(temp_str) - 1);
		} else {
			strncat(temp_str, "0", sizeof(temp_str) - strlen(temp_str) - 1);
		}
#endif
		strncat(ppartinfo, temp_str, bufsize - strlen(ppartinfo) - 1);

		first_partition = 0;
		p = p->nextpartition;
	}

	Printf("Built partition info:%s\n", ppartinfo);
}

#define METAZONE_USB0_PROTOCOL   0x10050
#define METAZONE_USB1_PROTOCOL   0x10051
#define METAZONE_USB_OTG_MODE   0x10052

static void set_dtb_args(void)
{
	int chosen_noffset;
	char *propvalue = NULL;
	int len, ret;
	int val;
	int args_len;
	int fdt_len;
	unsigned int cpu_num = 2;
	void *fdt = (void *)((ARM1PHY2ARM2UCV)FDT_LOAD_ADDR);
	fdt_len = fdt_totalsize(fdt);
	char args[1000] = {0};
	char bootargs[1024] = {0};
	char parts_info[800] = {0};
	char mtkd_str[16];
	int system_index = 0;

	system_index = fgSystemIndex();
	int_to_string(system_index, mtkd_str);
#if ATC_AB_PARTITION_SUPPORT
#if ATC_BOOT_NAND
	strncat(args, "root=/dev/mtkd", 14);
	strncat(args, mtkd_str, strlen(mtkd_str));
#else
	strncat(args, "root=/dev/mmcblk0p", 18);
	strncat(args, mtkd_str, strlen(mtkd_str));
#endif
	if (fgSlotSuffix())
	{
		strncat(args, " slot_suffix=_b", 15);
	}
	else
	{
		strncat(args, " slot_suffix=_a", 15);
	}
	Printf("set_dtb_args args_from_bootloader->ab_slot:%d\n", fgSlotSuffix());
#else
#if ATC_BOOT_NAND
	strncat(args, "root=/dev/mtkd", 14);
	strncat(args, mtkd_str, strlen(mtkd_str));
#else
	strncat(args, "root=/dev/mmcblk0p", 18);
	strncat(args, mtkd_str, strlen(mtkd_str));
#endif
#endif
	build_partition_info(parts_info, sizeof(parts_info));
	strncat(args, parts_info, strlen(parts_info));

	ret = _MetaZone_Read(METAZONE_USB0_PROTOCOL, &val);
	if (ret) {
		Printf("_MetaZone_Read METAZONE_USB0_PROTOCOL Failed\r\n");
	} else {
		Printf("_MetaZone_Read METAZONE_USB0_PROTOCOL %d successed\r\n", val);
		if (val == 0x554f3131)
			strncat(args, " usbo=full", 10);
		else
			strncat(args, " usbo=high", 10);
	}

	ret = _MetaZone_Read(METAZONE_USB1_PROTOCOL, &val);
	if (ret) {
		Printf("_MetaZone_Read METAZONE_USB1_PROTOCOL Failed\r\n");
	} else {
		Printf("_MetaZone_Read METAZONE_USB1_PROTOCOL %d successed\r\n", val);
		if (val == 0x55483131)
			strncat(args, " usbh=full", 10);
		else
			strncat(args, " usbh=high", 10);
	}

	ret = _MetaZone_Read(METAZONE_USB_OTG_MODE, &val);
	if (ret) {
		Printf("_MetaZone_Read METAZONE_USB_OTG_MODE Failed\r\n");
	} else {
		Printf("_MetaZone_Read METAZONE_USB_OTG_MODE %d successed\r\n", val);
		if (val == 0x55534248)
			strncat(args, " otg=host", 9);
		else
			strncat(args, " otg=dev", 8);
	}

	args_len = strlen(args);
	pr_info("set_dtb_args fdt_len:%d\n", fdt_len);
	fdt_len += 0x400;

	fdt_open_into(fdt, fdt, fdt_len);
	chosen_noffset = fdt_path_offset(fdt, "/chosen");
	propvalue = (char *)fdt_getprop(fdt, chosen_noffset, "bootargs", &len);
	pr_info("set_dtb_args :%s --- %d\n", propvalue, len);
	if (len + args_len >= 1024)
	{
		pr_err("bootargs len:%d too log ,please check!!!!\n", len + args_len);
		while(1);
	}

	strncat(bootargs, propvalue, len);
	strncat(bootargs, args, args_len);
	pr_info("set_dtb_args :%s\n", bootargs);
	modify_loglevel(bootargs);
	ret = fdt_setprop_string(fdt, chosen_noffset, "bootargs", (char *)bootargs);
	if (ret < 0)
	{
		pr_err("fdt_setprop_string error,please check!!!!\n");
		while (1);
	}

	cpu_num = num_cpu();
	fdt_set_cpu_core_num(cpu_num);

	Flush_Cache((unsigned int)fdt, fdt_totalsize(fdt));
	fgDualHALSetDtbStatus(STATUS_MODIFY_END);
}

static void arm2system_service_set_dtb(void *parameters)
{
	int ret;

	(void)parameters;
	pr_info("arm2system_service_set_dtb enter\n");
	if (0 == fgDualHALGetUpgradeMode())
	{
		for (;;)
		{
			/* Example Task Code */
			if (STATUS_LOAD_READY == fgDualHALGetDtbStatus())
			{
				set_dtb_args();
				break;
			}
			else
				vTaskDelay(20);
		}
		pr_info("arm2system_service_set_dtb destroy\n");
	}
	ret = mmu_unmap_region(NULL, ARM1PHY2ARM2UCV(FDT_LOAD_ADDR), 0x200000);
	if (ret) {
		pr_err("dtb region release failed:%d.\n", ret);
	}
	vTaskDelete(NULL);
}

unsigned int arm2system_service_init(void)
{
	xSem = xQueueCreateCountingSemaphore(1, 1);
	xTaskCreate(arm2system_service_set_dtb,
				"setdtb",
				2048,
				NULL,
				3U,
				NULL);
	xTaskCreate(arm2system_service_wdt,
				"watchdog",
				2048,
				NULL,
				3U,
				NULL);
#ifdef ARM2SYSTEM_TEST
	arm2system_service_gpioinit(131);
	xTaskCreate(arm2system_service_key,
				"onkey",
				2048,
				NULL,
				2U,
				NULL);
#endif
	return 0;
}

int arm2system_service_status_handler_register(arm2system_service_func handler, int status, void *arg)
{
	if (status < 0 || status >= STATUS_MAX)
		return -1;
	sm.handler_status[status] = handler;
	sm.args_status[status] = arg;
	return 0;
}

int arm2system_service_msg_handler_register(arm2system_service_func handler, int msg_type, void *arg)
{
	if (msg_type < 0 || msg_type >= ARM2SYSTEM_SERVICE_MAX)
		return -1;
	sm.handler_msg[msg_type] = handler;
	sm.args_msg[msg_type] = arg;
	return 0;
}

unsigned int arm2system_service_callback(unsigned int u4ModuleID, unsigned int u4Param1, unsigned int u4Param2, unsigned int u4Param3)
{
	UINT32 u4Curtime = 0;
	//    pr_info("get message  arm2system_service_Callback :%x\n ",u4ModuleID);
	switch (GETMESSAGEID(u4ModuleID))
	{
	case ARM2SYSTEM_SERVICE_HEARTBEAT:
		arm2system_service_keep_heart();
		break;
	case ARM2SYSTEM_SERVICE_HEARTBEAT_START:
		pr_info("heart start\n");
		check_heart = 1;
		heart_cnt = 3;
		SetAbnormalFlag(0);
		break;
	case ARM2SYSTEM_SERVICE_KERNEL_PANIC:
		SetArm1VsyncReady(0);
		SetCarplayStartFlag(0);
		SetEnableAwtkFlag(1);
		SetAbnormalFlag(1);
		pr_info("panic:send signal to app\n");
		pr_info("panic\n");
		break;
	case ARM2SYSTEM_SERVICE_REBOOT:
		SetCarplayStartFlag(0);
		SetRebootFlag(1);
		if (send_shutdown)
			// uart_send_shutdown_end();
			pr_info("REBOOT or\n");
		break;
	case ARM2SYSTEM_SERVICE_AWTK_START:
		// HWSendMessage(MSG_COMBINE(3, ARM2SYSTEM_SERVICE_DISPLAY_VSYNC), 0, 0, 0);
		if (2 == u4Param3) {
			SetCarplayStatusUpdateFlag(2);
			SetCarplayStartFlag(0);
			Printf("AWTK carplay STOP\n");
		} else {
			SetCarplayStatusUpdateFlag(1);
			SetCarplayStartFlag(1);
			Printf("AWTK carplay START\n");
		}
		break;
	case ARM2SYSTEM_SERVICE_DISPLAY_VSYNC:
		//Printf("AWTK ARM2SYSTEM_SERVICE_DISPLAY_VSYNC\n");
		// if (GetCarplayStartFlag()) {
		// u4Curtime = GetMinisecond();
		SetArm1VsyncReady(1);
		LTDC_IRQHandler();
		// vPmxHalMainIsr(VECTOR_VSYNC, 0);
		// vSclHalIsr(VECTOR_PANEL_SCALER, 0);
		g_frame++;
		//}
		/*if (g_frame == 100) {
			g_frame = 0;
			pr_info("%s frame times: %u\n", __func__, u4Curtime - g_u4PrevTime);
		}
		g_u4PrevTime = u4Curtime;*/
		break;
	default:
		pr_info("%s message error id:%u\n", __func__, u4ModuleID);
		break;
	}

	return (0);
}
