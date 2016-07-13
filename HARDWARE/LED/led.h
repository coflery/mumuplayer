#ifndef __LED_H
#define	__LED_H

#include "stm32f10x.h"

#define ON  0
#define OFF 1

#define LED1(a)	if (a)	\
					GPIO_SetBits(GPIOD,GPIO_Pin_12);\
					else		\
					GPIO_ResetBits(GPIOD,GPIO_Pin_12)

#define LED2(a)	if (a)	\
					GPIO_SetBits(GPIOD,GPIO_Pin_13);\
					else		\
					GPIO_ResetBits(GPIOD,GPIO_Pin_13)

#define LED3(a)	if (a)	\
					GPIO_SetBits(GPIOD,GPIO_Pin_14);\
					else		\
					GPIO_ResetBits(GPIOD,GPIO_Pin_14)
#define LED4(a)	if (a)	\
					GPIO_SetBits(GPIOD,GPIO_Pin_15);\
					else		\
					GPIO_ResetBits(GPIOD,GPIO_Pin_15)

#define LED5(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_6);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_6)

#define LED6(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_7);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_7)
					
#define LED7(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_8);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_8)
					
#define LED8(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_9);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_9)

void LED_GPIO_Config(void);

#endif /* __LED_H */
