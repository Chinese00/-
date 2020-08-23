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
float Pid_V = 0.0;   //����������ٶ�
float Actual_velocity = 50;   //ʵ���ٶ�
u8 P[5],I[5],D[5] ,PID_V[10];
float PP[4] = {1.5,0.2,0,1500};  //EPPROM��������ݵĻ����


extern PID_InitTypeDef Pid;
extern int	Encoder_Left;
extern unsigned int PWMA;
extern float Incremental_PI (float encoder,float target);
extern short Flag1;
extern int x;

 void Limit_Pwm(void) //����PWM���ȵĺ���
 {
   int Amplitude=3000;  //===PWM������Ӧ���ٶ�
	 if(Pid_V<-Amplitude)  Pid_V = -Amplitude;  
	 if(Pid_V>Amplitude)   Pid_V =  Amplitude;	 
 }

//�������ܣ�����ֵ����
         
float myabs(float dat) //ȡ����ֵ
{ 		   
	 int temp;
	 if(dat<0)  temp = -dat;  
	 else temp = dat;
	 return temp;
}

//�ٶ�ת��,�ɵ�λʱ��������������ÿ�ַ���ת�˶���ת
float Velocity(float dat)
{
     int Target_velocity;
     
     Target_velocity = ((dat*5.0)/12.0)*60;
     
     return Target_velocity;
     
}
//����������ת�����ַ�����
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

//��ʱ��1PWM���
void Time1_Pwm_Init(u16 arr,u16 psc) //��ʼ��pwm�������
{
     GPIO_InitTypeDef GPIO_InitStruct;
     TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
     TIM_OCInitTypeDef TIM_OCInitStruct;

     RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);  //ʹ�ܶ�ʱ��1ʱ��
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); //ʹ��GPIOA��ʱ�� 
     
     GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
     GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
     GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_Init(GPIOA,&GPIO_InitStruct); 
     
     TIM_TimeBaseInitStruct.TIM_Prescaler =  psc;  //Ԥ��Ƶ��
     TIM_TimeBaseInitStruct.TIM_Period = arr;   //Period�Զ���װֵ
     TIM_TimeBaseInitStruct.TIM_CounterMode =   TIM_CounterMode_Up;
     TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
     //TIM_TimeBaseInitStruct.TIM_RepetitionCounter =  
     TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStruct);
      
     TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1; //ģʽ1
     TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
     TIM_OCInitStruct.TIM_Pulse = 0; 
     TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
  
     TIM_OC1Init(TIM1,&TIM_OCInitStruct); 
     TIM_CtrlPWMOutputs(TIM1,ENABLE);  //�߼���ʱ�����ʹ�� 
     TIM_OC1PreloadConfig(TIM1,TIM_OCPreload_Enable); //CH1ʹ��Ԥװ�ؼĴ���
     TIM_ARRPreloadConfig(TIM1, ENABLE);              //ʹ��TIM1��ARR�ϵ�Ԥװ�ؼĴ���
     TIM_Cmd(TIM1,ENABLE);                            //ʹ�ܶ�ʱ��1
         
}




//��ʱ��2��ʼ��
void Time2_Init(u16 arr,u16 psc)
{
      TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
      NVIC_InitTypeDef NVIC_InitStruct;     
       
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

      TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
      TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //������ʽ
      TIM_TimeBaseInitStruct.TIM_Period = arr; //�Զ�װ��ֵ
      TIM_TimeBaseInitStruct.TIM_Prescaler = psc;   //Ԥ��Ƶϵ��

      TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);
      TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE); //ʹ��
      
      NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
      NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
      NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
      NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
      NVIC_Init(&NVIC_InitStruct); 

     TIM_Cmd(TIM2,ENABLE);      
             
}


//��ʱ��3PWM�������ʱ����
void Time3_Pwm_Init(u16 arr,u16 psc)
{
     GPIO_InitTypeDef GPIO_TypeDefInit;
     TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
     TIM_OCInitTypeDef TIM_OCInitStruct;
     
     RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE); //ʹ�ܶ�ʱ��3��ʱ��
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,ENABLE); //
     
     GPIO_TypeDefInit.GPIO_Mode = GPIO_Mode_AF_PP;
     GPIO_TypeDefInit.GPIO_Pin = GPIO_Pin_7;//
     GPIO_TypeDefInit.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_Init(GPIOC,&GPIO_TypeDefInit); //
     
     GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,ENABLE); //��ӳ�亯��
     
     TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
     TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //������ʽ
     TIM_TimeBaseInitStruct.TIM_Period = arr; //�Զ�װ��ֵ
     TIM_TimeBaseInitStruct.TIM_Prescaler = psc;   //Ԥ��Ƶϵ��
     TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);
     
     TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;        //ģʽ    
     TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;        //
     TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;   //����
     
     TIM_OC2Init(TIM3,&TIM_OCInitStruct);
     
     TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);
     
     TIM_Cmd(TIM3,ENABLE);      //ʹ�ܶ�ʱ��3
     
          
}

//��ʱ��4��ʼ��
void Time4_Init(u16 arr,u16 psc)
{

      TIM_TimeBaseInitTypeDef TIM_TimeInitStruct;
      NVIC_InitTypeDef NVIC_InitStruct3;     
       
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);

      TIM_TimeInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
      TIM_TimeInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //������ʽ
      TIM_TimeInitStruct.TIM_Period = arr; //�Զ�װ��ֵ
      TIM_TimeInitStruct.TIM_Prescaler = psc;   //Ԥ��Ƶϵ��

      TIM_TimeBaseInit(TIM4,&TIM_TimeInitStruct);
      
      TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //ʹ��
      
      NVIC_InitStruct3.NVIC_IRQChannel = TIM4_IRQn;
      NVIC_InitStruct3.NVIC_IRQChannelCmd = ENABLE;
      NVIC_InitStruct3.NVIC_IRQChannelPreemptionPriority = 0;
      NVIC_InitStruct3.NVIC_IRQChannelSubPriority = 3;
      NVIC_Init(&NVIC_InitStruct3); 

      TIM_Cmd(TIM4,ENABLE);  
    
     /*����
      TIM_TimeBaseInitTypeDef TIM_TimeInitStruct;
      NVIC_InitTypeDef NVIC_InitStruct3;     
       
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

      TIM_TimeInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
      TIM_TimeInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //������ʽ
      TIM_TimeInitStruct.TIM_Period = arr; //�Զ�װ��ֵ
      TIM_TimeInitStruct.TIM_Prescaler = psc;   //Ԥ��Ƶϵ��

      TIM_TimeBaseInit(TIM3,&TIM_TimeInitStruct);
      
      TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //ʹ��
      
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
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //���TIMx���жϴ�����λ:TIM �ж�Դ
        Encoder_Left  = Read_Encoder(3);
        
        Actual_velocity = Velocity(Encoder_Left); //ʵ���ٶ�
         
        Pid_V = Pid_Realize(Actual_velocity);	//PId���е���
        Limit_Pwm();
        if(Flag1==0)PWMA = myabs(Pid_V*2/7);
      
    }
    
}
//�����ʾ����
void Cursor_On(int dat)  //�����
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
void Cursor_Down(int dat)  //�����
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

void TIM4_IRQHandler()       //����ˢ����ʾ���������ʾ
{ 
    static int Number = 0;
    
    if (TIM_GetITStatus(TIM4,TIM_IT_Update) != RESET)
    { 
          TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
          
          
          if(Number > 9)
          {
             LCD_Char_Change();
             
             Encoder_Left  = Read_Encoder(3);
             Actual_velocity = Velocity(Encoder_Left); //ʵ���ٶ�
             
             printf("%d,%d,/r/n",(int)Pid.SetSpeed,(int)Actual_velocity);//���ݲɼ�����CSV��ʽ
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
          if(Number==5&&Flag1!=0) //���������
          {
             Cursor_On(x);
          }
          if(Number==10&&Flag1!=0) //�������
          {
             Cursor_Down(x);
          }
    }
   
    

    
}


