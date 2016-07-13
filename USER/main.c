////////////////////////////////////////////////////////////////////////////////
//  �� �� ��   : main.c
//  �� �� ��   : V1.0
//  �� 		��   : TinderSmith
//  ��������   : 2014-01-01
//  ����޸�   : 2015-06-27
//  ��������   : <MP3><stm32 vs1053 oled tfcard(spi) fat0.11>
//  ��Ȩ����   : �θ���
//  ˵ 		��   :
//              ----------------------------------------------------------------
//              GND  �ӵ�Դ��
//              VCC  ��5V��3.3v��Դ
//              CS   ��PE7
//              RES  ��PE8
//              DC   ��PE9
//              D0   ��PE10(SCL)
//              D1   ��PE11(SDA)
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
	 	RCC_Configuration();											//����ϵͳʱ��
		NVIC_Configuration();										  //�����ж�����
	 	key_init();																//����RCC��ʼ���Ƶ���RCC_Configuration,���̶�ʱ�ж�Ҫ��GPIO���ʼ��,��������������
		TIM_Configuration();											//���ö�ʱ�ж���
		uart_init(115200); 												//���ô���1������
		LED_GPIO_Config(); 												//LED�˿ڳ�ʼ��	
	 	delay_init();	  //��ʼ����ʱ��
		OLED_Init();		//��ʼ��OLED
		OLED_ShowString(0,4,"WelcomeBack XR");
		printf("WelcomeBack XR");
		OLED_ShowString(0,0,"Loading-VS");
		while(VS_Init())//VS��ʼ����ֵΪ0�ɹ�,����ֵʧ��
			{
			OLED_ShowString(0,0,"VS INT ER ");
			}
		OLED_ShowString(0,0,"Loading-SD");
		while(SD_Init())	//SD��ʼ����ֵΪ0�ɹ�,����ֵʧ��
			{
			OLED_ShowString(0,0,"SD INT ER ");
			}
		OLED_ShowString(0,0,"Loading-FATFS");
		get_disk_info();	 //��ȡSD����Ϣ
		list_file();			 //�г��ļ��б�
		//delay_ms(1500);
		OLED_Clear();
		Main_Player();
		//VS_recorde_save();
}

