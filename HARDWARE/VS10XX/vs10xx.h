
////////////////////////////////////////////////////////////////////////////////
//  文 件 名   : vs10xx.h
//  版 本 号   : V1.2
//  作    者   : TinderSmith
//  生成日期   : 2014-01-01
//  最近修改   : 2011/6/18
//  功能描述   : <MP3><stm32 vs1053 oled tfcard(spi) fat0.11>
//  版权所有   : 任复衡
//  说    明   :  --------------------------------------------------------------
// 							 *1,增加VS1053的FLAC播放支持.
// 							 *2,修改GetHeadInfo函数. 
//              	--------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////

#ifndef __VS10XX_H__
#define __VS10XX_H__
#include "sys.h"


#define VS_Default_Clock  0X9800		//VS时钟,CLCK=12.288M,MULT=4(3.5倍CLCK),ADD=3(2倍MULT)
#define VS_High_Clock			0xC000		//VS时钟,CLCK=12.288M,MULT=6(4.5倍CLCK),ADD=0(不允许) 共计55.3M
#define VS_Default_Volume 0x2525		//默认标准音量
#define VS_Low_Volume 		0x3131		//测试用小音量
#define VS_WRITE_COMMAND 	2
#define VS_READ_COMMAND 	3
//VS10XX寄存器
#define SPI_MODE        	0
#define SPI_STATUS      	1
#define SPI_BASS        	2
#define SPI_CLOCKF      	3		//时钟频率加乘数
#define SPI_DECODE_TIME 	4
#define SPI_AUDATA      	5		//音频数据
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
//VS10XX的SPI_MODE寄存器参数位意义
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


/* VS1053 参数寄存器 */
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

//STM32 to VS10XX 端口定义
#define VS_DQ       PDin(8)    //DREQ 
#define VS_RST      PDout(9) 	 //RST
#define VS_XCS      PBout(12)  //XCS
#define VS_XDCS     PDout(10)  //XDCS

u16 GetDecodeTime(void);   	        //得到解码时间
u16 GetHeadInfo(void); 			        //得到比特率
u32 VS_RD_Reg(u8 address);					//读寄存器
void Send_EndByte(void);						//发送结束字节
void ResetDecodeTime(void);	        //重设解码时间
void VS_WR_Data(u8 data);						//写数据
void VS_WR_Cmd(u8 address,u16 data);//写命令
void VS_ST_Reset(void);							//软复位
void VS_IO_INIT(void);      			  //初始化 VS10XX的GPIO
void VS_Recorde_Init(void);  				//初始化 进入录音状态
extern u8 VS_Init(void);						//初始化VS10XX
extern u8 VS_RAM[5];
extern u8 recording_on;

void VS_LoadUserCode(u16 *plugin,u16 len);	//加载用户patch
/*
void GetSpec(u8 *p);       	        //得到分析数据
void LoadFlacPatch(void);           //加载FLAC解码PATCH
*/

#endif
