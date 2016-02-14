
// ͨ�ö�ʱ��TIMx,x[2,3,4,5]��ʱӦ��
#include "stm32f10x.h"
#include "bsp_led.h"
#include "bsp_TiMbase.h"


int main(void)
{
	/* HT1621 �˿����� */ 
	 HT1621_GPIO_Config ();

	/* ͨ�ö�ʱ�� TIMx,x[2,3,4,5] ��ʱ���� */	
        TIMx_Configuration();
	
	/* ����ͨ�ö�ʱ�� TIMx,x[2,3,4,5]���ж����ȼ� */
	TIMx_NVIC_Configuration();

	/* ͨ�ö�ʱ�� TIMx,x[2,3,4,5] ���¿�ʱ�ӣ���ʼ��ʱ */
	macTIM_APBxClock_FUN (macTIM_CLK, ENABLE);
	
	
	HT1621_BL(OFF);
       Ht1621_clrbuf(); 
       Ht1621_cls();  //����
       delay_ms(50);
	
  while(1)
  {
      
     Key_Scan();   //����ɨ��
     PollingKey();
     onoff_Scan(); //���ػ�
 	
  }
}
/*********************************************END OF FILE**********************/

