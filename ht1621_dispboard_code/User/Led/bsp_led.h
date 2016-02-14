#ifndef __LED_H
#define	__LED_H

#include "stm32f10x.h"

//#ifndef __HT1621_H

/* ����HT1621���ӵ�GPIO�˿�, �û�ֻ��Ҫ�޸�����Ĵ��뼴�ɸı���Ƶ�HT1621���� */

#define HT1621_GPIOB_PORT    	    GPIOB			                              /* GPIOB�˿� */

#define HT1621_GPIOB_CLK 	    RCC_APB2Periph_GPIOB		         /* GPIOB�˿�ʱ�� */

#define HT1621CS_GPIOB_PIN		    GPIO_Pin_8			         /* ���ӵ�SCLʱ���ߵ�GPIOB */

#define HT1621WR_GPIOB_PIN		    GPIO_Pin_7			         /* ���ӵ�SCLʱ���ߵ�GPIO */

#define HT1621DATA_GPIOB_PIN		  GPIO_Pin_6			               /* ���ӵ�SCLʱ���ߵ�GPIO */


#define HT1621_GPIOC_PORT    	   GPIOC		                           /* GPIOC�˿� */
#define HT1621_GPIOC_CLK 	    RCC_APB2Periph_GPIOC               /* GPIOC�˿�ʱ�� */

#define HT1621_BL_GPIOC_PIN		  GPIO_Pin_1		             /* ����GPIO  */

#define HT1621_KEY1_GPIOC_PIN		  GPIO_Pin_7                     /* ����/��ʱGPIO */
#define HT1621_KEY2_GPIOC_PIN		  GPIO_Pin_8                     /*���� GPIO */
#define HT1621_KEY3_GPIOC_PIN		  GPIO_Pin_9                    /* ����   GPIO */


#define  HT1621_GPIOA_PORT    	      GPIOA		                      /* GPIOC�˿� */
#define  HT1621_GPIOA_CLK               RCC_APB2Periph_GPIOA		/* GPIOC�˿�ʱ�� */   
#define HT1621_KEY4_GPIOA_PIN		  GPIO_Pin_8                     /*  ����  GPIO */
#define HT1621_KEY5_GPIOA_PIN		  GPIO_Pin_9                    /* ����/��λGPIO */


#define KEY_ON	1
#define KEY_OFF	0

#define ON  1
#define OFF 0

/* ���κ꣬��������������һ��ʹ�� */

#define HT1621_CS(a)	             if (a)	\
					             GPIO_SetBits(HT1621_GPIOB_PORT,HT1621CS_GPIOB_PIN);\
					             else		\
					             GPIO_ResetBits(HT1621_GPIOB_PORT,HT1621CS_GPIOB_PIN)

#define HT1621_WR(a)	          if (a)	\
					             GPIO_SetBits(HT1621_GPIOB_PORT,HT1621WR_GPIOB_PIN);\
					             else		\
					             GPIO_ResetBits(HT1621_GPIOB_PORT,HT1621WR_GPIOB_PIN)

#define HT1621_DATA(a)        if (a)	\
					            GPIO_SetBits(HT1621_GPIOB_PORT,HT1621DATA_GPIOB_PIN);\
					            else		\
					            GPIO_ResetBits(HT1621_GPIOB_PORT,HT1621DATA_GPIOB_PIN)


#define HT1621_BL(a)             if (a)	\
					             GPIO_SetBits(HT1621_GPIOC_PORT,HT1621_BL_GPIOC_PIN);\
					            else		\
					            GPIO_ResetBits(HT1621_GPIOC_PORT,HT1621_BL_GPIOC_PIN)


#define  KeyPin  (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7))


					
#define BIAS 0x52 //0b1000 0101 0010 1/3duty 4com
#define SYSDIS 0X00 //0b1000 0000 0000 ����ϵͳ������LCDƫѹ������
#define SYSEN 0X02 //0b1000 0000 0010 ��ϵͳ����
#define LCDOFF 0X04 //0b1000 0000 0100 ��LCDƫѹ
#define LCDON 0X06 //0b1000 0000 0110 ��LCDƫѹ
#define XTAL 0x28 //0b1000 0010 1000 �ⲿ��ʱ��
#define RC256 0X30 //0b1000 0011 0000 �ڲ�ʱ��
#define TONEON 0X12 //0b1000 0001 0010 ���������
#define TONEOFF 0X10 //0b1000 0001 0000 �ر��������
#define WDTDIS 0X0A //0b1000 0000 1010 ��ֹ���Ź�

void delay(u32 nCount);
void delay_us(u32 nus);
void delay_ms(u16 nms);
void HT1621_GPIO_Config (void);
void Ht1621Wr_Data(u8 Data,u8 cnt);
void Ht1621WrOneData(u8 Addr,u8 Data);
void Ht1621WrCmd(u8 Cmd);
void Ht1621WrAllData(u8 Addr,u8 *p,u8 cnt);
void Ht1621DisplayState(u8 *Data,u8 *string,u8 Addr,u8 cnt);
void DisplayStrings(u8 *Data,u8 *string,u8 Addr,u8 cnt);
void Ht1621_cls(void); 
void Ht1621_clrbuf(void);
void Ht1621_off_disp(u8 f) ;
void Ht1621_on_disp(u8 f); 
void Key_Scan(void)  ;
void PollingKey(void);  //����/�ֶ�/��ʱ
void onoff_Scan(void); //����
void pin0_Scan(void) ;
void pin1_Scan(void) ;
void pin2_Scan(void) ;
void pin3_Scan(void) ;
void pin4_Scan(void) ;
void pin5_Scan(void) ;
void pin6_Scan(void) ;
void Ht1621_BL(void);
void Ht1621Display(void);

#endif /* __LED_H */

