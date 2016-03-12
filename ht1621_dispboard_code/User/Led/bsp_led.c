
#include "bsp_led.h"   
#include "bsp_TiMbase.h"





//////////////////////


u8 power_key_state = 0x01; //0xff,none change; 0x01,change to power on ;0x00,change to power off



extern DEVICE_WORK_TYPE device_work_changing_data;



///////////////////////






const u8 Ht1621Tab1[10]={0xfa,0x0a,0xbc,0x9e,0x4e,0xd6,0xf6,0x8a,0xfe,0xde}; //0123456789
volatile u8 Ht1621Tab2[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
volatile u8 Ht1621Tab3[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
u8 DIS_BUF[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
u8 Ht1621Tab[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};	
volatile u8 count2=0,count3=0;  //计数
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


void HT1621_GPIO_Config (void)   //端口初始化
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


	       GPIO_InitStructure.GPIO_Pin = HT1621_BL_GPIOC_PIN|HT1621_LED_GPIOC_PIN; 

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



void Ht1621DisplayState(u8 *Data,u8 *string,u8 Addr,u8 cnt)  //面板显示函数
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

void Ht1621_clrbuf(void) //显示初始化
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

Ht1621Tab3[0]=0;   //PM2.5 高位
Ht1621Tab3[1]=0;  //PM2.5 
Ht1621Tab3[2]=0;  //PM2.5 
Ht1621Tab3[3]=0;  //PM2.5 低位
Ht1621Tab3[4]=0;  //房间号
Ht1621Tab3[5]=0;  // 定时时间高位
Ht1621Tab3[6]=0;  //定时时间低位
Ht1621Tab3[7]=0;  // co2 低位
Ht1621Tab3[8]=0;  //co2 
Ht1621Tab3[9]=0;  //co2 
Ht1621Tab3[10]=0;  //co2 高位

}

/*
//关闭对应显示

Ht1621_off_disp(1);    //T9 定时
Ht1621_off_disp(2);   //T8 手动
Ht1621_off_disp(3);    //T7 智能
Ht1621_off_disp(4);   //T6 ug/m3/ppm
Ht1621_off_disp(5);    //T3 关机
Ht1621_off_disp(6);   //S2 定时H图标
Ht1621_off_disp(7);   //S1 定时S1O图标
Ht1621_off_disp(8);    //T14 清洗故障
Ht1621_off_disp(9);    //T13 光氢故障
Ht1621_off_disp(10);  //T12 电机故障
Ht1621_off_disp(11);  //T11 静电故障
Ht1621_off_disp(12);    //T10 运行故障
Ht1621_off_disp(13);    //S3 风速低  
Ht1621_off_disp(14);   //S3 风速中 
Ht1621_off_disp(15);    //S3 风速高
Ht1621_off_disp(16);    //T1 静电 
Ht1621_off_disp(17);   //T2 光氢 
Ht1621_off_disp(18);   //T4 PM2.5 
Ht1621_off_disp(19);   //T5 co2

*/

void Ht1621_off_disp(u8 f)  //关闭对应显示
{

  if(f==1)                              //T9 定时
   Ht1621Tab2[1]&=~0x01;

  if(f==2)                            //T8 手动
   Ht1621Tab2[2]&=~0x01;

  if(f==3)                           //T7 智能
   Ht1621Tab2[3]&=~0x01;

  if(f==4)                         //T6 ug/m3/ppm
   Ht1621Tab2[4]&=~0x01;

  if(f==5)                         //T3 关机
   Ht1621Tab2[5]&=~0x01;

  if(f==6)                         //S2 定时H图标
   Ht1621Tab2[6]&=~0x01;

  if(f==7)                             //S1 定时S1O图标
   Ht1621Tab2[7]&=~0x01;

  if(f==8)                           //T14 清洗故障
   Ht1621Tab2[8]&=~0x01;

  if(f==9)                          //T13 光氢故障
   Ht1621Tab2[9]&=~0x01;

  if(f==10)                        //T12 电机故障
   Ht1621Tab2[10]&=~0x01;
   
  if(f==11)                        //T11 静电故障
   Ht1621Tab2[11]&=~0x01;

  if(f==12)                           //T10 运行故障
   Ht1621Tab2[17]&=~0x01;	
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];
   
   if(f==13)                        //S3 风速低    
   Ht1621Tab2[18]&=~0x08;  
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];

   if(f==14)                        //S3 风速中     
   Ht1621Tab2[18]&=~0x04;  
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];


   if(f==15)                        //S3 风速高    
   Ht1621Tab2[18]&=~0x02;  
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];


   if(f==16)                       //T1 静电  
   Ht1621Tab2[16]&=~0x08;   
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];

   if(f==17)                            //T2 光氢  
   Ht1621Tab2[15]&=~0x04;  
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];

   if(f==18)                            //T4 PM2.5     
   Ht1621Tab2[14]&=~0x02;  
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];


   if(f==19)                            //T5 co2
   Ht1621Tab2[13]&=~0x01;  
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];

}




/*
 //开启对应显示
 
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
Ht1621_on_disp(12);    //T10 运行故障
Ht1621_on_disp(13);    //S3 风速低  
Ht1621_on_disp(14);   //S3 风速中 
Ht1621_on_disp(15);    //S3 风速高
Ht1621_on_disp(16);    //T1 静电 
Ht1621_on_disp(17);   //T2 光氢 
Ht1621_on_disp(18);   //T4 PM2.5 
Ht1621_on_disp(19);   //T5 co2

*/



void Ht1621_on_disp(u8 f)  //开启对应显示
 {

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

  if(f==12)                           //T10 运行故障
   Ht1621Tab2[17]=0x01;	
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];
   
   if(f==13)                        //S3 风速低    
   Ht1621Tab2[18]=0x08;  
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];

   if(f==14)                        //S3 风速中     
   Ht1621Tab2[18]=0x04;  
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];


   if(f==15)                        //S3 风速高    
   Ht1621Tab2[18]=0x02;  
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];


   if(f==16)                       //T1 静电  
   Ht1621Tab2[16]=0x08;   
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];

   if(f==17)                            //T2 光氢  
   Ht1621Tab2[15]=0x04;  
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];

   if(f==18)                            //T4 PM2.5     
   Ht1621Tab2[14]=0x02;  
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];


   if(f==19)                            //T5 co2
   Ht1621Tab2[13]=0x01;  
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];


  
}




void ESD_onoff(u8 mode) //静电开关
{
     if (mode==ON)
      {
          device_work_changing_data.para_type.high_pressur_state = 1;
      }	  
    else
     {
          device_work_changing_data.para_type.high_pressur_state = 0;
      }

}

void PHT_onoff(u8 mode)      //T13 光氢开关
{
     if (mode==ON)
      {
         device_work_changing_data.para_type.pht_work_state = 1;
      } 	 
    else
     {
         device_work_changing_data.para_type.pht_work_state = 0;
      }

}


void workspeed_onoff(u8 mode) //风速开关
{
        if((mode==2)&&(key4 !=0))
        {
        device_work_changing_data.para_type.wind_speed_state = 2;
        }	
	else if(mode==1)
        {
        device_work_changing_data.para_type.wind_speed_state = 1;     //S3 风速低 
        }		
       else if(mode==2)
        {
        device_work_changing_data.para_type.wind_speed_state = 2;      //S3 风速中
        }
       else if(mode==3)
        { 
        device_work_changing_data.para_type.wind_speed_state = 3;      //S3 风速高
        
        }
         else
        {
         device_work_changing_data.para_type.wind_speed_state = 0;      //风速关
        }
		
}


void MainSwitch (u8 mode) //智能/手动/定时

{
	if(mode==2)    //T8 开手动
        {
        device_work_changing_data.para_type.device_mode = 2;   	
        }
        if(mode==1)    //T7  开智能
        {
        device_work_changing_data.para_type.device_mode = 1;   
        }
	
        if(mode==3)        //T9 定时显示
        { 
         Ht1621_off_disp(2);    //T8 关手动
         Ht1621_off_disp(3);   //T7  关智能
         Ht1621_on_disp(1);    //T9 开定时
        }
 
        if( mode ==4)    //WIFI复位
        { 
         reset_wifi();
         //device_work_data.para_type.fault_state |= 0x20;
        }
        else
        {
         //device_work_data.para_type.fault_state &= ~0x20;
	 }

}


void power_onoff(u8 mode) //静电开关
{
     if (mode==ON)
      {
          device_work_changing_data.para_type.device_power_state = 1;
      }	  
    else
     {
          device_work_changing_data.para_type.device_power_state = 0;
		
      }

}




void PollingKey1(void)  //开关机
{

 static u8 KeyStep;

 switch (KeyStep)
 {
  case 0:
  {
   if (!KeyPina9)
   
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
      if(device_work_data.para_type.device_power_state == 0)//开关机
         //power_onoff(ON); 
        device_work_changing_data.para_type.device_power_state = 1;
       else
        device_work_changing_data.para_type.device_power_state = 0;

       if(power_key_state)
        power_key_state = 0x00;
       else
        power_key_state = 0x01;
       
      // power_onoff(OFF); 
     KeyState = KeyUp;
    }
   }
  }
   break;
  case 1://按下
  {

   if (!KeyPina9)
   {
    KeyDownTimes++;
    if (KeyDownTimes < 400)
    {
     KeyState = KeyShort;
    }
    else if(KeyState != KeyLong)
    {
     KeyState = KeyLong;
    // printf("\r\n长按处理");
       time = 0;

        if(device_work_data.para_type.device_power_state == 1)//开机状态
        {//只在开机状态下才执行 WIFI复位
            MainSwitch(4);  //WIFI复位

        }
    }
    else if (KeyDownTimes >= 400)//防止加满溢出
    {
     KeyDownTimes = 400;
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
   if (KeyPina9)//弹起
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
   if (KeyPina9)//弹起
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
   if (KeyPina9)//如果弹起的话就执行
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


#if 1
void Key_Scan(void)   //按键扫描
{
	if((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == KEY_OFF ) &&(device_work_data.para_type.device_power_state==ON))//静电按键
      {
       delay_ms(10);  //---延时10ms
	if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == KEY_OFF )  
      
	  if(key4==0)
	  {
	  	device_work_changing_data.para_type.timing_state = device_work_data.para_type.timing_state;

		device_work_changing_data.para_type.timing_state++;
		count2 = device_work_changing_data.para_type.timing_state;

		  
	   if(count2==10)
	   {
          count2=0;
          count3=1;
	    }   
	   
	   if((count3==1)&&(count2>=3))
	   {
	    count2=2;
           count3=1;
		   device_work_changing_data.para_type.timing_state = 12;
	   }	 	   
          Ht1621Tab3[5]=count3;  // 定时时间高位
           Ht1621Tab3[6]=count2;  //定时时间低位

		   
		  // device_work_data.para_type.timing_state = count3*10+count2;	  //定时时间显示
	  }
	  
	  else
	  {
	   if(device_work_data.para_type.device_mode == 2)//不是智能模式下
	   	{
	    	     
            if(device_work_data.para_type.high_pressur_state == 0)//静电关
               ESD_onoff(ON); //静电开
             else
	       ESD_onoff(OFF); //静电关
	   	}
	  }
	  
       while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == KEY_OFF );     
	   time = 0;
     }    


      if((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == KEY_OFF )&&(device_work_data.para_type.device_power_state==ON))  // 光氢按键
      {
      delay_ms(10); //---延时10ms
      if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == KEY_OFF )  
	 	
	 if(key4 ==0)
	  {
	       if((count3==1)&&(count2 ==0))
            	{
            	count3=0;
		count2=10;
            	}
		   
	     if((count2!=0)&&(count2 > 0))
	  	{
			if(device_work_changing_data.para_type.timing_state)
				device_work_changing_data.para_type.timing_state--;
	        count2 = (device_work_changing_data.para_type.timing_state);	
	  	}
	    else
	 	{
               count2 =0;
			   (device_work_changing_data.para_type.timing_state) = 0;
	 	}   
		
           Ht1621Tab3[5]=count3;  // 定时时间高位
          Ht1621Tab3[6]=count2;  //定时时间低位
          device_work_changing_data.para_type.timing_state = count3*10+count2;	 //定时时间显示
	  }	
	  else
	  {
	    if(device_work_data.para_type.device_mode == 2)   //不是智能模式下
	    	{
                if(device_work_data.para_type.pht_work_state == 0)//T2 光氢 关
                PHT_onoff(ON);//T13 光氢开 
                 else
	        PHT_onoff(OFF);//T13 光氢关
	    	}
	  }
	  
      while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == KEY_OFF ) ;  
	  time = 0;
      }   

	  
       if((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF ) &&(device_work_data.para_type.device_power_state==ON)) //风速按键
      {
       delay_ms(10); //---延时10ms
       if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF )
	   	
	  if(key4!=0)
	   {
             if(device_work_data.para_type.wind_speed_state == 0)//风速关
                 workspeed_onoff(1); //风速低
            else if(device_work_data.para_type.wind_speed_state == 1)//风速低
                 workspeed_onoff(2); //风速中 
           else if (device_work_data.para_type.wind_speed_state == 2)//风速中
	        workspeed_onoff(3); //风速高
           else if (device_work_data.para_type.wind_speed_state == 3)//风速高
               workspeed_onoff(1);//风速低
             else
		workspeed_onoff(0); //风速关
	       
	    }
	  
       while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF ) ;   
	   time = 0;
      }  

  
}

#else
void Key_Scan(void)   //按键扫描
{
	if((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == KEY_OFF ) &&(device_work_data.para_type.device_power_state==ON))//静电按键
      {
       delay_ms(10);  //---延时10ms
	if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == KEY_OFF )  
      
	  if(key4==0)
	  {
	    count2++;
	      
	   if(count2==10)
	   {
          count2=0;
          count3=1;
	    }   
	   
	   if((count3==1)&&(count2>=3))
	   {
	    count2=2;
           count3=1;
	   }	 	   
          Ht1621Tab3[5]=count3;  // 定时时间高位
           Ht1621Tab3[6]=count2;  //定时时间低位

		   
		   device_work_data.para_type.timing_state = count3*10+count2;	  //定时时间显示
	  }
	  
	  else
	  {
	   if(device_work_data.para_type.device_mode == 2)//不是智能模式下
	   	{
	    	     
            if(device_work_data.para_type.high_pressur_state == 0)//静电关
               ESD_onoff(ON); //静电开
             else
	       ESD_onoff(OFF); //静电关
	   	}
	  }
	  
       while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == KEY_OFF );     
	   time = 0;
     }    


      if((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == KEY_OFF )&&(device_work_data.para_type.device_power_state==ON))  // 光氢按键
      {
      delay_ms(10); //---延时10ms
      if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == KEY_OFF )  
	 	
	 if(key4 ==0)
	  {
	       if((count3==1)&&(count2 ==0))
            	{
            	count3=0;
		count2=10;
            	}
		   
	     if((count2!=0)&&(count2 > 0))
	  	{
	        count2--;	
	  	}
	    else
	 	{
               count2 =0;
	 	}   
		
           Ht1621Tab3[5]=count3;  // 定时时间高位
          Ht1621Tab3[6]=count2;  //定时时间低位
          device_work_data.para_type.timing_state = count3*10+count2;	 //定时时间显示
	  }	
	  else
	  {
	    if(device_work_data.para_type.device_mode == 2)   //不是智能模式下
	    	{
                if(device_work_data.para_type.pht_work_state == 0)//T2 光氢 关
                PHT_onoff(ON);//T13 光氢开 
                 else
	        PHT_onoff(OFF);//T13 光氢关
	    	}
	  }
	  
      while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == KEY_OFF ) ;  
	  time = 0;
      }   

	  
       if((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF ) &&(device_work_data.para_type.device_power_state==ON)) //风速按键
      {
       delay_ms(10); //---延时10ms
       if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF )
	   	
	  if(key4!=0)
	   {
             if(device_work_data.para_type.wind_speed_state == 0)//风速关
                 workspeed_onoff(1); //风速低
            else if(device_work_data.para_type.wind_speed_state == 1)//风速低
                 workspeed_onoff(2); //风速中 
           else if (device_work_data.para_type.wind_speed_state == 2)//风速中
	        workspeed_onoff(3); //风速高
           else if (device_work_data.para_type.wind_speed_state == 3)//风速高
               workspeed_onoff(1);//风速低
             else
		workspeed_onoff(0); //风速关
	       
	    }
	  
       while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF ) ;   
	   time = 0;
      }  

  
}
#endif

void PollingKey(void)  //智能/手动/定时
{
if(device_work_data.para_type.device_power_state==ON)//电源开
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
	      if( key4 == 0)
	          key4 = 1;
            else if(device_work_data.para_type.device_mode == 1)//智能
                 MainSwitch(2); // 手动
           else if(device_work_data.para_type.device_mode == 2)//手动
                 MainSwitch(1); //智能
	     else      
		   MainSwitch(2); // 手动
         KeyState = KeyUp;
    }
   }
  }
   break;
  case 1://按下
  {

   if (!KeyPin)
   {
    KeyDownTimes++;
    if (KeyDownTimes < 400)
    {
     KeyState = KeyShort;
    }
    else if(KeyState != KeyLong)
    {
     KeyState = KeyLong;
    // printf("\r\n长按处理");
       time = 0;
	key4 = 0;
	    Ht1621_off_disp(2);    //关手动显示
	   Ht1621_off_disp(3);   // 关智能显示
	   Ht1621_on_disp(1);    //定时显示
    }
    else if (KeyDownTimes >= 400)//防止加满溢出
    {
     KeyDownTimes = 400;
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


void onoff_Scan(void) //按键开关
{
    if(device_work_data.para_type.device_power_state == 1)
    {	
     //pin0_Scan();  //PM2.5 CO2
     //pin1_Scan();  //ug/m3  ppm
     //pin2_Scan();  //静电开关
    // pin3_Scan();  //光氢开关
    // pin4_Scan();  //风速
    // pin5_Scan();  //智能/手动/定时
     pin6_Scan(); //定时图标
     Ht1621_BL();  //背光
     HT1621_LED(ON);
     Ht1621Display();  //PM2.5位置显示  
      device_work_changing_data.para_type.device_power_state = 1;
     }
     else
     {
     
     	HT1621_BL(OFF);     //显示背光
	HT1621_LED(OFF);   //按键背光
        Ht1621_cls();         //清屏
       
      // device_work_data.para_type.wind_speed_state = 0;    // 风速
       //device_work_data.para_type.high_pressur_state = 0;    //T1 静电 
       //device_work_data.para_type.pht_work_state = 0;   //T2 光氢 
	
       //Ht1621Tab3[5]=count3;  // 定时时间高位
       // Ht1621Tab3[6]=count2;  //定时时间低位

	//device_work_data.para_type.timing_state = count3*10+count2;   //T2 光氢 
	
       //count3=0;
	//count2=0;
       
	 device_work_changing_data.para_type.device_power_state = 0;
	 
     }
   
}


//mode ; 0,off; 1,on
void onoff_device_set(u8 mode) //主板命令开关
{

    if(mode==ON)
    {	
      time = 0;
     pin0_Scan();  //PM2.5 CO2
     pin1_Scan();  //ug/m3  ppm
     //pin2_Scan();  //静电开关
    // pin3_Scan();  //光氢开关
     //pin4_Scan();  //风速
    // pin5_Scan();  //智能/手动/定时
     pin6_Scan(); //智能/手动/定时
     Ht1621_BL();  //背光
     Ht1621Display();  //PM2.5位置显示  
      device_work_data.para_type.device_power_state = 1;
     }
     else
     {
     	 HT1621_BL(OFF);      //显示背光
	 HT1621_LED(OFF);   //按键背光
        Ht1621_cls();             //清屏
       
       device_work_data.para_type.wind_speed_state = 0;    // 风速
       device_work_data.para_type.high_pressur_state = 0;    //T1 静电 
       device_work_data.para_type.pht_work_state = 0;   //T2 光氢 
	
        Ht1621Tab3[5]=count3;  // 定时时间高位
        Ht1621Tab3[6]=count2;  //定时时间低位

	device_work_data.para_type.timing_state = count3*10+count2;   
	
        count3=0;
	 count2=0;

	 device_work_data.para_type.device_power_state = 0;
     }

}


void pin0_Scan(void)  //PM2.5 CO2
{

Ht1621_on_disp(18);   //T4 PM2.5 
Ht1621_on_disp(19);   //T5 co2

}

void pin1_Scan(void)   //ug/m3  ppm
{
Ht1621_on_disp(4);   //T6 ug/m3/ppm
}

void pin6_Scan(void)  //定时图标
{ 
      if(device_work_data.para_type.timing_state!=0)
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
    if ( time>= 3000 )  //1000  1 ms = 1s 时间到 
    {
     HT1621_BL(OFF);
     HT1621_LED(OFF);
    }  
    else
    {
     HT1621_BL(ON);
     HT1621_LED(ON);
    }
	
     if (time >= 3000)//防止加满溢出
    {
     time = 3000;
    }
}

void Ht1621Display(void)  //显示
{
     Ht1621_on_disp(6);   //S2 定时H图标
     Ht1621DisplayState(DIS_BUF,Ht1621Tab3,0,12);     //显示
}



void device_work_mode_check(void)  //收到主板按键显示命令
{

	u8 tmp,tmp1,tmp2,tmp3,tmp4;
	
        tmp =device_work_data.para_type.device_mode;          //模式
        tmp1 =device_work_data.para_type.wind_speed_state;  //风速
        tmp2 =device_work_data.para_type.high_pressur_state;  //静电 
	tmp3 =device_work_data.para_type.pht_work_state;        //光氢
	
       switch(tmp)
    	{
    	case 0x00:
	break;

	case 0x01:
	if( key4 != 0) 
	{
        Ht1621_off_disp(2);    //T8 关手动
        Ht1621_on_disp(3);    //T7  开智能
        Ht1621_off_disp(1);    //T9 关定时
        device_work_data.para_type.device_mode = 1;
	}
	break;
	case 0x02:
        if( key4 != 0) 
        {
	 Ht1621_off_disp(3);   //T7  关智能
        Ht1621_off_disp(1);    //T9 关定时
        Ht1621_on_disp(2);   //T8 开手动
        device_work_data.para_type.device_mode = 2;	
        }
	break;
	default:  
	break;
    	}

     
      switch(tmp1)
      {
    	case 0x00:
        Ht1621_off_disp(13);    //S3 风速低
        Ht1621_off_disp(14);   //S3 风速中
        Ht1621_off_disp(15);    //S3 风速高
        device_work_data.para_type.wind_speed_state = 0;				
	break;
	case 0x01:
        Ht1621_on_disp(13);    //S3 风速低  
        device_work_data.para_type.wind_speed_state = 1;
	break;
	case 0x02:
        Ht1621_on_disp(14);   //S3 风速中
        device_work_data.para_type.wind_speed_state = 2;
	break;
	case 0x03:
        Ht1621_on_disp(15);    //S3 风速高
       device_work_data.para_type.wind_speed_state = 3;
	break;	
	default:  	
	break;

      	}

      switch(tmp2)
      {
    	case 0x00:
       Ht1621_off_disp(16);    //T1 静电关
        device_work_data.para_type.high_pressur_state= 0;			
	break;
	case 0x01:
         Ht1621_on_disp(16);    //T1 静电 开
        device_work_data.para_type.high_pressur_state= 1;	
	break;
	default:  
	break;
      	}

      switch(tmp3)
      {
    	case 0x00:
         Ht1621_off_disp(17);   //T2 光氢关 
       device_work_data.para_type.pht_work_state = 0;			
	break;
	case 0x01:
         Ht1621_on_disp(17);   //T2 光氢开 
        device_work_data.para_type.pht_work_state = 0;	
	break;
	default:  
	break;
      	}
 
}







/*********************************************END OF FILE**********************/

