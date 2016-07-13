////////////////////////////////////////////////////////////////////////////////
//  文 件 名   : led.c
//  版 本 号   : V1.0
//  作 		者   : TinderSmith
//  生成日期   : 2014-01-01
//  最近修改   : 2015-06-27
//  功能描述   : <MP3><stm32 vs1053 oled tfcard(spi) fat0.11>
//  版权所有   : 任复衡
//  说 		明   :
//              ----------------------------------------------------------------
//              GND  接电源地
//              VCC  接5V或3.3v电源
//              CS   接PE7
//              RES  接PE8
//              DC   接PE9
//              D0   接PE10(SCL)
//              D1   接PE11(SDA)
//              ----------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
#include "led.h"
/*
 * 函数名：LED_GPIO_Config
 * 描述  ：配置LED的I/O口
 * 输入  ：无
 * 输出  ：无
 */
void LED_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC |　RCC_APB2Periph_GPIOC, ENABLE);//前面系统入口时候初始化了

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_SetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);	 // turn off all led
	GPIO_SetBits(GPIOC, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);	 // turn off all led
}

