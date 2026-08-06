/*! \file
    \brief  Declaration of library functions

    Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/




/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#ifdef DFT_TAG
#undef DFT_TAG
#endif
#define DFT_TAG "[CNN][WMT_CMB_HW-6630]"


/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "wmt_plat.h"
#include "wmt_lib.h"
#include "wmt_dev.h"
#include "mtk_wcn_cmb_hw.h"
#include "osal_typedef.h"
#include <linux/pinctrl/consumer.h>

#ifdef CONFIG_ARCH_AC83XX
#include <mach/ac83xx_gpio_pinmux.h>
#include <mach/ac83xx_gpio_pinmux_mapping.h>
#endif

#ifdef CONFIG_ATC_OS_VERSION_JB2
#include <linux/gpio.h>
#include <mach/ac83xx_pinmux_table.h>
#include <mach/pinmux.h>
#else
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#endif

#include <linux/platform_device.h>
#include <linux/printk.h>

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define DFT_RTC_STABLE_TIME 100
#define DFT_LDO_STABLE_TIME 100
#define DFT_RST_STABLE_TIME 30
#define DFT_OFF_STABLE_TIME 10
#define DFT_ON_STABLE_TIME 30

#ifdef CONFIG_ATC_OS_VERSION_JB2
#define WIFI_POWER_PIN 43
#define WIFI_RESET_PIN 44
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))
#define __vfs_read vfs_read
#endif

#ifdef CONFIG_ARCH_AC83XX
extern int AC_BoardType_Get(void);
#endif
#ifdef CONFIG_SDIO_CLK_SWITCH
static UINT32 wifi_ac_enable = 0;
#endif

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/



/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

PWR_SEQ_TIME gPwrSeqTime;




/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/



/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#if defined(CONFIG_ARCH_AC83XX) || defined(CONFIG_ARCH_AC823X)

#ifdef CONFIG_ATC_OS_VERSION_JB2
	// disable this macro after sdio update msdc_detect_change()
	#define MSDC_DETECT_CHANGE_LEGACY   1
#endif

#ifdef MSDC_DETECT_CHANGE_LEGACY    1
	#ifdef CONFIG_SDIO_CLK_SWITCH
	extern void msdc_detect_change(u32 slot, u32 enable, u32 type, u32 clksrc);
	#else
	extern void msdc_detect_change(u32 slot, u32 enable, u32 type);
	#endif
#else
	#ifdef CONFIG_SDIO_CLK_SWITCH
	extern int msdc_detect_change(u32 slot, u32 enable, u32 type, u32 clksrc);
	#else
	extern int msdc_detect_change(u32 slot, u32 enable, u32 type);
	#endif
#endif

#ifdef CONFIG_ARCH_AC823X
	#define STP_SDIO_SLOT   3
#else
	#define STP_SDIO_SLOT   1
#endif

#elif defined(CONFIG_ARCH_AC8X)
	#if 0
	/* NOTE:
	 * Runtime ko will call __weak function instead of the real
	 * function at drivers/mmc/host/autochips/ac8x/sdhci-of-atc.c.
	 * Builtin code will call the real function instead of __weak
	 * function.
	 */
	int __weak sdhci_cadence_detect_change(u32 slot, u32 enable)
	{
		WMT_ERR_FUNC("%s not implemented\n", __func__);

		return -1;
	}
	#else
	extern int sdhci_cadence_detect_change(u32 slot, u32 enable);
	#endif

	#define STP_SDIO_SLOT   1

#endif

static INT32 _mtk_wcn_cmb_hw_detect_change(UINT32 enable)
{
	INT32 iRet = 0;

	WMT_INFO_FUNC("[6630-sdio] detect_change slot %u en %u start\n",
			STP_SDIO_SLOT, enable);

#if defined(CONFIG_ARCH_AC83XX) || defined(CONFIG_ARCH_AC823X)

#ifdef MSDC_DETECT_CHANGE_LEGACY
	#ifdef CONFIG_SDIO_CLK_SWITCH
	msdc_detect_change(STP_SDIO_SLOT, enable, 0, wifi_ac_enable);
	#else
	msdc_detect_change(STP_SDIO_SLOT, enable, 0);
	#endif
#else
	#ifdef CONFIG_SDIO_CLK_SWITCH
	iRet = msdc_detect_change(STP_SDIO_SLOT, enable, 0, wifi_ac_enable);
	#else
	iRet = msdc_detect_change(STP_SDIO_SLOT, enable, 0);
	#endif
#endif

#elif defined(CONFIG_ARCH_AC8X)

	iRet = sdhci_cadence_detect_change(STP_SDIO_SLOT, enable);

#endif

	WMT_INFO_FUNC("[6630-sdio] detect_change slot %u en %u end, ret %d\n",
			STP_SDIO_SLOT, enable, iRet);

	return iRet;
}

extern UINT32 coredump_packets_counter;

INT32 mtk_wcn_cmb_hw_pwr_off(VOID)
{
	INT32 iRet = 0;
#ifdef CONFIG_ARCH_AC83XX
	unsigned int boardtype;
#endif

	WMT_INFO_FUNC("CMB-HW, hw_pwr_off start\n");

#ifdef CONFIG_ATC_OS_VERSION_JB2
	/*1. disable irq --> should be done when do wmt-ic swDeinit period */
	/* TODO:[FixMe][GeorgeKuo] clarify this */

	/*2. set bgf eint/all eint to deinit state, namely input low state */
	if (!((0x6630 == mtk_wcn_wmt_chipid_query())
				&& (STP_SDIO_IF_TX == wmt_plat_get_comm_if_type()))) {
		iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_EINT_DIS);
		WMT_INFO_FUNC("CMB-HW, BGF_EINT IRQ unregistered and disabled\n");
		iRet += wmt_plat_gpio_ctrl(PIN_BGF_EINT, PIN_STA_DEINIT);
	}
	/* 2.1 set ALL_EINT pin to correct state even it is not used currently */
	iRet += wmt_plat_eirq_ctrl(PIN_ALL_EINT, PIN_STA_DEINIT);
	WMT_INFO_FUNC("CMB-HW, ALL_EINT IRQ unregistered and disabled\n");
	iRet += wmt_plat_gpio_ctrl(PIN_ALL_EINT, PIN_STA_DEINIT);
	/* 2.2 deinit gps sync */
	iRet += wmt_plat_gpio_ctrl(PIN_GPS_SYNC, PIN_STA_DEINIT);

	/*3. set audio interface to CMB_STUB_AIF_0, BT PCM OFF, I2S OFF */
	iRet += wmt_plat_audio_ctrl(CMB_STUB_AIF_0, CMB_STUB_AIF_CTRL_DIS);

	/*4. set control gpio into deinit state, namely input low state */
	iRet += wmt_plat_gpio_ctrl(PIN_SDIO_GRP, PIN_STA_DEINIT);
	iRet += wmt_plat_gpio_ctrl(PIN_RST, PIN_STA_OUT_L);
	iRet += wmt_plat_gpio_ctrl(PIN_PMU, PIN_STA_OUT_L);

	/*5. set uart tx/rx into deinit state, namely input low state */
	iRet += wmt_plat_gpio_ctrl(PIN_UART_GRP, PIN_STA_DEINIT);

	/* 6. Last, LDO output low */
	iRet += wmt_plat_gpio_ctrl(PIN_LDO, PIN_STA_OUT_L);

	/*7. deinit gps_lna */
	iRet += wmt_plat_gpio_ctrl(PIN_GPS_LNA, PIN_STA_DEINIT);
#endif

#ifdef CONFIG_ARCH_AC83XX
	boardtype = AC_BoardType_Get();

	if (boardtype == 0) {
#endif

#ifdef CONFIG_ATC_OS_VERSION_JB2
		gpio_set_value(WIFI_POWER_PIN,0);
		gpio_free(WIFI_POWER_PIN);
		gpio_set_value(WIFI_RESET_PIN,0);
		gpio_free(WIFI_RESET_PIN);
#elif defined(CONFIG_ARCH_AC83XX) || defined(CONFIG_ARCH_AC823X)
		gpiod_set_value(mt6630_gpio[PWN_PIN].desc, 0);
		gpiod_put(mt6630_gpio[PWN_PIN].desc);
		gpiod_set_value(mt6630_gpio[RST_PIN].desc, 0);
		gpiod_put(mt6630_gpio[RST_PIN].desc);

#elif defined(CONFIG_ARCH_AC8X)
		if (gpio_is_valid(mt6630_gpio[PWN_PIN].num)
				&& gpio_is_valid(mt6630_gpio[RST_PIN].num)) {

			gpio_set_value(mt6630_gpio[PWN_PIN].num, 0);
			gpio_set_value(mt6630_gpio[RST_PIN].num, 0);
			WMT_INFO_FUNC("[6630-gpio] pull down PWN_PIN: %d RST_PIN: %d\n",
					mt6630_gpio[PWN_PIN].num, mt6630_gpio[RST_PIN].num);
		} else {
			WMT_ERR_FUNC("[6630-gpio] invalid PWN_PIN: %d RST_PIN: %d\n",
					mt6630_gpio[PWN_PIN].num, mt6630_gpio[RST_PIN].num);
		}
#endif
		/* msleep(10); */
		msleep(100);

		iRet += _mtk_wcn_cmb_hw_detect_change(0);
		/* msleep(20000); */
		msleep(100);

#ifdef CONFIG_ARCH_AC83XX
	} else {
		WMT_INFO_FUNC("this is EVB board\n");
	}
#endif

	coredump_packets_counter = 0;

	WMT_INFO_FUNC("CMB-HW, hw_pwr_off finish\n");

	return iRet;
}

/*
 * Trigger whole chip reset if sdio init fail for more than 3 times.
 *
 * Return non-zero on trigger reset failed.
 */
static INT32 _mtk_wcn_cmb_hw_err_handler(INT32 err)
{
	// trigger reset
	#define HW_ERR_TRG_THRESHOLD      3
	// turnaround the counter for the next trigger
	#define HW_ERR_TA_THRESHOLD       6

	static UINT32 s_err_cnt = 0;
	static INT32 s_reset_flag = 0;
	INT32 ret = -1;

	if (err == 0) {
		s_err_cnt = 0;
		s_reset_flag = 0;
		return 0;
	}

	s_err_cnt++;

	if (s_err_cnt < HW_ERR_TRG_THRESHOLD) {
		return 0;
	} else if (s_err_cnt == HW_ERR_TRG_THRESHOLD) {
		// trigger reset once, fail retry at wmt_lib_cmb_rst
		if (0 == s_reset_flag) {
			WMT_ERR_FUNC("HW_ERR(%d) continually, "
					"trigger whole chip reset\n", err);
			dump_stack();
			s_reset_flag = 1;
			ret = mtk_wcn_stp_trg_reset();
			WMT_ERR_FUNC("trigger reset %s, ret(%d)\n",
					ret ? "FAIL" : "OK", ret);
			return ret;
		} else {
			WMT_ERR_FUNC("still HW_ERR(%d) after "
					"whole chip reset, please FIXME\n", err);
			return -EFAULT;
		}
	} else if (s_err_cnt < HW_ERR_TA_THRESHOLD) {
		return 0;
	} else {
		s_err_cnt = 0;
		return 0;
	}

	return 0;
}

INT32 mtk_wcn_cmb_hw_pwr_on(VOID)
{
	static UINT32 _pwr_first_time = 1;
	INT32 iRet = 0;
#ifdef CONFIG_ARCH_AC83XX
	unsigned int boardtype;
#endif

	WMT_DBG_FUNC("CMB-HW, hw_pwr_on start\n");

#ifdef CONFIG_ATC_OS_VERSION_JB2
	/* disable interrupt firstly */
	if (!((0x6630 == mtk_wcn_wmt_chipid_query())
	      && (STP_SDIO_IF_TX == wmt_plat_get_comm_if_type())))
		iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_EINT_DIS);
	iRet += wmt_plat_eirq_ctrl(PIN_ALL_EINT, PIN_STA_EINT_DIS);

	/*set all control and eint gpio to init state, namely input low mode */
	iRet += wmt_plat_gpio_ctrl(PIN_LDO, PIN_STA_INIT);
	iRet += wmt_plat_gpio_ctrl(PIN_PMU, PIN_STA_INIT);
	iRet += wmt_plat_gpio_ctrl(PIN_RST, PIN_STA_INIT);
	iRet += wmt_plat_gpio_ctrl(PIN_SDIO_GRP, PIN_STA_INIT);
	if (!((0x6630 == mtk_wcn_wmt_chipid_query())
	      && (STP_SDIO_IF_TX == wmt_plat_get_comm_if_type())))
		iRet += wmt_plat_gpio_ctrl(PIN_BGF_EINT, PIN_STA_INIT);
	iRet += wmt_plat_gpio_ctrl(PIN_ALL_EINT, PIN_STA_INIT);
	iRet += wmt_plat_gpio_ctrl(PIN_GPS_SYNC, PIN_STA_INIT);
	iRet += wmt_plat_gpio_ctrl(PIN_GPS_LNA, PIN_STA_INIT);
	/* wmt_plat_gpio_ctrl(PIN_WIFI_EINT, PIN_STA_INIT); *//* WIFI_EINT is controlled by SDIO host driver */
	/* TODO: [FixMe][George]:WIFI_EINT is used in common SDIO */

	/*1. pull high LDO to supply power to chip */
	iRet += wmt_plat_gpio_ctrl(PIN_LDO, PIN_STA_OUT_H);
	osal_sleep_ms(gPwrSeqTime.ldoStableTime);

	/* 2. export RTC clock to chip */
	if (_pwr_first_time) {
		/* rtc clock should be output all the time, so no need to enable output again */
		iRet += wmt_plat_gpio_ctrl(PIN_RTC, PIN_STA_INIT);
		osal_sleep_ms(gPwrSeqTime.rtcStableTime);
		WMT_INFO_FUNC("CMB-HW, rtc clock exported\n");
	}

	/*3. set UART Tx/Rx to UART mode */
	iRet += wmt_plat_gpio_ctrl(PIN_UART_GRP, PIN_STA_INIT);

	if (0x6630 == mtk_wcn_wmt_chipid_query()) {
		switch (wmt_plat_get_comm_if_type()) {
			WMT_INFO_FUNC("wmt_plat_get_comm_if_type() is %d\n",
				      wmt_plat_get_comm_if_type());
		case STP_UART_IF_TX:
			iRet += wmt_plat_gpio_ctrl(PIN_UART_RX, PIN_STA_OUT_H);
			break;
		case STP_SDIO_IF_TX:
				iRet += wmt_plat_gpio_ctrl(PIN_UART_RX, PIN_STA_IN_L);
			break;
		default:
			WMT_ERR_FUNC("not supported common interface\n");
			break;
		}
	}
	/*4. PMU->output low, RST->output low, sleep off stable time */
	iRet += wmt_plat_gpio_ctrl(PIN_PMU, PIN_STA_OUT_L);
	iRet += wmt_plat_gpio_ctrl(PIN_RST, PIN_STA_OUT_L);
	osal_sleep_ms(gPwrSeqTime.offStableTime);

	/*5. PMU->output high, sleep rst stable time */
	iRet += wmt_plat_gpio_ctrl(PIN_PMU, PIN_STA_OUT_H);
	osal_sleep_ms(gPwrSeqTime.rstStableTime);

	/*6. RST->output high, sleep on stable time */
	iRet += wmt_plat_gpio_ctrl(PIN_RST, PIN_STA_OUT_H);
	osal_sleep_ms(gPwrSeqTime.onStableTime);

	/*set UART Tx/Rx to UART mode */
	if (0x6630 == mtk_wcn_wmt_chipid_query())
			iRet += wmt_plat_gpio_ctrl(PIN_UART_RX, PIN_STA_IN_H);


	/*7. set audio interface to CMB_STUB_AIF_1, BT PCM ON, I2S OFF */
	/* BT PCM bus default mode. Real control is done by audio */
	iRet += wmt_plat_audio_ctrl(CMB_STUB_AIF_1, CMB_STUB_AIF_CTRL_DIS);

	/*8. set EINT< -ommited-> move this to WMT-IC module,
	   where common sdio interface will be identified and do proper operation */
	/* TODO: [FixMe][GeorgeKuo] double check if BGF_INT is implemented ok */
	if (!((0x6630 == mtk_wcn_wmt_chipid_query())
	      && (STP_SDIO_IF_TX == wmt_plat_get_comm_if_type()))) {
		iRet += wmt_plat_gpio_ctrl(PIN_BGF_EINT, PIN_STA_MUX);
		iRet += wmt_plat_eirq_ctrl(PIN_BGF_EINT, PIN_STA_INIT);
		WMT_INFO_FUNC("CMB-HW, BGF_EINT IRQ registered and disabled\n");
	} else {
		WMT_DBG_FUNC("CMB-HW, no need to register BGF_EINT for MT6630 SDIO mode\n");
	}

	/* 8.1 set ALL_EINT pin to correct state even it is not used currently */
	iRet += wmt_plat_gpio_ctrl(PIN_ALL_EINT, PIN_STA_MUX);
	iRet += wmt_plat_eirq_ctrl(PIN_ALL_EINT, PIN_STA_INIT);
	WMT_INFO_FUNC("CMB-HW, hw_pwr_on finish (%d)\n", iRet);
#endif
	_pwr_first_time = 0;

#ifdef CONFIG_SDIO_CLK_SWITCH
	struct file *fileAddr = NULL;

	fileAddr = filp_open("/data/misc/wifi/wificlock", O_RDONLY, 0);
	if (IS_ERR(fileAddr)) {
		WMT_INFO_FUNC("/data/misc/wifi/wificlock is not exist\n");
	} else {
		WMT_INFO_FUNC("/data/misc/wifi/wificlock is exist\n");
		fileAddr->f_pos = 0;
		__vfs_read(fileAddr, (__force void __user *)&wifi_ac_enable,
				sizeof(UINT32), &fileAddr->f_pos);
		WMT_INFO_FUNC("wifi_ac_enable is %d\n",wifi_ac_enable);
	}
	if ((fileAddr != NULL) && !IS_ERR(fileAddr)) {
		filp_close(fileAddr, NULL);
		fileAddr = NULL;
	}
#endif

#ifdef CONFIG_ARCH_AC83XX
	boardtype = AC_BoardType_Get();

	if (boardtype == 0) {
#endif

#ifdef CONFIG_ATC_OS_VERSION_JB2
		GPIO_MultiFun_Set(WIFI_POWER_PIN,PINMUX_LEVEL_GPIO_END_FLAG);
		gpio_request(WIFI_POWER_PIN,"WIFI_POWER");
		gpio_direction_output(WIFI_POWER_PIN,0);
		gpio_set_value(WIFI_POWER_PIN,0);

		GPIO_MultiFun_Set(WIFI_RESET_PIN,PINMUX_LEVEL_GPIO_END_FLAG);
		gpio_request(WIFI_RESET_PIN,"WIFI_RESET");
		gpio_direction_output(WIFI_RESET_PIN,0);
		gpio_set_value(WIFI_RESET_PIN,0);
		msleep(20);

		gpio_set_value(WIFI_POWER_PIN,1);
		msleep(30);

		gpio_set_value(WIFI_RESET_PIN,1);
		msleep(100);
#elif defined(CONFIG_ARCH_AC83XX) || defined(CONFIG_ARCH_AC823X)
		iRet += gpiod_direction_output(mt6630_gpio[PWN_PIN].desc, 0);
		if (iRet) {
			pr_debug("[WMT_DEV:]can not set %s to output!\n",
					mt6630_gpio[PWN_PIN].name);
			goto err_gpio;
		}
		gpiod_set_value(mt6630_gpio[PWN_PIN].desc, 0);
		iRet += gpiod_direction_output(mt6630_gpio[RST_PIN].desc, 0);
		if (iRet) {
			pr_debug("[WMT_DEV:]can not set %s to output!\n",
					mt6630_gpio[PWN_PIN].name);
			goto err_gpio;
		}
		gpiod_set_value(mt6630_gpio[RST_PIN].desc, 0);
		msleep(20);
		gpiod_set_value(mt6630_gpio[PWN_PIN].desc, 1);
		msleep(30);
		gpiod_set_value(mt6630_gpio[RST_PIN].desc, 1);
		msleep(100);

#elif defined(CONFIG_ARCH_AC8X)
		if (gpio_is_valid(mt6630_gpio[PWN_PIN].num)
				&& gpio_is_valid(mt6630_gpio[RST_PIN].num)) {

			gpio_set_value(mt6630_gpio[PWN_PIN].num, 0);
			gpio_set_value(mt6630_gpio[RST_PIN].num, 0);
			msleep(20);
			gpio_set_value(mt6630_gpio[PWN_PIN].num, 1);
			msleep(30);
			gpio_set_value(mt6630_gpio[RST_PIN].num, 1);
			msleep(100);
			WMT_INFO_FUNC("[6630-gpio] pull up PWN_PIN: %d RST_PIN: %d\n",
					mt6630_gpio[PWN_PIN].num, mt6630_gpio[RST_PIN].num);
		} else {
			WMT_ERR_FUNC("[6630-gpio] invalid PWN_PIN: %d RST_PIN: %d\n",
					mt6630_gpio[PWN_PIN].num, mt6630_gpio[RST_PIN].num);
		}
#endif
		iRet += _mtk_wcn_cmb_hw_detect_change(1);
		WMT_INFO_FUNC("gpio is ok\n");
		msleep(100);

#ifdef CONFIG_ARCH_AC83XX
	} else {
		WMT_INFO_FUNC("this is EVB board\n");
	}
#endif

#if defined(CONFIG_ARCH_AC83XX) || defined(CONFIG_ARCH_AC823X)
err_gpio:
#endif
	_mtk_wcn_cmb_hw_err_handler(iRet);

	return iRet;
}

INT32 mtk_wcn_cmb_hw_rst(VOID)
{
#ifdef CONFIG_ATC_OS_VERSION_JB2
	unsigned int boardtype;
#endif
	INT32 iRet = 0;

	WMT_INFO_FUNC("CMB-HW, hw_rst start, eirq should be disabled before this step\n");
	if (0x6630 == mtk_wcn_wmt_chipid_query()) {
		switch (wmt_plat_get_comm_if_type()) {
		case STP_UART_IF_TX:
			iRet += wmt_plat_gpio_ctrl(PIN_UART_RX, PIN_STA_OUT_H);
			break;
		case STP_SDIO_IF_TX:
				iRet += wmt_plat_gpio_ctrl(PIN_UART_RX, PIN_STA_IN_L);
			break;
		default:
			WMT_ERR_FUNC("not supported common interface\n");
			break;
		}
	}

	/*1. PMU->output low, RST->output low, sleep off stable time */
	iRet += wmt_plat_gpio_ctrl(PIN_PMU, PIN_STA_OUT_L);
	iRet += wmt_plat_gpio_ctrl(PIN_RST, PIN_STA_OUT_L);
	osal_sleep_ms(gPwrSeqTime.offStableTime);

	/*2. PMU->output high, sleep rst stable time */
	iRet += wmt_plat_gpio_ctrl(PIN_PMU, PIN_STA_OUT_H);
	osal_sleep_ms(gPwrSeqTime.rstStableTime);

	/*3. RST->output high, sleep on stable time */
	iRet += wmt_plat_gpio_ctrl(PIN_RST, PIN_STA_OUT_H);
	osal_sleep_ms(gPwrSeqTime.onStableTime);

	/*set UART Tx/Rx to UART mode */
	if (0x6630 == mtk_wcn_wmt_chipid_query())
			iRet += wmt_plat_gpio_ctrl(PIN_UART_RX, PIN_STA_IN_H);

	WMT_INFO_FUNC("CMB-HW, hw_rst finish, eirq should be enabled after this step\n");
	/*Actual GPIO reset of MT6630 for ATC*/

#ifdef CONFIG_ATC_OS_VERSION_JB2
	boardtype = AC_BoardType_Get();

	if (boardtype == 0 ) {
		GPIO_MultiFun_Set(WIFI_POWER_PIN,PINMUX_LEVEL_GPIO_END_FLAG);
		gpio_request(WIFI_POWER_PIN,"WIFI_POWER");
		gpio_direction_output(WIFI_POWER_PIN,0);
		gpio_set_value(WIFI_POWER_PIN,0);

		GPIO_MultiFun_Set(WIFI_RESET_PIN,PINMUX_LEVEL_GPIO_END_FLAG);
		gpio_request(WIFI_RESET_PIN,"WIFI_RESET");
		gpio_direction_output(WIFI_RESET_PIN,0);
		gpio_set_value(WIFI_RESET_PIN,0);
		msleep(10);

		iRet += _mtk_wcn_cmb_hw_detect_change(0);
		msleep(100);
		coredump_packets_counter = 0;

		gpio_set_value(WIFI_POWER_PIN,1);
		msleep(30);

		gpio_set_value(WIFI_RESET_PIN,1);
		msleep(100);

		iRet += _mtk_wcn_cmb_hw_detect_change(1);
		//msleep(20000);
		msleep(100);
	} else {
		WMT_INFO_FUNC("this is EVB board\n");
		coredump_packets_counter = 0;
	}
#elif defined(CONFIG_ARCH_AC83XX) || defined(CONFIG_ARCH_AC823X)
	iRet += gpiod_direction_output(mt6630_gpio[PWN_PIN].desc, 0);
	if (iRet) {
		WMT_ERR_FUNC("[WMT_DEV:]can not set %s to output!\n",
			 mt6630_gpio[PWN_PIN].name);
		goto err_gpio;
	}
	gpiod_set_value(mt6630_gpio[PWN_PIN].desc, 0);
	iRet += gpiod_direction_output(mt6630_gpio[RST_PIN].desc, 0);
	if (iRet) {
		WMT_ERR_FUNC("[WMT_DEV:]can not set %s to output!\n",
			 mt6630_gpio[PWN_PIN].name);
		goto err_gpio;
	}
	gpiod_set_value(mt6630_gpio[RST_PIN].desc, 0);
	msleep(100);

	iRet += _mtk_wcn_cmb_hw_detect_change(0);
	msleep(100);
	coredump_packets_counter = 0;

	gpiod_set_value(mt6630_gpio[RST_PIN].desc, 0);
	msleep(20);
	gpiod_set_value(mt6630_gpio[PWN_PIN].desc, 1);
	msleep(30);
	gpiod_set_value(mt6630_gpio[RST_PIN].desc, 1);
	msleep(100);

	iRet += _mtk_wcn_cmb_hw_detect_change(1);
	msleep(100);

#elif defined(CONFIG_ARCH_AC8X)
		if (gpio_is_valid(mt6630_gpio[PWN_PIN].num)
				&& gpio_is_valid(mt6630_gpio[RST_PIN].num)) {

			gpio_set_value(mt6630_gpio[PWN_PIN].num, 0);
			gpio_set_value(mt6630_gpio[RST_PIN].num, 0);
			msleep(100);

			iRet += _mtk_wcn_cmb_hw_detect_change(0);
			msleep(100);
			coredump_packets_counter = 0;

			gpio_set_value(mt6630_gpio[RST_PIN].num, 0);
			msleep(20);
			gpio_set_value(mt6630_gpio[PWN_PIN].num, 1);
			msleep(30);
			gpio_set_value(mt6630_gpio[RST_PIN].num, 1);
			msleep(100);

			iRet += _mtk_wcn_cmb_hw_detect_change(1);
			msleep(100);

			WMT_WARN_FUNC("[6630-gpio] reset PWN_PIN: %d RST_PIN: %d\n",
					mt6630_gpio[PWN_PIN].num, mt6630_gpio[RST_PIN].num);
		} else {
			WMT_ERR_FUNC("[6630-gpio] invalid PWN_PIN: %d RST_PIN: %d\n",
					mt6630_gpio[PWN_PIN].num, mt6630_gpio[RST_PIN].num);
			coredump_packets_counter = 0;
		}
#endif

#if defined(CONFIG_ARCH_AC83XX) || defined(CONFIG_ARCH_AC823X)
err_gpio:
#endif
	WMT_INFO_FUNC("hw_rst finish iRet(%d)\n", iRet);

	return iRet;
}

static VOID mtk_wcn_cmb_hw_dmp_seq(VOID)
{
	PUINT32 pTimeSlot = (PUINT32) &gPwrSeqTime;

	WMT_INFO_FUNC
	    ("combo chip power on sequence time, RTC (%d), LDO (%d), RST(%d), OFF(%d), ON(%d)\n",
	     pTimeSlot[0],
		      /**pTimeSlot++,*/
	     pTimeSlot[1], pTimeSlot[2], pTimeSlot[3], pTimeSlot[4]
	    );
}

INT32 mtk_wcn_cmb_hw_state_show(VOID)
{
	wmt_plat_gpio_ctrl(PIN_PMU, PIN_STA_SHOW);
	wmt_plat_gpio_ctrl(PIN_RST, PIN_STA_SHOW);
	wmt_plat_gpio_ctrl(PIN_RTC, PIN_STA_SHOW);
	return 0;
}



INT32 mtk_wcn_cmb_hw_init(P_PWR_SEQ_TIME pPwrSeqTime)
{
	WMT_INFO_FUNC("mtk_wcn_cmb_hw_init\n");
	if (NULL != pPwrSeqTime &&
	    pPwrSeqTime->ldoStableTime > 0 &&
	    pPwrSeqTime->rtcStableTime > 0 &&
	    pPwrSeqTime->offStableTime > DFT_OFF_STABLE_TIME &&
	    pPwrSeqTime->onStableTime > DFT_ON_STABLE_TIME &&
	    pPwrSeqTime->rstStableTime > DFT_RST_STABLE_TIME) {
		/*memcpy may be more performance */
		WMT_DBG_FUNC("setting hw init sequence parameters\n");
		osal_memcpy(&gPwrSeqTime, pPwrSeqTime, osal_sizeof(gPwrSeqTime));
	} else {
		WMT_INFO_FUNC("invalid pPwrSeqTime parameter, "
				"use default hw init sequence parameters\n");
		gPwrSeqTime.ldoStableTime = DFT_LDO_STABLE_TIME;
		gPwrSeqTime.offStableTime = DFT_OFF_STABLE_TIME;
		gPwrSeqTime.onStableTime = DFT_ON_STABLE_TIME;
		gPwrSeqTime.rstStableTime = DFT_RST_STABLE_TIME;
		gPwrSeqTime.rtcStableTime = DFT_RTC_STABLE_TIME;
	}
	mtk_wcn_cmb_hw_dmp_seq();

	return 0;
}

INT32 mtk_wcn_cmb_hw_deinit(VOID)
{

	WMT_INFO_FUNC("mtk_wcn_cmb_hw_deinit start, "
			"set to default hw init sequence parameters\n");
	gPwrSeqTime.ldoStableTime = DFT_LDO_STABLE_TIME;
	gPwrSeqTime.offStableTime = DFT_OFF_STABLE_TIME;
	gPwrSeqTime.onStableTime = DFT_ON_STABLE_TIME;
	gPwrSeqTime.rstStableTime = DFT_RST_STABLE_TIME;
	gPwrSeqTime.rtcStableTime = DFT_RTC_STABLE_TIME;
	WMT_INFO_FUNC("mtk_wcn_cmb_hw_deinit finish\n");

	return 0;
}
