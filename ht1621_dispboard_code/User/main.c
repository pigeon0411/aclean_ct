
// 通用定时器TIMx,x[2,3,4,5]定时应用
#include "stm32f10x.h"
#include "bsp_led.h"
#include "bsp_TiMbase.h"
#include "api.h"


u8 rxd1_buffer[40];
u8 counter_receive;
u8 rxd1_buff_cFlag = 0;

u8 txd1_buffer[20];
u8 counter_send;
u8 txd1_buff_cFlag;
u8 cmd_send_lenth;


extern u32 time_tick_cnt;


extern volatile  u8 Ht1621_BUF[];



DEVICE_WORK_TYPE device_work_data;

void device_state_init(void)
{
    for(u8 i=0;i<sizeof(struct __para_type);i++)
    {
        device_work_data.device_data[i] = 0;
    }

}

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



u8 Isr_i=0;
u8 Isr_j=0;
u8 Isr_com = 0;

u8 rec_data_num = 0;
extern u32 time_tick_cnt2;

void serial_int1_receive(u8 udr1)//receive data from USAR1
{
//    u8 k;

	static u32 cnt_tmp=0;

	if(time_tick_cnt2 > cnt_tmp)
		{
			
		if(time_tick_cnt2-cnt_tmp > 800)
			{
			Isr_j = 0;
			return;
		}
	}

	
    if (0x00 == Isr_j) 
    {
        if(0xF1 !=udr1 && 0xF2 !=udr1)
        {
            Isr_com = 0; 
            Isr_j = 0;
            return;
        }

        if(0xF1 ==Isr_i)
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





#define FAULT_MOTOR_BIT    (0)
#define FAULT_PHT_BIT    (1)
#define FAULT_CLEAN_BIT    (2)
#define FAULT_ESD_BIT    (3)
#define FAULT_RUN_BIT    (4)
#define FAULT_WIND_BIT    (5)




void fault_set_bit(u8 fault_type,u8 val) 
{
    if(!val)
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



*/
void fault_check(void)
{
	u8 tmp;
	
	tmp = device_work_data.para_type.fault_state;

	u8 tmp2=0;
	for(u8 i=0;i<=5;i++)
	{
		if(fault_get_bit(i,tmp))
		{
			switch(i)
			{
			case FAULT_MOTOR_BIT:
				case FAULT_WIND_BIT:
				tmp = 1;
				Ht1621_on_disp(10);	  //T13 光氢故障
				break;
			case FAULT_PHT_BIT:
				Ht1621_on_disp(9);	  //T13 光氢故障
				break;
			case FAULT_CLEAN_BIT:
				Ht1621_on_disp(8);	  //T13 光氢故障
				break;
			case FAULT_ESD_BIT:
				Ht1621_on_disp(11);	  //T13 光氢故障
				break;
			case FAULT_RUN_BIT:
				Ht1621_on_disp(12);	  //T13 光氢故障
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
			case FAULT_MOTOR_BIT:
			case FAULT_WIND_BIT:
				if(tmp!=1)
				Ht1621_off_disp(10);	  //T13 光氢故障
				break;
			case FAULT_PHT_BIT:
				Ht1621_off_disp(9);	  //T13 光氢故障
				break;
			case FAULT_CLEAN_BIT:
				Ht1621_off_disp(8);	  //T13 光氢故障
				break;
			case FAULT_ESD_BIT:
				Ht1621_off_disp(11);	  //T13 光氢故障
				break;
			case FAULT_RUN_BIT:
				Ht1621_off_disp(12);	  //T13 光氢故障
				break;
			default:
				break;
			}

			
		}

	}
}

void cmd_uart_check(void)
{
	u8 rx_buff_tmp[50];

	
    if(rxd1_buff_cFlag)
    {
        rxd1_buff_cFlag = 0;

		for(u8 i=0;i<50;i++)
			rx_buff_tmp[i] = rxd1_buffer[i];
		
        if(rx_buff_tmp[0] == 0xF1 && rx_buff_tmp[1] == 0xf1)
        {
            if(rx_buff_tmp[2] == 0x01)
            {
                return_current_device_state();

            }
        }
        else if(rx_buff_tmp[0] == 0xF2 && rx_buff_tmp[1] == 0xf2)
        {

            device_work_data.para_type.house1_co2 = (u16)rx_buff_tmp[10]<<8+rx_buff_tmp[11];
			device_work_data.para_type.house1_pm2_5 = (u16)rx_buff_tmp[12]<<8+rx_buff_tmp[13];


            device_work_data.para_type.house2_co2 = (u16)rx_buff_tmp[14]<<8+rx_buff_tmp[15];
			device_work_data.para_type.house2_pm2_5 = (u16)rx_buff_tmp[16]<<8+rx_buff_tmp[17];


            device_work_data.para_type.house3_co2 = (u16)rx_buff_tmp[18]<<8+rx_buff_tmp[19];
			device_work_data.para_type.house3_pm2_5 = (u16)rx_buff_tmp[20]<<8+rx_buff_tmp[21];

            device_work_data.para_type.house4_co2 = (u16)rx_buff_tmp[22]<<8+rx_buff_tmp[23];
			device_work_data.para_type.house4_pm2_5 = (u16)rx_buff_tmp[24]<<8+rx_buff_tmp[25];

            device_work_data.para_type.house5_co2 = (u16)rx_buff_tmp[26]<<8+rx_buff_tmp[27];
			device_work_data.para_type.house5_pm2_5 = (u16)rx_buff_tmp[28]<<8+rx_buff_tmp[29];

			device_work_data.para_type.fault_state = rx_buff_tmp[30];

			fault_check();
			
     
        }

    }

}


#define TICKS_PER_SECOND            1000
u8 house_id = 1;



int main(void)
{
    u8 mybuff[10];

    /* HT1621 端口配置 */ 
    HT1621_GPIO_Config ();

    SysTick_Config(SystemCoreClock / 1000);
	//time2_init();


    /* 通用定时器 TIMx,x[2,3,4,5] 定时配置 */	
    TIMx_Configuration();

	/* 配置通用定时器 TIMx,x[2,3,4,5]的中断优先级 */
	TIMx_NVIC_Configuration();

	/* 通用定时器 TIMx,x[2,3,4,5] 重新开时钟，开始计时 */
	macTIM_APBxClock_FUN (macTIM_CLK, ENABLE);
	
	uart2_init();

    
	HT1621_BL(OFF);
    Ht1621_clrbuf(); 
    Ht1621_cls();  //清屏
    delay_ms(50);

#if 1
					Ht1621_on_disp(10);   //T13 光氢故障

					Ht1621_on_disp(9);	  //T13 光氢故障

					Ht1621_on_disp(8);	  //T13 光氢故障

					Ht1621_on_disp(11);   //T13 光氢故障

					Ht1621_on_disp(12);   //T13 光氢故障

	
				
				Ht1621Display();  //PM2.5位置显示

#endif


    while(1)
    {

        Key_Scan();   //按键扫描
        PollingKey();
        onoff_Scan(); //开关机
        cmd_uart_check();	

        if(time_tick_cnt> TICKS_PER_SECOND )
        {
            if(house_id >= 5)
                house_id = 1;
            else
                house_id++;

            Ht1621_BUF[4]=house_id;  //房间号
            
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


            Ht1621_BUF[0]= mybuff[2]>>4;   //PM2.5 高位
            Ht1621_BUF[1]= mybuff[2]&0x0f;  //PM2.5 
            Ht1621_BUF[2]= mybuff[3]>>4;  //PM2.5 
            Ht1621_BUF[3]= mybuff[3]&0x0f;  //PM2.5 低位

            Ht1621_BUF[7]=mybuff[0]>>4;  // co2 低位
            Ht1621_BUF[8]=mybuff[0]&0x0f;  //co2 
            Ht1621_BUF[9]=mybuff[1]>>4;  //co2 
            Ht1621_BUF[10]=mybuff[1]&0x0f;  //co2 高位
            
            time_tick_cnt = 0;
        }
    }
}
/*********************************************END OF FILE**********************/

