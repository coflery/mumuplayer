////////////////////////////////////////////////////////////////////////////////
//  �� �� ��   : key.h
//  �� �� ��   : V1.0
//  ��    ��   : TinderSmith
//  ��������   : 2014-01-01
//  ����޸�   : 2015-06-27
//  ��������   : <MP3><stm32 vs1053 oled tfcard(spi) fat0.11>
//  ��Ȩ����   : �θ���
//	˵    ��   :
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
#ifndef KEY_H
#define KEY_H

#define blank        0

#define play         1
#define back         2
#define next         3
#define volume_add   4
#define volume_sub   5

#define long_play    6
#define long_back    7
#define long_next    8
#define long_volume_add   9
#define long_volume_sub   10

void key_init(void);
void read_keyboard(void);

#endif
