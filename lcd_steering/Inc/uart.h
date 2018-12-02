/*
 * uart.h
 *
 *  Created on: Dec 2, 2018
 *      Author: Matt Flanagan
 */

#ifndef UART_H_
#define UART_H_
#include "lcd.h"

void HAL_USART_RxCpltCallback(UART_HandleTypeDef* huart);
void HAL_USART_TxCpltCallback(UART_HandleTypeDef* huart);
void task_txUart();

#endif /* UART_H_ */
