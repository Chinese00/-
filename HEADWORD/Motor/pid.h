#ifndef _PID_H_
#define _PID_H_
#include "sys.h"
#include "usart.h"

void rin(u8 t); //���ת��Ԥ��ֵ
void rout(u8 t); //���ת�����ֵ
void Pid_Init(float Target,float Kpp,float Kii,float Kdd );
float Pid_Realize(float Encoder);


typedef struct {

    float SetSpeed;  //�����趨ֵ
    float ActualSpeed; //����ʵ��ֵ
    float err;           //����ƫ��ֵ
    float err_last;       //������һ��ƫ��
    float Kp,Ki,Kd;       //������������֣�΢��
    float voltage;        //����Ŀ���ٶ�
    float integral;  //�������ֵ
    float umax;     //���u(k-1)>umax�ۼӸ�ƫ��
    float umin;     //��� u(k-1)<umin����ֻ�ۼ���ƫ��

}PID_InitTypeDef;





 
#endif

