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
*       1. uart.h
*
*     File Description: Implements uart for the dashboard. It uses interrupts
*     for rx and tx. The rx callback adds messages to the queue for processing
*     in lcd_main. The tx will update the LCD screen appropriately.
*
***************************************************************************/

#include "uart.h"

/***************************************************************************
*
*     Function Information
*
*     Name of Function: HAL_UART_RxCpltCallback
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type: None
*
*     Parameters (list data type, name, and comment one per line):
*       1. None
*       
*      Global Dependents:
*       1. None
*
*     Function Description: After RX_SIZE_UART bytes have been received allocate
*     a new rx struct and send to the back of the queue for lcd_main to process.
*
***************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
  uart_rx_t rx;
  rx.rx_size = RX_SIZE_UART;
  memcpy(rx.rx_buffer, myrx_data, rx.rx_size);
  xQueueSendToBackFromISR(lcd.q_rx_uart, &rx, 0);
  HAL_UART_Receive_IT(lcd.uart, myrx_data, RX_SIZE_UART); //start the receive
}

/***************************************************************************
*
*     Function Information
*
*     Name of Function: HAL_UART_TxCpltCallback
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type: 
*
*     Parameters (list data type, name, and comment one per line):
*       1. None
*       
*      Global Dependents:
*       1. None
*
*     Function Description: Does nothing. Could add error catching. 
*
***************************************************************************/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
  //do nothing and exit
}

/***************************************************************************
*
*     Function Information
*
*     Name of Function: task_txUart
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type: None
*
*     Parameters (list data type, name, and comment one per line):
*       1. None
*       
*      Global Dependents:
*       1. None
*
*     Function Description: Task runs at 10ms rate. Just continually checks for 
*     messages to send to the LCD and when there are it will send.
*
***************************************************************************/
void task_txUart()
{
  uart_tx_t tx;
  TickType_t time_init = 0;
  while (1) {
    time_init = xTaskGetTickCount();
    //check if this task is triggered
    if (xQueuePeek(lcd.q_tx_uart, &tx, TIMEOUT) == pdTRUE)
    {
      xQueueReceive(lcd.q_tx_uart, &tx, TIMEOUT);  //actually take item out of queue

      //send the message
      HAL_UART_Transmit_IT(lcd.uart, tx.tx_buffer, tx.tx_size);
      //This delay might be able to change. Used to prevent to much throughput to the Nextion
      HAL_Delay(DELAY_UART * 5);
    }

    vTaskDelayUntil(&time_init, TX_UART_RATE);
  }
}

/***************************************************************************
*
*     Function Information
*
*     Name of Function: update_lcd
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type: None
*
*     Parameters (list data type, name, and comment one per line):
*       1. None
*       
*      Global Dependents:
*       1. None
*
*     Function Description: Adds a tx message to the uart queue.
*
***************************************************************************/
void update_lcd(uint8_t* buffer, uint8_t size)
{
  uart_tx_t tx;
  tx.tx_size = size;
  memcpy(&tx.tx_buffer[0], buffer, size);
  while(uxQueueMessagesWaiting(lcd.q_tx_uart) == TX_UART_QUEUE_SIZE)
  {
    vTaskDelay(WAIT_QUEUE_FULL); //wait till space opens up
  }
  if (xQueueSendToBack(lcd.q_tx_uart, &tx, TIMEOUT) != pdTRUE)
  {
    //should never get here
    error_blink();
  }
}
