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
#include "main.h"
#include <stdio.h>

#define START_ID 		0x0001 //start button ID
#define STOP_ID			0x0106 //stop button ID
#define ACTIVE_AERO_ID 	0x0102 //active aero ID
#define ECO_MODE_ID		0x0202 //ECO Mode ID
#define RACE_MODE_ID	0x0203 //Race Mode ID
#define SPORT_MODE_ID	0x0204 //Sport Mode ID


uint8_t start_flag;
uint8_t stop_flag;
uint8_t race_mode;
uint8_t eco_mode;
uint8_t sport_mode;
uint8_t active_aero;

typedef struct {
	uint8_t* rx_buffer;
	uint16_t rx_size;
}uart_rx_t;

/**
  * @brief  CAN Tx message structure definition
  */
typedef struct
{
  uint32_t StdId;    /*!< Specifies the standard identifier.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0x7FF */

  uint32_t ExtId;    /*!< Specifies the extended identifier.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0x1FFFFFFF */

  uint32_t IDE;      /*!< Specifies the type of identifier for the message that will be transmitted.
                          This parameter can be a value of @ref CAN_Identifier_Type */

  uint32_t RTR;      /*!< Specifies the type of frame for the message that will be transmitted.
                          This parameter can be a value of @ref CAN_remote_transmission_request */

  uint32_t DLC;      /*!< Specifies the length of the frame that will be transmitted.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 8 */

  uint8_t Data[8];   /*!< Contains the data to be transmitted.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0xFF */

}CanTxMsgTypeDef;

typedef struct {
	CAN_HandleTypeDef* can;
	UART_HandleTypeDef* uart;
}lcd_t;

int lcd_main(void);
int send_can_msg(CanTxMsgTypeDef* msg);
int btn_handler(uint8_t btn);

lcd_t lcd;

extern uart_rx_t rx_buff;
extern CAN_HandleTypeDef hcan1;
extern UART_HandleTypeDef huart1;

#endif /* LCD_H_ */
