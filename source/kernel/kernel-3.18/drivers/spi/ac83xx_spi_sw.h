#ifndef AC83XX_SPI_SW_H
#define AC83XX_SPI_SW_H

#include "x_typedef.h"
#include <linux/types.h>

#define SPI_TX_FIFO_DEPTH 32
#define SPI_RX_FIFO_DEPTH 32

#define SPI_CF0_DEFAULT_VALUE       0x05050505
#define SPI_CF1_DEFAULT_VALUE       0x03ffff32

#define SPI_MOTO_OK          1
#define SPI_MOTO_ERR         2
#define SPI_MOTO_TIMEOUT     3


#define SPI_MOTIO_OPERATION_TIMEOUT  60000
/*
typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

typedef char   INT8;
typedef short  INT16;
typedef int    s32;
*/


/*
 * \enum SPI_TIME_TYPE
 * \ingroup spi
 *
 * @brief
 * Specify the time intervals used by the SPI interface.
 */
enum SPI_TIME_TYPE {
	SPI_TIME_SETUP,
	/**<
	* \ingroup spi
	* Setup time.
	*/
	SPI_TIME_HOLD,
	/**<
	* \ingroup spi
	* Hold time.
	*/
	SPI_TIME_LOW,
	/**<
	* \ingroup spi
	* Low voltage time of the SPI clock.
	*/
	SPI_TIME_HIGH,
	/**<
	* \ingroup spi
	* High voltage time of the SPI clock.
	*/
	SPI_TIME_IDLE
	/**<
	* \ingroup spi
	* Idle time.
	*/
};
typedef enum SPI_TIME_TYPE SPI_TIME_TYPE;

/** \enum SPI_INT_TYPE
 * \ingroup spi
 *
 * @brief
 * SPI interrupt category enum.
 *
 * This enumeration defines the two interrupts which SPI devices can generate.
 */
enum SPI_INT_TYPE {
	SPI_INT_PAUSE,
	/**<
	* \ingroup spi
	* Pause interrupt.
	*/
	SPI_INT_FINISH
	/**<
	* \ingroup spi
	* Finish interrupt.
	*/
};
typedef enum SPI_INT_TYPE SPI_INT_TYPE;

/** \enum SPI_DIRECTION_TYPE
 * \ingroup spi
 *
 * @brief
 * SPI direction enum.
 *
 * This enumeration defines whether this SPI operation is
 * transmission of reception.
 */
enum SPI_DIRECTION_TYPE {
	SPI_TX,
	/**<
	* \ingroup spi
	* Means transmission
	*/
	SPI_RX
	/**<
	* \ingroup spi
	* Means reception
	*/
};
typedef enum SPI_DIRECTION_TYPE SPI_DIRECTION_TYPE;

/** \enum SPI_MLSB
 * \ingroup spi
 *
 * @brief
 * Specify the MSB or LSB used by the SPI TX/RX operation.
 */
enum SPI_MLSB {
	SPI_LSB = 0,
	/**<
	* \ingroup spi
	* LSB.
	*/
	SPI_MSB
	/**<
	* \ingroup spi
	* MSB.
	*/
};
typedef enum SPI_MLSB SPI_MLSB;

/** \enum SPI_ENDIAN
 * \ingroup spi
 *
 * @brief
 * Specify the endian used by the SPI interface.
 */
enum SPI_ENDIAN {
	SPI_ENDIAN_BIG = 0,
	/**<
	* \ingroup spi
	* Big endian.
	*/
	SPI_ENDIAN_LITTLE
	/**<
	* \ingroup spi
	* Little endian.
	*/
};
typedef enum SPI_ENDIAN SPI_ENDIAN;

/** \enum SPI_CPOL
 * \ingroup spi
 *
 * @brief
 * Choose the desired clock polarities supported by the SPI interface.
 */
enum SPI_CPOL_ {
	SPI_CPOL_0 = 0,
	/**<
	* \ingroup spi
	* SPI clock polarity 0.
	*/
	SPI_CPOL_1
	/**<
	* \ingroup spi
	* SPI clock polarity 1.
	*/
};
typedef enum SPI_CPOL_ SPI_CPOL_;

/** \enum SPI_CPHA
 * \ingroup spi
 *
 * @brief
 * Choose the desired clock formats supported by the SPI interface.
 */
enum SPI_CPHA_ {
	SPI_CPHA_0 = 0,
	/**<
	* \ingroup spi
	* SPI clock format 0.
	*/
	SPI_CPHA_1
	/**<
	* \ingroup spi
	* SPI clock format 1.
	*/
};
typedef enum SPI_CPHA_ SPI_CPHA_;

/** \enum SPI_MODE
 * \ingroup spi
 *
 * @brief
 * Choose the SPI FIFO mode or the SPI DMA mode.
 */
enum SPI_MODE {
	SPI_MODE_FIFO = 0,
	/**<
	* \ingroup spi
	* SPI FIFO mode.
	*/
	SPI_MODE_DMA
	/**<
	* \ingroup spi
	* SPI DMA mode.
	*/
};
typedef enum SPI_MODE SPI_MODE;

enum SPI_STATE {
	SPI_STATE_IDLE2IDLE,
	SPI_STATE_IDLE2PAUSE,
	SPI_STATE_PAUSE2PAUSE,
	SPI_STATE_PAUSE2IDLE
};
typedef enum SPI_STATE SPI_STATE;


enum SPI_BIT_STATUS {
	SPI_DISABLE,
	SPI_ENABLE
};
typedef enum SPI_BIT_STATUS SPI_BIT_STATUS;

typedef void (*SPI_MOTO_ISR_FUN)(void);

typedef struct _tag_SPI_CONFIG {
	u8 setup_time;
	u8 hold_time;
	u8 clk_low;
	u8 clk_high;
	u8 idle_time;
	u32 clk_polarity;
	u32 clk_fmt;
	u32 enable_pause_mode;
	u32 enable_deassert_mode;
	SPI_BIT_STATUS enable_pause_int;
	SPI_BIT_STATUS enable_finish_int;
	SPI_ENDIAN tx_endian;
	SPI_ENDIAN rx_endian;
	SPI_MLSB tx_mlsb;
	SPI_MLSB rx_mlsb;
	SPI_MODE tx_mode;
	SPI_MODE rx_mode;
} SPI_USER_CONFIG;

typedef struct _spi_rw_config {
	u32 size;
	u32 rPA;
	u32 wPA;
} SPI_RW_CONFIG;

/* Export function prototype. */

extern s32 SPI_Moto_HAL_EnableInterrupt(u32 en);
extern s32 SPI_Moto_HAL_RegisterDMAISR(SPI_MOTO_ISR_FUN pISRFun);
extern void SPI_Moto_HAL_DMAISR(u16 ui2_vector_id);


u32 SPI_SetTimeInterval(SPI_TIME_TYPE const type,  u8 const value);
u32 SPI_SetDesiredSize(u16 const pkg_length, u16 const pkg_count);
u32 SPI_SetRWAddr(SPI_DIRECTION_TYPE const type, u32 addr);
u32 SPI_ClearFifo(SPI_DIRECTION_TYPE const direction);
void SPI_PushTxFifo(u32 data);
u32 SPI_PopFifo(SPI_DIRECTION_TYPE const direction, u32 *data);
u32 SPI_SetInterrupt(SPI_INT_TYPE const type, SPI_BIT_STATUS const status);
u32 SPI_SetEndian(SPI_DIRECTION_TYPE const direction, SPI_ENDIAN const endian);
u32 SPI_SetMsb(SPI_DIRECTION_TYPE const type, SPI_MLSB const msb);
u32 SPI_SelectMode(SPI_DIRECTION_TYPE const type, SPI_MODE const mode);
void SPI_SetCpol(u32 const status);
void SPI_SetCpha(u32 const status);
void SPI_SetDeassertMode(u32 const status);
void SPI_SetPauseMode(u32 const status);
u32 SPI_IsInPasuseMode(void);
void SPI_Resume(void);
void SPI_Activate(void);
u32 SPI_IsBusy(void);
void SPI_LISR(void);
u32 SPI_Hal_Init(void);
void SPI_Moto_Write_reg(u32 reg, u32 data);

u32 SPI_WaitFinished(void);
void SPI_PushRxFifo(u32 const data);
void SPI_Moto_Clear_Int(void);
u32 SPI_Moto_Is_Int(void);
u32 SPI_Set_User_Config(SPI_USER_CONFIG *userConfig);


#define IOCTL_SPI_SET_USER_CONFIG   \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0301, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SPI_LOOPBACK_MODE     \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0303, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SPI_READ_DATA         \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0304, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SPI_WRITE_DATA        \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0305, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define MAX_PACKET_LOOP_CNT     256
#define MAX_PACKET_LENGTH       1024
#define MAX_TRANSCATION_BYTE (MAX_PACKET_LOOP_CNT * MAX_PACKET_LENGTH)

#define SPI_DMA_BUF_SIZE   MAX_TRANSCATION_BYTE //4096


#endif  /* SPI_MOTO_SW_H */
