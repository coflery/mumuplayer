////////////////////////////////////////////////////////////////////////////////
//  �� �� ��   : oled.c
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

#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"  	 
#include "delay.h"
//OLED���Դ�
//��Ÿ�ʽ����.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 			   

#if OLED_MODE==1
//��SSD1106д��һ���ֽڡ�
//dat:Ҫд�������/����
//cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
void OLED_WR_Byte(u8 dat,u8 cmd)
{
	DATAOUT(dat);	    
	if(cmd)
	  OLED_DC_Set();
	else 
	  OLED_DC_Clr();		   
	OLED_CS_Clr();
	OLED_WR_Clr();	 
	OLED_WR_Set();
	OLED_CS_Set();	  
	OLED_DC_Set();	 
} 	    	    
#else
//��SSD1106д��һ���ֽڡ�
//dat:Ҫд�������/����
//cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
void OLED_WR_Byte(u8 dat,u8 cmd)
{	
	u8 i;			  
	if(cmd)
	  OLED_DC_Set();
	else 
	  OLED_DC_Clr();		  
	OLED_CS_Clr();
	for(i=0;i<8;i++)
	{			  
		OLED_SCLK_Clr();
		if(dat&0x80)
		{
			OLED_SDIN_Set();
		}
		else 
		{
			OLED_SDIN_Clr();
		}
		OLED_SCLK_Set();
		dat<<=1;   
	}				 		  
	OLED_CS_Set();
	OLED_DC_Set();   	  
} 
#endif
void OLED_Set_Pos(unsigned char x, unsigned char y) 
{ 
	OLED_WR_Byte(0xb0+y,OLED_CMD);
	OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
	OLED_WR_Byte((x&0x0f)|0x01,OLED_CMD);
}   	  
//����OLED��ʾ    
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC����
	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//�ر�OLED��ʾ     
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC����
	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}		   			 
//��������,������,������Ļ�Ǻ�ɫ,��û����һ��!!!	  
void OLED_Clear(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //����ҳ��ַ��0~7��
		OLED_WR_Byte (0x02,OLED_CMD);      //������ʾλ�á��е͵�ַ
		OLED_WR_Byte (0x10,OLED_CMD);      //������ʾλ�á��иߵ�ַ   
		for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA); 
	} //������ʾ
}

//��ָ��λ����ʾһ���ַ�,���������ַ�
//x:0~127
//y:0~63
//mode:0,������ʾ;1,������ʾ				 
//size:ѡ������ 16/12 
void OLED_ShowChar(u8 x,u8 y,u8 chr)
{      	
	unsigned char c=0,i=0;	
		c=chr-' ';//�õ�ƫ�ƺ��ֵ			
		if(x>Max_Column-1){x=0;y=y+2;}
		if(SIZE ==16)
			{
			OLED_Set_Pos(x,y);	
			for(i=0;i<8;i++)
			OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
			OLED_Set_Pos(x,y+1);
			for(i=0;i<8;i++)
			OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
			}
		else {	
				OLED_Set_Pos(x,y+1);
				for(i=0;i<6;i++)
				OLED_WR_Byte(F6x8[c][i],OLED_DATA);
				}
}
//m^n����
u32 oled_pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}				  
//��ʾ2������
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//mode:ģʽ	0,���ģʽ;1,����ģʽ
//num:��ֵ(0~4294967295);	 		  
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size/2)*t,y,' ');
				continue;
			}
			else enshow=1; 
		 	 
		}
	 	OLED_ShowChar(x+(size/2)*t,y,temp+'0');
	}
} 
//��ʾһ���ַ��Ŵ�
void OLED_ShowString(u8 x,u8 y,u8 *chr)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{		OLED_ShowChar(x,y,chr[j]);
			x+=8;
		if(x>120){x=0;y+=2;}
			j++;
	}
}
//��ʾͼ��
void OLED_DrawIcon(u8 x,u8 y,u8 num)
{
	unsigned char i;
	OLED_Set_Pos(x,y);//���ó�ʼλ��
	for (i=0; i<6; i++)//��һ��6*8��ͼ��
			{
				OLED_WR_Byte(Icon[num][i],OLED_DATA);
			}
}
//��������
void OLED_DrawProcess(u8 x,u8 y)
{
	unsigned char i=117;
	OLED_Set_Pos(x,y);//���ó�ʼλ��
	OLED_WR_Byte(0x7f,OLED_DATA);
	while(i--)
	{
		OLED_WR_Byte(0x41,OLED_DATA);
	}
		OLED_WR_Byte(0x7f,OLED_DATA);
}
//��ʾ����
void OLED_ShowCHinese(u8 x,u8 y,u8 no)
{      			    
	u8 t,adder=0;
	OLED_Set_Pos(x,y);	
    for(t=0;t<16;t++)
		{
				OLED_WR_Byte(Hzk[2*no][t],OLED_DATA);
				adder+=1;
     }	
		OLED_Set_Pos(x,y+1);	
    for(t=0;t<16;t++)
			{	
				OLED_WR_Byte(Hzk[2*no+1][t],OLED_DATA);
				adder+=1;
      }					
}
/***********������������ʾ��ʾBMPͼƬ128��64��ʼ������(x,y),x�ķ�Χ0��127��yΪҳ�ķ�Χ0��7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{ 	
 unsigned int j=0;
 unsigned char x,y;
  
  if(y1%8==0) y=y1/8;      
  else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
    for(x=x0;x<x1;x++)
	    {      
	    	OLED_WR_Byte(BMP[j++],OLED_DATA);	    	
	    }
	}
} 

//��ʼ��SSD1306					    
void OLED_Init(void)
{
 	GPIO_InitTypeDef  GPIO_InitStructure;
	
//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOG, ENABLE);	 //ʹ��PC,D,G�˿�ʱ��,�Ѿ���main���

	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;	 //PD3,PD6�������  
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//�ٶ�50MHz
 	GPIO_Init(GPIOE, &GPIO_InitStructure);	  //��ʼ��GPIODE,6
 	GPIO_SetBits(GPIOE,GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11);	//PD3,PD6 �����
	

/*����SPI����PE����Ŀ�
 #if OLED_MODE==1
 
 	GPIO_InitStructure.GPIO_Pin =0xFF; //PC0~7 OUT�������
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOC,0xFF); 				 //PC0~7�����

 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	 //PG13,14,15 OUT�������
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOG,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);						 //PG13,14,15 OUT  �����

 #else
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;	 //PC0,1 OUT�������
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOC,GPIO_Pin_0|GPIO_Pin_1);						 //PC0,1 OUT  �����

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	 //PG15 OUT�������	  RST
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOG,GPIO_Pin_15);			 			 //PG15 OUT  �����
 #endif
*/
	OLED_RST_Set();
	delay_ms(100);
	OLED_RST_Clr();
	delay_ms(100);
	OLED_RST_Set();
	
	OLED_WR_Byte(0xAE,OLED_CMD);//--�ر���ʾ
	OLED_WR_Byte(0x10,OLED_CMD);//--���ø��е�ַ
	OLED_WR_Byte(0x40,OLED_CMD);//--���ÿ�ʼ�е�ַ,[5:0],����(0x00-0x3F)
	OLED_WR_Byte(0x81,OLED_CMD);//--�Աȶ�����
	OLED_WR_Byte(0xCF,OLED_CMD);//--����SEG���ȵ���
	OLED_WR_Byte(0xA1,OLED_CMD);//--����SEG/��ӳ��(���ҷ���/����)(0xa0/0xa1)
	OLED_WR_Byte(0xC8,OLED_CMD);//--����COM/��ɨ�跽��(���·���/����)(0xc0/0xc8)
	OLED_WR_Byte(0xA8,OLED_CMD);//--���ø�����(1��64)
	OLED_WR_Byte(0x3F,OLED_CMD);//--1/64ռ�ձ�
	OLED_WR_Byte(0xD3,OLED_CMD);//--������ʾƫ�Ƽ��� (0x00-0x3F)
	OLED_WR_Byte(0x00,OLED_CMD);//--Ĭ��Ϊ0
	OLED_WR_Byte(0xD5,OLED_CMD);//--����ʱ�ӷ�Ƶ��/��Ƶ��
	OLED_WR_Byte(0x80,OLED_CMD);//--���÷�Ƶ��=1(bit 3-0),����ʱ��Ϊ100֡/��(bit 7-4)
	OLED_WR_Byte(0xD9,OLED_CMD);//--����Ԥ�������
	OLED_WR_Byte(0xF1,OLED_CMD);//--����Ԥ���15ʱ��&�ŵ�1ʱ��
	OLED_WR_Byte(0xDA,OLED_CMD);//--Ӳ��COM��������
	OLED_WR_Byte(0x12,OLED_CMD);//--��ѡCOM��������
	OLED_WR_Byte(0xDB,OLED_CMD);//--VCOMH��ѹ����
	OLED_WR_Byte(0x20,OLED_CMD);//--ҳ��ַģʽ(0x00/0x01/0x02)
	OLED_WR_Byte(0x02,OLED_CMD);//--00,�е�ַģʽ;01,�е�ַģʽ;10,ҳ��ַģʽ;Ĭ��10
	OLED_WR_Byte(0x8D,OLED_CMD);//--��ɱÿ���
	OLED_WR_Byte(0x14,OLED_CMD);//--(0x10):�ر�
	OLED_WR_Byte(0xA4,OLED_CMD);//--ȫ����ʾ(����/����)(0xa4/0xa5)
	OLED_WR_Byte(0xA6,OLED_CMD);//--��ʾ��ʽ(��ɫ/��ɫ)(0xa6/0xa7)
	OLED_WR_Byte(0xAF,OLED_CMD);//--������ʾ
	OLED_Clear();
	OLED_Set_Pos(0,0); 
}  

