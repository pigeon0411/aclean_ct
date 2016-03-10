
#include "user_mb_app_2.h"
#include "wifi_mod_uart.h"


#define MB_SER_PDU_SIZE_MAX     256     /*!< Maximum size of a Modbus RTU frame. */

extern volatile UCHAR	ucMasterRTURcvBuf_2[MB_SER_PDU_SIZE_MAX];
extern DEVICE_WORK_TYPE device_work_data;
extern DEVICE_WORK_TYPE device_work_data_bak;



extern eMBMasterReqErrCode	eMBMasterReqRead_not_rtu_datas_2(UCHAR *ucMBFrame, USHORT usLength, LONG lTimeOut ); //���ͷ�MODBUS��ʽ������


static u8 rs485_send_buf_not_modbus[50];









static void get_display_board_data(void)
{
	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
	u8 i;
	static u8 device_power_state_bak = 0xff;
	
    rs485_send_buf_not_modbus[0] = 0xF1;
    rs485_send_buf_not_modbus[1] = 0xF1;
    rs485_send_buf_not_modbus[2] = 0x01;
    rs485_send_buf_not_modbus[3] = 0x01;
    rs485_send_buf_not_modbus[4] = 0x00;
    rs485_send_buf_not_modbus[5] = 0x02;
    rs485_send_buf_not_modbus[6] = 0x7E;

    ucMasterRTURcvBuf_2[0] = 0;
	errorCode = eMBMasterReqRead_not_rtu_datas_2(rs485_send_buf_not_modbus,7,RT_WAITING_FOREVER);
	if(errorCode == MB_MRE_REV_DATA)
	{
		if(ucMasterRTURcvBuf_2[0] == 0xF2 && ucMasterRTURcvBuf_2[1] == 0xF2 && ucMasterRTURcvBuf_2[32] == 0x7e)
        {

        
    		for(i=0;i<sizeof(struct __para_type);i++)
			{
				
				device_work_data_bak.device_data[i] = device_work_data.device_data[i];
				
				
			}


			
            //device_work_data.para_type.device_power_state = ucMasterRTURcvBuf_2[4];
            device_work_data.para_type.device_mode = ucMasterRTURcvBuf_2[5];

            if(device_work_data.para_type.device_mode == 1)
            {

                ;
            }
            else
            {

                device_work_data.para_type.wind_speed_state = ucMasterRTURcvBuf_2[6];
                device_work_data.para_type.high_pressur_state = ucMasterRTURcvBuf_2[7];
                device_work_data.para_type.pht_work_state = ucMasterRTURcvBuf_2[8];
            }

            device_work_data.para_type.timing_state = ucMasterRTURcvBuf_2[9];
            //device_work_data.para_type.fault_state = ucMasterRTURcvBuf_2[9];

			//fault_set_bit(FAULT_RESET_WIFI_BIT,ucMasterRTURcvBuf_2[30]&(1<<FAULT_RESET_WIFI_BIT));

//			if(device_work_data.para_type.device_mode == 2)
//				airclean_power_onoff(device_work_data.para_type.device_power_state);


			
			if(ucMasterRTURcvBuf_2[30]&(1<<FAULT_RESET_WIFI_BIT))
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
		else if(ucMasterRTURcvBuf_2[0] == 0xF1 && ucMasterRTURcvBuf_2[1] == 0xF1 && ucMasterRTURcvBuf_2[6] == 0x7e)
		{

			device_work_data.para_type.device_power_state = ucMasterRTURcvBuf_2[4];
			if(device_power_state_bak == 0xff)
			{

				device_power_state_bak = ucMasterRTURcvBuf_2[4];	

				if(ucMasterRTURcvBuf_2[4])
				{
	
					airclean_power_onoff(ucMasterRTURcvBuf_2[4]);
				}
				else
					{

					airclean_power_onoff(0);
					return;
				}
				
				
			}
			else
			{

				if(ucMasterRTURcvBuf_2[4] != device_power_state_bak)
				{
					device_power_state_bak = ucMasterRTURcvBuf_2[4];
					
					airclean_power_onoff(ucMasterRTURcvBuf_2[4]);

					if(ucMasterRTURcvBuf_2[4] == 0)
						return;
				}


				device_power_state_bak = ucMasterRTURcvBuf_2[4];
			}
		}
	}
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


static void set_display_board_data(void)
{
	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;

    disp_board_packet_data(33-4);

    ucMasterRTURcvBuf_2[0] = 0;
	errorCode = eMBMasterReqRead_not_rtu_datas_2(rs485_send_buf_not_modbus,33,RT_WAITING_FOREVER);
	if(errorCode == MB_MRE_REV_DATA)
	{
		if(ucMasterRTURcvBuf_2[0] == 0xF2 && ucMasterRTURcvBuf_2[1] == 0xF2 && ucMasterRTURcvBuf_2[32] == 0x7e)
        {
            ;
        }      
	}
}


//************************ Modbus������ѵ�߳�***************************
//��������: void thread_entry_ModbusMasterPoll(void* parameter)
//��ڲ�������
//���ڲ�������
//��    ע��Editor��Armink   2013-08-28    Company: BXXJS
//******************************************************************
void thread_entry_ModbusMasterPoll_2(void* parameter)
{
	eMBMasterInit_2(MB_RTU, 2, 9600,  MB_PAR_NONE);
	eMBMasterEnable_2();


	
	while (1)
	{
		eMBMasterPoll_2();
		rt_thread_delay(DELAY_MS(10));
	}
}




static void thread_entry_com_displayboard(void* parameter)
{
    rt_thread_t init_thread;


	init_thread = rt_thread_create("MBMasterPoll",
								   thread_entry_ModbusMasterPoll_2, RT_NULL,
								   512, 9, 50);
	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);

	u8 mystate;
	while (1)
	{
		
		rt_thread_delay(RT_TICK_PER_SECOND/10);



		if(mystate)
		{
	        set_display_board_data(); //100ms
			mystate=0;
    		rt_thread_delay(RT_TICK_PER_SECOND/10);

		}
		else
		{
			mystate=1;

			get_display_board_data(); //1s
    		rt_thread_delay(RT_TICK_PER_SECOND/10);

		}

	}
}








