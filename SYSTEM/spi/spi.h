#ifndef __SPI_H
#define __SPI_H
#include "sys.h"

/* SPI总线速度设置*/

#define SPI_SPEED_HIGH  8
#define SPI_SPEED_16		16
#define SPI_SPEED_64    64
#define SPI_SPEED_LOW   128

/*SD卡SD_CS引脚配置*/
#define			SD_CS  PAout(4) 				 //SD卡片选脚	
#define	Clr_SD_CS  {PAout(4)=0;}
#define	Set_SD_CS  {PAout(4)=1;}

void SPI_SetSpeed_SPI1(uint8_t SpeedSet);		//设置SPI2速度 
void SPI_SetSpeed_SPI2(uint8_t SpeedSet);		//设置SPI2速度 
uint8_t SPI_ReadWriteByte(uint8_t TxData);	 	 //SPI1总线读写一个字节for SD
uint8_t SPI_ReadWriteByte_SPI2(uint8_t TxData);//SPI2总线读写一个字节for VS10XX

#endif
