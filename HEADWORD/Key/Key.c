#include "stm32f10x.h"
#include "key.h"
#include "delay.h"
#include  "sys.h"
void Key_Init(void)
{
      GPIO_InitTypeDef GPIO_InitStruct;
			
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE,ENABLE);
       
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_3|GPIO_Pin_2;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;   //GPIO_Mode_IPU  上拉输入
			//GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;//GPIO_Speed_50MHz
			
		  GPIO_Init(GPIOE,&GPIO_InitStruct);
			
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;  //xia拉输入
			//GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
			
			GPIO_Init(GPIOA,&GPIO_InitStruct);
}

u8 Key_Scan(u8 MODE)
{
    static u8 Key_Up = 1;  //按键松开标志
    if(MODE)  Key_Up = 1;    //支持连续按压
		if(Key_Up&&(KEY0==0||KEY1==0||KEY2==0||WK_UP==1))
		{
		     delay_ms(10);
				 Key_Up = 0;
				 if(KEY0==0) return KEY0_PRES;
				 else if(KEY1==0)  return KEY1_PRES;
				 else if(KEY2==0)  return KEY2_PRES;
				 else if(WK_UP==1) return WKUP_PRES;
		}
		else if(KEY0==1&&KEY1==1&&KEY2==1&&WK_UP==0) Key_Up = 1;//确认按键已松开。这时可以进行操作
		return 0; //返回值为没有按键按下	 
			 			 
 			 
}
