#ifndef __BSP_TIM_H
#define __BSP_TIM_H

#include "stm32f10x.h"

extern uint8_t keyPressed;

void TIM2_Int_Init(u16 arr,u16 psc);

#endif /* __BSP_TIM_H */

