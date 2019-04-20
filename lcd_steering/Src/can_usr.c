/***************************************************************************
*
*     File Information
*
*     Name of File: lcd.c
*
*     Authors (Include Email):
*       1. Matthew Flanagan       matthewdavidflanagan@outlook.com
*
*     File dependents: (header files, flow charts, referenced documentation)
*       1. can.h
*
*     File Description: This manages all of the can being sent for the dashboard
*
***************************************************************************/

#include "can_usr.h"

/***************************************************************************
*
*     Function Information
*
*     Name of Function: HAL_CAN_RxFifo0MsgPendingCallback
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type: None
*
*     Parameters (list data type, name, and comment one per line):
*       1. CAN_HandleTypeDef *hcan      Can Handle
*
*      Global Dependents:
*       1. None
*
*     Function Description: After a message has been received add it to the
*     rx can queue and move on with life.
*
***************************************************************************/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CanRxMsgTypeDef rx;
  CAN_RxHeaderTypeDef header;
  HAL_CAN_GetRxMessage(hcan, 0, &header, rx.Data);
  rx.DLC = header.DLC;
  rx.StdId = header.StdId;
  xQueueSendFromISR(lcd.q_rx_can, &rx, 0);
//  HAL_GPIO_TogglePin(SUCCESS_GPIO_Port, SUCCESS_Pin);
}

/***************************************************************************
*
*     Function Information
*
*     Name of Function: HAL_CAN_RxFifo1MsgPendingCallback
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type: None
*
*     Parameters (list data type, name, and comment one per line):
*       1. CAN_HandleTypeDef *hcan      Can Handle
*
*      Global Dependents:
*       1. None
*
*     Function Description: After a message has been received add it to the
*     rx can queue and move on with life.
*
***************************************************************************/
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CanRxMsgTypeDef rx;
  CAN_RxHeaderTypeDef header;
  HAL_CAN_GetRxMessage(hcan, 0, &header, rx.Data);
  rx.DLC = header.DLC;
  rx.StdId = header.StdId;
  xQueueSendFromISR(lcd.q_rx_can, &rx, 0);
}

/***************************************************************************
*
*     Function Information
*
*     Name of Function: task_txCan
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type: None
*
*     Parameters (list data type, name, and comment one per line):
*       1. None
*
*      Global Dependents:
*       1. Can queue and such
*
*     Function Description: Task that runs at TX_CAN_RATE and polls for can
*     messages to arrive to send them out to the main module.
*
***************************************************************************/
void task_txCan()
{
  CanTxMsgTypeDef tx;
  TickType_t time_init = 0;
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
      //HAL_GPIO_TogglePin(SUCCESS_GPIO_Port, SUCCESS_Pin);
      //send the message
      while (!HAL_CAN_GetTxMailboxesFreeLevel(lcd.can)); // while mailboxes not free
      HAL_CAN_AddTxMessage(lcd.can, &header, tx.Data, &mailbox);
    }
    vTaskDelayUntil(&time_init, TX_CAN_RATE);
  }
}

/***************************************************************************
*
*     Function Information
*
*     Name of Function: can_filter_init
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type: None
*
*     Parameters (list data type, name, and comment one per line):
*       1. CAN_HandleTypeDef* hcan        Can Handle
*
*      Global Dependents:
*       1. None
*
*     Function Description: Sets the can filter to only take Messages from BMS
*     and MAIN. Only uses FIFO0. If more messages need to be read change FilterMaskIdHigh
*     and FilterMaskIdLow.
*
***************************************************************************/
void can_filter_init(CAN_HandleTypeDef* hcan)
{
  CAN_FilterTypeDef FilterConf;
  FilterConf.FilterIdHigh =         BMS_MSG_ID << 5;
  FilterConf.FilterIdLow =          MAIN_FAULT_ID << 5;
  FilterConf.FilterMaskIdHigh =     MAIN_ACK_ID << 5;       // 3
  FilterConf.FilterMaskIdLow =      0;       // 1
  FilterConf.FilterFIFOAssignment = CAN_FilterFIFO0;
  FilterConf.FilterBank = 0;
  FilterConf.FilterMode = CAN_FILTERMODE_IDLIST;
  FilterConf.FilterScale = CAN_FILTERSCALE_16BIT;
  FilterConf.FilterActivation = ENABLE;
  HAL_CAN_ConfigFilter(hcan, &FilterConf);
}
