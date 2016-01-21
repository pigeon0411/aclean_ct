//#include <board.h>
#include "app_task.h"
#include<string.h>



void uart_wifi_set_device(void);



u8 wifi_send_packet_buf_pub[100];
u8 wifi_recv_packet_buf_pub[100];
u8 wifi_data_buffer_recv_tmp[100];  
u8 wifi_recieve_data_length = 0;

struct _uart_dev_my* wifi_uart_dev_my;

// return 0,fail; 1,success
//buf,���յ������ݻ��壬len�������������ݵĳ���
u8 wifi_receive_data_check(u8* buf,u8 len)
{
    u8 i;
    u8 chk=0;

    u8 chk_src = buf[len-2];
    u8 end_code = buf[len-1];
    
    if(buf[0]==0xF1 && buf[1] == 0xF1 && end_code == 0x7E)
    {
        for(i=0;i<len-4;i++)
        {
            chk += buf[i+2];

        }
        if(chk == chk_src)
        {


            return 1;

        }
        
    }

    return 0;

}

rt_err_t wifi_send_data(u8* data,u16 len)
{
	//rt_mutex_take(rs485_send_mut,RT_WAITING_FOREVER);
	
	//RS485_TX_ENABLE;
	if(wifi_uart_dev_my->device == RT_NULL)	
	{
		uart_wifi_set_device();
	}
	
	rt_device_write(wifi_uart_dev_my->device, 0, data, len);

	rt_thread_delay (80);
	//RS485_RX_ENABLE;

	//rt_mutex_release(rs485_send_mut);
	return RT_EOK;
}


//�Դ����͵����ݽ��з����������
//buf,�����͵Ļ�������len,���ͻ��������������ݵĳ��ȣ���������ʼ�������ֽ�0XF2,0XF2,Ҳ������У���(һ�ֽ�)�ͽ�����(0X7Eһ�ֽ�)
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

    wifi_send_data(wifi_send_packet_buf_pub,len+4);
	
		return 1;
}



void send_F7_packet(void)
{
    u8 buf[20];


    buf[0] = 0xF7;
    buf[1] = 0x0E;
    
    buf[2] = 0x01;
    buf[3] = 0x32;
    
    buf[4] = 0x01;
    buf[5] = 0x32;

    buf[6] = 0x01;
    buf[7] = 0x00;
    
    //�豸�ͺ�
    
    buf[8] = 0x01;
    buf[9] = 0x01;

//Ӳ���汾��

    buf[10] = 0x01;
    buf[11] = 0x01;
    //(����汾��)

    buf[12] = 0x01;
    buf[13] = 0x01;

//��������
    buf[14] = 0x01;
    buf[15] = 0x01;


    wifi_send_packet_data(buf,16);
}




DEVICE_WORK_TYPE device_work_data;

void device_state_init(void)
{
    for(u8 i=0;i<sizeof(struct __para_type);i++)
    {
        device_work_data.device_data[i] = 0;
    }

}


u8 return_current_device_state(void)
{
    u8 buftmp[30];

    buftmp[0] = 0x01;
    buftmp[1] = 0x1b;

    u8 i;

    for(i=0;i<sizeof(struct __para_type);i++)
        {
        buftmp[2+i] = device_work_data.device_data[i];

    }

    wifi_send_packet_data(buftmp,i+2);

    return 0;
}

u8 set_device_work_mode(u8 type,u8 data)
{
    switch(type)
        {
    case 0x02:
        if(data)
            device_work_data.para_type.device_power_state = 1;
        else
            device_work_data.para_type.device_power_state = 0;

        
        break;
    case 0x03:
        if(data==1||data==2)
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

        
        break;
    case 0x06:
        if(data<=0x0c)
            device_work_data.para_type.timing_state = 1;
        else
            device_work_data.para_type.timing_state = 0;

        
        break;
    case 0x07:
        if(data<=3)
            device_work_data.para_type.wind_speed_state = 1;
        
        break;

    default:break;

    }
	return 1;
}

u8 wifi_receive_data_decode(u8* buf,u8 len)
{
//    u8 i;
//    u8 chk;
    
    switch(buf[0])
    {
    case 0x01:
         return_current_device_state();   
        break;
    case 0x02:

    case 0x03:

    case 0x04:

    case 0x05:

    case 0x06:

    case 0x07:
        set_device_work_mode(buf[0],buf[2]);
        return_current_device_state();  
        break;
    case 0xf7:
        send_F7_packet();
        break;

    default:break;       
    }

    return 1;
}





rt_sem_t	wifi_uart_sem;



static rt_err_t wifi_rx_ind(rt_device_t dev, rt_size_t size)
{
    RT_ASSERT(wifi_uart_dev_my != RT_NULL);

    /* release semaphore to let  thread rx data */
    rt_sem_release(&wifi_uart_dev_my->rx_sem);

    return RT_EOK;
}
void uart_wifi_set_device(void)
{
	    rt_device_t dev = RT_NULL;

    dev = rt_device_find("uart1");
	if (dev == RT_NULL)
    {
        rt_kprintf("finsh: can not find device: %s\n", "uart1");
        return;
    }

    /* check whether it's a same device */
    if (dev == wifi_uart_dev_my->device) return;
		
    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX |\
                       RT_DEVICE_FLAG_STREAM) == RT_EOK)
    {
        if (wifi_uart_dev_my->device != RT_NULL)
        {
            /* close old finsh device */
            rt_device_close(wifi_uart_dev_my->device);
            rt_device_set_rx_indicate(wifi_uart_dev_my->device, RT_NULL);
        }

        wifi_uart_dev_my->device = dev;
        rt_device_set_rx_indicate(dev, wifi_rx_ind);
    }
}


void wifi_rx_isr(u8 udr0)
{
    u8 i;
    static u8 wait_len = 0,wait_len_cnt = 0;
    static u8 recieved_len = 0;


    if(recieved_len == 0)
    {
        if(udr0 == 0xF1)
        {
            wifi_data_buffer_recv_tmp[recieved_len] = 0xf1;
            recieved_len++;
        }
        else
        {
            
            recieved_len = 0;
            return;
        }
    }
    else if(recieved_len == 1)
    {
        if(udr0 == 0xf1)
        {
            wifi_data_buffer_recv_tmp[recieved_len] = 0xf1;
            recieved_len++;
        }
        else
        {
            recieved_len = 0;
            return;
        }
    }
    else if(recieved_len <= 3)
    {
        wifi_data_buffer_recv_tmp[recieved_len] = udr0;
        recieved_len++;
			
		if(recieved_len == 4)
			{
		wait_len = udr0;
        wait_len_cnt = 0;
			}
    }
	else
		{
    if(wait_len_cnt <= wait_len)
    {
        wait_len_cnt++;
        wifi_data_buffer_recv_tmp[recieved_len] = udr0;
        recieved_len++;
    }
    else if(wait_len_cnt == wait_len+1)
    {
	     wifi_data_buffer_recv_tmp[recieved_len] = udr0;
	 
        recieved_len++;
        wifi_recieve_data_length = recieved_len;

        for(i=0;i<recieved_len;i++)
        {
            wifi_recv_packet_buf_pub[i] = wifi_data_buffer_recv_tmp[i];
        }

        recieved_len = 0;
        wait_len_cnt = 0;
        wait_len = 0;
       rt_sem_release(wifi_uart_sem);
 
    }
		}
}

void rt_wifi_thread_entry(void* parameter)
{
    char ch;

	while (1)
    {
        /* wait receive */
        if (rt_sem_take(&wifi_uart_dev_my->rx_sem, RT_WAITING_FOREVER) != RT_EOK) continue;

            /* read one character from device */
            while (rt_device_read(wifi_uart_dev_my->device, 0, &ch, 1) == 1)
            {
                wifi_rx_isr(ch);
            } /* end of device read */
    }	
	
}


void rt_wifi_decode_thread_entry(void* parameter)
{
//	u16 k;

	while(1)
	{
		if(rt_sem_take(wifi_uart_sem, RT_WAITING_FOREVER) == RT_EOK)
        {
			if (wifi_receive_data_check(wifi_recv_packet_buf_pub,wifi_recieve_data_length)) 
            {
                wifi_receive_data_decode(&wifi_recv_packet_buf_pub[2],wifi_recieve_data_length-4);
            }
                       
            rt_thread_delay(RT_TICK_PER_SECOND/50);
		}
    }

}

void rt_wifi_key_thread_entry(void* parameter)
{
//	u16 k;

	while(1)
	{
		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5) == 0)
        {
                       
            rt_thread_delay(DELAY_MS(20));

			if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5) == 0)
	        {
				wifi_factory_set();
			}
		}

		rt_thread_delay(DELAY_MS(100));
    }

}


rt_thread_t wifi_thread_id;

int wifi_uart_init(void)
{
//    rt_err_t result;
    rt_thread_t init_thread;

		
    wifi_uart_dev_my = (struct _uart_dev_my*)rt_malloc(sizeof(struct _uart_dev_my));
    if (wifi_uart_dev_my == RT_NULL)
    {
        rt_kprintf("no memory for shell\n");
        return -1;
    }
//    memset(wifi_uart_dev_my, 0, sizeof(struct _uart_dev_my));
    rt_sem_init(&(wifi_uart_dev_my->rx_sem), "wifirx", 0, 0);
	wifi_uart_dev_my->device = RT_NULL;
	uart_wifi_set_device();

	
	
	
	wifi_uart_sem = rt_sem_create("wifi_sem",0, RT_IPC_FLAG_FIFO);  


		wifi_thread_id = rt_thread_create("wifif",rt_wifi_thread_entry, RT_NULL,
                                   256, 8, 21);
	  if (wifi_thread_id != RT_NULL)
        rt_thread_startup(wifi_thread_id);

    init_thread = rt_thread_create("wifidecode",rt_wifi_decode_thread_entry, RT_NULL,
                                   512, 8, 21);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

    init_thread = rt_thread_create("wifiKEY",rt_wifi_key_thread_entry, RT_NULL,
                                   256, 8, 21);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);


    return 0;
}


void wifi_comm_init(void)
{
    device_state_init();
    wifi_uart_init();

}


const u8* wifi_enter_at_mode="+++AtCmd\\r\\n";
const u8* wifi_set_factory="AT+Default=1\\r\\n";

void wifi_factory_set(void)
{
	u8 datatmp[22];
	u8 len=0;
	u8 ch;
	
		rt_thread_suspend(wifi_thread_id);
		
		wifi_send_data((u8*)wifi_enter_at_mode,strlen((u8*)wifi_enter_at_mode));

		while(1)
		{
			if (rt_sem_take(&wifi_uart_dev_my->rx_sem,  DELAY_S(10)) != RT_EOK) continue;
			
            /* read one character from device */
            while (rt_device_read(wifi_uart_dev_my->device, 0, &ch, 1) == 1)
            {
            	
                datatmp[len++] = ch;

				
				
            } 
			if(len > strlen(wifi_enter_at_mode))
			{		
				if(strncmp(wifi_enter_at_mode,datatmp,len-1) == 0)
					{

					break;
				}

				len = 0;
			}
		}
		
		rt_thread_delay(DELAY_S(1));
		wifi_send_data((u8*)wifi_set_factory,strlen((u8*)wifi_set_factory));

		len = 0;
		while(1)
		{
			if (rt_sem_take(&wifi_uart_dev_my->rx_sem, DELAY_S(10)) != RT_EOK) continue;
			
            /* read one character from device */
            while (rt_device_read(wifi_uart_dev_my->device, 0, &ch, 1) == 1)
            {
            	
                datatmp[len++] = ch;

				
				
            } 
			if(len > strlen(wifi_set_factory))
			{		
				if(strncmp(wifi_set_factory,datatmp,len-1) == 0)
					{

					break;
				}

				len = 0;
			}
		}
		rt_thread_delay(DELAY_S(1));

		
		rt_thread_resume(wifi_thread_id);
}

