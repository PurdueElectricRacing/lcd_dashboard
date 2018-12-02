/*
 * lcd.c
 *
 *  Created on: Nov 17, 2018
 *      Author: Matt Flanagan
 */
#include "lcd.h"

int send_can_msg(CanTxMsgTypeDef * msg) {
	CAN_TxHeaderTypeDef header;
	header.DLC = msg->DLC;
	header.IDE = msg->IDE;
	header.RTR = msg->RTR;
	header.StdId = msg->StdId;
	header.TransmitGlobalTime = DISABLE;
	uint32_t mailbox;
	while (!HAL_CAN_GetTxMailboxesFreeLevel(lcd.can)); // while mailboxes not free
	HAL_CAN_AddTxMessage(lcd.can, &header, msg->Data, &mailbox);

	return 0;
}

int btn_handler(uint8_t btn) {
	char* btns[6] = {"start", "stop", "active aero", "eco", "race", "sport"};
	printf("%s button\n", btns[btn-1]);
	CanTxMsgTypeDef msg;
	msg.IDE = CAN_ID_STD;
	msg.RTR = CAN_RTR_DATA;
	msg.DLC = 1;

	msg.StdId = 0x350;

	msg.Data[0] = btn;

	send_can_msg(&msg);
	return 0;
}

int lcd_main(void) {
	lcd.can = &hcan1;
	lcd.uart = &huart1;
	HAL_UART_Receive_IT(lcd.uart, myrx_data, 7);
	mytx_data[0] = 'h';
	mytx_data[1] = 'e';
	mytx_data[2] = 'l';
	mytx_data[3] = 'l';
	mytx_data[4] = 'o';
	mytx_data[5] = '!';
	mytx_data[6] = '\n';
	while (1) {
		HAL_Delay(500);
		HAL_GPIO_TogglePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin);
		HAL_UART_Transmit_IT(lcd.uart, mytx_data, 7);
		HAL_Delay(500);
		HAL_GPIO_TogglePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin);

		if (start_flag) {
			btn_handler(1);
			start_flag = 0;
		} else if (stop_flag) {
			btn_handler(2);
			stop_flag = 0;
		} else if (active_aero) {
			btn_handler(3);
			active_aero = 0;
		} else if (eco_mode) {
			btn_handler(4);
			eco_mode = 0;
		} else if (race_mode) {
			btn_handler(5);
			race_mode = 0;
		} else if (sport_mode) {
			btn_handler(6);
			sport_mode = 0;
		}
	}
	return 0;
}
