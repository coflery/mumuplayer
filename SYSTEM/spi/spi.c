#include "spi.h"
#include "usart.h"

/*******************************************************************************
* ��    ��: SPI_SetSpeed_SPI1
* ��    ��: SPI�����ٶ�Ϊ����
* ��ڲ���: uint8_t SpeedSet
* ���ڲ���: 
* ˵    ��: 
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
		case SPI_SPEED_HIGH://����ģʽ
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
					break;
		case SPI_SPEED_16:
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
					break;
		case SPI_SPEED_64:
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
					break;
		case SPI_SPEED_LOW://����ģʽ
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
					break;
		default:
				printf("\r\nSPI_SetSpeedֵ�쳣");
				break;
	}
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
}


/*******************************************************************************
* ��    ��: SPI_SetSpeed_SPI2
* ��    ��: SPI�����ٶ�Ϊ����
* ��ڲ���: uint8_t SpeedSet
* ���ڲ���: 
* ˵    ��: 
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
		case SPI_SPEED_HIGH://����ģʽ
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
					break;
		case SPI_SPEED_16:
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
					break;
		case SPI_SPEED_64:
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
					break;
		case SPI_SPEED_LOW://����ģʽ
					SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
					break;
		default:
				printf("\r\nSPI_SetSpeedֵ�쳣");
				break;
	}
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);
}


/*******************************************************************************
* ��    ��: SPI_ReadWriteByte
* ��    ��: SPI��дһ���ֽ�(������ɺ󷵻ر���ͨѶ��ȡ������)
* ��ڲ���: uint8_t TxData �����͵���
* ���ڲ���: 
* ���ز���: uint8_t RxData �յ�����
* ˵    ��: 
*******************************************************************************/
uint8_t SPI_ReadWriteByte(uint8_t TxData)//SPI1�Ͻ������SPI_SD
{
    uint8_t RxData = 0;
		//�ȴ����ͻ�������
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
		//��һ���ֽ�
    SPI_I2S_SendData(SPI1, TxData);
		//�ȴ����ݽ���
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
		//ȡ����
    RxData = SPI_I2S_ReceiveData(SPI1);

    return (uint8_t)RxData; 
}

/*******************************************************************************
* ��    ��: SPI_ReadWriteByte_SPI2
* ��    ��: SPI2��дһ���ֽ�(������ɺ󷵻ر���ͨѶ��ȡ������)
* ��ڲ���: uint8_t TxData �����͵���
* ���ڲ���: 
* ���ز���: uint8_t RxData �յ�����
* ˵    ��: 
*******************************************************************************/
uint8_t SPI_ReadWriteByte_SPI2(uint8_t TxData)//SPI2�Ͻ������VS1053
{
    uint8_t RxData = 0;
		//�ȴ����ͻ�������
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		//��һ���ֽ�
    SPI_I2S_SendData(SPI2, TxData);
		//�ȴ����ݽ���
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
		//ȡ����
    RxData = SPI_I2S_ReceiveData(SPI2);

    return (uint8_t)RxData; 
}
