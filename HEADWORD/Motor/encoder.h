#ifndef __ENCODER_H
#define __ENCODER_H
#include <sys.h>
	 
#define ENCODER_TIM_PERIOD (u16)(65535)   //103的定时器是16位 
void Encoder_Init_TIM3(void);							//定时器4 测频率
int Read_Encoder(u8 TIMX);								//单位时间读取编码器计数
void TIM4_IRQHandler(void);								//TIM4中断
#endif


