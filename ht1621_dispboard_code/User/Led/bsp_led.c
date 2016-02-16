
#include "bsp_led.h"   
#include "bsp_TiMbase.h"

//#include "HT1621.h"

const u8 Ht1621Tab1[10]={0xfa,0x0a,0xbc,0x9e,0x4e,0xd6,0xf6,0x8a,0xfe,0xde}; //0123456789
volatile u8 Ht1621Tab2[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
volatile  u8 Ht1621_BUF[]={0,0,0,0,0,0,0,0,0,0,0};
u8 DIS_BUF[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
u8 Ht1621Tab[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};	
volatile u8 count=0,count1=0,count2=0,count3=0,count4=0;  //计数
volatile u8 key1 = 1; //----加一个按键标志//静电
volatile u8  key2 = 1; //----加一个按键标志//光氢
volatile u8  key3 = 1; //----加一个按键标志//开关
volatile u8  key4 = 1; //----加一个按键标志//
volatile u32  time = 0; // ms 计时变量 
static u16 KeyDownTimes,KeyDoubleTimes;

typedef enum 
{
 KeyDown,
 KeyUp,
 KeyShort,
 KeyLong
}KEYSTATE;
KEYSTATE KeyState;

void delay(u32 nCount)
{
	for(;nCount!=0;nCount--);
}


/****************************************************************************
* 名    称：delay_us(u32 nus)
* 功    能：微秒延时函数
* 入口参数：u32  nus
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void delay_us(u32 nus)
{
	 u32 temp;
	 SysTick->LOAD = 9*nus;
	 SysTick->VAL=0X00;//清空计数器
	 SysTick->CTRL=0X01;//使能，减到零是无动作，采用外部时钟源
	 do
	 {
	  temp=SysTick->CTRL;//读取当前倒计数值
	 }while((temp&0x01)&&(!(temp&(1<<16))));//等待时间到达
	 
	 SysTick->CTRL=0x00; //关闭计数器
	 SysTick->VAL =0X00; //清空计数器
}

/****************************************************************************
* 名    称：delay_ms(u16 nms)
* 功    能：毫秒延时函数
* 入口参数：u16 nms
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void delay_ms(u16 nms)
{
     //注意 delay_ms函数输入范围是1-1863
	 //所以最大延时为1.8秒

	 u32 temp;
	 SysTick->LOAD = 9000*nms;
	 SysTick->VAL=0X00;//清空计数器
	 SysTick->CTRL=0X01;//使能，减到零是无动作，采用外部时钟源
	 do
	 {
	  temp=SysTick->CTRL;//读取当前倒计数值
	 }while((temp&0x01)&&(!(temp&(1<<16))));//等待时间到达
	 SysTick->CTRL=0x00; //关闭计数器
	 SysTick->VAL =0X00; //清空计数器
}


void HT1621_GPIO_Config (void)
{		
		/*定义一个GPIO_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;

		/*开启GPIO的外设时钟*/
		RCC_APB2PeriphClockCmd( HT1621_GPIOA_CLK|HT1621_GPIOB_CLK|HT1621_GPIOC_CLK, ENABLE); 
		
              /*选择要控制的GPIO引脚*/	
	       GPIO_InitStructure.GPIO_Pin=HT1621WR_GPIOB_PIN|HT1621CS_GPIOB_PIN|HT1621DATA_GPIOB_PIN;//TX

		/*设置引脚模式为通用推挽输出*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

		/*设置引脚速率为50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		
		/*调用库函数，初始化GPIOB*/
		GPIO_Init(HT1621_GPIOB_PORT, &GPIO_InitStructure);	


	       GPIO_InitStructure.GPIO_Pin = HT1621_BL_GPIOC_PIN; 

		/*设置引脚模式为通用推挽输出*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

		/*设置引脚速率为50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

      	       GPIO_Init(HT1621_GPIOC_PORT, &GPIO_InitStructure);
			   

	       GPIO_InitStructure.GPIO_Pin = HT1621_KEY1_GPIOC_PIN|HT1621_KEY2_GPIOC_PIN|HT1621_KEY3_GPIOC_PIN; 

	       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 


      	       GPIO_Init(HT1621_GPIOC_PORT, &GPIO_InitStructure);
			   

	       GPIO_InitStructure.GPIO_Pin = HT1621_KEY4_GPIOA_PIN|HT1621_KEY5_GPIOA_PIN; 

	       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 

      	       GPIO_Init(HT1621_GPIOA_PORT, &GPIO_InitStructure);
		   
		/* 开	*/
		GPIO_SetBits(HT1621_GPIOB_PORT, HT1621CS_GPIOB_PIN);
		
		/*开	*/
		GPIO_SetBits(HT1621_GPIOB_PORT, HT1621WR_GPIOB_PIN);	 
    
              /*开	*/
		GPIO_SetBits(HT1621_GPIOB_PORT, HT1621DATA_GPIOB_PIN);	 

	      delay_ms(200); //延时，使LCD工作电压稳定
	      Ht1621WrCmd(BIAS);
	      Ht1621WrCmd(RC256); //使用内部振荡器
	      Ht1621WrCmd(SYSDIS);
	      Ht1621WrCmd(WDTDIS);
	      Ht1621WrCmd(SYSEN);
	      Ht1621WrCmd(LCDON);
	
}


void Ht1621Wr_Data(u8 Data,u8 cnt)
{
  u8 i,Temp=0x80;;
  for (i=0;i<cnt;i++)
  {
     HT1621_WR(OFF);
     if (Data&Temp)
     HT1621_DATA(ON);
     else
     HT1621_DATA(OFF); 
     Temp>>=1;
     delay_us(1);
      HT1621_WR(ON);
	 
   }
}

void Ht1621WrOneData(u8 Addr,u8 Data)
{
  HT1621_CS(OFF);
  Addr<<= 2;
  Data<<= 4; 
  Ht1621Wr_Data(0xa0,3);     // - - 写入数据标志101
  Ht1621Wr_Data(Addr,6);  // - - 写入地址数据
  Ht1621Wr_Data(Data,4);  // - - 写入数据
  HT1621_CS(ON);
  delay_us(1);
}

void Ht1621WrCmd(u8 Cmd)
{
  HT1621_CS(OFF);
  delay_us(1);
  Ht1621Wr_Data(0x80,4);  // - - 写入命令标志100
  Ht1621Wr_Data(Cmd,8);   // - - 写入命令数据
  HT1621_CS(ON);
  delay_us(1);
}

void Ht1621WrAllData(u8 Addr,u8 *p,u8 cnt)
{
  u8 i;
  HT1621_CS(OFF);
  Ht1621Wr_Data(0xa0,3);     // - - 写入数据标志101
  Ht1621Wr_Data(Addr<<2,6);  // - - 写入地址数据
  for (i=0;i<cnt;i++)
  {
    Ht1621Wr_Data(*p,8);     // - - 写入数据 
    p++;
  }
  HT1621_CS(ON);
  delay_us(1);
}


//显示
void Ht1621DisplayState(u8 *Data,u8 *string,u8 Addr,u8 cnt)
{   
    u8 i,ADDH,ADDL,Temp=0x0f;
    for(i=0;i<cnt;i++)
	{
	 if(Addr==0)
	 {
	 ADDL = Addr*2;
     Temp = Ht1621Tab2[*Data];
     Ht1621WrOneData(ADDL,Temp);
     Addr++;
	 Data++;
	 }
	 
       if((Addr != 0)&&(Addr <= 7))
	{
	Temp =Ht1621Tab1[*string]|Ht1621Tab2[*Data];
    ADDL = Addr*2;
    Ht1621WrOneData(ADDL,Temp);
    ADDH = Addr*2-1;
	Ht1621WrOneData(ADDH,Temp>>4);
    Data++; 
	string++;
	Addr ++; 
       }
	     
	if((Addr > 7)&&(Addr !=12))
	{
	Temp =Ht1621Tab1[*string]|Ht1621Tab2[*Data];
    ADDL = Addr*2-1;
    Ht1621WrOneData(ADDL,Temp);
    ADDH = Addr*2;
	Ht1621WrOneData(ADDH,Temp>>4);
    string++;
	Data++; 
	Addr ++; 
       }
	
        if(Addr==12)
	{
     Temp = Ht1621Tab2[*Data];
	 ADDL = Addr*2-1;
     Ht1621WrOneData(ADDL,Temp);
    // Addr--;
	}
   }
}

void Ht1621_cls(void) //清屏
  {
   Ht1621WrAllData(0,Ht1621Tab,16); 
   //delay_ms(50);
   }

void Ht1621_clrbuf(void) 
{
Ht1621Tab2[0]=0x00;    //T5 co2/T4 PM2.5 /T2 光氢/T1 静电
Ht1621Tab2[1]=0x00;    //T9 定时
Ht1621Tab2[2]=0x00;    //T8 手动
Ht1621Tab2[3]=0x00;    //T7 智能
Ht1621Tab2[4]=0x00;    //T6 ug/m3/ppm
Ht1621Tab2[5]=0x00;    //T3 关机
Ht1621Tab2[6]=0x00;    //S2 定时H图标
Ht1621Tab2[7]=0x00;    //S1 定时S1O图标
Ht1621Tab2[8]=0x00;    //T14 清洗故障
Ht1621Tab2[9]=0x00;    //T13 光氢故障
Ht1621Tab2[10]=0x00;   //T12 电机故障
Ht1621Tab2[11]=0x00;   //T11 静电故障
Ht1621Tab2[12]=0x00;   //T10 运行故障/ S5 风速高 /S4 风速中/S3 风速低
                       
Ht1621Tab2[13]=0x00;  //T5 co2 
Ht1621Tab2[14]=0x00;  //T4 PM2.5 
Ht1621Tab2[15]=0x00;  //T2 光氢
Ht1621Tab2[16]=0x00;   //T1 静电

Ht1621Tab2[17]=0x00;  //T11运行故障   01
Ht1621Tab2[18]=0x00;  // S5 风速高 02 /S4 风速中 04 /S3 风速低  08

Ht1621_BUF[0]=0;   //PM2.5 高位
Ht1621_BUF[1]=0;  //PM2.5 
Ht1621_BUF[2]=0;  //PM2.5 
Ht1621_BUF[3]=0;  //PM2.5 低位
Ht1621_BUF[4]=0;  //房间号
Ht1621_BUF[5]=0;  // 定时时间高位
Ht1621_BUF[6]=0;  //定时时间低位
Ht1621_BUF[7]=0;  // co2 低位
Ht1621_BUF[8]=0;  //co2 
Ht1621_BUF[9]=0;  //co2 
Ht1621_BUF[10]=0;  //co2 高位


}


void Ht1621_off_disp(u8 f)  //关闭显示
{
   if(f==0)
   Ht1621Tab2[0]&=~0x01;

  if(f==1)
   Ht1621Tab2[1]&=~0x01;

  if(f==2)
   Ht1621Tab2[2]&=~0x01;

  if(f==3)
   Ht1621Tab2[3]&=~0x01;

  if(f==4)
   Ht1621Tab2[4]&=~0x01;

  if(f==5)
   Ht1621Tab2[5]&=~0x01;

  if(f==6)
   Ht1621Tab2[6]&=~0x01;

  if(f==7)
   Ht1621Tab2[7]&=~0x01;

  if(f==8)
   Ht1621Tab2[8]&=~0x01;

  if(f==9)
   Ht1621Tab2[9]&=~0x01;

  if(f==10)
   Ht1621Tab2[10]&=~0x01;

  if(f==11)
   Ht1621Tab2[11]&=~0x01;

  if(f==12)
   Ht1621Tab2[12]&=~0x01;
}


void Ht1621_on_disp(u8 f)  //开启显示
 {
   if(f==0)
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];

  if(f==1)
   Ht1621Tab2[1]=0x01;

  if(f==2)
   Ht1621Tab2[2]=0x01;

  if(f==3)
   Ht1621Tab2[3]=0x01;

  if(f==4)
   Ht1621Tab2[4]=0x01;

  if(f==5)
   Ht1621Tab2[5]=0x01;

  if(f==6)
   Ht1621Tab2[6]=0x01;

  if(f==7)
   Ht1621Tab2[7]=0x01;

  if(f==8)
   Ht1621Tab2[8]=0x01;

  if(f==9)
   Ht1621Tab2[9]=0x01;

  if(f==10)
   Ht1621Tab2[10]=0x01;

  if(f==11)
   Ht1621Tab2[11]=0x01;

  if(f==12)
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];
}

/*
Ht1621_on_disp(0);   //T5 co2/T4 PM2.5 /T2 光氢/T1 静电
Ht1621_on_disp(1);    //T9 定时
Ht1621_on_disp(2);   //T8 手动
Ht1621_on_disp(3);    //T7 智能
Ht1621_on_disp(4);   //T6 ug/m3/ppm
Ht1621_on_disp(5);    //T3 关机
Ht1621_on_disp(6);   //S2 定时H图标
Ht1621_on_disp(7);   //S1 定时S1O图标
Ht1621_on_disp(8);    //T14 清洗故障
Ht1621_on_disp(9);    //T13 光氢故障
Ht1621_on_disp(10);  //T12 电机故障
Ht1621_on_disp(11);  //T11 静电故障
Ht1621_on_disp(12);   //T10 运行故障/ S5 风速高 /S4 风速中/S3 风速低

*/

void Key_Scan(void)   //按键扫描
{

       if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_9) == KEY_OFF )  //开关机
      {
       delay_ms(10); //---延时10ms
       if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_9) == KEY_OFF )

	  key3 = ~key3;
	   
       while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_9) == KEY_OFF ) ;   
	   time = 0;
      } 



	if((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == KEY_OFF ) &&(key3==ON))//静电
      {
       delay_ms(10);  //---延时10ms
	if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == KEY_OFF )  
      
	  if((key4 ==0))
	  {
	    count2++;
	     
	   if((count3==1)&&(count2>=3))
	   {
	   count2=2;
          count3=1;
	   }	   
	   
	   if(count2==10)
	   {
          count2=0;
          count3=1;
	    }   
          Ht1621_BUF[5]=count3;  // 定时时间高位
           Ht1621_BUF[6]=count2;  //定时时间低位
	  }
         else if((count1==1))
	   {
            return;
	   }	 
	  else
	  {
         key1 = ~key1;
	  }
       while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == KEY_OFF );     
	   time = 0;
     }    


      if((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == KEY_OFF )&&(key3==ON))  // 光氢
      {
      delay_ms(10); //---延时10ms
      if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == KEY_OFF )  
	 	
	 if(key4 ==0)
	  {
	  if(count2 !=0)
	     count2--;	
	 else
           count2 =0;
	 
          Ht1621_BUF[5]=count3;  // 定时时间高位
          Ht1621_BUF[6]=count2;  //定时时间低位
         if((count3==1)&&(count2 ==0))
            	{
            	count3=0;
		count2=10;
            	}
	  }
        else if((count1==1))
	   {
           return;
	   }	 
	  else
	  {
         key2 = ~key2;
	  }
	  
      while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == KEY_OFF ) ;  
	  time = 0;
      }   

	  
       if((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF ) &&(key3==ON)) //风速
      {
       delay_ms(10); //---延时10ms
       if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF )
	   	
	  if(key4 !=0)
	   {
	    if(count1==1) 
	    {
	     count1=0;
	    }
              count++;
	     if(count==3)
	      {
              count=0;
	       }
	    }
	  
       while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF ) ;   
	   time = 0;
      }  

  
}


void PollingKey(void)  //智能/手动/定时
{
if(key3==ON)
{
 static u8 KeyStep;
 
 switch (KeyStep)
 {
  case 0:
  {
   if (!KeyPin)
   //if (KeyPin==KEY_OFF)//有键按下
   {
    //printf("\r\n按下");
    KeyState = KeyDown;

    if (KeyDoubleTimes > 0)
    {
     KeyStep = 4;//双击
    }
    else
    {
     KeyStep = 2;//不是双击
    }
   }
   else if(KeyDoubleTimes > 0)
   {
    KeyDoubleTimes--;
    if ((KeyDoubleTimes == 0)&&(KeyState == KeyShort))
    {
     //printf("\r\n短按处理");
      time = 0;
	 key4 = 1;
	 count1++;	 
	  if(count1==2)
	    {
            count1=0;
	    }
     KeyState = KeyUp;
    }
   }
   
  }
   break;
  case 1://按下
  {
   if (!KeyPin)
  // if (KeyPin==KEY_OFF)
   {
    KeyDownTimes++;
    if (KeyDownTimes < 200)
    {
     KeyState = KeyShort;
    }
    else if(KeyState != KeyLong)
    {
     KeyState = KeyLong;
    // printf("\r\n长按处理");
       time = 0;
	key4 = 0;
	count1=1;
    }
    else if (KeyDownTimes >= 200)//防止加满溢出
    {
     KeyDownTimes = 200;
    }
   }
   else
   {
    KeyStep = 2;
   }
  }
   break;
  case 2://
  {
   if (KeyPin)//弹起
   {
    KeyStep = 3;
   }
   else//按下
   {
    KeyStep = 1;
   }
  }
   break;
  case 3://弹起
  {
   if (KeyPin)//弹起
   {
    if (KeyDownTimes < 200)
    {
     KeyDoubleTimes = 10;
    }
    KeyDownTimes = 0;
    KeyStep = 0;
   }
   else 
   {
    KeyStep = 2;
   }
  }
   break;
  case 4:
  {
   if (KeyPin)//如果弹起的话就执行
   {
    KeyState = KeyUp;
   // printf("\r\n双击处理");
    KeyStep = 0;
   }
  }
   break;
 default:
  KeyStep = 0;
  break;
 }
 delay_us(100);
}
}


void onoff_Scan(void) //开关
{

    if(key3==ON)
    {	
     pin0_Scan();  //PM2.5 CO2
     pin1_Scan();  //ug/m3  ppm
     pin2_Scan();  //静电开关
     pin3_Scan();  //光氢开关
     pin4_Scan();  //风速
     pin5_Scan();  //智能/手动/定时
     pin6_Scan(); //智能/手动/定时
     Ht1621_BL();  //背光
     Ht1621Display();  //PM2.5位置显示   
     }
     else
     {
     	 HT1621_BL(OFF);
        Ht1621_cls();  //清屏
        Ht1621_BUF[5]=count3;  // 定时时间高位
        Ht1621_BUF[6]=count2;  //定时时间低位
        count3=0;
	 count2=0;
     }

}

void pin0_Scan(void)  //PM2.5 CO2
{
Ht1621Tab2[14]=0x02;  //T4 PM2.5  02
Ht1621Tab2[13]=0x01;  //T5 co2    01
Ht1621_on_disp(0); 
}

void pin1_Scan(void)   //ug/m3  ppm
{
Ht1621_on_disp(4);   //T6 ug/m3/ppm
}


void pin2_Scan(void)  //静电开关
{
     if(key1==ON)
      {
       Ht1621Tab2[16]=0x08;   //T1 静电  08
       device_work_data.para_type.high_pressur_state = 1;
      }
     else if(count1==ON)
      {
       Ht1621Tab2[16]=0x08;   //T1 静电  08
              device_work_data.para_type.high_pressur_state = 1;

	}  
    else
    {
        Ht1621Tab2[16]&=~0x08;   //T1 静电  08
        device_work_data.para_type.high_pressur_state = 0;

    }
    Ht1621_on_disp(0); 
}

void pin3_Scan(void)  //光氢开关
{
        if(key2==ON)
        {
        Ht1621Tab2[15]=0x04;  //T2 光氢   04
        }
	else if(count1==1)
	 {
         Ht1621Tab2[15]=0x04;  //T2 光氢   04
	 }
        else
        {
        Ht1621Tab2[15]&=~0x04;  //T2 光氢   04
        }
    Ht1621_on_disp(0); 
}

void pin4_Scan(void) //风速
{
        if(count==0)
        {
        Ht1621Tab2[18]=0x08;  //S3 风速低     08
        }
			
        if(count==1)
        {
        Ht1621Tab2[18]=0x04;  //S4 风速中     04
        }
		
        if(count==2)
        { 
        Ht1621Tab2[18]=0x02;  //S5 风速高     02
        }
		
     Ht1621_on_disp(12); 
}
void pin5_Scan(void) //智能/手动/定时

{
        if(count1==0)
        {
         Ht1621_off_disp(3);   //T7  关智能
         Ht1621_off_disp(1);    //T9 关定时
         Ht1621_on_disp(2);   //T8 开手动
        }
        if(count1==1)
        {
        Ht1621_off_disp(2);    //T8 关手动
        Ht1621_on_disp(3);    //T7  开智能
        Ht1621_off_disp(1);    //T9 关定时
        }
		
        if(key4 ==0)
        { 
        Ht1621_off_disp(2);    //T8 关手动
	  Ht1621_off_disp(3);   //T7  关智能
        Ht1621_on_disp(1);    //T9 开定时
        }

}

void pin6_Scan(void)  //定时图标
{ 
      
	  if((count1==0)&&((count2+count3)!=0))
	 {
          Ht1621_on_disp(5); //T3 关机
          Ht1621_on_disp(6);   //S2 定时H图标
          Ht1621_on_disp(7);   //S1 定时S1O图标
	  } 
         
	 else  if((count1==1)&&((count2+count3)!=0))
	 {
          Ht1621_on_disp(5); //T3 关机
          Ht1621_on_disp(6);   //S2 定时H图标
          Ht1621_on_disp(7);   //S1 定时S1O图标
	  } 
	else  if((key4 ==0)&&((count2+count3)!=0))
	 {
          Ht1621_on_disp(5); //T3 关机
          Ht1621_on_disp(6);   //S2 定时H图标
          Ht1621_on_disp(7);   //S1 定时S1O图标
	  } 	  
	 else
      {
       Ht1621_off_disp(5); //T3 关机
       Ht1621_off_disp(6);   //S2 定时H图标
       Ht1621_off_disp(7);   //S1 定时S1O图标
      }
}

void Ht1621_BL(void) //背光
{
    if ( time>= 1000 )  //1000  1 ms = 1s 时间到 
    {
     HT1621_BL(OFF);
    }  
    else if (time >= 1000)//防止加满溢出
    {
     time = 1000;
    }
    else
    {
     HT1621_BL(ON);
    }
}

void Ht1621Display(void)  //显示
{
     Ht1621_on_disp(6);   //S2 定时H图标
     Ht1621DisplayState(DIS_BUF,Ht1621_BUF,0,12);     //位置显示
}

/*********************************************END OF FILE**********************/

