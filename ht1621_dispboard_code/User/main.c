
// 通用定时器TIMx,x[2,3,4,5]定时应用
#include "stm32f10x.h"
#include "bsp_led.h"
#include "bsp_TiMbase.h"


int main(void)
{
	/* HT1621 端口配置 */ 
	 HT1621_GPIO_Config ();

	/* 通用定时器 TIMx,x[2,3,4,5] 定时配置 */	
        TIMx_Configuration();
	
	/* 配置通用定时器 TIMx,x[2,3,4,5]的中断优先级 */
	TIMx_NVIC_Configuration();

	/* 通用定时器 TIMx,x[2,3,4,5] 重新开时钟，开始计时 */
	macTIM_APBxClock_FUN (macTIM_CLK, ENABLE);
	
	
	HT1621_BL(OFF);
       Ht1621_clrbuf(); 
       Ht1621_cls();  //清屏
       delay_ms(50);
	
  while(1)
  {
      
     Key_Scan();   //按键扫描
     PollingKey();
     onoff_Scan(); //开关机
 	
  }
}
/*********************************************END OF FILE**********************/

