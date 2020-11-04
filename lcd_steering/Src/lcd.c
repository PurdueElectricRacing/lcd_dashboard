/***************************************************************************
*
*     File Information
*
*     Name of File: lcd.c
*
*     Authors (Include Email):
*       1. Matthew Flanagan       matthewdavidflanagan@outlook.com
*       2. Josh Damman            shao77@purdue.edu
*
*     File dependents: (header files, flow charts, referenced documentation)
*       1. lcd.h
*
*     File Description: This is the main file for the lcd dashborad. It has a
*     main task that looks for any messages coming in through CAN or UART. It
*     also creates all of the freeRTOS objects and tasks.
*
***************************************************************************/

#include "lcd.h"

/***************************************************************************
*
*     Function Information
*
*     Name of Function: btn_handler
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type: int 0 if success
*
*     Parameters (list data type, name, and comment one per line):
*       1. uint8_t btn  which button from the lcd screen was pressed
*           
*       
*      Global Dependents:
*       1. None
*
*     Function Description: Creates and adds a can message to the tx queue 
*     to be sent to main module.
*
***************************************************************************/
int btn_handler(uint8_t btn)
{
  CanTxMsgTypeDef msg;
  msg.IDE = CAN_ID_STD;
  msg.RTR = CAN_RTR_DATA;
  msg.DLC = 1;
  msg.StdId = START_MSG_ID;
  msg.Data[0] = btn;

  xQueueSendToBack(lcd.q_tx_can, &msg, 100);
  return 0;
}

/***************************************************************************
*
*     Function Information
*
*     Name of Function: error_blink
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type: None
*
*     Parameters (list data type, name, and comment one per line):
*       1.None
*       
*      Global Dependents:
*       1. None
*
*     Function Description: Error catching for debugging purposes.
*
***************************************************************************/
void error_blink()
{
  while (1)
  {
    //HAL_GPIO_TogglePin(ERROR_LED_GPIO_Port, ERROR_LED_Pin);
    HAL_Delay(1000);
  }
}

/***************************************************************************
*
*     Function Information
*
*     Name of Function: initRTOSObjects
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
*     Function Description: Creates the queues for rx and tx on CAN and UART.
*     
*
***************************************************************************/
void initRTOSObjects(void)
{
    //initialize the queues
  lcd.q_rx_can = xQueueCreate(RX_CAN_QUEUE_SIZE, sizeof(CanRxMsgTypeDef));
  lcd.q_tx_can = xQueueCreate(TX_CAN_QUEUE_SIZE, sizeof(CanTxMsgTypeDef));
  lcd.q_rx_uart = xQueueCreate(RX_UART_QUEUE_SIZE, sizeof(uart_rx_t));
  lcd.q_tx_uart = xQueueCreate(TX_UART_QUEUE_SIZE, sizeof(uart_tx_t));

  //create tasks
  if (xTaskCreate(task_lcd_main, "Main Task", LCD_MAIN_STACK_SIZE, NULL, LCD_MAIN_PRIORTIY, NULL) != pdPASS)
  {
      error_blink();
  }
  if (xTaskCreate(task_txCan, "Tx Can Task", TX_CAN_STACK_SIZE, NULL, TX_CAN_PRIORITY, NULL) != pdPASS)
  {
      error_blink();
  }
  if (xTaskCreate(task_txUart, "TX Uart Task", TX_UART_STACK_SIZE, NULL, TX_UART_PRIORITY, NULL) != pdPASS)
  {
      error_blink();
  }
  if (xTaskCreate(taskPollSteer, "Steer Sensor Task", STEER_STACK_SIZE, NULL, STEER_PRIORITY, NULL) != pdPASS)
  {
  		error_blink();
  }
}

/***************************************************************************
*
*     Function Information
*
*     Name of Function: task_lcd_main
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type:  None
*
*     Parameters (list data type, name, and comment one per line):
*       1. None
*       
*      Global Dependents:
*       1. Can handle and Uart Handle
*
*     Function Description: Main task that will check if any messages have been
*     received from can or the lcd screen. This currently is set to run at a 100 Hz.
*
***************************************************************************/
void task_lcd_main()
{
  lcd.can = &hcan1;
  lcd.uart = &huart2;
  page_t page = START;
  bms_data_t bms;
  CanRxMsgTypeDef rx_can;
  uint16_t counter = 0;
  uint16_t counter_status = 0;
  uint8_t main_fault_code = 0;
  TickType_t time_init = 0;
  uart_rx_t rx_uart;
  uart_tx_t tx_uart;
  HAL_UART_Receive_IT(lcd.uart, myrx_data, RX_SIZE_UART); //start the receive

  while (1) 
  {
    time_init = xTaskGetTickCount(); // get the initial time of the task

    if (counter_status++ % 100 == 0)
    {
    	//btn_handler(1);
    	//HAL_GPIO_TogglePin(TRACTION_LED_GPIO_Port, TRACTION_LED_Pin);
    }

    //handle message requests from the LCD screen
    if (xQueuePeek(lcd.q_rx_uart, &rx_uart, TIMEOUT) == pdTRUE)
    {
      //HAL_GPIO_TogglePin(SUCCESS_GPIO_Port, SUCCESS_Pin);
      xQueueReceive(lcd.q_rx_uart, &rx_uart, TIMEOUT);

      //determine which button was pressed
      if (rx_uart.rx_buffer[1] == START_ID_0 && rx_uart.rx_buffer[2] == START_ID_1)
      {
        btn_handler(1);
      } else if (rx_uart.rx_buffer[1] == STOP_ID_0 && rx_uart.rx_buffer[2] == STOP_ID_1)
      {
        btn_handler(1);
      } else if (rx_uart.rx_buffer[1] == ACTIVE_AERO_ID_0 && rx_uart.rx_buffer[2] == ACTIVE_AERO_ID_1)
      {
        btn_handler(2);

      } else if (rx_uart.rx_buffer[1] == ECO_MODE_ID_0 && rx_uart.rx_buffer[2] == ECO_MODE_ID_1)
      {
        btn_handler(3);
      } else if (rx_uart.rx_buffer[1] == RACE_MODE_ID_0 && rx_uart.rx_buffer[2] == RACE_MODE_ID_1)
      {
        btn_handler(4);
      } else if (rx_uart.rx_buffer[1] == SPORT_MODE_ID_0 && rx_uart.rx_buffer[2] == SPORT_MODE_ID_1)
      {
        btn_handler(5);
      }
    }

    //receive can messages and update the lcd screen as necessary
    //Live SOC/Voltage/Temperature
    if (xQueuePeek(lcd.q_rx_can, &rx_can, TIMEOUT) == pdTRUE)
    {
      xQueueReceive(lcd.q_rx_can, &rx_can, TIMEOUT);

      switch (rx_can.StdId)
      {
        case BMS_MSG_ID:
        {
          //if Xth message then get ~1hz
          if (counter++ % LCD_UPDATE_RATE == 0)
          {
            //update the screen
            bms.pack_volt = ((rx_can.Data[2] << 8) | rx_can.Data[3]) / 10;
            bms.pack_soc = (rx_can.Data[4]) / 2;
            bms.high_temp = rx_can.Data[5];

            set_value("Char", bms.pack_soc);
            set_value("Volt", bms.pack_volt);
            set_value("Temp", bms.high_temp);

            //temp color
            if (bms.high_temp > BMS_OVER_TEMP_RED)
            {
              set_bco("Temp", RED);
            }
            else if (bms.high_temp > BMS_OVER_TEMP_YEL)
            {
              set_bco("Temp", YELLOW);
            }
            else
            {
              set_bco("Temp", GREEN);
            }
            //volt color
            if (bms.pack_volt < BMS_UNDER_VOLT_RED)
            {
              set_bco("Volt", RED);
            }
            else
            {
              set_bco("Char", GREEN);
            }

            if (bms.pack_soc < BMS_SOC_RED)
            {
              set_bco("Char", RED);
            }
            else if (bms.pack_soc < BMS_SOC_YEL)
            {
              set_bco("Char", YELLOW);
            }
            else
            {
              set_bco("Char", GREEN);
            }
          }
          break;
        }
        case MAIN_FAULT_ID:
        {
          //display whatever main faults to the screen
          if ((counter_status % 5) == 0 && page == 1) {
            main_fault_code = rx_can.Data[0];
            sprintf((char*) &tx_uart.tx_buffer[0], "Main Faults: %x ", main_fault_code & 0xff);
            set_text("noti", (char *) &tx_uart.tx_buffer[0]);
          }
          break;
        }
        case MAIN_ACK_ID:
        {
        	//start message accepted change state to ready to drive
        	if (page == START && rx_can.Data[0] == 1) {
        		page = RACE;
        		set_page("Race");
        	} else if (page == RACE && rx_can.Data[0] == 2){
        		page = START;
        		set_page("Start");
        	}
        	break;
        }
      }
    }

    vTaskDelayUntil(&time_init, LCD_MAIN_RATE);
  }
}


