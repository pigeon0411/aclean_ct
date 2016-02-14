
// 通用定时器TIMx,x[2,3,4,5]定时应用
#include "stm32f10x.h"
#include "bsp_led.h"
#include "bsp_TiMbase.h"
#include "api.h"

void uart1_init(void)
{
 // USART_2_InitStructure.USART_Parity=USART_Parity_Even;
  BSP_USART2_Init();
  APP_USART2_Baudrate(9600);
  
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);// Enable USART2 receive intterrupt 
  USART_Cmd(USART2, ENABLE); // Enable USART2
}

void serial_int1_receive(uchar udr1)//receive data from USAR1
{
uchar k;

//rxd1_buffer[counter_receive]=UDR1;
rxd1_buffer[counter_receive] = udr1;

if (cameraType==SONY)	   //sony
   {
   if (rxd1_buffer[counter_receive]==0xff)		   //command end byte
      {
	   counter_receive=0;
	   if ((rxd1_buffer[0]==0x90) && ((rxd1_buffer[1] & 0xFE) == 0x50))
	      rxd1_buff_cFlag=1;
	  }
   else
      counter_receive++;
   }
else if (cameraType==HITACHI)
   {
   	if ((rxd1_buffer[counter_receive]==':') || (rxd1_buffer[counter_receive]=='/'))		   //hitachi command start byte  
       {
	    rxd1_buffer[0]=rxd1_buffer[counter_receive];
		counter_receive=0;
		
	   }
    counter_receive++;
    if (counter_receive==cmd_lenth)
    {
		rxd1_buff_cFlag=1;
		counter_receive=0;
#if IP_MODULE_USING
		USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); //
#endif
		
    }
   }   
else if (cameraType==CNB)
   {
   if (rxd1_buffer[counter_receive]=='*')		   //CNB command start byte  
      counter_receive=0;

   counter_receive++;
   if (counter_receive>10)
   	  {
		USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); //

		
	   counter_receive=0;
	   rxd1_buff_cFlag=1;
	  }
   }
else if (cameraType==LG)
   {
   if (rxd1_buffer[counter_receive]==0xC5)
      counter_receive=0;

   counter_receive++;
   if (counter_receive > 8)
   	  {
	   counter_receive=0;
	   rxd1_buff_cFlag=1;
	  }
   }
else if ((cameraType==SANYO) && (sanyoANT_cmd_receiveAllow==1))	   //SANYO
   {
   if (rxd1_buffer[counter_receive]==0xff)		   //command end byte
      {   
	   k=1;
	   while(k < counter_receive)   //checkout
	        {
			 rxd1_buffer[0] = rxd1_buffer[0] + rxd1_buffer[k];
			 k++;
			}
	   if (!(rxd1_buffer[0] & 0x7F))
	      rxd1_buff_cFlag=1;
		  
	   sanyoANT_cmd_receiveAllow=0;
	   counter_receive=0;
	  }
   else
      counter_receive++;
   }
 else if (cameraType==SAMSUNG) //samsung
   {
    /*if (rxd1_buffer[0]==0xA0 || rxd1_buffer[0]==0xA1)
	   {if ((counter_receive==5 || counter_receive==7) && rxd1_buffer[counter_receive]==0xAF)
	       {rxd1_buff_cFlag=1;
		    counter_receive=0;
		    //rxd1_buffer[0]=0;
		    //rxd1_buffer[counter_receive]=0;
		   }
		else if (counter_receive<7)
	       counter_receive++;
		else
	       {counter_receive=0;
		    //rxd1_buffer[0]=0;
		    //rxd1_buffer[counter_receive]=0;
		   }
	   }*/
	    if (rxd1_buffer[counter_receive]==0xaf)		   //command end byte
      {
      	//rxd1_buffer[counter_receive]=0;
	   counter_receive=0;
	   rxd1_buff_cFlag=1;
	  // rxd1_buffer[0]=0;
	  }
   else
      counter_receive++;
   }
 else if (cameraType == ANT && sanyoANT_cmd_receiveAllow)     //ANT
   {
    if (counter_receive == 0)
       {
	    if (rxd1_buffer[counter_receive] == 0x06)
		   counter_receive++;
		else
		   counter_receive = 0;
	   }
    else if (counter_receive == 1)
       {
	    if (rxd1_buffer[counter_receive] == txd1_buffer[counter_receive-1])
		   counter_receive++;
		else
		   counter_receive = 0;
	   }
    else if (counter_receive == 2)
       {
	    if (rxd1_buffer[counter_receive] == txd1_buffer[counter_receive-1])
		   counter_receive++;
		else
		   counter_receive = 0;
	   }
	else if (counter_receive < 7)
	   counter_receive++;
	else
	   {
	    counter_receive = 0; //counter_receive is a temp here,for the checksum
	    for (k=1; k<8; k++)
		    counter_receive += rxd1_buffer[k];
		if (counter_receive == 0)
		   rxd1_buff_cFlag = 1;
		
		counter_receive = 0;
	   }
   }
 else if (cameraType == KTC)
   {
    if (0xf5 == rxd1_buffer[0]) //command head
	   counter_receive++;
	else
	   counter_receive = 0;
	   
	if (counter_receive > 7)
	   {
	    counter_receive = rxd1_buffer[0]; //counter_receive is a temp here,for the checksum
	    for (k=1; k<7; k++)
		    counter_receive += rxd1_buffer[k];
		if (counter_receive == rxd1_buffer[7])
		   rxd1_buff_cFlag = 1;
		
		counter_receive = 0;
	   }
   }
 else if (cameraType == LG_NEW)
   {
    if (0xE5 == rxd1_buffer[0]) //command head
	   counter_receive++;
	else
	   counter_receive = 0;
	   
	if (counter_receive > (cmd_receive_lenth - 1))
	   {
	    counter_receive = 0;
	    if (rxd1_buffer[cmd_receive_lenth-1] == cam_LGnew_checksum(rxd1_buffer, cmd_receive_lenth-1))
		   rxd1_buff_cFlag = 1;
	   } 

   }

 if (counter_receive > 14)
    counter_receive = 0;
}

void serial_int1_send(void)	   //send data to USAR1		   
{
if (cameraType==SONY)	   //sony
  { 	
   if (txd1_buffer[counter_send-1]!=0xff)		
//      UDR1=txd1_buffer[counter_send++];
	{
		USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		counter_send++; 
	}
   else
      {
	   	USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE); 
	   counter_send=1;
	   txd1_buff_cFlag=1;
	   USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	  }
   }
else if (cameraType==HITACHI)
   {
    delay_2us(10);
   	if (counter_send<cmd_lenth)
//	   UDR1=txd1_buffer[counter_send++];
	{
		USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		counter_send++; 
	}
    else
   	   {
	   	USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
	    counter_send=1;
	    txd1_buff_cFlag=1;
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	   }
   } 
else if (cameraType==CNB)
   {
   if (counter_send<cnb_send_bytes_len)
   	  {
//	   TCCR0 = 0x00; //stop timer0
//       TCNT0 = 0x83; //set count
//	   TCCR0 = 0x06; //start timer0
	   
	  // cam_command_sendEnable=1;
	   //counter_send++;

	   		USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		counter_send++; 

	  }
   else
   	  {
	 /*  cam_command_sendEnable=0;
	   counter_send=1;
	   txd1_buff_cFlag=1;	  */


			   	USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE); 
	   counter_send=1;
	   txd1_buff_cFlag=1;
	   cnb_send_bytes_len = 1;
	   USART_ITConfig(USART2, USART_IT_TXE, DISABLE);

	  }
   }
else if (cameraType==LG)
   {
    if (1==cam_subType || 5==cam_subType) //LG/ss880 camera
	   {
        if (counter_send<6)
	       {
//	        TCCR0 = 0x00; //stop timer0
//            TCNT0 = 0x83; //set count
//	        TCCR0 = 0x06; //start timer0
	   
	        cam_command_sendEnable=1;
	        counter_send++;
		   }
        else
   	       {
	        cam_command_sendEnable=0;
	        counter_send=1;
	        txd1_buff_cFlag=1;
	       }
	   }
	else      //UK camera
	   {
   	    if (counter_send<6)
//	       UDR1=txd1_buffer[counter_send++];
		{
			USART_SendData(USART2, txd1_buffer[counter_send-1]);
			USART_ClearITPendingBit(USART2, USART_IT_TXE);
			counter_send++; 
		}
        else
   	       {
		   	USART_SendData(USART2, txd1_buffer[counter_send-1]);
			USART_ClearITPendingBit(USART2, USART_IT_TXE);
	        counter_send=1;
	        txd1_buff_cFlag=1;
			USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	       } 
	   }
   }
else if (cameraType==SANYO)	   //SANYO
   {
   if (txd1_buffer[counter_send-1]!=0xff)		   //
//      UDR1=txd1_buffer[counter_send++];
	{
		USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		counter_send++; 
	}
   else
      {
	  	USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
	   counter_send=1;
	   txd1_buff_cFlag=1;
	   USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	  }
   }
else if (cameraType==SAMSUNG)	   //SAMSUNG
   {
//    if ((txd1_buffer[0]==0xA0 && counter_send<6) || (txd1_buffer[0]==0xA1 && counter_send<8))
//	   UDR1=txd1_buffer[counter_send++];
//    else
//   	{
//	    counter_send=1;
//	    txd1_buff_cFlag=1;
//	   }
	if (txd1_buffer[counter_send-1]!=0xaf)		   //
//      UDR1=txd1_buffer[counter_send++];
	{
		USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		counter_send++; 
	}
   else
      {
	  	USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
	   counter_send=1;
	   txd1_buff_cFlag=1;
	   USART_ITConfig(USART2, USART_IT_TXE, DISABLE);

	  }
   }
else if (cameraType==ANT)	   //ANT
   {
    if (counter_send<7)
//	   UDR1=txd1_buffer[counter_send++];
	{
		USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		counter_send++; 
	}
    else
   	   {
	   	USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
	    counter_send=1;
	    txd1_buff_cFlag=1;
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	   } 
   }
else if (cameraType==KTC)    //KTC
   {
   if (counter_send<8)
   	  {
//	   TCCR0 = 0x00; //stop timer0
//       TCNT0 = 0x83; //set count
//	   TCCR0 = 0x06; //start timer0
	   
	    USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		counter_send++;
	  /* cam_command_sendEnable=1;
	   counter_send++;	 */
	  }
   else
   	  {
	   	USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
	    counter_send = 1;
		txd1_buff_cFlag = 1;
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	  /*
	   cam_command_sendEnable=0;
	   counter_send=1;
	   txd1_buff_cFlag=1; */
	  }
   }
else if (cameraType == LG_NEW)	   //LG_NEW
   {
    if (counter_send < cmd_send_lenth)
//	   UDR1=txd1_buffer[counter_send++];
	{
		USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		counter_send++;
	}
    else
	   {
	   	USART_SendData(USART2, txd1_buffer[counter_send-1]);
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
	    counter_send = 1;
		txd1_buff_cFlag = 1;
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	   }
   }

if (counter_send>20)
   counter_send=1; 
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
	
	uart1_init();
	HT1621_BL(OFF);
       Ht1621_clrbuf(); 
       Ht1621_cls();  //清屏
       delay_ms(50);
	
  while(1)
  {
      
     Key_Scan();   //按键扫描
     PollingKey();
     onoff_Scan(); //开关机
 	
  }
}
/*********************************************END OF FILE**********************/

