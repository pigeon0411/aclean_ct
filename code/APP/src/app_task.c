#include "app_task.h"

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#elif __ICCARM__
#pragma section="HEAP"
#else
extern int __bss_end;
#endif

uint8_t CpuUsageMajor, CpuUsageMinor; //CPUʹ����
USHORT  usModbusUserData[MB_PDU_SIZE_MAX];
UCHAR   ucModbusUserData[MB_PDU_SIZE_MAX];
//====================����ϵͳ���߳����ȼ�==================================
#define thread_SysMonitor_Prio		    	11
#define thread_ModbusSlavePoll_Prio      	10
#define thread_ModbusMasterPoll_Prio      	 9
ALIGN(RT_ALIGN_SIZE)
//====================����ϵͳ���̶߳�ջ====================================
static rt_uint8_t thread_SysMonitor_stack[256];
static rt_uint8_t thread_ModbusMasterPoll_stack[512];

struct rt_thread thread_SysMonitor;

struct rt_thread thread_ModbusMasterPoll;


u16 myreg1,myreg2;

extern USHORT   usMRegInBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_INPUT_NREGS];
extern USHORT   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];

extern DEVICE_WORK_TYPE device_work_data;

//***************************ϵͳ����߳�***************************
//��������: void thread_entry_SysRunLed(void* parameter)
//��ڲ�������
//���ڲ�������
//��    ע��Editor��Armink   2013-08-02   Company: BXXJS
//******************************************************************
void thread_entry_SysMonitor(void* parameter)
{
	eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
	uint16_t errorCount = 0;
	while (1)
	{

		rt_thread_delay(RT_TICK_PER_SECOND);

//		errorCode = eMBMasterReqReadDiscreteInputs(1,3,8,RT_WAITING_FOREVER);
//		errorCode = eMBMasterReqWriteMultipleCoils(1,3,5,ucModbusUserData,RT_WAITING_FOREVER);
		//errorCode = eMBMasterReqWriteCoil(1,8,0xFF00,RT_WAITING_FOREVER);
//		errorCode = eMBMasterReqReadCoils(1,3,8,RT_WAITING_FOREVER);
		//errorCode = eMBMasterReqReadInputRegister(1,1,2,RT_WAITING_FOREVER);

		errorCode = eMBMasterReqReadHoldingRegister(1,0,2,RT_WAITING_FOREVER);

		if(errorCode == MB_MRE_NO_ERR)
			{
//				device_work_data.para_type.house1_co2 = usMRegHoldBuf[0][0];
//				device_work_data.para_type.house1_pm2_5 = usMRegHoldBuf[0][1];

		}


			
		
//		errorCode = eMBMasterReqWriteHoldingRegister(1,3,usModbusUserData[0],RT_WAITING_FOREVER);
//		errorCode = eMBMasterReqWriteMultipleHoldingRegister(1,3,2,usModbusUserData,RT_WAITING_FOREVER);
//		errorCode = eMBMasterReqReadWriteMultipleHoldingRegister(1,3,2,usModbusUserData,5,2,RT_WAITING_FOREVER);
		//��¼�������
		if (errorCode != MB_MRE_NO_ERR) {
			errorCount++;
		}
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
	eMBMasterInit(MB_RTU, 4, 9600,  MB_PAR_NONE);
	eMBMasterEnable();
	while (1)
	{
		eMBMasterPoll();
		rt_thread_delay(DELAY_MS(30));
	}
}


void rt_main_thread_entry(void* parameter)
{

	rs485_system_init();
	//osd_init();
	rt_thread_delay(600);
	//rt_key_ctl_init();
	
	//rt_adc_ctl_init();


    wifi_comm_init();

	//osd_test();
}



//**********************ϵͳ��ʼ������********************************
//��������: int rt_application_init(void)
//��ڲ�������
//���ڲ�������
//��    ע��Editor��Liuqiuhu   2013-1-31   Company: BXXJS
//********************************************************************
int rt_application_init(void)
{
    rt_thread_t init_thread;

    init_thread = rt_thread_create("mY",
                                   rt_main_thread_entry, RT_NULL,
                                   512, 6, 50);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);


	rt_thread_init(&thread_SysMonitor, "SysMonitor", thread_entry_SysMonitor,
			RT_NULL, thread_SysMonitor_stack, sizeof(thread_SysMonitor_stack),
			thread_SysMonitor_Prio, 50);
	rt_thread_startup(&thread_SysMonitor);


	rt_thread_init(&thread_ModbusMasterPoll, "MBMasterPoll",
			thread_entry_ModbusMasterPoll, RT_NULL, thread_ModbusMasterPoll_stack,
			sizeof(thread_ModbusMasterPoll_stack), thread_ModbusMasterPoll_Prio,
			50);
	rt_thread_startup(&thread_ModbusMasterPoll);

	return 0;
}

//**************************��ʼ��RT-Thread����*************************************
//��������: void rtthread_startup(void)
//��ڲ�������
//���ڲ�������
//��    ע��Editor��Armink 2011-04-04    Company: BXXJS
//**********************************************************************************
void rtthread_startup(void)
{
	/* init board */
	rt_hw_board_init();

	/* show version */
	rt_show_version();

	/* init tick */
	rt_system_tick_init();

	/* init kernel object */
	rt_system_object_init();

	/* init timer system */
	rt_system_timer_init();

#ifdef RT_USING_HEAP
#ifdef __CC_ARM
	rt_system_heap_init((void*)&Image$$RW_IRAM1$$ZI$$Limit, (void*)STM32_SRAM_END);
#elif __ICCARM__
	rt_system_heap_init(__segment_end("HEAP"), (void*)STM32_SRAM_END);
#else
	/* init memory system */
	rt_system_heap_init((void*)&__bss_end, (void*)STM32_SRAM_END);
#endif
#endif

	/* init scheduler system */
	rt_system_scheduler_init();

	/* init all device */
	rt_device_init_all();

	/* init application */
	rt_application_init();

#ifdef RT_USING_FINSH
	/* init finsh */
	finsh_system_init();
	finsh_set_device("uart1");
#endif

	/* init timer thread */
	rt_system_timer_thread_init();

	/* init idle thread */
	rt_thread_idle_init();

	/* Add CPU usage to system */
	cpu_usage_init();

	/* start scheduler */
	rt_system_scheduler_start();

	/* never reach here */
	return;
}

