#include <config.h>
#include <command.h>
#include <asm/io.h>
#include <common.h>

#include <ac83xx_gpio_pinmux.h>
#include <pinmux.h>
#include <ac83xx_pinmux_table.h>
#include <asm-arm/arch-ac83xx/ac83xx_i2c.h>
#include <ac83xx_gpio_pinmux_mapping.h>

/*************************CUST*********************/
#define RESET_PORT          TP_RESET_PORT
#define INT_GPIO                TP_EINT_PIN
    
#define INT_PORT            TP_INT_PORT
#define INT_FUNCTION    TP_INT_FUNCTION
#define EINT_NUM            TP_EINT_NUM

#define DISPLAY_MAX_HEIGHT      600 
#define DISPLAY_MAX_WIDTH       1024    
/*************************END*********************/

#define MAX_BUTTONS 4
#define GOODIX_MAX_CFG_GROUP    6
#define GTP_FW_NAME_MAXSIZE 50

#if GTP_CUSTOM_CFG
#define GTP_MAX_HEIGHT      600 
#define GTP_MAX_WIDTH       1024    
#define GTP_INT_TRIGGER     GTP_IRQ_TAB_RISING
#else
#define GTP_MAX_HEIGHT      4096
#define GTP_MAX_WIDTH       4096
#define GTP_INT_TRIGGER     GTP_IRQ_TAB_FALLING
#endif

#define GTP_PRODUCT_ID_MAXSIZE  5
#define GTP_PRODUCT_ID_BUFFER_MAXSIZE   6
#define GTP_FW_VERSION_BUFFER_MAXSIZE   4
#define GTP_MAX_TOUCH       5
#define GTP_ESD_CHECK_CIRCLE    2000      /* jiffy: ms */

/***************************PART3:OTHER define*********************************/
#define GTP_DRIVER_VERSION  "V1.8.1<2013/09/01>"
#define GTP_I2C_NAME        "ac83xx_tsadcc"
#define GTP_POLL_TIME       10     /* jiffy: ms*/
#define GTP_ADDR_LENGTH     2
#define GTP_CONFIG_MIN_LENGTH   186
#define GTP_CONFIG_MAX_LENGTH   240
#define FAIL            0
#define SUCCESS         1
#define SWITCH_OFF      0
#define SWITCH_ON       1

/* Registers define */
#define GTP_READ_COOR_ADDR  0x814E
#define GTP_REG_SLEEP       0x8040
#define GTP_REG_SENSOR_ID   0x814A
#define GTP_REG_CONFIG_DATA 0x8047
#define GTP_REG_FW_VERSION  0x8144
#define GTP_REG_PRODUCT_ID  0x8140

#define GTP_I2C_RETRY_3     3
#define GTP_I2C_RETRY_5     5
#define GTP_I2C_RETRY_10    10

#define RESOLUTION_LOC      3
#define TRIGGER_LOC         8

/* HIGH: 0x28/0x29, LOW: 0xBA/0xBB */
#define GTP_I2C_ADDRESS_HIGH    0x14
#define GTP_I2C_ADDRESS_LOW     0x5D

#define CFG_GROUP_LEN(p_cfg_grp) (sizeof(p_cfg_grp) / sizeof(p_cfg_grp[0]))

/* GTP CM_HEAD RW flags */
#define GTP_RW_READ                 0
#define GTP_RW_WRITE                1
#define GTP_RW_READ_IC_TYPE         2
#define GTP_RW_WRITE_IC_TYPE        3
#define GTP_RW_FILL_INFO            4
#define GTP_RW_NO_WRITE             5
#define GTP_RW_READ_ERROR           6
#define GTP_RW_DISABLE_IRQ          7
#define GTP_RW_READ_VERSION         8
#define GTP_RW_ENABLE_IRQ           9
#define GTP_RW_ENTER_UPDATE_MODE    11
#define GTP_RW_LEAVE_UPDATE_MODE    13
#define GTP_RW_UPDATE_FW            15
#define GTP_RW_CHECK_RAWDIFF_MODE   17

/* GTP need flag or interrupt */
#define GTP_NO_NEED             0
#define GTP_NEED_FLAG           1
#define GTP_NEED_INTERRUPT      2

/*****************************End of Part III********************************/

/* cfg group0 data */
unsigned char cfg_group0_data[] = {
0x00 ,0x58 ,0x00 ,0x00 ,0x04 ,0x0A ,0x04 ,0x00 ,0x02 ,0x0B ,
0x14 ,0x0F ,0x64 ,0x50 ,0x03 ,0x05 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00 ,0x8B ,0x2B ,0x0D ,
0x3C ,0x36 ,0x05 ,0x0D ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x2D ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x64 ,0x32 ,0x00 ,0x00 ,
0x00 ,0x5A ,0x96 ,0x94 ,0xC5 ,0x02 ,0x08 ,0x00 ,0x00 ,0x05 ,
0x0D ,0x38 ,0xC1 ,0x0F ,0x34 ,0x89 ,0x10 ,0x36 ,0x51 ,0x11 ,
0x3A ,0x19 ,0x12 ,0x3D ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x02 ,0x04 ,0x06 ,0x08 ,0x0A ,0x0C ,0x0E ,0x10 ,
0x12 ,0x14 ,0x16 ,0x18 ,0x1A ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0x00 ,0x02 ,0x04 ,0x06 ,0x08 ,0x0A ,0x0C ,0x0F ,
0x10 ,0x12 ,0x13 ,0x16 ,0x18 ,0x1C ,0x1D ,0x1E ,0x1F ,0x20 ,
0x21 ,0x22 ,0x24 ,0x26 ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xD0 ,0x01
};

unsigned char cfg_group1_data[] = {
0x00 ,0x58 ,0x00 ,0x00 ,0x04 ,0x0A ,0x04 ,0x00 ,0x02 ,0x0B ,
0x14 ,0x0F ,0x64 ,0x50 ,0x03 ,0x05 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00 ,0x8B ,0x2B ,0x0D ,
0x3C ,0x36 ,0x05 ,0x0D ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x2D ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x64 ,0x32 ,0x00 ,0x00 ,
0x00 ,0x5A ,0x96 ,0x94 ,0xC5 ,0x02 ,0x08 ,0x00 ,0x00 ,0x05 ,
0x0D ,0x38 ,0xC1 ,0x0F ,0x34 ,0x89 ,0x10 ,0x36 ,0x51 ,0x11 ,
0x3A ,0x19 ,0x12 ,0x3D ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x02 ,0x04 ,0x06 ,0x08 ,0x0A ,0x0C ,0x0E ,0x10 ,
0x12 ,0x14 ,0x16 ,0x18 ,0x1A ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0x00 ,0x02 ,0x04 ,0x06 ,0x08 ,0x0A ,0x0C ,0x0F ,
0x10 ,0x12 ,0x13 ,0x16 ,0x18 ,0x1C ,0x1D ,0x1E ,0x1F ,0x20 ,
0x21 ,0x22 ,0x24 ,0x26 ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xD0 ,0x01
};

unsigned char cfg_group2_data[] = {
0x00 ,0x58 ,0x00 ,0x00 ,0x04 ,0x0A ,0x04 ,0x00 ,0x02 ,0x0B ,
0x14 ,0x0F ,0x64 ,0x50 ,0x03 ,0x05 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00 ,0x8B ,0x2B ,0x0D ,
0x3C ,0x36 ,0x05 ,0x0D ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x2D ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x64 ,0x32 ,0x00 ,0x00 ,
0x00 ,0x5A ,0x96 ,0x94 ,0xC5 ,0x02 ,0x08 ,0x00 ,0x00 ,0x05 ,
0x0D ,0x38 ,0xC1 ,0x0F ,0x34 ,0x89 ,0x10 ,0x36 ,0x51 ,0x11 ,
0x3A ,0x19 ,0x12 ,0x3D ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x02 ,0x04 ,0x06 ,0x08 ,0x0A ,0x0C ,0x0E ,0x10 ,
0x12 ,0x14 ,0x16 ,0x18 ,0x1A ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0x00 ,0x02 ,0x04 ,0x06 ,0x08 ,0x0A ,0x0C ,0x0F ,
0x10 ,0x12 ,0x13 ,0x16 ,0x18 ,0x1C ,0x1D ,0x1E ,0x1F ,0x20 ,
0x21 ,0x22 ,0x24 ,0x26 ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xD0 ,0x01
};

unsigned char cfg_group3_data[] = {
0x00 ,0x58 ,0x00 ,0x00 ,0x04 ,0x0A ,0x04 ,0x00 ,0x02 ,0x0B ,
0x14 ,0x0F ,0x64 ,0x50 ,0x03 ,0x05 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00 ,0x8B ,0x2B ,0x0D ,
0x3C ,0x36 ,0x05 ,0x0D ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x2D ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x64 ,0x32 ,0x00 ,0x00 ,
0x00 ,0x5A ,0x96 ,0x94 ,0xC5 ,0x02 ,0x08 ,0x00 ,0x00 ,0x05 ,
0x0D ,0x38 ,0xC1 ,0x0F ,0x34 ,0x89 ,0x10 ,0x36 ,0x51 ,0x11 ,
0x3A ,0x19 ,0x12 ,0x3D ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x02 ,0x04 ,0x06 ,0x08 ,0x0A ,0x0C ,0x0E ,0x10 ,
0x12 ,0x14 ,0x16 ,0x18 ,0x1A ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0x00 ,0x02 ,0x04 ,0x06 ,0x08 ,0x0A ,0x0C ,0x0F ,
0x10 ,0x12 ,0x13 ,0x16 ,0x18 ,0x1C ,0x1D ,0x1E ,0x1F ,0x20 ,
0x21 ,0x22 ,0x24 ,0x26 ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xD0 ,0x01
};
unsigned char cfg_group4_data[] = {
0x00 ,0x58 ,0x00 ,0x00 ,0x04 ,0x0A ,0x04 ,0x00 ,0x02 ,0x0B ,
0x14 ,0x0F ,0x64 ,0x50 ,0x03 ,0x05 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00 ,0x8B ,0x2B ,0x0D ,
0x3C ,0x36 ,0x05 ,0x0D ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x2D ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x64 ,0x32 ,0x00 ,0x00 ,
0x00 ,0x5A ,0x96 ,0x94 ,0xC5 ,0x02 ,0x08 ,0x00 ,0x00 ,0x05 ,
0x0D ,0x38 ,0xC1 ,0x0F ,0x34 ,0x89 ,0x10 ,0x36 ,0x51 ,0x11 ,
0x3A ,0x19 ,0x12 ,0x3D ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x02 ,0x04 ,0x06 ,0x08 ,0x0A ,0x0C ,0x0E ,0x10 ,
0x12 ,0x14 ,0x16 ,0x18 ,0x1A ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0x00 ,0x02 ,0x04 ,0x06 ,0x08 ,0x0A ,0x0C ,0x0F ,
0x10 ,0x12 ,0x13 ,0x16 ,0x18 ,0x1C ,0x1D ,0x1E ,0x1F ,0x20 ,
0x21 ,0x22 ,0x24 ,0x26 ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xD0 ,0x01
};
unsigned char cfg_group5_data[] = {
0x00 ,0x58 ,0x00 ,0x00 ,0x04 ,0x0A ,0x04 ,0x00 ,0x02 ,0x0B ,
0x14 ,0x0F ,0x64 ,0x50 ,0x03 ,0x05 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00 ,0x8B ,0x2B ,0x0D ,
0x3C ,0x36 ,0x05 ,0x0D ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x2D ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x64 ,0x32 ,0x00 ,0x00 ,
0x00 ,0x5A ,0x96 ,0x94 ,0xC5 ,0x02 ,0x08 ,0x00 ,0x00 ,0x05 ,
0x0D ,0x38 ,0xC1 ,0x0F ,0x34 ,0x89 ,0x10 ,0x36 ,0x51 ,0x11 ,
0x3A ,0x19 ,0x12 ,0x3D ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
0x00 ,0x00 ,0x02 ,0x04 ,0x06 ,0x08 ,0x0A ,0x0C ,0x0E ,0x10 ,
0x12 ,0x14 ,0x16 ,0x18 ,0x1A ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0x00 ,0x02 ,0x04 ,0x06 ,0x08 ,0x0A ,0x0C ,0x0F ,
0x10 ,0x12 ,0x13 ,0x16 ,0x18 ,0x1C ,0x1D ,0x1E ,0x1F ,0x20 ,
0x21 ,0x22 ,0x24 ,0x26 ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,0xFF ,
0xFF ,0xFF ,0xFF ,0xFF ,0xD0 ,0x01
};


#define GT_WRITE(_reg, _val)    (*((volatile uint32_t*)(_reg)) = (_val))
#define GT_READ(_reg)       (*((volatile uint32_t*)(_reg)))

#define mdelay(n) ({unsigned long msec=(n); while (msec--) udelay(1000);})

#define TEST_GPIO   0
#define DEBUG 1
#ifdef DEBUG
#define SHAKE_COUNT     (GTP_REG_CONFIG_DATA + 10)
#define TOUCH_LEVEL     (GTP_REG_CONFIG_DATA + 14)
#define LEAVE_LEVEL     (GTP_REG_CONFIG_DATA + 15)
#define REFRESH_RATE        (GTP_REG_CONFIG_DATA + 17)

static int gtp_i2c_config_read(unsigned int addr)
{
    u8 buf[7] = { addr >> 8, addr & 0xff };
    int retry = GTP_I2C_RETRY_5;
    int ret = -1;
    int i=0;
    while (retry--)
    {
        ret = i2c_read(GTP_I2C_ADDRESS_LOW, addr, 2, &buf[2], 5);
        if (ret == 0)
        {
            return ret;
        }
        printf("GTP i2c test failed time %d.\n", retry);
        mdelay(20);
    }
    return ret;
}


static void gtp_i2c_config_reads()
{
    gtp_i2c_config_read(SHAKE_COUNT);
    gtp_i2c_config_read(TOUCH_LEVEL);
    gtp_i2c_config_read(LEAVE_LEVEL);
    gtp_i2c_config_read(REFRESH_RATE);
}

#endif

void gtp_gpio_set_value(unsigned gpio, int value)
{
    unsigned val, idx, offset;
    offset = gpio % 32;
    idx  = gpio / 32;
    val = GT_READ(0xF00000E0 + (4 * idx));
    val = (value == 1) ? (val | (1U << offset)) : (val & ~(1U << offset));
    GT_WRITE(0xF00000E0 + (4 * idx), val);
}

int gtp_gpio_get_value(unsigned gpio)
{
    unsigned val, idx, offset;
    offset = gpio % 32;
    idx  = gpio / 32;

    val = GT_READ(0xF0000100 + (4 * idx));

    return ((val & (1U << offset)) ? 1 : 0);
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

int gtp_gpio_direction_input(unsigned gpio)
{
    gtp_gpio_inout_sel(gpio, 0);   //INPUT = 1
    return gtp_gpio_get_value(gpio);
}

int get_touch_status()
{
    u8 end_cmd[3] = { GTP_READ_COOR_ADDR >> 8,
            GTP_READ_COOR_ADDR & 0xFF, 0};
    u8 point_data[2 + 1 + 8 * GTP_MAX_TOUCH + 1] = {
            GTP_READ_COOR_ADDR >> 8,
            GTP_READ_COOR_ADDR & 0xFF};
    u8 touch_num = 0;
    u8 finger = 0;
    static u16 pre_touch;
    static u8 pre_key;

    u8 key_value = 0;
    u8 *coor_data = NULL;
    s32 input_x = 0;
    s32 input_y = 0;
    s32 input_w = 0;
    s32 id = 0;
    s32 i = 0;
    int ret = -1;
    int value = 0;
    struct goodix_ts_data *ts = NULL;

    ret = i2c_read(GTP_I2C_ADDRESS_LOW, GTP_READ_COOR_ADDR, 2, &point_data[2], 10);
    if (ret < 0)
    {
        printf("I2C transfer error. errno:%d\n ", ret);
        return value;
    }
    finger = point_data[GTP_ADDR_LENGTH];
    touch_num = finger & 0x0f;
    if (touch_num > GTP_MAX_TOUCH)
        return value;
    if(touch_num > 0)
    {
         value = touch_num;
         
    }
    else
    {
        value = 0;
        goto exit_fun;
    }

    exit_fun:
        i2c_write(GTP_I2C_ADDRESS_LOW, GTP_READ_COOR_ADDR, 2,
            &end_cmd[2], 1);
    return value;
}

int get_touch_status_pos_xy(unsigned int *position_x,unsigned int *position_y)
{
    u8 end_cmd[3] = { GTP_READ_COOR_ADDR >> 8,
            GTP_READ_COOR_ADDR & 0xFF, 0};
    u8 point_data[2 + 1 + 8 * GTP_MAX_TOUCH + 1] = {
            GTP_READ_COOR_ADDR >> 8,
            GTP_READ_COOR_ADDR & 0xFF};
    u8 touch_num = 0;
    u8 finger = 0;
    static u16 pre_touch;
    static u8 pre_key;

    u8 key_value = 0;
    u8 *coor_data = NULL;
    s32 input_x = 0;
    s32 input_y = 0;
    s32 input_w = 0;
    s32 id = 0;
    s32 i = 0;
    s32 pos = 0;
    int ret = -1;
    int value = 0;
    struct goodix_ts_data *ts = NULL;

    ret = i2c_read(GTP_I2C_ADDRESS_LOW, GTP_READ_COOR_ADDR, 2, &point_data[2], 10);
    if (ret < 0)
    {
        printf("I2C transfer error. errno:%d\n ", ret);
        return value;
    }
    finger = point_data[GTP_ADDR_LENGTH];
    touch_num = finger & 0x0f;
    printf("touch_num:%d\n",touch_num);
    if (touch_num > GTP_MAX_TOUCH)
        return value;
    if(touch_num > 0)
    {
         value = 1;
         coor_data = &point_data[3];
         input_x= coor_data[pos + 1] |
                        coor_data[pos + 2] << 8;
         input_y = coor_data[pos + 3] |
                        coor_data[pos + 4] << 8;
         *position_x = input_x;
         *position_y = input_y;
         printf("point_position_x=%d    point_position_y=%d\n",*position_x,*position_y);
    }
    else
    {
        value = 0;
        goto exit_fun;
    }

    exit_fun:
        i2c_write(GTP_I2C_ADDRESS_LOW, GTP_READ_COOR_ADDR, 2,
            &end_cmd[2], 1);
    return value;
}


/*******************************************************
Function:
    i2c read twice, compare the results
Input:
    client:  i2c device
    addr:    operate address
    rxbuf:   read data to store, if compare successful
    len:     bytes to read
Output:
    FAIL:    read failed
    SUCCESS: read successful
*********************************************************/
int gtp_i2c_read_dbl_check(u16 addr, u8 *rxbuf, int len)
{
    u8 buf[16] = {0};
    u8 confirm_buf[16] = {0};
    u8 retry = 0;
    int i = 0;
       //printf("gtp_i2c_read_dbl_check\n");
    while (retry++ < GTP_I2C_RETRY_3)
    {
        memset(buf, 0xAA, 16);
        buf[0] = (u8)(addr >> 8);
        buf[1] = (u8)(addr & 0xFF);
             i2c_read(GTP_I2C_ADDRESS_LOW, addr, 2, &buf[2], 1);
        memset(confirm_buf, 0xAB, 16);
        confirm_buf[0] = (u8)(addr >> 8);
        confirm_buf[1] = (u8)(addr & 0xFF);
        i2c_read(GTP_I2C_ADDRESS_LOW, addr, 2, &confirm_buf[2], 1);
        if (!memcmp(buf, confirm_buf, len + 2))
            break;
    }
    if (retry < GTP_I2C_RETRY_3)
    {
        memcpy(rxbuf, confirm_buf + 2, len);
        return SUCCESS;
    } 
    else
    {
        printf("i2c read 0x%04X, %d bytes, double check failed!\n",
            addr, len);
        return FAIL;
    }
}

/*******************************************************
Function:
    Send config data.
Input:
    client: i2c device.
Output:
    result of i2c write operation.
    > 0: succeed, otherwise: failed
*********************************************************/
int gtp_send_cfg(unsigned char *config_data)
{
    int ret;
    int retry = 0;
    
    for (retry = 0; retry < GTP_I2C_RETRY_3; retry++)
    {
        ret = i2c_write(GTP_I2C_ADDRESS_LOW, 0x8047, 2,
            config_data,
            GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH);
        if (ret == 0)
            break;
    }
    mdelay(10);
    
    //gtp_i2c_config_reads();
    return ret;
}

/*******************************************************
Function:
    Synchronization.
Input:
    ms: synchronization time in millisecond.
Output:
    None.
*******************************************************/
void gtp_int_sync()
{
    gtp_gpio_direction_output(INT_GPIO, 0);
    mdelay(100);
    gtp_gpio_direction_input(INT_GPIO);
}

void reset_panel()
{
    int ret = -1000;
    /* This reset sequence will selcet I2C slave address */
    gtp_gpio_inout_sel(INT_FUNCTION, 1);
    gtp_gpio_inout_sel(RESET_PORT, 1);
    ret  = gtp_gpio_direction_output(RESET_PORT, 0);
    mdelay(20);
    //debug
    ret  = gtp_gpio_direction_output(INT_GPIO, 0);
    printf("INT_GPIO:%d\n",INT_GPIO);
    //usleep(RESET_DELAY_T3_US);
    mdelay(20);
    ret  = gtp_gpio_direction_output(RESET_PORT, 1);
    mdelay(20);

    gtp_int_sync();
    
}
int init_panel()
{
    //struct i2c_client *client = ts->client;
    int ret = -1;
    int i;
    u8 check_sum = 0;
    u8 opr_buf[16];
    u8 sensor_id = 0;
        u8 config_data_len[] = { 
                       CFG_GROUP_LEN(cfg_group0_data),
                       CFG_GROUP_LEN(cfg_group1_data),
                       CFG_GROUP_LEN(cfg_group2_data),
                       CFG_GROUP_LEN(cfg_group3_data),
                       CFG_GROUP_LEN(cfg_group4_data),
                       CFG_GROUP_LEN(cfg_group5_data),
                       };
                             
        u8 *config_data[] = {
                         cfg_group0_data, 
                         cfg_group1_data, 
                         cfg_group2_data, 
                         cfg_group3_data, 
                         cfg_group4_data, 
                         cfg_group5_data, 
                      };
    ret = gtp_i2c_read_dbl_check(0x41E4, opr_buf, 1);
    if (SUCCESS == ret)
    {
        if (opr_buf[0] != 0xBE) 
        {
            //ts->fw_error = 1;
            printf("Firmware error, no config sent!\n");
            return -1;
        }
    }

    for (i = 1; i < GOODIX_MAX_CFG_GROUP; i++)
    {
        if (config_data_len[i])
            break;
    }
    if (i == GOODIX_MAX_CFG_GROUP)
    {
        sensor_id = 0;
    } 
    else
    {
        ret = gtp_i2c_read_dbl_check(GTP_REG_SENSOR_ID,
            &sensor_id, 1);
        if (SUCCESS == ret)
        {
            if (sensor_id >= GOODIX_MAX_CFG_GROUP)
            {
                printf("Invalid sensor_id(0x%02X), No Config Sent!\n",
                    sensor_id);
                return -1;
            }
        }
        else
        {
            printf("Failed to get sensor_id, No config sent!\n");
            return -1;
        }
    }

    printf("Sensor ID selected: %d\n", sensor_id);
   
    ret = gtp_i2c_read_dbl_check(GTP_REG_CONFIG_DATA,
        &opr_buf[0], 1);
    if (ret == SUCCESS)
    {
        if (opr_buf[0] < 90)
        {
            /* backup group config version */
            
            config_data[sensor_id][GTP_ADDR_LENGTH] = 0x00;
            
        } 
        else
        {
            /* treated as fixed config, not send config */
            printf("Ic fixed config with config version(%d, 0x%02X)\n",
                opr_buf[0], opr_buf[0]);
            //ts->fixed_cfg = 1;
        }
    } 
    else
    {
        printf("Failed to get ic config version!No config sent!\n");
        return -1;
    }
    
    check_sum = 0;
    for (i = GTP_ADDR_LENGTH; i < config_data_len[sensor_id]; i++)
        check_sum += config_data[sensor_id][i];
    
    config_data[config_data_len[sensor_id]] = (~check_sum) + 1;
    
    ret = gtp_send_cfg(config_data[sensor_id]);
    if (ret < 0)
        printf("%s: Send config error.\n", __func__);
    return ret;
}

int check_ctouch_pressed()
{
    unsigned int u4Rt = 0;
    unsigned int ret =0;
    int Pressed = 0;
    unsigned int adctouch_count = 0;
    unsigned int confirmed_count = 0;
#define  RETRY_CNT 10
    reset_panel();
    init_panel();
    while(1)
    {
        mdelay(20);
        u4Rt = get_touch_status();
        Pressed = (u4Rt)?1:0;

        if(0 == Pressed)
        {
            confirmed_count++;
           if(confirmed_count > 10)
            {
                 printf("[Recovery Mode] No key is pressed!\n");
                 ret  = 0;     
                 break;
            }
            
            
        }
        if(1 == Pressed)
        {
            adctouch_count++;
            if (adctouch_count > RETRY_CNT)
            {
                /* Assume that key is pressed */
                 printf("[Recovery Mode] Key is pressed to enter into recovery mode!\n");
                ret  = 1;   
                confirmed_count = 0;
                break;
            }
        }
    }
    return ret;
}

int check_ctouch_pressed_th_num()
{
    unsigned int u4Rt = 0;
    unsigned int ret =0;
    int Pressed = 0;
    unsigned int adctouch_count = 0;
    unsigned int confirmed_count = 0;
    unsigned int point_position_x =0 ;
    unsigned int point_position_y = 0;
#define  RETRY_CNT 20
    reset_panel();
    init_panel();
  
    while(1)
    {
        mdelay(20);
        u4Rt = get_touch_status();
        Pressed = (u4Rt)?1:0;

        if(0 == Pressed)
        {
            confirmed_count++;
           if(confirmed_count > 10)
            {
                 printf("[Recovery Mode] No key is pressed!\n");
                 ret  = 0;     
                 break;
            }
            
            
        }
        if(1 == Pressed)
        {
            adctouch_count++;
            if (adctouch_count > RETRY_CNT)
            {
                /* Assume that key is pressed */
                 printf("[Recovery Mode] Key is pressed to enter into recovery mode!\n");
                ret  = 1;   
                confirmed_count = 0;
                break;
            }
        }
    }
    return ret;
}
int check_ctouch_pressed_pos_xy(unsigned int *position_x,unsigned int *position_y)
{
    unsigned int u4Rt = 0;
    unsigned int ret =0;
    int Pressed = 0;
    unsigned int adctouch_count = 0;
    unsigned int confirmed_count = 0;
    unsigned int point_position_x =0 ;
    unsigned int point_position_y = 0;
#define  RETRY_CNT 10
    reset_panel();
    init_panel();
    if(position_x ==NULL||position_y == NULL)
    {
        return 0;
    }
    while(1)
    {
        mdelay(20);
        u4Rt = get_touch_status_pos_xy(&point_position_x,&point_position_y);
        
        *position_x = point_position_x;
        *position_y = point_position_y;
         printf("point_x = %d  point_y = %d \n",*position_x,*position_y);
      
        point_position_x = 0;
        point_position_y = 0;
        Pressed = (u4Rt)?1:0;

        if(0 == Pressed)
        {
            confirmed_count++;
           if(confirmed_count > 10)
            {
                 printf("[Recovery Mode] No key is pressed!\n");
                 ret  = 0;     
                 break;
            }
            
            
        }
        if(1 == Pressed)
        {
            adctouch_count++;
            if (adctouch_count > RETRY_CNT)
            {
                /* Assume that key is pressed */
                 printf("[Recovery Mode] Key is pressed to enter into recovery mode!\n");
                ret  = 1;   
                confirmed_count = 0;
                break;
            }
        }
    }
    return ret;
}
