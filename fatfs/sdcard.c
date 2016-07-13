/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "spi.h"
#include "sdcard.h"

//********************************************************************************
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//SD�� ��������		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/11/28 
//�汾��V1.1
//2010/5/13									   
//������һЩ��ʱ,ʵ�����֧��TF��(1G),��ʿ��2G,4G 16G SD��
//2010/6/24
//������uint8_t SD_GetResponse(uint8_t Response)����
//�޸���uint8_t SD_WaitDataReady(void)����
//������USB������֧�ֵ�uint8_t MSD_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);
//��uint8_t MSD_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite);��������
//�ȴ�SD����Ӧ
//Response:Ҫ�õ��Ļ�Ӧֵ
//����ֵ:0,�ɹ��õ��˸û�Ӧֵ
//����,�õ���Ӧֵʧ��
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************

uint8_t  SD_Type=0;//SD��������

uint8_t SD_GetResponse(uint8_t Response)
{
	uint16_t Count=0xFFF;                                     //�ȴ�����	   						  
	while ((SPI_ReadWriteByte(0XFF)!=Response)&&Count)Count--;//�ȴ��õ�׼ȷ�Ļ�Ӧ  	  
	if (Count==0)return MSD_RESPONSE_FAILURE;                 //�õ���Ӧʧ��   
	else return MSD_RESPONSE_NO_ERROR;                        //��ȷ��Ӧ
}
//�ȴ�SD��д�����
//����ֵ:0,�ɹ� 
//����,�������
uint8_t SD_WaitDataReady(void)
{
    uint8_t r1=MSD_DATA_OTHER_ERROR;
    uint32_t retry;
    retry=0;
    do
    {
        r1=SPI_ReadWriteByte(0xFF)&0X1F;//������Ӧ
        if(retry==0xfffe)return 1; 
		retry++;
		switch (r1)
		{					   
			case MSD_DATA_OK:					//���ݽ�����ȷ
				r1=MSD_DATA_OK;
				break;  
			case MSD_DATA_CRC_ERROR:  //CRCУ�����
				return MSD_DATA_CRC_ERROR;  
			case MSD_DATA_WRITE_ERROR://����д�����
				return MSD_DATA_WRITE_ERROR;  
			default:									//δ֪����    
				r1=MSD_DATA_OTHER_ERROR;
				break;	 
		}   
    }while(r1==MSD_DATA_OTHER_ERROR); //���ݴ���ʱһֱ�ȴ�
	retry=0;
	while(SPI_ReadWriteByte(0XFF)==0)   //��������Ϊ0,�����ݻ�δд���
	{
		retry++;
		//Delay_us(10);                   //SD��д�ȴ���Ҫ�ϳ���ʱ��
		if(retry>=0XFFFFFFFE)return 0XFF; //�ȴ�ʧ��
	};	    
    return 0;                         //�ɹ�
}
uint8_t SD_WaitReady(void)
{
    u8 r1;
    u16 retry;
    retry = 0;
    do
    {
        r1 = SPI_ReadWriteByte(0xFF);
        if(retry==0xfffe)
        {
            return 1;
        }
    }while(r1!=0xFF);

    return 0;
}	 
//��SD������һ������
//����: uint8_t cmd   ���� 
//      uint32_t arg  �������
//      uint8_t crc   crcУ��ֵ	   
//����ֵ:SD�����ص���Ӧ															  
uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    uint8_t r1;	
	uint8_t Retry=0;	         
	SD_CS=1;
  SPI_ReadWriteByte(0xff);//����д������ʱ
	SPI_ReadWriteByte(0xff);     
 	SPI_ReadWriteByte(0xff);  	 
  SD_CS=0; 								//Ƭѡ���õͣ�ѡ��SD��

    //����
    SPI_ReadWriteByte(cmd | 0x40);//�ֱ�д������
    SPI_ReadWriteByte(arg >> 24);
    SPI_ReadWriteByte(arg >> 16);
    SPI_ReadWriteByte(arg >> 8);
    SPI_ReadWriteByte(arg);
    SPI_ReadWriteByte(crc); 
    //�ȴ���Ӧ����ʱ�˳�
    while((r1=SPI_ReadWriteByte(0xFF))==0xFF)
    {
       Retry++;	    
       if(Retry>200)break; 
    }
		
    SD_CS=1;                  //�ر�Ƭѡ
    SPI_ReadWriteByte(0xFF);	//�������϶�������8��ʱ�ӣ���SD�����ʣ�µĹ���
		
    return r1;    						//����״ֵ̬
}		  																				 
//��SD������һ������(�����ǲ�ʧ��Ƭѡ�����к������ݴ�����
//����:uint8_t  cmd  ����
//     uint8_t  crc  crcУ��ֵ
//     uint32_t arg  �������
//����ֵ:SD�����ص���Ӧ
uint8_t SD_SendCommand_NoDeassert(uint8_t cmd, uint32_t arg, uint8_t crc)
{
	uint8_t Retry=0;	         
	uint8_t r1;			   
    SPI_ReadWriteByte(0xff);       //����д������ʱ
	SPI_ReadWriteByte(0xff);  	 	 
    SD_CS=0;                       //Ƭѡ���õͣ�ѡ��SD��	   
    //����
    SPI_ReadWriteByte(cmd | 0x40); //�ֱ�д������
    SPI_ReadWriteByte(arg >> 24);
    SPI_ReadWriteByte(arg >> 16);
    SPI_ReadWriteByte(arg >> 8);
    SPI_ReadWriteByte(arg);
    SPI_ReadWriteByte(crc);
    //�ȴ���Ӧ����ʱ�˳�
    while((r1=SPI_ReadWriteByte(0xFF))==0xFF)
    {
        Retry++;
        if(Retry>200)break;
    }
    return r1;				//������Ӧֵ
}
//��SD�����õ�����ģʽ
//����ֵ:0,�ɹ�����
//       1,����ʧ��
uint8_t SD_Idle_Sta(void)
{
	uint16_t i;
	uint8_t retry;
    for(i=0;i<0xf00;i++);													//����ʱ���ȴ�SD���ϵ����	 
    for(i=0;i<10;i++)SPI_ReadWriteByte(0xFF);     //�Ȳ���>74�����壬��SD���Լ���ʼ�����
    //-----------------SD����λ��idle��ʼ-----------------
    //ѭ����������CMD0��ֱ��SD������0x01,����IDLE״̬
    //��ʱ��ֱ���˳�
    retry = 0;
    do
    {	   
        i = SD_SendCommand(CMD0, 0, 0x95);//����CMD0����SD������IDLE״̬
        retry++;
    }
		while((i!=0x01)&&(retry<200));
    //����ѭ������ԭ�򣺳�ʼ���ɹ� ���� ���Գ�ʱ
    if(retry==200)return 1; //ʧ��
	return 0;                 //�ɹ�	 						  
}														    

void SD_IO_Init(void)			//��ʼ��SD��GPIO
{		
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;
	
	  /* ���� PA5/6/7    �� SCK, MISO,MOSI(for MMC_SD) */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		/* ���� PA4        �� SD_CS             ��������*/
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
		/* SPI1 ���� -------------------------------------------------------------*/
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //˫��˫��ģʽ
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	 										 //SPIΪ��ģʽ
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;									 //���͵�����λΪ8
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;												 //����״̬ʱ��SCK���ָߵ�ƽ
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;											 //���ݲ����ӵڶ���ʱ�ӱ��ؿ�ʼ
 	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;												   //����������豸����
 	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;//������Ϊ100�� fPCLK/32
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;								 //�ȴ����λ
 	SPI_InitStructure.SPI_CRCPolynomial = 7;													 
 	SPI_Init(SPI1, &SPI_InitStructure);																 //��ʼ��SPI1
  SPI_Cmd(SPI1, ENABLE);																					   //��  SPI1
	SD_CS = 1;
}

//��ʼ��SD��
//����ɹ�����,����Զ�����SPI�ٶ�Ϊ18Mhz
//����ֵ:0��NO_ERR
//       1��TIME_OUT
//      99��NO_CARD																 
uint8_t SD_Init(void)
{		
    uint8_t r1=0;      // ���SD���ķ���ֵ
    uint16_t retry=0;  // �������г�ʱ����
    uint8_t buff[6];
	
	SD_IO_Init();		//��ʼ��SD��GPIO
 	SPI_SetSpeed_SPI1(SPI_SPEED_LOW);//���õ�����ģʽ		 
	SD_CS=1;	
    if(SD_Idle_Sta()) return 1;//��ʱ����1 ���õ�idle ģʽʧ��	  
    //-----------------SD����λ��idle����-----------------	 
    //��ȡ��Ƭ��SD�汾��Ϣ
 	SD_CS=0;	
	r1 = SD_SendCommand_NoDeassert(8, 0x1aa,0x87);	     
  //�����Ƭ�汾��Ϣ��v1.0�汾�ģ���r1=0x05����������³�ʼ��
	//����û��Ӧ
    if(r1 == 0x05)
    {
        //���ÿ�����ΪSDV1.0����������⵽ΪMMC�������޸�ΪMMC
        SD_Type = SD_TYPE_V1;	   
        //�����V1.0����CMD8ָ���û�к�������
        //Ƭѡ�øߣ�������������
        SD_CS=1;
        //�෢8��CLK����SD������������
        SPI_ReadWriteByte(0xFF);	  
        //-----------------SD����MMC����ʼ����ʼ-----------------	 
        //������ʼ��ָ��CMD55+ACMD41
        // �����Ӧ��˵����SD�����ҳ�ʼ�����
        // û�л�Ӧ��˵����MMC�������������Ӧ��ʼ��
        retry = 0;
        do
        {
            //�ȷ�CMD55��Ӧ����0x01���������
            r1 = SD_SendCommand(CMD55, 0, 0);
            if(r1 == 0XFF)return r1;//ֻҪ����0xff,�ͽ��ŷ���	  
            //�õ���ȷ��Ӧ�󣬷�ACMD41��Ӧ�õ�����ֵ0x00����������200��
            r1 = SD_SendCommand(ACMD41, 0, 0);
            retry++;
        }while((r1!=0x00) && (retry<400));
        // �ж��ǳ�ʱ���ǵõ���ȷ��Ӧ
        // ���л�Ӧ����SD����û�л�Ӧ����MMC��	  
        //----------MMC�������ʼ��������ʼ------------
//	retry=400;
        if(retry==400)
        {
            retry = 0;
            //����MMC����ʼ�����û�в��ԣ�
            do
            {
                r1 = SD_SendCommand(1,0,0);
                retry++;
            }while((r1!=0x00)&& (retry<400));
            if(retry==400)return 1;   //MMC����ʼ����ʱ		    
            //д�뿨����
            SD_Type = SD_TYPE_MMC;
        }
        //----------MMC�������ʼ����������------------	    
        //����SPIΪ����ģʽ
    SPI_SetSpeed_SPI1(SPI_SPEED_HIGH);   
		SPI_ReadWriteByte(0xFF);	 
        //��ֹCRCУ��	   
		r1 = SD_SendCommand(CMD59, 0, 0x95);
        if(r1 != 0x00)return r1;  //������󣬷���r1   	   
        //����Sector Size
        r1 = SD_SendCommand(CMD16, 512, 0x95);
        if(r1 != 0x00)return r1;//������󣬷���r1		 
        //-----------------SD����MMC����ʼ������-----------------

    }//SD��ΪV1.0�汾�ĳ�ʼ������	 
    //������V2.0���ĳ�ʼ��
    //������Ҫ��ȡOCR���ݣ��ж���SD2.0����SD2.0HC��
    else if(r1 == 0x01)
    {
        //V2.0�Ŀ���CMD8�����ᴫ��4�ֽڵ����ݣ�Ҫ�����ٽ���������
        buff[0] = SPI_ReadWriteByte(0xFF);  //should be 0x00
        buff[1] = SPI_ReadWriteByte(0xFF);  //should be 0x00
        buff[2] = SPI_ReadWriteByte(0xFF);  //should be 0x01
        buff[3] = SPI_ReadWriteByte(0xFF);  //should be 0xAA
        SD_CS=1;	  
        SPI_ReadWriteByte(0xFF);//the next 8 clocks			 
        //�жϸÿ��Ƿ�֧��2.7V-3.6V�ĵ�ѹ��Χ
        //if(buff[2]==0x01 && buff[3]==0xAA) //���жϣ�����֧�ֵĿ�����
        {	  
            retry = 0;
            //������ʼ��ָ��CMD55+ACMD41
    		do
    		{
    			r1 = SD_SendCommand(CMD55, 0, 0);
    			if(r1!=0x01)return r1;	   
    			r1 = SD_SendCommand(ACMD41, 0x40000000, 0);
                if(retry>200)return r1;  //��ʱ�򷵻�r1״̬  
            }while(r1!=0);		  
            //��ʼ��ָ�����ɣ���������ȡOCR��Ϣ		   
            //-----------����SD2.0���汾��ʼ-----------
            r1 = SD_SendCommand_NoDeassert(CMD58, 0, 0);
            if(r1!=0x00)
			{
				SD_CS=1;//�ͷ�SDƬѡ�ź�
				return r1;  //�������û�з�����ȷӦ��ֱ���˳�������Ӧ��	 
			}//��OCRָ����󣬽�������4�ֽڵ�OCR��Ϣ
            buff[0] = SPI_ReadWriteByte(0xFF);
            buff[1] = SPI_ReadWriteByte(0xFF); 
            buff[2] = SPI_ReadWriteByte(0xFF);
            buff[3] = SPI_ReadWriteByte(0xFF);		 
            //OCR������ɣ�Ƭѡ�ø�
            SD_CS=1;
            SPI_ReadWriteByte(0xFF);	   
            //�����յ���OCR�е�bit30λ��CCS����ȷ����ΪSD2.0����SDHC
            //���CCS=1��SDHC   CCS=0��SD2.0
            if(buff[0]&0x40)SD_Type = SD_TYPE_V2HC;    //���CCS	 
            else SD_Type = SD_TYPE_V2;	    
            //-----------����SD2.0���汾����----------- 
            //����SPIΪ����ģʽ
            SPI_SetSpeed_SPI1(SPI_SPEED_HIGH);  
        }	    
    }
    return r1;
}	 																			   
//��SD���ж���ָ�����ȵ����ݣ������ڸ���λ��
//����: uint8_t *data(��Ŷ������ݵ��ڴ�>len)
//      uint16_t len(���ݳ��ȣ�
//      uint8_t release(������ɺ��Ƿ��ͷ�����CS�ø� 0�����ͷ� 1���ͷţ�	 
//����ֵ:0��NO_ERR
//  	 other��������Ϣ														  
uint8_t SD_ReceiveData(uint8_t *data, uint16_t len, uint8_t release)
{
    // ����һ�δ���
    SD_CS=0;				  	  
	if(SD_GetResponse(0xFE))//�ȴ�SD������������ʼ����0xFE
	{	  
		SD_CS=1;
		return 1;
	}
    while(len--)//��ʼ��������
    {
        *data=SPI_ReadWriteByte(0xFF);
        data++;
    }
    //������2��αCRC��dummy CRC��
    SPI_ReadWriteByte(0xFF);
    SPI_ReadWriteByte(0xFF);
    if(release==RELEASE)//�����ͷ����ߣ���CS�ø�
    {
        SD_CS=1;//�������
        SPI_ReadWriteByte(0xFF);
    }											  					    
    return 0;
}																				  
//��ȡSD����CID��Ϣ��������������Ϣ
//����: uint8_t *cid_data(���CID���ڴ棬����16Byte)
//����ֵ:0��NO_ERR
//		 	 1��TIME_OUT
//	 other��������Ϣ
uint8_t SD_GetCID(uint8_t *cid_data)
{
    uint8_t r1;	   
    //��CMD10�����CID
    r1 = SD_SendCommand(CMD10,0,0xFF);
    if(r1 != 0x00)return r1;  //û������ȷӦ�����˳�������  
    SD_ReceiveData(cid_data,16,RELEASE);//����16���ֽڵ�����	 
    return 0;
}																				  
//��ȡSD����CSD��Ϣ�������������ٶ���Ϣ
//����:uint8_t *cid_data(���CID���ڴ棬����16Byte��	    
//����ֵ:0��NO_ERR
//       1��TIME_OUT
// 	 other��������Ϣ														   
uint8_t SD_GetCSD(uint8_t *csd_data)
{
    uint8_t r1;	 
    r1=SD_SendCommand(CMD9,0,0xFF);//��CMD9�����CSD
    if(r1)return r1;  //û������ȷӦ�����˳�������  
    SD_ReceiveData(csd_data, 16, RELEASE);//����16���ֽڵ����� 
    return 0;
}
//��ȡSD��������(�ֽ�)
//����ֵ:0�� 	ȡ�������� 
//       ����:SD��������(�ֽ�)
uint32_t SD_GetCapacity(void)
{
    uint8_t csd[16];
    uint32_t Capacity;
    uint8_t r1;
    uint16_t i;
		uint16_t temp;  					    
    if(SD_GetCSD(csd)!=0) return 0;	//ȡCSD��Ϣ������ڼ��������0
    if((csd[0]&0xC0)==0x40)					//���ΪSDHC�����������淽ʽ����
    {									  
	    Capacity=((uint32_t)csd[8])<<8;
			Capacity+=(uint32_t)csd[9]+1;	 
      Capacity = (Capacity)*1024;//�õ�������
			Capacity*=512;//�õ��ֽ��� 
    }
    else
    {		    
    	i = csd[6]&0x03;
    	i<<=8;
    	i += csd[7];
    	i<<=2;
    	i += ((csd[8]&0xc0)>>6);
        //C_SIZE_MULT
    	r1 = csd[9]&0x03;
    	r1<<=1;
    	r1 += ((csd[10]&0x80)>>7);	 
    	r1+=2;//BLOCKNR
    	temp = 1;
    	while(r1)
    	{
    		temp*=2;
    		r1--;
    	}
    	Capacity = ((uint32_t)(i+1))*((uint32_t)temp);	 
        // READ_BL_LEN
    	i = csd[5]&0x0f;
        // BLOCK_LEN
    	temp = 1;
    	while(i)
    	{
    		temp*=2;
    		i--;
    	}
        //The final result
    	Capacity *= (uint32_t)temp;//�ֽ�Ϊ��λ 	  
    }
    return (uint32_t)Capacity;
}	    																			    
/*******************************************************************************
* ��    ��: SD_ReadSingleBlock
* ��    ��: ��SD����һ��block
* ��ڲ���: uint32_t sector ȡ��ַ(sectorֵ���������ַ)
* 					uint8_t *buffer ���ݴ洢��ַ(��С����512byte)
* ���ڲ���: 0��   �ɹ�
*						other��ʧ��
* ˵    ��: 
*******************************************************************************/						  
uint8_t SD_ReadSingleBlock(uint32_t sector, uint8_t *buffer)
{
	uint8_t r1;	    
    //����Ϊ����ģʽ
    SPI_SetSpeed_SPI1(SPI_SPEED_HIGH);  		   
    //�������SDHC����������sector��ַ������ת����byte��ַ
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector = sector<<9;
    } 
	r1 = SD_SendCommand(CMD17, sector, 0);//������	 		    
	if(r1 != 0x00)return r1; 		   							  
	r1 = SD_ReceiveData(buffer, 512, RELEASE);		 
	if(r1 != 0)return r1;   //�����ݳ���
    else return 0; 
}

/*******************************************************************************
* ��    ��: MSD_WriteBuffer
* ��    ��: д��MSD/SD����(USB��дSD��)
* ��ڲ���: 
*					pBuffer:���ݴ����
*					WriteAddr:д����׵�ַ
*					NumByteToWrite:Ҫд����ֽ���
* ���ڲ���: 0��   �ɹ�
*						������ʧ��
* ˵    ��: 
*******************************************************************************/
#define BLOCK_SIZE 512			//����SD���Ŀ��С
uint8_t MSD_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite)
{
	uint32_t i,NbrOfBlock = 0, Offset = 0;
	uint32_t sector;
	uint8_t r1;
   	NbrOfBlock = NumByteToWrite / BLOCK_SIZE;//�õ�Ҫд��Ŀ����Ŀ	    
    SD_CS=0;	  		   
	while (NbrOfBlock--)//д��һ������
	{
		sector=WriteAddr+Offset;
		if(SD_Type==SD_TYPE_V2HC)sector>>=9;//ִ������ͨ�����෴�Ĳ���					  			 
		r1=SD_SendCommand_NoDeassert(CMD24,sector,0xff);//д����   
 		if(r1)
		{
			SD_CS=1;
			return 1;//Ӧ����ȷ��ֱ�ӷ��� 	   
	    }
	    SPI_ReadWriteByte(0xFE);//����ʼ����0xFE   
	    //��һ��sector������
	    for(i=0;i<512;i++)SPI_ReadWriteByte(*pBuffer++);  
	    //��2��Byte��dummy CRC
	    SPI_ReadWriteByte(0xff);
	    SPI_ReadWriteByte(0xff); 
 		if(SD_WaitDataReady())//�ȴ�SD������д�����
		{
			SD_CS=1;
			return 2;    
		}
		Offset += 512;	   
	}	    
    //д����ɣ�Ƭѡ��1
    SD_CS=1;
    SPI_ReadWriteByte(0xff);	 
    return 0;
}
/*******************************************************************************
* ��    ��: MSD_ReadBuffer
* ��    ��: ��ȡMSD/SD����(USB��SD��)
* ��ڲ���: 
*					pBuffer:���ݴ����
*					ReadAddr:��ȡ���׵�ַ
*					NNumByteToRead:Ҫ�������ֽ���
* ���ڲ���: 0��   �ɹ�
*						������ʧ��
* ˵    ��: 
*******************************************************************************/
uint8_t MSD_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
	uint32_t NbrOfBlock=0,Offset=0;
	uint32_t sector=0;
	uint8_t r1=0;   	 
  	NbrOfBlock=NumByteToRead/BLOCK_SIZE;	  
    SD_CS=0;
	while (NbrOfBlock --)
	{	
		sector=ReadAddr+Offset;
		if(SD_Type==SD_TYPE_V2HC)sector>>=9;//ִ������ͨ�����෴�Ĳ���					  			 
		r1=SD_SendCommand_NoDeassert(CMD17,sector,0xff);//������	 		    
		if(r1)//����ʹ���
		{
    		SD_CS=1;
			return r1;
		}	   							  
		r1=SD_ReceiveData(pBuffer,512,RELEASE);		 
		if(r1)//��������
		{
    		SD_CS=1;
			return r1;
		}
		pBuffer+=512;	 					    
	  	Offset+=512;				 	 
	}	 	 
    SD_CS=1;
    SPI_ReadWriteByte(0xff);	 
    return 0;
}
/*******************************************************************************
* ��    ��: SD_WriteSingleBlock
* ��    ��: д��SD����һ��block(δʵ�ʲ��Թ�)
* ��ڲ���: 
*					uint32_t sector ������ַ(sectorֵ���������ַ)
*					uint8_t *buffer ���ݴ洢��ַ(��С����512byte)
*					NNumByteToRead:Ҫ�������ֽ���
* ���ڲ���: 0��   �ɹ�
*						������ʧ��
* ˵    ��: 
*******************************************************************************/
uint8_t SD_WriteSingleBlock(uint32_t sector, const uint8_t *data)
{
    uint8_t r1;
    uint16_t i;
    uint16_t retry;

    //����Ϊ����ģʽ
    SPI_SetSpeed_SPI1(SPI_SPEED_HIGH);	   
    //�������SDHC����������sector��ַ������ת����byte��ַ
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector = sector<<9;
    }   
    r1 = SD_SendCommand(CMD24, sector, 0x00);
    if(r1 != 0x00)
    {
        return r1;  //Ӧ����ȷ��ֱ�ӷ���
    }
    
    //��ʼ׼�����ݴ���
    SD_CS=0;
    //�ȷ�3�������ݣ��ȴ�SD��׼����
    SPI_ReadWriteByte(0xff);
    SPI_ReadWriteByte(0xff);
    SPI_ReadWriteByte(0xff);
    //����ʼ����0xFE
    SPI_ReadWriteByte(0xFE);
    //��һ��sector������
    for(i=0;i<512;i++)
    {
        SPI_ReadWriteByte(*data++);
    }
    //��2��Byte��dummy CRC
    SPI_ReadWriteByte(0xff);
    SPI_ReadWriteByte(0xff);
    
    //�ȴ�SD��Ӧ��
    r1 = SPI_ReadWriteByte(0xff);
    if((r1&0x1F)!=0x05)
    {
        SD_CS=1;
        return r1;
    }
    
    //�ȴ��������
    retry = 0;
    while(!SPI_ReadWriteByte(0xff))
    {
        retry++;
        if(retry>0xfffe)        //�����ʱ��д��û����ɣ������˳�
        {
            SD_CS=1;
            return 1;           //д�볬ʱ����1
        }
    }	    
    //д����ɣ�Ƭѡ��1
    SD_CS=1;
    SPI_ReadWriteByte(0xff);

    return 0;
}				           
/*******************************************************************************
* ��    ��: SD_ReadMultiBlock
* ��    ��: ��SD���Ķ��block(ʵ�ʲ��Թ�)
* ��ڲ���: 
*					uint32_t sector ������ַ(sectorֵ���������ַ)
*					uint8_t *buffer ���ݴ洢��ַ(��С����512byte)
*					uint8_t count ������count��block
* ���ڲ���: 0��   �ɹ�
*						������ʧ��
* ˵    ��: 
*******************************************************************************/											  
uint8_t SD_ReadMultiBlock(uint32_t sector, uint8_t *buffer, uint8_t count)
{
    uint8_t r1;	 			 
  SPI_SetSpeed_SPI1(SPI_SPEED_HIGH);//����Ϊ����ģʽ  
 	//�������SDHC����sector��ַת��byte��ַ
    if(SD_Type!=SD_TYPE_V2HC)sector = sector<<9;  
    //SD_WaitDataReady();
    //�����������
	r1 = SD_SendCommand(CMD18, sector, 0);//������
	if(r1 != 0x00)return r1;	 
    do//��ʼ��������
    {
        if(SD_ReceiveData(buffer, 512, NO_RELEASE) != 0x00)break; 
        buffer += 512;
    } while(--count);		 
    //ȫ��������ϣ�����ֹͣ����
    SD_SendCommand(CMD12, 0, 0);
    //�ͷ�����
    SD_CS=1;
    SPI_ReadWriteByte(0xFF);    
    if(count != 0)return count;   //���û�д��꣬����ʣ�����	 
    else return 0;	 
}
/*******************************************************************************
* ��    ��: SD_WriteMultiBlock
* ��    ��: д��SD����N��block(δʵ�ʲ��Թ�)
* ��ڲ���: 
*					uint32_t sector ������ַ(sectorֵ���������ַ)
*					uint8_t *buffer ���ݴ洢��ַ(��С����512byte)
*					uint8_t count д���block��Ŀ
* ���ڲ���: 0��		�ɹ�
*				   ������ ʧ��
* ˵    ��: 
*******************************************************************************/												   
uint8_t SD_WriteMultiBlock(uint32_t sector, const uint8_t *data, uint8_t count)
{
    uint8_t r1;
    uint16_t i;	 		 
    //SPI_SetSpeed_SPI1(SPI_SPEED_HIGH);//����Ϊ����ģʽ	 
    if(SD_Type != SD_TYPE_V2HC)sector = sector<<9;//�������SDHC����������sector��ַ������ת����byte��ַ  
    if(SD_Type != SD_TYPE_MMC) r1 = SD_SendCommand(ACMD23, count, 0x00);//���Ŀ�꿨����MMC��������ACMD23ָ��ʹ��Ԥ����   
    r1 = SD_SendCommand(CMD25, sector, 0x00);//�����д��ָ��
    if(r1 != 0x00)return r1;  //Ӧ����ȷ��ֱ�ӷ���	 
    SD_CS=0;//��ʼ׼�����ݴ���   
    SPI_ReadWriteByte(0xff);//�ȷ�3�������ݣ��ȴ�SD��׼����
    SPI_ReadWriteByte(0xff);   
    //--------������N��sectorд���ѭ������
    do
    {
        //����ʼ����0xFC �����Ƕ��д��
        SPI_ReadWriteByte(0xFC);	  
        //��һ��sector������
        for(i=0;i<512;i++)
        {
            SPI_ReadWriteByte(*data++);
        }
        //��2��Byte��dummy CRC
        SPI_ReadWriteByte(0xff);
        SPI_ReadWriteByte(0xff);
        
        //�ȴ�SD��Ӧ��
        r1 = SPI_ReadWriteByte(0xff);
        if((r1&0x1F)!=0x05)
        {
            SD_CS=1;    //���Ӧ��Ϊ��������������ֱ���˳�
            return r1;
        }		   
        //�ȴ�SD��д�����
        if(SD_WaitDataReady()==1)
        {
            SD_CS=1;    //�ȴ�SD��д����ɳ�ʱ��ֱ���˳�����
            return 1;
        }	   
    }while(--count);//��sector���ݴ������  
    //��������������0xFD
    r1 = SPI_ReadWriteByte(0xFD);
    if(r1==0x00)
    {
        count =  0xfe;
    }		   
    if(SD_WaitDataReady()) //�ȴ�׼����
	{
		SD_CS=1;
		return 1;  
	}
    //д����ɣ�Ƭѡ��1
    SD_CS=1;
    SPI_ReadWriteByte(0xff);  
    return count;   //����countֵ�����д����count=0������count=1
}						  					  
/*******************************************************************************
* ��    ��: SD_Read_Bytes
* ��    ��: ��ָ������,��offset��ʼ����bytes���ֽ�
* ��ڲ���: 
*					uint32_t sector ������ַ��sectorֵ���������ַ��
*					uint8_t *buf     ���ݴ洢��ַ����С<=512byte��
*					uint16_t offset  �����������ƫ����
*				  uint16_t bytes   Ҫ�������ֽ���	
* ���ڲ���: 0��   �ɹ�
*						������ʧ��
* ˵    ��: 
*******************************************************************************/
uint8_t SD_Read_Bytes(uint32_t address,unsigned char *buf,unsigned int offset,unsigned int bytes)
{
    uint8_t r1;uint16_t i=0;  
    r1=SD_SendCommand(CMD17,address<<9,0);//���Ͷ���������      
    if(r1)return r1;  //Ӧ����ȷ��ֱ�ӷ���
	SD_CS=0;//ѡ��SD��
	if(SD_GetResponse(0xFE))//�ȴ�SD������������ʼ����0xFE
	{
		SD_CS=1; //�ر�SD��
		return 1;//��ȡʧ��
	}	 
		for(i=0;i<offset;i++)SPI_ReadWriteByte(0xff);//����offsetλ 
    for(;i<offset+bytes;i++)*buf++=SPI_ReadWriteByte(0xff);//��ȡ��������	
    for(;i<512;i++) SPI_ReadWriteByte(0xff); 	 //����ʣ���ֽ�
    SPI_ReadWriteByte(0xff);//����αCRC��
    SPI_ReadWriteByte(0xff);  
    SD_CS=1;//�ر�SD��
	return 0;
}
