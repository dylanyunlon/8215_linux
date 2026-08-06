#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/gpio/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/clk-private.h>
#include "ac823x_spi.h"
#include "ac823x_spi_sw.h"
#include "x_typedef.h"
#include "x_hal_ic.h"
#include <linux/printk.h>
#include <linux/gpio/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/printk.h>
#include "x_ver.h"
#include "x_os.h"

#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <generated/atc_project.h>

ulong SPI0_VBASE_ADDR = 0;
ulong SPI1_VBASE_ADDR = 0;


#define MODEBITS (SPI_CPOL|SPI_CPHA|SPI_CS_HIGH)
/***** version info *****/
#define SPI_VER_NAME    "SPI"
#define SPI_VER_MAIN     01
#define SPI_VER_MINOR    00
#define SPI_VER_REV      00

#define TAG	"[SPI]"
#define pr_fmt(fmt) TAG fmt

#define SPI_DBG(fmt, ...) 					\
	({							\
		pr_debug(fmt, ##__VA_ARGS__); \
	})

#define SPI_ERR(fmt, ...) 					\
	({							\
		pr_err("file name is %s ,function at %s, line %d", FILE_ONLY,__func__, __LINE__); \
		pr_err(fmt, ##__VA_ARGS__); \
	})


static int spi_nr = 0;
struct gpio_desc *pSpi0Gpio1, *pSpi0Gpio2, *pSpi0Gpio3, *pSpi0Gpio4;
struct gpio_desc *pSpi1Gpio1, *pSpi1Gpio2, *pSpi1Gpio3, *pSpi1Gpio4; 
struct pinctrl *pinctrl_spi0;
struct pinctrl *pinctrl_spi1;
struct pinctrl_state *pins_spi0_st1, *pins_spi0_st2;
struct pinctrl_state *pins_spi0_st3, *pins_spi0_st4;
struct pinctrl_state *pins_spi0_st5, *pins_spi0_st6;
struct pinctrl_state *pins_spi0_st7, *pins_spi0_st8;
struct pinctrl_state *pins_spi1_st1, *pins_spi1_st2;
struct pinctrl_state *pins_spi1_st3, *pins_spi1_st4;
struct pinctrl_state *pins_spi1_st5, *pins_spi1_st6;
struct pinctrl_state *pins_spi1_st7, *pins_spi1_st8;

struct clk* clk_ac8317_spi[2] ;
struct clk* clk_ac8317_spi_select[2];
struct clk* clk_ac8317_spi_parent[2];


struct ac83xx_spi {
	struct completion  done;

	void *regs;
	int   irq;
	int   len;
	int   count;

	void  (*set_cs)(struct ac83xx_spi_info *spi, int cs, int pol);

	const unsigned char *tx;
	unsigned char       *rx;

	struct clk *clk;
	struct resource   *ioarea;
	struct spi_master *master;
	struct spi_device *curdev;
	struct device     *dev;
	struct ac83xx_spi_info *pdata;
	struct resource *res;
	u8			*dmaTXbuffer;
	u8			*dmaRXbuffer;
    u32         dmaTXphyAddr;
    u32         dmaRXphyAddr;
};

u32 SPI_IsFinished(void)
{
	u32 u4Counter;

	u4Counter = 0;
	while (u4Counter < SPI_MOTIO_OPERATION_TIMEOUT) {
		if (SPI_MOTO_READ32(SPI_STATUS0)&SPI_STATUS1_FINISH_MASK) {
			return SPI_MOTO_OK;
		}
		u4Counter++;
	}

	return SPI_MOTO_TIMEOUT;
}

static int spi_dma_init(struct ac83xx_spi *hw)
{
    
    if (hw->master->bus_num == 0) 
    {
        hw->dmaTXbuffer = dma_alloc_coherent(hw->dev, 
            SPI_DMA_BUF_SIZE , 
            &hw->dmaTXphyAddr, 
            GFP_KERNEL);    
        if (!hw->dmaTXbuffer) {
            printk("Alloc DMA buffer fail\n");
            return 0;
        }
        memset(hw->dmaTXbuffer,0,SPI_DMA_BUF_SIZE);
        hw->dmaRXbuffer = dma_alloc_coherent(hw->dev, 
            SPI_DMA_BUF_SIZE , 
            &hw->dmaRXphyAddr, 
            GFP_KERNEL);    
        if (!hw->dmaRXbuffer ) {
            printk("Alloc DMA buffer fail\n");
            return 0;
        }
        memset(hw->dmaRXbuffer,0,SPI_DMA_BUF_SIZE );

    }

    if (hw->master->bus_num == 1) 
    {
        hw->dmaTXbuffer = dma_alloc_coherent(hw->dev, 
            SPI_DMA_BUF_SIZE , 
            &hw->dmaTXphyAddr, 
            GFP_KERNEL);    
        if (!hw->dmaTXbuffer) {
            printk("Alloc DMA buffer fail\n");
            return 0;
        }
        memset(hw->dmaTXbuffer,0,SPI_DMA_BUF_SIZE );

        hw->dmaRXbuffer = dma_alloc_coherent(hw->dev, 
            SPI_DMA_BUF_SIZE , 
            &hw->dmaRXphyAddr, 
            GFP_KERNEL);    
        if (!hw->dmaRXbuffer ) {
            printk("Alloc DMA buffer\n");
            return 0;
        }
        memset(hw->dmaRXbuffer,0,SPI_DMA_BUF_SIZE );
    }
    return 0;

}
static int spi_init(struct ac83xx_spi *sdd)
{
	u32 u4Tmp = 0;
	int ret = 0;
	u32 data = 0;

	if (sdd->master->bus_num == 0) {
        u4Tmp = SPI_IO_READ32(MISC_CONTROL);
        u4Tmp &= ~SPI_SEL_MASK;
        u4Tmp |= SPI_SEL_MOTO1_MOTO2;
        SPI_IO_WRITE32(MISC_CONTROL, u4Tmp);
		
		clk_ac8317_spi_parent[0] = clk_get(NULL,"clk27m_ck");
		ret = clk_set_parent(clk_ac8317_spi[0],clk_ac8317_spi_parent[0]);
		clk_ac8317_spi_parent[0] = clk_get_parent(clk_ac8317_spi[0]);
		if(clk_ac8317_spi_parent[0] == NULL)    {
			SPI_ERR("get spi0 clk parent error!  ;%d \n",ret);
		}
		
		clk_prepare(clk_ac8317_spi_select[0]);
		clk_enable(clk_ac8317_spi_select[0]); 
		#if defined (CONFIG_ATC_PRJ_ac823x_adas)
		clk_disable(clk_ac8317_spi_select[0]);
      	clk_unprepare(clk_ac8317_spi_select[0]);
		//u4Tmp = SPI_IO_READ32(SPI_MOTO_CLOCK);
        //u4Tmp &= ~SPI_MOTO1_CLOCK;
        //SPI_IO_WRITE32(SPI_MOTO_CLOCK, u4Tmp);
		#endif
		
		u4Tmp = SPI_IO_READ32(SPI_MOTO_RESET);
		u4Tmp |= SPI_MOTO1_RESET;
		SPI_IO_WRITE32(SPI_MOTO_RESET, u4Tmp);  
		
		pinctrl_spi0 = devm_pinctrl_get(sdd->dev);
		if (IS_ERR(pinctrl_spi0)) {
			SPI_ERR("spi0 get pinctrl failed!");
		}

		/* gpio_request(PIN_119_SP0_CLK,"PIN_119_SP0_CLK"); */
		pSpi0Gpio1 = __gpiod_get(sdd->dev, "spi01", GPIOD_ASIS);
		if (IS_ERR(pSpi0Gpio1)) {
			SPI_ERR("get gpio spi01 failed");
		}
		/* GPIO_MultiFun_Set(PIN_119_SP0_CLK, SP0_SEL); */
		pins_spi0_st1 = pinctrl_lookup_state(pinctrl_spi0, "spi0_state_1");
		if (IS_ERR(pins_spi0_st1)) {
			SPI_ERR("lookup spi0_state_1 failed");
		}
		ret = pinctrl_select_state(pinctrl_spi0, pins_spi0_st1);
		if (ret) {
			SPI_ERR("select spi0_state_1 failed");
		}

		/* gpio_request(PIN_120_SP0_CS,"PIN_120_SP0_CS"); */
		pSpi0Gpio2 = __gpiod_get(sdd->dev, "spi02", GPIOD_ASIS);
		if (IS_ERR(pSpi0Gpio2)) {
			SPI_ERR("get gpio spi02 failed");
		}
		/* GPIO_MultiFun_Set(PIN_120_SP0_CS,   SP0_SEL); */
		pins_spi0_st3 = pinctrl_lookup_state(pinctrl_spi0, "spi0_state_3");
		if (IS_ERR(pins_spi0_st3)) {
			SPI_ERR("lookup spi0_state_3 failed");
		}
		ret = pinctrl_select_state(pinctrl_spi0, pins_spi0_st3);
		if (ret) {
			SPI_ERR("select spi0_state_3 failed");
		}

		/* gpio_request(PIN_121_SP0_SI,"PIN_121_SP0_SI"); */
		pSpi0Gpio3 = __gpiod_get(sdd->dev, "spi03", GPIOD_ASIS);
		if (IS_ERR(pSpi0Gpio3)) {
			SPI_ERR("get gpio spi03 failed");
		}
		/* GPIO_MultiFun_Set(PIN_121_SP0_SI,    SP0_SEL); */
		pins_spi0_st5 = pinctrl_lookup_state(pinctrl_spi0, "spi0_state_5");
		if (IS_ERR(pins_spi0_st5)) {
			SPI_ERR("lookup spi0_state_5 failed");
		}
		ret = pinctrl_select_state(pinctrl_spi0, pins_spi0_st5);
		if (ret) {
			SPI_ERR("select spi0_state_5 failed");
		}

		/* gpio_request(PIN_122_SP0_SO,"PIN_122_SP0_SO"); */
		pSpi0Gpio4 = __gpiod_get(sdd->dev, "spi04", GPIOD_ASIS);
		if (IS_ERR(pSpi0Gpio4)) {
			SPI_ERR("get gpio spi04 failed");
		}        
		/* GPIO_MultiFun_Set(PIN_122_SP0_SO,   SP0_SEL); */
		pins_spi0_st7 = pinctrl_lookup_state(pinctrl_spi0, "spi0_state_7");
		if (IS_ERR(pins_spi0_st7)) {
			SPI_ERR("lookup spi0_state_7 failed");
		}
		ret = pinctrl_select_state(pinctrl_spi0, pins_spi0_st7);
		if (ret) {
			SPI_ERR("select spi0_state_7 failed");
		}
		
        SPI_MOTO_WRITE32(SPI_CFG0, SPI_CF0_DEFAULT_VALUE);
        SPI_MOTO_WRITE32(SPI_CFG1,SPI_CF1_DEFAULT_VALUE);
	}
	if (sdd->master->bus_num == 1) {
        u4Tmp = SPI_IO_READ32(MISC_CONTROL);
        u4Tmp &= ~SPI_SEL_MASK;
        u4Tmp |= SPI_SEL_MOTO1_MOTO2;
        SPI_IO_WRITE32(MISC_CONTROL, u4Tmp);

		u4Tmp = SPI_IO_READ32(SPI_MOTO_RESET);
		u4Tmp |= SPI_MOTO2_RESET;
		SPI_IO_WRITE32(SPI_MOTO_RESET, u4Tmp);

		pinctrl_spi1 = devm_pinctrl_get(sdd->dev);
		if (IS_ERR(pinctrl_spi1)) {
			SPI_ERR("spi1 get pinctrl failed!");
		}

		/* gpio_request(PIN_123_SP1_CLK,"PIN_123_SP1_CLK"); */
		pSpi1Gpio1 = __gpiod_get(sdd->dev, "spi11", GPIOD_ASIS);
		if (IS_ERR(pSpi1Gpio1)) {
			SPI_ERR("get gpio spi11 failed");
		}
		/* GPIO_MultiFun_Set(PIN_123_SP1_CLK, SP1_SEL); */
		pins_spi1_st1 = pinctrl_lookup_state(pinctrl_spi1, "spi1_state_1");
		if (IS_ERR(pins_spi1_st1)) {
			SPI_ERR("lookup spi1_state_1 failed");
		}
		ret = pinctrl_select_state(pinctrl_spi1, pins_spi1_st1);
		if (ret) {
			SPI_ERR("select spi1_state_1 failed");
		}

		/* gpio_request(PIN_126_SP1_CS,"PIN_126_SP1_CS"); */
		pSpi1Gpio2 = __gpiod_get(sdd->dev, "spi12", GPIOD_ASIS);
		if (IS_ERR(pSpi1Gpio2)) {
			SPI_ERR("get gpio spi12 failed");
		}
		/* GPIO_MultiFun_Set(PIN_126_SP1_CS,   SP1_SEL); */
		pins_spi1_st3 = pinctrl_lookup_state(pinctrl_spi1, "spi1_state_3");
		if (IS_ERR(pins_spi1_st3)) {
			SPI_ERR("lookup spi1_state_3 failed");
		}
		ret = pinctrl_select_state(pinctrl_spi1, pins_spi1_st3);
		if (ret) {
			SPI_ERR("select spi1_state_3 failed");
		}

		/* gpio_request(PIN_127_SP1_SI,"PIN_127_SP1_SI"); */
		pSpi1Gpio3 = __gpiod_get(sdd->dev, "spi13", GPIOD_ASIS);
		if (IS_ERR(pSpi1Gpio3)) {
			SPI_ERR("get gpio spi13 failed");
		}
		/* GPIO_MultiFun_Set(PIN_127_SP1_SI,    SP1_SEL); */
		pins_spi1_st5 = pinctrl_lookup_state(pinctrl_spi1, "spi1_state_5");
		if (IS_ERR(pins_spi1_st5)) {
			SPI_ERR("lookup spi1_state_5 failed");
		}
		ret = pinctrl_select_state(pinctrl_spi1, pins_spi1_st5);
		if (ret) {
			SPI_ERR("select spi1_state_5 failed");
		}

		/* gpio_request(PIN_128_SP1_SO,"PIN_128_SP1_SO"); */
		pSpi1Gpio4 = __gpiod_get(sdd->dev, "spi14", GPIOD_ASIS);
		if (IS_ERR(pSpi1Gpio4)) {
			SPI_ERR("get gpio spi14 failed");
		}
		/* GPIO_MultiFun_Set(PIN_128_SP1_SO,   SP1_SEL); */
		pins_spi1_st7 = pinctrl_lookup_state(pinctrl_spi1, "spi1_state_7");
		if (IS_ERR(pins_spi1_st7)) {
			SPI_ERR("lookup spi1_state_7 failed");
		}
		ret = pinctrl_select_state(pinctrl_spi1, pins_spi1_st7);
		if (ret) {
			SPI_ERR("select spi1_state_7 failed");
		}
		clk_ac8317_spi_parent[1] = clk_get(NULL,"clk27m_ck");
		ret = clk_set_parent(clk_ac8317_spi[0],clk_ac8317_spi_parent[1]);
		clk_ac8317_spi_parent[1] = clk_get_parent(clk_ac8317_spi[0]);
		if(clk_ac8317_spi_parent[1]  == NULL)  {
			SPI_ERR("get  SPI1 clk parent1 error! \n");
			return 0;
		}

		clk_prepare(clk_ac8317_spi_select[1]);
		clk_enable(clk_ac8317_spi_select[1]);   
		#if defined (CONFIG_ATC_PRJ_ac823x_adas)
		clk_disable(clk_ac8317_spi_select[1]);
      	clk_unprepare(clk_ac8317_spi_select[1]);
		#endif
        SPI_MOTO2_WRITE32(SPI_CFG0,SPI_CF0_DEFAULT_VALUE);
        SPI_MOTO2_WRITE32(SPI_CFG1,SPI_CF1_DEFAULT_VALUE);
	}

	return 0;
}

static inline struct ac83xx_spi *to_hw(struct spi_device *sdev)
{
	return spi_master_get_devdata(sdev->master);
}
static void ac83xx_spi_gpiocs(struct ac83xx_spi_info *spi, int cs, int pol)
{
	gpio_set_value(spi->pin_cs, pol);
}
u32 SPI_PopFifo(SPI_DIRECTION_TYPE const direction, u32 *data)
{
	if (data == NULL) {
		return 0;
	}

	switch (direction) {
	case SPI_TX:
		*data = SPI_MOTO_READ32(SPI_TX_DATA);
		break;
	case SPI_RX:
		*data = SPI_MOTO_READ32(SPI_RX_DATA);
		break;
	default:
		return 0;
	}

	return 1;
}

static void ac83xx_spi_chipsel(struct spi_device *spi)
{
	unsigned int spcon;

	if (spi->master->bus_num == 0) {
		spcon = SPI_MOTO_READ32(SPI_CMD);

		if (spi->mode&SPI_CPHA)
			spcon |= SPI_CMD_BIT_CPHA_MASK;
		else
			spcon &= ~SPI_CMD_BIT_CPHA_MASK;

		if (spi->mode&SPI_CPOL)
			spcon |= SPI_CMD_BIT_CPOL_MASK;
		else
			spcon &= ~SPI_CMD_BIT_CPOL_MASK;

		SPI_MOTO_WRITE32(SPI_CMD, spcon);
	}
	if (spi->master->bus_num == 1) {
		spcon = SPI_MOTO2_READ32(SPI_CMD);

		if (spi->mode&SPI_CPHA)
			spcon |= SPI_CMD_BIT_CPHA_MASK;
		else
			spcon &= ~SPI_CMD_BIT_CPHA_MASK;

		if (spi->mode&SPI_CPOL)
			spcon |= SPI_CMD_BIT_CPOL_MASK;
		else
			spcon &= ~SPI_CMD_BIT_CPOL_MASK;

		SPI_MOTO2_WRITE32(SPI_CMD, spcon);
	}
}
static int ac83xx_spi_setupxfer(struct spi_device *spi,
				struct spi_transfer *t)
{
	unsigned int bpw;
	unsigned int hz;

	bpw = t?t->bits_per_word:spi->bits_per_word;
	hz  = t?t->speed_hz:spi->max_speed_hz;

	if (bpw != 8 && bpw != 16 && bpw != 32) {
		dev_err(&spi->dev, "invalid bit-per-word(%d)\n", bpw);
		return -1;
	}

	ac83xx_spi_chipsel(spi);

	return 0;
}
static int ac83xx_spi_setup(struct spi_device *spi)
{
	int ret;

	if (!spi->bits_per_word)/* default value is 8 */
		spi->bits_per_word = 8;

	if (spi->mode&~MODEBITS) {
		dev_dbg(&spi->dev, "setup:unsuooort mode bit %x\n",
				spi->mode&~MODEBITS);
		return -1;
	}
	SPI_DBG("ac83xx_spi_set_up start\n");
	ret = ac83xx_spi_setupxfer(spi, NULL);
	if (ret < 0) {
		dev_err(&spi->dev, "setupxfer returned %d\n", ret);
		return ret;
	}
	dev_dbg(&spi->dev, "%d:mode, %u bpw, %d hz\n", spi->mode,
			spi->bits_per_word, spi->max_speed_hz);

	return 0;
}
void SPI_SetPauseMode(u32 const status)
{
	u32 value;

	value = SPI_MOTO_READ32(SPI_CMD);

	if (status == 1)
		SPI_MOTO_WRITE32(SPI_CMD, value|SPI_CMD_BIT_PAUSE_EN_MASK);
	else
		SPI_MOTO_WRITE32(SPI_CMD, value&(~SPI_CMD_BIT_PAUSE_EN_MASK));
}
void SPI2_SetPauseMode(u32 const status)
{
	u32 value;

	value = SPI_MOTO2_READ32(SPI_CMD);

	if (status == 1)
		SPI_MOTO2_WRITE32(SPI_CMD, value|SPI_CMD_BIT_PAUSE_EN_MASK);
	else
		SPI_MOTO2_WRITE32(SPI_CMD, value&(~SPI_CMD_BIT_PAUSE_EN_MASK));
}

void SPI_PauseModeResume(u32 const status)
{

    u32 value;

    value = SPI_MOTO_READ32(SPI_CMD);
    value = value|(SPI_CMD_BIT_RESUME_MASK|SPI_CMD_BIT_RESET_MASK);
    if (status == 1) 
	   SPI_MOTO_WRITE32(SPI_CMD, value|SPI_CMD_BIT_PAUSE_EN_MASK);
    else
       SPI_MOTO_WRITE32(SPI_CMD, value&(~SPI_CMD_BIT_PAUSE_EN_MASK));
}
void SPI2_PauseModeResume(u32 const status)
{

    u32 value;

    value = SPI_MOTO2_READ32(SPI_CMD);
	value = value|(SPI_CMD_BIT_RESUME_MASK|SPI_CMD_BIT_RESET_MASK);
	if (status == 1) 
	   SPI_MOTO2_WRITE32(SPI_CMD, value|SPI_CMD_BIT_PAUSE_EN_MASK);
    else
       SPI_MOTO2_WRITE32(SPI_CMD, value&(~SPI_CMD_BIT_PAUSE_EN_MASK));

}

void SPI_Activate(void)
{
	u32 value;

	value = SPI_MOTO_READ32(SPI_CMD);
	value = value|(SPI_CMD_BIT_CMD_ACT_MASK|SPI_CMD_BIT_RESET_MASK);
	SPI_MOTO_WRITE32(SPI_CMD, value);
}

void SPI2_Activate(void)
{
	u32 value;

	value = SPI_MOTO2_READ32(SPI_CMD);
	value = value|(SPI_CMD_BIT_CMD_ACT_MASK|SPI_CMD_BIT_RESET_MASK);
	SPI_MOTO2_WRITE32(SPI_CMD, value);
}

long GetTickcount(void)
{
	struct timespec tv;

	getnstimeofday(&tv);

	return tv.tv_sec;
}

u32 SPI_SelectMode(SPI_DIRECTION_TYPE const type, SPI_MODE const mode)
{
	u32 value;

	value = SPI_MOTO_READ32(SPI_CMD);

	if ((mode != SPI_MODE_DMA) && (mode != SPI_MODE_FIFO)) {
		SPI_ERR("SPI_SelectMode: Incorrect spi_mode parameter.\r\n");
		return 0;
	}

	switch (type) {
	case SPI_TX:
		if (mode == SPI_MODE_DMA)
			SPI_MOTO_WRITE32(SPI_CMD,
					value|SPI_CMD_BIT_TX_DMA_EN_MASK);
		else
			SPI_MOTO_WRITE32(SPI_CMD,
					value&(~SPI_CMD_BIT_TX_DMA_EN_MASK));
		break;

	case SPI_RX:
		if (mode == SPI_MODE_DMA)
			SPI_MOTO_WRITE32(SPI_CMD,
					value|SPI_CMD_BIT_RX_DMA_EN_MASK);
		else
			SPI_MOTO_WRITE32(SPI_CMD,
					value&(~SPI_CMD_BIT_RX_DMA_EN_MASK));
		break;

	default:
		SPI_ERR("SPI_SelectMode: Wrong spi_direction_type para\r\n");
		return 0;
	}

	return 1;
}

u32 SPI2_SelectMode(SPI_DIRECTION_TYPE const type, SPI_MODE const mode)
{
	u32 value;

	value = SPI_MOTO2_READ32(SPI_CMD);

	if ((mode != SPI_MODE_DMA) && (mode != SPI_MODE_FIFO)) {
		SPI_ERR("SPI_SelectMode: Incorrect spi_mode parameter.\r\n");
		return 0;
	}

	switch (type) {
	case SPI_TX:
		if (mode == SPI_MODE_DMA)
			SPI_MOTO2_WRITE32(SPI_CMD,
					value|SPI_CMD_BIT_TX_DMA_EN_MASK);
		else
			SPI_MOTO2_WRITE32(SPI_CMD,
					value&(~SPI_CMD_BIT_TX_DMA_EN_MASK));
		break;

	case SPI_RX:
		if (mode == SPI_MODE_DMA)
			SPI_MOTO2_WRITE32(SPI_CMD,
					value|SPI_CMD_BIT_RX_DMA_EN_MASK);
		else
			SPI_MOTO2_WRITE32(SPI_CMD,
					value&(~SPI_CMD_BIT_RX_DMA_EN_MASK));
		break;

	default:
		SPI_ERR("SPI_SelectMode: Wrong spi_direction_type arg\r\n");
		return 0;
	}

	return 1;
}

u32 SPI_ClearFifo(SPI_DIRECTION_TYPE const direction)
{
	u32 i;

	u32 volatile tmp;

	for (i = 0; i < (SPI_FIFO_SIZE/4); ++i) {
		switch (direction) {
		case SPI_TX:
			SPI_MOTO_WRITE32(SPI_TX_DATA, 0x0);
			break;
		case SPI_RX:
			tmp = SPI_MOTO_READ32(SPI_RX_DATA);
			break;
		default:
			SPI_ERR("SPI_ClearFifo: Incorrect spi_direction_type parameter.\r\n");
			return 0;
		}
	}

	return 1;
}

u32 SPI2_ClearFifo(SPI_DIRECTION_TYPE const direction)
{
	u32 i;
	u32 volatile tmp;

	for (i = 0; i < (SPI_FIFO_SIZE/4); ++i) {
		switch (direction) {
		case SPI_TX:
			SPI_MOTO2_WRITE32(SPI_TX_DATA, 0x0);
			break;
		case SPI_RX:
			tmp = SPI_MOTO2_READ32(SPI_RX_DATA);
			break;
		default:
			SPI_ERR("SPI_ClearFifo: Incorrect spi_direction_type parameter.\r\n");
			return 0;
		}
	}

	return 1;
}

u32 SPI_SetRWAddr(SPI_DIRECTION_TYPE const type, u32 addr)
{
	if ((type != SPI_TX) && (type != SPI_RX)) {
		SPI_ERR("SPI_SetRWAddr: Incorrect SPI_Direction_Type parameter.\r\n");
		return 0;
	}

	if (0 == addr) {
		SPI_ERR("SPI_SetRWAddr: Incorrect address parameter.\r\n");
		return 0;
	}

	if ((addr & 0x3) != 0) {
		SPI_ERR("SPI_SetRWAddr: Incorrect address illegal.\r\n");
		return 0;
	}

	if (SPI_TX == type) {
		SPI_MOTO_WRITE32(SPI_TX_SRC, addr);
	} else {
		SPI_MOTO_WRITE32(SPI_RX_DST, addr);
	}

	return 1;
}

u32 SPI2_SetRWAddr(SPI_DIRECTION_TYPE const type, u32 addr)
{
	if ((type != SPI_TX) && (type != SPI_RX)) {
		SPI_ERR("SPI_SetRWAddr: Incorrect SPI_Direction_Type parameter.\r\n");
		return 0;
	}

	if (0 == addr) {
		SPI_ERR("SPI_SetRWAddr: Incorrect address parameter.\r\n");
		return 0;
	}

	if ((addr & 0x3) != 0) {
		SPI_ERR("SPI_SetRWAddr: Incorrect address illegal.\r\n");
		return 0;
	}

	if (SPI_TX == type) {
		SPI_MOTO2_WRITE32(SPI_TX_SRC, addr);
	} else {
		SPI_MOTO2_WRITE32(SPI_RX_DST, addr);
	}

	return 1;
}

u32 SPI_SetDesiredSize(u16 const pkg_length, u16 const pkg_count)
{
	u32 u4Value;

	if ((pkg_length > SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES)
			|| (0 == pkg_length)) {
		SPI_ERR("SPI_SetDesiredSize: pkg_length is illegal.\r\n");
		return 0;
	}

	if ((pkg_count > SPI_INTERFACE_MAX_PKT_COUNT_PER_TIMES)
			|| (0 == pkg_count)) {
		SPI_ERR("SPI_SetDesiredSize: pkg_count is illegal.\r\n");
		return 0;
	}

	if (0 == pkg_length) {
		return 0;
	}

	if (0 == pkg_count) {
		return 0;
	}

	u4Value = SPI_MOTO_READ32(SPI_CFG1);
	u4Value &= ~0x3FFFF00;
	u4Value |= ((pkg_count - 1) << 8);
	u4Value |= ((pkg_length - 1) << 16);

	/* Set 'PACKET_LOOP_CNT' field. */
	SPI_MOTO_WRITE32(SPI_CFG1, u4Value);

	return 1;
}

u32 SPI2_SetDesiredSize(u16 const pkg_length, u16 const pkg_count)
{
	u32 u4Value;

	if ((pkg_length > SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES)
			|| (0 == pkg_length)) {
		SPI_ERR("SPI_SetDesiredSize: pkg_length is illegal.\r\n");
		return 0;
	}

	if ((pkg_count > SPI_INTERFACE_MAX_PKT_COUNT_PER_TIMES)
			|| (0 == pkg_count)) {
		SPI_ERR("SPI_SetDesiredSize: pkg_count is illegal.\r\n");
		return 0;
	}

	if (0 == pkg_length) {
		return 0;
	}

	if (0 == pkg_count) {
		return 0;
	}

	u4Value = SPI_MOTO2_READ32(SPI_CFG1);
	u4Value &= ~0x3FFFF00;
	u4Value |= ((pkg_count - 1) << 8);
	u4Value |= ((pkg_length - 1) << 16);

	/* Set 'PACKET_LOOP_CNT' field. */
	SPI_MOTO2_WRITE32(SPI_CFG1, u4Value);

	return 1;
}

u32 SPI_WaitFinished(void)
{
	long counter, counter1;

	counter1 = 0;
	counter = GetTickcount();

    while(counter1 < (SPI_MOTIO_OPERATION_TIMEOUT/1000))
    {
        if(SPI_MOTO_READ32(SPI_STATUS1))
        {
            return SPI_MOTO_OK;
        }
        else
        {
            counter1 = GetTickcount() - counter;
        }
    }

	return SPI_MOTO_TIMEOUT;
}

u32 SPI2_WaitFinished(void)
{
	long counter, counter1;

	counter1 = 0;
	counter = GetTickcount();

    while(counter1 < (SPI_MOTIO_OPERATION_TIMEOUT/1000))
    {
        if(SPI_MOTO2_READ32(SPI_STATUS1))
        {
            return SPI_MOTO_OK;
        }
        else
        {
            counter1 = GetTickcount() - counter;
        }
    }

	return SPI_MOTO_TIMEOUT;
}

bool SPI_SetTransactionLength(u32 dwCount)
{
	u16 u2PkgLength = 0, u2PkgCount = 0;
	u32 dwTmp;

	if (dwCount > MAX_TRANSCATION_BYTE) {
		SPI_ERR("SPI_SetTransactionLength: Incorrect dwCount parameter.\r\n");
		return FALSE;
	}

	/* find a good pair for the tarnsaction length */
	for (u2PkgCount = 1; u2PkgCount <= MAX_PACKET_LOOP_CNT; u2PkgCount++) {
		dwTmp = (dwCount / u2PkgCount);
		if (dwTmp <= MAX_PACKET_LENGTH) {
			u2PkgLength = (u16)dwTmp;
			break;
		}
	}

	return SPI_SetDesiredSize(u2PkgLength, u2PkgCount);
}
bool SPI2_SetTransactionLength(u32 dwCount)
{
    u16 u2PkgLength = 0, u2PkgCount = 0;
    u32 dwTmp;

    if (dwCount > MAX_TRANSCATION_BYTE)
    {
        SPI_DBG("SPI_SetTransactionLength: Incorrect dwCount parameter.\r\n");
        return FALSE;
    }

    // find a good pair for the tarnsaction length
    for (u2PkgCount = 1; u2PkgCount <= MAX_PACKET_LOOP_CNT; u2PkgCount++)
    {
        dwTmp = (dwCount / u2PkgCount);
        if (dwTmp <= MAX_PACKET_LENGTH)
        {
            u2PkgLength = (UINT16)dwTmp;
            break;
        }
    }

    return SPI2_SetDesiredSize(u2PkgLength, u2PkgCount);
}
SPI_STATE SPI_StartTransaction(SPI_STATE spiState, u32 dwCount)
{
	SPI_STATE state;

	if (dwCount > MAX_TRANSCATION_BYTE) {
		state = (spiState == SPI_STATE_IDLE2IDLE)
				? SPI_STATE_IDLE2PAUSE:SPI_STATE_PAUSE2PAUSE;
	} else {
		state = (spiState == SPI_STATE_PAUSE2PAUSE)
				? SPI_STATE_PAUSE2IDLE:SPI_STATE_IDLE2IDLE;
	}

	return state;
}

u32 SPI_StartTransactions(u32 dwCount, void *pBufferTx, void *pBufferRx)
{
	u32 dwTranscationLength = 0;
	u32 bRet = 1;

	u8 *pTx = NULL, *pRx = NULL;
	u32 spiState = SPI_STATE_IDLE2IDLE;
    u32 dwRty = 0;
    u32 dwLeft = 0;
	u32 dwTanscationAgain = 0;

	if (pBufferRx == NULL && pBufferTx == NULL) {
		SPI_ERR("pBufferRx and pBufferTx is NULL\n");
		return 0;
	}
	if (NULL != pBufferTx)
		pTx = pBufferTx;
	if (NULL != pBufferRx)
		pRx = pBufferRx;
    if(dwCount > 1024)
		dwTanscationAgain = 2;

#if 0
	if (pTx && pRx) {
		TX_RX_InterruptMode = RXInterruptMode; /* RX */
	} else if (pTx) {
		/* Tx with small transaction bytes for firmware download */
		TX_RX_InterruptMode = TXInterruptMode; /* TX */
	} else {        /* Rx */
		TX_RX_InterruptMode = RXInterruptMode; /* RX */
	}
#endif
	SPI_DBG("set bufferTx and bufferRx\n");
	flush_cache_all();
    while(dwCount > 0)
    {
        dwTranscationLength = ((dwCount > MAX_TRANSCATION_BYTE) ? MAX_TRANSCATION_BYTE: dwCount);
        dwRty = dwTranscationLength / 1024;
        if (dwRty > 0)
        {
            dwLeft = dwTranscationLength - dwRty*1024;
        }
        else
        {
            dwLeft = 0;
        }

        if(dwTanscationAgain == 2)
		{
			spiState = SPI_STATE_IDLE2PAUSE;
		}
		else if(dwTanscationAgain == 1)
		{
			spiState = SPI_STATE_PAUSE2IDLE;
		}
        SPI_SetTransactionLength(dwTranscationLength - dwLeft);

        if (pBufferTx != NULL)
        {
            SPI_SetRWAddr(SPI_TX, (UINT32)pTx);
            pTx+= (dwTranscationLength - dwLeft);
            SPI_DBG("send Tx\n");
        }
    
        if (pBufferRx != NULL)
        {
            SPI_SetRWAddr(SPI_RX, (UINT32)pRx);
            pRx+= (dwTranscationLength - dwLeft);
            SPI_DBG("rescive Rx\n");
        }


        switch (spiState)
        {
        case SPI_STATE_IDLE2IDLE:
            SPI_SetPauseMode(0);
            SPI_Activate( );
            SPI_DBG("SPI Activate:SPI_STATE_IDLE2IDLE\n");
            break;
        case SPI_STATE_IDLE2PAUSE:
            SPI_SetPauseMode(1);
            SPI_Activate( );
            SPI_DBG("SPI Activate:SPI_STATE_IDLE2PAUSE\n");
            break;
        case SPI_STATE_PAUSE2PAUSE:
            SPI_PauseModeResume(1);
            SPI_DBG("SPI Resume:SPI_STATE_PAUSE2PAUSE\n");
            break;
        case SPI_STATE_PAUSE2IDLE:
            SPI_PauseModeResume(0);
            SPI_DBG("SPI Resume:SPI_STATE_PAUSE2IDLE\n");
            break;
        default:
            break;
        }    
#if 0
        if (WaitComplete() == FALSE)
        {
            SPI_DBG("SPI_StartTransactions:WaitComplete failed.\r\n");
            bRet = 0;
            break;
        }
#else
        bRet = SPI_WaitFinished();  
        if(bRet == SPI_MOTO_TIMEOUT)
        {
            SPI_DBG("SPI DMA WRITE TIMEOUT.\r\n");
            break;
        }
        
#endif
        
        dwCount -= (dwTranscationLength - dwLeft);
        dwTanscationAgain --;
      
    }
    flush_cache_all();
    
    return bRet;
}

u32 SPI2_StartTransactions(u32 dwCount, void *pBufferTx, void *pBufferRx)
{
	u32 dwTranscationLength = 0;
	u32 bRet = 1;

	u8 *pTx = NULL, *pRx = NULL;
	u32 spiState = SPI_STATE_IDLE2IDLE;
    u32 dwRty = 0;
    u32 dwLeft = 0;
	u32 dwTanscationAgain = 0;

	if (pBufferRx == NULL && pBufferTx == NULL) {
		SPI_ERR("pBufferRx and pBufferTx is NULL\n");
		return 0;
	}
	if (NULL != pBufferTx)
		pTx = pBufferTx;
	if (NULL != pBufferRx)
		pRx = pBufferRx;

	if(dwCount > 1024)
		dwTanscationAgain = 2;
#if 0
	if (pTx && pRx) {
		TX_RX_InterruptMode = RXInterruptMode; /* RX */
	} else if (pTx) {
		/* Tx with small transaction bytes for firmware download */
		TX_RX_InterruptMode = TXInterruptMode; /* TX */
	} else {       /* Rx */
		TX_RX_InterruptMode = RXInterruptMode; /* RX */
	}
#endif
	SPI_DBG("set bufferTx and bufferRx\n");
	flush_cache_all();
    while(dwCount > 0)
    {
        dwTranscationLength = ((dwCount > MAX_TRANSCATION_BYTE) ? MAX_TRANSCATION_BYTE: dwCount);
        dwRty = dwTranscationLength / 1024;
        if (dwRty > 0)
        {
            dwLeft = dwTranscationLength - dwRty*1024;
        }
        else
        {
            dwLeft = 0;

        }
		
		if(dwTanscationAgain == 2)
		{
			spiState = SPI_STATE_IDLE2PAUSE;
		}
		else if(dwTanscationAgain == 1)
		{
			spiState = SPI_STATE_PAUSE2IDLE;
		}
        SPI2_SetTransactionLength(dwTranscationLength - dwLeft);

        if (pBufferTx != NULL)
        {
            SPI2_SetRWAddr(SPI_TX, (UINT32)pTx);
            pTx+= (dwTranscationLength - dwLeft);
            SPI_DBG("send Tx\n");
        }
    
        if (pBufferRx != NULL)
        {
            SPI2_SetRWAddr(SPI_RX, (UINT32)pRx);
            pRx+= (dwTranscationLength - dwLeft);
            SPI_DBG("rescive Rx\n");
        }

        switch (spiState)
        {
        case SPI_STATE_IDLE2IDLE:
            SPI2_SetPauseMode(0);
            SPI2_Activate( );
            SPI_DBG("SPI 2 Activate:SPI_STATE_IDLE2IDLE\n");
            break;
        case SPI_STATE_IDLE2PAUSE:
            SPI2_SetPauseMode(1);
            SPI2_Activate( );
            SPI_DBG("SPI 2 Activate:SPI_STATE_IDLE2PAUSE\n");
            break;
        case SPI_STATE_PAUSE2PAUSE:
            SPI2_PauseModeResume(1);
            SPI_DBG("SPI 2 Resume:SPI_STATE_PAUSE2PAUSE\n");
            break;
        case SPI_STATE_PAUSE2IDLE:
            SPI2_PauseModeResume(0);
            SPI_DBG("SPI 2 Resume:SPI_STATE_PAUSE2IDLE\n");
            break;
        default:
            break;
        }    
#if 0
        if (WaitComplete() == FALSE)
        {
            SPI_DBG("SPI_StartTransactions:WaitComplete failed.\r\n");
            bRet = 0;
            break;
        }
#else
        bRet = SPI2_WaitFinished();  
        if(bRet == SPI_MOTO_TIMEOUT)
        {
            SPI_DBG("SPI 2 DMA WRITE TIMEOUT.\r\n");
            break;
        }
        
#endif
        
        dwCount -= (dwTranscationLength - dwLeft);
		dwTanscationAgain --;
    }
    flush_cache_all();
    
    return bRet;
}

static int ac83xx_spi_transfer(struct spi_device *spi, struct spi_message *msg)
{
	struct ac83xx_spi *hw = to_hw(spi);
	struct spi_transfer *xfer;
	int spiRet = 0;
	int i;
	char *tmp = NULL;

	//SPI_DBG("transfer start\n");
	printk("transfer start\n");

	if (hw == NULL) {
		SPI_ERR("hw is null\n");
		return -EIO;
	}
	/* user configer ??? */
	init_completion(&hw->done);
	if (spi->master->bus_num == 0) {
		SPI_SelectMode(SPI_TX, SPI_MODE_FIFO);
		SPI_ClearFifo(SPI_TX);

		SPI_SelectMode(SPI_TX, SPI_MODE_DMA);
		SPI_SelectMode(SPI_RX, SPI_MODE_DMA);
		SPI_DBG("mode select ok !\n");
        list_for_each_entry(xfer,&msg->transfers,transfer_list)
        {

            memset(hw->dmaTXbuffer,0,SPI_DMA_BUF_SIZE);
            memset(hw->dmaRXbuffer,0,SPI_DMA_BUF_SIZE);

						printk("tx xfer->len=%d\n", xfer->len);
						tmp = (char*)(xfer->tx_buf);
						for(i=0; i<xfer->len; i++)
							printk("%x, ", *(tmp+i));
						printk("\n");
						
						printk("rx xfer->len=%d\n", xfer->len);
						tmp = (char*)(xfer->rx_buf);
						for(i=0; i<xfer->len; i++)
							printk("%x, ", *(tmp+i));
						printk("\n");
						
            if (xfer->tx_buf && xfer->len <= SPI_DMA_BUF_SIZE)
                memcpy(hw->dmaTXbuffer,xfer->tx_buf,xfer->len);
            else
								printk("SPI0:empty tx buf or low tx size\n");
                
            hw->len= xfer->len;
            if (SPI_StartTransactions(hw->len, (LPVOID)hw->dmaTXphyAddr, (LPVOID)hw->dmaRXphyAddr) == 1)
            {
                spiRet = 0;
                
                if (xfer->rx_buf && xfer->len <= SPI_DMA_BUF_SIZE)
                    memcpy(xfer->rx_buf,hw->dmaRXbuffer,xfer->len);
                else
									printk("SPI0:SPI:empty rx buf or low rx size\n");
            }
            else
            {
                spiRet = -1;
                SPI_DBG("SPI: Incorrect Transactions.\r\n");
            }   
        }
    }
	if (spi->master->bus_num == 1) {
		SPI2_SelectMode(SPI_TX, SPI_MODE_FIFO);
		SPI2_ClearFifo(SPI_TX);

		SPI2_SelectMode(SPI_TX, SPI_MODE_DMA);
		SPI2_SelectMode(SPI_RX, SPI_MODE_DMA);
		printk("mode select ok !\n");
        list_for_each_entry(xfer,&msg->transfers,transfer_list)
        {
            memset(hw->dmaTXbuffer,0,SPI_DMA_BUF_SIZE);
            memset(hw->dmaRXbuffer,0,SPI_DMA_BUF_SIZE);

            if (xfer->tx_buf && xfer->len <= SPI_DMA_BUF_SIZE)
                memcpy(hw->dmaTXbuffer,xfer->tx_buf,xfer->len);
            else
				printk("SPI1:empty buf or low size\n");
                
            hw->len= xfer->len;

            if (SPI2_StartTransactions(hw->len, (LPVOID)hw->dmaTXphyAddr, (LPVOID)hw->dmaRXphyAddr) == 1)
            {
                spiRet = 0;
                
                if (xfer->rx_buf && xfer->len <= SPI_DMA_BUF_SIZE)
                    memcpy(xfer->rx_buf,hw->dmaRXbuffer,xfer->len);
                else
					printk("SPI1:SPI:empty buf or low size\n");
            }
            else
            {
                spiRet = -1;
                SPI_DBG("SPI1: Incorrect Transactions.\r\n");
            }   

        }
    }

    
    msg->status = 0;
    msg->complete(msg->context);
    complete(&hw->done);
	printk("tansfer end spiRet: %d\n", spiRet);

	return spiRet;
}

/************************for spi test*************************/
static struct spi_board_info test_spi0_board[] = {
     [0] = {
              .modalias = "spidev",
              .bus_num = 0,
              .chip_select = 0,
              .max_speed_hz = 500*1000,
     },
};

static struct spi_board_info test_spi1_board[] = {
     [0] = {
              .modalias = "spidev",
              .bus_num = 1,
              .chip_select = 0,
              .max_speed_hz = 500*1000,
     },
};

int spi_write_read(struct spi_device *spi,
		const void *txbuf, unsigned n_tx,
		void *rxbuf, unsigned n_rx)
{
	int			status;
	struct spi_message	message;
	struct spi_transfer	x[1];
	u8			*local_buf;

	local_buf = kmalloc(n_tx + n_rx,GFP_KERNEL | GFP_DMA);
	printk("n_tx=%d, n_rx=%d\n",n_tx, n_rx);
	if (!local_buf)
		return -ENOMEM;
	
	spi_message_init(&message);
	memset(x, 0, sizeof(x));
	if (n_tx) {
		x[0].len = n_tx;
	}
	if (n_rx) {
		x[0].len = n_rx;
	}
	
	spi_message_add_tail(&x[0], &message);

	memcpy(local_buf, txbuf, n_tx);
	x[0].tx_buf = local_buf;
	x[0].rx_buf = local_buf + n_tx;

	/* do the i/o */
	status = spi_sync(spi, &message);
	if (status == 0)
		memcpy(rxbuf, x[0].rx_buf, n_rx);

	return status;
}

void spi_test(struct ac83xx_spi *hw)
{
	int err = 0;
	struct spi_device *test_spi_dev;
	u32 tx_buffer = 0xF055F055;
	u32 rx_buffer = 0xaaaaaaaa;
	u32 len = 4;

	if(hw->master->bus_num == 0)
	{
		rx_buffer = 0xaaaaaaaa;
		test_spi_dev = spi_new_device(hw->master, test_spi0_board);
		/*spi_write_read(test_spi_dev, &tx_buffer, len, &rx_buffer, len);
	
		if(err != 0)
		{
			printk("fail to test spi[%d]\n", hw->master->bus_num);
			printk("tx=%x, rx=%x\n", tx_buffer, rx_buffer);
		}
		else
		{
			printk("test spi[%d] ok\n", hw->master->bus_num);
			printk("tx=%x, rx=%x\n", tx_buffer, rx_buffer);
		}*/
	}
	
	if(hw->master->bus_num == 1)
	{
		rx_buffer = 0xaaaaaaaa;
		test_spi_dev = spi_new_device(hw->master, test_spi1_board);
		/*spi_write_read(test_spi_dev, &tx_buffer, len, &rx_buffer, len);
	
		if(err != 0)
		{
			printk("fail to test spi[%d]\n", hw->master->bus_num);
			printk("tx=%x, rx=%x\n", tx_buffer, rx_buffer);
		}
		else
		{
			printk("test spi[%d] ok\n", hw->master->bus_num);
			printk("tx=%x, rx=%x\n", tx_buffer, rx_buffer);
		}
		*/
	}
}
/**************************************************************/




static int ac83xx_spi_probe(struct platform_device *pdev)
{
	struct ac83xx_spi_info *pdata;
	struct ac83xx_spi *hw;
	struct spi_master *master;
	struct resource *res;
	struct device_node *node = NULL;
	int err = 0;
	
	pdev->id = spi_nr++;
	//clk_ac8317_spi[0] = devm_clk_get(&pdev->dev, "spi-mux");

	if(spi_nr == 1)  {
		node = of_find_compatible_node(NULL, NULL, "Autochips,ac823x-spi0");
		if (node) {
			SPI0_VBASE_ADDR = (ulong)of_iomap(node, 0);
			if (SPI0_VBASE_ADDR == 0) {
	            pr_err("[spi0]can't find io virtual base address");
				return -1;
			}
			pr_info("SPI[%d]_VBASE_ADDR=%lx\n", pdev->id, SPI0_VBASE_ADDR);
		}else {
	        pr_err("[spi0]can't find compatible node\n");
		}
		
		clk_ac8317_spi[0] = devm_clk_get(&pdev->dev, "spi-mux");
		clk_ac8317_spi_select[0] = devm_clk_get(&pdev->dev, "spi-select1");
		if((clk_ac8317_spi_select[0] == NULL) || (clk_ac8317_spi[0] == NULL))  {
			SPI_ERR("select spi %d clk error\n",pdev->id );
			return -1;
		}
		
	}
	else {
		node = of_find_compatible_node(NULL, NULL, "Autochips,ac823x-spi1");
		if (node) {
			SPI1_VBASE_ADDR = (ulong)of_iomap(node, 0);
			if (SPI0_VBASE_ADDR == 0) {
	            pr_err("[spi1]can't find io virtual base address");
				return -1;
			}
			pr_info("SPI[%d]_VBASE_ADDR=%lx\n", pdev->id, SPI0_VBASE_ADDR);
		}else {
	        pr_err("[spi1]can't find compatible node\n");
		}
		
		clk_ac8317_spi_select[1] = devm_clk_get(&pdev->dev, "spi-select2");
		if(clk_ac8317_spi_select[1] == NULL)  {
			SPI_ERR("select spi %d clk error\n",pdev->id );
			return -1;
		}
		
	}
	
	if (pdev->id < 0) {
		dev_err(&pdev->dev,"Invalid platform device id-%d\n", pdev->id);
		return -ENODEV;
	}
	master = spi_alloc_master(&pdev->dev, sizeof(struct ac83xx_spi));
	if (master == NULL) {
		dev_err(&pdev->dev, "No memory for spi master\n");
		err = -ENOMEM;
		goto err_nomem;
	}
	hw = spi_master_get_devdata(master);
	if (hw == NULL) {
		SPI_ERR("hw  is null spi master get devdata fail\n");
		return -EIO;
	}
	memset(hw, 0, sizeof(struct ac83xx_spi));

	hw->master = spi_master_get(master);
	hw->pdata = pdata = pdev->dev.platform_data;
	hw->dev = &pdev->dev;
	/*
	if (pdata==NULL) {
		dev_err(&pdev->dev, "No platform data supplid\n");
		err = -ENOENT;
		goto err_no_pdata;
	}
	*/
	platform_set_drvdata(pdev, hw);
	init_completion(&hw->done);

	master->num_chipselect = 1;
	master->bus_num = pdev->id;
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;

	master->setup = ac83xx_spi_setup;
	master->transfer = ac83xx_spi_transfer;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	hw->res = res;

	if (res == NULL) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");
		err = -ENOENT;
		goto err_no_iores;
	}
	spi_init(hw);
    //alloc DMA memory here
    spi_dma_init(hw);
	err = spi_register_master(master);
	if (err) {
		dev_err(&pdev->dev, "Fail to register SPI master\n");
		goto err_register;
	}
	pr_info("register SPI master ok\n");
	/***************for spi test*********************/
	spi_test(hw);
	
	return 0;

err_register:
	if (hw->set_cs == ac83xx_spi_gpiocs)
		gpio_free(pdata->pin_cs);
	clk_disable(hw->clk);
	clk_put(hw->clk);
err_no_iores:

err_nomem:
	return err;
}
static int ac83xx_spi_remove(struct platform_device *dev)
{
	int ret = 0;
	struct ac83xx_spi *hw = platform_get_drvdata(dev);
 
	platform_set_drvdata(dev, NULL);
	if (hw != NULL) {
		spi_unregister_master(hw->master);
		clk_disable(hw->clk);
		clk_put(hw->clk);

		if (hw->set_cs == ac83xx_spi_gpiocs)
			gpio_free(hw->pdata->pin_cs);

		spi_master_put(hw->master);
	}

	pins_spi0_st2 = pinctrl_lookup_state(pinctrl_spi0, "spi0_state_2");
	if (IS_ERR(pins_spi0_st2)) {
		SPI_ERR("lookup spi0_state_2 failed");
	}
	ret = pinctrl_select_state(pinctrl_spi0, pins_spi0_st2);
	if (ret) {
		SPI_ERR("select spi0_state_2 failed");
	}

	pins_spi0_st4 = pinctrl_lookup_state(pinctrl_spi0, "spi0_state_4");
	if (IS_ERR(pins_spi0_st4)) {
		SPI_ERR("lookup spi0_state_4 failed");
	}
	ret = pinctrl_select_state(pinctrl_spi0, pins_spi0_st4);
	if (ret) {
		SPI_ERR("select spi0_state_4 failed");
	}

	pins_spi0_st6 = pinctrl_lookup_state(pinctrl_spi0, "spi0_state_6");
	if (IS_ERR(pins_spi0_st6)) {
		SPI_ERR("lookup spi0_state_6 failed");
	}
	ret = pinctrl_select_state(pinctrl_spi0, pins_spi0_st6);
	if (ret) {
		SPI_ERR("select spi0_state_6 failed");
	}

	pins_spi0_st8 = pinctrl_lookup_state(pinctrl_spi0, "spi0_state_8");
	if (IS_ERR(pins_spi0_st8)) {
		SPI_ERR("lookup spi0_state_8 failed");
	}
	ret = pinctrl_select_state(pinctrl_spi0, pins_spi0_st8);
	if (ret) {
		SPI_ERR("select spi0_state_8 failed");
	}

	pins_spi1_st2 = pinctrl_lookup_state(pinctrl_spi1, "spi1_state_2");
	if (IS_ERR(pins_spi1_st2)) {
		SPI_ERR("lookup spi1_state_2 failed");
	}
	ret = pinctrl_select_state(pinctrl_spi1, pins_spi1_st2);
	if (ret) {
		SPI_ERR("select spi1_state_2 failed");
	}

	pins_spi1_st4 = pinctrl_lookup_state(pinctrl_spi1, "spi1_state_4");
	if (IS_ERR(pins_spi1_st4)) {
		SPI_ERR("lookup spi1_state_4 failed");
	}
	ret = pinctrl_select_state(pinctrl_spi1, pins_spi1_st4);
	if (ret) {
		SPI_ERR("select spi1_state_4 failed");
	}

	pins_spi1_st6 = pinctrl_lookup_state(pinctrl_spi1, "spi1_state_6");
	if (IS_ERR(pins_spi1_st6)) {
		SPI_ERR("lookup spi1_state_6 failed");
	}
	ret = pinctrl_select_state(pinctrl_spi1, pins_spi1_st6);
	if (ret) {
		SPI_ERR("select spi1_state_6 failed");
	}

	pins_spi1_st8 = pinctrl_lookup_state(pinctrl_spi1, "spi1_state_8");
	if (IS_ERR(pins_spi1_st8)) {
		SPI_ERR("lookup spi1_state_8 failed");
	}
	ret = pinctrl_select_state(pinctrl_spi1, pins_spi1_st8);
	if (ret) {
		SPI_ERR("select spi1_state_8 failed");
	}

	gpiod_put(pSpi0Gpio1);
	gpiod_put(pSpi0Gpio2);
	gpiod_put(pSpi0Gpio3);
	gpiod_put(pSpi0Gpio4);
	gpiod_put(pSpi1Gpio1);
	gpiod_put(pSpi1Gpio2);
	gpiod_put(pSpi1Gpio3);
	gpiod_put(pSpi1Gpio4);

	return 0;
}
static int ac83xx_spi_suspend(struct platform_device *pdev, pm_message_t msg)
{
	struct ac83xx_spi *hw = platform_get_drvdata(pdev);

	if (hw != NULL) {
		if (hw->pdata && hw->pdata->gpio_setup)
			hw->pdata->gpio_setup(hw->pdata, 0);
		
			#if defined(CONFIG_ATC_PRJ_ac823x_evb)
		    if(pdev->id == 0)  {
		    clk_disable(clk_ac8317_spi_select[0]);
      		clk_unprepare(clk_ac8317_spi_select[0]);
			}
			if(pdev->id == 1)  {
				clk_disable(clk_ac8317_spi_select[1]);
	      		clk_unprepare(clk_ac8317_spi_select[1]);
			}
			#endif
	}
	SPI_DBG("[SPI] suspend\n");
	return 0;
}
static int ac83xx_spi_resume(struct platform_device *pdev)
{
	struct ac83xx_spi *hw = platform_get_drvdata(pdev);

	if (hw != NULL) {
		spi_init(hw);
	}
	SPI_DBG("[SPI] resume\n");
	return 0;
}

static const struct of_device_id ac83xx_spi_of_ids[] = {
	{ .compatible = "Autochips,ac823x-spi0", },
	{ .compatible = "Autochips,ac823x-spi1", },
	{}
};

static struct platform_driver ac83xx_spi_driver = {
	.probe  = ac83xx_spi_probe,
	.remove = ac83xx_spi_remove,
	.suspend = ac83xx_spi_suspend,
	.resume = ac83xx_spi_resume,
	.driver = {
		.name = "ac83xx_spi",
		.owner = THIS_MODULE,
		.of_match_table = ac83xx_spi_of_ids,
	},
};

static int ac83xx_spi_init(void)
{
	int ret;

	MOD_VERSION_INFO(SPI_VER_NAME, SPI_VER_MAIN,
					SPI_VER_MINOR, SPI_VER_REV);
	SPI_DBG("AC83XX SPI: init\n");
	ret = platform_driver_register(&ac83xx_spi_driver);
	if (ret)
		SPI_ERR("spi register failed\n");
	SPI_DBG("spi register ok\n");
	return 0;
}

static void ac83xx_spi_exit(void)
{
	platform_driver_unregister(&ac83xx_spi_driver);
}

module_init(ac83xx_spi_init);
module_exit(ac83xx_spi_exit);

MODULE_AUTHOR("AutoChips");
MODULE_DESCRIPTION("AC83XX SPI Controller Driver");
MODULE_LICENSE("GPL");

