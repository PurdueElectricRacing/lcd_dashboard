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
    }
    else if (id == (ID_EMDRIVE_SLAVE_PDO_3 | NODE_ID))
    {
        lcd.phase_b_current = parse_from_lil_16(data + BEGIN_DATA_BYTE(6));
        lcd.velocity = parse_from_lil_32((data + BEGIN_DATA_BYTE(2)));
        lcd.actual_current = parse_from_lil_16(data);
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
  lcd.drive_stat = 1;
  lcd.voltage = 44100;

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
    uart_rx_t       rx_uart;
    uart_tx_t       tx_uart;

    page_t          page_next;
    static uint8_t  init;
    static uint8_t  counts;

    page_next = lcd.page;

    if (!init)
    {
        init = 1;
        set_page("race");

        page_next = RACE;
    }

    if (qReceive(&lcd.q_rx_uart, &rx_uart) == QUEUE_SUCCESS)                            // Check if we can pull an item from the queue
    {
        //byte_comb = (uint16_t) rx_uart.rx_buffer[2] | (rx_uart.rx_buffer[1] << 8);      // Combine the data bytes

        switch (lcd.page)                                                               // Check which page we're on
        {
            case SPLASH:
            {
                set_page("race");                                                       // Move to race page
                page_next = RACE;                                                       // Update page

                break;
            }
            case ERR:
            {
//                btn_handler(1);                                                         // Restart MC
                set_page("race");                                                       // Move to race page
                page_next = RACE;                                                       // Update page

                break;
            }
            case RACE:
            {
//                btn_handler(0);                                                         // Send start button

                break;
            }
        }
    }

    if (lcd.page == ERR)
    {
        lcd.drive_stat = 0;
        if (counts++ == 500)
        {
            sprintf((char*) &tx_uart.tx_buffer[0], "YEET");
            set_text("car_stat", (char *) &tx_uart.tx_buffer[0]);
        }
    }

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
                break;
            }

            case ID_EMDRIVE_SLAVE_PDO_1 | NODE_ID:

            case ID_EMDRIVE_SLAVE_PDO_2 | NODE_ID:

            case ID_EMDRIVE_SLAVE_PDO_3 | NODE_ID:
            {
                emdrive_parse_pdo(rx.StdId, rx.Data);

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
        case ERROR:
        {
            set_text("car_stat", (char*) "skrrt");
            break;
        }

        case RACE:
        {
            // TODO: Add data timeout
            if (lcd.voltage > 0)
            {
                sprintf((char*) &tx_uart.tx_buffer[0], "%dV", lcd.voltage / 100);
                set_text("volts", (char *) &tx_uart.tx_buffer[0]);
            }

            sprintf((char*) &tx_uart.tx_buffer[0], "Drive %s", lcd.drive_stat == 1 ? "On" : "Off");
            set_text("drive_stat", (char *) &tx_uart.tx_buffer[0]);
        }
    }
}
