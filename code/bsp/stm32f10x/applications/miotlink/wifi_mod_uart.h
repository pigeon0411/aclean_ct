
void wifi_comm_init(void);

rt_err_t wifi_send_data(u8* data,u16 len);


#ifndef DEVICE_WORK_TYPE_MACRO
#define DEVICE_WORK_TYPE_MACRO

typedef union __DEVICE_WORK_TYPE {
    struct __para_type
    {
    u8 device_power_state; //0,off;1,on
    u8 device_mode; //1 auto; 2,manual
    u8 wind_speed_state;
	
    u8 high_pressur_state;
    u8 pht_work_state;//0,off;1,on
    u8 timing_state; //定时值 为0时表示关闭定时；若为1到12的值时，表示定时的小时数

    u16 house1_pm2_5;
    u16 house1_co2;
    u16 house2_pm2_5;
    u16 house2_co2;
    u16 house3_pm2_5;
    u16 house3_co2;
    u16 house4_pm2_5;
    u16 house4_co2;
    u16 house5_pm2_5;
    u16 house5_co2;
    u8 fault_state; //bit0,motor;bit1,pht; bit2,clean; bit3,esd; bit4,run
    } para_type;
    u8 device_data[27];

} DEVICE_WORK_TYPE;



#define FAULT_MOTOR_BIT    (1)
#define FAULT_PHT_BIT    (2)
#define FAULT_CLEAN_BIT    (3)
#define FAULT_ESD_BIT    (0)
#define FAULT_RUN_BIT    (4)
#define FAULT_WIND_BIT    (5)
#define FAULT_RESET_WIFI_BIT    (6)




struct _uart_dev_my
{
	struct rt_semaphore rx_sem;

	rt_device_t device;
};



#endif


extern 	void wifi_factory_set(void);
extern u8 device_power_state_pre;
extern void device_sys_para_save(void);

extern void device_sys_para_get(void);
extern void device_state_init(void);




