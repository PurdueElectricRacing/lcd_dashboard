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

uint8_t loop, run;

// @brief: Parsing function so I don't have to type the data array out each time.
// @param: uint8_t * data: address of the first byte of the data array
//         i.e. if first byte is data[3], pass (&data) + 3 * sizeof(uint8_t *)
static uint32_t parse_from_lil_32(uint8_t * data)
{
    return ((uint32_t) *(data + (3 * sizeof(uint8_t *)))) << 24
            | ((uint32_t) *(data + (2 * sizeof(uint8_t *)))) << 16
            | ((uint16_t) *(data + (sizeof(uint8_t *)))) << 8
            | *(data);
}

// @brief: Parsing function so I don't have to type the data array out each time.
// @param: uint8_t * data: address of the first byte of the data array
//         i.e. if first byte is data[3], pass (&data) + 3 * sizeof(uint8_t *)
static uint16_t parse_from_lil_16(uint8_t * data)
{
    return ((uint16_t) *(data + sizeof(uint8_t *)) << 8) | *data;
}

// @brief: Function to translate output (wheel, in our case) RPM to good 'ol
//         American speed units
static void calcSpeed()
{
    lcd.vehicle_speed = ((float) lcd.velocity) * PI * WHEEL_DIAM;               // Revolutions / minute * circumference = inches / minute
    lcd.vehicle_speed /= IPM_CONV;                                              // Convert imp to mph
}

// @brief: function for parsing the data returned from the emdrive
//         TPDO1 - TPDO3 values. These are sent after the sync function
//         initiates the transaction.
static void emdrive_parse_pdo(uint16_t id, uint8_t* data)
{
    if (id == (ID_EMDRIVE_SLAVE_PDO_1 | NODE_ID))
    {
        lcd.torque_actual = parse_from_lil_16(data + BEGIN_DATA_BYTE(6));
        lcd.position_actual = parse_from_lil_32(data + BEGIN_DATA_BYTE(2));
        lcd.status_word = parse_from_lil_16(data);
    }
    else if (id == (ID_EMDRIVE_SLAVE_PDO_2 | NODE_ID))
    {
        lcd.current_demand = parse_from_lil_16(data + BEGIN_DATA_BYTE(6));
        lcd.voltage = parse_from_lil_16(data + BEGIN_DATA_BYTE(2));
        lcd.motor_temp = data[1];
        lcd.emdrive_temp = data[0];

        // TODO: Go branchless!
        if (lcd.voltage < VOLTS_MIN)
        {
            lcd.error_stat |= 1 << UVVOLT;
        }
        else
        {
            lcd.error_stat |= ~(1 << UVVOLT);
        }

        if (lcd.current_demand > CURRENT_MAX)
        {
            lcd.error_stat |= 1 << OVDMDCURR;
        }
        else
        {
            lcd.error_stat |= ~(1 << OVDMDCURR);
        }
    }
    else if (id == (ID_EMDRIVE_SLAVE_PDO_3 | NODE_ID))
    {
        lcd.phase_b_current = parse_from_lil_16(data + BEGIN_DATA_BYTE(6));
        lcd.velocity = parse_from_lil_32((data + BEGIN_DATA_BYTE(2)));
        lcd.actual_current = parse_from_lil_16(data);

        if (lcd.actual_current > CURRENT_MAX)
        {
            lcd.error_stat |= 1 << OVCURR;
        }
        else
        {
            lcd.error_stat |= ~(1 << OVCURR);
        }

        if (lcd.velocity > VELOCITY_MAX)
        {
            lcd.error_stat |= 1 << OVSPEED;
        }
        else
        {
            lcd.error_stat &= ~(1 << OVSPEED);
        }

        calcSpeed();
    }
}

// @brief: Checks all values RX'd via CAN to see if we have any errors in
//         in the system so we can let the driver know what's going on
static void errorCheck()
{
    // Locals
    uint8_t   i;                                    // Loop counter
    uart_tx_t tx_uart;                              // Buffer for TX transmission
    uart_tx_t tx_uart2;                             // Buffer for TX transmission

    if (lcd.error_stat != 0)                        // Check if we have an error
    {
        for (i = 0; i < 16; i++)                    // Loop through each
        {
            if (lcd.error_stat & (1 << i))          // Check if we've found the error (highest priority only)
            {
                switch(i)                           // Check how we need to handle the error
                {
                    case MC_ERROR:                  // Motor controller error. This is a special one
                    {
                        sprintf((char*) &tx_uart.tx_buffer[0], "MC ERROR");
                        sprintf((char*) &tx_uart2.tx_buffer[0], "%ld", lcd.error_codes);
                        return;
                    }

                    case UVVOLT:
                    {
                        sprintf((char*) &tx_uart.tx_buffer[0], "UNDERVOLT");
                        sprintf((char*) &tx_uart2.tx_buffer[0], "%dV", lcd.voltage / 100);
                        break;
                    }


                    case OVCURR:
                    {
                        sprintf((char*) &tx_uart.tx_buffer[0], "OVERCURRENT");
                        sprintf((char*) &tx_uart2.tx_buffer[0], "%dA", lcd.actual_current / 100);
                        break;
                    }


                    case OVDMDCURR:
                    {
                        sprintf((char*) &tx_uart.tx_buffer[0], "OVERCURRDEM");
                        sprintf((char*) &tx_uart2.tx_buffer[0], "%dA", lcd.current_demand / 100);
                        break;
                    }


                    case VEH_ERROR:
                    {
                        sprintf((char*) &tx_uart.tx_buffer[0], "MAIN ERR");
                        sprintf((char*) &tx_uart2.tx_buffer[0], "%d", lcd.vehicle_stat);
                        break;
                    }


                    case OVSPEED:
                    {
                        sprintf((char*) &tx_uart.tx_buffer[0], "OVERSPEED");
                        sprintf((char*) &tx_uart2.tx_buffer[0], "%ld RPM", lcd.velocity);
                        break;
                    }


                    case PED_ERROR:
                    {
                        sprintf((char*) &tx_uart.tx_buffer[0], "PEDAL ERROR");
                        sprintf((char*) &tx_uart2.tx_buffer[0], "%d", lcd.pedalbox_stat);
                        break;
                    }
                }

                set_text("car_stat", (char*) "!ERROR!");
                set_text("car_err", (char*) &tx_uart.tx_buffer[0]);
                set_text("t0", (char*) &tx_uart2.tx_buffer[0]);
            }
        }
    }
    else
    {
        lcd.page = RACE;
    }
}

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
static void btn_handler(uint8_t btn)
{
  CanTxMsgTypeDef msg;
  msg.IDE = CAN_ID_STD;
  msg.RTR = CAN_RTR_DATA;

  switch (btn)
  {
    case 0:
    {
        msg.DLC = 1;
        msg.StdId = START_MSG_ID;
        msg.Data[0] = 0x1;

//        qSendToBack(&lcd.q_tx_can, &msg);
//        qSendToBack(&lcd.q_tx_can, &msg);
        break;
    }

    case 1:
    {
        msg.DLC = 1;
        msg.StdId = START_MSG_ID;
        msg.Data[0] = 0x1;

        lcd.drive_stat = !lcd.drive_stat;

//        qSendToBack(&lcd.q_tx_can, &msg);
        break;
    }
  }
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

// Sets up queues
// Sets up the timer to increment os_ticks every 1ms
void initLcd()
{
  qConstruct(&lcd.q_rx_can, sizeof(CanRxMsgTypeDef));
  qConstruct(&lcd.q_tx_can, sizeof(CanTxMsgTypeDef));
  qConstruct(&lcd.q_rx_uart, sizeof(uart_rx_t));
  qConstruct(&lcd.q_tx_uart, sizeof(uart_tx_t));

  lcd.can = &hcan1;
  lcd.uart = &huart2;

  /* Yes, I know this code is inefficient. Yes, I know I could have used memset().
   * I've done it this way so we can set specific starting values to avoid errors
   * if (and when) the complexity of this system grows. We might not want them all
   * to start at 0, but for now, that's what we'll go with.
   */
  lcd.drive_stat = DRIVE_INACTIVE;
  lcd.voltage = VOLTS_IMPLAUS;
  lcd.vehicle_stat = CAR_STATE_INIT;
  lcd.torque_actual = 0;
  lcd.position_actual = 0;
  lcd.status_word = 0;
  lcd.electrical_power = 0;
  lcd.error_codes = 0;
  lcd.motor_temp = 0;
  lcd.emdrive_temp = 0;
  lcd.phase_b_current = 0;
  lcd.velocity = 0;
  lcd.actual_current = 0;
  lcd.current_demand = 0;
  lcd.error_stat = 0;
  lcd.vehicle_speed = 0;
  lcd.throttle = 0;
  lcd.brake = 0;
  lcd.pc_stat = 0;
  lcd.pedalbox_stat = PEDALBOX_STATUS_NO_ERROR;

  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;   // Enable timer clock in RCC
  TIM2->PSC = 3200 - 1; // AHB2 is configured at 32Mhz
  TIM2->ARR = 10;
  TIM2->CR1 &= ~(TIM_CR1_DIR);
  TIM2->DIER |= TIM_DIER_UIE;
  NVIC->ISER[0] |= 1 << TIM2_IRQn;
  TIM2->CR1 |= TIM_CR1_CEN;
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
    // Locals
    CanRxMsgTypeDef rx;
    //uint16_t        byte_comb;
    //uart_rx_t       rx_uart;

    page_t          page_next;
    static uint8_t  init;

    errorCheck();                                                                       // Check errors first and interject with new action

    page_next = lcd.page;                                                               // Default to where we were

    if (!init)                                                                          // Wait for bootup sequence
    {
        init = 1;                                                                       // Don't hang
        set_page("race");                                                               // Move to race page

        page_next = RACE;                                                               // Update "FSM"
    }

// Due to instability with button handling, no user requests for now :/
//    if (qReceive(&lcd.q_rx_uart, &rx_uart) == QUEUE_SUCCESS)                            // Check if we can pull an item from the queue
//    {
//        //byte_comb = (uint16_t) rx_uart.rx_buffer[2] | (rx_uart.rx_buffer[1] << 8);      // Combine the data bytes
//
//        switch (lcd.page)                                                               // Check which page we're on
//        {
//            case SPLASH:
//            {
//                set_page("race");                                                       // Move to race page
//                page_next = RACE;                                                       // Update page
//
//                break;
//            }
//            case ERR:
//            {
////                btn_handler(1);                                                         // Restart MC
//                set_page("race");                                                       // Move to race page
//                page_next = RACE;                                                       // Update page
//
//                break;
//            }
//            case RACE:
//            {
////                btn_handler(0);                                                         // Send start button
//
//                break;
//            }
//        }
//    }

    if (qReceive(&lcd.q_rx_can, &rx) == QUEUE_SUCCESS)
    {
        switch (rx.StdId)
        {
            case ID_START:
            {
                lcd.drive_stat = !lcd.drive_stat;
                break;
            }

            case ID_SDO:
            {
                // Unused. Can catch individual SDO reads

                break;
            }

            case ID_EMDRIVE_SLAVE_PDO_1 | NODE_ID:

            case ID_EMDRIVE_SLAVE_PDO_2 | NODE_ID:

            case ID_EMDRIVE_SLAVE_PDO_3 | NODE_ID:
            {
                emdrive_parse_pdo(rx.StdId, rx.Data);

                break;
            }

            case ID_HEARTBEAT:
            {
                lcd.vehicle_stat = rx.Data[0];
                lcd.pedalbox_stat = rx.Data[1];
                lcd.pc_stat = rx.Data[2];

                if (lcd.vehicle_stat == CAR_STATE_ERROR || lcd.vehicle_stat == CAR_STATE_RESET)
                {
                    lcd.error_stat |= 1 << VEH_ERROR;
                }
                else
                {
                    lcd.error_stat |= ~(1 << VEH_ERROR);
                }

                if (lcd.pedalbox_stat != PEDALBOX_STATUS_NO_ERROR)
                {
                    lcd.error_stat |= 1 << PED_ERROR;
                }
                else
                {
                    lcd.error_stat |= ~(1 << PED_ERROR);
                }

                break;
            }
        }
    }

    lcd.page = page_next;
}

void task_lcd_help()
{
    uart_tx_t   tx_uart;
    switch (lcd.page)
    {
        case RACE:
        {
            // TODO: Add data timeout
            if (lcd.voltage > 0)
            {
                sprintf((char*) &tx_uart.tx_buffer[0], "%dV", lcd.voltage / 100);
                set_text("volts", (char *) &tx_uart.tx_buffer[0]);
            }
            else
            {
                sprintf((char*) &tx_uart.tx_buffer[0], "No Data");
                set_text("volts", (char *) &tx_uart.tx_buffer[0]);
            }

            sprintf((char*) &tx_uart.tx_buffer[0], "%d mph", (int) lcd.vehicle_speed);
            set_text("speed", (char *) &tx_uart.tx_buffer[0]);

            sprintf((char*) &tx_uart.tx_buffer[0], "Drive %s", lcd.vehicle_stat == CAR_STATE_READY2DRIVE ? "On" : "Off");
            set_text("drive_stat", (char *) &tx_uart.tx_buffer[0]);
        }
    }
}
