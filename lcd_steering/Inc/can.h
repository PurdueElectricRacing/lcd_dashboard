/*
 * can.h
 *
 *  Created on: Dec 2, 2018
 *      Author: Matt Flanagan
 */

#ifndef CAN_H_
#define CAN_H_

#include "lcd.h"
#include "cmsis_os.h"

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
int send_can_msg(CanTxMsgTypeDef* msg);
void task_txCan();
void can_filter_init();

#endif /* CAN_H_ */
