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
û��ʵ�ֵĹ���
1������ͨ����λ��ʵʱ����PID��ת��
2��û��ʹ�ò���ϵͳ
*/

unsigned int PWMA = 0;
int	Encoder_Left = 0; 
short  Flag1 = 0,Flag2 = 0,Flag3 = 0; //��־λ
short  Flag_Pid;   //�Ƿ�����PID
int x = 5;
extern float PP[4];



extern float Actual_velocity;		//ʵ���ٶ�
extern PID_InitTypeDef Pid;



void MOTO_Init(void)//��ʼ�����Ƶ�������IO
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
void Pre_Write_Read()   //����ʱ��ȡ24c02�е�����
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
    delay_init();	    	 //��ʱ������ʼ��	
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	  uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
 	  Led_Init();	 //LED�˿ڳ�ʼ��
    MOTO_Init();						//���״̬��ʼ��
    Key_Init();
    Pid_Init(1500,1.5,0.3,0);//1.8 0.5 0.2 //0.5 2 0//
    AT24CXX_Init();			//IIC��ʼ��
    Pre_Write_Read();    
    LCD_Init();
  
    Time1_Pwm_Init(999,71);  //1KHz,PWM�ֳ�1000��
    Time4_Init(99,7199);    //0.01s
    Time2_Init(49,7199);    //5ms ��ʱ��2 �ж�һ�� 50 * 1/10000  ������������Ƶ��Ϊ10K
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
           
            if(Key==WKUP_PRES) //�Ƿ�������ģʽ
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
              if(x==1&&Key==KEY1_PRES)PWMA+=50;     //����ռ�ձȣ�����1000                 
              if(x==1&&Key==KEY0_PRES)PWMA-=50;     
              
              if(x==0&&Key==KEY1_PRES) //Ŀ���ٶȼ�100������3400���ң������ٶȱ��������ϵ粻��ʧ
              {
                  PP[3]+=100;
                  AT24CXX_Write(13,(u8*)&PP[3],4);    
              }                 
              if(x==0&&Key==KEY0_PRES)
              {
                  PP[3]-=100;
                  AT24CXX_Write(13,(u8*)&PP[3],4);
              }
              
              if(x==2&&Key==KEY1_PRES)   //�ӱ���������
              {
                  PP[0]+=0.1;
                  AT24CXX_Write(0,(u8*)PP,4);
                
              }
              if(x==2&&Key==KEY0_PRES)
              {
                  PP[0]-=0.1;
                  AT24CXX_Write(0,(u8*)PP,4);
              }
              
              if(x==3&&Key==KEY1_PRES)   //�ӻ��ֲ�����
              {
                  PP[1]+=0.1;
                  AT24CXX_Write(5,(u8*)&PP[1],4);
                  
              }              
              if(x==3&&Key==KEY0_PRES)
              {
                   PP[1]-=0.1;
                   AT24CXX_Write(5,(u8*)&PP[1],4);
              }
              
              if(x==4&&Key==KEY1_PRES) //��΢�ֲ�����
              {
                  PP[2]+=0.1;
                  AT24CXX_Write(9,(u8*)&PP[2],4);
              }                 
              if(x==4&&Key==KEY0_PRES)  
              {
                   PP[2]-=0.1;
                  AT24CXX_Write(9,(u8*)&PP[2],4);
              } 
              if(x==5&&Key==KEY1_PRES)  //��ת
              {
                   PBout(14) = 0;
                   PBout(15) = 1;
                   LCD_ShowString(30,330,200,24,24,"Dr: = Y",0);  
              } 
              if(x==5&&Key==KEY0_PRES)  //��ת
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
   	
  

  
    /*�˲��ֿɺ���
    
    
    
    
       switch(Key)
            {     
               //case KEY1_PRES:PWMA+=50;printf("\r\nԤ���ڵ�PWM��%d\r\n",PWMA);LED0 = !LED0;break;
               case KEY2_PRES:TIM_SetCompare1(TIM1,PWMA);printf("\r\nʵ�������PWM��%d\r\n",PWMA);break;
               case KEY0_PRES:Test_Send_User(Encoder_Left,Actual_velocity);break;
               case WKUP_PRES:printf("\r\nʵ�������PWM��%d\r\n",PWMA);break;
            } 
          } else delay_ms(10);
          
            
       key=KEY_Scan(0);	//�õ���ֵ
	   	if(key)
		{						   
			switch(key)
			{				 
				case WKUP_PRES:	//���Ʒ�����
					BEEP=!BEEP;
					break;
				case KEY2_PRES:	//����LED0��ת
					LED0=!LED0;
					break;
				case KEY1_PRES:	//����LED1��ת	 
					LED1=!LED1;
					break;
				case KEY0_PRES:	//ͬʱ����LED0,LED1��ת 
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
     if(TIM5CH1_CAPTURE_STA&0X80)//�ɹ�������һ��������
          {
             temp=TIM5CH1_CAPTURE_STA&0X3F;
             temp*=65536;//���ʱ���ܺ�
             temp+=TIM5CH1_CAPTURE_VAL;//�õ��ܵĸߵ�ƽʱ��
             printf("HIGH:%d us\r\n",temp); //��ӡ�ܵĸߵ�ƽʱ��
             TIM5CH1_CAPTURE_STA=0; //������һ�β���
           }    
	}	
  */
  
  

/*
void TIM5_IRQHandler()
{
    if((TIM5CH1_CAPTURE_STA&0x80) == 0)
      {
          if(TIM_GetITStatus(TIM5,TIM_IT_Update) == SET) //�Ѿ������ж�
          {
              if(TIM5CH1_CAPTURE_STA&0x40)     //�Ѿ�����ߵ�ƽ
              {
                if((TIM5CH1_CAPTURE_STA&0x3F)==0x3F)//�ߵ�ƽ����ʱ��̫��
                   {
                      TIM5CH1_CAPTURE_STA|=0x80;
                      TIM5CH1_CAPTURE_VAL = 0xFFFF;
                   }else  TIM5CH1_CAPTURE_STA++;               
              }
          }
          if(TIM_GetITStatus(TIM5,TIM_IT_CC1) == SET)
          {
               if(TIM5CH1_CAPTURE_STA&0X40) //����һ���½���
                  { 
                      TIM5CH1_CAPTURE_STA|=0X80;  //��ǳɹ�����һ��������
                      TIM5CH1_CAPTURE_VAL=TIM_GetCapture1(TIM5);
                      TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Rising); //����Ϊ�����ز���
                  }
                else
                  {
                      TIM5CH1_CAPTURE_STA=0;  //���
                      TIM5CH1_CAPTURE_VAL=0;
                      TIM_SetCounter(TIM5,0);
                      TIM5CH1_CAPTURE_STA|=0X40;  //��ǲ�����������
                      TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Falling); //����Ϊ�½��ز���
                  } 
                 
           }
          
       } 
          
           TIM_ClearITPendingBit(TIM5, TIM_IT_CC1|TIM_IT_Update); //����жϱ�־λ
             
         
}

*/





