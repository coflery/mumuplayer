/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "spi.h"
#include "sdcard.h"

//********************************************************************************
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//SD卡 驱动代码		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/11/28 
//版本：V1.1
//2010/5/13									   
//增加了一些延时,实测可以支持TF卡(1G),金士顿2G,4G 16G SD卡
//2010/6/24
//加入了uint8_t SD_GetResponse(uint8_t Response)函数
//修改了uint8_t SD_WaitDataReady(void)函数
//增加了USB读卡器支持的uint8_t MSD_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);
//和uint8_t MSD_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite);两个函数
//等待SD卡回应
//Response:要得到的回应值
//返回值:0,成功得到了该回应值
//其他,得到回应值失败
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
//********************************************************************************

uint8_t  SD_Type=0;//SD卡的类型

uint8_t SD_GetResponse(uint8_t Response)
{
	uint16_t Count=0xFFF;                                     //等待次数	   						  
	while ((SPI_ReadWriteByte(0XFF)!=Response)&&Count)Count--;//等待得到准确的回应  	  
	if (Count==0)return MSD_RESPONSE_FAILURE;                 //得到回应失败   
	else return MSD_RESPONSE_NO_ERROR;                        //正确回应
}
//等待SD卡写入完成
//返回值:0,成功 
//其他,错误代码
uint8_t SD_WaitDataReady(void)
{
    uint8_t r1=MSD_DATA_OTHER_ERROR;
    uint32_t retry;
    retry=0;
    do
    {
        r1=SPI_ReadWriteByte(0xFF)&0X1F;//读到回应
        if(retry==0xfffe)return 1; 
		retry++;
		switch (r1)
		{					   
			case MSD_DATA_OK:					//数据接收正确
				r1=MSD_DATA_OK;
				break;  
			case MSD_DATA_CRC_ERROR:  //CRC校验错误
				return MSD_DATA_CRC_ERROR;  
			case MSD_DATA_WRITE_ERROR://数据写入错误
				return MSD_DATA_WRITE_ERROR;  
			default:									//未知错误    
				r1=MSD_DATA_OTHER_ERROR;
				break;	 
		}   
    }while(r1==MSD_DATA_OTHER_ERROR); //数据错误时一直等待
	retry=0;
	while(SPI_ReadWriteByte(0XFF)==0)   //读到数据为0,则数据还未写完成
	{
		retry++;
		//Delay_us(10);                   //SD卡写等待需要较长的时间
		if(retry>=0XFFFFFFFE)return 0XFF; //等待失败
	};	    
    return 0;                         //成功
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
//向SD卡发送一个命令
//输入: uint8_t cmd   命令 
//      uint32_t arg  命令参数
//      uint8_t crc   crc校验值	   
//返回值:SD卡返回的响应															  
uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    uint8_t r1;	
	uint8_t Retry=0;	         
	SD_CS=1;
  SPI_ReadWriteByte(0xff);//高速写命令延时
	SPI_ReadWriteByte(0xff);     
 	SPI_ReadWriteByte(0xff);  	 
  SD_CS=0; 								//片选端置低，选中SD卡

    //发送
    SPI_ReadWriteByte(cmd | 0x40);//分别写入命令
    SPI_ReadWriteByte(arg >> 24);
    SPI_ReadWriteByte(arg >> 16);
    SPI_ReadWriteByte(arg >> 8);
    SPI_ReadWriteByte(arg);
    SPI_ReadWriteByte(crc); 
    //等待响应，或超时退出
    while((r1=SPI_ReadWriteByte(0xFF))==0xFF)
    {
       Retry++;	    
       if(Retry>200)break; 
    }
		
    SD_CS=1;                  //关闭片选
    SPI_ReadWriteByte(0xFF);	//在总线上额外增加8个时钟，让SD卡完成剩下的工作
		
    return r1;    						//返回状态值
}		  																				 
//向SD卡发送一个命令(结束是不失能片选，还有后续数据传来）
//输入:uint8_t  cmd  命令
//     uint8_t  crc  crc校验值
//     uint32_t arg  命令参数
//返回值:SD卡返回的响应
uint8_t SD_SendCommand_NoDeassert(uint8_t cmd, uint32_t arg, uint8_t crc)
{
	uint8_t Retry=0;	         
	uint8_t r1;			   
    SPI_ReadWriteByte(0xff);       //高速写命令延时
	SPI_ReadWriteByte(0xff);  	 	 
    SD_CS=0;                       //片选端置低，选中SD卡	   
    //发送
    SPI_ReadWriteByte(cmd | 0x40); //分别写入命令
    SPI_ReadWriteByte(arg >> 24);
    SPI_ReadWriteByte(arg >> 16);
    SPI_ReadWriteByte(arg >> 8);
    SPI_ReadWriteByte(arg);
    SPI_ReadWriteByte(crc);
    //等待响应，或超时退出
    while((r1=SPI_ReadWriteByte(0xFF))==0xFF)
    {
        Retry++;
        if(Retry>200)break;
    }
    return r1;				//返回响应值
}
//把SD卡设置到挂起模式
//返回值:0,成功设置
//       1,设置失败
uint8_t SD_Idle_Sta(void)
{
	uint16_t i;
	uint8_t retry;
    for(i=0;i<0xf00;i++);													//纯延时，等待SD卡上电完成	 
    for(i=0;i<10;i++)SPI_ReadWriteByte(0xFF);     //先产生>74个脉冲，让SD卡自己初始化完成
    //-----------------SD卡复位到idle开始-----------------
    //循环连续发送CMD0，直到SD卡返回0x01,进入IDLE状态
    //超时则直接退出
    retry = 0;
    do
    {	   
        i = SD_SendCommand(CMD0, 0, 0x95);//发送CMD0，让SD卡进入IDLE状态
        retry++;
    }
		while((i!=0x01)&&(retry<200));
    //跳出循环后检查原因：初始化成功 还是 重试超时
    if(retry==200)return 1; //失败
	return 0;                 //成功	 						  
}														    

void SD_IO_Init(void)			//初始化SD的GPIO
{		
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;
	
	  /* 配置 PA5/6/7    作 SCK, MISO,MOSI(for MMC_SD) */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		/* 配置 PA4        作 SD_CS             引脚悬空*/
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
		/* SPI1 配置 -------------------------------------------------------------*/
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //双向双线模式
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	 										 //SPI为主模式
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;									 //传送的数据位为8
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;												 //空闲状态时，SCK保持高电平
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;											 //数据采样从第二个时钟边沿开始
 	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;												   //启用软件从设备管理
 	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;//波特率为100： fPCLK/32
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;								 //先传最高位
 	SPI_InitStructure.SPI_CRCPolynomial = 7;													 
 	SPI_Init(SPI1, &SPI_InitStructure);																 //初始化SPI1
  SPI_Cmd(SPI1, ENABLE);																					   //打开  SPI1
	SD_CS = 1;
}

//初始化SD卡
//如果成功返回,则会自动设置SPI速度为18Mhz
//返回值:0：NO_ERR
//       1：TIME_OUT
//      99：NO_CARD																 
uint8_t SD_Init(void)
{		
    uint8_t r1=0;      // 存放SD卡的返回值
    uint16_t retry=0;  // 用来进行超时计数
    uint8_t buff[6];
	
	SD_IO_Init();		//初始化SD的GPIO
 	SPI_SetSpeed_SPI1(SPI_SPEED_LOW);//设置到低速模式		 
	SD_CS=1;	
    if(SD_Idle_Sta()) return 1;//超时返回1 设置到idle 模式失败	  
    //-----------------SD卡复位到idle结束-----------------	 
    //获取卡片的SD版本信息
 	SD_CS=0;	
	r1 = SD_SendCommand_NoDeassert(8, 0x1aa,0x87);	     
  //如果卡片版本信息是v1.0版本的，即r1=0x05，则进行以下初始化
	//上面没反应
    if(r1 == 0x05)
    {
        //设置卡类型为SDV1.0，如果后面检测到为MMC卡，再修改为MMC
        SD_Type = SD_TYPE_V1;	   
        //如果是V1.0卡，CMD8指令后没有后续数据
        //片选置高，结束本次命令
        SD_CS=1;
        //多发8个CLK，让SD结束后续操作
        SPI_ReadWriteByte(0xFF);	  
        //-----------------SD卡、MMC卡初始化开始-----------------	 
        //发卡初始化指令CMD55+ACMD41
        // 如果有应答，说明是SD卡，且初始化完成
        // 没有回应，说明是MMC卡，额外进行相应初始化
        retry = 0;
        do
        {
            //先发CMD55，应返回0x01；否则出错
            r1 = SD_SendCommand(CMD55, 0, 0);
            if(r1 == 0XFF)return r1;//只要不是0xff,就接着发送	  
            //得到正确响应后，发ACMD41，应得到返回值0x00，否则重试200次
            r1 = SD_SendCommand(ACMD41, 0, 0);
            retry++;
        }while((r1!=0x00) && (retry<400));
        // 判断是超时还是得到正确回应
        // 若有回应：是SD卡；没有回应：是MMC卡	  
        //----------MMC卡额外初始化操作开始------------
//	retry=400;
        if(retry==400)
        {
            retry = 0;
            //发送MMC卡初始化命令（没有测试）
            do
            {
                r1 = SD_SendCommand(1,0,0);
                retry++;
            }while((r1!=0x00)&& (retry<400));
            if(retry==400)return 1;   //MMC卡初始化超时		    
            //写入卡类型
            SD_Type = SD_TYPE_MMC;
        }
        //----------MMC卡额外初始化操作结束------------	    
        //设置SPI为高速模式
    SPI_SetSpeed_SPI1(SPI_SPEED_HIGH);   
		SPI_ReadWriteByte(0xFF);	 
        //禁止CRC校验	   
		r1 = SD_SendCommand(CMD59, 0, 0x95);
        if(r1 != 0x00)return r1;  //命令错误，返回r1   	   
        //设置Sector Size
        r1 = SD_SendCommand(CMD16, 512, 0x95);
        if(r1 != 0x00)return r1;//命令错误，返回r1		 
        //-----------------SD卡、MMC卡初始化结束-----------------

    }//SD卡为V1.0版本的初始化结束	 
    //下面是V2.0卡的初始化
    //其中需要读取OCR数据，判断是SD2.0还是SD2.0HC卡
    else if(r1 == 0x01)
    {
        //V2.0的卡，CMD8命令后会传回4字节的数据，要跳过再结束本命令
        buff[0] = SPI_ReadWriteByte(0xFF);  //should be 0x00
        buff[1] = SPI_ReadWriteByte(0xFF);  //should be 0x00
        buff[2] = SPI_ReadWriteByte(0xFF);  //should be 0x01
        buff[3] = SPI_ReadWriteByte(0xFF);  //should be 0xAA
        SD_CS=1;	  
        SPI_ReadWriteByte(0xFF);//the next 8 clocks			 
        //判断该卡是否支持2.7V-3.6V的电压范围
        //if(buff[2]==0x01 && buff[3]==0xAA) //不判断，让其支持的卡更多
        {	  
            retry = 0;
            //发卡初始化指令CMD55+ACMD41
    		do
    		{
    			r1 = SD_SendCommand(CMD55, 0, 0);
    			if(r1!=0x01)return r1;	   
    			r1 = SD_SendCommand(ACMD41, 0x40000000, 0);
                if(retry>200)return r1;  //超时则返回r1状态  
            }while(r1!=0);		  
            //初始化指令发送完成，接下来获取OCR信息		   
            //-----------鉴别SD2.0卡版本开始-----------
            r1 = SD_SendCommand_NoDeassert(CMD58, 0, 0);
            if(r1!=0x00)
			{
				SD_CS=1;//释放SD片选信号
				return r1;  //如果命令没有返回正确应答，直接退出，返回应答	 
			}//读OCR指令发出后，紧接着是4字节的OCR信息
            buff[0] = SPI_ReadWriteByte(0xFF);
            buff[1] = SPI_ReadWriteByte(0xFF); 
            buff[2] = SPI_ReadWriteByte(0xFF);
            buff[3] = SPI_ReadWriteByte(0xFF);		 
            //OCR接收完成，片选置高
            SD_CS=1;
            SPI_ReadWriteByte(0xFF);	   
            //检查接收到的OCR中的bit30位（CCS），确定其为SD2.0还是SDHC
            //如果CCS=1：SDHC   CCS=0：SD2.0
            if(buff[0]&0x40)SD_Type = SD_TYPE_V2HC;    //检查CCS	 
            else SD_Type = SD_TYPE_V2;	    
            //-----------鉴别SD2.0卡版本结束----------- 
            //设置SPI为高速模式
            SPI_SetSpeed_SPI1(SPI_SPEED_HIGH);  
        }	    
    }
    return r1;
}	 																			   
//从SD卡中读回指定长度的数据，放置在给定位置
//输入: uint8_t *data(存放读回数据的内存>len)
//      uint16_t len(数据长度）
//      uint8_t release(传输完成后是否释放总线CS置高 0：不释放 1：释放）	 
//返回值:0：NO_ERR
//  	 other：错误信息														  
uint8_t SD_ReceiveData(uint8_t *data, uint16_t len, uint8_t release)
{
    // 启动一次传输
    SD_CS=0;				  	  
	if(SD_GetResponse(0xFE))//等待SD卡发回数据起始令牌0xFE
	{	  
		SD_CS=1;
		return 1;
	}
    while(len--)//开始接收数据
    {
        *data=SPI_ReadWriteByte(0xFF);
        data++;
    }
    //下面是2个伪CRC（dummy CRC）
    SPI_ReadWriteByte(0xFF);
    SPI_ReadWriteByte(0xFF);
    if(release==RELEASE)//按需释放总线，将CS置高
    {
        SD_CS=1;//传输结束
        SPI_ReadWriteByte(0xFF);
    }											  					    
    return 0;
}																				  
//获取SD卡的CID信息，包括制造商信息
//输入: uint8_t *cid_data(存放CID的内存，至少16Byte)
//返回值:0：NO_ERR
//		 	 1：TIME_OUT
//	 other：错误信息
uint8_t SD_GetCID(uint8_t *cid_data)
{
    uint8_t r1;	   
    //发CMD10命令，读CID
    r1 = SD_SendCommand(CMD10,0,0xFF);
    if(r1 != 0x00)return r1;  //没返回正确应答，则退出，报错  
    SD_ReceiveData(cid_data,16,RELEASE);//接收16个字节的数据	 
    return 0;
}																				  
//获取SD卡的CSD信息，包括容量和速度信息
//输入:uint8_t *cid_data(存放CID的内存，至少16Byte）	    
//返回值:0：NO_ERR
//       1：TIME_OUT
// 	 other：错误信息														   
uint8_t SD_GetCSD(uint8_t *csd_data)
{
    uint8_t r1;	 
    r1=SD_SendCommand(CMD9,0,0xFF);//发CMD9命令，读CSD
    if(r1)return r1;  //没返回正确应答，则退出，报错  
    SD_ReceiveData(csd_data, 16, RELEASE);//接收16个字节的数据 
    return 0;
}
//获取SD卡的容量(字节)
//返回值:0： 	取容量出错 
//       其他:SD卡的容量(字节)
uint32_t SD_GetCapacity(void)
{
    uint8_t csd[16];
    uint32_t Capacity;
    uint8_t r1;
    uint16_t i;
		uint16_t temp;  					    
    if(SD_GetCSD(csd)!=0) return 0;	//取CSD信息，如果期间出错，返回0
    if((csd[0]&0xC0)==0x40)					//如果为SDHC卡，按照下面方式计算
    {									  
	    Capacity=((uint32_t)csd[8])<<8;
			Capacity+=(uint32_t)csd[9]+1;	 
      Capacity = (Capacity)*1024;//得到扇区数
			Capacity*=512;//得到字节数 
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
    	Capacity *= (uint32_t)temp;//字节为单位 	  
    }
    return (uint32_t)Capacity;
}	    																			    
/*******************************************************************************
* 名    称: SD_ReadSingleBlock
* 功    能: 读SD卡的一个block
* 入口参数: uint32_t sector 取地址(sector值，非物理地址)
* 					uint8_t *buffer 数据存储地址(大小至少512byte)
* 出口参数: 0：   成功
*						other：失败
* 说    明: 
*******************************************************************************/						  
uint8_t SD_ReadSingleBlock(uint32_t sector, uint8_t *buffer)
{
	uint8_t r1;	    
    //设置为高速模式
    SPI_SetSpeed_SPI1(SPI_SPEED_HIGH);  		   
    //如果不是SDHC，给定的是sector地址，将其转换成byte地址
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector = sector<<9;
    } 
	r1 = SD_SendCommand(CMD17, sector, 0);//读命令	 		    
	if(r1 != 0x00)return r1; 		   							  
	r1 = SD_ReceiveData(buffer, 512, RELEASE);		 
	if(r1 != 0)return r1;   //读数据出错！
    else return 0; 
}

/*******************************************************************************
* 名    称: MSD_WriteBuffer
* 功    能: 写入MSD/SD数据(USB读写SD卡)
* 入口参数: 
*					pBuffer:数据存放区
*					WriteAddr:写入的首地址
*					NumByteToWrite:要写入的字节数
* 出口参数: 0：   成功
*						其他：失败
* 说    明: 
*******************************************************************************/
#define BLOCK_SIZE 512			//定义SD卡的块大小
uint8_t MSD_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite)
{
	uint32_t i,NbrOfBlock = 0, Offset = 0;
	uint32_t sector;
	uint8_t r1;
   	NbrOfBlock = NumByteToWrite / BLOCK_SIZE;//得到要写入的块的数目	    
    SD_CS=0;	  		   
	while (NbrOfBlock--)//写入一个扇区
	{
		sector=WriteAddr+Offset;
		if(SD_Type==SD_TYPE_V2HC)sector>>=9;//执行与普通操作相反的操作					  			 
		r1=SD_SendCommand_NoDeassert(CMD24,sector,0xff);//写命令   
 		if(r1)
		{
			SD_CS=1;
			return 1;//应答不正确，直接返回 	   
	    }
	    SPI_ReadWriteByte(0xFE);//放起始令牌0xFE   
	    //放一个sector的数据
	    for(i=0;i<512;i++)SPI_ReadWriteByte(*pBuffer++);  
	    //发2个Byte的dummy CRC
	    SPI_ReadWriteByte(0xff);
	    SPI_ReadWriteByte(0xff); 
 		if(SD_WaitDataReady())//等待SD卡数据写入完成
		{
			SD_CS=1;
			return 2;    
		}
		Offset += 512;	   
	}	    
    //写入完成，片选置1
    SD_CS=1;
    SPI_ReadWriteByte(0xff);	 
    return 0;
}
/*******************************************************************************
* 名    称: MSD_ReadBuffer
* 功    能: 读取MSD/SD数据(USB读SD卡)
* 入口参数: 
*					pBuffer:数据存放区
*					ReadAddr:读取的首地址
*					NNumByteToRead:要读出的字节数
* 出口参数: 0：   成功
*						其他：失败
* 说    明: 
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
		if(SD_Type==SD_TYPE_V2HC)sector>>=9;//执行与普通操作相反的操作					  			 
		r1=SD_SendCommand_NoDeassert(CMD17,sector,0xff);//读命令	 		    
		if(r1)//命令发送错误
		{
    		SD_CS=1;
			return r1;
		}	   							  
		r1=SD_ReceiveData(pBuffer,512,RELEASE);		 
		if(r1)//读数错误
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
* 名    称: SD_WriteSingleBlock
* 功    能: 写入SD卡的一个block(未实际测试过)
* 入口参数: 
*					uint32_t sector 扇区地址(sector值，非物理地址)
*					uint8_t *buffer 数据存储地址(大小至少512byte)
*					NNumByteToRead:要读出的字节数
* 出口参数: 0：   成功
*						其他：失败
* 说    明: 
*******************************************************************************/
uint8_t SD_WriteSingleBlock(uint32_t sector, const uint8_t *data)
{
    uint8_t r1;
    uint16_t i;
    uint16_t retry;

    //设置为高速模式
    SPI_SetSpeed_SPI1(SPI_SPEED_HIGH);	   
    //如果不是SDHC，给定的是sector地址，将其转换成byte地址
    if(SD_Type!=SD_TYPE_V2HC)
    {
        sector = sector<<9;
    }   
    r1 = SD_SendCommand(CMD24, sector, 0x00);
    if(r1 != 0x00)
    {
        return r1;  //应答不正确，直接返回
    }
    
    //开始准备数据传输
    SD_CS=0;
    //先放3个空数据，等待SD卡准备好
    SPI_ReadWriteByte(0xff);
    SPI_ReadWriteByte(0xff);
    SPI_ReadWriteByte(0xff);
    //放起始令牌0xFE
    SPI_ReadWriteByte(0xFE);
    //放一个sector的数据
    for(i=0;i<512;i++)
    {
        SPI_ReadWriteByte(*data++);
    }
    //发2个Byte的dummy CRC
    SPI_ReadWriteByte(0xff);
    SPI_ReadWriteByte(0xff);
    
    //等待SD卡应答
    r1 = SPI_ReadWriteByte(0xff);
    if((r1&0x1F)!=0x05)
    {
        SD_CS=1;
        return r1;
    }
    
    //等待操作完成
    retry = 0;
    while(!SPI_ReadWriteByte(0xff))
    {
        retry++;
        if(retry>0xfffe)        //如果长时间写入没有完成，报错退出
        {
            SD_CS=1;
            return 1;           //写入超时返回1
        }
    }	    
    //写入完成，片选置1
    SD_CS=1;
    SPI_ReadWriteByte(0xff);

    return 0;
}				           
/*******************************************************************************
* 名    称: SD_ReadMultiBlock
* 功    能: 读SD卡的多个block(实际测试过)
* 入口参数: 
*					uint32_t sector 扇区地址(sector值，非物理地址)
*					uint8_t *buffer 数据存储地址(大小至少512byte)
*					uint8_t count 连续读count个block
* 出口参数: 0：   成功
*						其他：失败
* 说    明: 
*******************************************************************************/											  
uint8_t SD_ReadMultiBlock(uint32_t sector, uint8_t *buffer, uint8_t count)
{
    uint8_t r1;	 			 
  SPI_SetSpeed_SPI1(SPI_SPEED_HIGH);//设置为高速模式  
 	//如果不是SDHC，将sector地址转成byte地址
    if(SD_Type!=SD_TYPE_V2HC)sector = sector<<9;  
    //SD_WaitDataReady();
    //发读多块命令
	r1 = SD_SendCommand(CMD18, sector, 0);//读命令
	if(r1 != 0x00)return r1;	 
    do//开始接收数据
    {
        if(SD_ReceiveData(buffer, 512, NO_RELEASE) != 0x00)break; 
        buffer += 512;
    } while(--count);		 
    //全部传输完毕，发送停止命令
    SD_SendCommand(CMD12, 0, 0);
    //释放总线
    SD_CS=1;
    SPI_ReadWriteByte(0xFF);    
    if(count != 0)return count;   //如果没有传完，返回剩余个数	 
    else return 0;	 
}
/*******************************************************************************
* 名    称: SD_WriteMultiBlock
* 功    能: 写入SD卡的N个block(未实际测试过)
* 入口参数: 
*					uint32_t sector 扇区地址(sector值，非物理地址)
*					uint8_t *buffer 数据存储地址(大小至少512byte)
*					uint8_t count 写入的block数目
* 出口参数: 0：		成功
*				   其他： 失败
* 说    明: 
*******************************************************************************/												   
uint8_t SD_WriteMultiBlock(uint32_t sector, const uint8_t *data, uint8_t count)
{
    uint8_t r1;
    uint16_t i;	 		 
    //SPI_SetSpeed_SPI1(SPI_SPEED_HIGH);//设置为高速模式	 
    if(SD_Type != SD_TYPE_V2HC)sector = sector<<9;//如果不是SDHC，给定的是sector地址，将其转换成byte地址  
    if(SD_Type != SD_TYPE_MMC) r1 = SD_SendCommand(ACMD23, count, 0x00);//如果目标卡不是MMC卡，启用ACMD23指令使能预擦除   
    r1 = SD_SendCommand(CMD25, sector, 0x00);//发多块写入指令
    if(r1 != 0x00)return r1;  //应答不正确，直接返回	 
    SD_CS=0;//开始准备数据传输   
    SPI_ReadWriteByte(0xff);//先放3个空数据，等待SD卡准备好
    SPI_ReadWriteByte(0xff);   
    //--------下面是N个sector写入的循环部分
    do
    {
        //放起始令牌0xFC 表明是多块写入
        SPI_ReadWriteByte(0xFC);	  
        //放一个sector的数据
        for(i=0;i<512;i++)
        {
            SPI_ReadWriteByte(*data++);
        }
        //发2个Byte的dummy CRC
        SPI_ReadWriteByte(0xff);
        SPI_ReadWriteByte(0xff);
        
        //等待SD卡应答
        r1 = SPI_ReadWriteByte(0xff);
        if((r1&0x1F)!=0x05)
        {
            SD_CS=1;    //如果应答为报错，则带错误代码直接退出
            return r1;
        }		   
        //等待SD卡写入完成
        if(SD_WaitDataReady()==1)
        {
            SD_CS=1;    //等待SD卡写入完成超时，直接退出报错
            return 1;
        }	   
    }while(--count);//本sector数据传输完成  
    //发结束传输令牌0xFD
    r1 = SPI_ReadWriteByte(0xFD);
    if(r1==0x00)
    {
        count =  0xfe;
    }		   
    if(SD_WaitDataReady()) //等待准备好
	{
		SD_CS=1;
		return 1;  
	}
    //写入完成，片选置1
    SD_CS=1;
    SPI_ReadWriteByte(0xff);  
    return count;   //返回count值，如果写完则count=0，否则count=1
}						  					  
/*******************************************************************************
* 名    称: SD_Read_Bytes
* 功    能: 在指定扇区,从offset开始读出bytes个字节
* 入口参数: 
*					uint32_t sector 扇区地址（sector值，非物理地址）
*					uint8_t *buf     数据存储地址（大小<=512byte）
*					uint16_t offset  在扇区里面的偏移量
*				  uint16_t bytes   要读出的字节数	
* 出口参数: 0：   成功
*						其他：失败
* 说    明: 
*******************************************************************************/
uint8_t SD_Read_Bytes(uint32_t address,unsigned char *buf,unsigned int offset,unsigned int bytes)
{
    uint8_t r1;uint16_t i=0;  
    r1=SD_SendCommand(CMD17,address<<9,0);//发送读扇区命令      
    if(r1)return r1;  //应答不正确，直接返回
	SD_CS=0;//选中SD卡
	if(SD_GetResponse(0xFE))//等待SD卡发回数据起始令牌0xFE
	{
		SD_CS=1; //关闭SD卡
		return 1;//读取失败
	}	 
		for(i=0;i<offset;i++)SPI_ReadWriteByte(0xff);//跳过offset位 
    for(;i<offset+bytes;i++)*buf++=SPI_ReadWriteByte(0xff);//读取有用数据	
    for(;i<512;i++) SPI_ReadWriteByte(0xff); 	 //读出剩余字节
    SPI_ReadWriteByte(0xff);//发送伪CRC码
    SPI_ReadWriteByte(0xff);  
    SD_CS=1;//关闭SD卡
	return 0;
}
