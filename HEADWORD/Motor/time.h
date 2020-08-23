#ifndef _TIME_H_
#define _TIME_H_
#include "sys.h"

void Time1_Pwm_Init(u16 arr,u16 psc);
void Time2_Init(u16 arr,u16 psc);
void Time4_Init(u16 arr,u16 psc);
void Time3_Pwm_Init(u16 arr,u16 psc);
void Cursor_On(int dat);
void Cursor_Down(int dat);

#endif
