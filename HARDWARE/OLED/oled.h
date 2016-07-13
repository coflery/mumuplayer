////////////////////////////////////////////////////////////////////////////////
//  �� �� ��   : oled.h
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
#ifndef __OLED_H
#define __OLED_H			  	 
#include "sys.h"
#include "stdlib.h"	    	
//OLEDģʽ����
//0:4�ߴ���ģʽ
//1:����8080ģʽ
#define OLED_MODE 0
#define SIZE 			8
#define XLevelL			0x00
#define XLevelH			0x10
#define Max_Column	128
#define Max_Row			64
#define	Brightness	0xFF 
#define X_WIDTH 		128
#define Y_WIDTH 		64	    						  
//-----------------OLED�˿ڶ���----------------  					   
#define OLED_CS_Clr()  GPIO_ResetBits(GPIOE,GPIO_Pin_7)//CS
#define OLED_CS_Set()  GPIO_SetBits(GPIOE,GPIO_Pin_7)

#define OLED_RST_Clr() GPIO_ResetBits(GPIOE,GPIO_Pin_8)//RES
#define OLED_RST_Set() GPIO_SetBits(GPIOE,GPIO_Pin_8)

#define OLED_DC_Clr() GPIO_ResetBits(GPIOE,GPIO_Pin_9)//DC
#define OLED_DC_Set() GPIO_SetBits(GPIOE,GPIO_Pin_9)

#define OLED_WR_Clr() GPIO_ResetBits(GPIOG,GPIO_Pin_14)
#define OLED_WR_Set() GPIO_SetBits(GPIOG,GPIO_Pin_14)

#define OLED_RD_Clr() GPIO_ResetBits(GPIOG,GPIO_Pin_13)
#define OLED_RD_Set() GPIO_SetBits(GPIOG,GPIO_Pin_13)

//PC0~7,��Ϊ������
#define DATAOUT(x) GPIO_Write(GPIOC,x);//���  
//ʹ��4�ߴ��нӿ�ʱʹ�� 

#define OLED_SCLK_Clr() GPIO_ResetBits(GPIOE,GPIO_Pin_10)//CLK
#define OLED_SCLK_Set() GPIO_SetBits(GPIOE,GPIO_Pin_10)

#define OLED_SDIN_Clr() GPIO_ResetBits(GPIOE,GPIO_Pin_11)//DIN
#define OLED_SDIN_Set() GPIO_SetBits(GPIOE,GPIO_Pin_11)

 		     
#define OLED_CMD  0	//д����
#define OLED_DATA 1	//д����


//OLED�����ú���
void OLED_WR_Byte(u8 dat,u8 cmd);	    
void OLED_Display_On(void);
void OLED_Display_Off(void);	   							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawProcess(u8 x,u8 y);
void OLED_DrawIcon(u8 x,u8 y,u8 num);
void OLED_ShowChar(u8 x,u8 y,u8 chr);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);
void OLED_ShowString(u8 x,u8 y, u8 *chr);	 
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_ShowCHinese(u8 x,u8 y,u8 no);
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);
#endif  
	 

