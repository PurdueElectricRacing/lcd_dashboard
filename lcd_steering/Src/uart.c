/*
 * uart.c
 *
 *  Created on: Dec 2, 2018
 *      Author: Matt Flanagan
 */
#include "uart.h"

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
	uart_rx_t rx;
	rx.rx_size = RX_SIZE_UART;
	rx.rx_buffer = malloc(sizeof(*rx.rx_buffer) * RX_SIZE_UART);
	memcpy(rx.rx_buffer, myrx_data, rx.rx_size);
	xQueueSendToBackFromISR(lcd.q_rx_uart, &rx, 0);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
	//do nothing and exit
}

void task_txUart() {
	uart_tx_t tx;
	TickType_t time_init = 0;
	TickType_t time_to_wait = 0;
	TickType_t time_fin = 0;
	uint8_t* temp_pt = NULL;
	while (1) {
			time_init = xTaskGetTickCount();
		//check if this task is triggered
			if (xQueuePeek(lcd.q_tx_uart, &tx, TIMEOUT) == pdTRUE)
			{
				xQueueReceive(lcd.q_tx_uart, &tx, TIMEOUT);  //actually take item out of queue
				temp_pt = tx.tx_buffer;
				HAL_UART_Transmit_IT(lcd.uart, tx.tx_buffer, tx.tx_size);
				HAL_Delay(DELAY_UART * 5);
				free(temp_pt);
			}
			time_fin =  xTaskGetTickCount();
			time_to_wait = (TX_UART_RATE + time_init) - time_fin;
			time_to_wait = (TX_UART_RATE + time_init)  < time_fin ? 0: time_to_wait;
			vTaskDelay(time_to_wait);
	}
}

void update_lcd(uint8_t* buffer, uint8_t size) {
	uart_tx_t tx;
	tx.tx_size = size;
	tx.tx_buffer = buffer;
	while(uxQueueMessagesWaiting(lcd.q_tx_uart) == TX_UART_QUEUE_SIZE) {
		vTaskDelay(WAIT_QUEUE_FULL); //wait till space opens up
	}
	if (xQueueSendToBack(lcd.q_tx_uart, &tx, TIMEOUT) != pdTRUE) {
		//should never get here
		free(buffer);
		error_blink();
	}
}
