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
*       1.
*
*     File Description: Main constants for Can messages, button identifiers
*     and BMS constants.
*
***************************************************************************/

#ifndef LCD_H_
#define LCD_H_

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_can.h"
#include "stm32l4xx_hal_uart.h"
#include "stm32l4xx_hal_adc.h"
//#include "cmsis_os.h"
#include "main.h"
#include "uart.h"
#include "steer.h"
#include "nextion_hardware.h"
//#include "task.h"
#include "can_usr.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

#define PER 1
#define GREAT PER

//LCD Constants What the LCD SCREEN Sends
#define RACE_START          0
#define LCD_UPDATE_RATE		1	 //pick a rate that makes 6b1 ~1hz used currently message is sent at 100hz
//Can Message Constants
#define START_MSG_ID		0x350
#define BMS_MSG_ID			0x6B1
#define MAIN_ACK_ID			0x360
#define STEER_SENSE_ID		0x7C0
//#define MC_FAULT_ID
#define MAIN_FAULT_ID		0x200

//Queue Constants
// OLD: #define RX_CAN_QUEUE_SIZE 	32
// OLD: #define TX_CAN_QUEUE_SIZE	  8
// OLD: #define RX_UART_QUEUE_SIZE 	5
// OLD: #define TX_UART_QUEUE_SIZE 	10

//Stack Constants
// OLD: #define LCD_MAIN_STACK_SIZE 512
// OLD: #define TX_CAN_STACK_SIZE	  256
// OLD: #define TX_UART_STACK_SIZE	256
// OLD: #define STEER_STACK_SIZE		256

//Priority Constatns
#define LCD_MAIN_PRIORTIY 	1
#define TX_CAN_PRIORITY		  1
#define TX_UART_PRIORITY  	1
#define STEER_PRIORITY			1

//Rates
#define LCD_MAIN_RATE 		  10 // portTICK_RATE_MS
#define TX_CAN_RATE			    5 // portTICK_RATE_MS
#define TX_UART_RATE		    10 // portTICK_RATE_MS
#define STEER_RATE					50 // portTICK_RATE_MS

//Delays
#define DELAY_STARTUP				500 // portTICK_RATE_MS

//Timeouts
#define TIMEOUT				      5 // portTICK_RATE_MS
#define WAIT_QUEUE_FULL		  30 //OLD: portTICK_RATE_MS

//LCD Bounds Constants
#define BMS_OVER_TEMP_RED	  60
#define BMS_OVER_TEMP_YEL	  55
#define BMS_UNDER_VOLT_RED	355
#define BMS_SOC_RED			    10
#define	BMS_SOC_YEL			    50
#define BMS_SOC_GREEN		    50

// CAN IDs /* TODO: Move these to CAN header file */
#define ID_START                0x102
#define ID_SDO                  0x581
#define ID_EMDRIVE_SLAVE_PDO_1  0x180
#define ID_EMDRIVE_SLAVE_PDO_2  0x280
#define ID_EMDRIVE_SLAVE_PDO_3  0x380
#define ID_HEARTBEAT            0x420

// Generic defines
#define NODE_ID                 1                   // Node ID of EMDrive
#define VOLTS_IMPLAUS           0                   // Implausible voltage
#define PI                      3.14159f            // Not importing math.h for freaking pi oml
#define WHEEL_DIAM              18                  // Diameter of the wheels in inches
#define IPM_CONV                1056                // Magic number for converting inches per minute to rpm
#define CURRENT_MAX             13000               // Max MC current
#define VOLTS_MIN               30000               // Min voltage
#define VELOCITY_MAX            1300                // Max RPM (~70 MPH)

#define BEGIN_DATA_BYTE(x) (x * sizeof(uint8_t *)) // macro for returning the offset of a can data array

typedef enum {
	SPLASH = 0,
	ERR,
	RACE
} page_t;

enum {
    DRIVE_INACTIVE,
    DRIVE_ACTIVE
} drive_stat_t;

enum {
    MC_ERROR,
    UVVOLT,
    OVCURR,
    OVDMDCURR,
    VEH_ERROR,
    OVSPEED,
    PED_ERROR
} fault_bits_t;

typedef enum {
    CAR_STATE_INIT           = 0,
    CAR_STATE_PREREADY2DRIVE = 1,
    CAR_STATE_READY2DRIVE    = 2,
    CAR_STATE_ERROR          = 3,
    CAR_STATE_RESET          = 4,
    CAR_STATE_RECOVER        = 5
} car_state_t;

typedef enum {
    PEDALBOX_STATUS_NO_ERROR           = 0,
    PEDALBOX_STATUS_ERROR              = 0b00000001,    // Generic error
    PEDALBOX_STATUS_ERROR_EOR          = 0b00000010,    // Encoder out of range
    PEDALBOX_STATUS_ERROR_APPSIMP      = 0b00000100,    // APPS Implausibility error, EV 2.3.5,
    PEDALBOX_STATUS_ERROR_APPSIMP_PREV = 0b00001000,    // APPS Implausibility error, provisional (before it has lasted .1 second)
    PEDALBOX_STATUS_ERROR_BPIMP        = 0b00010000,    // Brake pedal implaus //EV 2.5.1,
} pedalbox_status_t;

//Main LCD structure that holds can handles and all of the queues
typedef struct
{
    CAN_HandleTypeDef* 	can;
    UART_HandleTypeDef* uart;

    q_handle_t q_rx_can;
    q_handle_t q_tx_can;
    q_handle_t q_rx_uart;
    q_handle_t q_tx_uart;

    uint8_t     drive_stat;                 // Status of EMDrive (on/off)
    uint16_t    voltage;                    // DC link voltage (not pack voltage)
    uint16_t    torque_actual;              // Torque out of the motor
    uint32_t    position_actual;            // Position of the motor
    uint16_t    status_word;                // Current EMDrive status
    uint16_t    electrical_power;           // Electrical power output
    uint32_t    error_codes;                // Motor controller specific error codes (not the same as error_stat)
    uint8_t     motor_temp;                 // Current temperature of the motor
    uint8_t     emdrive_temp;               // Current temperature of the motor controller
    uint16_t    phase_b_current;            // Phase B output current
    uint32_t    velocity;                   // Velocity of the vehicle (in RPM)
    uint16_t    actual_current;             // Actual current output (not demanded)
    uint16_t    current_demand;             // Current demand
    car_state_t vehicle_stat;               // Overall vehicle status (mirrored from main)
    uint16_t    error_stat;                 // Bit flags for specific vehicle wide errors
    float       vehicle_speed;              // "Speed" of the vehicle (does not account for slip)
    uint16_t    throttle;                   // Current throttle value
    uint16_t    brake;                      // Current brake pressure
    uint8_t     pc_stat;                    // Precharge status
    pedalbox_status_t pedalbox_stat;        // Pedalbox status
    page_t      page;                       // Current LCD page

    /*
     * error_stat flags:
     * [0] - MC error
     * [1] - DC link undervoltage
     * [2] - MC overcurrent
     * [3] - MC overdemand current
     * [4] - Vehicle status error
     * [5] - Powertrain overspeed
     * [6] - Pedalbox error
     */
}lcd_t;

//Used to update the LCD screen
typedef struct
{
    uint16_t 	pack_volt;				//Most recent pack voltage
    uint8_t 	pack_soc;				//pack SOC
    uint8_t  	high_temp;				//the current highest temperature of a cell
}bms_data_t;

void task_lcd_main();
void error_blink();
void task_lcd_help();
// OLD: void initRTOSObjects(void);
void initLcd();

lcd_t lcd;

extern CAN_HandleTypeDef 	hcan1;
extern UART_HandleTypeDef 	huart2;
extern ADC_HandleTypeDef hadc1;

#endif /* LCD_H_ */
