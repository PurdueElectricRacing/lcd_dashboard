/*
 * lcd.c
 *
 *  Created on: Nov 17, 2018
 *      Author: Matt Flanagan
 */
#include "lcd.h"

int btn_handler(uint8_t btn) {
	char* btns[6] = {"start", "stop", "active aero", "eco", "race", "sport"};
	printf("%s button\n", btns[btn-1]);
	CanTxMsgTypeDef msg;
	msg.IDE = CAN_ID_STD;
	msg.RTR = CAN_RTR_DATA;
	msg.DLC = 1;

	msg.StdId = 0x350;
	msg.Data[0] = btn;

	xQueueSendToBack(lcd.q_tx_can, &msg, 100);
	return 0;
}

void initRTOSObjects(void) {
	//initialize the queues
	lcd.q_rx_can = xQueueCreate(RX_CAN_QUEUE_SIZE, sizeof(CanRxMsgTypeDef));
	lcd.q_tx_can = xQueueCreate(TX_CAN_QUEUE_SIZE, sizeof(CanTxMsgTypeDef));
	lcd.q_rx_uart = xQueueCreate(RX_UART_QUEUE_SIZE, sizeof(uart_rx_t));
	lcd.q_tx_uart = xQueueCreate(TX_UART_QUEUE_SIZE, sizeof(uart_tx_t));

	//create tasks
	xTaskCreate(lcd_main, "Main Task", 256, NULL, 1, NULL);
	xTaskCreate(task_txCan, "Tx Can Task", 256, NULL, 1, NULL);
	xTaskCreate(task_txUart, "TX Uart Task", 256, NULL, 1, NULL);
}

int lcd_main(void) {
	lcd.can = &hcan1;
	lcd.uart = &huart1;

	CanRxMsgTypeDef rx_can;
	uint16_t counter = 0;
	uart_rx_t rx_uart;
	while (1) {
		//handle message requests from the LCD screen
		if (xQueuePeek(lcd.q_rx_uart, &rx_uart, portMAX_DELAY) == pdTRUE) {
			xQueueReceive(lcd.q_rx_uart, &rx_uart, portMAX_DELAY);
			if (rx_uart.rx_buffer[0] == START_ID_0 && rx_uart.rx_buffer[1] == START_ID_1) {
				btn_handler(1);
			} else if (rx_uart.rx_buffer[0] == STOP_ID_0 && rx_uart.rx_buffer[1] == STOP_ID_1) {
				btn_handler(2);
			} else if (rx_uart.rx_buffer[0] == ACTIVE_AERO_ID_0 && rx_uart.rx_buffer[1] == ACTIVE_AERO_ID_1) {
				btn_handler(3);
			} else if (rx_uart.rx_buffer[0] == ECO_MODE_ID_0 && rx_uart.rx_buffer[1] == ECO_MODE_ID_1) {
				btn_handler(4);
			} else if (rx_uart.rx_buffer[0] == RACE_MODE_ID_0 && rx_uart.rx_buffer[1] == RACE_MODE_ID_1) {
				btn_handler(5);
			} else if (rx_uart.rx_buffer[0] == SPORT_MODE_ID_0 && rx_uart.rx_buffer[1] == SPORT_MODE_ID_1) {
				btn_handler(6);
			}
		}

		//receive can messages and update the lcd screen as necessary
		//Live SOC/Voltage/Temperature
		if (xQueuePeek(lcd.q_rx_can, &rx_can, portMAX_DELAY) == pdTRUE) {
			xQueueReceive(lcd.q_rx_can, &rx_can, portMAX_DELAY);

			switch (rx_can.StdId) {
				case BMS_MSG_ID:
				{
					//if Xth message then get ~1hz
					if (counter++ % LCD_UPDATE_RATE == 0) {
						//update the screen
						//TODO how to send
					}
					break;
				}
				case MAIN_FAULT_ID:
				{
					//display whatever main faults to the screen
					//TODO how to send
					break;
				}
			}
		}



	}
	return 0;
}
