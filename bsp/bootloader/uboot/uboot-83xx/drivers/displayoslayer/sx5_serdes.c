#include <config.h>
#include <command.h>
#include <asm/io.h>
#include <common.h>

#include <ac83xx_gpio_pinmux.h>
#include <pinmux.h>
#include <ac83xx_pinmux_table.h>
#include <asm-arm/arch-ac83xx/ac83xx_i2c.h>
#include <ac83xx_gpio_pinmux_mapping.h>

#define TI_928 (0)
#define TI_924 (1)

/*		Deserializer registers		*/
#define FPD3_DES_DEV_ID			0x00
#define FPD3_DES_RESET			0x01
	#define DIGITAL_RESET0	(0<<0)
	#define DIGITAL_RESET1	(2<<1)

#define FPD3_DES_CONFIG0		0x02
#define FPD3_DES_CONFIG1		0x03
#define FPD3_DES_CONFIG1_VALUE		0x4c

#define FPD3_DES_SER_ID			0x06
#define FPD3_DES_SER_AL			0x07
#define FPD3_DES_SLAVE_ID0		0x08
#define FPD3_DES_SLAVE_AL0		0x10

#define FPD3_DES_GN_STS			0x1c
	#define SIGNAL_DETECT	(1<<1)

#define FPD3_DES_GPIO_01		0x1d
#define FPD3_DES_GPIO_23		0x1e
	#define SHIFT_DIR	3
	#define SHIFT_VAL	1
	#define DIR_INPUT	1
	#define DIR_OUTPUT	0
#define FPD3_DES_GPIO_3		0x1f

#define FPD3_DES_I2C_CTRL1		0x21
#define FPD3_DES_I2C_CTRL2		0x22

#define FPD3_DES_GPIO_56               	0x20
#define FPD3_DES_GPIO_78               	0x21

#define FPD3_DES_I2C_CTRL1X	0x05
#define FPD3_DES_MAP_SEL		0x49

/*		Serializer registers		*/
#define FPD3_SER_DEV_ID			0x00
#define FPD3_SER_RESET			0x01

#define FPD3_SER_CONFIG0		0x02
#define FPD3_SER_CONFIG1		0x03
#define FPD3_SER_CONFIG2		0x04

#define FPD3_SER_DES_ID			0x06
#define FPD3_SER_DES_AL			0x06
#define FPD3_SER_SLAVE_ID0		0x07
#define FPD3_SER_SLAVE_AL0		0x08

#define FPD3_SER_GN_STS			0x0c
	#define SIGNAL_DETECT	(1<<1)

#define FPD3_SER_GPIO_01		0x0d
#define FPD3_SER_GPIO_23		0x0e

#define FPD3_SER_GPIO_3		    0x0f
#define FPD3_SER_GPIO_56		0x10
#define FPD3_SER_GPIO_78		0x11

#define FPD3_SER_DATA_CTRL		0x12
#define FPD3_SER_GPC_CTRL		0x13
#define FPD3_SER_I2C_CTRL1		0x21
#define FPD3_SER_I2C_CTRL2		0x22

#define FPD3_SER_I2C_CTRL		0x17
#define FPD3_SER_INT_ICR		0xC6
#define FPD3_SER_INT_ISR		0xC7
#define MAX_POLL_COUNT			100
#define FPD3_SERDES_MAX_SLAVES		10
#define FPD3_I2C_MSG_BUFFER_NUM		32
//device address (i2c address)
#define FPD3_SER_921Q_DEVICE_ID		0x0C
#define FPD3_DES_924Q_DEVICE_ID		0x2C

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))


static const unsigned int fpd3_24bit_ser_921_928_init[] = {
	/* digital reset1 */
	FPD3_SER_RESET,		0x02,
	/* back chan en, auto ack WR, i2c passthrough, rising edge pclk */
	FPD3_SER_CONFIG1,	0xdb,
	/* failsafe low, LFMODE override, high freq*/
	FPD3_SER_CONFIG2,	0x8a,
	/* RGB data, 24bit, i2s data forwarding */
	FPD3_SER_DATA_CTRL,	0x00,
	FPD3_SER_GPC_CTRL,0x70,
	FPD3_SER_GPIO_01, 0x01,
	FPD3_SER_GPIO_23, 0x35,
	FPD3_SER_GPIO_3, 0x03,
	//FPD3_SER_GPIO_78, 0x00,
	FPD3_SER_GPIO_78, 0x90,
	FPD3_SER_SLAVE_ID0,		0x94,
	FPD3_SER_SLAVE_AL0,		0x94,
	//FPD3_SER_I2C_CTRL,		0x9E,
	FPD3_SER_INT_ICR, 		0x21,
};

static const unsigned int fpd3_24bit_ser_921_924_init[] = {
	/* digital reset1 */
	FPD3_SER_RESET,		0x02,
	/* back chan en, auto ack WR, i2c passthrough, rising edge pclk */
	FPD3_SER_CONFIG1,	0xdb,
	/* failsafe low, LFMODE override, high freq*/
	FPD3_SER_CONFIG2,	0x8a,
	/* RGB data, 24bit, i2s data forwarding */
	FPD3_SER_DATA_CTRL,	0x00,
	FPD3_SER_GPC_CTRL,0x70,
	FPD3_SER_GPIO_01, 0x01,
	FPD3_SER_GPIO_23, 0x35,
	FPD3_SER_GPIO_3, 0x03,
	//FPD3_SER_GPIO_78, 0x00,
	FPD3_SER_GPIO_78, 0x30,
	FPD3_SER_SLAVE_ID0,		0x94,
	FPD3_SER_SLAVE_AL0,		0x94,
	//FPD3_SER_I2C_CTRL,		0x9E,
	FPD3_SER_INT_ICR, 		0x21,
};

static const unsigned int fpd3_24bit_des_928_init[] = {
	/* digital reset1 */
	FPD3_DES_RESET,		0x02,
	/* back channel en */
	FPD3_DES_RESET,		0x04,
	/* auto clk en, LFMODE override, high freq */
	FPD3_DES_CONFIG0,	0x2a,
	/* failsafe pull up, i2c passthrough, auto ack */
	FPD3_DES_CONFIG1,	FPD3_DES_CONFIG1_VALUE,
	FPD3_DES_MAP_SEL,	0x60,
	//gpio-0:output,enable remote control,reset pin-mxt641
	FPD3_DES_GPIO_01,	0x01,

	//FPD3_DES_GPIO_23,  	0x53,
	//FPD3_DES_GPIO_3, 	0x05,

	FPD3_DES_GPIO_3, 	0x01,
	//FPD3_DES_GPIO_56,	0x11,
	//FPD3_DES_GPIO_56,	0x91,
	FPD3_DES_GPIO_78,	0x10,
	FPD3_DES_GPIO_56,	0x19,
	//FPD3_DES_I2C_CTRL1X,0x9e,

	/*928 Setting*/
	FPD3_DES_GPIO_23,  	0x93,
	FPD3_DES_GPIO_3, 	0x09,
};

static const unsigned int fpd3_24bit_des_924_init[] = {
	/* digital reset1 */
	FPD3_DES_RESET,		0x02,
	/* back channel en */
	FPD3_DES_RESET,		0x04,
	/* auto clk en, LFMODE override, high freq */
	FPD3_DES_CONFIG0,	0x2a,
	/* failsafe pull up, i2c passthrough, auto ack */
	FPD3_DES_CONFIG1,	FPD3_DES_CONFIG1_VALUE,
	FPD3_DES_MAP_SEL,	0x60,
	//gpio-0:output,enable remote control,reset pin-mxt641
	FPD3_DES_GPIO_01,	0x01,

	//FPD3_DES_GPIO_23,  	0x53,
	//FPD3_DES_GPIO_3, 	0x05,

	FPD3_DES_GPIO_3, 	0x01,
	//FPD3_DES_GPIO_56,	0x11,
	//FPD3_DES_GPIO_56,	0x91,
	FPD3_DES_GPIO_78,	0x10,
	FPD3_DES_GPIO_56,	0x19,
	//FPD3_DES_I2C_CTRL1X,0x9e,
	/*924 Setting
	FPD3_DES_GPIO_23,  	0x53,*/

	/*928 Setting*/
	FPD3_DES_GPIO_23,  	0x93,
	FPD3_DES_GPIO_3, 	0x09,

};

#define mdelay(n) ({unsigned long msec=(n); while (msec--) udelay(1000);})

static int ts_restore()
{
	int ret;

	printf("ts_restore  i2c addr:0x%x\n",FPD3_DES_924Q_DEVICE_ID);
	mdelay(1);
	ret = i2c_write(FPD3_DES_924Q_DEVICE_ID, FPD3_DES_GPIO_01,2,0x09,1);
	if(ret){
		printf("set gpio0 to high i2c error:%d\n",ret);
	}else{
		printf("tp reset succeed\n");
	}
	return 0;
}
#if 0
void wakeup_process()
{
	u8 buf[5] = {0x1,0x1,0x0,0x0,0x0};

	i2c_write(FPD3_SER_921Q_DEVICE_ID,FPD3_SER_RESET,1,&buf[2],1);
	i2c_write(FPD3_SER_921Q_DEVICE_ID,FPD3_SER_CONFIG1,1,&buf[3],1);
	i2c_write(FPD3_SER_921Q_DEVICE_ID,0x05,1,&buf[4],1);
	i2c_write(FPD3_DES_924Q_DEVICE_ID,FPD3_DES_I2C_CTRL1X,1,&buf[0],1);
	i2c_write(FPD3_DES_924Q_DEVICE_ID,FPD3_DES_CONFIG1,1,&buf[1],1);


}
#endif
u8 des_val[]={0x90,0x99,0x01,0x9};
int init_serdes(unsigned serdes_addr,unsigned int * seq,int len)
{
	int i;
	int ret = 0;
	//printf("init start 0000 ---\n");
	u8 buf[7] = {};

	for (i = 0; i < len; i += 2) {
		//printf("yds-11 serdes addr 0x%x ,is i=%d\n",serdes_addr,i);
#if 0
		if(client->addr == FPD3_DES_928Q_DEVICE_ID)
			if((seq[i] == FPD3_DES_GPIO_23 && (seq[i+1] == 0x53 ))){
				dev_err(&client->dev,"FPD SER-928 delay 5ms to open pwm\n");
				mdelay(5);
		}
#endif

		ret = i2c_write(serdes_addr,seq[i],1,&seq[i+1],1);
		if (ret) {
			printf("i2c write error: seq-0:0x%02x  seq-1:0x%02x,errcode=%d\n",
						seq[i], seq[i+1],ret);
			if(serdes_addr == FPD3_DES_924Q_DEVICE_ID &&
				seq[i] ==  FPD3_DES_RESET && seq[i+1] == 0x04) {
				printf("FPD SER-928 in reset state,wait for 1 ms\n");
				udelay(1000);
				continue;
			} else {
				//printf("FPD device i2c write error:%d\n",ret);
				//break;
			}
		}
#if 0
		/*ret = i2c_read(serdes_addr, seq[i], 1, &buf[2],1);
		if(ret) {
			printf("Read addr is 0x%x device addr 0x%x ,read offest value is 0x%x  fail\n",serdes_addr,seq[i],buf[2]);
		} else {
			printf("Read addr is 0x%x device addr 0x%x ,read offest value  is 0x%x sucess \n",serdes_addr,seq[i],buf[2]);
		}*/
#endif
		if (seq[i] == 0x01)
			udelay(500);
		else
			udelay(10);

		if(serdes_addr== FPD3_DES_924Q_DEVICE_ID)
			if(seq[i] == FPD3_DES_GPIO_56 && seq[i+1] == 0x19) {
				printf("setup stb status!\n");
				mdelay(5);
		}
		if(serdes_addr == FPD3_DES_924Q_DEVICE_ID)
			if(seq[i] == FPD3_DES_GPIO_3 && seq[i+1] == 0x09) {
				mdelay(1);
				ret = i2c_write(serdes_addr, FPD3_DES_GPIO_78,1,&des_val[0],1);
				if(ret){
					printf("set gpio8 to high i2c error:%d\n",ret);
				}
				ret = i2c_write(serdes_addr, FPD3_DES_GPIO_56,1,&des_val[1],1);
				if(ret) {
					printf("set gpio6 to high i2c error:%d\n",ret);
				}
				mdelay(100);
				ret = i2c_write(serdes_addr, FPD3_DES_GPIO_3,1,&des_val[2],1);
				if(ret){
					printf("set tft-reset to low i2c error:%d\n",ret);
				}
				mdelay(50);
				ret = i2c_write(serdes_addr, FPD3_DES_GPIO_3,1,&des_val[3],1);
				if(ret){
					printf("set tft-reset to high i2c error:%d\n",ret);
				}
			}
	}
	return ret;

}

#define GT_WRITE(_reg, _val)    (*((volatile uint32_t*)(_reg)) = (_val))
#define GT_READ(_reg)       (*((volatile uint32_t*)(_reg)))
#define LVDS_RST PIN_124_GPIO124

void gtp_gpio_set_value(unsigned gpio, int value)
{
	unsigned val, idx, offset;

	offset = gpio % 32;
	idx  = gpio / 32;
	val = GT_READ(0xF00000E0 + (4 * idx));
	val = (value == 1) ? (val | (1U << offset)) : (val & ~(1U << offset));
	GT_WRITE(0xF00000E0 + (4 * idx), val);
}

void gtp_gpio_inout_sel(unsigned gpio, int dir)
{
	unsigned val, idx, offset;

	offset = gpio % 32;
	idx = gpio / 32;

	val = GT_READ(0xF0000074 + (4 * idx));
	val = (dir == OUTPUT) ? (val | (1U << offset)) : (val & ~(1U << offset));
	GT_WRITE(0xF0000074 + (4 * idx), val);

}

int gtp_gpio_direction_output(unsigned gpio, int value)
{
	gtp_gpio_set_value(gpio, value);
	gtp_gpio_inout_sel(gpio, 1);   //OUTPUT = 1
	return 0;
}

void reset_panel()
{

}

int init_ti_chip(int ti_type)
{
	printf("i2c_init in ti chip and reset 124 gpio \n");
	//gtp_gpio_direction_output(LVDS_RST,0);
	//mdelay(20);
	gtp_gpio_direction_output(LVDS_RST,1);
	i2c_init(400);
	if(ti_type == TI_928) {
		printf("Start init 921_928 ser ti chip\n");
		init_serdes(FPD3_SER_921Q_DEVICE_ID,fpd3_24bit_ser_921_928_init,ARRAY_SIZE(fpd3_24bit_ser_921_928_init));

		printf("Start init 928 des ti chip\n");
		init_serdes(FPD3_DES_924Q_DEVICE_ID,fpd3_24bit_des_928_init,ARRAY_SIZE(fpd3_24bit_des_928_init));
		ts_restore();
	}else if(ti_type == TI_924){
		printf("Start init 921_924 ser ti chip\n");
		des_val[0] = 0x50;
		init_serdes(FPD3_SER_921Q_DEVICE_ID,fpd3_24bit_ser_921_924_init,ARRAY_SIZE(fpd3_24bit_ser_921_924_init));

		printf("Start init 924 des ti chip\n");
		init_serdes(FPD3_DES_924Q_DEVICE_ID,fpd3_24bit_des_924_init,ARRAY_SIZE(fpd3_24bit_des_924_init));
		ts_restore();
	} else {
		printf("Not Support this ti type\n");
	}

	printf("End init serdes ti chip\n");
}
