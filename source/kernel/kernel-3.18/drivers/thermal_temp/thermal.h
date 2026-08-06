
#ifndef _MTK_DRV_THERMAL_H
#define _MTK_DRV_THERMAL_H

#include "x_hal_ic.h"
#include "hal_io.h"
#include "mach/mt3365_irqs_vector.h"

// extern unsigned long rtc_reg_base0;

//#ifndef IO_VIRT_BASE
// #define IO_VIRT_BASE_EX   (rtc_reg_base0)
//#endif

//#ifndef THERM_CTRL_BASE
//#define THERM_CTRL_BASE  (IO_VIRT_BASE_EX + 0xA0000)
#define THERM_CTRL_BASE  (thm_reg_base)
//#endif

//#ifndef PDWNC_REG_BASE
//#define PDWNC_REG_BASE  (IO_VIRT_BASE_EX + 0x24000)
#define PDWNC_REG_BASE  (pdwnc_reg_base)
//#endif

//#ifndef CKGEN_BASE
//#define CKGEN_BASE  (IO_VIRT_BASE_EX)
#define CKGEN_BASE  (ckgen_reg_base)
//#endif

//#ifndef EFUSE_CTRL_BASE
//#define EFUSE_CTRL_BASE  (IO_VIRT_BASE_EX + 0x54000)
#define EFUSE_CTRL_BASE  (efuse_reg_base)
//#endif

//#define kal_uint32 UINT32
//#define kal_int32 INT32

#ifndef kal_uint32
typedef unsigned int       	kal_uint32;
#endif

#ifndef kal_int32
typedef int       			kal_int32;
#endif

#ifndef kal_uint16
typedef unsigned short int 	kal_uint16;
#endif

#ifndef kal_int16
typedef short int 			kal_int16;
#endif

#ifndef kal_uint8
typedef unsigned char      	kal_uint8;
#endif

#ifndef kal_int8
typedef char      			kal_int8;
#endif

#ifndef kal_bool
typedef unsigned int      	kal_bool;
#endif

//#define DRV_WriteReg32(reg32, val32) vIO32Write4B(reg32,val32)
//#define DRV_Reg32(reg32) u4IO32Read4B(reg32)

/*******************************************************************************
 * TV PDWNC servo ADC Register Definition
 ******************************************************************************/

#define PDWNC_GPIOIN (PDWNC_REG_BASE + 0xD0)
#define PDWNC_GPIOEN (PDWNC_REG_BASE + 0xD4)
#define PDWNC_GPIOOUT (PDWNC_REG_BASE + 0xD8)

#define PDWNC_PINMUX1 (PDWNC_REG_BASE + 0xF4)
#define PDWNC_PINMUX2 (PDWNC_REG_BASE + 0xF8)


#define PDWNC_SRVADC_BASE        (PDWNC_REG_BASE + 0x600) //0x24600

#define PDWNC_SRVCFG0 (PDWNC_SRVADC_BASE + 0x20)
	#define FLD_SRVCH_SEL Fld(5,4,AC_MSKW10)//[8:4]
	#define FLD_ABIST_MODE Fld(1,3,AC_MSKB0)//[3:3]
	#define FLD_SRVWAK_HD Fld(1,2,AC_MSKB0)//[2:2]
	#define FLD_SWEN Fld(1,1,AC_MSKB0)//[1:1]
	#define FLD_HWEN Fld(1,0,AC_MSKB0)//[0:0]

#define PDWNC_SRVCFG1 (PDWNC_SRVADC_BASE + 0x24)
	#define FLD_SRVCH_EN Fld(32,0,AC_FULLDW)//[31:0]
	
#define PDWNC_SRVSWT (PDWNC_SRVADC_BASE + 0x28)  //SRV adc sw trigger or hw trigger 
	#define FLD_SWTRG Fld(1,0,AC_MSKB0)//[0:0]
	
#define PDWNC_SRVCLR (PDWNC_SRVADC_BASE + 0x2C)
	#define FLD_ADCLR Fld(1,1,AC_MSKB0)//[1:1]
	
#define PDWNC_SRVRAT (PDWNC_SRVADC_BASE + 0x30)
	#define FLD_CK_DIV Fld(16,16,AC_FULLW10)//[31:16]
	#define FLD_SMP_RATE Fld(16,0,AC_FULLW10)//[15:0]
	
#define PDWNC_SRVTOTEN (PDWNC_SRVADC_BASE + 0x34)
	#define FLD_TOUT_CNT Fld(8,8,AC_FULLB1)//[15:8]
	#define FLD_SRVHD_ST Fld(1,3,AC_MSKB0)//[3:3]
	#define FLD_DATA_RDST Fld(1,2,AC_MSKB0)//[2:2]
	#define FLD_TOUT_ST Fld(1,1,AC_MSKB0)//[1:1]
	#define FLD_TOUT_EN Fld(1,0,AC_MSKB0)//[0:0]


/* for Tablet IC
#define AUXADC_CON1         (AUXADC_BASE + 0x004)
#define AUXADC_CON1_SET     (AUXADC_BASE + 0x008)
#define AUXADC_CON1_CLR     (AUXADC_BASE + 0x00C)
#define AUXADC_CON2         (AUXADC_BASE + 0x010)
//#define AUXADC_CON3         (AUXADC_BASE + 0x014)
#define AUXADC_DAT0         (AUXADC_BASE + 0x014)
#define AUXADC_DAT1         (AUXADC_BASE + 0x018)
#define AUXADC_DAT2         (AUXADC_BASE + 0x01C)
#define AUXADC_DAT3         (AUXADC_BASE + 0x020)
#define AUXADC_DAT4         (AUXADC_BASE + 0x024)
#define AUXADC_DAT5         (AUXADC_BASE + 0x028)
#define AUXADC_DAT6         (AUXADC_BASE + 0x02C)
#define AUXADC_MISC         (AUXADC_BASE + 0x094)
#define AUXADC_DAT11	    (AUXADC_BASE + 0x40)

//Peri Module 
#define PERI_GLOBALCON_RST0 (PERICFG_BASE + 0x000)

// APB Module infracfg_ao

#define INFRA_GLOBALCON_RST_0_SET (INFRACFG_AO_BASE + 0x120) //0x10001000
#define INFRA_GLOBALCON_RST_0_CLR (INFRACFG_AO_BASE + 0x124) //0x10001000
#define INFRA_GLOBALCON_RST_0_STA (INFRACFG_AO_BASE + 0x128) //0x10001000

*/


/*for SROADC channel*/
#define PDWNC_ADOUT0 	( PDWNC_SRVADC_BASE + 0x40 )
#define PDWNC_ADOUT1 	( PDWNC_SRVADC_BASE + 0x44 )
#define PDWNC_ADOUT2 	( PDWNC_SRVADC_BASE + 0x48 )
#define PDWNC_ADOUT3 	( PDWNC_SRVADC_BASE + 0x4C )
#define PDWNC_ADOUT4 	( PDWNC_SRVADC_BASE + 0x50 )
#define PDWNC_ADOUT5 	( PDWNC_SRVADC_BASE + 0x54 )
#define PDWNC_ADOUT6 	( PDWNC_SRVADC_BASE + 0x58 )
#define PDWNC_ADOUT7 	( PDWNC_SRVADC_BASE + 0x5C )
#define PDWNC_ADOUT8 	( PDWNC_SRVADC_BASE + 0x60 )
#define PDWNC_ADOUT9 	( PDWNC_SRVADC_BASE + 0x64 )
#define PDWNC_ADOUT10 	( PDWNC_SRVADC_BASE + 0x68 )
#define PDWNC_ADOUT11 	( PDWNC_SRVADC_BASE + 0x6C )
#define PDWNC_ADOUT12 	( PDWNC_SRVADC_BASE + 0x70 )
#define PDWNC_ADOUT13 	( PDWNC_SRVADC_BASE + 0x74 )
#define PDWNC_ADOUT14 	( PDWNC_SRVADC_BASE + 0x78 )
#define PDWNC_ADOUT15 	( PDWNC_SRVADC_BASE + 0x7C )


/*******************************************************************************
 * Thermal Controller Register Definition
 ******************************************************************************/
#define TEMPMONCTL0         (THERM_CTRL_BASE + 0x000)
#define TEMPMONCTL1         (THERM_CTRL_BASE + 0x004)
#define TEMPMONCTL2         (THERM_CTRL_BASE + 0x008)
#define TEMPMONINT          (THERM_CTRL_BASE + 0x00C)
#define TEMPMONINTSTS       (THERM_CTRL_BASE + 0x010)
#define TEMPMONIDET0        (THERM_CTRL_BASE + 0x014)
#define TEMPMONIDET1        (THERM_CTRL_BASE + 0x018)
#define TEMPMONIDET2        (THERM_CTRL_BASE + 0x01C)
#define TEMPMONIDET3        (THERM_CTRL_BASE + 0x0B0)
#define TEMPH2NTHRE         (THERM_CTRL_BASE + 0x024)
#define TEMPHTHRE           (THERM_CTRL_BASE + 0x028)
#define TEMPCTHRE           (THERM_CTRL_BASE + 0x02C)
#define TEMPOFFSETH         (THERM_CTRL_BASE + 0x030)
#define TEMPOFFSETL         (THERM_CTRL_BASE + 0x034)
#define TEMPMSRCTL0         (THERM_CTRL_BASE + 0x038)
#define TEMPMSRCTL1         (THERM_CTRL_BASE + 0x03C)
#define TEMPAHBPOLL         (THERM_CTRL_BASE + 0x040)
#define TEMPAHBTO           (THERM_CTRL_BASE + 0x044)
#define TEMPADCPNP0         (THERM_CTRL_BASE + 0x048)
#define TEMPADCPNP1         (THERM_CTRL_BASE + 0x04C)
#define TEMPADCPNP2         (THERM_CTRL_BASE + 0x050)
#define TEMPADCPNP3         (THERM_CTRL_BASE + 0x0B4)

#define TEMPADCMUX          (THERM_CTRL_BASE + 0x054)
#define TEMPADCEXT          (THERM_CTRL_BASE + 0x058)
#define TEMPADCEXT1         (THERM_CTRL_BASE + 0x05C)
#define TEMPADCEN           (THERM_CTRL_BASE + 0x060)
#define TEMPPNPMUXADDR      (THERM_CTRL_BASE + 0x064)
#define TEMPADCMUXADDR      (THERM_CTRL_BASE + 0x068)
#define TEMPADCEXTADDR      (THERM_CTRL_BASE + 0x06C)
#define TEMPADCEXT1ADDR     (THERM_CTRL_BASE + 0x070)
#define TEMPADCENADDR       (THERM_CTRL_BASE + 0x074)
#define TEMPADCVALIDADDR    (THERM_CTRL_BASE + 0x078)
#define TEMPADCVOLTADDR     (THERM_CTRL_BASE + 0x07C)
#define TEMPRDCTRL          (THERM_CTRL_BASE + 0x080)
#define TEMPADCVALIDMASK    (THERM_CTRL_BASE + 0x084)
#define TEMPADCVOLTAGESHIFT (THERM_CTRL_BASE + 0x088)
#define TEMPADCWRITECTRL    (THERM_CTRL_BASE + 0x08C)
#define TEMPMSR0            (THERM_CTRL_BASE + 0x090)
#define TEMPMSR1            (THERM_CTRL_BASE + 0x094)
#define TEMPMSR2            (THERM_CTRL_BASE + 0x098)
#define TEMPMSR3            (THERM_CTRL_BASE + 0x0B8)
#define TEMPIMMD0           (THERM_CTRL_BASE + 0x0A0)
#define TEMPIMMD1           (THERM_CTRL_BASE + 0x0A4)
#define TEMPIMMD2           (THERM_CTRL_BASE + 0x0A8)
#define TEMPIMMD3           (THERM_CTRL_BASE + 0x0BC)

#define TEMPSPARE0          (THERM_CTRL_BASE + 0x0F0)
#define TEMPSPARE1          (THERM_CTRL_BASE + 0x0F4)
#define TEMPSPARE2          (THERM_CTRL_BASE + 0x0F8)
#define TEMPSPARE3          (THERM_CTRL_BASE + 0x0FC)


#define TEMPPROTCTL         (THERM_CTRL_BASE + 0x0C0)
#define TEMPPROTTA          (THERM_CTRL_BASE + 0x0C4)
#define TEMPPROTTB          (THERM_CTRL_BASE + 0x0C8)
#define TEMPPROTTC          (THERM_CTRL_BASE + 0x0CC)


/// zplee
#define PTPCORESEL          (THERM_CTRL_BASE + 0x400)
#define THERMINTST          (THERM_CTRL_BASE + 0x404)
#define PTPODINTST          (THERM_CTRL_BASE + 0x408)
#define THSTAGE0ST          (THERM_CTRL_BASE + 0x40C)
#define THSTAGE1ST          (THERM_CTRL_BASE + 0x410)
#define THSTAGE2ST          (THERM_CTRL_BASE + 0x414)
#define THAHBST0            (THERM_CTRL_BASE + 0x418)
#define THAHBST1            (THERM_CTRL_BASE + 0x41C) //Only for DE debug

#if 0
#define PTPSPARE0           (THERM_CTRL_BASE + 0x420)
#define PTPSPARE1           (THERM_CTRL_BASE + 0x424)
#define PTPSPARE2           (THERM_CTRL_BASE + 0x428)
#define PTPSPARE3           (THERM_CTRL_BASE + 0x42C)
#define THSLPEVEB           (THERM_CTRL_BASE + 0x430)

#define PTPSPARE0_P         (THERM_CTRL_BASE + 0x420)
#define PTPSPARE1_P         (THERM_CTRL_BASE + 0x424)
#define PTPSPARE2_P         (THERM_CTRL_BASE + 0x428)
#define PTPSPARE3_P         (THERM_CTRL_BASE + 0x42C)
#else
#define PTPSPARE0 TEMPSPARE0
#define PTPSPARE1 TEMPSPARE1
#define PTPSPARE2 TEMPSPARE2
#define PTPSPARE3 TEMPSPARE3    

#define PTPSPARE0_P PTPSPARE0
#define PTPSPARE1_P PTPSPARE1
#define PTPSPARE2_P PTPSPARE2
#define PTPSPARE3_P PTPSPARE3
#endif

//#define TS_CON0             (APMIXED_BASE + 0x800)
//#define TS_CON1             (APMIXED_BASE + 0x804)

//APMIXED_BASE related with row data convert document( CY Chien)
//#define TS_CON0             (APMIXED_BASE + 0x600)
//#define TS_CON1             (APMIXED_BASE + 0x604)
//#define TS_CON2             (APMIXED_BASE + 0x808)

/*******************************************************************************
 * Thermal Controller Register Mask Definition
 ******************************************************************************/
#define THERMAL_ENABLE_SEN0     0x1
#define THERMAL_ENABLE_SEN1     0x2
#define THERMAL_ENABLE_SEN2     0x4
#define THERMAL_MONCTL0_MASK    0x00000007

#define THERMAL_PUNT_MASK       0x00000FFF
#define THERMAL_FSINTVL_MASK    0x03FF0000
#define THERMAL_SPINTVL_MASK    0x000003FF
#define THERMAL_MON_INT_MASK    0x0007FFFF

#define THERMAL_MON_CINTSTS0    0x000001
#define THERMAL_MON_HINTSTS0    0x000002
#define THERMAL_MON_LOINTSTS0   0x000004
#define THERMAL_MON_HOINTSTS0   0x000008
#define THERMAL_MON_NHINTSTS0   0x000010
#define THERMAL_MON_CINTSTS1    0x000020
#define THERMAL_MON_HINTSTS1    0x000040
#define THERMAL_MON_LOINTSTS1   0x000080
#define THERMAL_MON_HOINTSTS1   0x000100
#define THERMAL_MON_NHINTSTS1   0x000200
#define THERMAL_MON_CINTSTS2    0x000400
#define THERMAL_MON_HINTSTS2    0x000800
#define THERMAL_MON_LOINTSTS2   0x001000
#define THERMAL_MON_HOINTSTS2   0x002000
#define THERMAL_MON_NHINTSTS2   0x004000
#define THERMAL_MON_TOINTSTS    0x008000
#define THERMAL_MON_IMMDINTSTS0 0x010000
#define THERMAL_MON_IMMDINTSTS1 0x020000
#define THERMAL_MON_IMMDINTSTS2 0x040000
#define THERMAL_MON_FILTINTSTS0 0x080000
#define THERMAL_MON_FILTINTSTS1 0x100000
#define THERMAL_MON_FILTINTSTS2 0x200000
#define THERMAL_MON_CINTSTS3    0x400000
#define THERMAL_MON_HINTSTS3    0x800000
#define THERMAL_MON_LOINTSTS3   0x1000000
#define THERMAL_MON_HOINTSTS3   0x2000000
#define THERMAL_MON_NHINTSTS3   0x4000000
#define THERMAL_MON_IMMDINTSTS3 0x8000000
#define THERMAL_MON_FILTINTSTS3 0x10000000

//hywu: Interrupt status for thermal protection stage 0/1/2
#define THERMAL_tri_SPM_State0	0x20000000
#define THERMAL_tri_SPM_State1	0x40000000
#define THERMAL_tri_SPM_State2	0x80000000


#define THERMAL_MSRCTL0_MASK    0x00000007
#define THERMAL_MSRCTL1_MASK    0x00000038
#define THERMAL_MSRCTL2_MASK    0x000001C0


#if 1  // for temp
#define REG_RW_CKRST_CFG2		0x00000BC
#define VECTOR_PTP_THERM        VECTOR_THERM
#endif

#if 1
#ifndef HAL_READ32
#define HAL_READ32(_reg_)          (*((volatile uint32_t*)((unsigned long)_reg_)))
#endif

#ifndef HAL_WRITE32
#define HAL_WRITE32(_reg_, _val_)  (*((volatile uint32_t*)((unsigned long)_reg_)) = (_val_))
#endif

#ifndef IO_READ32
#define IO_READ32(base, offset)				HAL_READ32((base) + (offset))
#endif

#ifndef IO_WRITE32
#define IO_WRITE32(base, offset, value)		HAL_WRITE32((base) + (offset), (value))
#endif

#define DRV_Reg32(reg)		IO_READ32(reg, 0)
#define DRV_WriteReg32(reg, value)		IO_WRITE32(reg, 0 , value)
#else
#define DRV_Reg32(offset)			IO_READ32(IO_UCV_BASE, (offset))
#define DRV_WriteReg32(offset, value)		IO_WRITE32(IO_UCV_BASE, (offset), (value))

//#define RTC_READ32(offset)			    IO_READ32((RTC_UCV_BASE + offset), 0)
//#define RTC_WRITE32(offset, value)	    IO_WRITE32((RTC_UCV_BASE + offset), 0 , (value))

//#define RTC_SET_BIT(offset, Bit)        RTC_WRITE32(offset, RTC_READ32(offset) | (Bit))
//#define RTC_CLR_BIT(offset, Bit)        RTC_WRITE32(offset, RTC_READ32(offset) & (~(Bit)))


#endif


typedef enum
{
    THERMAL_SENSOR1     = 0,//TS_MCU1
    THERMAL_SENSOR2     = 1,//TS_MCU2
    THERMAL_SENSOR3     = 2,//TS_MCU3
    THERMAL_SENSOR4     = 3,//TS_MCU4
    //THERMAL_SENSORABB   = 4,//TS_ABB
    THERMAL_SENSOR_NUM
} thermal_sensor_name;

/*
Bank0 : CPU (TS3, TS4)     (TS_MCU1,TS_MCU2)
Bank1 : GPU (TS5)          (TS_MCU3)
Bank2 : SOC (TS1, TS4, TS5)(TS_MCU4,TS_MCU2,TS_MCU3)

TS_ABB: TS2
*/

typedef enum
{
    THERMAL_BANK0     = 0,//CPU  TSMCU1 (TS_MCU1,TS_MCU2)            (TS3, TS4)
    THERMAL_BANK1     = 1,//GPU  TSMCU2 (TS_MCU3) 					 (TS5)
    THERMAL_BANK2     = 2,//SOC  TSMCU3 (TS_MCU4,TS_MCU2,TS_MCU3)    (TS1, TS4, TS5)
    ROME_BANK_NUM	  = 1
} thermal_bank_name;

extern int thermal_init(void);
extern void thermal_exit(void);


extern int thermal_interrupt_trigger_test(void);
extern kal_uint32 thermal_get_interrupt_status(void);

extern int thermal_interrupt_occurrance_test(kal_uint32 times);
extern int thermal_sensing_filter_option_test(kal_uint16 option);
extern int thermal_immediate_measurement_test(void);
extern int thermal_ahb_timeout_test(kal_uint32 u4poling, kal_uint32 u4timeout);
extern int thermal_first_hot_interrupt_test(kal_bool first_hot_en);
extern int thermal_filter_sample_interval_test(kal_uint32 filt_interval, kal_uint32 sen_interval);
extern int thermal_different_threshold_test(kal_uint32 hot_threshold, kal_uint32 high_offset_threshold, kal_uint32 h2n_threshold, kal_uint32 low_offset_threshold, kal_uint32 cold_threshold);
extern int thermal_interrupt_mask_test(void);
extern int thermal_real_interrupt_test(unsigned int sensor);
extern int thermal_real_interrupt_test_HCHL(unsigned int sensor);
extern int thermal_real_interrupt_test_H2N(unsigned int sensor);
extern int thermal_interrupt_trigger_to_SPM_test(void);
extern int thermal_trigger_wdt_reset(void);
extern int mtktscpu_get_hw_temp(void);
extern int mtktscpu_switch_bank(thermal_bank_name bank);

extern int thermal_adc_hw_auto_mode(unsigned int channel);
extern int thermal_adc_sw_trigger_mode(unsigned int channel);
extern int thermal_adc_io_trigger_mode(unsigned int channel);
extern int thermal_adc_change_sample_rate(unsigned int channel);
extern int thermal_adc_calibration(unsigned int channel);
extern int thermal_adc_sndr(unsigned int channel);



extern int mtktscpu_get_each_bank_temp(thermal_bank_name bank_num);
extern int thermal_get_all_TS_temp(void);
#endif

