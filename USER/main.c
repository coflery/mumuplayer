////////////////////////////////////////////////////////////////////////////////
//  文 件 名   : main.c
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

#include "delay.h"
#include "usart.h"
#include "vs10xx.h"
#include "sdcard.h"
#include "sys.h"
#include "oled.h"
#include "fatfs.h"
#include "bmp.h"
#include "spi.h"
#include "key.h"
#include "led.h"

int main(void)
{	
	 	RCC_Configuration();											//配置系统时钟
		NVIC_Configuration();										  //配置中断向量
	 	key_init();																//里面RCC初始化移到了RCC_Configuration,键盘定时中断要在GPIO后初始化,否则键盘输入错误
		TIM_Configuration();											//配置定时中断器
		uart_init(115200); 												//配置串口1波特率
		LED_GPIO_Config(); 												//LED端口初始化	
	 	delay_init();	  //初始化延时函
		OLED_Init();		//初始化OLED
		OLED_ShowString(0,4,"WelcomeBack XR");
		printf("WelcomeBack XR");
		OLED_ShowString(0,0,"Loading-VS");
		while(VS_Init())//VS初始化的值为0成功,其他值失败
			{
			OLED_ShowString(0,0,"VS INT ER ");
			}
		OLED_ShowString(0,0,"Loading-SD");
		while(SD_Init())	//SD初始化的值为0成功,其他值失败
			{
			OLED_ShowString(0,0,"SD INT ER ");
			}
		OLED_ShowString(0,0,"Loading-FATFS");
		get_disk_info();	 //获取SD卡信息
		list_file();			 //列出文件列表
		//delay_ms(1500);
		OLED_Clear();
		Main_Player();
		//VS_recorde_save();
}

