#ifndef __ENCODER_H
#define __ENCODER_H
#include <sys.h>
	 
#define ENCODER_TIM_PERIOD (u16)(65535)   //103�Ķ�ʱ����16λ 
void Encoder_Init_TIM3(void);							//��ʱ��4 ��Ƶ��
int Read_Encoder(u8 TIMX);								//��λʱ���ȡ����������
void TIM4_IRQHandler(void);								//TIM4�ж�
#endif


