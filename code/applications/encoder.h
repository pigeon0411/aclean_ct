/*
 * File      : ENCODER.h
 * Function  : PWM ENCODER FUNCTION
 *
 * Change Logs:
 * Date           Author       Notes
*/
#ifndef __ENCODER_H_
#define __ENCODER_H_
#include "stm32f10x_it.h"
#include <rtthread.h>
#include "rtdevice.h"
#include <rthw.h>

#undef NULL
#define NULL 0                  

#define ENCODER_NUMB	2   
#define ENCODER_RB		256

#define ENCODER_TIME                TIM6
#define RCC_APB1Periph_ENCODER_TIME RCC_APB1Periph_TIM6
#define ENCODER_TIME_IRQn TIM6_IRQn
#define ENCODER_DETECTION_TIME 1 //20MS 米ㄓ?? D豕∩車車迆0 D?車迆910
#define ENCODER_DETECTION_PERIOD (uint16_t)(72*(ENCODER_DETECTION_TIME)-1)//?足2a那㊣??
struct ENCODERx_GPIO_TYPE
{
    GPIO_TypeDef* GPIOx;
	  uint16_t GPIO_Pin_ch1;
		uint16_t GPIO_Pin_ch2;
    uint32_t RCC_APB2Periph_GPIOx;
};

struct TIMx_ENCODERx_TYPE
{
	TIM_TypeDef* TIMx;
	//uint16_t TIM_Channel;
	uint32_t RCC_APBxPeriph;
	//IRQn_Type TIMx_IRQn;
  const struct	ENCODERx_GPIO_TYPE* encoder_gpio;
	uint32_t gpio_remap_tim;
	//uint8_t* rece_data;
};

struct ENCODER_DATA_TYPE
{
	uint16_t data[ENCODER_NUMB];
	uint8_t  direction;
};



void all_encoder_init(void);

extern rt_sem_t encoder_rd_sem ;
#endif


