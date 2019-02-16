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
*
*     File Description: This is the nextion abstraction file. That makes the 
*     logic for interacting with the Nextion display easy.
*
***************************************************************************/

#ifndef NEXTION_HARDWARE_H_
#define NEXTION_HARDWARE_H_

#include "lcd.h"

#define SET_VALUE_EXTRA 	13
#define SET_BCO_EXTRA 		13
#define SET_TEXT_EXTRA 		10
#define SET_PAGE_EXTRA      8
#define ASCII_OFFSET 		48

#define RED					63488
#define YELLOW				65504
#define GREEN				4065

void set_value(char* str, uint16_t val);
void set_text(char* obj_name, char* value);
void set_bco(char* obj_name, uint16_t val);
void set_page(char* page_name);

#endif /* NEXTION_HARDWARE_H_ */
