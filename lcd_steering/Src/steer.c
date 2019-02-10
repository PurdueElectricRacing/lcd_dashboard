/*
 * steer.c
 *
 *  Created on: Jan 30, 2019
 *      Author: Matt Flanagan
 */
#include "steer.h"


/***************************************************************************
*
*     Function Information
*
*     Name of Function: Steering Position and Strain Sensor
*
*     Programmer's Name: Chuyen Le, Dawson Moore
*
*     Function Return Type: void
*
*     Parameters (list data type, name, and comment one per line):
*       1. None
*
*
*     Global Dependents:
*       1. None
*
*     Function Description: Task that runs and reads adc's from the steering
*     angle sensor and the steering strain sensor at a predescribed rate
*
***************************************************************************/
void taskPollSteer()
{
	int steer_angle = 0;
	int steer_strain = 0;
	TickType_t time_init = 0;
	while (1)
	{
		time_init = xTaskGetTickCount();
		HAL_ADC_Start(&hadc1); //Start the ADC
		HAL_ADC_PollForConversion(&hadc1, 100); //read channel 12
		steer_angle = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 100);
		steer_strain = HAL_ADC_GetValue(&hadc1);	//read channel 16
		HAL_ADC_Stop(&hadc1);

		CanTxMsgTypeDef tx;
		tx.IDE = CAN_ID_STD;
		tx.RTR = CAN_RTR_DATA;
		tx.StdId = STEER_SENSE_ID;
		tx.DLC = 4;
		tx.Data[0] =	(uint8_t) steer_angle;	//bits 7-0
		tx.Data[1] =	(uint8_t) (steer_angle >> 8);		//bits 11-8
		tx.Data[2] =	(uint8_t) steer_strain;	//bits 7-0
		tx.Data[3] =	(uint8_t) (steer_strain >> 8);		//bits 11-8

		xQueueSendToBack(lcd.q_tx_can, &tx, 100);

		vTaskDelayUntil(&time_init, STEER_RATE / portTICK_RATE_MS);
	}
}
