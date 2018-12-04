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
	rx.rx_buffer = malloc(sizeof(*rx.rx_buffer) * RX_SIZE_UART);
	memcpy(rx.rx_buffer, myrx_data, rx.rx_size);
	xQueueSendToBackFromISR(lcd.q_rx_uart, &rx, 0);
}

void HAL_USART_TxCpltCallback(UART_HandleTypeDef* huart) {
	//do nothing and exit
	__NOP();
}

void task_txUart() {
	uart_tx_t tx;
	TickType_t time_init = 0;
	while (1) {
			time_init = xTaskGetTickCount();
		//check if this task is triggered
			if (xQueuePeek(lcd.q_tx_uart, &tx, portMAX_DELAY) == pdTRUE)
			{
				xQueueReceive(lcd.q_tx_uart, &tx, portMAX_DELAY);  //actually take item out of queue
				HAL_UART_Transmit_IT(lcd.uart, tx.tx_buffer, tx.tx_size);
			}
			vTaskDelayUntil(&time_init, TX_UART_RATE);
	}
}

void update_lcd(uint8_t* buffer, uint8_t size) {
	uart_tx_t tx;
	tx.tx_size = size;
	tx.tx_buffer = malloc(sizeof(*tx.tx_buffer) * size);
	memcpy(tx.tx_buffer, buffer, size);
	if (xQueueSendToBack(lcd.q_tx_uart, &tx, 100) != pdTRUE) {
		error_blink();
	}
}
