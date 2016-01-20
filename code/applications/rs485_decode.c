
#include <rthw.h>
#include "rs485_decode.h"
#include "includes.h"
#include <string.h>




uchar *keyboard_data_buffer_point;
uchar keyboard_data_buffer[20];


rt_sem_t	uart1_sem;


struct _uart_dev_my
{
	struct rt_semaphore rx_sem;

	rt_device_t device;
};

struct _uart_dev_my* uart1_dev_my;
static rt_err_t rs485_rx_ind(rt_device_t dev, rt_size_t size)
{
    RT_ASSERT(uart1_dev_my != RT_NULL);

    /* release semaphore to let  thread rx data */
    rt_sem_release(&uart1_dev_my->rx_sem);

    return RT_EOK;
}
void uart1_rs485_set_device(void)
{
	    rt_device_t dev = RT_NULL;

    dev = rt_device_find("uart2");
	if (dev == RT_NULL)
    {
        rt_kprintf("finsh: can not find device: %s\n", "uart1");
        return;
    }

    /* check whether it's a same device */
    if (dev == uart1_dev_my->device) return;
		
    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX |\
                       RT_DEVICE_FLAG_STREAM) == RT_EOK)
    {
        if (uart1_dev_my->device != RT_NULL)
        {
            /* close old finsh device */
            rt_device_close(uart1_dev_my->device);
            rt_device_set_rx_indicate(uart1_dev_my->device, RT_NULL);
        }

        uart1_dev_my->device = dev;
        rt_device_set_rx_indicate(dev, rs485_rx_ind);
    }
}


#if 0
void pelco_rx_isr(u8 udr0)
{
    u8 i;
    static uchar keyboard_data_buffer1[20];  

	ProUart0Rec = 0;
	{ 
		Isr_i = udr0;

	  	KBDRecWrongFlag = 0;
	  	if (0x00 == Isr_j) 
		{
			if (0x01 == Current_Protocol) 
			{
				switch (Isr_i) 
				{
				 case 0xff: Rec_byte_count = 0x07;
				            Rec_byte_count_buff = 0x07;
				            Rec_spe_byte_count_buff = 0x09;
							Protocol_No = PELCO_D;
				            break;
				 case 0xa0: Rec_byte_count = 0x08;
				            Rec_spe_byte_count_buff = 0x0a;
				            Rec_byte_count_buff = 0x08;
							Protocol_No = PELCO_P;
							break;	
                case CAMERA_LINK_CMD_HEAD:
                    Rec_byte_count = 8;
                    Rec_spe_byte_count_buff = 0x0a;
                    Rec_byte_count_buff = 0x08;
                    Protocol_No = PROTOCOL_CAM_LINK;
                    break;	            
				 default:   
					Isr_com = 0; 
						Isr_j = 0;
					return;
				}
			}
			else
			{
				switch (Protocol_No)
				{
				case PELCO_D: 
					if(PELCO_D_FRAME_HEADE !=Isr_i)
					{
						Isr_com = 0; 
						Isr_j = 0;
						return;
					}
					break;
				case PELCO_P: 
					if(PELCO_P_FRAME_HEADE !=Isr_i)
					{
						Isr_com = 0; 
						Isr_j = 0;
						return;
					}
					break;
				default:

					break;
				}
			}
			keyboard_data_buffer1[0] = Isr_i;
			Isr_com = 1; 
			Isr_j = 0x01;
		}
		else 
		{                   //????????
			keyboard_data_buffer1[Isr_com] = Isr_i;
			Isr_com++;

			switch (Protocol_No)
			{
			case PELCO_P: 
			case PELCO_D: //if (0x07 == Rec_byte_count)
				Rec_byte_count = Rec_byte_count_buff;  //    
				break;
 			case PROTOCOL_CAM_LINK: 
				Rec_byte_count = Rec_byte_count_buff;  //    
				break;
               
			default:
                
				break;
			}
	
			if (Isr_com >= Rec_byte_count)
			{
				for (i=0x00; i<20; i++) 
					Rec_keyboard_data_buffer[i] = keyboard_data_buffer1[i];

				addqueue(20,keyboard_data_buffer1);

				for (i=0x00; i<20; i++) 
					keyboard_data_buffer1[i] = 0x00;
				if ((0xa0 == Rec_keyboard_data_buffer[0]) && (Protocol_No == PELCO_P))
				  Rec_keyboard_data_buffer[1]++;
				if (0x80 == (0xf0 & Rec_keyboard_data_buffer[0]))
				{
					i = Rec_keyboard_data_buffer[0] << 4;
					i |= Rec_keyboard_data_buffer[1] & 0x0f;
				}
				else 
					i = Rec_keyboard_data_buffer[1];

				if (i == domeNo)
				{
					switch (Protocol_No)
					{
					case PELCO_P: 

						break;
					case PELCO_D: 
 
						break;
					default:
						break;
					}
				}
				Isr_com = 0x00; 
				Isr_j = 0x00; 
				Rec_byte_com = 0X01;	
				rt_sem_release(uart1_sem);
			}	
		}
	}	
       
}

#else

u8 rrcnt = 0;
void pelco_rx_isr(u8 udr0)
{
    u8 i;
    static uchar keyboard_data_buffer1[50];  

	if(rrcnt > 40)
		rrcnt = 0;

	keyboard_data_buffer1[rrcnt] = udr0;
	rrcnt++;

	if(rrcnt == 5)
		rrcnt = 6;
       
}
#endif

void rt_rs485_thread_entry(void* parameter)
{
    char ch;

	while (1)
    {
        /* wait receive */
        if (rt_sem_take(&uart1_dev_my->rx_sem, RT_WAITING_FOREVER) != RT_EOK) continue;

        /* read one character from device */
        while (rt_device_read(uart1_dev_my->device, 0, &ch, 1) == 1)
        {
            pelco_rx_isr(ch);
        } /* end of device read */
    }	
	
}


void rs485_data_init(void)
{

	
}


rt_mutex_t rs485_send_mut = RT_NULL;


void rt_protocol_analyse_thread_entry(void* parameter)
{
    char k;

	while (1)
    {
        /* wait receive */
        if (rt_sem_take(uart1_sem, RT_WAITING_FOREVER) == RT_EOK) 
		{
			

				
		

			rt_thread_delay(40);
		}

	   
    }	
	
}




rt_err_t rs485_send_data(u8* data,u16 len)
{
	rt_mutex_take(rs485_send_mut,RT_WAITING_FOREVER);
	
	RS485_TX_ENABLE;
	if(uart1_dev_my->device == RT_NULL)	
	{
		uart1_rs485_set_device();
	}
	
	rt_device_write(uart1_dev_my->device, 0, data, len);

	rt_thread_delay (80);
	RS485_RX_ENABLE;

	rt_mutex_release(rs485_send_mut);
	return RT_EOK;
}

int rs485_system_init(void)
{
    rt_err_t result;
    rt_thread_t init_thread;

	rs485_send_mut = rt_mutex_create("rs485mut",RT_IPC_FLAG_FIFO);
		
	rs485_data_init();	
	


    uart1_dev_my = (struct _uart_dev_my*)rt_malloc(sizeof(struct _uart_dev_my));
    if (uart1_dev_my == RT_NULL)
    {
        rt_kprintf("no memory for shell\n");
        return -1;
    }
    memset(uart1_dev_my, 0, sizeof(struct _uart_dev_my));
    rt_sem_init(&(uart1_dev_my->rx_sem), "rs485rx", 0, 0);
	uart1_dev_my->device = RT_NULL;
	uart1_rs485_set_device();

	
	
	
	uart1_sem = rt_sem_create("uart1_sem",0, RT_IPC_FLAG_FIFO);  


		init_thread = rt_thread_create("rs485",rt_rs485_thread_entry, RT_NULL,
                                   4092, 8, 21);
	  if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);


	  init_thread = rt_thread_create("decode485",rt_protocol_analyse_thread_entry, RT_NULL,
								 1024, 8, 21);
	if (init_thread != RT_NULL)
		rt_thread_startup(init_thread);

	

    return 0;
}


