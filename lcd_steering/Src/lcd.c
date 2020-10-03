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
	tx.IDE =  	CAN_ID_STD;            //information relating to CAN frame
	tx.RTR =	CAN_RTR_DATA;          //information relating to CAN frame
	tx.DLC =  	1;                     //Number of bits the buttons need
	tx.Data[0] = 0x01;	               //information relating to CAN frame
if (btn == 1)					//checks if button 1 is pressed
{
	tx.StdId = START_BUTTON;	//sends START_BUTTON can frame
}
else if (btn == 2)              //checks if button 2 is pressed if button 1 is not
{
	 tx.StdId = TC_TV;          //sends TC_TV can frame
}
xQueueSendToBack(car.q_txcan, &tx, 100);	//Queue for can frames

  CanTxMsgTypeDef msg;
  msg.IDE = CAN_ID_STD;
  msg.RTR = CAN_RTR_DATA;
  msg.DLC = 1;
  msg.StdId = START_MSG_ID;
  msg.Data[0] = btn;

  xQueueSendToBack(lcd.q_tx_can, &msg, 100);
  return 0;
  tx.StdId = 	START_BUTTON;
  tx.IDE =  	CAN_ID_STD;
  tx.RTR =	CAN_RTR_DATA;
  tx.DLC =  	1;
  tx.Data[0] = 0x01;
  xQueueSendToBack(car.q_txcan, &tx, 100);

  tx.StdId = 	TC_TV;
  tx.IDE =  	CAN_ID_STD;
  tx.RTR =	CAN_RTR_DATA;
  tx.DLC =  	1;
  tx.Data[0] = 0x01;
    xQueueSendToBack(car.q_txcan, &tx, 100);
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
    	HAL_GPIO_TogglePin(TRACTION_LED_GPIO_Port, TRACTION_LED_Pin);
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
void task_lcd_main()
{
  lcd.can = &hcan1;
  lcd.uart = &huart2;
  lcd.data.speed = 100;
  lcd.data.pack_volt = 310;
  lcd.data.precharge = 1;
  lcd.data.imd = 0;
  lcd.data.bms = 0;
  char outBuffer[8];
  page_t page = RACE;
  status_t state = OFF;
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
        HAL_GPIO_TogglePin(TRACTION_LED_GPIO_Port, TRACTION_LED_Pin);
    }

    //Handle message requests from the LCD screen
    if (xQueuePeek(lcd.q_rx_uart, &rx_uart, TIMEOUT) == pdTRUE)
    {
      xQueueReceive(lcd.q_rx_uart, &rx_uart, TIMEOUT);

      //Determine which button was pressed
      if (page == RACE && rx_uart.rx_buffer[1] == START_ID_0 && rx_uart.rx_buffer[2] == START_ID_1 && state == OFF)
      {
          //We should handle the changing of pages and text through the LCD rather than UART
//        set_text("statusLabel", "Car State: ON");
//        set_text("startButton", "STOP");
          state = ON;
          btn_handler(1);
      } else if (page == RACE && rx_uart.rx_buffer[1] == START_ID_0 && rx_uart.rx_buffer[2] == START_ID_1 && state == ON)
      {
          //Again, handle with LCD rather than UART
//        set_text("statusLabel", "Car State: OFF");
//        set_text("startButton", "START");
          state = OFF;
          btn_handler(1);
      } else if (page == RACE && rx_uart.rx_buffer[1] == SETTINGS_ID_0 && rx_uart.rx_buffer[2] == SETTINGS_ID_1)
      {
          page = SETTINGS;
      } else if (page == RACE && rx_uart.rx_buffer[1] == INFO_ID_0 && rx_uart.rx_buffer[2] == INFO_ID_1)
      {
          page = INFO;
      } else if (page == RACE && rx_uart.rx_buffer[1] == LAP_ID_0 && rx_uart.rx_buffer[2] == LAP_ID_1)
      {
          page = LAPS;
      } else if ((page == INFO || page == LAPS || page == SETTINGS) && rx_uart.rx_buffer[2] == BACK_ID)
      {
          page = RACE;
      } else if (page == RACE && rx_uart.rx_buffer[1] == YEET_ID_0 && rx_uart.rx_buffer[2] == YEET_ID_1)
      {
          //Todo: Tell the user launch control is on
          btn_handler(2);
      }
    }

//    //receive can messages and update the lcd screen as necessary
//    //Live SOC/Voltage/Temperature
      if (QueuePeek (lcd.q_rx_can, &rx_can, TIMEOUT) == pdTRUE)
     {
    	  xQueueReceive (lcd.q_rx_can, &rx_can, TIMEOUT);

    	  switch (rx_can.StdId)
    	  {
    	  case BMS_AVG_CELL_VOLTAGE:
		  {
			data.avg_volts_bms = 0x0000 | (rx_can.Data[0]) | ((rx.Data[1]) << 8); //avg volts
		  }
    	  case BMS_MAX_CELL_VOLTAGE:
		  {
    		data.max_volts_bms = 0x0000 | (rx_can.Data[0]) | ((rx.Data[1]) << 8); //max volts
		  }
    	  case BMS_LOW_CELL_VOLTAGE:
		  {
			 data.low_volts_bms = 0x0000 | (rx_can.Data[0]) | ((rx.Data[1]) << 8); //low volts
		  }
    	  case BMS_AVG_TEMP:
		  {
			data.avg_temp_bms = 0x0000 | (rx_can.Data[0]) | ((rx.Data[1]) << 8);  //avg temp
		  }
    	  case BMS_HIGH_TEMP
		  {
    		data.high_temp_bms = 0x0000 | (rx_can.Data[0]) | ((rx.Data[1]) << 8); //high temp
		  }
    	  case BMS_LOW_TEMP:
		  {
			data.low_temp_bms = 0x0000 | (rx_can.Data[0]) | ((rx.Data[1]) << 8); //low temp
		  }
    	  case BMS_PACK_CURRENT:
		  {
			data.pack_current_bms = 0x0000 | (rx_can.Data[0]) | ((rx.Data[1]) << 8); //pack current
		  }
    	  case BMS_INSTANT_PACK_VOLT:
		  {
			data.instant_pack_volt_bms = 0x0000 | (rx_can.Data[0]) | ((rx.Data[1]) << 8); //instant pack volt
		  }
    	  case BMS_PACK_CURRENT_LOW:
    	  {
    		data.pack_current_low_bms = 0x0000 | (rx_can.Data[1]) | ((rx.Data[0]) <<8); //pack current low
    	  }
    	  case BMS_PACK_DISCH_LIM
		  {
    		data.pack_disch_lim_bms = 0x0000 | (rx_can.Data[1]) | ((rx.Data[0]) <<8); //pack disch lim
		  }
    	  case BMS_SOC_CELL_LIMS
		  {
    	    data.soc_cell_lim_bms = 0x0000 | (rx_can.Data[1]) | ((rx.Data[0]) <<8); //soc cell lim
		  }
    	  case MAIN_MODULE_MAIN_HEARTBEAT        //main module
		  {
    		data.car_state = (rx_can.Data[0]);      //car state
    	    data.PC_status = ((rx.Data[2]));        //PC status
		  }
    	  case DAQ2_FRONT_WHEEL_SPEED_FRONT
		  {
    		data.wheel_speed_front_l = 0x0000 | (rx_can.Data[3]) | ((rx_can.Data[2]) <<8) | ((rx_can.Data[1]) <<16) | ((rx_can.Data[0] <<24));
    		data.wheel_speed_front_r = 0x0000 | (rx_can.Data[7]) | ((rx_can.Data[6]) <<8) | ((rx_can.Data[5]) <<16) | ((rx_can.Data[4] <<24));
		  }
    	  case DAQ2_REAR_WHEEL_SPEED_REAR
		  {
    		  data.wheel_speed_rear_l = 0x0000 | (rx_can.Data[3]) | ((rx_can.Data[2]) <<8) | ((rx_can.Data[1]) <<16) | ((rx_can.Data[0] <<24));
    		  data.wheel_speed_rear_r = 0x0000 | (rx_can.Data[7]) | ((rx_can.Data[6]) <<8) | ((rx_can.Data[5]) <<16) | ((rx_can.Data[4] <<24));
		  }
    	  CanTxMsgTypeDef tx;
    	  tx.StdId = 	START_BUTTON;
    	  tx.IDE =  	CAN_ID_STD;
    	  tx.RTR =	CAN_RTR_DATA;
    	  tx.DLC =  	1;
    	  tx.Data[0] = 0xab;
    	  xQueueSendToBack(car.q_txcan, &tx, 100);

    	  tx.StdId = 	TC_TV;
    	  tx.IDE =  	CAN_ID_STD;
    	  tx.RTR =	CAN_RTR_DATA;
    	  tx.DLC =  	1;
    	  tx.Data[0] = 0xab;
    	  xQueueSendToBack(car.q_txcan, &tx, 100);
//    if (xQueuePeek(lcd.q_rx_can, &rx_can, TIMEOUT) == pdTRUE)
//    {
//      xQueueReceive(lcd.q_rx_can, &rx_can, TIMEOUT);
//
//      switch (rx_can.StdId)
//      {
//        case BMS_MSG_ID:
//        {
//          //if Xth message then get ~1hz
//          if (counter++ % LCD_UPDATE_RATE == 0)
//          {
//            //update the screen
//            bms.pack_volt = ((rx_can.Data[2] << 8) | rx_can.Data[3]) / 10;
//            bms.pack_soc = (rx_can.Data[4]) / 2;
//            bms.high_temp = rx_can.Data[5];
//
//            set_value("Char", bms.pack_soc);
//            set_value("Volt", bms.pack_volt);
//            set_value("Temp", bms.high_temp);
//          }
//          break;
//        }
//        case MAIN_FAULT_ID:
//        {
//          //display whatever main faults to the screen
//          if ((counter_status % 5) == 0 && page == 1) {
//            main_fault_code = rx_can.Data[0];
//            sprintf((char*) &tx_uart.tx_buffer[0], "Main Faults: %x ", main_fault_code & 0xff);
//            set_text("noti", (char *) &tx_uart.tx_buffer[0]);
//          }
//          break;
//        }
//        case MAIN_ACK_ID:
//        {
//          //start message accepted change state to ready to drive
//          if (page == RACE && rx_can.Data[0] == 1) {
//              page = RACE;
//              set_page("main");
//          } else if (page == RACE && rx_can.Data[0] == 2){
//              page = RACE;
//              set_page("main");
//          }
//          break;
//        }
//      }
//    }

    //Update the values on the LCD
    if (page == RACE)
    {
        set_value("speed", lcd.data.speed);
        set_value("voltage", lcd.data.pack_volt);
        set_value("batteryBar", (int)(0.666 * lcd.data.pack_volt - 200));
        if (lcd.data.pack_volt > 382)
        {
            //Set the voltage labels to green
            set_pco("voltage", 0x06E0);
            set_pco("voltLabel", 0x06E0);
        }
        else if (lcd.data.pack_volt <= 382 && lcd.data.pack_volt > 316)
        {
            //Set the voltage labels to yellow
            set_pco("voltage", 0xDEC0);
            set_pco("voltLabel", 0xDEC0);
        }
        else
        {
            //Set the voltage labels to red
            set_pco("voltage", 0xF800);
            set_pco("voltLabel", 0xF800);
        }
    }
    else if (page == INFO)
    {
        set_value("pvVar", lcd.data.pack_volt);
        set_value("socVar", lcd.data.pack_soc);
        set_value("sohVar", lcd.data.pack_soh);
        set_value("minVar", lcd.data.min);
        set_value("maxVar", lcd.data.max);
        if (lcd.data.state == ON)
        {
            strcpy(outBuffer, "BIG YEET");
        } else {
            strcpy(outBuffer, "OFF");
        }
        set_text("carState", outBuffer);
        set_pic("prechargeStat", 6 + lcd.data.precharge);
        set_pic("imdStat", 6 + lcd.data.imd);
        set_pic("bmsStat", 6 + lcd.data.precharge);
        sprintf(outBuffer, "%3d mph", lcd.data.maxSpeed);
        set_text("maxStat", outBuffer);
        sprintf(outBuffer, "%3d mph", lcd.data.avgSpeed);
        set_text("avgStat", outBuffer);
    }
    else if (page == SETTINGS)
    {
        //No dynamic settings at the moment
    }

    vTaskDelayUntil(&time_init, LCD_MAIN_RATE);
  }
}
