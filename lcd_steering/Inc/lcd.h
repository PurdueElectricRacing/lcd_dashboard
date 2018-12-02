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
#define LCD_UPDATE_RATE		100	 //pick a rate that makes 6b1 ~1hz used currently message is sent at 100hz
//Can Message Constants
#define BMS_MSG_ID			0x6B1
//#define BMS_FAULT_ID
//#define MC_FAULT_ID
#define MAIN_FAULT_ID		0x200

//Queue Constants
#define RX_CAN_QUEUE_SIZE 16
#define TX_CAN_QUEUE_SIZE 16
#define RX_UART_QUEUE_SIZE 10
#define TX_UART_QUEUE_SIZE 10

typedef struct {
	uint8_t rx_buffer[6];
	uint16_t rx_size;
}uart_rx_t;

typedef struct {
	uint8_t tx_buffer[6];
	uint16_t tx_size;
}uart_tx_t;

typedef struct {
	CAN_HandleTypeDef* can;
	UART_HandleTypeDef* uart;

	QueueHandle_t q_rx_can;
	QueueHandle_t q_tx_can;
	QueueHandle_t q_rx_uart;
	QueueHandle_t q_tx_uart;
}lcd_t;

int lcd_main(void);
int btn_handler(uint8_t btn);
void initRTOSObjects(void);

lcd_t lcd;

extern uart_rx_t rx_buff;
extern CAN_HandleTypeDef hcan1;
extern UART_HandleTypeDef huart1;

#endif /* LCD_H_ */