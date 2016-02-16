
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
u8 Isr_com1=0;
u8 Isr_j=0;
u8 Isr_com = 0;

u8 rec_data_num = 0;

void serial_int1_receive(u8 udr1)//receive data from USAR1
{
    u8 k;

    Isr_i = udr1;

    if (0x00 == Isr_j) 
    {
        if(0xF1 !=Isr_i||0xF2 !=Isr_i)
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
        rxd1_buffer[Isr_com] = Isr_i;
        Isr_com++;

        if (Isr_com >= 7)
		{
    		Isr_com = 0x00; 
    		Isr_j = 0x00; 
    		rxd1_buff_cFlag = 0x01;	
        }
    }
    
}



void serial_int1_send(void)	   //send data to USAR1		   
{

    if (counter_send < cmd_send_lenth)
//	   UDR1=txd1_buffer[counter_send++];
	{
		USART_SendData(USART2, wifi_send_packet_buf_pub[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		counter_send++;
	}
    else
	   {
	   	USART_SendData(USART2, wifi_send_packet_buf_pub[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
	    counter_send = 1;
		txd1_buff_cFlag = 1;
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
        RS485_RX_ENABLE;
	   }
    
    if (counter_send>20)
        counter_send=1; 
}


void cmd_uart_check(void)
{
    if(rxd1_buff_cFlag)
    {
        rxd1_buff_cFlag = 0;

        if(rxd1_buffer[0] == 0xF1 && rxd1_buffer[1] == 0xf1)
        {
            if(rxd1_buffer[2] == 0x01)
            {
                return_current_device_state();

            }
        }
        else if(rxd1_buffer[0] == 0xF2 && rxd1_buffer[1] == 0xf2)
        {


        }

    }

}

int main(void)
{
    /* HT1621 端口配置 */ 
    HT1621_GPIO_Config ();

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
	
  while(1)
  {
     Key_Scan();   //按键扫描
     PollingKey();
     onoff_Scan(); //开关机
     cmd_uart_check();	
  }
}
/*********************************************END OF FILE**********************/

