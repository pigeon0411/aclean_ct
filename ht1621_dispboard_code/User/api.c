
#include "api.h"
#include <stdint.h>

static vu32 TimingDelay = 0; 
vu32 TimeDisplay = 0;


/* Initialize the USART2----------------------------------------------*/
void BSP_USART2_Init(void)
{
    GPIO_InitTypeDef GPIOA_InitStructure;
//	USART_InitTypeDef USART_2_InitStructure;
//	USART_ClockInitTypeDef USART_2_ClockInitStructure;
	NVIC_InitTypeDef NVIC_USART2_InitStructure;
//	INT16U nCount;

	/* open the USART2 RCC */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* open the USART2 intterrupt service */
	NVIC_USART2_InitStructure.NVIC_IRQChannel = USART2_IRQn;
					
	NVIC_USART2_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_USART2_InitStructure.NVIC_IRQChannelSubPriority = 7;//8;

	NVIC_USART2_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_USART2_InitStructure);

  	/* Configure USART2 Tx (PA2) as alternate function push-pull */ 
  	GPIOA_InitStructure.GPIO_Pin = TTLTXD2;
  	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOA, &GPIOA_InitStructure);
    
  	/* Configure USART2 Rx (PA3) as input floating */ 
  	GPIOA_InitStructure.GPIO_Pin = TTLRXD2;
  	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOA, &GPIOA_InitStructure);


	/* configure the rs485 control pin of the receive or send */
	GPIOA_InitStructure.GPIO_Pin = RS485_RX_TX_CTL_PIN;
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RS485_RX_TX_CTL_PORT, &GPIOA_InitStructure);
/*设置RS485为接收模式*/
	GPIO_ResetBits(RS485_RX_TX_CTL_PORT,RS485_RX_TX_CTL_PIN);

}


/*******************************************************************************
* Function Name  : fputc
* Description    : Retargets the C library printf function to the USART.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int fputc(int ch, FILE *f)
{
  	//Place your implementation of fputc here
  	//e.g. write a character to the USART 
  	//USART_SendData(USART1, (u8) ch);
	USART_SendData(USART1, (u8)ch);

  	//Loop until the end of transmission 
  	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

  	return ch;
}


void APP_USART1_Baudrate(u16 baudrate)
{
	USART_InitTypeDef USART_1_InitStructure;
//	INT16U BaudrateTemp;

	USART_ClockInitTypeDef  USART_ClockInitStructure;
//	USART_InitTypeDef USART_InitStructure;
	/* Configure the USART1 synchronous paramters */
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_ClockInit(USART1, &USART_ClockInitStructure);

//	USART_ClockInitTypeDef USART_1_ClockInitStructure;
	/* USART1 configuration */
  	/*USART1 configured as follow:
        - BaudRate = 9600 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
        - USART Clock disabled
        - USART CPOL: Clock is active low
        - USART CPHA: Data is captured on the middle 
        - USART LastBit: The clock pulse of the last data bit is not output to 
                         the SCLK pin
  	*/
//	BaudrateTemp = baudrate;
  	USART_1_InitStructure.USART_BaudRate = baudrate;
  	USART_1_InitStructure.USART_WordLength = USART_WordLength_8b;
  	USART_1_InitStructure.USART_StopBits = USART_StopBits_1;
  	USART_1_InitStructure.USART_Parity = USART_Parity_No;
  	USART_1_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  	USART_1_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;//使能发送和接收
	USART_Init(USART1, &USART_1_InitStructure);
	
}


void APP_USART1_IRQHandler(void)
{
	u8 udr0Temp = 0;   

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//判断是不是接收中断
	{
		if ((USART1->SR & USART_FLAG_RXNE) != (u16)RESET)
		{	           
			udr0Temp = ((u16)(USART1->DR & (u16)0x01FF)) & 0xFF;
			//uart0_rx_isr(udr0Temp);
		}
	}
}



void APP_USART2_Baudrate(u16 baudrate)
{
	USART_InitTypeDef USART_2_InitStructure;
	u16 BaudrateTemp;

    BSP_USART2_Init();
    
//	USART_ClockInitTypeDef USART_2_ClockInitStructure;
	/* USART2 configuration */
  	/*USART2 configured as follow:
        - BaudRate = 9600 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
        - USART Clock disabled
        - USART CPOL: Clock is active low
        - USART CPHA: Data is captured on the middle 
        - USART LastBit: The clock pulse of the last data bit is not output to 
                         the SCLK pin
  	*/
	BaudrateTemp = baudrate;
  	USART_2_InitStructure.USART_BaudRate = BaudrateTemp;
  	USART_2_InitStructure.USART_WordLength = USART_WordLength_8b;
  	USART_2_InitStructure.USART_StopBits = USART_StopBits_1;
  	USART_2_InitStructure.USART_Parity = USART_Parity_No;
  	USART_2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  	USART_2_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_2_InitStructure);
		
}

extern void serial_int1_receive(u8 udr1);//receive data from USAR1
extern void serial_int1_send(void);	   //send data to USAR1		   

void APP_USART2_IRQHandler(void)
{
	u8 udr1Temp = 0;

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//判断是不是接收中断
	{
		if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET) 
		//if ((USART2->SR & USART_IT_RXNE) != (u16)RESET)
		{	
			udr1Temp = USART_ReceiveData(USART2) & 0xFF;
			serial_int1_receive(udr1Temp);
			//USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		}
	}
	if(USART_GetITStatus(USART2, USART_IT_TXE) == SET) 
	{ 	
		if(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == SET) 
			serial_int1_send();			
    } 
	
	return;		
}


/*************************************************************************************/




