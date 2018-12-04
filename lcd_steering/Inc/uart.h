/*
 * uart.h
 *
 *  Created on: Dec 2, 2018
 *      Author: Matt Flanagan
 */

#ifndef UART_H_
#define UART_H_
#include "lcd.h"

//standard rx size constant
#define RX_SIZE_UART 6
#define DELAY_UART	 10 / portTICK_RATE_MS

typedef struct {
	uint8_t* rx_buffer;
	uint16_t rx_size;
}uart_rx_t;

typedef struct {
	uint8_t* tx_buffer;
	uint16_t tx_size;
}uart_tx_t;


void HAL_USART_RxCpltCallback(UART_HandleTypeDef* huart);
void HAL_USART_TxCpltCallback(UART_HandleTypeDef* huart);
void task_txUart();
void update_lcd(uint8_t* buffer, uint8_t size);

extern uint8_t myrx_data[];

#endif /* UART_H_ */
