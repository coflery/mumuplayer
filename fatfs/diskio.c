/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
//#include "usbdisk.h"	/* Example: Header file of existing USB MSD control module */
//#include "atadrive.h"	/* Example: Header file of existing ATA harddisk control module */
#include "sdcard.h"		/* Example: Header file of existing MMC/SDC contorl module */
#include "spi.h"      /*�ṩClr_SD_CS��Set_SD_CS����*/

/* Definitions of physical drive number for each drive */
#define ATA		0	/* Example: Map ATA harddisk to physical drive 0 */
#define MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status ( BYTE pdrv	/* Physical drive nmuber to identify the drive */)
{	
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize ( BYTE pdrv				/* Physical drive nmuber to identify the drive */)
{	
    unsigned char state;

    if(pdrv)
	{
		return 	STA_NOINIT;  //��֧�ִ���0�Ĳ���
	}
    state = SD_Init();
    if(state == STA_NODISK)
	{
        return 	STA_NODISK;
	}
    else if(state != 0)
		{return STA_NOINIT;}  //�������󣺳�ʼ��ʧ��
		else
		{return 0;}           //��ʼ���ɹ�
}


/*-----------------------------------------------------------------------*/
/* ������                                                                */
/*drv:���̱��0~9                                                        */
/*buff:���ݽ��ջ����׵�ַ                                                */
/*sector:������ַ                                                        */
/*count:��Ҫ��ȡ��������                                                 */
/*-----------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	unsigned char res=0; 
    if (!count)return RES_PARERR;//��֧�ֵ����̲���,��count����Ϊ0�����򷵻ز�������
	if(count==1)            //1��sector�Ķ�����      
    {                                                
        res = SD_ReadSingleBlock(sector, buff);      
    }                                                
    else                    //���sector�Ķ�����     
    {                                                
        res = SD_ReadMultiBlock(sector, buff, count);
    }     

   //������ֵ����SPI_SD_driver.c�ķ���ֵת��ff.c�ķ���ֵ
    if(res==0x00)
    return RES_OK;
    else return RES_ERROR;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	unsigned char res=0;
    if (!count) return RES_PARERR;  //��֧�ֵ����̲�����count���ܵ���0�����򷵻ز�������
    if(count == 1)//1��sector��д����
    {
        res = SD_WriteSingleBlock(sector, buff);
    }
    else		  //���sector��д���� 
    {
        res = SD_WriteMultiBlock(sector, buff, count);
    }
    // ����ֵת��
    if(res == 0)
        return RES_OK;
    else
        return RES_ERROR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    DRESULT res;
    if (pdrv)
    {    
        return RES_PARERR;  //��֧�ֵ����̲��������򷵻ز�������
    }
    //FATFSĿǰ�汾���账��CTRL_SYNC��GET_SECTOR_COUNT��GET_BLOCK_SIZ��������
    switch(cmd)
    {
    case CTRL_SYNC:
        Clr_SD_CS;
        if(SD_WaitReady()==0)
        {
            res = RES_OK;
        }
        else
        {
            res = RES_ERROR;
        }
        Set_SD_CS;
        break;
        
    case GET_BLOCK_SIZE:
        *(WORD*)buff = 512;
        res = RES_OK;
        break;

    case GET_SECTOR_COUNT:
        *(DWORD*)buff = SD_GetCapacity();
        res = RES_OK;
        break;
    default:
        res = RES_PARERR;
        break;
    }
    return res;
}
#endif
