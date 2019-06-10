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
	int16_t steer_angle = 0;
	uint16_t raw_angle = 0;
	float per_angle = 0;
	TickType_t time_init = 0;
	while (1)
	{
		time_init = xTaskGetTickCount();
		HAL_ADC_Start(&hadc1); //Start the ADC
		HAL_ADC_PollForConversion(&hadc1, 10); //read channel 12
		raw_angle = HAL_ADC_GetValue(&hadc1);
//		HAL_ADC_PollForConversion(&hadc1, 10);
//		HAL_ADC_GetValue(&hadc1);	//read channel 16
		HAL_ADC_Stop(&hadc1);

//		per_angle = (float) (raw_angle - FULL_LEFT) / (FULL_RIGHT - FULL_LEFT); //get raw percentage
//		per_angle = (per_angle - .5) * 100;
//		steer_angle = (int16_t) per_angle;
		CanTxMsgTypeDef tx;
		tx.IDE = CAN_ID_STD;
		tx.RTR = CAN_RTR_DATA;
		tx.StdId = STEER_SENSE_ID;
		tx.DLC = 2;
		tx.Data[0] =	(uint8_t) raw_angle;	//bits 7-0
		tx.Data[1] =	(uint8_t) (raw_angle >> 8);		//bits 11-8
//		tx.Data[2] =	(uint8_t) steer_strain;	//bits 7-0
//		tx.Data[3] =	(uint8_t) (steer_strain >> 8);		//bits 11-8

		xQueueSendToBack(lcd.q_tx_can, &tx, 100);

		vTaskDelayUntil(&time_init, STEER_RATE / portTICK_RATE_MS);
	}
}
