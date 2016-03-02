
// 通用定时器TIMx,x[2,3,4,5]定时应用
#include "stm32f10x.h"
#include "bsp_led.h"
#include "bsp_TiMbase.h"
#include "api.h"

u8 in_com_buff[40];

u8 rxd1_buffer[40];
u8 counter_receive;
u8 rxd1_buff_cFlag = 0;

u8 txd1_buffer[20];
u8 counter_send;
u8 txd1_buff_cFlag;
u8 cmd_send_lenth;


extern u32 time_tick_cnt;
void fault_set_bit(u8 fault_type,u8 val);


extern volatile  u8 Ht1621_BUF[];


#if 1
#define FAULT_MOTOR_BIT    (0)
#define FAULT_PHT_BIT    (1)
#define FAULT_CLEAN_BIT    (2)
#define FAULT_ESD_BIT    (3)
#define FAULT_RUN_BIT    (4)
#define FAULT_WIND_BIT    (5)
#define FAULT_RESET_WIFI_BIT    (6)


#else
#define FAULT_ESD_BIT    (0)
#define FAULT_MOTOR_BIT    (1)
#define FAULT_PHT_BIT    (2)
#define FAULT_CLEAN_BIT    (3)
#define FAULT_RUN_BIT    (4)

#define FAULT_WIND_BIT    (5)
#define FAULT_RESET_WIFI_BIT    (6)
#endif

DEVICE_WORK_TYPE device_work_data;



u8 wifi_send_packet_buf_pub[100];


void txd1_buffer_send(void)
{
	txd1_buff_cFlag=0;		//clear camera command send complete flag
	rxd1_buff_cFlag=0;	    //clear camera command receive complete flag
	//UDR1=txd1_buffer[0];


    RS485_TX_ENABLE;
    delay_ms(1);
	//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); //
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE); 	 //
}


//对待发送的数据进行封包，并发送
//buf,待发送的缓冲区，len,发送缓冲区中所有数据的长度，不包括起始码两个字节0XF2,0XF2,也不包括校验和(一字节)和结束码(0X7E一字节)
// return 0,fail; 1,success
u8 wifi_send_packet_data(u8* buf,u8 len)
{
    u8 i;
    u8 chk;


    wifi_send_packet_buf_pub[0] = 0xF2;
    wifi_send_packet_buf_pub[1] = 0xF2;

    for(i=0;i<len;i++)
    {
        wifi_send_packet_buf_pub[i+2] = buf[i];
        chk += buf[i];    
    }

    wifi_send_packet_buf_pub[i+2] = chk;
    wifi_send_packet_buf_pub[i+3] = 0x7E;

    cmd_send_lenth = len+4;
    //wifi_send_data(wifi_send_packet_buf_pub,len+4);
	txd1_buffer_send();
		return 1;
}



u8 return_current_device_state(void)
{
    u8 buftmp[35];

    buftmp[0] = 0x01;
    buftmp[1] = 0x1b;

    u8 i;

    for(i=0;i<27;i++)
        {
        buftmp[2+i] = device_work_data.device_data[i];

    }

    wifi_send_packet_data(buftmp,29);


	fault_set_bit(FAULT_RESET_WIFI_BIT,0);

    return 0;
}




void uart2_init(void)
{
 // USART_2_InitStructure.USART_Parity=USART_Parity_Even;
  BSP_USART2_Init();
  APP_USART2_Baudrate(9600);
  
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);// Enable USART2 receive intterrupt 
  USART_Cmd(USART2, ENABLE); // Enable USART2
}



u8 Isr_j=0;
u8 Isr_com = 0;

u8 rec_data_num = 0;
extern u32 time_tick_cnt2;

void serial_int1_receive(u8 udr1)//receive data from USAR1
{
//    u8 k;

	static u32 cnt_tmp=0;

	
//	if(time_tick_cnt2 > cnt_tmp)
//		{
//			
//		if(time_tick_cnt2-cnt_tmp > 800)
//			{
//			Isr_j = 0;
//			return;
//		}
//	}

	
    if (0x00 == Isr_j) 
    {
        if(0xF1 !=udr1 && 0xF2 !=udr1)
        {
            Isr_com = 0; 
            Isr_j = 0;
            return;
        }

        if(0xF1 ==udr1)
            rec_data_num = 7;
        else
            rec_data_num = 33;
        
        rxd1_buffer[Isr_com] = udr1;
        Isr_com = 1; 
		Isr_j = 0x01;

    }
    else
    {
    	if(Isr_com == 1)
		{
			if(udr1 != 0xF1 && rxd1_buffer[0] == 0Xf1)
				{
				Isr_com = 0; 
				Isr_j = 0;
				return;


			}
			else if(udr1 != 0xF2 && rxd1_buffer[0] == 0Xf2)
				{
				            Isr_com = 0; 
            Isr_j = 0;
            return;

			}

		}
		
        rxd1_buffer[Isr_com] = udr1;
        Isr_com++;

        if (Isr_com >= rec_data_num)
		{
    		Isr_com = 0x00; 
    		Isr_j = 0x00; 
    		rxd1_buff_cFlag = 0x01;	
			time_tick_cnt2 = 0;

			for(u8 i=0;i<rec_data_num;i++)
				{
				in_com_buff[i] = rxd1_buffer[i];

				rxd1_buffer[i] = 0;
			}

			
        }
    }

	cnt_tmp = time_tick_cnt2;
}



void serial_int1_send(void)	   //send data to USAR1		   
{

    if (counter_send <= cmd_send_lenth)
//	   UDR1=txd1_buffer[counter_send++];
	{
		USART_SendData(USART2, wifi_send_packet_buf_pub[counter_send]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		counter_send++;
	}
    else
	   {
	   	USART_SendData(USART2, wifi_send_packet_buf_pub[counter_send]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
	    counter_send = 0;
		txd1_buff_cFlag = 1;
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		
		time_tick_cnt2 = 0;
        RS485_RX_ENABLE;
	   }
    
    if (counter_send>40)
        counter_send=0; 
}




/*
#define FAULT_MOTOR_BIT    (0)
#define FAULT_PHT_BIT    (1)
#define FAULT_CLEAN_BIT    (2)
#define FAULT_ESD_BIT    (3)
#define FAULT_RUN_BIT    (4)
#define FAULT_WIND_BIT    (5)
#define FAULT_RESET_WIFI_BIT    (6)

*/



void fault_set_bit(u8 fault_type,u8 val) 
{
    if(val)
        device_work_data.para_type.fault_state |= 1<<fault_type;
    else
        device_work_data.para_type.fault_state &= ~(1<<fault_type);


}

u8 fault_get_bit(u8 fault_type,u8 val) 
{
    //device_work_data.para_type.fault_state |= 1<<fault_type;
    
	return (val & (1<<fault_type));
}


/*
Ht1621_on_disp(8);    //T14 清洗故障
Ht1621_on_disp(9);    //T13 光氢故障
Ht1621_on_disp(10);  //T12 电机故障
Ht1621_on_disp(11);  //T11 静电故障
Ht1621_on_disp(12);   //T10 运行故障/ S5 风速高 /S4 风速中/S3 风速低


Ht1621_on_disp(8);    //T14 清洗故障
Ht1621_on_disp(9);    //T13 光氢故障
Ht1621_on_disp(10);  //T12 电机故障
Ht1621_on_disp(11);  //T11 静电故障
Ht1621_on_disp(12);    //T10 运行故障
*/

//ljy start 160302 
/*
void fault_check(void)
{
	u8 tmp;
	
	tmp = device_work_data.para_type.fault_state;

	u8 tmp2=0;

	u8 motortmp = fault_get_bit(0,tmp);

	if(motortmp)
		//Ht1621_on_disp(10);	  //T12 电机故障
		Ht1621_on_disp(11);	  //T11 静电故障
		
	for(u8 i=1;i<=5;i++)
	{
		if(fault_get_bit(i,tmp))
		{
			switch(i)
			{
			case FAULT_MOTOR_BIT:
				case FAULT_WIND_BIT:
				tmp = 1;
				Ht1621_on_disp(10);	  //T12 电机故障
				break;
			case FAULT_PHT_BIT:
				Ht1621_on_disp(9);	  //T13 光氢故障
				break;
			case FAULT_CLEAN_BIT:
				Ht1621_on_disp(8);	  //T14 清洗故障
				break;
			case FAULT_ESD_BIT:
				Ht1621_on_disp(11);	  //T11 静电故障
				break;
			case FAULT_RUN_BIT:
				Ht1621_on_disp(12);	 //T10 运行故障
				break;
				
			default:   //T13 光氢故障
				break;

			}

			
			delay_ms(50);
			Ht1621Display();  //PM2.5位置显示	
		}
		else
		{
			switch(i)
			{
			case FAULT_ESD_BIT:
			//case FAULT_MOTOR_BIT:
			case FAULT_WIND_BIT:
				if(!motortmp)
				{
					if(tmp!=1)
					//Ht1621_off_disp(10);	  //T12 电机故障
					Ht1621_off_disp(11);	  //T11 静电故障
				}
				break;
			case FAULT_PHT_BIT:
				Ht1621_off_disp(9);	  //T13 光氢故障
				break;
			case FAULT_CLEAN_BIT:
				Ht1621_off_disp(8);	  //T14 清洗故障
				break;
			//case FAULT_ESD_BIT:
			case FAULT_MOTOR_BIT:
				//Ht1621_off_disp(11);	  //T11 静电故障
				Ht1621_off_disp(10);	  //T12 电机故障
				break;
			case FAULT_RUN_BIT:
				Ht1621_off_disp(12);	  //T10 运行故障
				break;
			default:
				break;
			}

			
		}

	}
}
*/



#if 1
void fault_check(void)
{
	u8 tmp;
	
	tmp = device_work_data.para_type.fault_state;

	u8 tmp2=0;

	u8 motortmp = fault_get_bit(0,tmp);

	if(motortmp)
		Ht1621_on_disp(10);	  //T13 1a?a1ê??
		
	for(u8 i=1;i<=5;i++)
	{
		if(fault_get_bit(i,tmp))
		{
			switch(i)
			{
			case FAULT_MOTOR_BIT:
				case FAULT_WIND_BIT:
				tmp = 1;
				Ht1621_on_disp(10);	  //T13 1a?a1ê??
				break;
			case FAULT_PHT_BIT:
				Ht1621_on_disp(9);	  //T13 1a?a1ê??
				break;
			case FAULT_CLEAN_BIT:
				Ht1621_on_disp(8);	  //T13 1a?a1ê??
				break;
			case FAULT_ESD_BIT:
				Ht1621_on_disp(11);	  //T13 1a?a1ê??
				break;
			case FAULT_RUN_BIT:
				Ht1621_on_disp(12);	  //T13 1a?a1ê??
				break;
				
			default:   //T13 1a?a1ê??
				break;

			}

			
			delay_ms(50);
			Ht1621Display();  //PM2.5??????ê?	
		}
		else
		{
			switch(i)
			{
			case FAULT_MOTOR_BIT:
			case FAULT_WIND_BIT:
				if(!motortmp)
				{
					if(tmp!=1)
					Ht1621_off_disp(10);	  //T13 1a?a1ê??

				}
				break;
			case FAULT_PHT_BIT:
				Ht1621_off_disp(9);	  //T13 1a?a1ê??
				break;
			case FAULT_CLEAN_BIT:
				Ht1621_off_disp(8);	  //T13 1a?a1ê??
				break;
			case FAULT_ESD_BIT:
				Ht1621_off_disp(11);	  //T13 1a?a1ê??
				break;
			case FAULT_RUN_BIT:
				Ht1621_off_disp(12);	  //T13 1a?a1ê??
				break;
			default:
				break;
			}

			
		}

	}
}

#else
void fault_check(void)
{
	u8 tmp;
	tmp = device_work_data.para_type.fault_state;

	for(u8 i=0;i<=5;i++)
	{
		if(fault_get_bit(i,tmp))
		{
			switch(i)
			{
			case FAULT_ESD_BIT:
				Ht1621_on_disp(11);	  //T11 静电故障	
				break;
			case FAULT_MOTOR_BIT:
				Ht1621_on_disp(10);	  //T12 电机故障
				break;
			case FAULT_PHT_BIT:
				Ht1621_on_disp(9);	  //T13 光氢故障
				break;
			case FAULT_RUN_BIT:
				Ht1621_on_disp(12);	 //T10 运行故障
			       break;
			case FAULT_CLEAN_BIT:
				Ht1621_on_disp(8);	  //T14 清洗故障
				break;
			default:  
				break;

			}

			delay_ms(50);
			Ht1621Display();  //PM2.5位置显示	
		}
		else
		{
			switch(i)
			{
			case FAULT_ESD_BIT:
				Ht1621_off_disp(11);	  //T11 静电故障
				break;	
			case FAULT_MOTOR_BIT:
				Ht1621_off_disp(10);	  //T12 电机故障
			
				break;
			case FAULT_PHT_BIT:
				Ht1621_off_disp(9);	  //T13 光氢故障
				break;
			case FAULT_CLEAN_BIT:
				Ht1621_off_disp(8);	  //T14 清洗故障
				break;
			case FAULT_RUN_BIT:
				Ht1621_off_disp(12);	  //T10 运行故障
				break;
			default:
				break;
			}

		}

	}
}
//ljy end 160302 
#endif

u8 device_power_state_pre = 0xff;

void cmd_uart_check(void)
{
	u8 rx_buff_tmp[40];

	
    if(rxd1_buff_cFlag)
    {
        rxd1_buff_cFlag = 0;

		for(u8 i=0;i<40;i++)
			rx_buff_tmp[i] = in_com_buff[i];

		
		for(u8 i=0;i<40;i++)
			in_com_buff[i] = 0;

        if(rx_buff_tmp[0] == 0xF1 && rx_buff_tmp[1] == 0xf1)
        {
            if(rx_buff_tmp[2] == 0x01)
            {
                return_current_device_state();

            }
        }
        else if(rx_buff_tmp[0] == 0xF2 && rx_buff_tmp[1] == 0xf2)
        {

            device_work_data.para_type.house1_co2 = (u16)rx_buff_tmp[12]<<8;
			device_work_data.para_type.house1_co2 += (u16)rx_buff_tmp[13];
			
			device_work_data.para_type.house1_pm2_5 = (u16)rx_buff_tmp[10]<<8;
			device_work_data.para_type.house1_pm2_5 +=(u16)rx_buff_tmp[11];


            device_work_data.para_type.house2_co2 = (u16)rx_buff_tmp[16]<<8;
			device_work_data.para_type.house2_co2	+= (u16)rx_buff_tmp[17];
			
			device_work_data.para_type.house2_pm2_5 = (u16)rx_buff_tmp[14]<<8;
			device_work_data.para_type.house2_pm2_5	+= (u16)rx_buff_tmp[15];


            device_work_data.para_type.house3_co2 = (u16)rx_buff_tmp[20]<<8;
			device_work_data.para_type.house3_co2	+=(u16)rx_buff_tmp[21];
			
			device_work_data.para_type.house3_pm2_5 = (u16)rx_buff_tmp[18]<<8;
			device_work_data.para_type.house3_pm2_5+= (u16)rx_buff_tmp[19];

			

            device_work_data.para_type.house4_co2 = (u16)rx_buff_tmp[24]<<8;
			device_work_data.para_type.house4_co2+=(u16)rx_buff_tmp[25];
			
			device_work_data.para_type.house4_pm2_5 = (u16)rx_buff_tmp[22]<<8;
			device_work_data.para_type.house4_pm2_5+=(u16)rx_buff_tmp[23];

			

            device_work_data.para_type.house5_co2 = (u16)rx_buff_tmp[28]<<8;
			device_work_data.para_type.house5_co2+=(u16)rx_buff_tmp[29];
			
			device_work_data.para_type.house5_pm2_5 = (u16)rx_buff_tmp[26]<<8;
			device_work_data.para_type.house5_pm2_5+=(u16)rx_buff_tmp[27];

			device_work_data.para_type.fault_state = rx_buff_tmp[30];

			device_work_data.para_type.device_power_state = rx_buff_tmp[4];



#if 1
// 以下部分为收到主板关机命令后，将关闭显示屏，在主函数中仍然需要检测电源按键
//如果 device_work_data.para_type.device_power_state 为0的时候，屏幕将关闭，直到收到 device_work_data.para_type.device_power_state为1才开启屏幕
//或者直到按显示板的电源键才开开启显示
			if(device_power_state_pre==0xff || device_power_state_pre!= device_work_data.para_type.device_power_state)
			{
				if(device_work_data.para_type.device_power_state == 0)
				{
				
					onoff_device_set(OFF);

				}
				else
				{
					onoff_device_set(ON);
				}

				device_power_state_pre = device_work_data.para_type.device_power_state;
			}
#endif

			
			fault_check();
			
     
        }

    }

}


#define TICKS_PER_SECOND            1000//7000
u8 house_id = 0;

extern volatile u8 Ht1621Tab3[];




//复位WIFI
void reset_wifi(void)
{


	fault_set_bit(FAULT_RESET_WIFI_BIT,1);

	

}


void device_state_init(void)
{
    for(u8 i=0;i<sizeof(struct __para_type);i++)
    {
        device_work_data.device_data[i] = 0;
    }

	device_power_state_pre = 0xff;


	device_work_data.para_type.fault_state = 0;
	device_work_data.para_type.device_power_state = 1;
}


int main(void)
{
    u8 mybuff[10];

	/* HT1621 端口配置 */ 
	 HT1621_GPIO_Config ();
    

	/* 通用定时器 TIMx,x[2,3,4,5] 定时配置 */	
        TIMx_Configuration();
	
	/* 配置通用定时器 TIMx,x[2,3,4,5]的中断优先级 */
	TIMx_NVIC_Configuration();

	/* 通用定时器 TIMx,x[2,3,4,5] 重新开时钟，开始计时 */
	macTIM_APBxClock_FUN (macTIM_CLK, ENABLE);
	
	HT1621_BL(OFF);
	HT1621_LED(OFF);
       Ht1621_clrbuf(); 
       Ht1621_cls();  //清屏
       delay_ms(50);


	device_state_init();

	

	uart2_init();


#if 0	
	Ht1621_on_disp(8);	  //T14 清洗故障
	Ht1621_on_disp(9);	  //T13 光氢故障
	Ht1621_on_disp(10);  //T12 电机故障
	Ht1621_on_disp(11);  //T11 静电故障
	Ht1621_on_disp(12);    //T10 运行故障
	Ht1621Display();  //PM2.5位置显示	
#endif	



//
//				       Ht1621Tab3[0]= 12;   //PM2.5 高位
//					Ht1621Tab3[1]= 8;	//PM2.5 
//					Ht1621Tab3[2]= 7;  //PM2.5 
//					Ht1621Tab3[3]= 6;	//PM2.5 低位
//
//					Ht1621Tab3[7]=3;  // co2 低位
//					Ht1621Tab3[8]=2;  //co2 
//					Ht1621Tab3[9]=1;  //co2 
//					Ht1621Tab3[10]=5;//co2 高位
//


	time_tick_cnt = TICKS_PER_SECOND;




  while(1)
  {

		
        PollingKey1();
        Key_Scan();   //按键扫描
        PollingKey();

		onoff_Scan(); //开关机
		
        cmd_uart_check();	

        if(time_tick_cnt> TICKS_PER_SECOND )
        {

			for(u8 kk=0;kk<5;kk++)
			{
	            if(house_id >= 5)
	                house_id = 1;
	            else
	                house_id++;

	            Ht1621Tab3[4]=house_id;  //房间号

				
	            switch(house_id)
	            {
	            case 1:
	                
	                mybuff[0] = device_work_data.para_type.house1_pm2_5>>8;
	                mybuff[1] = device_work_data.para_type.house1_pm2_5&0xff;
	                mybuff[2] = device_work_data.para_type.house1_co2>>8;
	                mybuff[3] = device_work_data.para_type.house1_co2&0xff;
					
	                break;
	            case 2:
	                mybuff[0] = device_work_data.para_type.house2_pm2_5>>8;
	                mybuff[1] = device_work_data.para_type.house2_pm2_5&0xff;
	                mybuff[2] = device_work_data.para_type.house2_co2>>8;
	                mybuff[3] = device_work_data.para_type.house2_co2&0xff;
	                break;
	            case 3:
	                mybuff[0] = device_work_data.para_type.house3_pm2_5>>8;
	                mybuff[1] = device_work_data.para_type.house3_pm2_5&0xff;
	                mybuff[2] = device_work_data.para_type.house3_co2>>8;
	                mybuff[3] = device_work_data.para_type.house3_co2&0xff;
	                break;
	            case 4:
	                mybuff[0] = device_work_data.para_type.house4_pm2_5>>8;
	                mybuff[1] = device_work_data.para_type.house4_pm2_5&0xff;
	                mybuff[2] = device_work_data.para_type.house4_co2>>8;
	                mybuff[3] = device_work_data.para_type.house4_co2&0xff;
	                break;
	            case 5:
	                mybuff[0] = device_work_data.para_type.house5_pm2_5>>8;
	                mybuff[1] = device_work_data.para_type.house5_pm2_5&0xff;
	                mybuff[2] = device_work_data.para_type.house5_co2>>8;
	                mybuff[3] = device_work_data.para_type.house5_co2&0xff;
	                break;

	            default:
	                break;
	            }         

				if(mybuff[0]==0&&mybuff[1]==0&&mybuff[2]==0&&mybuff[3]==0)
				{
					continue;
				}
				else
				{


					u16 tmp;

					tmp = (u16)mybuff[0]<<8;
					tmp += (u16)mybuff[1];
					
				    Ht1621Tab3[0]= tmp/1000;   //PM2.5 高位
		            Ht1621Tab3[1]= tmp%1000/100;  //PM2.5 
		            Ht1621Tab3[2]= tmp%1000%100/10;  //PM2.5 
		            Ht1621Tab3[3]= tmp%1000%100%10;  //PM2.5 低位

					tmp = (u16)mybuff[2]<<8;
					tmp += (u16)mybuff[3];
				    Ht1621Tab3[10]= tmp/1000;   //PM2.5 高位
		            Ht1621Tab3[9]= tmp%1000/100;  //PM2.5 
		            Ht1621Tab3[8]= tmp%1000%100/10;  //PM2.5 
		            Ht1621Tab3[7]= tmp%1000%100%10;  //PM2.5 低位
					break;
				}

			}

            
            time_tick_cnt = 0;
        }
    }
}
/*********************************************END OF FILE**********************/

