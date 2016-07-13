////////////////////////////////////////////////////////////////////////////////
//  文 件 名   : key.c
//  版 本 号   : V2.2
//  作    者   : TinderSmith
//  生成日期   : 2014-01-01
//  最近修改   : 2015-11-02
//  功能描述   : <MP3>@<stm32+vs1053+oled+tfcard(spi)+fat0.11>
//  版权所有   : 任复衡
//	说    明   :
//              ----------------------------------------------------------------
//              这个键盘输入程序,使用时间中断(stm32-tim)扫描键盘实现,利用时间中断特
//              点消除抖动方式比较特别,没有刻意用delay,而是检测时间中断进入次数实现.
//              另外实现了长按键和短按键输出.
//              ----------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
#include "sys.h"
#include "key.h"
unsigned char key_value;

void key_init(void)
{ GPIO_InitTypeDef GPIO_InitStructure;
	/* 配置 PC3  as K1  后*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	/* 配置 PA0.1.2.3  as K2.3.4.5  前.下.确定.上.*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉
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
					if(key_on()!= blank)//有键按下
					{
							key_state = 1;
					}
	break;
  case 1:
					if(key_on()!= blank)//不是抖动
					{
							key_state = 2;
							tmp_key = key_on();
					}
					else key_state = 0;//是抖动
	break;
  case 2:
         if(key_on() == blank)//如果松开
				 {
							key_state=0;//表明已经处理
							tmp_time = 1;//避免出现多次短按键累积形成长按键的情况
							if(key_long != 1)//上次不是长按
							{
									key_value = tmp_key;
							}
							else key_long = 0;//是长按键就清除标志	
				 }
				 else//没松开
				 {
							tmp_time++;//计时,直到松开
							key_state=2;
							if(tmp_time>7)//按下大于7个单位时间
							{
									tmp_time = 1;
									key_long = 1;//长按键处理标志
									key_value = tmp_key+5;//据key.h定义,tmp_key_value+5就是长按键定义
							}
				 }
	break; 
 }
}
