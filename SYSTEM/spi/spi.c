#include "spi.h"
#include "usart.h"

/*******************************************************************************
* 名    称: SPI_SetSpeed_SPI1
* 功    能: SPI设置速度为高速
* 入口参数: uint8_t SpeedSet
* 出口参数: 
* 说    明: 
*******************************************************************************/
void SPI_SetSpeed_SPI1(uint8_t SpeedSet)
{
	SPI_InitTypeDef SPI_InitStructure;
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	 switch(SpeedSet)
	{
		case SPI_SPEED_HIGH://高速模式
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
					break;
		case SPI_SPEED_16:
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
					break;
		case SPI_SPEED_64:
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
					break;
		case SPI_SPEED_LOW://低速模式
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
					break;
		default:
				printf("\r\nSPI_SetSpeed值异常");
				break;
	}
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
}


/*******************************************************************************
* 名    称: SPI_SetSpeed_SPI2
* 功    能: SPI设置速度为高速
* 入口参数: uint8_t SpeedSet
* 出口参数: 
* 说    明: 
*******************************************************************************/
void SPI_SetSpeed_SPI2(uint8_t SpeedSet)
{
	SPI_InitTypeDef SPI_InitStructure;
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	 switch(SpeedSet)
	{
		case SPI_SPEED_HIGH://高速模式
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
					break;
		case SPI_SPEED_16:
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
					break;
		case SPI_SPEED_64:
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
					break;
		case SPI_SPEED_LOW://低速模式
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
					break;
		default:
				printf("\r\nSPI_SetSpeed值异常");
				break;
	}
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);
}


/*******************************************************************************
* 名    称: SPI_ReadWriteByte
* 功    能: SPI读写一个字节(发送完成后返回本次通讯读取的数据)
* 入口参数: uint8_t TxData 待发送的数
* 出口参数: 
* 返回参数: uint8_t RxData 收到的数
* 说    明: 
*******************************************************************************/
uint8_t SPI_ReadWriteByte(uint8_t TxData)//SPI1上接入的是SPI_SD
{
    uint8_t RxData = 0;
		//等待发送缓冲区空
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
		//发一个字节
    SPI_I2S_SendData(SPI1, TxData);
		//等待数据接收
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
		//取数据
    RxData = SPI_I2S_ReceiveData(SPI1);

    return (uint8_t)RxData; 
}

/*******************************************************************************
* 名    称: SPI_ReadWriteByte_SPI2
* 功    能: SPI2读写一个字节(发送完成后返回本次通讯读取的数据)
* 入口参数: uint8_t TxData 待发送的数
* 出口参数: 
* 返回参数: uint8_t RxData 收到的数
* 说    明: 
*******************************************************************************/
uint8_t SPI_ReadWriteByte_SPI2(uint8_t TxData)//SPI2上接入的是VS1053
{
    uint8_t RxData = 0;
		//等待发送缓冲区空
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		//发一个字节
    SPI_I2S_SendData(SPI2, TxData);
		//等待数据接收
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
		//取数据
    RxData = SPI_I2S_ReceiveData(SPI2);

    return (uint8_t)RxData; 
}
