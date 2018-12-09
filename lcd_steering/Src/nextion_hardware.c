/*
 * nextion_hardware.c
 *
 *  Created on: Dec 4, 2018
 *      Author: Matt Flanagan
 */

#include "nextion_hardware.h"

//create the string that needs to be sent to the nextion display

//set_value is used for the number box on the display
//str -> str.val=XX0xff0xff0xff
void set_value(char* obj_name, uint16_t val)
{
	char* result = malloc(sizeof(*result) * (strlen(obj_name) + SET_VALUE_EXTRA + 1)); //.val=XXXXXFFF
	result[0] = '\0';
	char  str_buff[6] = {0,0,0,0,0, '\0'};
	uint16_t rem = 0;

	for (int count = 4; count >= 0; count--)
	{
		rem = val % 10;
		val = val / 10;
		str_buff[count] = rem + ASCII_OFFSET;
	}

	strcat(result, obj_name);
	strcat(result, ".val=");
	strcat(result, (char*) &str_buff);
	strcat(result, "\xFF\xFF\xFF");

	update_lcd((uint8_t*) result, strlen(obj_name) + SET_VALUE_EXTRA);
}

//set_text is used for the text box on the display
//str -> str.txt=XX0xff0xff0xff
void set_text(char* obj_name, char* value)
{
	char* result = malloc(sizeof(*result) * (strlen(obj_name) + strlen(value) + SET_TEXT_EXTRA + 1));
	result[0] = '\0';
	strcat(result, obj_name);
	strcat(result, ".txt=\"");
	strcat(result, value);
	strcat(result, "\"\xFF\xFF\xFF");

	update_lcd((uint8_t*) result, strlen(obj_name) + strlen(value) + SET_TEXT_EXTRA);
}

void set_bco(char* obj_name, uint16_t val)
{
	char* result = malloc(sizeof(*result) * (strlen(obj_name) + SET_BCO_EXTRA + 1)); //.val=XXXXXFFF
	result[0] = '\0';
	char  str_buff[6] = {0,0,0,0,0, '\0'};
	uint16_t rem = 0;

	for (int count = 4; count >= 0; count--)
	{
		rem = val % 10;
		val = val / 10;
		str_buff[count] = rem + ASCII_OFFSET;
	}

	strcat(result, obj_name);
	strcat(result, ".bco=");
	strcat(result, (char*) &str_buff);
	strcat(result, "\xFF\xFF\xFF");

	update_lcd((uint8_t*) result, strlen(obj_name) + SET_BCO_EXTRA);
}
