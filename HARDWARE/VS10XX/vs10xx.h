
////////////////////////////////////////////////////////////////////////////////
//  �� �� ��   : vs10xx.h
//  �� �� ��   : V1.2
//  ��    ��   : TinderSmith
//  ��������   : 2014-01-01
//  ����޸�   : 2011/6/18
//  ��������   : <MP3><stm32 vs1053 oled tfcard(spi) fat0.11>
//  ��Ȩ����   : �θ���
//  ˵    ��   :  --------------------------------------------------------------
// 							 *1,����VS1053��FLAC����֧��.
// 							 *2,�޸�GetHeadInfo����. 
//              	--------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

#ifndef __VS10XX_H__
#define __VS10XX_H__
#include "sys.h"


#define VS_Default_Clock  0X9800		//VSʱ��,CLCK=12.288M,MULT=4(3.5��CLCK),ADD=3(2��MULT)
#define VS_High_Clock			0xC000		//VSʱ��,CLCK=12.288M,MULT=6(4.5��CLCK),ADD=0(������) ����55.3M
#define VS_Default_Volume 0x2525		//Ĭ�ϱ�׼����
#define VS_Low_Volume 		0x3131		//������С����
#define VS_WRITE_COMMAND 	2
#define VS_READ_COMMAND 	3
//VS10XX�Ĵ���
#define SPI_MODE        	0
#define SPI_STATUS      	1
#define SPI_BASS        	2
#define SPI_CLOCKF      	3		//ʱ��Ƶ�ʼӳ���
#define SPI_DECODE_TIME 	4
#define SPI_AUDATA      	5		//��Ƶ����
#define SPI_WRAM        	6
#define SPI_WRAMADDR    	7
#define SPI_HDAT0       	8
#define SPI_HDAT1       	9
#define SPI_AIADDR      	10
#define SPI_VOL         	11
#define SPI_AICTRL0     	12
#define SPI_AICTRL1     	13
#define SPI_AICTRL2     	14
#define SPI_AICTRL3     	15
//VS10XX��SPI_MODE�Ĵ�������λ����
#define SM_DIFF         	0x0001
#define SM_LAYER12       	0x0002
#define SM_RESET        	0x0004
#define SM_CANCEL     		0x0008
#define SM_EARSPEAKER_LO	0x0010
#define SM_TESTS        	0x0020
#define SM_STREAM       	0x0040
#define SM_EARSPEAKER_HI	0x0080
#define SM_DACT         	0x0100
#define SM_SDIORD       	0x0200
#define SM_SDISHARE     	0x0400
#define SM_SDINEW       	0x0800
#define SM_ADPCM        	0x1000
#define SM_LINE1        	0x4000
#define SM_CLK_RANGE     	0x8000


/* VS1053 �����Ĵ��� */
#define PAR_CHIP_ID                  0x1e00 /* VS1063, VS1053, 32 bits */
#define PAR_VERSION                  0x1e02 /* VS1063, VS1053 */
#define PAR_CONFIG1                  0x1e03 /* VS1063, VS1053 */
#define PAR_PLAY_SPEED               0x1e04 /* VS1063, VS1053 */
#define PAR_BYTERATE                 0x1e05 /* VS1053 */
#define PAR_END_FILL_BYTE            0x1e06 /* VS1063, VS1053 */
#define PAR_JUMP_POINTS              0x1e16 /*         VS1053 */
#define PAR_LATEST_JUMP              0x1e26 /*         VS1053 */
#define PAR_POSITION_MSEC            0x1e27 /* VS1063, VS1053, 32 bits */
#define PAR_RESYNC                   0x1e29 /* VS1063, VS1053 */

//STM32 to VS10XX �˿ڶ���
#define VS_DQ       PDin(8)    //DREQ 
#define VS_RST      PDout(9) 	 //RST
#define VS_XCS      PBout(12)  //XCS
#define VS_XDCS     PDout(10)  //XDCS

u16 GetDecodeTime(void);   	        //�õ�����ʱ��
u16 GetHeadInfo(void); 			        //�õ�������
u32 VS_RD_Reg(u8 address);					//���Ĵ���
void Send_EndByte(void);						//���ͽ����ֽ�
void ResetDecodeTime(void);	        //�������ʱ��
void VS_WR_Data(u8 data);						//д����
void VS_WR_Cmd(u8 address,u16 data);//д����
void VS_ST_Reset(void);							//��λ
void VS_IO_INIT(void);      			  //��ʼ�� VS10XX��GPIO
void VS_Recorde_Init(void);  				//��ʼ�� ����¼��״̬
extern u8 VS_Init(void);						//��ʼ��VS10XX
extern u8 VS_RAM[5];
extern u8 recording_on;

void VS_LoadUserCode(u16 *plugin,u16 len);	//�����û�patch
/*
void GetSpec(u8 *p);       	        //�õ���������
void LoadFlacPatch(void);           //����FLAC����PATCH
*/

#endif
