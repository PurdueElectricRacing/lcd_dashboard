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
#include "can_usr.h"
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
void btn_handler(uint8_t btn)
{
	CanTxMsgTypeDef tx;
	tx.IDE =  	CAN_ID_STD;                 //information relating to CAN frame
	tx.RTR =	CAN_RTR_DATA;               //information relating to CAN frame
	tx.DLC =  	1;                          //Number of bits the buttons need
	tx.Data[0] = 0x01;	                    //information relating to CAN frame
if (btn == START_MSG)					            //checks if button 1 is pressed
{
	tx.StdId = START_MSG_ID;	            //sends START_BUTTON can frame
}
else if (btn == TC_TV)                          //checks if button 2 is pressed if button 1 is not
{
	tx.StdId = TC_TV_ID;					//sends TC_TV can frame
}

xQueueSendToBack(lcd.q_tx_can, &tx, 100);	//Queue for can frames
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
  CanRxMsgTypeDef rx_can;
  float integral_gain = 0;
  float new_integral_gain;
  float proportional_gain = 0;
  float new_proportional_gain;
  float derivative_gain = 0;
  float new_derivative_gain;
  float signal_deadzone = 0;
  float new_signal_deadzone;
  uint8_t tctv_state;
  uint8_t lcd_dashboard;
  CanTxMsgTypeDef tx;
  lcd.can = &hcan1;
  lcd.uart = &huart2;
/*  lcd.data.speed = 100;
  lcd.data.pack_volt = 310;
  lcd.data.precharge = 1;
  lcd.data.imd = 0;
  lcd.data.bms = 0;
  char outBuffer[8];
  page_t page = RACE;
  status_t state = OFF;
  uint16_t counter_status = 0;
  uint8_t main_fault_code = 0;*/
  TickType_t time_init = 0;
 /* uart_rx_t rx_uart;
  uart_tx_t tx_uart;
  HAL_UART_Receive_IT(lcd.uart, myrx_data, RX_SIZE_UART); //start the receive*/

/*
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

*/

//    //receive can messages and update the lcd screen as necessary
//    //Live SOC/Voltage/Temperature
    if (xQueuePeek(lcd.q_rx_can, &rx_can, TIMEOUT) == pdTRUE)
     {
    	  xQueueReceive (lcd.q_rx_can, &rx_can, TIMEOUT);
    	  switch (rx_can.StdId)
			  {
			  case AVG_CELL_VOLTAGE_ID:
			  {
				lcd.data.avg_volts_bms =  rx_can.Data[0] | (rx_can.Data[1] << 8); //avg volts
				break;
			  }
			  case MAX_CELL_VOLTAGE_ID:
			  {
				lcd.data.max_volts_bms = rx_can.Data[0] | (rx_can.Data[1] << 8); //max volts
				break;
			  }
			  case LOW_CELL_VOLTAGE_ID:
			  {
				 lcd.data.low_volts_bms = rx_can.Data[0] | (rx_can.Data[1] << 8); //low volts
				 break;
			  }
			  case AVG_TEMP_ID:
			  {
				lcd.data.avg_temp_bms = rx_can.Data[0] | (rx_can.Data[1] << 8);  //avg temp
				break;
			  }
			  case HIGH_TEMP_ID:
			  {
				lcd.data.high_temp_bms = rx_can.Data[0] | (rx_can.Data[1] << 8); //high temp
				break;
			  }
			  case LOW_TEMP_ID:
			  {
				lcd.data.low_temp_bms = rx_can.Data[0] | (rx_can.Data[1] << 8); //low temp
				break;
			  }
			  case PACK_CURRENT_ID:
			  {
				lcd.data.pack_current_bms = rx_can.Data[0] | (rx_can.Data[1] << 8); //pack current
				break;
			  }
			  case INSTANT_PACK_VOLT_ID:
			  {
				lcd.data.instant_pack_volt_bms = rx_can.Data[0] | (rx_can.Data[1] << 8); //instant pack volt
				break;
			  }
			  case PACK_CURRENT_LOW_ID:
			  {
				lcd.data.pack_current_low_bms = rx_can.Data[0] | (rx_can.Data[1] << 8); //pack current low
				break;
			  }
			  case PACK_DISCH_LIM_ID:
			  {
				lcd.data.pack_disch_lim_bms = rx_can.Data[0] | (rx_can.Data[1] << 8); //pack disch lim
				break;
			  }
			  case SOC_CELL_LIMS_ID:
			  {
				lcd.data.soc_cell_lim_bms = rx_can.Data[0] | (rx_can.Data[1] << 8); //soc cell lim
				break;
			  }
			  case MAIN_HEARTBEAT:        //main module
			  {
				lcd.data.car_state = (rx_can.Data[0]);      //car state
				lcd.data.PC_status = ((rx_can.Data[2]));        //PC status
				lcd.data.tctv_state = ((rx_can.Data[3]));       // tctv state
				break;
			  }
			  case WHEEL_SPEED_FRONT:
			  {
				lcd.data.wheel_speed_front_l = rx_can.Data[3] | (rx_can.Data[2] <<8) | (rx_can.Data[1] <<16) | (rx_can.Data[0] <<24);
				lcd.data.wheel_speed_front_r = rx_can.Data[7] | (rx_can.Data[6] <<8) | (rx_can.Data[5] <<16) | (rx_can.Data[4] <<24);
				break;
			  }
			  case WHEEL_SPEED_REAR:
			  {
				  lcd.data.wheel_speed_rear_l =  rx_can.Data[3] | (rx_can.Data[2] <<8) | (rx_can.Data[1] <<16) | (rx_can.Data[0] <<24);
				  lcd.data.wheel_speed_rear_r =  rx_can.Data[7] | (rx_can.Data[6] <<8) | (rx_can.Data[5] <<16) | (rx_can.Data[4] <<24);
				  break;
			  }

			  if (new_integral_gain != integral_gain || new_proportional_gain != proportional_gain
			    || new_derivative_gain != derivative_gain || new_signal_deadzone != signal_deadzone)   //Checks if new values match the old values. If not, then it updates the values.
			  {
			  integral_gain = new_integral_gain;
			  proportional_gain = new_proportional_gain;
			  derivative_gain = new_derivative_gain;
			  signal_deadzone = new_signal_deadzone;
			  tx.StdId = ID_UPDATE_PARAM_1;
			  tx.IDE = CAN_ID_STD;
			  tx.RTR = CAN_RTR_DATA;
			  tx.DLC = 4;
			  tx.Data[0] = (uint16_t) (lcd.data.proportional_gain * 100);
			  tx.Data[1] = (uint16_t) (lcd.data.integral_gain * 100);
			  tx.StdId = ID_UPDATE_PARAM_2;
			  tx.Data[2] = (uint16_t) (lcd.data.derivative_gain * 100);
			  tx.Data[3] = (uint16_t) (lcd.data.signal_deadzone * 100);
			  xQueueSendToBack(lcd.q_tx_can, &tx, 100);
			  }
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

/*
    Update the values on the LCD
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
*/


	if (tctv_state != LCD_dashboard)
	{
	  tctv_state = !tctv_state;
	}

    vTaskDelayUntil(&time_init, LCD_MAIN_RATE);
  }
}


}
