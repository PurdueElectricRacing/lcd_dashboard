/*
 * lcd.h
 *
 *  Created on: Nov 17, 2018
 *      Author: Matt Flanagan
 */

#ifndef LCD_H_
#define LCD_H_

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_can.h"
#include "stm32l4xx_hal_uart.h"
#include "cmsis_os.h"
#include "main.h"
#include "uart.h"
#include "can.h"
#include "nextion_hardware.h"
#include "task.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
b'e\x00\x01\x00\xff\xff\xff' //start button
b'e\x01\x06\x00\xff\xff\xff' //Stop Button
b'e\x02\x02\x00\xff\xff\xff' //eco mode
b'e\x02\x03\x00\xff\xff\xff' //race mode
b'e\x02\x04\x00\xff\xff\xff' //sport mode
b'e\x01\x01\x01\xff\xff\xff' //Active Aero Enable
*/

//LCD Constants
#define START_ID_0			0x00 //start button ID
#define START_ID_1			0x01 //second byte
#define STOP_ID_0			0x01 //stop button ID
#define STOP_ID_1			0x06
#define ACTIVE_AERO_ID_0 	0x01 //active aero ID
#define ACTIVE_AERO_ID_1	0x01
#define ECO_MODE_ID_0		0x02 //ECO Mode ID
#define ECO_MODE_ID_1		0x02
#define RACE_MODE_ID_0		0x02 //Race Mode ID
#define RACE_MODE_ID_1 		0x03
#define SPORT_MODE_ID_0		0x02 //Sport Mode ID
#define SPORT_MODE_ID_1		0x04
#define LCD_UPDATE_RATE		1	 //pick a rate that makes 6b1 ~1hz used currently message is sent at 100hz
//Can Message Constants
#define BMS_MSG_ID			0x6B1
//#define BMS_FAULT_ID
//#define MC_FAULT_ID
#define MAIN_FAULT_ID		0x200

//Queue Constants
#define RX_CAN_QUEUE_SIZE 8
#define TX_CAN_QUEUE_SIZE 8
#define RX_UART_QUEUE_SIZE 5
#define TX_UART_QUEUE_SIZE 10

//Stack Constants
#define LCD_MAIN_STACK_SIZE 128
#define TX_CAN_STACK_SIZE	128
#define TX_UART_STACK_SIZE	64

//Priority Constatns
#define LCD_MAIN_PRIORTIY 	1
#define TX_CAN_PRIORITY		1
#define TX_UART_PRIORITY	1

//Rates
#define LCD_MAIN_RATE 		10 / portTICK_RATE_MS
#define TX_CAN_RATE		10 / portTICK_RATE_MS
#define TX_UART_RATE	10 / portTICK_RATE_MS

//Timeouts
#define TIMEOUT			5 / portTICK_RATE_MS
#define WAIT_QUEUE_FULL	30 / portTICK_RATE_MS

//LCD Bounds Constants
#define BMS_OVER_TEMP_RED	60
#define BMS_OVER_TEMP_YEL	55
#define BMS_UNDER_VOLT_RED	355
#define BMS_SOC_RED			10
#define	BMS_SOC_YEL			50
#define BMS_SOC_GREEN		50


typedef struct QueueDefinition
{
	int8_t *pcHead;					/*< Points to the beginning of the queue storage area. */
	int8_t *pcTail;					/*< Points to the byte at the end of the queue storage area.  Once more byte is allocated than necessary to store the queue items, this is used as a marker. */
	int8_t *pcWriteTo;				/*< Points to the free next place in the storage area. */

	union							/* Use of a union is an exception to the coding standard to ensure two mutually exclusive structure members don't appear simultaneously (wasting RAM). */
	{
		int8_t *pcReadFrom;			/*< Points to the last place that a queued item was read from when the structure is used as a queue. */
		UBaseType_t uxRecursiveCallCount;/*< Maintains a count of the number of times a recursive mutex has been recursively 'taken' when the structure is used as a mutex. */
	} u;

	List_t xTasksWaitingToSend;		/*< List of tasks that are blocked waiting to post onto this queue.  Stored in priority order. */
	List_t xTasksWaitingToReceive;	/*< List of tasks that are blocked waiting to read from this queue.  Stored in priority order. */

	volatile UBaseType_t uxMessagesWaiting;/*< The number of items currently in the queue. */
	UBaseType_t uxLength;			/*< The length of the queue defined as the number of items it will hold, not the number of bytes. */
	UBaseType_t uxItemSize;			/*< The size of each items that the queue will hold. */

	volatile int8_t cRxLock;		/*< Stores the number of items received from the queue (removed from the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */
	volatile int8_t cTxLock;		/*< Stores the number of items transmitted to the queue (added to the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */

	#if( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
		uint8_t ucStaticallyAllocated;	/*< Set to pdTRUE if the memory used by the queue was statically allocated to ensure no attempt is made to free the memory. */
	#endif

	#if ( configUSE_QUEUE_SETS == 1 )
		struct QueueDefinition *pxQueueSetContainer;
	#endif

	#if ( configUSE_TRACE_FACILITY == 1 )
		UBaseType_t uxQueueNumber;
		uint8_t ucQueueType;
	#endif

} xQUEUE_my;

typedef struct {
	CAN_HandleTypeDef* can;
	UART_HandleTypeDef* uart;

	QueueHandle_t q_rx_can;
	QueueHandle_t q_tx_can;
	QueueHandle_t q_rx_uart;
	QueueHandle_t q_tx_uart;

	xQUEUE_my* q_rx_can_debug;
	xQUEUE_my* q_tx_can_debug;
	xQUEUE_my* q_rx_uart_debug;
	xQUEUE_my* q_tx_uart_debug;
}lcd_t;

typedef struct {
	uint16_t pack_volt;				//Most recent pack voltage
	uint8_t pack_soc;				//pack SOC
	uint8_t  high_temp;			//the current highest temperature of a cell
}bms_data_t;

void task_lcd_main();
int btn_handler(uint8_t btn);
void error_blink();
void initRTOSObjects(void);

lcd_t lcd;

extern CAN_HandleTypeDef hcan1;
extern UART_HandleTypeDef huart1;

#endif /* LCD_H_ */
