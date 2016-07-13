////////////////////////////////////////////////////////////////////////////////
//  文 件 名   : key.h
//  版 本 号   : V1.0
//  作    者   : TinderSmith
//  生成日期   : 2014-01-01
//  最近修改   : 2015-06-27
//  功能描述   : <MP3><stm32 vs1053 oled tfcard(spi) fat0.11>
//  版权所有   : 任复衡
//	说    明   :
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
