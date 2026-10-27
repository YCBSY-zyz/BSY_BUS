#ifndef __LED_H__
#define __LED_H__
	 
#include "stm32f10x.h"

//LED¶Ë¿Ú¶¨Òå
#define LED_RCC     RCC_APB2Periph_GPIOB                            
#define LED_PORT	GPIOB    
#define LED0        GPIO_Pin_5    



void Led_Config(void);
void LED0_ON(void);
void LED0_OFF(void);
void Task_Led(void);
void Locked_Task_Led (u8 sta);
void  Del_Task_Led (void);


#endif


