/***************************************************************************
*
*     File Information
*
*     Name of File: lcd.c
*
*     Authors (Include Email):
*       1. Matthew Flanagan       matthewdavidflanagan@outlook.com
*
*     File dependents: (header files, flow charts, referenced documentation)
*       1.
*
*     File Description: Main constants for Can messages, button identifiers
*     and BMS constants.
*
***************************************************************************/

#ifndef LCD_H_
#define LCD_H_

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_can.h"
#include "stm32l4xx_hal_uart.h"
#include "stm32l4xx_hal_adc.h"
#include "cmsis_os.h"
#include "main.h"
#include "uart.h"
#include "steer.h"
#include "nextion_hardware.h"
#include "task.h"
#include "can_usr.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//LCD Constants What the LCD SCREEN Sends
#define START_ID_0			    0x00 //start button ID
#define START_ID_1			    0x01 //second byte
#define STOP_ID_0			      0x01 //stop button ID
#define STOP_ID_1			      0x05
#define ACTIVE_AERO_ID_0 	  0x01 //active aero ID
#define ACTIVE_AERO_ID_1	  0x07
#define ECO_MODE_ID_0		    0x02 //ECO Mode ID
#define ECO_MODE_ID_1		    0x01
#define RACE_MODE_ID_0		  0x02 //Race Mode ID
#define RACE_MODE_ID_1 		  0x02
#define SPORT_MODE_ID_0		  0x02 //Sport Mode ID
#define SPORT_MODE_ID_1	  	0x03
#define LCD_UPDATE_RATE		  1	 //pick a rate that makes 6b1 ~1hz used currently message is sent at 100hz
//Can Message Constants
#define START_MSG_ID				0x350
#define BMS_MSG_ID			    0x6B1
#define MAIN_ACK_ID					0x360
#define STEER_SENSE_ID			0x7C0
//#define MC_FAULT_ID
#define MAIN_FAULT_ID		    0x200

//Queue Constants
#define RX_CAN_QUEUE_SIZE 	32
#define TX_CAN_QUEUE_SIZE	  8
#define RX_UART_QUEUE_SIZE 	5
#define TX_UART_QUEUE_SIZE 	10

//Stack Constants
#define LCD_MAIN_STACK_SIZE 512
#define TX_CAN_STACK_SIZE	  256
#define TX_UART_STACK_SIZE	256
#define STEER_STACK_SIZE		256

//Priority Constatns
#define LCD_MAIN_PRIORTIY 	1
#define TX_CAN_PRIORITY		  1
#define TX_UART_PRIORITY  	1
#define STEER_PRIORITY			1

//Rates
#define LCD_MAIN_RATE 		  10 / portTICK_RATE_MS
#define TX_CAN_RATE			    5 / portTICK_RATE_MS
#define TX_UART_RATE		    10 / portTICK_RATE_MS
#define STEER_RATE					50 / portTICK_RATE_MS

//Delays
#define DELAY_STARTUP				500 / portTICK_RATE_MS

//Timeouts
#define TIMEOUT				      5 / portTICK_RATE_MS
#define WAIT_QUEUE_FULL		  30 / portTICK_RATE_MS

//LCD Bounds Constants
#define BMS_OVER_TEMP_RED	  60
#define BMS_OVER_TEMP_YEL	  55
#define BMS_UNDER_VOLT_RED	355
#define BMS_SOC_RED			    10
#define	BMS_SOC_YEL			    50
#define BMS_SOC_GREEN		    50

typedef enum {
	START = 0,
	RACE = 1,
	SETTINGS = 2
}page_t;

//Main LCD structure that holds can handles and all of the queues
typedef struct
{
  CAN_HandleTypeDef* 	can;
  UART_HandleTypeDef* uart;

  QueueHandle_t 		q_rx_can;
  QueueHandle_t 		q_tx_can;
  QueueHandle_t 		q_rx_uart;
  QueueHandle_t 		q_tx_uart;
}lcd_t;

//Used to update the LCD screen
typedef struct
{
  uint16_t 	pack_volt;				//Most recent pack voltage
  uint8_t 	pack_soc;				//pack SOC
  uint8_t  	high_temp;				//the current highest temperature of a cell
}bms_data_t;

void task_lcd_main();
int btn_handler(uint8_t btn);
void error_blink();
void initRTOSObjects(void);

lcd_t lcd;

extern CAN_HandleTypeDef 	hcan1;
extern UART_HandleTypeDef 	huart2;
extern ADC_HandleTypeDef hadc1;

#endif /* LCD_H_ */
