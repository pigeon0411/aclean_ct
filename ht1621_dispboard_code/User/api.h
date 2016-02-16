#ifndef _BSP_H_
#define _BSP_H_

/*Includes------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"


//#define uchar unsigned char
//#define uint unsigned short

/*Defines**************************************************************/


#define	SYSTEM_CPU_CLOCK_FREQUENCE_8M	1
#define	SYSTEM_CPU_CLOCK_FREQUENCE_16M	2
#define	SYSTEM_CPU_CLOCK_FREQUENCE_24M	3
#define	SYSTEM_CPU_CLOCK_FREQUENCE_32M	4
#define	SYSTEM_CPU_CLOCK_FREQUENCE_40M	5
#define	SYSTEM_CPU_CLOCK_FREQUENCE_48M	6
#define	SYSTEM_CPU_CLOCK_FREQUENCE_56M	7
#define	SYSTEM_CPU_CLOCK_FREQUENCE_64M	8
#define	SYSTEM_CPU_CLOCK_FREQUENCE_72M	9

#define	SYSTEM_CPU_CLOCK_FREQUENCE	SYSTEM_CPU_CLOCK_FREQUENCE_72M

//8M 外部晶振
#define	EXTERNAL_OSC_FREQUENCE		(double)8000000	

//单位为HZ 表示定时器的时钟频率,定时器时钟总线频率为系统时钟频率的1/2
#define	TIMER_CLOCK_FREQUENCE	(double)(EXTERNAL_OSC_FREQUENCE/2*SYSTEM_CPU_CLOCK_FREQUENCE) 



/*--------------------------------------------------------------------*/
/* USART1 for rs485 */
#define RS485RXD1 GPIO_Pin_10       /* PA10 */
#define RS485TXD1 GPIO_Pin_9		/* PA9 */


#define RS485Control GPIO_Pin_1	/* PA1 */

#define	RS485_RX_TX_CTL_PORT	GPIOA	//rs485发送，接收使能引脚控制为PE0
#define	RS485_RX_TX_CTL_PIN		GPIO_Pin_1

#define	RS485_RX_ENABLE		(RS485_RX_TX_CTL_PORT->BRR = RS485_RX_TX_CTL_PIN)

#define	RS485_TX_ENABLE		(RS485_RX_TX_CTL_PORT->BSRR = RS485_RX_TX_CTL_PIN)


/*--------------------------------------------------------------------*/ 
/* USART2 for TTL of the  */
#define TTLRXD2 GPIO_Pin_3		/* PA3 */
#define TTLTXD2 GPIO_Pin_2		/* PA2 */
/*--------------------------------------------------------------------*/ 



/* Initialize the USART1----------------------------------------------*/
void BSP_USART1_Init(void);

/* Initialize the USART2----------------------------------------------*/
void BSP_USART2_Init(void);



/************************************************************************/
/* Retargets the C library printf function to the USART. */
 int fputc(int ch, FILE *f);


/* Decrements the TimingDelay variable. */


void APP_USART1_Baudrate(u16 baudrate);

void APP_USART1_IRQHandler(void);

void APP_USART2_Baudrate(u16 baudrate);

void APP_USART2_IRQHandler(void);


#endif
