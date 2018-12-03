/*
 * uart.c
 *
 *  Created on: Dec 2, 2018
 *      Author: Matt Flanagan
 */
#include "uart.h"

void HAL_USART_RxCpltCallback(UART_HandleTypeDef* huart) {
	uart_rx_t rx;
	rx.rx_size = 6;
	rx.rx_buffer = memcpy(&rx.rx_buffer, huart->pRxBuffPtr, rx.rx_size);
	xQueueSendToBackFromISR(lcd.q_rx_uart, &rx, 0);
}

void HAL_USART_TxCpltCallback(UART_HandleTypeDef* huart) {
	//do nothing and exit
	__NOP();
}

void task_txUart() {
	uart_tx_t tx;
	tx.tx_size = 6;
	while (1) {
		//check if this task is triggered
			if (xQueuePeek(lcd.q_tx_uart, &tx, portMAX_DELAY) == pdTRUE)
			{
				xQueueReceive(lcd.q_tx_uart, &tx, portMAX_DELAY);  //actually take item out of queue
				HAL_UART_Transmit_IT(lcd.uart, tx.tx_buffer, tx.tx_size);
			}
	}
}
