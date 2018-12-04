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
char* set_value(char* str, uint16_t val) {
	char* result = malloc(sizeof(*result) * (strlen(str) + SET_VALUE_EXTRA)); //.val=XXXXXFFF
	char  str_buff[6] = {0,0,0,0,0, '\0'};
	uint16_t rem = 0;
	uint16_t count = 4;
	while (val != 0) {
		rem = val % 10;
		val = val / 10;
		str_buff[count--] = rem;
	}
	strcat(result, &str_buff);
	strcat(result, "\xFF\xFF\xFF");
	return result;
}

//set_text is used for the text box on the display
//str -> str.txt=XX0xff0xff0xff
char* set_text(char* obj_name, char* value) {
	char* result = malloc(sizeof(*result) * (strlen(obj_name) + strlen(value) + SET_TEXT_EXTRA));
	char temp[] = ".txt=";
	strcat(obj_name, temp);
	strcat(obj_name, value);
	strcat(obj_name, "\xFF\xFF\xFF");
	return result;
}
