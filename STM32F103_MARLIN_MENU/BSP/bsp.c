#include "bsp.h"

void bsp_init(void)
{ 
  /* 配置中断优先级分组 */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  systick_init();
  usart1_init(115200);
  KEY_Init();
  TIM2_Int_Init(999, 719); //10ms
  LCD12864_Init();
//  ultrasonic_init();  //初始化超声波模块
}


