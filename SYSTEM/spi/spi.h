#ifndef __SPI_H
#define __SPI_H
#include "sys.h"

/* SPI�����ٶ�����*/

#define SPI_SPEED_HIGH  8
#define SPI_SPEED_16		16
#define SPI_SPEED_64    64
#define SPI_SPEED_LOW   128

/*SD��SD_CS��������*/
#define			SD_CS  PAout(4) 				 //SD��Ƭѡ��	
#define	Clr_SD_CS  {PAout(4)=0;}
#define	Set_SD_CS  {PAout(4)=1;}

void SPI_SetSpeed_SPI1(uint8_t SpeedSet);		//����SPI2�ٶ� 
void SPI_SetSpeed_SPI2(uint8_t SpeedSet);		//����SPI2�ٶ� 
uint8_t SPI_ReadWriteByte(uint8_t TxData);	 	 //SPI1���߶�дһ���ֽ�for SD
uint8_t SPI_ReadWriteByte_SPI2(uint8_t TxData);//SPI2���߶�дһ���ֽ�for VS10XX

#endif
