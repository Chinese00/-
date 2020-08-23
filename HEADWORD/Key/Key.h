#ifndef _KEY_H_
#define _KEY_H_
#include "sys.h"

//#define KEY0   PEin(4)
//#define KEY1   PEin(3)
//#define KEY2   PEin(2)
//#define WK_UP  PAin(0)


#define KEY0  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)    //������ȡ�궨��
#define KEY1  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)
#define KEY2  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)
#define WK_UP GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)


#define  KEY0_PRES  1       //KEY0����
#define  KEY1_PRES  2       //KEY1����
#define  KEY2_PRES  3       //KEY2����
#define  WKUP_PRES  4       //WKUP_PRES����


void Key_Init(void);
u8 Key_Scan(u8);


#endif

