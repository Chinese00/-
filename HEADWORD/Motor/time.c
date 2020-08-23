#include "stm32f10x.h"
#include "time.h"
#include "led.h"
#include "usart.h"
#include "encoder.h"
#include "PID.H"
#include "lcd.h"
#include <stdio.h>
#include <string.h>



int number = 0;
float Pid_V = 0.0;   //调节输出的速度
float Actual_velocity = 50;   //实际速度
u8 P[5],I[5],D[5] ,PID_V[10];
float PP[4] = {1.5,0.2,0,1500};  //EPPROM储存读数据的缓冲带


extern PID_InitTypeDef Pid;
extern int	Encoder_Left;
extern unsigned int PWMA;
extern float Incremental_PI (float encoder,float target);
extern short Flag1;
extern int x;

 void Limit_Pwm(void) //限制PWM幅度的函数
 {
   int Amplitude=3000;  //===PWM满幅对应的速度
	 if(Pid_V<-Amplitude)  Pid_V = -Amplitude;  
	 if(Pid_V>Amplitude)   Pid_V =  Amplitude;	 
 }

//函数功能：绝对值函数
         
float myabs(float dat) //取绝对值
{ 		   
	 int temp;
	 if(dat<0)  temp = -dat;  
	 else temp = dat;
	 return temp;
}

//速度转换,由单位时间的脉冲数来求出每分分钟转了多少转
float Velocity(float dat)
{
     int Target_velocity;
     
     Target_velocity = ((dat*5.0)/12.0)*60;
     
     return Target_velocity;
     
}
//浮点型数字转换成字符函数
void LCD_Char_Change(void)
{
         
     sprintf((char*)PID_V,"%0.2f",Pid_V);
     sprintf((char*)P,"%0.2f",Pid.Kp);
     sprintf((char*)I,"%0.2f",Pid.Ki);
     sprintf((char*)D,"%0.2f",Pid.Kd);
     Pid.Kp = PP[0];
     Pid.Ki = PP[1];
     Pid.Kd = PP[2];
     Pid.SetSpeed = PP[3];
     
 
}

//定时器1PWM输出
void Time1_Pwm_Init(u16 arr,u16 psc) //初始化pwm输出引脚
{
     GPIO_InitTypeDef GPIO_InitStruct;
     TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
     TIM_OCInitTypeDef TIM_OCInitStruct;

     RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);  //使能定时器1时钟
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); //使能GPIOA的时钟 
     
     GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
     GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
     GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_Init(GPIOA,&GPIO_InitStruct); 
     
     TIM_TimeBaseInitStruct.TIM_Prescaler =  psc;  //预分频器
     TIM_TimeBaseInitStruct.TIM_Period = arr;   //Period自动重装值
     TIM_TimeBaseInitStruct.TIM_CounterMode =   TIM_CounterMode_Up;
     TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
     //TIM_TimeBaseInitStruct.TIM_RepetitionCounter =  
     TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStruct);
      
     TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1; //模式1
     TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
     TIM_OCInitStruct.TIM_Pulse = 0; 
     TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
  
     TIM_OC1Init(TIM1,&TIM_OCInitStruct); 
     TIM_CtrlPWMOutputs(TIM1,ENABLE);  //高级定时器输出使能 
     TIM_OC1PreloadConfig(TIM1,TIM_OCPreload_Enable); //CH1使能预装载寄存器
     TIM_ARRPreloadConfig(TIM1, ENABLE);              //使能TIM1在ARR上的预装载寄存器
     TIM_Cmd(TIM1,ENABLE);                            //使能定时器1
         
}




//定时器2初始化
void Time2_Init(u16 arr,u16 psc)
{
      TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
      NVIC_InitTypeDef NVIC_InitStruct;     
       
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

      TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
      TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //计数方式
      TIM_TimeBaseInitStruct.TIM_Period = arr; //自动装载值
      TIM_TimeBaseInitStruct.TIM_Prescaler = psc;   //预分频系数

      TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);
      TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE); //使能
      
      NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
      NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
      NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
      NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
      NVIC_Init(&NVIC_InitStruct); 

     TIM_Cmd(TIM2,ENABLE);      
             
}


//定时器3PWM输出，暂时不用
void Time3_Pwm_Init(u16 arr,u16 psc)
{
     GPIO_InitTypeDef GPIO_TypeDefInit;
     TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
     TIM_OCInitTypeDef TIM_OCInitStruct;
     
     RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE); //使能定时器3的时钟
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,ENABLE); //
     
     GPIO_TypeDefInit.GPIO_Mode = GPIO_Mode_AF_PP;
     GPIO_TypeDefInit.GPIO_Pin = GPIO_Pin_7;//
     GPIO_TypeDefInit.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_Init(GPIOC,&GPIO_TypeDefInit); //
     
     GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,ENABLE); //重映射函数
     
     TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
     TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //计数方式
     TIM_TimeBaseInitStruct.TIM_Period = arr; //自动装载值
     TIM_TimeBaseInitStruct.TIM_Prescaler = psc;   //预分频系数
     TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);
     
     TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;        //模式    
     TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;        //
     TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;   //极性
     
     TIM_OC2Init(TIM3,&TIM_OCInitStruct);
     
     TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);
     
     TIM_Cmd(TIM3,ENABLE);      //使能定时器3
     
          
}

//定时器4初始化
void Time4_Init(u16 arr,u16 psc)
{

      TIM_TimeBaseInitTypeDef TIM_TimeInitStruct;
      NVIC_InitTypeDef NVIC_InitStruct3;     
       
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);

      TIM_TimeInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
      TIM_TimeInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //计数方式
      TIM_TimeInitStruct.TIM_Period = arr; //自动装载值
      TIM_TimeInitStruct.TIM_Prescaler = psc;   //预分频系数

      TIM_TimeBaseInit(TIM4,&TIM_TimeInitStruct);
      
      TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //使能
      
      NVIC_InitStruct3.NVIC_IRQChannel = TIM4_IRQn;
      NVIC_InitStruct3.NVIC_IRQChannelCmd = ENABLE;
      NVIC_InitStruct3.NVIC_IRQChannelPreemptionPriority = 0;
      NVIC_InitStruct3.NVIC_IRQChannelSubPriority = 3;
      NVIC_Init(&NVIC_InitStruct3); 

      TIM_Cmd(TIM4,ENABLE);  
    
     /*弃用
      TIM_TimeBaseInitTypeDef TIM_TimeInitStruct;
      NVIC_InitTypeDef NVIC_InitStruct3;     
       
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

      TIM_TimeInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
      TIM_TimeInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //计数方式
      TIM_TimeInitStruct.TIM_Period = arr; //自动装载值
      TIM_TimeInitStruct.TIM_Prescaler = psc;   //预分频系数

      TIM_TimeBaseInit(TIM3,&TIM_TimeInitStruct);
      
      TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //使能
      
      NVIC_InitStruct3.NVIC_IRQChannel = TIM3_IRQn;
      NVIC_InitStruct3.NVIC_IRQChannelCmd = ENABLE;
      NVIC_InitStruct3.NVIC_IRQChannelPreemptionPriority = 0;
      NVIC_InitStruct3.NVIC_IRQChannelSubPriority = 3;
      NVIC_Init(&NVIC_InitStruct3); 

      TIM_Cmd(TIM3,ENABLE);  
      */
     
}


void TIM2_IRQHandler()
{    
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源
        Encoder_Left  = Read_Encoder(3);
        
        Actual_velocity = Velocity(Encoder_Left); //实际速度
         
        Pid_V = Pid_Realize(Actual_velocity);	//PId进行调节
        Limit_Pwm();
        if(Flag1==0)PWMA = myabs(Pid_V*2/7);
      
    }
    
}
//光标显示函数
void Cursor_On(int dat)  //光标亮
{
      switch(dat)
      {
         case 0:LCD_ShowChar(220,90,'<',24,0);break;//Set_V
         case 1:LCD_ShowChar(220,180,'<',24,0);break;//PWMA
         case 2:LCD_ShowChar(150,210,'<',24,0);break;//P
         case 3:LCD_ShowChar(150,250,'<',24,0);break;//I
         case 4:LCD_ShowChar(150,290,'<',24,0);break;//D
         case 5:LCD_ShowChar(150,330,'<',24,0);break;//Direction
      }
      
}
void Cursor_Down(int dat)  //光标灭
{
      switch(dat)
      {
         case 0:LCD_ShowChar(220,90,' ',24,0);break;//Set_V
         case 1:LCD_ShowChar(220,180,' ',24,0);break;//PWMA
         case 2:LCD_ShowChar(150,210,' ',24,0);break;//P
         case 3:LCD_ShowChar(150,250,' ',24,0);break;//I
         case 4:LCD_ShowChar(150,290,' ',24,0);break;//D
         case 5:LCD_ShowChar(150,330,' ',24,0);break;//Direction
      }
}

void TIM4_IRQHandler()       //用来刷新显示屏，光标显示
{ 
    static int Number = 0;
    
    if (TIM_GetITStatus(TIM4,TIM_IT_Update) != RESET)
    { 
          TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
          
          
          if(Number > 9)
          {
             LCD_Char_Change();
             
             Encoder_Left  = Read_Encoder(3);
             Actual_velocity = Velocity(Encoder_Left); //实际速度
             
             printf("%d,%d,/r/n",(int)Pid.SetSpeed,(int)Actual_velocity);//数据采集采用CSV格式
             LCD_ShowxNum(220,60,Encoder_Left,4,24,0); 
             LCD_ShowxNum(140,90,Pid.SetSpeed,6,24,0);
             LCD_ShowxNum(140,120,Actual_velocity,6,24,0);

                                                      
             LCD_ShowString(140,150,200,24,24,PID_V,0);
             LCD_ShowxNum(140,180,PWMA,6,24,0);
                                                       //LCD_ShowxNum(90,210,Pid.Kp,6,24,0);
             LCD_ShowString(90,210,200,24,24,P,0);
                                                       //LCD_ShowxNum(90,240,Pid.Ki,6,24,0);
             LCD_ShowString(90,250,200,24,24,I,0);
                                                       //LCD_ShowxNum(90,270,Pid.Kd,6,24,0);
             LCD_ShowString(90,290,200,24,24,D,0);
             Number = 0;
          }
          Number++;
          if(Number==5&&Flag1!=0) //光标亮开启
          {
             Cursor_On(x);
          }
          if(Number==10&&Flag1!=0) //光标灭开启
          {
             Cursor_Down(x);
          }
    }
   
    

    
}


