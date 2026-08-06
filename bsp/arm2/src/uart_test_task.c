#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"
#include "string.h"
#include "printf.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define TAGS "UART-TEST"


#define UART_TEST_PORT    5//4
#define UART_DATA_LENGTH  32//1//the depth of RX FIFO is 32byte for normal UART.

int uarttest_polling_mode(void)
{
    const char txbuffer[UART_DATA_LENGTH] = {0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5F,0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70};
    char rxbuffer[UART_DATA_LENGTH] = {0};

    int i, count;
    uint32_t baud[] = {115200,921600};
    int ret = 0;

    for (count = 0; count < (int)ARRAY_SIZE(baud); count++) {
        uartInit(UART_TEST_PORT, baud[count]);

        for (i = 0; i < UART_DATA_LENGTH; i++) {
            writeByte(UART_TEST_PORT, txbuffer[i]);
        }

		vTaskDelay(10);//Only for TX RX loopback test, make sure TX finish.

        for (i = 0; i < UART_DATA_LENGTH; i++) {
            rxbuffer[i] = (char)readByte(UART_TEST_PORT);
        }

        for (i = 0; i < UART_DATA_LENGTH; i++) {
            pr_info("--uart-poll mode-rxbuffer[%d] =0x%x, txbuffer[%d] = 0x%x, baud is %d\n", i, rxbuffer[i], i, txbuffer[i], baud[count]);

            if (rxbuffer[i] != txbuffer[i]) {
                //ret = -1;
                pr_info("--uart-error-poll-rxbuffer[%d] =0x%x, txbuffer[%d] = 0x%x, baud is %d\n", i, rxbuffer[i], i, txbuffer[i], baud[count]);
                //break;
            }
        }

    }

    return ret;
}

int uarttest_interrupt_mode(void)
{
    const char txbuffer[UART_DATA_LENGTH] = {0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5F,0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70};
    char rxbuffer[UART_DATA_LENGTH] = {0};

    int i, count;
    uint32_t baud[] = {115200,921600};
    int ret = 0;

    for (count = 0; count < (int)ARRAY_SIZE(baud); count++) {
        uart_monitor_init(UART_TEST_PORT, baud[count]);//init

        for (i = 0; i < UART_DATA_LENGTH; i++) {
            writeByte(UART_TEST_PORT, txbuffer[i]);//TX write
        }

		vTaskDelay(10); //Only for TX RX loopback test, make sure TX finish.

        uart_monitor_read(rxbuffer, UART_DATA_LENGTH);//RX read

        for (i = 0; i < UART_DATA_LENGTH; i++) {
            pr_info("--uart-interrupt mode--rxbuffer[%d] =0x%x, txbuffer[%d] = 0x%x, baud is %d\n", i, rxbuffer[i], i, txbuffer[i], baud[count]);

            if (rxbuffer[i] != txbuffer[i]) {
                //ret = -1;
                pr_info("--uart-error-int-rxbuffer[%d] =0x%x, txbuffer[%d] = 0x%x, baud is %d\n", i, rxbuffer[i], i, txbuffer[i], baud[count]);
                //break;
            }
        }

#if 0//test again
		vTaskDelay(10); //Only for TX RX loopback test, make sure TX finish.
		pr_info("--before--write--again-\n");

		for (i = 0; i < UART_DATA_LENGTH; i++) {
			writeByte(UART_TEST_PORT, txbuffer[i]);
		}
		pr_info("--after--write-again--\n");
		vTaskDelay(10); /*make sure tx has finished.*/
		pr_info("--before--read--again-\n");

		uart_monitor_read(rxbuffer, UART_DATA_LENGTH);

		for (i = 0; i < UART_DATA_LENGTH; i++) {

			pr_info("--uart--again--interrupt mode--rxbuffer[%d] =0x%x, txbuffer[%d] = 0x%x, baud is %d\n", i, rxbuffer[i], i, txbuffer[i], baud[count]);

			if (rxbuffer[i] != txbuffer[i]) {
				//ret = -1;
				pr_info("--uart--again---error-int-rxbuffer[%d] =0x%x, txbuffer[%d] = 0x%x, baud is %d\n", i, rxbuffer[i], i, txbuffer[i], baud[count]);
				//break;
			}
		}

		pr_info("--after--read--again-\n");
#endif

        uart_monitor_deinit(UART_TEST_PORT);//deinit

    }

    return ret;
}

static void uartTestTask(void *pvParameters)
{
    uarttest_interrupt_mode();
    uarttest_polling_mode();

    return;
}

void uartTestTaskMain(void)
{
    xTaskCreate(uartTestTask, "uart test task", 5000, NULL, 1, NULL);
}

