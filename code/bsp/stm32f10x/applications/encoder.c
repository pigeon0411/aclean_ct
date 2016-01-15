/*
 * File      : ENCODER.c
 * Function  : PWM ENCODER FUNCTION
 *
 * Change Logs:
 * Date           Author       Notes
*/

#include "encoder.h"
extern Motor_Struct motor_h;
extern Motor_Struct motor_v;

static rt_uint8_t encoder1_buf[ENCODER_RB];
struct rt_ringbuffer encoder_rb;
static rt_sem_t encoder_rwrb_sem = RT_NULL;
 rt_sem_t encoder_rd_sem = RT_NULL;
//谷豕?“2?那y

//TIM1 OPS2 MOTO2
static const struct ENCODERx_GPIO_TYPE tim1_encoder_gpio_ops=
{
	GPIOE,
	GPIO_Pin_9,
	GPIO_Pin_11,
	RCC_APB2Periph_GPIOE,
};

 struct TIMx_ENCODERx_TYPE tim1_encoder_ops=
{
	TIM1,
	//TIM_Channel_2,
	RCC_APB2Periph_TIM1,//apb1
	//TIM5_IRQn,
	&tim1_encoder_gpio_ops,
	GPIO_FullRemap_TIM1,
//	0,
};

//TIM8 OPS5 MOTO WING2
static const struct ENCODERx_GPIO_TYPE tim8_encoder_gpio_ops=
{
	GPIOC,
	GPIO_Pin_6,
	GPIO_Pin_7,
	RCC_APB2Periph_GPIOC,
};

 struct TIMx_ENCODERx_TYPE tim8_encoder_ops=
{
	TIM8,
	RCC_APB2Periph_TIM8,
	&tim8_encoder_gpio_ops,
	NULL,
//	0,
};
//TIM3 encoder ch1(PA6), ch2(PA7)
static const struct ENCODERx_GPIO_TYPE tim3_encoder_gpio_ops=
{
	GPIOA,
	GPIO_Pin_6,
	GPIO_Pin_7,
	RCC_APB2Periph_GPIOA,
};

struct TIMx_ENCODERx_TYPE tim3_encoder_ops=
{
	TIM3,
	RCC_APB1Periph_TIM3,//apb1
	&tim3_encoder_gpio_ops,
	NULL,
};
struct TIMx_ENCODERx_TYPE* tim1_encoder_ops2 = &tim1_encoder_ops;
struct TIMx_ENCODERx_TYPE* tim8_encoder_ops1 = &tim8_encoder_ops;
//struct TIMx_ENCODERx_TYPE* tim3_encoder_ops1 = &tim3_encoder_ops;

/*
o‘那y㏒oencoder_init
1|?邦㏒o㊣角???¯3?那??‘
那?豕?㏒o米迆???﹞timx_encoder?芍11足? 
那?3?㏒o?T
*/
static void encoder_init(struct TIMx_ENCODERx_TYPE* timx_encoder)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;


    /* TIM clock source enable */
    if(timx_encoder->TIMx == TIM1 || timx_encoder->TIMx == TIM8)
    {
        RCC_APB2PeriphClockCmd(timx_encoder->RCC_APBxPeriph, ENABLE);  //RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(timx_encoder->RCC_APBxPeriph, ENABLE);
    }
    /* Enable GPIOA, clock */
    RCC_APB2PeriphClockCmd(timx_encoder->encoder_gpio->RCC_APB2Periph_GPIOx, ENABLE);

    /* Configure PC6,PC7(TIM8) && PE9,PE11(TIM1) as encoder input */
    GPIO_InitStructure.GPIO_Pin = timx_encoder->encoder_gpio->GPIO_Pin_ch1 | timx_encoder->encoder_gpio->GPIO_Pin_ch2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(timx_encoder->encoder_gpio->GPIOx, &GPIO_InitStructure);
  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	if(timx_encoder->gpio_remap_tim != NULL)
		GPIO_PinRemapConfig(timx_encoder->gpio_remap_tim, ENABLE);    
		
    /* Timer configuration in Encoder mode */
    TIM_DeInit(timx_encoder->TIMx);

    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    TIM_TimeBaseStructure.TIM_Prescaler = 0;  //  
    TIM_TimeBaseStructure.TIM_Period = 0xffff; //(4*12)-1 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
    TIM_TimeBaseInit(timx_encoder->TIMx, &TIM_TimeBaseStructure);
												  //TIM_EncoderMode_TI12
	TIM_EncoderInterfaceConfig(timx_encoder->TIMx, TIM_EncoderMode_TI1, 
                      TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    //TIM_EncoderInterfaceConfig(timx_encoder->TIMx, TIM_EncoderMode_TI1, 
    //                         TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 6;
    TIM_ICInit(timx_encoder->TIMx, &TIM_ICInitStructure);
  
    //Reset counter
    timx_encoder->TIMx->CNT = 0;

    //ENC_Clear_Speed_Buffer();
    TIM_Cmd(timx_encoder->TIMx, ENABLE);     
}
/*
o‘那y㏒oencoder_time_init
1|?邦㏒o㊣角???¯?“那㊣?¯3?那??‘
那?豕?㏒o?T
那?3?㏒o?T
*/
static void encoder_time_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_ENCODER_TIME , ENABLE);
	 TIM_DeInit(ENCODER_TIME); 
    /* Enable the  global Interrupt */

	//NVIC_SetVectorTable (NVIC_VectTab_FLASH ,0x0);
		
    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 19999;//9999;//20MS ?a米ㄓ??
    TIM_TimeBaseStructure.TIM_Prescaler = ENCODER_DETECTION_PERIOD;//143;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(ENCODER_TIME, &TIM_TimeBaseStructure);

    NVIC_InitStructure.NVIC_IRQChannel = ENCODER_TIME_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);	
	  /* TIM IT enable */
	//TIM_UpdateRequestConfig( ENCODER_TIME, TIM_UpdateSource_Regular);
	TIM_ClearITPendingBit(ENCODER_TIME, TIM_FLAG_Update);
    TIM_ITConfig(ENCODER_TIME, TIM_FLAG_Update, ENABLE);
	TIM_Cmd(ENCODER_TIME, ENABLE); 
}
/*
o‘那y㏒oall_encoder_init
1|?邦㏒o?迄車D㊣角???¯?⊿?“那㊣?¯3?那??‘
那?豕?㏒o?T
那?3?㏒o?T
*/
void all_encoder_init(void)
{
//	struct TIMx_ENCODERx_TYPE timx_encoder_ops1 = timx_encoder_ops;
	 rt_ringbuffer_init(&encoder_rb,encoder1_buf,ENCODER_RB);
    encoder_rwrb_sem = rt_sem_create("encoder0",1,RT_IPC_FLAG_FIFO);
    if (encoder_rwrb_sem == RT_NULL)
    {
        return ;
    }
		encoder_rd_sem = rt_sem_create("encoder1",0,RT_IPC_FLAG_FIFO);	
    if (encoder_rd_sem == RT_NULL)
    {
        return ;
    }	
	encoder_init(tim1_encoder_ops2); 
	encoder_init(tim8_encoder_ops1);  //encoder_init(tim3_encoder_ops1);	
	encoder_time_init();

	
}



rt_uint16_t encoder_realtime_data[2];
rt_uint8_t	encoder_rb_flag = 1;
/*
o‘那y㏒oTIM6_IRQHandler
1|?邦㏒o?“那㊣?¯?D??o‘那y ?“那㊣?芍豕?㊣角???¯??3?那y
那?豕?㏒o?T
那?3?㏒o?T
?米?¯:?“那㊣?¯?a16??1那車?bit15㊣赤那?﹞??辰 bit0~bit15㊣赤那???3?那y
*/

//void TIM6_IRQHandler(void)
//{
//	rt_base_t level;
//	rt_interrupt_enter();
//	if (TIM_GetITStatus(ENCODER_TIME, TIM_FLAG_Update) != RESET)
//	{
//		TIM_ClearITPendingBit(ENCODER_TIME, TIM_FLAG_Update);
//		level = rt_hw_interrupt_disable();
//		if(tim1_encoder_ops2->TIMx->CNT < 32767)
//		{
//			encoder_realtime_data[0]=tim1_encoder_ops2->TIMx->CNT;
//		}
//		else
//		{
//			encoder_realtime_data[0]=(0xffff - tim1_encoder_ops2->TIMx->CNT) | 0x8000;
//		}
//		if(encoder_rb_flag)
//		{
//			rt_ringbuffer_put(&encoder_rb,(rt_uint8_t *)&encoder_realtime_data[0],2);
//		}
//		tim1_encoder_ops2->TIMx->CNT = 0;
//		rt_hw_interrupt_enable(level);

//		rt_sem_release(encoder_rd_sem);//??豕?3谷1| ﹞⊿?赤D?o?芍?
//	}
//	    /* leave interrupt */
//    rt_interrupt_leave();
//}
s16 encoder_data[2];
void TIM6_IRQHandler(void)
{  
////	encoder_data[0]=tim1_encoder_ops2->TIMx->CNT;
////	tim1_encoder_ops2->TIMx->CNT = 0;
////	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);  //??3yTIMx米??D??∩y∩|角赤??:TIM ?D???∩      
    
    encoder_data[1]= tim8_encoder_ops1->TIMx->CNT;   //MOTOR2_ENCODER,TIM8 PC6,PC7
	tim8_encoder_ops1->TIMx->CNT = 0;
    
	encoder_data[0]=tim1_encoder_ops2->TIMx->CNT;    //MOTOR1_ENCODER,TIM1,PE9,PE11
	tim1_encoder_ops2->TIMx->CNT = 0;       

    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);      // 



    motor_h.site_code += encoder_data[0];
    motor_v.site_code += encoder_data[1];	


	
	//??豕?3谷1| ﹞⊿?赤D?o?芍?
    if(encoder_data[0]< 0)        
    {
        motor_h.spcode = -encoder_data[0];
        motor_h.code += -encoder_data[0];   //∩?㊣角???¯??那y
    }
    else
    {
        motor_h.spcode = encoder_data[0];
        motor_h.code +=  encoder_data[0];   //∩?㊣角???¯??那y    
    }
    
    if(encoder_data[1] < 0)
    {
        motor_v.spcode = -encoder_data[1];
        motor_v.code += -encoder_data[1];
    }
    else
    {
        motor_v.spcode = encoder_data[1];
        motor_v.code +=  encoder_data[1];       
    }
}

//void read_encoder_rd_data(struct rt_ringbuffer *rb,
//                            rt_uint8_t           *ptr,
//                            rt_uint16_t           length)
//{
//	// rt_sem_take (sem,RT_WAITING_FOREVER);
//   result = rt_ringbuffer_get(&encoder_rb,&data_buffer[0],33);
//}


