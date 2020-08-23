#ifndef _PID_H_
#define _PID_H_
#include "sys.h"
#include "usart.h"

void rin(u8 t); //电机转速预定值
void rout(u8 t); //电机转速输出值
void Pid_Init(float Target,float Kpp,float Kii,float Kdd );
float Pid_Realize(float Encoder);


typedef struct {

    float SetSpeed;  //定义设定值
    float ActualSpeed; //定义实际值
    float err;           //定义偏差值
    float err_last;       //定义上一个偏差
    float Kp,Ki,Kd;       //定义比例，积分，微分
    float voltage;        //定义目标速度
    float integral;  //定义积分值
    float umax;     //如果u(k-1)>umax累加负偏差
    float umin;     //如果 u(k-1)<umin，则只累加正偏差

}PID_InitTypeDef;





 
#endif

