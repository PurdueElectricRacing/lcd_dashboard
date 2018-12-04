/*
 * nextion_hardware.h
 *
 *  Created on: Dec 4, 2018
 *      Author: Matt Flanagan
 */

#ifndef NEXTION_HARDWARE_H_
#define NEXTION_HARDWARE_H_

#include "lcd.h"

#define SET_VALUE_EXTRA 13
#define SET_TEXT_EXTRA  13
#define ASCII_OFFSET 	48

void set_value(char* str, uint16_t val);
void set_text(char* obj_name, char* value);

#endif /* NEXTION_HARDWARE_H_ */
