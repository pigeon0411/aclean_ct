/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210B-EVAL Evaluation Board
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : STM32F103X RT-Thread 0.3.1 USB-CDC
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/
#define  BSP_MODULE

#include <bsp.h>
#include <rthw.h>
#include <rtthread.h>
#include "usart.h"
#include "gpio.h"


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/




 //*******************��ʼ���������Ź�*************************************
//��������: void IWDG_Configuration(void) 
//��    ������ʼ���������Ź�
//��ڲ�������
//���ڲ�������
//��    ע����Ƶ����=4*2^prer.�����ֵֻ����256!ʱ�����(���):Tout=40K/((4*2^prer)*rlr)ֵ	 2S��ʱ
//Editor��liuqh 2013-1-16  Company: BXXJS
//*******************************************************************
//static void IWDG_Configuration(void) 
//{
//	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);//ʹ�ܶ�IWDG->PR��IWDG->RLR��д
//	IWDG_SetPrescaler(IWDG_Prescaler_64);//64��Ƶ
//	IWDG_SetReload(1300);
//	IWDG_ReloadCounter();
//	IWDG_Enable();		
//}
//*******************ι�������Ź�*************************************
//��������: void IWDG_Feed(void)
//��    ������ʼ���������Ź�
//��ڲ�������
//���ڲ�����prer:��Ƶ��:0~7(ֻ�е�3λ��Ч!)��rlr:��װ�ؼĴ���ֵ:��11λ��Ч.
//��    ע����Ƶ����=4*2^prer.�����ֵֻ����256!ʱ�����(���):Tout=40K/((4*2^prer)*rlr)ֵ
//Editor��liuqh 2013-1-16  Company: BXXJS
//*******************************************************************

void IWDG_Feed(void)
{
	IWDG_ReloadCounter();//reload											   
}


/*******************************************************************************
 * Function Name  : SysTick_Configuration
 * Description    : Configures the SysTick for OS tick.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void  SysTick_Configuration(void)
{
	RCC_ClocksTypeDef  rcc_clocks;
	rt_uint32_t         cnts;

	RCC_GetClocksFreq(&rcc_clocks);

	cnts = (rt_uint32_t)rcc_clocks.HCLK_Frequency / RT_TICK_PER_SECOND;

	SysTick_Config(cnts);
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}
/**
 * This is the timer interrupt service routine.
 *
 */
void rt_hw_timer_handler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	rt_tick_increase();

	/* leave interrupt */
	rt_interrupt_leave();
}

/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               BSP_Init()
*
* Description : Initialize the Board Support Package (BSP).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function SHOULD be called before any other BSP function is called.
*********************************************************************************************************
*/


//****************************����ʱ����********************************
//��������: uint8_t AvoidTimeout(uint32_t TimeOfTimeout,uint32_t Period,uint8_t (*DetectCondition)())
//��    ������TimeOfTimeoutʱ���ڣ�ÿPeriodʱ����һ��DetectCondition()���ص�ֵ�Ƿ���Ч
//��ڲ�����TimeOfTimeout������ʱ��ʱ�䣨��λ��systick��
//          Period       ��ÿPeriodʱ����һ�Σ���ʱ�����ӣ���λ��systick��
//          (*DetectCondition)()���������������ConditionValue���������㣬��������������ʱPeriodʱ��������
//          ConditionValue      ������������ֵ
//���ڲ�����0����TimeOfTimeoutʱ���ڣ���⵽��������
//          1����TimeOfTimeoutʱ���ڣ�û�м�⵽��������
//��    ע��Editor��Armink 2012-03-09    Company: BXXJS
//**********************************************************************
uint8_t AvoidTimeout(uint32_t TimeOfTimeout,uint32_t Period,uint8_t (*DetectCondition)(),uint8_t ConditionValue)
{
	uint32_t LastTimeLocal, CurTimeLocal;
	uint8_t ConditionValueLocal;
	LastTimeLocal = rt_tick_get();
	CurTimeLocal  =  LastTimeLocal;
	while(CurTimeLocal - LastTimeLocal < TimeOfTimeout)
	{	 
		CurTimeLocal = rt_tick_get();
		ConditionValueLocal = DetectCondition();
		if (ConditionValueLocal == ConditionValue) return 0;
		rt_thread_delay(Period);
	}	
	return 1;
} 


//************************************��ʱ����**************************************
//��������: void Delay(vu32 nCount)
//��ڲ�����nCount ����ʱ�����У�ѭ���Ĵ���
//���ڲ�������
//��    ע��Editor��Armink 2011-03-18    Company: BXXJS
//**********************************************************************************





