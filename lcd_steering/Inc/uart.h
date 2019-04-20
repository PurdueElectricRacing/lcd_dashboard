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
*
*     File Description: All the constants required for UART peripheral on the 
*     dashboard.
*
***************************************************************************/
#ifndef UART_H_
#define UART_H_
#include "lcd.h"

//standard rx size constant
#define RX_SIZE_UART 7
#define TX_MAX_LEN	 30
#define DELAY_UART	 10 / portTICK_RATE_MS

typedef struct
{
  uint8_t rx_buffer[RX_SIZE_UART];
  uint16_t rx_size;
}uart_rx_t;

typedef struct
{
  uint8_t tx_buffer[TX_MAX_LEN];
  uint16_t tx_size;
}uart_tx_t;


void task_txUart();
void update_lcd(uint8_t* buffer, uint8_t size);

extern uint8_t myrx_data[];

#endif /* UART_H_ */
