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
#include "rtdevice.h"
#include "application.h"
#include "bsp_spi_flash.h"

#include "disp_board.h"



extern void fault_set_bit(u8 fault_type,u8 val) ;
void airclean_power_onoff(u8 mode);


extern void thread_entry_com_displayboard(void* parameter);

extern void set_display_board_data(void);
extern void get_display_board_data(void);
extern void thread_entry_ModbusMasterPoll(void* parameter);













u32 power_tim_cnt = 0;


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



//====================操作系统各线程优先级==================================
#define thread_SysMonitor_Prio		    	11
#define thread_ModbusSlavePoll_Prio      	10
#define thread_ModbusMasterPoll_Prio      	 9


//static rt_serial_t *serial;


u16 myreg1,myreg2;

extern USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];
extern USHORT   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];

extern DEVICE_WORK_TYPE device_work_data;

#define sw16(x) \
   ((short)(\
      (((short)(x) &(short)0x00ffU) << 8) |\
      (((short)(x) &(short)0xff00U) >> 8)))


#define	exchange_16bit_hi_lo(x)		sw16(x)


extern eMBMasterReqErrCode eMBMasterReqRead_not_rtu_datas(UCHAR *ucMBFrame, USHORT usLength, LONG lTimeOut );

u8 rs485_send_buf_not_modbus[50];


void set_dc_motor_speed(u8 speed)
{
	//device_work_data.para_type.wind_speed_state = speed;

    rs485_send_buf_not_modbus[0] = 0xBC;
    rs485_send_buf_not_modbus[1] = 0x07;
    rs485_send_buf_not_modbus[2] = 0x01;
    rs485_send_buf_not_modbus[3] = speed>3?3:speed;
    rs485_send_buf_not_modbus[4] = rs485_send_buf_not_modbus[0]+rs485_send_buf_not_modbus[1]+rs485_send_buf_not_modbus[2]+rs485_send_buf_not_modbus[3];

}

extern volatile UCHAR  ucMasterRTURcvBuf[256];
extern volatile UCHAR  ucMasterRTUSndBuf[256];

void set_dc_motor(void)
{
	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;


	if(device_work_data.para_type.device_power_state==0)
	{
		
		set_dc_motor_speed(0);
	}
	else
	{
		set_dc_motor_speed(device_work_data.para_type.wind_speed_state);

	}

    ucMasterRTURcvBuf[0] = 0;
	errorCode = eMBMasterReqRead_not_rtu_datas(rs485_send_buf_not_modbus,5,RT_WAITING_FOREVER);
	if(errorCode == MB_MRE_REV_DATA)
	{
		if(ucMasterRTURcvBuf[0] == 0xBC && ucMasterRTURcvBuf[1] == 0x07)
        {
            u8 tmp;

            tmp  = ucMasterRTURcvBuf[3];
            if(tmp&0x03)
                {
                fault_set_bit(FAULT_MOTOR_BIT,1);
            }
            else
                {
                fault_set_bit(FAULT_MOTOR_BIT,0);
            }
            
        }   
	}


}

//void get_display_board_data(void)
//{
//	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
//	u8 i;
//	static u8 device_power_state_bak = 0xff;
//	
//    rs485_send_buf_not_modbus[0] = 0xF1;
//    rs485_send_buf_not_modbus[1] = 0xF1;
//    rs485_send_buf_not_modbus[2] = 0x01;
//    rs485_send_buf_not_modbus[3] = 0x01;
//    rs485_send_buf_not_modbus[4] = 0x00;
//    rs485_send_buf_not_modbus[5] = 0x02;
//    rs485_send_buf_not_modbus[6] = 0x7E;

//    ucMasterRTURcvBuf[0] = 0;
//	errorCode = eMBMasterReqRead_not_rtu_datas(rs485_send_buf_not_modbus,7,RT_WAITING_FOREVER);
//	if(errorCode == MB_MRE_REV_DATA)
//	{
//		if(ucMasterRTURcvBuf[0] == 0xF2 && ucMasterRTURcvBuf[1] == 0xF2 && ucMasterRTURcvBuf[32] == 0x7e)
//        {

//        
//    		for(i=0;i<sizeof(struct __para_type);i++)
//			{
//				
//				device_work_data_bak.device_data[i] = device_work_data.device_data[i];
//				
//				
//			}


//			
//            //device_work_data.para_type.device_power_state = ucMasterRTURcvBuf[4];
//            device_work_data.para_type.device_mode = ucMasterRTURcvBuf[5];

//            if(device_work_data.para_type.device_mode == 1)
//            {

//                ;
//            }
//            else
//            {

//                device_work_data.para_type.wind_speed_state = ucMasterRTURcvBuf[6];
//                device_work_data.para_type.high_pressur_state = ucMasterRTURcvBuf[7];
//                device_work_data.para_type.pht_work_state = ucMasterRTURcvBuf[8];
//            }

//            device_work_data.para_type.timing_state = ucMasterRTURcvBuf[9];
//            //device_work_data.para_type.fault_state = ucMasterRTURcvBuf[9];

//			//fault_set_bit(FAULT_RESET_WIFI_BIT,ucMasterRTURcvBuf[30]&(1<<FAULT_RESET_WIFI_BIT));

////			if(device_work_data.para_type.device_mode == 2)
////				airclean_power_onoff(device_work_data.para_type.device_power_state);


//			
//			if(ucMasterRTURcvBuf[30]&(1<<FAULT_RESET_WIFI_BIT))
//			{
//				
//				wifi_factory_set();

//			}

//			for(i=0;i<6;i++)
//			{
//				
//				if(device_work_data_bak.device_data[i] != device_work_data.device_data[i])
//				{

//					device_sys_para_save();
//					break;
//				}
//				
//				
//			}


//					

//				
//        }      
//		else if(ucMasterRTURcvBuf[0] == 0xF1 && ucMasterRTURcvBuf[1] == 0xF1 && ucMasterRTURcvBuf[6] == 0x7e)
//		{

//			device_work_data.para_type.device_power_state = ucMasterRTURcvBuf[4];
//			if(device_power_state_bak == 0xff)
//			{

//				device_power_state_bak = ucMasterRTURcvBuf[4];	

//				if(ucMasterRTURcvBuf[4])
//				{
//	
//					airclean_power_onoff(ucMasterRTURcvBuf[4]);
//				}
//				else
//					{

//					airclean_power_onoff(0);
//					return;
//				}
//				
//				
//			}
//			else
//			{

//				if(ucMasterRTURcvBuf[4] != device_power_state_bak)
//				{
//					device_power_state_bak = ucMasterRTURcvBuf[4];
//					
//					airclean_power_onoff(ucMasterRTURcvBuf[4]);

//					if(ucMasterRTURcvBuf[4] == 0)
//						return;
//				}


//				device_power_state_bak = ucMasterRTURcvBuf[4];
//			}
//		}
//	}
//}

//u8 disp_board_packet_data(u8 len)
//{
//
//    u8 buftmp[35];
//
//    buftmp[0] = 0x01;
//    buftmp[1] = 0x1b;
//
//    u8 i;
//
//    for(i=0;i<27;i++)
//        {
//        buftmp[2+i] = device_work_data.device_data[i];
//
//    }
//
//
//    u8 chk;
//
//
//    rs485_send_buf_not_modbus[0] = 0xF2;
//    rs485_send_buf_not_modbus[1] = 0xF2;
//
//    for(i=0;i<len;i++)
//    {
//        rs485_send_buf_not_modbus[i+2] = buftmp[i];
//        chk += buftmp[i];    
//    }
//
//    rs485_send_buf_not_modbus[i+2] = chk;
//    rs485_send_buf_not_modbus[i+3] = 0x7E;
//	
//		return 1;
//}
//
//
//void set_display_board_data(void)
//{
//	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
//
//    disp_board_packet_data(33-4);
//
//    ucMasterRTURcvBuf[0] = 0;
//	errorCode = eMBMasterReqRead_not_rtu_datas(rs485_send_buf_not_modbus,33,RT_WAITING_FOREVER);
//	if(errorCode == MB_MRE_REV_DATA)
//	{
//		if(ucMasterRTURcvBuf[0] == 0xF2 && ucMasterRTURcvBuf[1] == 0xF2 && ucMasterRTURcvBuf[32] == 0x7e)
//        {
//            ;
//        }      
//	}
//}
//

FLASH_Status FLASH_Program_save_para(void)
{
    FLASH_Status revl;

	u16 i;
	u32 addr_temp;

	u32 data;
	
    FLASH_Unlock();


	FLASH_ClearFlag(FLASH_FLAG_PGERR);


	FLASH_ErasePage(SYS_PARA_ADDR);


	FLASH_ProgramWord(SYS_PARA_ADDR-1,0x86123456);
	
	for(i=0;i<20;)
	{
		addr_temp = i + SYS_PARA_ADDR;


		data = device_work_data.device_data[i] + (u32)device_work_data.device_data[i+1]<<8 + (u32)device_work_data.device_data[i+2]<<16 + (u32)device_work_data.device_data[i+3]<<24;

		
		FLASH_ProgramWord(addr_temp,data);
		
		i = i+4;
	}

	
    return revl;

}

void FLASH_Program_read_para(void)
{


	u16 i;
	u32 addr_temp;

	u32 data;
	

	if(__Read_Flash_Word(SYS_PARA_ADDR-1) == 0x86123456)
	{
		for(i=0;i<20;)
		{
			addr_temp = i + SYS_PARA_ADDR;
		
		
			data = __Read_Flash_Word(addr_temp);

			device_work_data.device_data[i] = data&0xff;
			device_work_data.device_data[i+1] = (data>>8)&0xff;
		
			device_work_data.device_data[i+1] = (data>>16)&0xff;
			device_work_data.device_data[i+1] = (data>>24)&0xff;
			
			i = i+4;
		}


	}


	

}

rt_mutex_t modbus_mutex = RT_NULL;
rt_mutex_t motor_mutex = RT_NULL;


void thread_dc_motor_set(void* parameter)
{

	u8 dc_motor_state_pre = 0xff;
	
	while (1)
	{
		
		//rt_thread_delay(RT_TICK_PER_SECOND/10);
		rt_thread_delay(RT_TICK_PER_SECOND/2);

		
		set_dc_motor();//1s


		
	}
}



void get_display_board_data(void)
{
	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
	u8 i;
	static u8 device_power_state_bak = 0xff;

	rt_mutex_take(modbus_mutex,RT_WAITING_FOREVER);
	
    rs485_send_buf_not_modbus[0] = 0xF1;
    rs485_send_buf_not_modbus[1] = 0xF1;
    rs485_send_buf_not_modbus[2] = 0x01;
    rs485_send_buf_not_modbus[3] = 0x01;
    rs485_send_buf_not_modbus[4] = 0x00;
    rs485_send_buf_not_modbus[5] = 0x02;
    rs485_send_buf_not_modbus[6] = 0x7E;

       ucMasterRTURcvBuf[0] = 0; 
	errorCode = eMBMasterReqRead_not_rtu_datas(rs485_send_buf_not_modbus,7,RT_WAITING_FOREVER);
	if(errorCode == MB_MRE_REV_DATA)
	{
		if(ucMasterRTURcvBuf[0] == 0xF2 && ucMasterRTURcvBuf[1] == 0xF2 && ucMasterRTURcvBuf[32] == 0x7e)
        {

        
    		for(i=0;i<sizeof(struct __para_type);i++)
			{
				
				device_work_data_bak.device_data[i] = device_work_data.device_data[i];
				
				
			}


			
            //device_work_data.para_type.device_power_state = ucMasterRTURcvBuf_2[4];
            device_work_data.para_type.device_mode = ucMasterRTURcvBuf[5];

            if(device_work_data.para_type.device_mode == 1)
            {
            
            }
            else
            {

                device_work_data.para_type.wind_speed_state = ucMasterRTURcvBuf[6];
                device_work_data.para_type.high_pressur_state = ucMasterRTURcvBuf[7];
                device_work_data.para_type.pht_work_state = ucMasterRTURcvBuf[8];
            }

            device_work_data.para_type.timing_state = ucMasterRTURcvBuf[9];
            //device_work_data.para_type.fault_state = ucMasterRTURcvBuf_2[9];

			//fault_set_bit(FAULT_RESET_WIFI_BIT,ucMasterRTURcvBuf_2[30]&(1<<FAULT_RESET_WIFI_BIT));

//			if(device_work_data.para_type.device_mode == 2)
//				airclean_power_onoff(device_work_data.para_type.device_power_state);


			
			if(ucMasterRTURcvBuf[30]&(1<<FAULT_RESET_WIFI_BIT))
			{
				
				wifi_factory_set();

			}

			for(i=0;i<6;i++)
			{
				
				if(device_work_data_bak.device_data[i] != device_work_data.device_data[i])
				{

					device_sys_para_save();
					break;
				}
				
				
			}

				
        }      
		else if(ucMasterRTURcvBuf[0] == 0xF1 && ucMasterRTURcvBuf[1] == 0xF1 && ucMasterRTURcvBuf[6] == 0x7e)
		{

			if((ucMasterRTURcvBuf[2] <=7) && (ucMasterRTURcvBuf[2] > 1))
			{
			
			set_device_work_mode(ucMasterRTURcvBuf[2],ucMasterRTURcvBuf[4],0);


			}


			
		}
	}

	rt_mutex_release(modbus_mutex);
	
}

static u8 disp_board_packet_data(u8 len)
{

    u8 buftmp[35];

    buftmp[0] = 0x01;
    buftmp[1] = 0x1b;

    u8 i;

    for(i=0;i<27;i++)
        {
        buftmp[2+i] = device_work_data.device_data[i];

    }


    u8 chk;


    rs485_send_buf_not_modbus[0] = 0xF2;
    rs485_send_buf_not_modbus[1] = 0xF2;

    for(i=0;i<len;i++)
    {
        rs485_send_buf_not_modbus[i+2] = buftmp[i];
        chk += buftmp[i];    
    }

    rs485_send_buf_not_modbus[i+2] = chk;
    rs485_send_buf_not_modbus[i+3] = 0x7E;
	
		return 1;
}


static u8 send_packet_data(u8 len,u8 *buftmp)
{


    u8 i;


    u8 chk;


    rs485_send_buf_not_modbus[0] = 0xF1;
    rs485_send_buf_not_modbus[1] = 0xF1;

    for(i=0;i<len;i++)
    {
        rs485_send_buf_not_modbus[i+2] = buftmp[i];
        chk += buftmp[i];    
    }

    rs485_send_buf_not_modbus[i+2] = chk;
    rs485_send_buf_not_modbus[i+3] = 0x7E;
	
		return 1;
}




void set_dispboard_function_mode(enum DEVICE_CMD_TYPE type,u8 mode)
{

#if 1
	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;


	u8 tmpbuf[10];

	
	tmpbuf[0] = (u8)type;
	tmpbuf[0] = 0x01;
	tmpbuf[0] = mode;

	rt_mutex_take(modbus_mutex,RT_WAITING_FOREVER);
	
    send_packet_data(3,tmpbuf);

      ucMasterRTURcvBuf[0] = 0; 
	errorCode = eMBMasterReqRead_not_rtu_datas(rs485_send_buf_not_modbus,7,RT_WAITING_FOREVER);
	if(errorCode == MB_MRE_REV_DATA)
	{
		if(ucMasterRTURcvBuf[0] == 0xF2 && ucMasterRTURcvBuf[1] == 0xF2 && ucMasterRTURcvBuf[32] == 0x7e)
        {
            ;
        }      
	}

	
	rt_mutex_release(modbus_mutex);

	#endif
}



void set_display_board_data(void)
{

#if 1
	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;

	rt_mutex_take(modbus_mutex,RT_WAITING_FOREVER);


    disp_board_packet_data(33-4);

      ucMasterRTURcvBuf[0] = 0; 
	errorCode = eMBMasterReqRead_not_rtu_datas(rs485_send_buf_not_modbus,33,RT_WAITING_FOREVER);
	if(errorCode == MB_MRE_REV_DATA)
	{
		if(ucMasterRTURcvBuf[0] == 0xF2 && ucMasterRTURcvBuf[1] == 0xF2 && ucMasterRTURcvBuf[32] == 0x7e)
        {
            ;
        }      
	}
	
	rt_mutex_release(modbus_mutex);
	#endif
	
}




void thread_disp_board_set_handle(void* parameter)
{

	
	u8 mystate;
	while (1)
	{
		
		rt_thread_delay(RT_TICK_PER_SECOND/10);


	    set_display_board_data(); //100ms
	       


		rt_thread_delay(RT_TICK_PER_SECOND/10);

	}

}

void thread_disp_board_get_handle(void* parameter)
{

	
	u8 mystate;
	while (1)
	{
		
		rt_thread_delay(RT_TICK_PER_SECOND/10);

		get_display_board_data(); //1s
		rt_thread_delay(RT_TICK_PER_SECOND/10);

	}

}



//***************************系统监控线程***************************
//函数定义: void thread_entry_SysRunLed(void* parameter)
//入口参数：无
//出口参数：无
//备    注：Editor：  
//******************************************************************

#if 1
void thread_entry_SysMonitor(void* parameter)
{
	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
    rt_thread_t init_thread;

	init_thread = rt_thread_create("MBMasterPoll",
								   thread_entry_ModbusMasterPoll, RT_NULL,
								   512, 20, 30);
	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);
	

	init_thread = rt_thread_create("disp",
								   thread_disp_board_get_handle, RT_NULL,
								   512, 9, 30);
	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);

	init_thread = rt_thread_create("dispset",
								   thread_disp_board_set_handle, RT_NULL,
								   512, 9, 30);
	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);



	init_thread = rt_thread_create("dcm",
								   thread_dc_motor_set, RT_NULL,
								   512, 20, 300);
	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);


	
	while (1)
	{
		
		rt_thread_delay(RT_TICK_PER_SECOND/10);
		//rt_thread_delay(RT_TICK_PER_SECOND);


#if 1
		for(u8 i=11;i<=15;i++)//1s
		{

		
			rt_mutex_take(modbus_mutex,RT_WAITING_FOREVER);
			
			errorCode = eMBMasterReqReadHoldingRegister(i,0,2,RT_WAITING_FOREVER);
			//errorCode = eMBMasterReqReadHoldingRegister(12,0,2,RT_WAITING_FOREVER);

			if(errorCode == MB_MRE_NO_ERR)
			{	
				u8 kk;

				kk = i-1;
				switch(i)
				{
				case 11:
					device_work_data.para_type.house1_co2 = sw16(usMRegHoldBuf[kk][0]);
					device_work_data.para_type.house1_pm2_5 = sw16(usMRegHoldBuf[kk][1]);
					break;
				case 12:
					device_work_data.para_type.house2_co2 = sw16(usMRegHoldBuf[kk][0]);
					device_work_data.para_type.house2_pm2_5 = sw16(usMRegHoldBuf[kk][1]);
					break;
				case 13:
					device_work_data.para_type.house3_co2 = sw16(usMRegHoldBuf[kk][0]);
					device_work_data.para_type.house3_pm2_5 = sw16(usMRegHoldBuf[kk][1]);
					break;
				case 14:
					device_work_data.para_type.house4_co2 = sw16(usMRegHoldBuf[kk][0]);
					device_work_data.para_type.house4_pm2_5 = sw16(usMRegHoldBuf[kk][1]);
					break;
				case 15:
					device_work_data.para_type.house5_co2 = sw16(usMRegHoldBuf[kk][0]);
					device_work_data.para_type.house5_pm2_5 = sw16(usMRegHoldBuf[kk][1]);
					break;

				default:
					break;
				}		  
				

			}

			rt_mutex_release(modbus_mutex);

			rt_thread_delay(RT_TICK_PER_SECOND/5);
		
		}

#endif

#if 0
			errorCode = eMBMasterReqReadHoldingRegister(1,0,2,RT_WAITING_FOREVER);

			if(errorCode == MB_MRE_NO_ERR)
			{	

					device_work_data.para_type.house1_co2 = sw16(usMRegHoldBuf[0][0]);
					device_work_data.para_type.house1_pm2_5 = sw16(usMRegHoldBuf[0][1]);

					

		}

#endif

		
	}
}

#else
void thread_entry_SysMonitor(void* parameter)
{
	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
	uint16_t errorCount = 0;

	u8 mstate = 0;

    u8 cnttmp = 0;

	u8 mystate = 0;
	
	while (1)
	{
		
		rt_thread_delay(RT_TICK_PER_SECOND/20);

		
		rt_mutex_take(modbus_mutex,RT_WAITING_FOREVER);

		if(mystate)
		{
	        set_display_board_data(); //100ms
			mystate=0;

		}
		else
		{
			mystate=1;

			get_display_board_data(); //1s

		}
#if 1
		rt_thread_delay(RT_TICK_PER_SECOND/20);

        
		set_dc_motor();//1s

		
		//rt_thread_delay(RT_TICK_PER_SECOND/10);

		for(u8 i=11;i<=15;i++)//1s
		{
			errorCode = eMBMasterReqReadHoldingRegister(i,0,2,RT_WAITING_FOREVER);

			if(errorCode == MB_MRE_NO_ERR)
			{	
				u8 kk;

				kk = i-1;
				switch(i)
				{
				case 11:
					device_work_data.para_type.house1_co2 = sw16(usMRegHoldBuf[kk][0]);
					device_work_data.para_type.house1_pm2_5 = sw16(usMRegHoldBuf[kk][1]);
					break;
				case 12:
					device_work_data.para_type.house2_co2 = sw16(usMRegHoldBuf[kk][0]);
					device_work_data.para_type.house2_pm2_5 = sw16(usMRegHoldBuf[kk][1]);
					break;
				case 13:
					device_work_data.para_type.house3_co2 = sw16(usMRegHoldBuf[kk][0]);
					device_work_data.para_type.house3_pm2_5 = sw16(usMRegHoldBuf[kk][1]);
					break;
				case 14:
					device_work_data.para_type.house4_co2 = sw16(usMRegHoldBuf[kk][0]);
					device_work_data.para_type.house4_pm2_5 = sw16(usMRegHoldBuf[kk][1]);
					break;
				case 15:
					device_work_data.para_type.house5_co2 = sw16(usMRegHoldBuf[kk][0]);
					device_work_data.para_type.house5_pm2_5 = sw16(usMRegHoldBuf[kk][1]);
					break;

				default:
					break;
				}		  
				

			}




		rt_thread_delay(RT_TICK_PER_SECOND/100);
		
        
	}

#if 0
			errorCode = eMBMasterReqReadHoldingRegister(1,0,2,RT_WAITING_FOREVER);

			if(errorCode == MB_MRE_NO_ERR)
			{	

					device_work_data.para_type.house1_co2 = sw16(usMRegHoldBuf[0][0]);
					device_work_data.para_type.house1_pm2_5 = sw16(usMRegHoldBuf[0][1]);

					

		}

#endif

	rt_mutex_release(modbus_mutex);

//		if(device_work_data.para_type.device_power_state  == 0)
//		{
//			if(device_power_state_pre==0xff)
//			{
//				device_power_state_pre = device_work_data.para_type.device_power_state;
//
//				FLASH_Program_save_para();
//			}
//			else if(device_power_state_pre != (device_work_data.para_type.device_power_state))
//			{
//
//				device_power_state_pre = device_work_data.para_type.device_power_state;
//
//				FLASH_Program_save_para();
//			}
//				
//		}
		
#endif		
}
}
#endif



//************************ Modbus主机轮训线程***************************
//函数定义: void thread_entry_ModbusMasterPoll(void* parameter)
//入口参数：无
//出口参数：无
//备    注：Editor：Armink   2013-08-28    Company: BXXJS
//******************************************************************
void thread_entry_ModbusMasterPoll(void* parameter)
{
	eMBMasterInit(MB_RTU, 2, 9600,  MB_PAR_NONE);
	eMBMasterEnable();
        extern struct rt_serial_device serial1;

	
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


	
	//device_work_data.para_type.high_pressur_state = mode;
}


//0,off; 1,  ;2,  ;3,speed
void ac_ac_motor_set(u8 mode)
{
	if(mode == 1)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_14);
		
		GPIO_ResetBits(GPIOB,GPIO_Pin_12);
		
		GPIO_ResetBits(GPIOB,GPIO_Pin_13);
	}
    else if(mode == 3)
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_14);

		GPIO_SetBits(GPIOB,GPIO_Pin_12);
		
		GPIO_ResetBits(GPIOB,GPIO_Pin_13);
	}
    else if(mode == 2)
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_14);

		GPIO_ResetBits(GPIOB,GPIO_Pin_12);
		
		GPIO_SetBits(GPIOB,GPIO_Pin_13);
	}
	else
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_14);

		GPIO_ResetBits(GPIOB,GPIO_Pin_12);
		
		GPIO_ResetBits(GPIOB,GPIO_Pin_13);
	}

//device_work_data.para_type.wind_speed_state = mode;

}

//0,off; 1,on
void ac_pht_set(u8 mode)
{
	if(mode>1)
		mode = 1;
	if(mode)
		GPIO_SetBits(GPIOB,GPIO_Pin_15);
	else
		GPIO_ResetBits(GPIOB,GPIO_Pin_15);

	//device_work_data.para_type.pht_work_state = mode;

}

void ac_high_presure_set(u8 mode)
{
    
}

//mode ,1,auto mode; 2,manual mode
void ac_workmode_set(u8 mode)
{
    if(mode > 2)
        return;
    if(mode == 0)
        return;
    
}

DEVICE_WORK_TYPE device_work_data_bak;

u8 set_device_work_mode(u8 type,u8 data,u8 signal_ch)
{

//02开关机，03模式2-手动1-自动，04高压，05光氢，06定时，07风速

	u8 i;


    if((type != 0x02) && (type != 0x03))
    {// 
        if ((device_work_data.para_type.device_mode == 1) || (device_work_data.para_type.device_power_state == 0))
            return 1;

    }   //关机、智能模式不设置数据wyh 0314
    
	for(i=0;i<sizeof(struct __para_type);i++)
	{
		
		device_work_data_bak.device_data[i] = device_work_data.device_data[i];
		
		
	}

	
//	if(device_work_data.para_type.device_mode == 1)
//		return 1;




    
    switch(type)
        {
    case 0x02:
        if(data)
        {    device_work_data.para_type.device_power_state = 1;
            airclean_power_onoff(1);
		}
        else
        {    device_work_data.para_type.device_power_state = 0;
            airclean_power_onoff(0);
        }
        break;
    case 0x03:
        if(data==1||data==2)//1,auto;2,manual
            device_work_data.para_type.device_mode = data;
         
        break;
    case 0x04:
        if(data)
            device_work_data.para_type.high_pressur_state = 1;
        else
            device_work_data.para_type.high_pressur_state = 0;

        
        break;
    case 0x05:
        if(data)
            device_work_data.para_type.pht_work_state = 1;
        else
            device_work_data.para_type.pht_work_state = 0;

        ac_pht_set(data);
 
        break;
    case 0x06:
        if(data<=0x0c)
            device_work_data.para_type.timing_state = data;
        else
            device_work_data.para_type.timing_state = 0;

        
        break;
    case 0x07:
        if(data<=3)
            device_work_data.para_type.wind_speed_state = data;
        set_dc_motor_speed(data);

		ac_ac_motor_set(data);
        break;

    default:

		return 1;

    }

	if(signal_ch == 1)//wifi
		set_dispboard_function_mode(type,data);



	for(i=0;i<6;i++)
	{
		
		if(device_work_data_bak.device_data[i] != device_work_data.device_data[i])
		{

			device_sys_para_save();
			break;
		}
		
		
	}
	
	return 1;
}

//
void airclean_motor_set(u8 mode)
{
	rt_mutex_take(motor_mutex,RT_WAITING_FOREVER);	
	device_work_data.para_type.wind_speed_state = mode;

	ac_ac_motor_set(device_work_data.para_type.wind_speed_state);
    
	
	rt_mutex_take(modbus_mutex,RT_WAITING_FOREVER);
	
	set_dc_motor_speed(device_work_data.para_type.wind_speed_state); //100ms
	rt_mutex_release(modbus_mutex);

	rt_mutex_release(motor_mutex);
	
}

u8 fault_get_bit(u8 fault_type) 
{
    //device_work_data.para_type.fault_state |= 1<<fault_type;
    
	return (device_work_data.para_type.fault_state & (1<<fault_type));
}



#define PM2_5_LEVEL1_MIN    96
#define CO2_LEVEL1_MIN      1300

#define PM2_5_LEVEL2_MIN    134
#define CO2_LEVEL2_MIN      2500

#define PM2_5_LEVEL1_MAX    55
#define CO2_LEVEL1_MAX      730

#define PM2_5_LEVEL2_MAX    (96-5)
#define CO2_LEVEL2_MAX      (1300-10)


#if 0
void airclean_work_auto_handle_thread(void * parameter)
{

    while(1)
    {
        if(device_work_data.para_type.device_mode == 1)
        {//auto
            if(device_work_data.para_type.house1_pm2_5 > 96 || 
                device_work_data.para_type.house1_co2 > 1300)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(device_work_data.para_type.house2_pm2_5 > 96 || 
                device_work_data.para_type.house2_co2 > 1300)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(device_work_data.para_type.house3_pm2_5 > 96 || 
                device_work_data.para_type.house3_co2 > 1300)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(device_work_data.para_type.house4_pm2_5 > 96 || 
                device_work_data.para_type.house4_co2 > 1300)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(device_work_data.para_type.house5_pm2_5 > 96 || 
                device_work_data.para_type.house5_co2 > 1300)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);
                goto LABEL_AUTO_AC_CONTINUE;
            }


///////////  2
            if(device_work_data.para_type.house1_pm2_5 > PM2_5_LEVEL2_MIN || 
                device_work_data.para_type.house1_co2 > CO2_LEVEL2_MIN)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(1);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(device_work_data.para_type.house2_pm2_5 > PM2_5_LEVEL2_MIN || 
                device_work_data.para_type.house2_co2 > CO2_LEVEL2_MIN)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(1);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(device_work_data.para_type.house3_pm2_5 > PM2_5_LEVEL2_MIN || 
                device_work_data.para_type.house3_co2 > CO2_LEVEL2_MIN)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(1);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(device_work_data.para_type.house4_pm2_5 > PM2_5_LEVEL2_MIN || 
                device_work_data.para_type.house4_co2 > CO2_LEVEL2_MIN)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(1);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(device_work_data.para_type.house5_pm2_5 > PM2_5_LEVEL2_MIN || 
                device_work_data.para_type.house5_co2 > CO2_LEVEL2_MIN)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(1);
                goto LABEL_AUTO_AC_CONTINUE;
            }

 ///////////  3
            if(device_work_data.para_type.house1_pm2_5 < PM2_5_LEVEL1_MAX || 
                device_work_data.para_type.house1_co2 < CO2_LEVEL1_MAX)
            {
                ac_esd_set(0);
                ac_pht_set(0);
                airclean_motor_set(0);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(device_work_data.para_type.house2_pm2_5 < PM2_5_LEVEL1_MAX || 
                device_work_data.para_type.house2_co2 < CO2_LEVEL1_MAX)
            {
                ac_esd_set(0);
                ac_pht_set(0);
                airclean_motor_set(0);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(device_work_data.para_type.house3_pm2_5 < PM2_5_LEVEL1_MAX || 
                device_work_data.para_type.house3_co2 < CO2_LEVEL1_MAX)
            {
                ac_esd_set(0);
                ac_pht_set(0);
                airclean_motor_set(0);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(device_work_data.para_type.house4_pm2_5 <PM2_5_LEVEL1_MAX || 
                device_work_data.para_type.house4_co2 <CO2_LEVEL1_MAX)
            {
                ac_esd_set(0);
                ac_pht_set(0);
                airclean_motor_set(0);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(device_work_data.para_type.house5_pm2_5 <PM2_5_LEVEL1_MAX || 
                device_work_data.para_type.house5_co2 <CO2_LEVEL1_MAX)
            {
                ac_esd_set(0);
                ac_pht_set(0);
                airclean_motor_set(0);

               goto LABEL_AUTO_AC_CONTINUE;
            }

 /////////// 4
            if(device_work_data.para_type.house1_pm2_5 < PM2_5_LEVEL2_MAX || 
                device_work_data.para_type.house1_co2 < CO2_LEVEL2_MAX)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);
               goto LABEL_AUTO_AC_CONTINUE;

            }
            else if(device_work_data.para_type.house2_pm2_5 < PM2_5_LEVEL2_MAX || 
                device_work_data.para_type.house2_co2 < CO2_LEVEL2_MAX)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);

                               goto LABEL_AUTO_AC_CONTINUE;

            }
            else if(device_work_data.para_type.house3_pm2_5 < PM2_5_LEVEL2_MAX || 
                device_work_data.para_type.house3_co2 < CO2_LEVEL2_MAX)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);

                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(device_work_data.para_type.house4_pm2_5 <PM2_5_LEVEL2_MAX || 
                device_work_data.para_type.house4_co2 <CO2_LEVEL2_MAX)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);

               goto LABEL_AUTO_AC_CONTINUE;

            }
            else if(device_work_data.para_type.house5_pm2_5 <PM2_5_LEVEL2_MAX || 
                device_work_data.para_type.house5_co2 <CO2_LEVEL2_MAX)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);

               goto LABEL_AUTO_AC_CONTINUE;
            }




            
        }
        else if(device_work_data.para_type.device_mode == 2)
        {//manual
        
			ac_esd_set(device_work_data.para_type.high_pressur_state);
			ac_pht_set(device_work_data.para_type.pht_work_state);
			airclean_motor_set(device_work_data.para_type.wind_speed_state);
	            

        }
		else if(device_work_data.para_type.device_mode == 0)
        {//manual

			
                ac_esd_set(device_work_data.para_type.high_pressur_state);
                ac_pht_set(device_work_data.para_type.pht_work_state);
                airclean_motor_set(device_work_data.para_type.wind_speed_state);
          
        }

    
//		if(fault_get_bit(FAULT_RESET_WIFI_BIT))
//		{
//			
//			wifi_factory_set();
//			fault_set_bit(FAULT_RESET_WIFI_BIT,0);
//		}
		
LABEL_AUTO_AC_CONTINUE:
        rt_thread_delay(RT_TICK_PER_SECOND/20);
    }

}
#endif



extern DEVICE_WORK_TYPE device_work_data_auto;

#define	HOUSE_CO2_NUM_LINK(ID)	device_work_data.para_type.house##ID##_co2
#define	HOUSE_PM25_NUM_LINK(ID)	device_work_data.para_type.house##ID##_pm2_5


#define	byte_2_to_short(x,y)	((short)(\
      (((short)(x) &(short)0x00ffU) << 8) |\
      (((short)(y) &(short)0x00ffU))))



#define	HOUSE_DATA_START		6
#if 1
void airclean_work_auto_handle(void)
{

	u8 i;
	u16 co2_tmp,pm25_tmp;
	u8 device_work_data_pre = 0xff;


	if(device_work_data.para_type.device_power_state)
    {
        if(device_work_data.para_type.device_mode == 1)
        {//auto

  
 //////////////////  1
			 for(i = 0;i<5;i++)
			 {
				 pm25_tmp = byte_2_to_short((device_work_data.device_data[HOUSE_DATA_START+i*4]),(device_work_data.device_data[HOUSE_DATA_START+i*4+1]));
				 co2_tmp = byte_2_to_short((device_work_data.device_data[HOUSE_DATA_START+i*4+2]),(device_work_data.device_data[HOUSE_DATA_START+i*4+3]));
			 
				 if(co2_tmp!=0 ||pm25_tmp!=0)
				 {
					 if(pm25_tmp > 134 || co2_tmp > 2500)
					 {
                        ac_esd_set(1);
                        ac_pht_set(1);
                        airclean_motor_set(3);

                        device_work_data_auto.para_type.high_pressur_state = 1;
            			(device_work_data_auto.para_type.pht_work_state) = 1;
            			(device_work_data_auto.para_type.wind_speed_state) = 3;
            	            
		device_work_data_pre = 0;
						 goto LABEL_AUTO_AC_CONTINUE;
					 }
			 
				 }
				 
			 
			 }

 //////////////////  2
			 for(i = 0;i<5;i++)
			 {
				 pm25_tmp = byte_2_to_short((device_work_data.device_data[HOUSE_DATA_START+i*4]),(device_work_data.device_data[HOUSE_DATA_START+i*4+1]));
				 co2_tmp = byte_2_to_short((device_work_data.device_data[HOUSE_DATA_START+i*4+2]),(device_work_data.device_data[HOUSE_DATA_START+i*4+3]));
			 
				 if(co2_tmp!=0 ||pm25_tmp!=0)
				 {
					 if(pm25_tmp > 96 || co2_tmp > 1300)
					 {
						 ac_esd_set(1);
						 ac_pht_set(1);
						 airclean_motor_set(2);
                         device_work_data_auto.para_type.high_pressur_state = 1;
            			(device_work_data_auto.para_type.pht_work_state) = 1;
            			(device_work_data_auto.para_type.wind_speed_state) = 2;
		device_work_data_pre = 0;
						 goto LABEL_AUTO_AC_CONTINUE;
					 }
			 
				 }
				 
			 
			 }


 ///////////  3

			for(i = 0;i<5;i++)
			{
				pm25_tmp = byte_2_to_short((device_work_data.device_data[HOUSE_DATA_START+i*4]),(device_work_data.device_data[HOUSE_DATA_START+i*4+1]));
				co2_tmp = byte_2_to_short((device_work_data.device_data[HOUSE_DATA_START+i*4+2]),(device_work_data.device_data[HOUSE_DATA_START+i*4+3]));

				if(co2_tmp!=0 ||pm25_tmp!=0)
				{
					if(pm25_tmp < PM2_5_LEVEL1_MAX && co2_tmp < CO2_LEVEL1_MAX)
		            {

				
						ac_esd_set(0);
						ac_pht_set(0);
						airclean_motor_set(0);
                        device_work_data_auto.para_type.high_pressur_state = 0;
            			(device_work_data_auto.para_type.pht_work_state) = 0;
            			(device_work_data_auto.para_type.wind_speed_state) = 0;
		device_work_data_pre = 0;
						goto LABEL_AUTO_AC_CONTINUE;
		            }

				}
				

			}
				

 /////////// 4
 
			for(i = 0;i<5;i++)
			{
				pm25_tmp = byte_2_to_short((device_work_data.device_data[HOUSE_DATA_START+i*4]),(device_work_data.device_data[HOUSE_DATA_START+i*4+1]));
				co2_tmp = byte_2_to_short((device_work_data.device_data[HOUSE_DATA_START+i*4+2]),(device_work_data.device_data[HOUSE_DATA_START+i*4+3]));

				if(co2_tmp!=0 ||pm25_tmp!=0)
				{
					if(pm25_tmp < PM2_5_LEVEL2_MAX && co2_tmp < CO2_LEVEL2_MAX)
		            {
						ac_esd_set(1);
						ac_pht_set(1);
						airclean_motor_set(1);
//wyh
                        	device_work_data_auto.para_type.high_pressur_state = 1;
            			(device_work_data_auto.para_type.pht_work_state) = 1;
            			(device_work_data_auto.para_type.wind_speed_state) = 1;
								device_work_data_pre = 0;
						goto LABEL_AUTO_AC_CONTINUE;
		            }

				}
				

			}
		ac_esd_set(0);
		ac_pht_set(0);
		airclean_motor_set(0);
                        	device_work_data_auto.para_type.high_pressur_state = 0;
            			(device_work_data_auto.para_type.pht_work_state) = 0;
            			(device_work_data_auto.para_type.wind_speed_state) = 0;
		device_work_data_pre = 0;
		goto LABEL_AUTO_AC_CONTINUE;	
            
        }
        else if(device_work_data.para_type.device_mode == 2 )
        {//manual
        		if ( device_work_data_pre == 0)  //标记从自动切换成手动的状态 //wyh 0314
        			{
			ac_esd_set(1);
			ac_pht_set(1);
			airclean_motor_set(1);
			device_work_data_pre = 0xff;
        			} else 
        				{
        				ac_esd_set(device_work_data.para_type.high_pressur_state);
					ac_pht_set(device_work_data.para_type.pht_work_state);
					if 	(device_work_data.para_type.wind_speed_state == 0)airclean_motor_set(1);//wyh 0313
					airclean_motor_set(device_work_data.para_type.wind_speed_state);
					
        				}

        }
	/*
		else if(device_work_data.para_type.device_mode == 0) //wyh 0 
        {//auto

			
                ac_esd_set(device_work_data.para_type.high_pressur_state);
                ac_pht_set(device_work_data.para_type.pht_work_state);
                airclean_motor_set(device_work_data.para_type.wind_speed_state);
          
        }
        */

    
//		if(fault_get_bit(FAULT_RESET_WIFI_BIT))
//		{
//			
//			wifi_factory_set();
//			fault_set_bit(FAULT_RESET_WIFI_BIT,0);
//		}
		
LABEL_AUTO_AC_CONTINUE:
       rt_thread_delay(RT_TICK_PER_SECOND/20);
	
    }

}


#else
void airclean_work_auto_handle(void)
{

	if(device_work_data.para_type.device_power_state)
    {
        if(device_work_data.para_type.device_mode == 1)
        {//auto

#if 0
		if(HOUSE_CO2_NUM_LINK(1) == 0 && HOUSE_PM25_NUM_LINK(1)==0)
				goto LABEL_AUTO_AC_CONTINUE;


			if(HOUSE_CO2_NUM_LINK(2) == 0 && HOUSE_PM25_NUM_LINK(2)==0)
				goto LABEL_AUTO_AC_CONTINUE;


			if(HOUSE_CO2_NUM_LINK(3) == 0 && HOUSE_PM25_NUM_LINK(3)==0)
				goto LABEL_AUTO_AC_CONTINUE;


			if(HOUSE_CO2_NUM_LINK(4) == 0 && HOUSE_PM25_NUM_LINK(4)==0)
				goto LABEL_AUTO_AC_CONTINUE;


			if(HOUSE_CO2_NUM_LINK(5) == 0 && HOUSE_PM25_NUM_LINK(5)==0)
				goto LABEL_AUTO_AC_CONTINUE;



#endif			


		
            if(exchange_16bit_hi_lo(device_work_data.para_type.house1_pm2_5) > 96 || 
                exchange_16bit_hi_lo(device_work_data.para_type.house1_co2) > 1300)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house2_pm2_5) > 96 || 
                exchange_16bit_hi_lo(device_work_data.para_type.house2_co2) > 1300)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house3_pm2_5) > 96 || 
                exchange_16bit_hi_lo(device_work_data.para_type.house3_co2) > 1300)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house4_pm2_5) > 96 || 
                exchange_16bit_hi_lo(device_work_data.para_type.house4_co2) > 1300)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house5_pm2_5) > 96 || 
                exchange_16bit_hi_lo(device_work_data.para_type.house5_co2) > 1300)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);
                goto LABEL_AUTO_AC_CONTINUE;
            }


///////////  2
            if(exchange_16bit_hi_lo(device_work_data.para_type.house1_pm2_5) > PM2_5_LEVEL2_MIN || 
                exchange_16bit_hi_lo(device_work_data.para_type.house1_co2) > CO2_LEVEL2_MIN)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(1);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house2_pm2_5) > PM2_5_LEVEL2_MIN || 
                exchange_16bit_hi_lo(device_work_data.para_type.house2_co2) > CO2_LEVEL2_MIN)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(1);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house3_pm2_5) > PM2_5_LEVEL2_MIN || 
                exchange_16bit_hi_lo(device_work_data.para_type.house3_co2) > CO2_LEVEL2_MIN)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(1);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house4_pm2_5) > PM2_5_LEVEL2_MIN || 
                exchange_16bit_hi_lo(device_work_data.para_type.house4_co2) > CO2_LEVEL2_MIN)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(1);
                goto LABEL_AUTO_AC_CONTINUE;
            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house5_pm2_5) > PM2_5_LEVEL2_MIN || 
                exchange_16bit_hi_lo(device_work_data.para_type.house5_co2) > CO2_LEVEL2_MIN)
            {
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(1);
                goto LABEL_AUTO_AC_CONTINUE;
            }

 ///////////  3
            if(exchange_16bit_hi_lo(device_work_data.para_type.house1_pm2_5) < PM2_5_LEVEL1_MAX || 
                exchange_16bit_hi_lo(device_work_data.para_type.house1_co2) < CO2_LEVEL1_MAX)
            {

			
				if(HOUSE_CO2_NUM_LINK(1) != 0 || HOUSE_PM25_NUM_LINK(1)!=0)
				{ 
					ac_esd_set(0);
					ac_pht_set(0);
					airclean_motor_set(0);
					goto LABEL_AUTO_AC_CONTINUE;
				}
            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house2_pm2_5) < PM2_5_LEVEL1_MAX || 
                exchange_16bit_hi_lo(device_work_data.para_type.house2_co2) < CO2_LEVEL1_MAX)
            {

							if(HOUSE_CO2_NUM_LINK(2) != 0 || HOUSE_PM25_NUM_LINK(2)!=0)
				{ 
                ac_esd_set(0);
                ac_pht_set(0);
                airclean_motor_set(0);
                goto LABEL_AUTO_AC_CONTINUE;
								}
            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house3_pm2_5) < PM2_5_LEVEL1_MAX || 
                exchange_16bit_hi_lo(device_work_data.para_type.house3_co2) < CO2_LEVEL1_MAX)
            {

				if(HOUSE_CO2_NUM_LINK(3) != 0 || HOUSE_PM25_NUM_LINK(3)!=0)
				{ 
                ac_esd_set(0);
                ac_pht_set(0);
                airclean_motor_set(0);
                goto LABEL_AUTO_AC_CONTINUE;

									}
            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house4_pm2_5) <PM2_5_LEVEL1_MAX || 
                exchange_16bit_hi_lo(device_work_data.para_type.house4_co2) <CO2_LEVEL1_MAX)
            {

							if(HOUSE_CO2_NUM_LINK(4) != 0 || HOUSE_PM25_NUM_LINK(4)!=0)
				{ 
                ac_esd_set(0);
                ac_pht_set(0);
                airclean_motor_set(0);
                goto LABEL_AUTO_AC_CONTINUE;

								}
            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house5_pm2_5) <PM2_5_LEVEL1_MAX || 
                exchange_16bit_hi_lo(device_work_data.para_type.house5_co2) <CO2_LEVEL1_MAX)
            {

				if(HOUSE_CO2_NUM_LINK(5) != 0 || HOUSE_PM25_NUM_LINK(5)!=0)
				{ 

				ac_esd_set(0);
                ac_pht_set(0);
                airclean_motor_set(0);

               goto LABEL_AUTO_AC_CONTINUE;
					}
            }

 /////////// 4
            if(exchange_16bit_hi_lo(device_work_data.para_type.house1_pm2_5) < PM2_5_LEVEL2_MAX || 
                exchange_16bit_hi_lo(device_work_data.para_type.house1_co2) < CO2_LEVEL2_MAX)
            {

							if(HOUSE_CO2_NUM_LINK(1) != 0 || HOUSE_PM25_NUM_LINK(1)!=0)
				{ 
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);
               goto LABEL_AUTO_AC_CONTINUE;}

            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house2_pm2_5) < PM2_5_LEVEL2_MAX || 
                exchange_16bit_hi_lo(device_work_data.para_type.house2_co2) < CO2_LEVEL2_MAX)
            {
            				if(HOUSE_CO2_NUM_LINK(2) != 0 || HOUSE_PM25_NUM_LINK(2)!=0)
				{ 
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);

                               goto LABEL_AUTO_AC_CONTINUE;}

            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house3_pm2_5) < PM2_5_LEVEL2_MAX || 
                exchange_16bit_hi_lo(device_work_data.para_type.house3_co2) < CO2_LEVEL2_MAX)
            {
            				if(HOUSE_CO2_NUM_LINK(3) != 0 || HOUSE_PM25_NUM_LINK(3)!=0)
				{ 
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);

                goto LABEL_AUTO_AC_CONTINUE;}
            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house4_pm2_5) <PM2_5_LEVEL2_MAX || 
                exchange_16bit_hi_lo(device_work_data.para_type.house4_co2) <CO2_LEVEL2_MAX)
            {
            				if(HOUSE_CO2_NUM_LINK(4) != 0 || HOUSE_PM25_NUM_LINK(4)!=0)
				{ 
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);

               goto LABEL_AUTO_AC_CONTINUE;}

            }
            else if(exchange_16bit_hi_lo(device_work_data.para_type.house5_pm2_5) <PM2_5_LEVEL2_MAX || 
                exchange_16bit_hi_lo(device_work_data.para_type.house5_co2) <CO2_LEVEL2_MAX)
            {
            				if(HOUSE_CO2_NUM_LINK(5) != 0 || HOUSE_PM25_NUM_LINK(5)!=0)
				{ 
                ac_esd_set(1);
                ac_pht_set(1);
                airclean_motor_set(2);

               goto LABEL_AUTO_AC_CONTINUE;}
            }




            
        }
        else if(device_work_data.para_type.device_mode == 2)
        {//manual
        
			ac_esd_set(device_work_data.para_type.high_pressur_state);
			ac_pht_set(device_work_data.para_type.pht_work_state);
			airclean_motor_set(device_work_data.para_type.wind_speed_state);
	            

        }
		else if(device_work_data.para_type.device_mode == 0)
        {//manual

			
                ac_esd_set(device_work_data.para_type.high_pressur_state);
                ac_pht_set(device_work_data.para_type.pht_work_state);
                airclean_motor_set(device_work_data.para_type.wind_speed_state);
          
        }

    
//		if(fault_get_bit(FAULT_RESET_WIFI_BIT))
//		{
//			
//			wifi_factory_set();
//			fault_set_bit(FAULT_RESET_WIFI_BIT,0);
//		}
		
LABEL_AUTO_AC_CONTINUE:
       rt_thread_delay(RT_TICK_PER_SECOND/20);
    }

}

#endif

void airclean_out_pin_init(void)
{
	GPIO_InitTypeDef GPIOA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* configure the rs485 control pin of the receive or send */
	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_15|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIOA_InitStructure);

	ac_esd_set(1);
	ac_pht_set(1);
	ac_ac_motor_set(1);

	

}


//return 1,fault; 0,normal
u8 motor_state_get(u8 mode)
{
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11))
		return 0;
	else
		return 1;

}

//return 1,fault; 0,normal
u8 wind_state_get(u8 mode)
{
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10))
	{
		if(wind_state_get(1))
			return 1;
		else
			return 0;

	}
	else
		return 1;

}


u8 pht_state_get(u8 mode)
{
	if( (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2)))
		return 0;
	else
		return 1;

}


u8 clean_state_get(u8 mode)
{
	if( (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)))
		return 0;
	else
		return 1;

}



u8 esd_state_get(u8 mode)
{
	if (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5))
		return 0;
	else
		return 1;

}



u8 run_state_get(u8 mode)
{
	if (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4))
		return 0;
	else
		return 1;

}



void airclean_in_pin_init(void)
{
	GPIO_InitTypeDef GPIOA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_2|GPIO_Pin_0|GPIO_Pin_10;
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIOA_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIOA_InitStructure);


	
}


//mode,0,off; 1,on
void airclean_power_onoff(u8 mode)
{
    if(mode>1)
        return;

    
    if(mode)
    {//on

	 device_sys_para_get();
	 
        ac_esd_set(device_work_data.para_type.high_pressur_state);
        ac_pht_set(device_work_data.para_type.pht_work_state);
        ac_ac_motor_set(device_work_data.para_type.wind_speed_state);
        
    }
    else
    {//off

		device_sys_para_save();
        ac_esd_set(0);
        ac_pht_set(0);
        ac_ac_motor_set(0);
    }
    

	if(mode)
    {   
        device_work_data.para_type.device_power_state = 1;
	}
    else
    {    
	    device_work_data.para_type.device_power_state = 0;

    }
}


void airclean_system_init(void)
{

	airclean_in_pin_init();
	airclean_out_pin_init();


}


void fault_set_bit(u8 fault_type,u8 val) 
{
    if(val)
        device_work_data.para_type.fault_state |= 1<<fault_type;
    else
        device_work_data.para_type.fault_state &= ~(1<<fault_type);
    

}


u8 fault_state_pre = 0xff;
void rt_check_ex_device_thread_entry(void* parameter)
{

	while(1)
	{
		//0,表示正常,1表示故障

		//device_work_data.para_type.fault_state  = motor_state_get(1);//bit7

        if(device_work_data.para_type.device_power_state == 1)
        {
            if(device_work_data.para_type.pht_work_state)
            {
        		fault_set_bit(FAULT_PHT_BIT,pht_state_get(1));

            }

            fault_set_bit(FAULT_MOTOR_BIT,motor_state_get(1));
            fault_set_bit(FAULT_ESD_BIT,esd_state_get(1));
            fault_set_bit(FAULT_RUN_BIT,run_state_get(1));
            fault_set_bit(FAULT_CLEAN_BIT,clean_state_get(1));
    		
    		
    		//fault_set_bit(FAULT_WIND_BIT,wind_state_get(1));


    		if(fault_state_pre==0xff || fault_state_pre!= device_work_data.para_type.fault_state)
    		{
    			if(device_work_data.para_type.fault_state)
    			{

        			set_display_board_data(); //100ms
        			

    			}
    			fault_state_pre = device_work_data.para_type.fault_state;
    		}

        }
        else
        {
            device_work_data.para_type.fault_state = 0;

        }
		rt_thread_delay(RT_TICK_PER_SECOND/5);

	}

}



void rt_main_thread_entry(void* parameter)
{
    rt_thread_t init_thread;


	airclean_system_init();



	SPI_FLASH_Init();

	device_state_init();

	if(device_work_data.para_type.device_mode == 0)
	{	
		device_work_data.para_type.device_mode = 2;
		device_work_data.para_type.high_pressur_state = 1;
		device_work_data.para_type.pht_work_state = 1;
		ac_esd_set(1);
		ac_pht_set(1);
		ac_ac_motor_set(1);
	} //读取数据为0时重新初始化；//wyh 0313



	
//    init_thread = rt_thread_create("com_disp",
//                                   thread_entry_com_displayboard, RT_NULL,
//                                   1024, 11, 50);
//    if (init_thread != RT_NULL)
//        rt_thread_startup(init_thread);
	

	rt_thread_delay(RT_TICK_PER_SECOND/2);


	//set_display_board_data(); //100ms


    init_thread = rt_thread_create("SysMonitor",
                                   thread_entry_SysMonitor, RT_NULL,
                                   1024, 11, 50);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);
			

			


	rt_thread_delay(RT_TICK_PER_SECOND);
	//rt_key_ctl_init();
	
	//rt_adc_ctl_init();
	
	
    	wifi_comm_init();

    	
    init_thread = rt_thread_create("work",
                                   rt_check_ex_device_thread_entry, RT_NULL,
                                   256, 6, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);


	
//    init_thread = rt_thread_create("auto",
//                                   airclean_work_auto_handle_thread, RT_NULL,
//                                   256, 6, 5);
//    if (init_thread != RT_NULL)
//        rt_thread_startup(init_thread);



}


u8 power_state_pre = 0xff;

void thread_entry_power_monitor (void* parameter)
{

    while(1)
    {

		if(power_state_pre==0xff)
		{
			if(device_work_data.para_type.timing_state)
			{
				
				power_tim_cnt=0;
				power_state_pre = device_work_data.para_type.timing_state;

				
			}

		}
		else
		{
			if(device_work_data.para_type.timing_state)
			{
				if(device_work_data.para_type.timing_state != power_state_pre)
					{
				power_tim_cnt=0;
				power_state_pre = device_work_data.para_type.timing_state;


					}
			}

		}
		
		power_state_pre = device_work_data.para_type.timing_state; 
		
        if(power_tim_cnt < 0xFFFFFFFF)
            power_tim_cnt++;

        //if(power_tim_cnt > (device_work_data.para_type.timing_state*10*60*60)) // hour

//		if(power_tim_cnt == ((u32)device_work_data.para_type.timing_state*0.01*60*60) && (device_work_data.para_type.timing_state) )
		if(power_tim_cnt == (0.01*60*60) && (power_state_pre) )
		{
			//power_tim_cnt = 0; //? power_tim_cnt == 0
			device_work_data.para_type.timing_state -= 1;
			
		}
		
//            if(power_tim_cnt > ((u32)device_work_data.para_type.timing_state*0.01*60*60) && (device_work_data.para_type.timing_state))// half minute
              if(power_tim_cnt > ((u32)device_work_data.para_type.timing_state*0.01*60*60) && (power_state_pre))// half minute
        {
//        	if(device_work_data.para_type.timing_state)
 		if(power_state_pre)
            {	
				device_work_data.para_type.device_power_state = 0;
				//wyh
				device_work_data.para_type.timing_state = 0 ;
				power_tim_cnt = 0; 
				power_state_pre = 0xff;

				set_dispboard_function_mode(D_CMD_POWER,0);
					
				airclean_power_onoff(0);
	            //power_tim_cnt = 0;

				
    		}
        }
        else
        {
        
			airclean_work_auto_handle();
        }
        rt_thread_delay(RT_TICK_PER_SECOND/10);


		
    }
}


int rt_application_init(void)
{
    rt_thread_t init_thread;


	modbus_mutex = rt_mutex_create("mdbusmut",RT_IPC_FLAG_FIFO);
	motor_mutex = rt_mutex_create("motor_set",RT_IPC_FLAG_FIFO);


	init_thread = rt_thread_create("mY",
                                   rt_main_thread_entry, RT_NULL,
                                   1024, 6, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);


#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   1024, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif

    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

		


	init_thread = rt_thread_create("power",
								   thread_entry_power_monitor, RT_NULL,
								   512, 9, 50);
	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);
					
			
    return 0;
}


/*@}*/
