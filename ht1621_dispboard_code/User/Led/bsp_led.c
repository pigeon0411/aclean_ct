
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
volatile u8 count2=0,count3=0;  //����
volatile u8  key4 = 1; //----��һ��������־//
volatile u32  time = 0; // ms ��ʱ���� 
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
* ��    �ƣ�delay_us(u32 nus)
* ��    �ܣ�΢����ʱ����
* ��ڲ�����u32  nus
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/ 
void delay_us(u32 nus)
{
	 u32 temp;
	 SysTick->LOAD = 9*nus;
	 SysTick->VAL=0X00;//��ռ�����
	 SysTick->CTRL=0X01;//ʹ�ܣ����������޶����������ⲿʱ��Դ
	 do
	 {
	  temp=SysTick->CTRL;//��ȡ��ǰ������ֵ
	 }while((temp&0x01)&&(!(temp&(1<<16))));//�ȴ�ʱ�䵽��
	 
	 SysTick->CTRL=0x00; //�رռ�����
	 SysTick->VAL =0X00; //��ռ�����
}

/****************************************************************************
* ��    �ƣ�delay_ms(u16 nms)
* ��    �ܣ�������ʱ����
* ��ڲ�����u16 nms
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/ 
void delay_ms(u16 nms)
{
     //ע�� delay_ms�������뷶Χ��1-1863
	 //���������ʱΪ1.8��

	 u32 temp;
	 SysTick->LOAD = 9000*nms;
	 SysTick->VAL=0X00;//��ռ�����
	 SysTick->CTRL=0X01;//ʹ�ܣ����������޶����������ⲿʱ��Դ
	 do
	 {
	  temp=SysTick->CTRL;//��ȡ��ǰ������ֵ
	 }while((temp&0x01)&&(!(temp&(1<<16))));//�ȴ�ʱ�䵽��
	 SysTick->CTRL=0x00; //�رռ�����
	 SysTick->VAL =0X00; //��ռ�����
}


void HT1621_GPIO_Config (void)   //�˿ڳ�ʼ��
{		
		/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
		GPIO_InitTypeDef GPIO_InitStructure;

		/*����GPIO������ʱ��*/
		RCC_APB2PeriphClockCmd( HT1621_GPIOA_CLK|HT1621_GPIOB_CLK|HT1621_GPIOC_CLK, ENABLE); 
		
              /*ѡ��Ҫ���Ƶ�GPIO����*/	
	       GPIO_InitStructure.GPIO_Pin=HT1621WR_GPIOB_PIN|HT1621CS_GPIOB_PIN|HT1621DATA_GPIOB_PIN;//TX

		/*��������ģʽΪͨ���������*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

		/*������������Ϊ50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		
		/*���ÿ⺯������ʼ��GPIOB*/
		GPIO_Init(HT1621_GPIOB_PORT, &GPIO_InitStructure);	


	       GPIO_InitStructure.GPIO_Pin = HT1621_BL_GPIOC_PIN|HT1621_LED_GPIOC_PIN; 

		/*��������ģʽΪͨ���������*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

		/*������������Ϊ50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

      	       GPIO_Init(HT1621_GPIOC_PORT, &GPIO_InitStructure);
			   

	       GPIO_InitStructure.GPIO_Pin = HT1621_KEY1_GPIOC_PIN|HT1621_KEY2_GPIOC_PIN|HT1621_KEY3_GPIOC_PIN; 

	       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 


      	       GPIO_Init(HT1621_GPIOC_PORT, &GPIO_InitStructure);
			   

	       GPIO_InitStructure.GPIO_Pin = HT1621_KEY4_GPIOA_PIN|HT1621_KEY5_GPIOA_PIN; 

	       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 

      	       GPIO_Init(HT1621_GPIOA_PORT, &GPIO_InitStructure);
		   
		/* ��	*/
		GPIO_SetBits(HT1621_GPIOB_PORT, HT1621CS_GPIOB_PIN);
		
		/*��	*/
		GPIO_SetBits(HT1621_GPIOB_PORT, HT1621WR_GPIOB_PIN);	 
    
              /*��	*/
		GPIO_SetBits(HT1621_GPIOB_PORT, HT1621DATA_GPIOB_PIN);	 

	      delay_ms(200); //��ʱ��ʹLCD������ѹ�ȶ�
	      Ht1621WrCmd(BIAS);
	      Ht1621WrCmd(RC256); //ʹ���ڲ�����
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
  Ht1621Wr_Data(0xa0,3);     // - - д�����ݱ�־101
  Ht1621Wr_Data(Addr,6);  // - - д���ַ����
  Ht1621Wr_Data(Data,4);  // - - д������
  HT1621_CS(ON);
  delay_us(1);
}

void Ht1621WrCmd(u8 Cmd)
{
  HT1621_CS(OFF);
  delay_us(1);
  Ht1621Wr_Data(0x80,4);  // - - д�������־100
  Ht1621Wr_Data(Cmd,8);   // - - д����������
  HT1621_CS(ON);
  delay_us(1);
}

void Ht1621WrAllData(u8 Addr,u8 *p,u8 cnt)
{
  u8 i;
  HT1621_CS(OFF);
  Ht1621Wr_Data(0xa0,3);     // - - д�����ݱ�־101
  Ht1621Wr_Data(Addr<<2,6);  // - - д���ַ����
  for (i=0;i<cnt;i++)
  {
    Ht1621Wr_Data(*p,8);     // - - д������ 
    p++;
  }
  HT1621_CS(ON);
  delay_us(1);
}



void Ht1621DisplayState(u8 *Data,u8 *string,u8 Addr,u8 cnt)  //�����ʾ����
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

void Ht1621_cls(void) //����
  {
   Ht1621WrAllData(0,Ht1621Tab,16); 
   //delay_ms(50);
   }

void Ht1621_clrbuf(void) //��ʾ��ʼ��
{

Ht1621Tab2[0]=0x00;    //T5 co2/T4 PM2.5 /T2 ����/T1 ����
Ht1621Tab2[1]=0x00;    //T9 ��ʱ
Ht1621Tab2[2]=0x00;    //T8 �ֶ�
Ht1621Tab2[3]=0x00;    //T7 ����
Ht1621Tab2[4]=0x00;    //T6 ug/m3/ppm
Ht1621Tab2[5]=0x00;    //T3 �ػ�
Ht1621Tab2[6]=0x00;    //S2 ��ʱHͼ��
Ht1621Tab2[7]=0x00;    //S1 ��ʱS1Oͼ��
Ht1621Tab2[8]=0x00;    //T14 ��ϴ����
Ht1621Tab2[9]=0x00;    //T13 �������
Ht1621Tab2[10]=0x00;   //T12 �������
Ht1621Tab2[11]=0x00;   //T11 �������
Ht1621Tab2[12]=0x00;   //T10 ���й���/ S5 ���ٸ� /S4 ������/S3 ���ٵ�
                       
Ht1621Tab2[13]=0x00;  //T5 co2 
Ht1621Tab2[14]=0x00;  //T4 PM2.5 
Ht1621Tab2[15]=0x00;  //T2 ����
Ht1621Tab2[16]=0x00;   //T1 ����

Ht1621Tab2[17]=0x00;  //T11���й���   01
Ht1621Tab2[18]=0x00;  // S5 ���ٸ� 02 /S4 ������ 04 /S3 ���ٵ�  08

Ht1621Tab3[0]=0;   //PM2.5 ��λ
Ht1621Tab3[1]=0;  //PM2.5 
Ht1621Tab3[2]=0;  //PM2.5 
Ht1621Tab3[3]=0;  //PM2.5 ��λ
Ht1621Tab3[4]=0;  //�����
Ht1621Tab3[5]=0;  // ��ʱʱ���λ
Ht1621Tab3[6]=0;  //��ʱʱ���λ
Ht1621Tab3[7]=0;  // co2 ��λ
Ht1621Tab3[8]=0;  //co2 
Ht1621Tab3[9]=0;  //co2 
Ht1621Tab3[10]=0;  //co2 ��λ

}

/*
//�رն�Ӧ��ʾ

Ht1621_off_disp(1);    //T9 ��ʱ
Ht1621_off_disp(2);   //T8 �ֶ�
Ht1621_off_disp(3);    //T7 ����
Ht1621_off_disp(4);   //T6 ug/m3/ppm
Ht1621_off_disp(5);    //T3 �ػ�
Ht1621_off_disp(6);   //S2 ��ʱHͼ��
Ht1621_off_disp(7);   //S1 ��ʱS1Oͼ��
Ht1621_off_disp(8);    //T14 ��ϴ����
Ht1621_off_disp(9);    //T13 �������
Ht1621_off_disp(10);  //T12 �������
Ht1621_off_disp(11);  //T11 �������
Ht1621_off_disp(12);    //T10 ���й���
Ht1621_off_disp(13);    //S3 ���ٵ�  
Ht1621_off_disp(14);   //S3 ������ 
Ht1621_off_disp(15);    //S3 ���ٸ�
Ht1621_off_disp(16);    //T1 ���� 
Ht1621_off_disp(17);   //T2 ���� 
Ht1621_off_disp(18);   //T4 PM2.5 
Ht1621_off_disp(19);   //T5 co2

*/

void Ht1621_off_disp(u8 f)  //�رն�Ӧ��ʾ
{

  if(f==1)                              //T9 ��ʱ
   Ht1621Tab2[1]&=~0x01;

  if(f==2)                            //T8 �ֶ�
   Ht1621Tab2[2]&=~0x01;

  if(f==3)                           //T7 ����
   Ht1621Tab2[3]&=~0x01;

  if(f==4)                         //T6 ug/m3/ppm
   Ht1621Tab2[4]&=~0x01;

  if(f==5)                         //T3 �ػ�
   Ht1621Tab2[5]&=~0x01;

  if(f==6)                         //S2 ��ʱHͼ��
   Ht1621Tab2[6]&=~0x01;

  if(f==7)                             //S1 ��ʱS1Oͼ��
   Ht1621Tab2[7]&=~0x01;

  if(f==8)                           //T14 ��ϴ����
   Ht1621Tab2[8]&=~0x01;

  if(f==9)                          //T13 �������
   Ht1621Tab2[9]&=~0x01;

  if(f==10)                        //T12 �������
   Ht1621Tab2[10]&=~0x01;
   
  if(f==11)                        //T11 �������
   Ht1621Tab2[11]&=~0x01;

  if(f==12)                           //T10 ���й���
   Ht1621Tab2[17]&=~0x01;	
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];
   
   if(f==13)                        //S3 ���ٵ�    
   Ht1621Tab2[18]&=~0x08;  
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];

   if(f==14)                        //S3 ������     
   Ht1621Tab2[18]&=~0x04;  
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];


   if(f==15)                        //S3 ���ٸ�    
   Ht1621Tab2[18]&=~0x02;  
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];


   if(f==16)                       //T1 ����  
   Ht1621Tab2[16]&=~0x08;   
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];

   if(f==17)                            //T2 ����  
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
 //������Ӧ��ʾ
 
Ht1621_on_disp(1);    //T9 ��ʱ
Ht1621_on_disp(2);   //T8 �ֶ�
Ht1621_on_disp(3);    //T7 ����
Ht1621_on_disp(4);   //T6 ug/m3/ppm
Ht1621_on_disp(5);    //T3 �ػ�
Ht1621_on_disp(6);   //S2 ��ʱHͼ��
Ht1621_on_disp(7);   //S1 ��ʱS1Oͼ��
Ht1621_on_disp(8);    //T14 ��ϴ����
Ht1621_on_disp(9);    //T13 �������
Ht1621_on_disp(10);  //T12 �������
Ht1621_on_disp(11);  //T11 �������
Ht1621_on_disp(12);    //T10 ���й���
Ht1621_on_disp(13);    //S3 ���ٵ�  
Ht1621_on_disp(14);   //S3 ������ 
Ht1621_on_disp(15);    //S3 ���ٸ�
Ht1621_on_disp(16);    //T1 ���� 
Ht1621_on_disp(17);   //T2 ���� 
Ht1621_on_disp(18);   //T4 PM2.5 
Ht1621_on_disp(19);   //T5 co2

*/



void Ht1621_on_disp(u8 f)  //������Ӧ��ʾ
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

  if(f==12)                           //T10 ���й���
   Ht1621Tab2[17]=0x01;	
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];
   
   if(f==13)                        //S3 ���ٵ�    
   Ht1621Tab2[18]=0x08;  
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];

   if(f==14)                        //S3 ������     
   Ht1621Tab2[18]=0x04;  
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];


   if(f==15)                        //S3 ���ٸ�    
   Ht1621Tab2[18]=0x02;  
   Ht1621Tab2[12]=Ht1621Tab2[18]|Ht1621Tab2[17];


   if(f==16)                       //T1 ����  
   Ht1621Tab2[16]=0x08;   
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];

   if(f==17)                            //T2 ����  
   Ht1621Tab2[15]=0x04;  
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];

   if(f==18)                            //T4 PM2.5     
   Ht1621Tab2[14]=0x02;  
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];


   if(f==19)                            //T5 co2
   Ht1621Tab2[13]=0x01;  
   Ht1621Tab2[0]=Ht1621Tab2[13]|Ht1621Tab2[14]|Ht1621Tab2[15]|Ht1621Tab2[16];


  
}




void ESD_onoff(u8 mode) //���翪��
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

void PHT_onoff(u8 mode)      //T13 ���⿪��
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


void workspeed_onoff(u8 mode) //���ٿ���
{
        if((mode==2)&&(key4 !=0))
        {
        device_work_changing_data.para_type.wind_speed_state = 2;
        }	
	else if(mode==1)
        {
        device_work_changing_data.para_type.wind_speed_state = 1;     //S3 ���ٵ� 
        }		
       else if(mode==2)
        {
        device_work_changing_data.para_type.wind_speed_state = 2;      //S3 ������
        }
       else if(mode==3)
        { 
        device_work_changing_data.para_type.wind_speed_state = 3;      //S3 ���ٸ�
        
        }
         else
        {
         device_work_changing_data.para_type.wind_speed_state = 0;      //���ٹ�
        }
		
}


void MainSwitch (u8 mode) //����/�ֶ�/��ʱ

{
	if(mode==2)    //T8 ���ֶ�
        {
        device_work_changing_data.para_type.device_mode = 2;   	
        }
        if(mode==1)    //T7  ������
        {
        device_work_changing_data.para_type.device_mode = 1;   
        }
	
        if(mode==3)        //T9 ��ʱ��ʾ
        { 
         Ht1621_off_disp(2);    //T8 ���ֶ�
         Ht1621_off_disp(3);   //T7  ������
         Ht1621_on_disp(1);    //T9 ����ʱ
        }
 
        if( mode ==4)    //WIFI��λ
        { 
         reset_wifi();
         //device_work_data.para_type.fault_state |= 0x20;
        }
        else
        {
         //device_work_data.para_type.fault_state &= ~0x20;
	 }

}


void power_onoff(u8 mode) //���翪��
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




void PollingKey1(void)  //���ػ�
{

 static u8 KeyStep;

 switch (KeyStep)
 {
  case 0:
  {
   if (!KeyPina9)
   
   //if (KeyPin==KEY_OFF)//�м�����
   {
    //printf("\r\n����");
    KeyState = KeyDown;

    if (KeyDoubleTimes > 0)
    {
     KeyStep = 4;//˫��
    }
    else
    {
     KeyStep = 2;//����˫��
    }
   }
   else if(KeyDoubleTimes > 0)
   {
    KeyDoubleTimes--;

    if ((KeyDoubleTimes == 0)&&(KeyState == KeyShort))
    {
    
     //printf("\r\n�̰�����");
      time = 0;
      if(device_work_data.para_type.device_power_state == 0)//���ػ�
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
  case 1://����
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
    // printf("\r\n��������");
       time = 0;

        if(device_work_data.para_type.device_power_state == 1)//����״̬
        {//ֻ�ڿ���״̬�²�ִ�� WIFI��λ
            MainSwitch(4);  //WIFI��λ

        }
    }
    else if (KeyDownTimes >= 400)//��ֹ�������
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
   if (KeyPina9)//����
   {
    KeyStep = 3;
   }
   else//����
   {
    KeyStep = 1;
   }
  }
   break;
  case 3://����
  {
   if (KeyPina9)//����
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
   if (KeyPina9)//�������Ļ���ִ��
   {
    KeyState = KeyUp;
   // printf("\r\n˫������");
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
void Key_Scan(void)   //����ɨ��
{
	if((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == KEY_OFF ) &&(device_work_data.para_type.device_power_state==ON))//���簴��
      {
       delay_ms(10);  //---��ʱ10ms
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
          Ht1621Tab3[5]=count3;  // ��ʱʱ���λ
           Ht1621Tab3[6]=count2;  //��ʱʱ���λ

		   
		  // device_work_data.para_type.timing_state = count3*10+count2;	  //��ʱʱ����ʾ
	  }
	  
	  else
	  {
	   if(device_work_data.para_type.device_mode == 2)//��������ģʽ��
	   	{
	    	     
            if(device_work_data.para_type.high_pressur_state == 0)//�����
               ESD_onoff(ON); //���翪
             else
	       ESD_onoff(OFF); //�����
	   	}
	  }
	  
       while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == KEY_OFF );     
	   time = 0;
     }    


      if((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == KEY_OFF )&&(device_work_data.para_type.device_power_state==ON))  // ���ⰴ��
      {
      delay_ms(10); //---��ʱ10ms
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
		
           Ht1621Tab3[5]=count3;  // ��ʱʱ���λ
          Ht1621Tab3[6]=count2;  //��ʱʱ���λ
          device_work_changing_data.para_type.timing_state = count3*10+count2;	 //��ʱʱ����ʾ
	  }	
	  else
	  {
	    if(device_work_data.para_type.device_mode == 2)   //��������ģʽ��
	    	{
                if(device_work_data.para_type.pht_work_state == 0)//T2 ���� ��
                PHT_onoff(ON);//T13 ���⿪ 
                 else
	        PHT_onoff(OFF);//T13 �����
	    	}
	  }
	  
      while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == KEY_OFF ) ;  
	  time = 0;
      }   

	  
       if((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF ) &&(device_work_data.para_type.device_power_state==ON)) //���ٰ���
      {
       delay_ms(10); //---��ʱ10ms
       if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF )
	   	
	  if(key4!=0)
	   {
             if(device_work_data.para_type.wind_speed_state == 0)//���ٹ�
                 workspeed_onoff(1); //���ٵ�
            else if(device_work_data.para_type.wind_speed_state == 1)//���ٵ�
                 workspeed_onoff(2); //������ 
           else if (device_work_data.para_type.wind_speed_state == 2)//������
	        workspeed_onoff(3); //���ٸ�
           else if (device_work_data.para_type.wind_speed_state == 3)//���ٸ�
               workspeed_onoff(1);//���ٵ�
             else
		workspeed_onoff(0); //���ٹ�
	       
	    }
	  
       while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF ) ;   
	   time = 0;
      }  

  
}

#else
void Key_Scan(void)   //����ɨ��
{
	if((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == KEY_OFF ) &&(device_work_data.para_type.device_power_state==ON))//���簴��
      {
       delay_ms(10);  //---��ʱ10ms
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
          Ht1621Tab3[5]=count3;  // ��ʱʱ���λ
           Ht1621Tab3[6]=count2;  //��ʱʱ���λ

		   
		   device_work_data.para_type.timing_state = count3*10+count2;	  //��ʱʱ����ʾ
	  }
	  
	  else
	  {
	   if(device_work_data.para_type.device_mode == 2)//��������ģʽ��
	   	{
	    	     
            if(device_work_data.para_type.high_pressur_state == 0)//�����
               ESD_onoff(ON); //���翪
             else
	       ESD_onoff(OFF); //�����
	   	}
	  }
	  
       while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == KEY_OFF );     
	   time = 0;
     }    


      if((GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == KEY_OFF )&&(device_work_data.para_type.device_power_state==ON))  // ���ⰴ��
      {
      delay_ms(10); //---��ʱ10ms
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
		
           Ht1621Tab3[5]=count3;  // ��ʱʱ���λ
          Ht1621Tab3[6]=count2;  //��ʱʱ���λ
          device_work_data.para_type.timing_state = count3*10+count2;	 //��ʱʱ����ʾ
	  }	
	  else
	  {
	    if(device_work_data.para_type.device_mode == 2)   //��������ģʽ��
	    	{
                if(device_work_data.para_type.pht_work_state == 0)//T2 ���� ��
                PHT_onoff(ON);//T13 ���⿪ 
                 else
	        PHT_onoff(OFF);//T13 �����
	    	}
	  }
	  
      while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == KEY_OFF ) ;  
	  time = 0;
      }   

	  
       if((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF ) &&(device_work_data.para_type.device_power_state==ON)) //���ٰ���
      {
       delay_ms(10); //---��ʱ10ms
       if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF )
	   	
	  if(key4!=0)
	   {
             if(device_work_data.para_type.wind_speed_state == 0)//���ٹ�
                 workspeed_onoff(1); //���ٵ�
            else if(device_work_data.para_type.wind_speed_state == 1)//���ٵ�
                 workspeed_onoff(2); //������ 
           else if (device_work_data.para_type.wind_speed_state == 2)//������
	        workspeed_onoff(3); //���ٸ�
           else if (device_work_data.para_type.wind_speed_state == 3)//���ٸ�
               workspeed_onoff(1);//���ٵ�
             else
		workspeed_onoff(0); //���ٹ�
	       
	    }
	  
       while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) == KEY_OFF ) ;   
	   time = 0;
      }  

  
}
#endif

void PollingKey(void)  //����/�ֶ�/��ʱ
{
if(device_work_data.para_type.device_power_state==ON)//��Դ��
{
 static u8 KeyStep;

 switch (KeyStep)
 {
  case 0:
  {
   if (!KeyPin)
   //if (KeyPin==KEY_OFF)//�м�����
   {
    //printf("\r\n����");
    KeyState = KeyDown;

    if (KeyDoubleTimes > 0)
    {
     KeyStep = 4;//˫��
    }
    else
    {
     KeyStep = 2;//����˫��
    }
   }
   else if(KeyDoubleTimes > 0)
   {
    KeyDoubleTimes--;

    if ((KeyDoubleTimes == 0)&&(KeyState == KeyShort))
    {
    
     //printf("\r\n�̰�����");
      time = 0;
	      if( key4 == 0)
	          key4 = 1;
            else if(device_work_data.para_type.device_mode == 1)//����
                 MainSwitch(2); // �ֶ�
           else if(device_work_data.para_type.device_mode == 2)//�ֶ�
                 MainSwitch(1); //����
	     else      
		   MainSwitch(2); // �ֶ�
         KeyState = KeyUp;
    }
   }
  }
   break;
  case 1://����
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
    // printf("\r\n��������");
       time = 0;
	key4 = 0;
	    Ht1621_off_disp(2);    //���ֶ���ʾ
	   Ht1621_off_disp(3);   // ��������ʾ
	   Ht1621_on_disp(1);    //��ʱ��ʾ
    }
    else if (KeyDownTimes >= 400)//��ֹ�������
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
   if (KeyPin)//����
   {
    KeyStep = 3;
   }
   else//����
   {
    KeyStep = 1;
   }
  }
   break;
  case 3://����
  {
   if (KeyPin)//����
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
   if (KeyPin)//�������Ļ���ִ��
   {
    KeyState = KeyUp;
   // printf("\r\n˫������");
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


void onoff_Scan(void) //��������
{
    if(device_work_data.para_type.device_power_state == 1)
    {	
     //pin0_Scan();  //PM2.5 CO2
     //pin1_Scan();  //ug/m3  ppm
     //pin2_Scan();  //���翪��
    // pin3_Scan();  //���⿪��
    // pin4_Scan();  //����
    // pin5_Scan();  //����/�ֶ�/��ʱ
     pin6_Scan(); //��ʱͼ��
     Ht1621_BL();  //����
     HT1621_LED(ON);
     Ht1621Display();  //PM2.5λ����ʾ  
      device_work_changing_data.para_type.device_power_state = 1;
     }
     else
     {
     
     	HT1621_BL(OFF);     //��ʾ����
	HT1621_LED(OFF);   //��������
        Ht1621_cls();         //����
       
      // device_work_data.para_type.wind_speed_state = 0;    // ����
       //device_work_data.para_type.high_pressur_state = 0;    //T1 ���� 
       //device_work_data.para_type.pht_work_state = 0;   //T2 ���� 
	
       //Ht1621Tab3[5]=count3;  // ��ʱʱ���λ
       // Ht1621Tab3[6]=count2;  //��ʱʱ���λ

	//device_work_data.para_type.timing_state = count3*10+count2;   //T2 ���� 
	
       //count3=0;
	//count2=0;
       
	 device_work_changing_data.para_type.device_power_state = 0;
	 
     }
   
}


//mode ; 0,off; 1,on
void onoff_device_set(u8 mode) //���������
{

    if(mode==ON)
    {	
      time = 0;
     pin0_Scan();  //PM2.5 CO2
     pin1_Scan();  //ug/m3  ppm
     //pin2_Scan();  //���翪��
    // pin3_Scan();  //���⿪��
     //pin4_Scan();  //����
    // pin5_Scan();  //����/�ֶ�/��ʱ
     pin6_Scan(); //����/�ֶ�/��ʱ
     Ht1621_BL();  //����
     Ht1621Display();  //PM2.5λ����ʾ  
      device_work_data.para_type.device_power_state = 1;
     }
     else
     {
     	 HT1621_BL(OFF);      //��ʾ����
	 HT1621_LED(OFF);   //��������
        Ht1621_cls();             //����
       
       device_work_data.para_type.wind_speed_state = 0;    // ����
       device_work_data.para_type.high_pressur_state = 0;    //T1 ���� 
       device_work_data.para_type.pht_work_state = 0;   //T2 ���� 
	
        Ht1621Tab3[5]=count3;  // ��ʱʱ���λ
        Ht1621Tab3[6]=count2;  //��ʱʱ���λ

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

void pin6_Scan(void)  //��ʱͼ��
{ 
      if(device_work_data.para_type.timing_state!=0)
	 {
          Ht1621_on_disp(5); //T3 �ػ�
          Ht1621_on_disp(6);   //S2 ��ʱHͼ��
          Ht1621_on_disp(7);   //S1 ��ʱS1Oͼ��
	  } 	  
	 else
      {
       Ht1621_off_disp(5); //T3 �ػ�
       Ht1621_off_disp(6);   //S2 ��ʱHͼ��
       Ht1621_off_disp(7);   //S1 ��ʱS1Oͼ��
      }
}

void Ht1621_BL(void) //����
{
    if ( time>= 3000 )  //1000  1 ms = 1s ʱ�䵽 
    {
     HT1621_BL(OFF);
     HT1621_LED(OFF);
    }  
    else
    {
     HT1621_BL(ON);
     HT1621_LED(ON);
    }
	
     if (time >= 3000)//��ֹ�������
    {
     time = 3000;
    }
}

void Ht1621Display(void)  //��ʾ
{
     Ht1621_on_disp(6);   //S2 ��ʱHͼ��
     Ht1621DisplayState(DIS_BUF,Ht1621Tab3,0,12);     //��ʾ
}



void device_work_mode_check(void)  //�յ����尴����ʾ����
{

	u8 tmp,tmp1,tmp2,tmp3,tmp4;
	
        tmp =device_work_data.para_type.device_mode;          //ģʽ
        tmp1 =device_work_data.para_type.wind_speed_state;  //����
        tmp2 =device_work_data.para_type.high_pressur_state;  //���� 
	tmp3 =device_work_data.para_type.pht_work_state;        //����
	
       switch(tmp)
    	{
    	case 0x00:
	break;

	case 0x01:
	if( key4 != 0) 
	{
        Ht1621_off_disp(2);    //T8 ���ֶ�
        Ht1621_on_disp(3);    //T7  ������
        Ht1621_off_disp(1);    //T9 �ض�ʱ
        device_work_data.para_type.device_mode = 1;
	}
	break;
	case 0x02:
        if( key4 != 0) 
        {
	 Ht1621_off_disp(3);   //T7  ������
        Ht1621_off_disp(1);    //T9 �ض�ʱ
        Ht1621_on_disp(2);   //T8 ���ֶ�
        device_work_data.para_type.device_mode = 2;	
        }
	break;
	default:  
	break;
    	}

     
      switch(tmp1)
      {
    	case 0x00:
        Ht1621_off_disp(13);    //S3 ���ٵ�
        Ht1621_off_disp(14);   //S3 ������
        Ht1621_off_disp(15);    //S3 ���ٸ�
        device_work_data.para_type.wind_speed_state = 0;				
	break;
	case 0x01:
        Ht1621_on_disp(13);    //S3 ���ٵ�  
        device_work_data.para_type.wind_speed_state = 1;
	break;
	case 0x02:
        Ht1621_on_disp(14);   //S3 ������
        device_work_data.para_type.wind_speed_state = 2;
	break;
	case 0x03:
        Ht1621_on_disp(15);    //S3 ���ٸ�
       device_work_data.para_type.wind_speed_state = 3;
	break;	
	default:  	
	break;

      	}

      switch(tmp2)
      {
    	case 0x00:
       Ht1621_off_disp(16);    //T1 �����
        device_work_data.para_type.high_pressur_state= 0;			
	break;
	case 0x01:
         Ht1621_on_disp(16);    //T1 ���� ��
        device_work_data.para_type.high_pressur_state= 1;	
	break;
	default:  
	break;
      	}

      switch(tmp3)
      {
    	case 0x00:
         Ht1621_off_disp(17);   //T2 ����� 
       device_work_data.para_type.pht_work_state = 0;			
	break;
	case 0x01:
         Ht1621_on_disp(17);   //T2 ���⿪ 
        device_work_data.para_type.pht_work_state = 0;	
	break;
	default:  
	break;
      	}
 
}







/*********************************************END OF FILE**********************/

