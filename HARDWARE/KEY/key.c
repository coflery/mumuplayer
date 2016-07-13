////////////////////////////////////////////////////////////////////////////////
//  �� �� ��   : key.c
//  �� �� ��   : V2.2
//  ��    ��   : TinderSmith
//  ��������   : 2014-01-01
//  ����޸�   : 2015-11-02
//  ��������   : <MP3>@<stm32+vs1053+oled+tfcard(spi)+fat0.11>
//  ��Ȩ����   : �θ���
//	˵    ��   :
//              ----------------------------------------------------------------
//              ��������������,ʹ��ʱ���ж�(stm32-tim)ɨ�����ʵ��,����ʱ���ж���
//              ������������ʽ�Ƚ��ر�,û�п�����delay,���Ǽ��ʱ���жϽ������ʵ��.
//              ����ʵ���˳������Ͷ̰������.
//              ----------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
#include "sys.h"
#include "key.h"
unsigned char key_value;

void key_init(void)
{ GPIO_InitTypeDef GPIO_InitStructure;
	/* ���� PC3  as K1  ��*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	/* ���� PA0.1.2.3  as K2.3.4.5  ǰ.��.ȷ��.��.*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

unsigned char key_on(void)
{if(!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0))	 return next;
 if(!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1))	 return volume_sub;
 if(!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2))	 return play;
 if(!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3))	 return volume_add;
 if(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_3))	 return back;
 return blank;
}

void read_keyboard(void)
{static unsigned char tmp_time;
 static unsigned char tmp_key;
 static unsigned char key_long;
 static unsigned char key_state;

 switch(key_state)
 {
  case 0:
					if(key_on()!= blank)//�м�����
					{
							key_state = 1;
					}
	break;
  case 1:
					if(key_on()!= blank)//���Ƕ���
					{
							key_state = 2;
							tmp_key = key_on();
					}
					else key_state = 0;//�Ƕ���
	break;
  case 2:
         if(key_on() == blank)//����ɿ�
				 {
							key_state=0;//�����Ѿ�����
							tmp_time = 1;//������ֶ�ζ̰����ۻ��γɳ����������
							if(key_long != 1)//�ϴβ��ǳ���
							{
									key_value = tmp_key;
							}
							else key_long = 0;//�ǳ������������־	
				 }
				 else//û�ɿ�
				 {
							tmp_time++;//��ʱ,ֱ���ɿ�
							key_state=2;
							if(tmp_time>7)//���´���7����λʱ��
							{
									tmp_time = 1;
									key_long = 1;//�����������־
									key_value = tmp_key+5;//��key.h����,tmp_key_value+5���ǳ���������
							}
				 }
	break; 
 }
}
