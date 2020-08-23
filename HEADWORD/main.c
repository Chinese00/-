#include "stm32f10x.h"
#include "TIME.H"
#include "delay.h"
#include "led.h"
#include "usart.h"
#include "lcd.h"
#include "key.h"
#include "PID.h"
#include "encoder.h"
#include "sys.h"
#include "time.h"
#include "24cxx.h"
/*
没有实现的功能
1：不能通过上位机实时更改PID和转速
2：没有使用操作系统
*/

unsigned int PWMA = 0;
int	Encoder_Left = 0; 
short  Flag1 = 0,Flag2 = 0,Flag3 = 0; //标志位
short  Flag_Pid;   //是否启用PID
int x = 5;
extern float PP[4];



extern float Actual_velocity;		//实际速度
extern PID_InitTypeDef Pid;



void MOTO_Init(void)//初始化控制电机所需的IO
{
  GPIO_InitTypeDef GPIO_InitStruct;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB,&GPIO_InitStruct);
  
   PBout(14) = 0;
   PBout(15) = 1;
 	
  
}
void Pre_Write_Read()   //开机时读取24c02中的数据
{
    AT24CXX_Read(0,(u8*)PP,4);  
    AT24CXX_Read(5,(u8*)&PP[1],4); 
    AT24CXX_Read(9,(u8*)&PP[2],4);
    AT24CXX_Read(13,(u8*)&PP[3],4);     
    
}

void LcdInit()
{
  LCD_Clear(GRAY);
  POINT_COLOR = DARKBLUE;
  LCD_ShowString(30,30,200,24,24,"Welcom To New",1);
  POINT_COLOR = RED;
  LCD_ShowString(30,60,200,24,24,"Encoder_Left: =",1);
  LCD_ShowString(30,90,200,24,24,"Set_V: = ",1);
  LCD_ShowString(30,120,200,24,24,"Act_V: =",1);
  LCD_ShowString(30,150,200,24,24,"PID_V: =",1);
  LCD_ShowString(30,180,200,24,24,"PWMA : = ",1); 
  POINT_COLOR =  BLACK;
  LCD_ShowString(30,210,200,24,24,"Kp: = ",1);
  LCD_ShowString(30,250,200,24,24,"Ki: =",1);
  LCD_ShowString(30,290,200,24,24,"Kd: =",1);
  LCD_ShowString(30,330,200,24,24,"Dr: = Y",1);    
  POINT_COLOR = 0x780F; 
  LCD_ShowChar(450,388,'>',24,1);
  LCD_ShowChar(440,360,'T',24,1);
  LCD_DrawLine(30, 400, 450, 400);
  LCD_DrawLine(30, 400, 30, 750);
  LCD_ShowChar(24,740,'V',24,1);
  
}


void Function_Init()
{
    delay_init();	    	 //延时函数初始化	
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	  uart_init(115200);	 //串口初始化为115200
 	  Led_Init();	 //LED端口初始化
    MOTO_Init();						//电机状态初始化
    Key_Init();
    Pid_Init(1500,1.5,0.3,0);//1.8 0.5 0.2 //0.5 2 0//
    AT24CXX_Init();			//IIC初始化
    Pre_Write_Read();    
    LCD_Init();
  
    Time1_Pwm_Init(999,71);  //1KHz,PWM分成1000份
    Time4_Init(99,7199);    //0.01s
    Time2_Init(49,7199);    //5ms 定时器2 中断一次 50 * 1/10000  计数器计数的频率为10K
    Encoder_Init_TIM3();
    LcdInit();
    

}


int main(void)
{
   
    u8 Key;
    
    Function_Init();
    
    AT24CXX_Read(0,(u8*)PP,4);  
    AT24CXX_Read(5,(u8*)&PP[1],4); 
    AT24CXX_Read(9,(u8*)&PP[2],4);
    AT24CXX_Read(13,(u8*)&PP[3],4); 
    
    while(1)
    {
         
         TIM_SetCompare1(TIM1,PWMA);
         Key =  Key_Scan(0);
         LCD_SetCursor(500,60);
         
         
         if(Key)
         {
           
            if(Key==WKUP_PRES) //是否进入参数模式
            {  
               Flag1++;
               LED0 = !LED0;
               if(Flag1==2)
               {
                 Flag1 = 0;
                 
               }
             }
                        
            if(Key==KEY2_PRES)
             {
                 
                Flag2++;
                if(Flag1==1&&Flag2==1) x = 0;     //Tar_v            
                if(Flag1==1&&Flag2==2) x = 1;      //PWMA
                if(Flag1==1&&Flag2==3) x = 2;    //P
                if(Flag1==1&&Flag2==4) x = 3;  //I
                if(Flag1==1&&Flag2==5) x = 4;   //D
                if(Flag1==1&&Flag2==6) x = 5;   //D
                if(Flag2>6){Flag2 = 0;   x=6;};                 
                                         
              }
              if(x==1&&Key==KEY1_PRES)PWMA+=50;     //调高占空比，满幅1000                 
              if(x==1&&Key==KEY0_PRES)PWMA-=50;     
              
              if(x==0&&Key==KEY1_PRES) //目标速度加100，满幅3400左右，并把速度保存起来断电不丢失
              {
                  PP[3]+=100;
                  AT24CXX_Write(13,(u8*)&PP[3],4);    
              }                 
              if(x==0&&Key==KEY0_PRES)
              {
                  PP[3]-=100;
                  AT24CXX_Write(13,(u8*)&PP[3],4);
              }
              
              if(x==2&&Key==KEY1_PRES)   //加比例并保存
              {
                  PP[0]+=0.1;
                  AT24CXX_Write(0,(u8*)PP,4);
                
              }
              if(x==2&&Key==KEY0_PRES)
              {
                  PP[0]-=0.1;
                  AT24CXX_Write(0,(u8*)PP,4);
              }
              
              if(x==3&&Key==KEY1_PRES)   //加积分并保存
              {
                  PP[1]+=0.1;
                  AT24CXX_Write(5,(u8*)&PP[1],4);
                  
              }              
              if(x==3&&Key==KEY0_PRES)
              {
                   PP[1]-=0.1;
                   AT24CXX_Write(5,(u8*)&PP[1],4);
              }
              
              if(x==4&&Key==KEY1_PRES) //加微分并保存
              {
                  PP[2]+=0.1;
                  AT24CXX_Write(9,(u8*)&PP[2],4);
              }                 
              if(x==4&&Key==KEY0_PRES)  
              {
                   PP[2]-=0.1;
                  AT24CXX_Write(9,(u8*)&PP[2],4);
              } 
              if(x==5&&Key==KEY1_PRES)  //正转
              {
                   PBout(14) = 0;
                   PBout(15) = 1;
                   LCD_ShowString(30,330,200,24,24,"Dr: = Y",0);  
              } 
              if(x==5&&Key==KEY0_PRES)  //反转
              {
                   PBout(14) = 1;
                   PBout(15) = 0;
                   LCD_ShowString(30,330,200,24,24,"Dr: = N",0);   
              }                          
             if(PWMA>1100)
             {
               PWMA = 0;
             }
         } else delay_ms(10);
            
        
    }
}
   	
  

  
    /*此部分可忽略
    
    
    
    
       switch(Key)
            {     
               //case KEY1_PRES:PWMA+=50;printf("\r\n预调节的PWM：%d\r\n",PWMA);LED0 = !LED0;break;
               case KEY2_PRES:TIM_SetCompare1(TIM1,PWMA);printf("\r\n实际输入的PWM：%d\r\n",PWMA);break;
               case KEY0_PRES:Test_Send_User(Encoder_Left,Actual_velocity);break;
               case WKUP_PRES:printf("\r\n实际输入的PWM：%d\r\n",PWMA);break;
            } 
          } else delay_ms(10);
          
            
       key=KEY_Scan(0);	//得到键值
	   	if(key)
		{						   
			switch(key)
			{				 
				case WKUP_PRES:	//控制蜂鸣器
					BEEP=!BEEP;
					break;
				case KEY2_PRES:	//控制LED0翻转
					LED0=!LED0;
					break;
				case KEY1_PRES:	//控制LED1翻转	 
					LED1=!LED1;
					break;
				case KEY0_PRES:	//同时控制LED0,LED1翻转 
					LED0=!LED0;
					LED1=!LED1;
					break;
			}
		}else delay_ms(10); 
	}	
    
    */
    
    
    /*
    
    
    if(Count < 1000)
          {
              speed=Pid_Realize(200.0);
             printf("\r\n:%f\r\n",speed);
           }
          Count++;
          if(Count>1100)
          {
            Count = 1100;
          }
    
    while(1)
	{
 		delay_ms(10);	 
		if(dir)led0pwmval++;
		else led0pwmval--;

 		if(led0pwmval>300)dir=0;
		if(led0pwmval==0)dir=1;										 
		TIM_SetCompare2(TIM3,led0pwmval);
     if(TIM5CH1_CAPTURE_STA&0X80)//成功捕获到了一次上升沿
          {
             temp=TIM5CH1_CAPTURE_STA&0X3F;
             temp*=65536;//溢出时间总和
             temp+=TIM5CH1_CAPTURE_VAL;//得到总的高电平时间
             printf("HIGH:%d us\r\n",temp); //打印总的高点平时间
             TIM5CH1_CAPTURE_STA=0; //开启下一次捕获
           }    
	}	
  */
  
  

/*
void TIM5_IRQHandler()
{
    if((TIM5CH1_CAPTURE_STA&0x80) == 0)
      {
          if(TIM_GetITStatus(TIM5,TIM_IT_Update) == SET) //已经更新中断
          {
              if(TIM5CH1_CAPTURE_STA&0x40)     //已经捕获高电平
              {
                if((TIM5CH1_CAPTURE_STA&0x3F)==0x3F)//高电平持续时间太长
                   {
                      TIM5CH1_CAPTURE_STA|=0x80;
                      TIM5CH1_CAPTURE_VAL = 0xFFFF;
                   }else  TIM5CH1_CAPTURE_STA++;               
              }
          }
          if(TIM_GetITStatus(TIM5,TIM_IT_CC1) == SET)
          {
               if(TIM5CH1_CAPTURE_STA&0X40) //捕获到一个下降沿
                  { 
                      TIM5CH1_CAPTURE_STA|=0X80;  //标记成功捕获到一次上升沿
                      TIM5CH1_CAPTURE_VAL=TIM_GetCapture1(TIM5);
                      TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Rising); //设置为上升沿捕获
                  }
                else
                  {
                      TIM5CH1_CAPTURE_STA=0;  //清空
                      TIM5CH1_CAPTURE_VAL=0;
                      TIM_SetCounter(TIM5,0);
                      TIM5CH1_CAPTURE_STA|=0X40;  //标记捕获到了上升沿
                      TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Falling); //设置为下降沿捕获
                  } 
                 
           }
          
       } 
          
           TIM_ClearITPendingBit(TIM5, TIM_IT_CC1|TIM_IT_Update); //清除中断标志位
             
         
}

*/





