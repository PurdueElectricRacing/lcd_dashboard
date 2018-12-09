/*
 * can.c
 *
 *  Created on: Dec 2, 2018
 *      Author: Matt Flanagan
 */
#include "can.h"

//can interrupt call back
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CanRxMsgTypeDef rx;
	CAN_RxHeaderTypeDef header;
	HAL_CAN_GetRxMessage(hcan, 0, &header, rx.Data);
	rx.DLC = header.DLC;
	rx.StdId = header.StdId;
	xQueueSendFromISR(lcd.q_rx_can, &rx, 0);
	HAL_GPIO_TogglePin(SUCCESS_GPIO_Port, SUCCESS_Pin);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CanRxMsgTypeDef rx;
	CAN_RxHeaderTypeDef header;
	HAL_CAN_GetRxMessage(hcan, 0, &header, rx.Data);
	rx.DLC = header.DLC;
	rx.StdId = header.StdId;
	xQueueSendFromISR(lcd.q_rx_can, &rx, 0);
}

void task_txCan()
{
	CanTxMsgTypeDef tx;
	TickType_t time_init = 0;
	TickType_t time_to_wait = 0;
	TickType_t time_fin = 0;
	while (1)
	{
			time_init = xTaskGetTickCount();
		//check if this task is triggered
			if (xQueuePeek(lcd.q_tx_can, &tx, TIMEOUT) == pdTRUE)
			{
				xQueueReceive(lcd.q_tx_can, &tx, TIMEOUT);  //actually take item out of queue
				CAN_TxHeaderTypeDef header;
				header.DLC = tx.DLC;
				header.IDE = tx.IDE;
				header.RTR = tx.RTR;
				header.StdId = tx.StdId;
				header.TransmitGlobalTime = DISABLE;
				uint32_t mailbox;

				//send the message
				while (!HAL_CAN_GetTxMailboxesFreeLevel(lcd.can)); // while mailboxes not free
				HAL_CAN_AddTxMessage(lcd.can, &header, tx.Data, &mailbox);
			}
			time_fin =  xTaskGetTickCount();
			time_to_wait = (TX_CAN_RATE + time_init) - time_fin;
			time_to_wait = (TX_CAN_RATE + time_init)  < time_fin ? 0: time_to_wait;
			vTaskDelay(time_to_wait);
	}
}

void can_filter_init(CAN_HandleTypeDef* hcan)
{
	CAN_FilterTypeDef FilterConf;
	FilterConf.FilterIdHigh =         BMS_MSG_ID << 5;
	FilterConf.FilterIdLow =          MAIN_FAULT_ID << 5;
	FilterConf.FilterMaskIdHigh =     0;       // 3
	FilterConf.FilterMaskIdLow =      0;       // 1
	FilterConf.FilterFIFOAssignment = CAN_FilterFIFO0;
	FilterConf.FilterBank = 0;
	FilterConf.FilterMode = CAN_FILTERMODE_IDLIST;
	FilterConf.FilterScale = CAN_FILTERSCALE_16BIT;
	FilterConf.FilterActivation = ENABLE;
	HAL_CAN_ConfigFilter(hcan, &FilterConf);
}
