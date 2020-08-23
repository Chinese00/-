#include "pid.h"
#include "usart.h"
#include "stdio.h"
#include "stdlib.h"

PID_InitTypeDef Pid;



void Pid_Init(float Target,float Kpp,float Kii,float Kdd )
{
      // printf("Pid_Init Begain: \r\n");
       Pid.SetSpeed = Target;
       Pid.ActualSpeed = 0.0;
       Pid.err = 0.0;
       Pid.err_last = 0.0;
       Pid.voltage  = 0.0;
       Pid.integral = 0.0;
       Pid.Kp = Kpp;
       Pid.Ki = Kii;
       Pid.Kd = Kdd;
       Pid.umax = 400;
       Pid.umin = -200;
       //printf("\r\nPid_Init Over: \r\n");
}

float Pid_Realize(float Encoder)
{
        float index;
        
        Pid.ActualSpeed = Encoder;
        Pid.err =Pid.SetSpeed -  Pid.ActualSpeed ;  //偏差值（实际速度与设定速度）
        
    
        
         if(abs(Pid.err)>1500) index = 0.0;//积分分离
          
         else if(abs(Pid.err)<1000)
        {  
              index=1.0;
              Pid.integral+=Pid.err;
        }
         else
         {
              index=(200-abs(Pid.err))/20;
              Pid.integral += Pid.err;
         }
            
       
        Pid.voltage = Pid.Kp*Pid.err + index*Pid.Ki*Pid.integral*1/2 + Pid.Kd*(Pid.err - Pid.err_last);//梯形积分
        Pid.err_last = Pid.err;
        Pid.ActualSpeed = Pid.voltage*1.0;
        return Pid.ActualSpeed;       
}
/*

//Pid.voltage = Pid.Kp*Pid.err + index*Pid.Ki*Pid.integral + Pid.Kd*(Pid.err - Pid.err_last);

           if(Pid.ActualSpeed>Pid.umax) //灰色底色表示抗积分饱和的实现
           {
               if(abs(Pid.err)>200) //积分分离
                {
                    index = 0.0;
                }
             
                else
                {
                    index = 1;
                    if(Pid.err<0)
                     {
                      Pid.integral += Pid.err;
                      }
                 }
            }
          else if(Pid.ActualSpeed<Pid.umin)
           {
                 if(abs(Pid.err)>200) //积分分离
                   {
                         index = 0;
                   }
                   else
                   {
                      index = 1;
                      if(Pid.err>0)
                      {
                          Pid.integral += Pid.err;
                       }
                     }
            }
           else
           {
              if(abs(Pid.err)>200) //积分分离过程
               {
                  index=0;
                }
                else
                {             
                   index=1;
                   Pid.integral+=Pid.err;
                 }
            }
            
            
            float Incremental_PI (float Encoder,float Target)
{ 	

	//增量式PI控制器 无需累加历史偏差 使控制更加准确 减弱了计算负荷 求的是输出量(被控量)的增量
	
   float Kp=5,Ki=0.072;	
	 static int Err,Pwm = 5000,Last_Err;
	 Err=Encoder-Target;                //计算偏差 target 为目标
	 Pwm+=Kp*(Err-Last_Err)+Ki*Err;   //PWM求出增量并且计算PWM
	 Last_Err=Err;	                   //保存上一次偏差 
	 return Pwm;                       //增量输出
   
   
} 
*/  



