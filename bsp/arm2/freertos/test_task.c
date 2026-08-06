/**
 *  Unit Test Code
 *
 */

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"
#include "string.h"

#define TAGS "VISS"

/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT        ( 0x222222 )

#define pr_info   Printf

/* The task functions. */
void vTask1( void *pvParameters );
void vTask2( void *pvParameters );
void vTask3( void *pvParameters );
void vTask4( void *pvParameters );
void vTask4_1( void *pvParameters );
void vTask5( void *pvParameters );
void vTask5_1( void *pvParameters );
void vTask5_2( void *pvParameters );
void vTask6( void *pvParameters );
void vTask7( void *pvParameters );
void vTask7_1( void *pvParameters );
void vTask7_2( void *pvParameters );
void vTask8( void *pvParameters );
void vTask8_1( void *pvParameters );
void vTask8_2( void *pvParameters );
void vTask9( void *pvParameters );
void vTask9_1( void *pvParameters );
void vTaskSum( void *pvParameters );
TaskHandle_t xTask1Handle, xTask2Handle, xTask3Handle, xTask4Handle, 
    xTask5Handle, xTask6Handle, xTask7Handle, xTask8Handle, xTask9Handle;
SemaphoreHandle_t xBinarySemaphore;
const TickType_t xDelay250ms = pdMS_TO_TICKS( 250UL );
UBaseType_t xTotalRunningCase = 0;
static QueueHandle_t xQueue1 = NULL, xQueue2 = NULL;
static QueueSetHandle_t xQueueSet = NULL;
static void prvOneShotTimerCallback( TimerHandle_t xTimer );
const char *pcTextForTask1 = "Task Unit Test 1 is running";
const char *pcTextForTask2 = "Task Unit Test 2 is running";
const char *pcTextForTask3 = "Task Unit Test 3 is running";
const char *pcTextForTask4 = "Task Unit Test 4 is running";
const char *pcTextForTask5 = "Task Unit Test 5 is running";
const char *pcTextForTask6 = "Task Unit Test 6 is running";
const char *pcTextForTask7 = "Task Unit Test 7 is running";
const char *pcTextForTask8 = "Task Unit Test 8 is running";
const char *pcTextForTask9 = "Task Unit Test 9 is running";
/*-----------------------------------------------------------*/

static void prvOneShotTimerCallback( TimerHandle_t xTimer );
static TimerHandle_t  xOneShotTimer;
SemaphoreHandle_t xMutex;
BaseType_t xCase7Take = 0;

EventGroupHandle_t xEventGroup = NULL;
EventGroupHandle_t xEventGroupSum = NULL;
#define taskBIT0    ( 1UL << 0UL ) /* Event bit 0, which is set by a task. */
#define taskBIT1    ( 1UL << 1UL ) /* Event bit 0, which is set by a task. */
#define taskBIT2    ( 1UL << 2UL ) /* Event bit 0, which is set by a task. */
#define taskBIT3    ( 1UL << 3UL ) /* Event bit 0, which is set by a task. */
#define taskBIT4    ( 1UL << 4UL ) /* Event bit 0, which is set by a task. */
#define taskBIT5    ( 1UL << 5UL ) /* Event bit 0, which is set by a task. */
#define taskBIT6    ( 1UL << 6UL ) /* Event bit 0, which is set by a task. */
#define taskBIT7    ( 1UL << 7UL ) /* Event bit 0, which is set by a task. */
#define taskBIT8    ( 1UL << 8UL ) /* Event bit 0, which is set by a task. */
#define taskBIT9    ( 1UL << 9UL ) /* Event bit 0, which is set by a task. */

/* Definitions for the event bits in the event group. */
#define mainFIRST_TASK_BIT  ( 1UL << 0UL ) /* Event bit 0, which is set by a task. */
#define mainSECOND_TASK_BIT ( 1UL << 1UL ) /* Event bit 1, which is set by a task. */

int TestMain( void )
{

    /* Before a semaphore is used it must be explicitly created.  In this
    example a binary semaphore is created. */
    xBinarySemaphore = xSemaphoreCreateBinary();

    /* Check the semaphore was created successfully. */
    if( xBinarySemaphore != NULL )
    {

        xEventGroupSum = xEventGroupCreate();
        xTaskCreate( vTask1, "Task 1", 1000, (void*)pcTextForTask1, 1, &xTask1Handle );
        xTaskCreate( vTask2, "Task 2", 1000, (void*)pcTextForTask2, 1, &xTask2Handle );
        xTaskCreate( vTask3, "Task 3", 1000, (void*)pcTextForTask3, 1, &xTask3Handle );
        xTaskCreate( vTask4, "Task 4", 1000, (void*)pcTextForTask4, 1, &xTask4Handle );
        xTaskCreate( vTask5, "Task 5", 1000, (void*)pcTextForTask5, 1, &xTask5Handle );
        xTaskCreate( vTask6, "Task 6", 1000, (void*)pcTextForTask6, 1, &xTask6Handle );
        //xTaskCreate( vTask7, "Task 7", 1000, (void*)pcTextForTask7, 1, &xTask7Handle );
        xTaskCreate( vTask8, "Task 8", 1000, (void*)pcTextForTask8, 1, &xTask8Handle );
        xTaskCreate( vTask9, "Task 9", 1000, (void*)pcTextForTask9, 1, &xTask9Handle );
        xTaskCreate( vTaskSum, "Summary", 1000, NULL, 1, NULL );
    }
    xSemaphoreGive( xBinarySemaphore);
    return 0;
}
/*-----------------------------------------------------------*/

void vTask1( void *pvParameters )
{
    volatile uint32_t ul;
    char *pcTaskNameFromPara;
    pcTaskNameFromPara = ( char * ) pvParameters;
    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
        xTotalRunningCase++;
        /* Print out the name of this task. */
        pr_info("%s." , pcTaskNameFromPara );

        /* Delay for a period. */
        for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ )
        {
            /* This loop is just a very crude delay implementation.  There is
            nothing to do in here.  Later exercises will replace this crude
            loop with a proper delay/sleep function. */
        }
        if (!strcmp(pcTaskNameFromPara, "Task Unit Test 1 is running"))
        {
            // pr_info( "Unit Test 1 is Passed");
            xEventGroupSetBits( xEventGroupSum, taskBIT1 );
            break;
        }
    }
    xSemaphoreGive( xBinarySemaphore);
    vTaskDelete(xTask1Handle);
}
/*-----------------------------------------------------------*/

void vTask2( void *pvParameters )
{
    char *pcTaskNameFromPara;
    TickType_t xLastWakeTime;
    pcTaskNameFromPara = ( char * ) pvParameters;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for( ;; )
    {
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
        xTotalRunningCase++;
        pr_info("%s." , pcTaskNameFromPara );
        vTaskDelay( xDelay250ms );
        /* The xLastWakeTime variable needs to be initialized with the current tick
        count.  Note that this is the only time we access this variable.  From this
        point on xLastWakeTime is managed automatically by the vTaskDelayUntil()
        API function. */
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil( &xLastWakeTime, xDelay250ms );
        xEventGroupSetBits( xEventGroupSum, taskBIT2 );
        // pr_info( "Unit Test 2 is passed.");
        break;
    }
    xSemaphoreGive( xBinarySemaphore);
    vTaskDelete(xTask2Handle);
}


void vTask3( void *pvParameters )
{
    UBaseType_t uxPriority;
    char *pcTaskNameFromPara;
    pcTaskNameFromPara = ( char * ) pvParameters;
    uxPriority = uxTaskPriorityGet( NULL );

    for( ;; )
    {
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
        xTotalRunningCase++;
        pr_info("%s." , pcTaskNameFromPara );
        /* Setting the Task2 priority above the Task1 priority will cause
        Task2 to immediately start running (as then Task2 will have the higher 
        priority of the    two created tasks). */
        // pr_info( "About to raise the Task priority" );
        vTaskPrioritySet( xTask3Handle, ( uxPriority + 1 ) );
        vTaskDelay( xDelay250ms );
        if (uxTaskPriorityGet(xTask3Handle) == ( uxPriority + 1))
        {
            xEventGroupSetBits( xEventGroupSum, taskBIT3 );
            // pr_info( "Unit Test 3 is passed.");
            break;
        }
        /* Task1 will only run when it has a priority higher than Task2.
        Therefore, for this task to reach this point Task2 must already have
        executed and set its priority back down to 0. */
    }
    xSemaphoreGive( xBinarySemaphore);
    vTaskDelete(xTask3Handle);
}



void vTask4_1( void *pvParameters )
{
    BaseType_t xStatus;
    int32_t lValueToSend;
    vTaskDelay( xDelay250ms );
    lValueToSend = 1;
    xStatus = xQueueSendToBack( xQueue1, &lValueToSend, 0 );
    if( xStatus != pdPASS )
    {
        /* We could not write to the queue because it was full ?this must
        be an error as the queue should never contain more than one item! */
        // pr_info( "Could not send to the queue." );
    }
    vTaskDelete(NULL);
}

void vTask4( void *pvParameters )
{
    char *pcTaskNameFromPara;
    BaseType_t xStatus;
    int32_t lReceivedValue;
    pcTaskNameFromPara = ( char * ) pvParameters;

    for( ;; )
    {
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
        xTotalRunningCase++;
        pr_info("%s." , pcTaskNameFromPara );
        /* The queue is created to hold a maximum of 5 long values. */
        xQueue1 = xQueueCreate( 5, sizeof( int32_t ) );
        xTaskCreate( vTask4_1, "Task 4", 1000, NULL, 1, NULL);      

        xStatus = xQueueReceive( xQueue1, &lReceivedValue,  portMAX_DELAY);
        if( xStatus == pdPASS )
        {
            // pr_info( "Received = ", lReceivedValue );

        }
        else
        {
            // pr_info( "Could not receive from the queue." );
        }   
        xQueueReset(xQueue1);
        vQueueDelete(xQueue1);
        xQueue1 = NULL;
        if (lReceivedValue == 1) {
            xEventGroupSetBits( xEventGroupSum, taskBIT4 );
            // pr_info( "Unit Test 4 is passed.");
            break;
        }
    }
    xSemaphoreGive( xBinarySemaphore);
    vTaskDelete(xTask4Handle);
}



void vTask5_1( void *pvParameters )
{
    BaseType_t xStatus;
    const char * const pcMessage = "sender1 ";
    vTaskDelay( xDelay250ms );
    xStatus = xQueueSendToBack( xQueue1, &pcMessage, 0 );
    if( xStatus != pdPASS )
    {
        /* We could not write to the queue because it was full ?this must
        be an error as the queue should never contain more than one item! */
        // pr_info( "Could not send to the queue." );
    }
    vTaskDelete(NULL);
}


void vTask5_2( void *pvParameters )
{
    BaseType_t xStatus;
    const char * const pcMessage = "sender3 ";
    vTaskDelay( xDelay250ms );
    xStatus = xQueueSendToBack( xQueue2, &pcMessage, 0 );
    if( xStatus != pdPASS )
    {
        /* We could not write to the queue because it was full ?this must
        be an error as the queue should never contain more than one item! */
        // pr_info( "Could not send to the queue." );
    }
    vTaskDelete(NULL);
}

void vTask5( void *pvParameters )
{
    char *pcTaskNameFromPara;
    BaseType_t xAllReceived = 0;
    QueueHandle_t xQueueThatContainsData;
    char *pcReceivedString = NULL;
    pcTaskNameFromPara = ( char * ) pvParameters;

    for( ;; )
    {
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
        xTotalRunningCase++;
        pr_info("%s." , pcTaskNameFromPara );

        /* Create the two queues.  Each queue sends character pointers.  The
        priority of the receiving task is above the priority of the sending tasks so
        the queues will never have more than one item in them at any one time. */
        xQueue1 = xQueueCreate( 1, sizeof( char * ) );
        xQueue2 = xQueueCreate( 1, sizeof( char * ) );

        /* Create the queue set.  There are two queues both of which can contain
        1 item, so the maximum number of queue handle the queue set will ever have
        to hold is 2 (1 item multiplied by 2 sets). */
        xQueueSet = xQueueCreateSet( 1 * 2 );

        /* Add the two queues to the set. */
        xQueueAddToSet( xQueue1, xQueueSet );
        xQueueAddToSet( xQueue2, xQueueSet );

        /* Create the tasks that send to the queues. */
        xTaskCreate( vTask5_1, "Sender1", 1000, NULL, 1, NULL );
        xTaskCreate( vTask5_2, "Sender2", 1000, NULL, 1, NULL );

        for( ;; )
        {
            /* Block on the queue set to wait for one of the queues in the set to
            contain data.  Cast the QueueSetMemberHandle_t values returned from
            xQueueSelectFromSet() to a QueueHandle_t as it is known that all the
            items in the set are queues (as opposed to semaphores, which can also be
            members of a queue set). */
            xQueueThatContainsData = ( QueueHandle_t ) xQueueSelectFromSet( xQueueSet, portMAX_DELAY );

            /* An indefinite block time was used when reading from the set so
            xQueueSelectFromSet() will not have returned unless one of the queues in
            the set contained data, and xQueueThatContansData must be valid.  Read
            from the queue.  It is not necessary to specify a block time because it
            is known that the queue contains data.  The block time is set to 0. */
            xQueueReceive( xQueueThatContainsData, &pcReceivedString, 0 );

            /* Print the string received from the queue. */
            pr_info("%s.", pcReceivedString );

            if (xQueueThatContainsData == xQueue1)
            {
                xAllReceived++;
            }
            if (xQueueThatContainsData == xQueue2)
            {
                xAllReceived++;
            }
            if (xAllReceived == 2)
            {
                break;
            }
        }
        xQueueRemoveFromSet(xQueue1, xQueueSet);
        xQueueRemoveFromSet(xQueue2, xQueueSet);
        vQueueDelete(xQueue1);
        vQueueDelete(xQueue2);
        xQueue1 = NULL;
        xQueue2 = NULL;
        if (xAllReceived == 2) {
            xEventGroupSetBits( xEventGroupSum, taskBIT5 );
            // pr_info( "Unit Test 5 is passed.");
            break;
        }
    }
    xSemaphoreGive( xBinarySemaphore);
    vTaskDelete(xTask5Handle);
}

static void prvOneShotTimerCallback( TimerHandle_t xTimer )
{
    static TickType_t xTimeNow;
    static BaseType_t xCalledTime;

    vTaskDelay( xDelay250ms );
    xTimeNow = xTaskGetTickCount();
    /* Output a string to show the time at which the callback was executed. */
    pr_info( "One-shot timer callback executing %ld.", xTimeNow );
    xCalledTime++;
    if (xCalledTime == 2) {
        BaseType_t xStatus;
        int32_t lValueToSend;
        lValueToSend = 2;
        xStatus = xQueueSendToBack( xQueue1, &lValueToSend, 0 );
        if( xStatus != pdPASS )
        {
            /* We could not write to the queue because it was full ?this must
            be an error as the queue should never contain more than one item!
            */
            // pr_info( "Could not send to the queue." );
        }
        return;
    } else {
        xTimerReset(xOneShotTimer, 0);
    }
}

void vTask6( void *pvParameters )
{
    char *pcTaskNameFromPara;
    BaseType_t xStatus;
    int32_t lReceivedValue;

    pcTaskNameFromPara = ( char * ) pvParameters;
    xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
    pr_info("%s." , pcTaskNameFromPara );

    xQueue1 = xQueueCreate( 5, sizeof( int32_t ) );

    /* Create the one shot software timer, storing the handle to the created
    software timer in xOneShotTimer. */
    xOneShotTimer = xTimerCreate( "OneShot",                    /* Text name for the software timer - not used by FreeRTOS. */
        pdMS_TO_TICKS( 3333UL ),    /* The software timer's period in ticks. */
        pdFALSE,                        /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
        0,                          /* This example does not use the timer id. */
        prvOneShotTimerCallback );  /* The callback function to be used by the software timer being created. */
    if( xOneShotTimer != NULL )
    {
        /* Start the software timers, using a block time of 0 (no block time).
        The scheduler has not been started yet so any block time specified here
        would be ignored anyway. */
        xTimerStart( xOneShotTimer, pdMS_TO_TICKS( 500 ) );
    }

    xStatus = xQueueReceive( xQueue1, &lReceivedValue,  portMAX_DELAY);
    if( xStatus == pdPASS )
    {
        // pr_info( "Received = ", lReceivedValue );

    }
    else
    {
        // pr_info( "Could not receive from the queue." );
    }

    if (lReceivedValue == 1) {
        xEventGroupSetBits( xEventGroupSum, taskBIT6 );
        // pr_info( "Unit Test 6 is passed.");
        xTotalRunningCase++;
    }
    xSemaphoreGive( xBinarySemaphore);
    vTaskDelete(xTask6Handle);
}

void vTask7_1( void *pvParameters )
{
    vTaskDelay( xDelay250ms );
    xSemaphoreTake( xMutex, portMAX_DELAY );
    xCase7Take++;
    xSemaphoreGive( xMutex );
    vTaskDelete(NULL);
}


void vTask7_2( void *pvParameters )
{
    vTaskDelay( xDelay250ms );
    xSemaphoreTake( xMutex, portMAX_DELAY );
    xCase7Take++;
    xSemaphoreGive( xMutex );
    vTaskDelete(NULL);
}

void vTask7( void *pvParameters )
{
    char *pcTaskNameFromPara;
    pcTaskNameFromPara = ( char * ) pvParameters;
    xMutex = xSemaphoreCreateMutex();
    if( xMutex == NULL )
    {
        return;
    }

    for( ;; )
    {
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
        xTotalRunningCase++;
        pr_info("task7 %s." , pcTaskNameFromPara );

        /* Create the tasks that send to the queues. */
        xTaskCreate( vTask7_1, "Mutex1", 1000, NULL, 2, NULL );
        xTaskCreate( vTask7_2, "Mutex2", 1000, NULL, 1, NULL );

        if (xCase7Take == 2) {
            xEventGroupSetBits( xEventGroupSum, taskBIT7 );
            pr_info( "Task7 is passed.");
            break;
        }
    }
    vSemaphoreDelete(xMutex);
    xSemaphoreGive( xBinarySemaphore);
    vTaskDelete(xTask7Handle);
}


void vTask8_1( void *pvParameters )
{
    const TickType_t xDelay200ms = pdMS_TO_TICKS( 200UL );

    for( ;; )
    {
        /* Delay for a short while before starting the next loop. */
        vTaskDelay( xDelay200ms );

        /* Print out a message to say event bit 0 is about to be set by the
        task, then set event bit 0. */
        // pr_info( "Bit setting task -\t about to set bit 0." );
        xEventGroupSetBits( xEventGroup, mainFIRST_TASK_BIT );
        break;
    }
    vTaskDelete(NULL);
}


void vTask8_2( void *pvParameters )
{
    const TickType_t xDelay200ms = pdMS_TO_TICKS( 200UL );

    for( ;; )
    {
        /* Delay for a short while before setting the other bit set within this
        task. */
        vTaskDelay( xDelay200ms );
        /* Print out a message to say event bit 1 is about to be set by the
        task, then set event bit 1. */
        // pr_info( "Bit setting task -\t about to set bit 1." );
        xEventGroupSetBits( xEventGroup, mainSECOND_TASK_BIT );
        break;
    }
    vTaskDelete(NULL);
}

void vTask8( void *pvParameters )
{
    char *pcTaskNameFromPara;
    const EventBits_t xBitsToWaitFor = ( mainFIRST_TASK_BIT | mainSECOND_TASK_BIT );
    pcTaskNameFromPara = ( char * ) pvParameters;

    for( ;; )
    {
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
        xTotalRunningCase++;
        pr_info("%s." , pcTaskNameFromPara );

        xEventGroup = xEventGroupCreate();
        /* Create the tasks that send to the queues. */
        xTaskCreate( vTask8_1, "Bit1", 1000, NULL, 2, NULL );
        xTaskCreate( vTask8_2, "Bit2", 1000, NULL, 1, NULL );

        /* Block to wait for event bits to become set within the event group. */
        xEventGroupWaitBits( /* The event group to read. */
            xEventGroup,
            /* Bits to test. */
            xBitsToWaitFor,
            /* Clear bits on exit if the
            unblock condition is met. */
            pdTRUE,
            /* Don't wait for all bits. */
            pdTRUE,
            /* Don't time out. */
            portMAX_DELAY );
        xEventGroupSetBits( xEventGroupSum, taskBIT8 );
        pr_info( "Task8 is passed.");
        vEventGroupDelete(xEventGroup);
        break;
    }

    xSemaphoreGive( xBinarySemaphore);
    vTaskDelete(xTask8Handle);
}



void vTask9_1( void *pvParameters )
{
    vTaskDelay( xDelay250ms );
    xTaskNotifyGive(xTask9Handle);
    vTaskDelete(NULL);
}

void vTask9( void *pvParameters )
{
    char *pcTaskNameFromPara;
    pcTaskNameFromPara = ( char * ) pvParameters;

    for( ;; )
    {
        xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
        xTotalRunningCase++;
        pr_info("%s." , pcTaskNameFromPara );
        xTaskCreate( vTask9_1, "TaskNotify", 1000, NULL, 1, NULL);      
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
        xEventGroupSetBits( xEventGroupSum, taskBIT9 );
        pr_info( "Task9 is passed.");
        break;
    }
    xSemaphoreGive( xBinarySemaphore);
    vTaskDelete(xTask9Handle);
}


void vTaskSum( void *pvParameters )
{
    EventBits_t xEventGroupValue;
    UBaseType_t xPassedCase = 0;
    const EventBits_t xBitsToWaitFor = ( taskBIT1  | taskBIT2 | taskBIT3 | taskBIT4 | taskBIT5 | taskBIT6 | taskBIT7 | taskBIT8 | taskBIT9 );

    for( ;; )
    {

        /* Block to wait for event bits to become set within the event group. */
        xEventGroupValue = xEventGroupWaitBits( /* The event group to read. */
            xEventGroupSum,
            /* Bits to test. */
            xBitsToWaitFor,
            /* Clear bits on exit if the
            unblock condition is met. */
            pdTRUE,
            /* Don't wait for all bits. */
            pdFALSE,
			/* Expired in 250ms. */
            xDelay250ms);
        if( ( xEventGroupValue & taskBIT1 ) != 0 )
        {
            pr_info( "Unit Test Case 1 passed. " );
            xPassedCase++;
        }
        if( ( xEventGroupValue & taskBIT2 ) != 0 )
        {
            pr_info( "Unit Test Case 2 passed. " );
            xPassedCase++;
        }
        if( ( xEventGroupValue & taskBIT3 ) != 0 )
        {
            pr_info( "Unit Test Case 3 passed. " );
            xPassedCase++;
        }
        if( ( xEventGroupValue & taskBIT4 ) != 0 )
        {
            pr_info( "Unit Test Case 4 passed. " );
            xPassedCase++;
        }
        if( ( xEventGroupValue & taskBIT5 ) != 0 )
        {
            pr_info( "Unit Test Case 5 passed. " );
            xPassedCase++;
        }
        if( ( xEventGroupValue & taskBIT6 ) != 0 )
        {
            pr_info( "Unit Test Case 6 passed. " );
            xPassedCase++;
        }
        if( ( xEventGroupValue & taskBIT7 ) != 0 )
        {
            pr_info( "Unit Test Case 7 passed. " );
            xPassedCase++;
        }
        if( ( xEventGroupValue & taskBIT8 ) != 0 )
        {
            pr_info( "Unit Test Case 8 passed. " );
            xPassedCase++;
        }
        if( ( xEventGroupValue & taskBIT9 ) != 0 )
        {
            pr_info( "Unit Test Case 9 passed. " );
            xPassedCase++;
        }
        if ((xPassedCase == 9) || (xTotalRunningCase == 9))
        {
            pr_info( "%ld Test Cases are passed, %ld Cases Failed.\n", xPassedCase, (xTotalRunningCase - xPassedCase));
            break;
        }

    }
    vTaskDelete(NULL);
}
