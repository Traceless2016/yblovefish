#ifndef _BUTTON_H_
#define _BUTTON_H_
#include <OneButton.h>
#include "./c_types.h"


extern OneButton btn;
void singleClick();
void doubleClick() ;
void longClickStart();
void longClick();
void button_setup();

#endif