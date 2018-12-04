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
#define LCD_UPDATE_RATE		100	 //pick a rate that makes 6b1 ~1hz used currently message is sent at 100hz
//Can Message Constants
#define BMS_MSG_ID			0x6B1
//#define BMS_FAULT_ID
//#define MC_FAULT_ID
#define MAIN_FAULT_ID		0x200

//Queue Constants
#define RX_CAN_QUEUE_SIZE 8
#define TX_CAN_QUEUE_SIZE 8
#define RX_UART_QUEUE_SIZE 5
#define TX_UART_QUEUE_SIZE 5

//Stack Constants
#define LCD_MAIN_STACK_SIZE 128
#define TX_CAN_STACK_SIZE	128
#define TX_UART_STACK_SIZE	64

//Priority Constatns
#define LCD_MAIN_PRIORTIY 	1
#define TX_CAN_PRIORITY		1
#define TX_UART_PRIORITY	1

//Rates
#define LCD_MAIN_RATE 		100 / portTICK_RATE_MS
#define TX_CAN_RATE		100 / portTICK_RATE_MS
#define TX_UART_RATE	100 / portTICK_RATE_MS

typedef struct {
	CAN_HandleTypeDef* can;
	UART_HandleTypeDef* uart;

	QueueHandle_t q_rx_can;
	QueueHandle_t q_tx_can;
	QueueHandle_t q_rx_uart;
	QueueHandle_t q_tx_uart;
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
