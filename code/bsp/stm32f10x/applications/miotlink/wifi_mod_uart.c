#include <board.h>
#include <rtthread.h>


u8 wifi_send_packet_buf_pub[100];
u8 wifi_recv_packet_buf_pub[100];
u8 wifi_data_buffer_recv_tmp[100];  
u8 wifi_recieve_data_length = 0;


// return 0,fail; 1,success
//buf,接收到的数据缓冲，len缓冲中所有数据的长度
u8 wifi_receive_data_check(u8* buf,u8 len)
{
    u8 i;
    u8 chk;

    u8 chk_src = buf[len-1];
    u8 end_code = buf[len];
    
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



//对待发送的数据进行封包，并发送
//buf,待发送的缓冲区，len,发送缓冲区中所有数据的长度，不包括起始码两个字节0XF2,0XF2,也不包括校验和(一字节)和结束码(0X7E一字节)
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
    
    //设备型号
    
    buf[8] = 0x01;
    buf[9] = 0x01;

//硬件版本号

    buf[10] = 0x01;
    buf[11] = 0x01;
    //(软件版本号)

    buf[12] = 0x01;
    buf[13] = 0x01;

//生产日期
    buf[14] = 0x01;
    buf[15] = 0x01;


    wifi_send_packet_data(buf,16);
}

u8 wifi_receive_data_decode(u8* buf,u8 len)
{
    u8 i;
    u8 chk;
    
    switch(buf[0])
    {
    case 0x01:

        break;
    case 0x02:

        break;
    case 0x03:

        break;
    case 0x04:

        break;
    case 0x05:

        break;
    case 0x06:

        break;
    case 0x07:

        break;
    case 0xf7:
        send_F7_packet();
        break;

    default:break;       
    }

    return 1;
}





rt_sem_t	wifi_uart_sem;


struct _uart_dev_my
{
	struct rt_semaphore rx_sem;

	rt_device_t device;
};

struct _uart_dev_my* wifi_uart_dev_my;
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
    else if(recieved_len == 4)
    {
        wifi_data_buffer_recv_tmp[recieved_len] = udr0;
        wait_len = udr0;
        wait_len_cnt = 0;
        recieved_len++;
    }

    if(wait_len_cnt <= wait_len)
    {
        wait_len_cnt++;
        wifi_data_buffer_recv_tmp[recieved_len] = udr0;
        recieved_len++;
    }
    else if(wait_len_cnt == wait_len+1)
    {
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
	u16 k;

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


int wifi_uart_init(void)
{
    rt_err_t result;
    rt_thread_t init_thread;

		
    wifi_uart_dev_my = (struct _uart_dev_my*)rt_malloc(sizeof(struct _uart_dev_my));
    if (wifi_uart_dev_my == RT_NULL)
    {
        rt_kprintf("no memory for shell\n");
        return -1;
    }
    memset(wifi_uart_dev_my, 0, sizeof(struct _uart_dev_my));
    rt_sem_init(&(wifi_uart_dev_my->rx_sem), "wifirx", 0, 0);
	wifi_uart_dev_my->device = RT_NULL;
	uart_wifi_set_device();

	
	
	
	wifi_uart_sem = rt_sem_create("wifi_sem",0, RT_IPC_FLAG_FIFO);  


		init_thread = rt_thread_create("wifif",rt_wifi_thread_entry, RT_NULL,
                                   512, 8, 21);
	  if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

    init_thread = rt_thread_create("wifidecode",rt_wifi_thread_entry, RT_NULL,
                                   1024, 8, 21);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

    return 0;
}




