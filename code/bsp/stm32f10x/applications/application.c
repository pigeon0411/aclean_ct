/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>

#ifdef  RT_USING_COMPONENTS_INIT
#include <components.h>
#endif  /* RT_USING_COMPONENTS_INIT */

#ifdef RT_USING_DFS
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#ifdef RT_USING_RTGUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#include <rtgui/calibration.h>
#endif

#include "includes.h"
#include "rs485_decode.h"
#include "app_task.h"


#ifdef RT_USING_RTGUI
rt_bool_t cali_setup(void)
{
    rt_kprintf("cali setup entered\n");
    return RT_FALSE;
}

void cali_store(struct calibration_data *data)
{
    rt_kprintf("cali finished (%d, %d), (%d, %d)\n",
               data->min_x,
               data->max_x,
               data->min_y,
               data->max_y);
}
#endif /* RT_USING_RTGUI */

void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif

#ifdef  RT_USING_FINSH
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif  /* RT_USING_FINSH */

    /* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
    /* mount sd card fat partition 1 as root directory */
    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("File System initialized!\n");
    }
    else
        rt_kprintf("File System initialzation failed!\n");
#endif  /* RT_USING_DFS */

}



//====================����ϵͳ���߳����ȼ�==================================
#define thread_SysMonitor_Prio		    	11
#define thread_ModbusSlavePoll_Prio      	10
#define thread_ModbusMasterPoll_Prio      	 9


static rt_serial_t *serial;


u16 myreg1,myreg2;

extern USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];
extern USHORT   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];

extern DEVICE_WORK_TYPE device_work_data;

#define sw16(x) \
   ((short)(\
      (((short)(x) &(short)0x00ffU) << 8) |\
      (((short)(x) &(short)0xff00U) >> 8)))


extern eMBMasterReqErrCode eMBMasterReqRead_not_rtu_datas(UCHAR *ucMBFrame, USHORT usLength, LONG lTimeOut );

u8 rs485_send_buf_not_modbus[30];

void set_dc_motor(void)
{
	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;


	
	errorCode = eMBMasterReqRead_not_rtu_datas(rs485_send_buf_not_modbus,0,RT_WAITING_FOREVER);
	if(errorCode == MB_MRE_NO_ERR)
	{
		
	}


}

//***************************ϵͳ����߳�***************************
//��������: void thread_entry_SysRunLed(void* parameter)
//��ڲ�������
//���ڲ�������
//��    ע��Editor��  
//******************************************************************
void thread_entry_SysMonitor(void* parameter)
{
	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
	uint16_t errorCount = 0;

	u8 mstate = 0;
	
	while (1)
	{

		rt_thread_delay(RT_TICK_PER_SECOND/2);


		for(u8 i=11;i<=15;i++)
		{
			errorCode = eMBMasterReqReadHoldingRegister(1,0,2,RT_WAITING_FOREVER);

			if(errorCode == MB_MRE_NO_ERR)
			{
					device_work_data.para_type.house1_co2 = sw16(usMRegHoldBuf[0][0]);
					device_work_data.para_type.house1_pm2_5 = sw16(usMRegHoldBuf[0][1]);

			}
		}

		
		
		rt_thread_delay(RT_TICK_PER_SECOND/2);
		set_dc_motor();
		
		
	}
}




//************************ Modbus������ѵ�߳�***************************
//��������: void thread_entry_ModbusMasterPoll(void* parameter)
//��ڲ�������
//���ڲ�������
//��    ע��Editor��Armink   2013-08-28    Company: BXXJS
//******************************************************************
void thread_entry_ModbusMasterPoll(void* parameter)
{
	eMBMasterInit(MB_RTU, 2, 9600,  MB_PAR_NONE);
	eMBMasterEnable();

	serial = serial1;
	
	while (1)
	{
		eMBMasterPoll();
		rt_thread_delay(DELAY_MS(10));
	}
}



void rt_myt1_thread_entry(void* parameter)
{

	

	while(1)
	{


		rt_thread_delay(RT_TICK_PER_SECOND/20);
		//if(i<160)
		//{
		//	LenDrvZoomMove(1,1);
		//	i++;
		//}
		//else
		//{
		//	LenDrvZoomMove(0,1);
		//	i--;
		//}
	}
	
}

//0,off; 1,on
void ac_esd_set(u8 mode)
{
	if(mode)
		GPIO_SetBits(GPIOB,GPIO_Pin_1);
	else
		GPIO_ResetBits(GPIOB,GPIO_Pin_1);

}


//0,off; 1,on
void ac_ac_motor_set(u8 mode)
{
	if(mode)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_14);
		
		GPIO_ResetBits(GPIOB,GPIO_Pin_12);
		
		GPIO_ResetBits(GPIOB,GPIO_Pin_13);
	}
	else
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_14);

		GPIO_ResetBits(GPIOB,GPIO_Pin_12);
		
		GPIO_ResetBits(GPIOB,GPIO_Pin_13);
	}
}

//0,off; 1,on
void ac_pht_set(u8 mode)
{
	if(mode)
		GPIO_SetBits(GPIOB,GPIO_Pin_15);
	else
		GPIO_ResetBits(GPIOB,GPIO_Pin_15);

}


void airclean_out_pin_init(void)
{
	GPIO_InitTypeDef GPIOA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* configure the rs485 control pin of the receive or send */
	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_15|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|;
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIOA_InitStructure);

	ac_esd_set(1);
	ac_pht_set(1);
	ac_ac_motor_set(1);

	

}


//return 0,fault; 1,normal
u8 motor_state_get(u8 mode)
{
	return (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11));

}

u8 pht_state_get(u8 mode)
{
	return (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2));

}


u8 clean_state_get(u8 mode)
{
	return (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0));

}



u8 esd_state_get(u8 mode)
{
	return (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5));

}



u8 run_state_get(u8 mode)
{
	return (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4));

}



void airclean_in_pin_init(void)
{
	GPIO_InitTypeDef GPIOA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_2|GPIO_Pin_0;
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIOA_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIOA_InitStructure);


	
}



void airclean_system_init(void)
{

	airclean_in_pin_init();
	airclean_out_pin_init();


}

void rt_check_ex_device_thread_entry(void* parameter)
{

	while(1)
	{
		//0,��ʾ����,1��ʾ����

		device_work_data.para_type.fault_state  = motor_state_get(1);//bit7
		
		pht_state_get(1);//bit6
		esd_state_get(1);//bit5
		
		run_state_get(1);//bit4
		clean_state_get(1);//bit3

		
		rt_thread_delay(RT_TICK_PER_SECOND);


	}

}



void rt_main_thread_entry(void* parameter)
{

	airclean_system_init();

	rt_thread_delay(600);
	//rt_key_ctl_init();
	
	//rt_adc_ctl_init();

	
    wifi_comm_init();
}



int rt_application_init(void)
{
    rt_thread_t init_thread;


	
    init_thread = rt_thread_create("mY",
                                   rt_main_thread_entry, RT_NULL,
                                   1024, 6, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);


#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif

    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

		
    init_thread = rt_thread_create("SysMonitor",
                                   thread_entry_SysMonitor, RT_NULL,
                                   512, 11, 50);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);
			
	init_thread = rt_thread_create("MBMasterPoll",
								   thread_entry_ModbusMasterPoll, RT_NULL,
								   512, 9, 50);
	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);
					
			
    return 0;
}


/*@}*/
