////////////////////////////////////////////////////////////////////////////////
//  文 件 名   : oled.c
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

#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"  	 
#include "delay.h"
//OLED的显存
//存放格式如下.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 			   

#if OLED_MODE==1
//向SSD1106写入一个字节。
//dat:要写入的数据/命令
//cmd:数据/命令标志 0,表示命令;1,表示数据;
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
//向SSD1106写入一个字节。
//dat:要写入的数据/命令
//cmd:数据/命令标志 0,表示命令;1,表示数据;
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
//开启OLED显示    
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//关闭OLED显示     
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}		   			 
//清屏函数,清完屏,整个屏幕是黑色,和没点亮一样!!!	  
void OLED_Clear(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte (0x02,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA); 
	} //更新显示
}

//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示				 
//size:选择字体 16/12 
void OLED_ShowChar(u8 x,u8 y,u8 chr)
{      	
	unsigned char c=0,i=0;	
		c=chr-' ';//得到偏移后的值			
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
//m^n函数
u32 oled_pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}				  
//显示2个数字
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//mode:模式	0,填充模式;1,叠加模式
//num:数值(0~4294967295);	 		  
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
//显示一个字符号串
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
//显示图标
void OLED_DrawIcon(u8 x,u8 y,u8 num)
{
	unsigned char i;
	OLED_Set_Pos(x,y);//设置初始位置
	for (i=0; i<6; i++)//画一个6*8的图标
			{
				OLED_WR_Byte(Icon[num][i],OLED_DATA);
			}
}
//画进度条
void OLED_DrawProcess(u8 x,u8 y)
{
	unsigned char i=117;
	OLED_Set_Pos(x,y);//设置初始位置
	OLED_WR_Byte(0x7f,OLED_DATA);
	while(i--)
	{
		OLED_WR_Byte(0x41,OLED_DATA);
	}
		OLED_WR_Byte(0x7f,OLED_DATA);
}
//显示汉字
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
/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
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

//初始化SSD1306					    
void OLED_Init(void)
{
 	GPIO_InitTypeDef  GPIO_InitStructure;
	
//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOG, ENABLE);	 //使能PC,D,G端口时钟,已经在main完成

	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;	 //PD3,PD6推挽输出  
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(GPIOE, &GPIO_InitStructure);	  //初始化GPIODE,6
 	GPIO_SetBits(GPIOE,GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11);	//PD3,PD6 输出高
	

/*四线SPI不用PE以外的口
 #if OLED_MODE==1
 
 	GPIO_InitStructure.GPIO_Pin =0xFF; //PC0~7 OUT推挽输出
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOC,0xFF); 				 //PC0~7输出高

 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	 //PG13,14,15 OUT推挽输出
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOG,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);						 //PG13,14,15 OUT  输出高

 #else
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;	 //PC0,1 OUT推挽输出
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOC,GPIO_Pin_0|GPIO_Pin_1);						 //PC0,1 OUT  输出高

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	 //PG15 OUT推挽输出	  RST
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOG,GPIO_Pin_15);			 			 //PG15 OUT  输出高
 #endif
*/
	OLED_RST_Set();
	delay_ms(100);
	OLED_RST_Clr();
	delay_ms(100);
	OLED_RST_Set();
	
	OLED_WR_Byte(0xAE,OLED_CMD);//--关闭显示
	OLED_WR_Byte(0x10,OLED_CMD);//--设置高列地址
	OLED_WR_Byte(0x40,OLED_CMD);//--设置开始行地址,[5:0],行数(0x00-0x3F)
	OLED_WR_Byte(0x81,OLED_CMD);//--对比度设置
	OLED_WR_Byte(0xCF,OLED_CMD);//--设置SEG亮度电流
	OLED_WR_Byte(0xA1,OLED_CMD);//--设置SEG/列映射(左右反置/正常)(0xa0/0xa1)
	OLED_WR_Byte(0xC8,OLED_CMD);//--设置COM/行扫描方向(上下反置/正常)(0xc0/0xc8)
	OLED_WR_Byte(0xA8,OLED_CMD);//--设置复用率(1到64)
	OLED_WR_Byte(0x3F,OLED_CMD);//--1/64占空比
	OLED_WR_Byte(0xD3,OLED_CMD);//--设置显示偏移计数 (0x00-0x3F)
	OLED_WR_Byte(0x00,OLED_CMD);//--默认为0
	OLED_WR_Byte(0xD5,OLED_CMD);//--设置时钟分频比/振荡频率
	OLED_WR_Byte(0x80,OLED_CMD);//--设置分频比=1(bit 3-0),设置时钟为100帧/秒(bit 7-4)
	OLED_WR_Byte(0xD9,OLED_CMD);//--设置预充电周期
	OLED_WR_Byte(0xF1,OLED_CMD);//--设置预充电15时钟&放电1时钟
	OLED_WR_Byte(0xDA,OLED_CMD);//--硬件COM引脚配置
	OLED_WR_Byte(0x12,OLED_CMD);//--备选COM引脚配置
	OLED_WR_Byte(0xDB,OLED_CMD);//--VCOMH电压倍率
	OLED_WR_Byte(0x20,OLED_CMD);//--页地址模式(0x00/0x01/0x02)
	OLED_WR_Byte(0x02,OLED_CMD);//--00,列地址模式;01,行地址模式;10,页地址模式;默认10
	OLED_WR_Byte(0x8D,OLED_CMD);//--电荷泵开关
	OLED_WR_Byte(0x14,OLED_CMD);//--(0x10):关闭
	OLED_WR_Byte(0xA4,OLED_CMD);//--全屏显示(白屏/黑屏)(0xa4/0xa5)
	OLED_WR_Byte(0xA6,OLED_CMD);//--显示方式(正色/反色)(0xa6/0xa7)
	OLED_WR_Byte(0xAF,OLED_CMD);//--开启显示
	OLED_Clear();
	OLED_Set_Pos(0,0); 
}  


