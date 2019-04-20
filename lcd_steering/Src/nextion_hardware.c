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
*       1. nextion_hardware.h
*
*     File Description: Used to obfuscate the nextion display to be easier to 
*     implement main logic with the nextion display.
*
***************************************************************************/

#include "nextion_hardware.h"

//create the string that needs to be sent to the nextion display

/***************************************************************************
*
*     Function Information
*
*     Name of Function: set_value
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type: None
*
*     Parameters (list data type, name, and comment one per line):
*       1.char*   obj_name    the name of the object on the actual .hmi file
*       2.uint16_t val        the value to set the object to
*       
*      Global Dependents:
*       1. None
*
*     Function Description: set_value is used for the number box on the display
*     str -> str.val=XX0xff0xff0xff
*
***************************************************************************/
void set_value(char* obj_name, uint16_t val)
{
  char result[TX_MAX_LEN]; //.val=XXXXXFFF
  char* ptr = &result[0];
  result[0] = '\0';
  char  str_buff[6] = {0,0,0,0,0, '\0'};
  uint16_t rem = 0;

  for (int count = 4; count >= 0; count--)
  {
    rem = val % 10;
    val = val / 10;
    str_buff[count] = rem + ASCII_OFFSET;
  }

  strcat(ptr, obj_name);
  strcat(ptr, ".val=");
  strcat(ptr, (char*) &str_buff);
  strcat(ptr, "\xFF\xFF\xFF");

  update_lcd((uint8_t*) ptr, strlen(obj_name) + SET_VALUE_EXTRA);
}

/***************************************************************************
*
*     Function Information
*
*     Name of Function: set_text
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type: None
*
*     Parameters (list data type, name, and comment one per line):
*       1.char*   obj_name    the name of the object on the actual .hmi file
*       2.char*   value       the value to set the object to
*       
*      Global Dependents:
*       1. None
*
*     Function Description: set_text is used for the text box on the display
*     str -> str.txt=XX0xff0xff0xff
*
***************************************************************************/
void set_text(char* obj_name, char* value)
{
	char result[TX_MAX_LEN]; //.val=XXXXXFFF
	char* ptr = &result[0];
  result[0] = '\0';
  strcat(ptr, obj_name);
  strcat(ptr, ".txt=\"");
  strcat(ptr, value);
  strcat(ptr, "\"\xFF\xFF\xFF");

  update_lcd((uint8_t*) ptr, strlen(obj_name) + strlen(value) + SET_TEXT_EXTRA);
}

/***************************************************************************
*
*     Function Information
*
*     Name of Function: set_bco
*
*     Programmer's Name: Matt Flanagan
*
*     Function Return Type: 
*
*     Parameters (list data type, name, and comment one per line):
*       1.char*   obj_name    the name of the object on the actual .hmi file
*       2.uint16_t val        the value to set the object to
*
*      Global Dependents:
*       1. None
*
*     Function Description: Used to set the background color of the object for 
*     the lcd screen.
*
***************************************************************************/
void set_bco(char* obj_name, uint16_t val)
{
	char result[TX_MAX_LEN]; //.val=XXXXXFFF
	char* ptr = &result[0];
	result[0] = '\0';
  char  str_buff[6] = {0,0,0,0,0, '\0'};
  uint16_t rem = 0;

  for (int count = 4; count >= 0; count--)
  {
    rem = val % 10;
    val = val / 10;
    str_buff[count] = rem + ASCII_OFFSET;
  }

  strcat(ptr, obj_name);
  strcat(ptr, ".bco=");
  strcat(ptr, (char*) &str_buff);
  strcat(ptr, "\xFF\xFF\xFF");

  update_lcd((uint8_t*) ptr, strlen(obj_name) + SET_BCO_EXTRA);
}

void set_page(char* page_name) {
	char result[TX_MAX_LEN]; //.val=XXXXXFFF
	char* ptr = &result[0];
	result[0] = '\0';
  
  strcat(ptr, "page ");
  strcat(ptr, page_name);
  strcat(ptr, "\xFF\xFF\xFF");

  update_lcd((uint8_t*) ptr, strlen(page_name) + SET_PAGE_EXTRA);
}
