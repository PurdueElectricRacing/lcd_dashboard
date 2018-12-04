/*
 * lcd.c
 *
 *  Created on: Nov 17, 2018
 *      Author: Matt Flanagan
 */
#include "lcd.h"

int btn_handler(uint8_t btn) {
	CanTxMsgTypeDef msg;
	msg.IDE = CAN_ID_STD;
	msg.RTR = CAN_RTR_DATA;
	msg.DLC = 1;

	msg.StdId = 0x350;
	msg.Data[0] = btn;

	xQueueSendToBack(lcd.q_tx_can, &msg, 100);
	return 0;
}

void error_blink() {
	while (1) {
		HAL_GPIO_TogglePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin);
		HAL_Delay(1000);
	}
}

void initRTOSObjects(void) {
	//initialize the queues
	lcd.q_rx_can = xQueueCreate(RX_CAN_QUEUE_SIZE, sizeof(CanRxMsgTypeDef));
	lcd.q_tx_can = xQueueCreate(TX_CAN_QUEUE_SIZE, sizeof(CanTxMsgTypeDef));
	lcd.q_rx_uart = xQueueCreate(RX_UART_QUEUE_SIZE, sizeof(uart_rx_t));
	lcd.q_tx_uart = xQueueCreate(TX_UART_QUEUE_SIZE, sizeof(uart_tx_t));

	lcd.q_rx_can_debug = (xQUEUE_my*) lcd.q_rx_can;
	lcd.q_tx_can_debug = (xQUEUE_my*) lcd.q_tx_can;
	lcd.q_rx_uart_debug = (xQUEUE_my*) lcd.q_rx_uart;
	lcd.q_tx_uart_debug = (xQUEUE_my*) lcd.q_tx_uart;

	//create tasks
	if (xTaskCreate(task_lcd_main, "Main Task", LCD_MAIN_STACK_SIZE, NULL, LCD_MAIN_PRIORTIY, NULL) != pdPASS) {
		error_blink();
	}
	if (xTaskCreate(task_txCan, "Tx Can Task", TX_CAN_STACK_SIZE, NULL, TX_CAN_PRIORITY, NULL) != pdPASS) {
		error_blink();
	}
	if (xTaskCreate(task_txUart, "TX Uart Task", TX_UART_STACK_SIZE, NULL, TX_UART_PRIORITY, NULL) != pdPASS) {
		error_blink();
	}
}

void task_lcd_main() {
	lcd.can = &hcan1;
	lcd.uart = &huart1;
	bms_data_t bms;
	CanRxMsgTypeDef rx_can;
	uint16_t counter = 0;
	TickType_t time_init = 0;
	TickType_t time_to_wait = 0;
	TickType_t time_fin = 0;
	uart_rx_t rx_uart;
	while (1) {
		time_init = xTaskGetTickCount();
		HAL_GPIO_TogglePin(SUCCESS_GPIO_Port, SUCCESS_Pin);
		//update_lcd(buff, 1);
		set_value("Char", 30);
		set_text("noti", "hello world!");
		//btn_handler(1);
		//handle message requests from the LCD screen
		if (xQueuePeek(lcd.q_rx_uart, &rx_uart, TIMEOUT) == pdTRUE) {
			//HAL_GPIO_TogglePin(SUCCESS_GPIO_Port, SUCCESS_Pin);
			xQueueReceive(lcd.q_rx_uart, &rx_uart, TIMEOUT);
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
		if (xQueuePeek(lcd.q_rx_can, &rx_can, TIMEOUT) == pdTRUE) {
			xQueueReceive(lcd.q_rx_can, &rx_can, TIMEOUT);

			switch (rx_can.StdId) {
				case BMS_MSG_ID:
				{
					//if Xth message then get ~1hz
					if (counter++ % LCD_UPDATE_RATE == 0) {
						//update the screen
						bms.pack_volt = (rx_can.Data[2] << 8) | rx_can.Data[3];
						bms.pack_soc = rx_can.Data[4];
						bms.high_temp = rx_can.Data[5];

						set_value("Char", bms.pack_soc);
						set_value("Volt", bms.pack_volt);
						set_value("Temp", bms.high_temp);
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
		time_fin =  xTaskGetTickCount();
		time_to_wait = (LCD_MAIN_RATE + time_init) - time_fin;
		time_to_wait = (LCD_MAIN_RATE + time_init)  < time_fin ? 0: time_to_wait;
		vTaskDelay(time_to_wait);
	}
}


