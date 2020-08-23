#include "stm32f10x.h"
#include "CapMode.h"

void TIM5_CapMode_Init(u16 arr,u16 psc)
{     

     GPIO_InitTypeDef GPIO_InitStruct;
     TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
     NVIC_InitTypeDef NVIC_InitStruct;
     TIM_ICInitTypeDef TIM_ICInitTStruct;
     
     RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE); //使能GPIOa
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); //使能定时器5
      
     GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;  //上拉输入（输入捕获模式）
     GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
     GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
     GPIO_Init(GPIOA,&GPIO_InitStruct);
     
     TIM_TimeBaseInitStruct.TIM_Prescaler = psc;       //预分频器
     TIM_TimeBaseInitStruct.TIM_Period = arr;              //装载值
     TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;         //计数模式
     TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;        //预分频系数
     TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStruct);
     
     TIM_ICInitTStruct.TIM_Channel = TIM_Channel_1;
     TIM_ICInitTStruct.TIM_ICFilter = 0x00;
     TIM_ICInitTStruct.TIM_ICPolarity =  TIM_ICPolarity_Rising;
     TIM_ICInitTStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
     TIM_ICInitTStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
     
     TIM_ICInit(TIM5,&TIM_ICInitTStruct);      //捕获模式初始化函数
    
     NVIC_InitStruct.NVIC_IRQChannel = TIM5_IRQn;
     NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
     NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;  //Preem抢先优先级
     NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;     //响应优先级
     NVIC_Init(&NVIC_InitStruct);
     
     TIM_ITConfig(TIM5,TIM_IT_Update|TIM_IT_CC1,ENABLE);
     TIM_Cmd(TIM5,ENABLE); 

      //Time3_Pwm_Init(899,0);	 //不分频。PWM频率=72000000/900=80Khz
      //TIM5_CapMode_Init(0xffff,72-1);     
     
}

