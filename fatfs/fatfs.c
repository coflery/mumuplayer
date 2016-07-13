#include "fatfs.h"
#include "delay.h"
#include "usart.h"
#include "vs10xx.h"	
#include "spi.h"
#include "sdcard.h"
#include "oled.h"
#include "key.h"
#include "led.h"
#include <string.h>

#define song_play   1
#define song_over   2
#define song_pause  3

uint8_t MUSIC_state=song_play;
uint8_t play_state = 0;
uint16_t SongPlayedNum=1;
uint16_t temp=0;
uint16_t sysfun=1;

char *string_bass[3]={"Normal","Bass  ","Light "};
const int Bass_Buffer[3]={0x0000,0x00f8,0x5a00};
char *format[6] = {".MP3",".OGG",".WAV",".WMA",".MID",".FLA"};//段文件名只能读出三个扩展名
const int Volume_Buf[9] ={0x7777,0x6969,0x6060,0x5252,0x4242,0x3636,0x2727,0x1919,0x1111};
uint8_t volume=6;				//也就是VS1053的DefaultVolume
float now_process;
/* <<<<<<<<<<<<<<<<文件系统变量<<<<<<<<<<<<<<<< */
			FATFS fs;            // Work area (file system object) for logical drive
			FIL fsrc, fdst;      // file objects
			BYTE buffer[512]; 	 // file copy buffer
			UINT br, bw;         // File R/W count
			FRESULT res;
			DIR dirs;
			FILINFO finfo;
/* <<<<<<<<<<<<<<<<文件系统变量<<<<<<<<<<<<<<<< */
					
FRESULT scan_files (char* path)
{
  DWORD acc_size;							/* Work register for fs command */
	WORD acc_files, acc_dirs;
	FILINFO finfo;
	DIR dirs;
	FRESULT res;
	BYTE i;

	if ((res = f_opendir(&dirs, path)) == FR_OK) {
		i = strlen(path);
		while (((res = f_readdir(&dirs, &finfo)) == FR_OK) && finfo.fname[0]) {
			if (finfo.fattrib & AM_DIR) {
				acc_dirs++;
				*(path+i) = '/'; strcpy(path+i+1, &finfo.fname[0]);
				res = scan_files(path);
				*(path+i) = '\0';
				if (res != FR_OK) break;
			} else {
				acc_files++;
				acc_size += finfo.fsize;
			}
		}
	}
	return res;
}
 
void OutPutFile(void)
{ 
	unsigned int a;
  	FRESULT res;
  	FILINFO finfo;
  	DIR dirs;
  	char path[50]={""};  
  	disk_initialize(0);
		f_mount(&fs,"0", 1);
 	if (f_opendir(&dirs, path) == FR_OK) 
  	{
    	while (f_readdir(&dirs, &finfo) == FR_OK)  
    	{
	      	if (finfo.fattrib & AM_ARC) 
	      	{
	        	if(!finfo.fname[0])	
	          		break;         
	        	printf("\r\n file name is:\n   %s\n",finfo.fname);
	        	res = f_open(&fsrc, finfo.fname, FA_OPEN_EXISTING | FA_READ);
	        	br=1;
				a=0;
				for (;;) 
				{
					for(a=0; a<512; a++) 
						buffer[a]=0; 
						res = f_read(&fsrc, buffer, sizeof(buffer), &br);
						//printf("%s\n",buffer);	
						//printp("\r\n@@@@@res=%2d  br=%6d  bw=%6d",res,br,bw);
						if (res || br == 0) break;   // error or eof
						//res = f_write(&fdst, buffer, br, &bw);
						//printp("\r\n$$$$$res=%2d  br=%6d  bw=%6d",res,br,bw);
						//if (res || bw < br) break;   // error or disk full	
	      }
				f_close(&fsrc);                      
	      	}
    	}     
  	}
  	while(1);
}
/*******************************************************/ 
/** first part of RIFF Header, insert 444 zeroes after this */  
const unsigned char RIFFHeader0[] = { 'R' , 'I' , 'F' , 'F' , // 块 ID (RIFF) 
                                      0x70, 0x70, 0x70, 0x70, // 存储块大小 (录完后计算) 
                                      'W' , 'A' , 'V' , 'E' , // RIFF 文件类型
                                      'f' , 'm' , 't' , ' ' , // 块 ID (fmt ) 
                                      0x14, 0x00, 0x00, 0x00, // 块容量 (0x14 = 20 bytes)
                                      0x11, 0x00,             // 格式名称(IMA ADPCM) 0x01,表示PCM ; 0x11,表示IMA ADPCM
                                      0x01, 0x00,             // 声道数 (2)
                                      0x44, 0xac, 0x00, 0x00, // 采样率 (0x0c80 = 32.0kHz) 
                                      0x53, 0x57, 0x01, 0x00, // 字节速率(16.0khz)
                                      0x00, 0x01,             // 数据块大小 (512 bytes)  
                                      0x10, 0x00,             // 单个采样 ADPCM 数据大小 (4 bits)
	                                    0x02, 0x00,             // 扩展数据 (2 bytes) 
                                      0xf9, 0x01,             // 每块的采样数 (505 samples) 
                                      'f' , 'a' , 'c' , 't' , // 块 ID (fact) 
                                      0xc8, 0x01, 0x00, 0x00, // 块容量 (456 bytes (zeropad!))
                                      0x70, 0x70, 0x70, 0x70  // 采样数 (录完后计算) 
																			};
//add 452 zeros after
const unsigned char RIFFHeader504[] = { 'd' , 'a' , 't' , 'a' , // 块 ID (data) 
																			  0x70, 0x70, 0x70, 0x70  // 块容量 (calculate after rec!) 
																			};

/*录音的写文件步骤为: 
1、写header（扩充到512字节） 
2、写音频数据（记录下多少个sector，1个sector为512字节） 
3、根据录音文件大小算出相应的值填充到header中 
(2步骤中，每128word为一个block，每block的第四个数据应该是0x00，可以通过这个数据来判断数据是否正确上传 )
*/
u8 VS_recorde_save(void)
{
	unsigned char blockNumber=0;
	unsigned long sectorCount=1;
	unsigned long SEC_CAL = 0;
	unsigned int  oneword;
	unsigned char databuffer[512];/* 磁盘数据缓冲区 */
	unsigned char wavheader[512];
  unsigned int a, temp;
  FRESULT res;
  unsigned char* dataBufPtr;

	VS_Recorde_Init();//初始化，使VS10XX进入录音状态 
  f_mount(&fs,"0", 1);
  f_open(&fdst, "voice.wav", FA_CREATE_ALWAYS | FA_WRITE); 

	//这里的header为了写sector方便，扩展为512字节
   for (temp=0; temp<56; temp++) 
   {
     wavheader[temp] = RIFFHeader0[temp]; 
   }
   for (temp=52; temp<504; temp++)
   {
     wavheader[temp] = 0;
   }
   for (temp=504; temp<512; temp++)
   {
     wavheader[temp] = RIFFHeader504[temp-504]; 
   }
   if(f_write( &fdst, wavheader, 512 , &a ))
	 {
		 while(1);
	 }
   while (VS_RD_Reg(SPI_HDAT1)>>8);  //过多数据在缓冲区,等待缓冲区级别重置为0
   dataBufPtr  = databuffer; 		     //重置 dataBufPtr 到 filebuffer 的开始处
   blockNumber = 0;
   while(recording_on==0);
	 OLED_ShowString(0,3,"REC NOW");
	 while(recording_on)
   {
	     if(VS_RD_Reg(SPI_HDAT1) >= 128)
	     {
				if (dataBufPtr>(databuffer+511))
					{
						while(1);//停止
					}
				for (temp=0;temp<128;temp++)  
					{
						oneword= VS_RD_Reg(SPI_HDAT0);
						*dataBufPtr++ = (unsigned char)(oneword>>8);
						*dataBufPtr++ = (unsigned char)(oneword&0xff);
					}
		     blockNumber++;
			 }
	     if(blockNumber==2)
	     {  
	        blockNumber=0;
	        if(f_write( &fdst, databuffer, 512 , &a ))  
	        {
	             while(1);
	        }
	        sectorCount++;
	        dataBufPtr = databuffer;
	     }
			 //Key_task();
   }
	OLED_ShowString(0,3,"REC OVER");

  SEC_CAL=(sectorCount-1)*1010;  //采样的数量 
	wavheader[48] = (SEC_CAL & 0xff);  
	wavheader[49] = ((SEC_CAL >>8) & 0xff);  
	wavheader[50] = ((SEC_CAL >>16) & 0xff);  
	wavheader[51] = ((SEC_CAL >>24) & 0xff);  

	SEC_CAL = (sectorCount*512)-8; //音频大小,RIFF块后(整个wav文件大小-8)  
	wavheader[4] = (SEC_CAL & 0xff);  
	wavheader[5] = ((SEC_CAL >>8) & 0xff);  
	wavheader[6] = ((SEC_CAL >>16) & 0xff);  
	wavheader[7] = ((SEC_CAL >>24) & 0xff);  
	
	SEC_CAL = (sectorCount-1)*512; //真实录音数据大小,Data块后 
	wavheader[508] = (SEC_CAL & 0xff);  
	wavheader[509] = ((SEC_CAL >>8) & 0xff);
	wavheader[510] = ((SEC_CAL >>16) & 0xff);
	wavheader[511] = ((SEC_CAL >>24) & 0xff);
	
	f_lseek(&fdst,0);  
	f_write(&fdst, wavheader, 512 , &a );  
	f_lseek(&fdst,fdst.fsize);  
	f_close(&fdst);

	return res; 
}

void format_disk(void)		//格式化SD卡
{
	FATFS fs;
	uint8_t res;
	res = f_mount(&fs,"0", 1);
	if (res != FR_OK)
	{
		printf("\r\n挂载文件系统失败,错误代码: %u",res);
		return;
	}	
	printf("\r\n正在格式化磁盘,请稍候...");
	res = f_mkfs(0,1,4096);
	if (res == FR_OK)
	{
		printf("\r\n格式化成功...");
	}
	else
	{
		printf("\r\n格式化失败代码: %u",res);
	}
	f_mount(NULL,"0",1);
}
void get_disk_info(void)	//获取SD卡内容
{
	FATFS fs;
	FATFS *fls = &fs;
	FRESULT res;
	DWORD clust;
	
	res = f_mount(&fs,"0", 1);
	if (res != FR_OK)
	{
		printf("\r\n挂载文件系统失败代码: %u",res);
		return;
	}
	res = f_getfree("/",&clust,&fls);
	if (res == FR_OK)
	{
		printf("\r\n%d MB 可用空间"
					 "\r\n%d MB 总大小",clust * fls->csize /2/1024,(DWORD)(fls->n_fatent - 2) * fls->csize /2/1024);
		//空闲的磁盘空间M=剩余簇数*每簇的扇区数/2/1024
		//总的磁盘空间M        =（总簇数-2）*每簇的扇区数/2/1024=可用簇数*每簇的扇区数/2/1024
		OLED_ShowString(0,1,"Free/Total(MB)");
		OLED_ShowNum(0,2,clust * fls->csize /2/1024,4,12);
		OLED_ShowString(25,2,"/       ");
		OLED_ShowNum(30,2,(DWORD)(fls->n_fatent - 2) * fls->csize /2/1024,4,12);
	}
	
	else
	{
		printf("\r\n获得存储信息失败,代码%u",res);
	}
	
	f_mount(NULL,"0",1);

}

void list_file(void)			//列出文件列表
{
	FATFS fs;
	FILINFO finfo;
	FRESULT res;
	DIR dirs;
	int i;
	int files_num=0;
	res = f_mount(&fs,"0", 1);
	if (res != FR_OK)
	{
		printf("\r\n挂载文件系统失败,代码%u",res);
		return;
	}
	res = f_opendir(&dirs,"/");
	printf("\r\n------------文件列表------------");
	if (res == FR_OK)
	{
		while ((f_readdir(&dirs, &finfo) == FR_OK) && finfo.fname[0]) 
		{
			i = strlen(finfo.fname);
			if (finfo.fattrib & AM_DIR)//如果是文件夹 
			{
				files_num++;
				printf("\r\n/%s", &finfo.fname[0]);
				switch(i)//作用：输出文件名左对齐
				{
				case 1:printf(" ");
				case 2:printf(" ");
				case 3:printf(" ");
				case 4:printf(" ");
				case 5:printf(" ");
				case 6:printf(" ");
				case 7:printf(" ");
				case 8:printf("%15s"," ");
				}				
			} 
			else 
			{
				continue;
			}	
		}
	}
	else
	{
		printf("\r\n打开根目录(list_file)失败,代码:%u",res);
	}
	res = f_opendir(&dirs, "/");
	if (res == FR_OK) 
	{
		while ((f_readdir(&dirs, &finfo) == FR_OK) && finfo.fname[0])//文件夹名字 
		{
			if (finfo.fattrib & AM_DIR) 
			{ 
				continue;
			} 
			else 
			{   
				files_num++;				
				printf("\r\n/.%12s%7ld KB ",  &finfo.fname[0],(finfo.fsize+512)/1024);//文件名字和大小				
			}
		}
		if( files_num==0 )//无文件
		{
			printf("\r\n无文件!");    
		}
	}
	else
	{
		printf("\r\n打开根目录失败,代码%u",res);
	}
	f_mount(NULL,"0",1);
}


uint8_t PrePare_File(void)
{
	//FILINFO finfo;
	uint8_t i;
	uint16_t files_num;
	char pname[25];//带全路径的文件名
  char path[]= {"0:/MUSIC"}; //MUSIC目录
	res = f_mount(&fs,"0", 0);//挂载文件系统到0区,第三位0为了SD取卡的检测
	if (res != FR_OK) return 1;//挂载文件系统失败,返回代码
 	if (f_opendir(&dirs, path) == FR_OK)//打开MUSIC目录
	{
		files_num=0;//清空歌曲文件计数
    while (f_readdir(&dirs, &finfo) == FR_OK) //依次读取文件名
    {
			if(finfo.fname[0]==0)//过了最后一个文件了
			{
				SongPlayedNum=1;//歌曲数目置零
				break;
			}
			if((finfo.fattrib & AM_DIR)!=1)//不是文件夹
			{
				for(i=0;i<6;i++)
				{
						if(strstr( finfo.fname, format[i] )!=NULL)//是音乐文件
						{
							files_num++;
							if(files_num==SongPlayedNum)
							{
								strcpy(pname,path);	//复制目录路径
								strcat(pname,"/");  //接一个地址符号接在后面
								strcat(pname,finfo.fname); //将文件名接在后面
								//OLED_ShowString(50,0,format[i]);
								OLED_ShowNum(0,2,(finfo.fsize+512)/1024/1024,2,12);OLED_ShowString(14,2,"M");
								//printf("\r\n/.%12s%7ld MB ",  &finfo.fname[0],(finfo.fsize+512)/1024/1024);//四舍五入文件大小
								OLED_DrawIcon(0,5,0);//画播放
								OLED_DrawProcess(9,5);//画进度条
								OLED_ShowNum(0,0,SongPlayedNum,3,12);//显示歌曲数目
								OLED_ShowNum(120,0,volume,1,1);//显示音量
								if(f_open(&fsrc,pname,FA_READ)==FR_OK) return 0;
							}
						}
				}
			}
    }
	}
	return 1;
}

void Main_Player(void)
{
	VS_WR_Cmd(SPI_MODE,0x0810);//开启EarSpeaker环绕音效
	OLED_ShowString(0,6,string_bass[0]);
	OLED_DrawIcon(0,5,2);//写停止
	while(1)
	{
		WR_Conf_File();
		while(PrePare_File());
		Song_Player();
		OLED_DrawIcon(0,5,2);//写停止
		//Send_EndByte();
		VS_ST_Reset();
	}
}

void OLED_MusicProcess(void)
{
	float a,b;
	a=fsrc.fptr;
	b=fsrc.fsize;
	now_process=a/b;
	OLED_Set_Pos(now_process*115+10,5);
	OLED_WR_Byte(0x5D,OLED_DATA);
}

uint8_t WR_Conf_File(void)
{
	uint16_t i;
	while((f_mount(&fs,"0", 0))!=FR_OK);//挂载文件系统到0区,第三位0为了SD取卡的检测
	for(i = 0;i < 512;i++)
		{
			buffer[i] = 0x00;
		}
	if((MUSIC_state!=song_play) || (f_open(&fsrc,"0:/CONF",FA_READ)==FR_NO_FILE))
		{
			f_open(&fsrc,"CONF",FA_CREATE_ALWAYS | FA_WRITE);
			f_lseek(&fsrc,0);//偏移到文件头
			buffer[0]=SongPlayedNum;
			buffer[1]=volume;
			f_write(&fsrc,buffer,512,&bw);//然后写入数据
			f_close(&fsrc);
		}
	if((f_read(&fsrc,buffer,512,&br))==FR_OK)
		{
			SongPlayedNum=buffer[0];
			volume=buffer[1];
			VS_WR_Cmd(SPI_VOL,Volume_Buf[volume]); //设置音量
			f_close(&fsrc);
			f_mount(NULL,"0",1);//卸载存储器
			return 1;
		}
	f_mount(NULL,"0",1);//卸载存储器
	return 0;
}


uint8_t Song_Player(void)
{
	uint16_t i,n,re;
		if (res == FR_OK)
		{
			while (1)
			{
				if (MUSIC_state==song_over)//切歌后还原播放恢复模式
				{
						MUSIC_state=song_play;
				}
				for(i = 0;i < 512;i++)
				{
					buffer[i] = 0x00;
				}
				res = f_read(&fsrc,buffer,512,(UINT *)&re);
				i=0;
				VS_XCS=1;
					do                 					//主播放循环
					{
						if(VS_DQ!=0) 		 					//非暂停 送数据给VS10XX 
						{
							VS_XDCS=0;   						//MP3_DATA_CS=0;
							for(n=0;n<32;n++)
								{
									SPI_ReadWriteByte_SPI2(buffer[i++]);	 		
								}
							VS_XDCS=1;   	//MP3_DATA_CS=1;
						}
					}
				while(i<511);//循环发送512个字节
				keyboard_task();
				while (MUSIC_state==song_pause)
				{
					keyboard_task();
				}
				if (MUSIC_state==song_over)
				{
					printf("\r\n切歌,关闭文件");
					f_close(&fsrc);
					break;
				}
				if (res || re == 0)
				{
					MUSIC_state=song_over;
					printf("\r\n读完音乐,关闭文件");
					f_close(&fsrc);
					SongPlayedNum++;
					break;
				}	
			}
		}
return 0;
}


void keyboard_task(void)
{
extern unsigned char key_value;

    if(key_value==0)
        {
				return ;
				}
    switch(key_value)
        {
        case volume_add:
            {
							if(volume<8)
							{
							volume++;
							}
						VS_WR_Cmd(SPI_VOL,Volume_Buf[volume]);  	 //音量加
						OLED_ShowNum(120,0,volume,1,1);
						break;
						}
				case long_volume_add:
            {
							if(volume<8)
							{
							volume++;
							}
						VS_WR_Cmd(SPI_VOL,Volume_Buf[volume]);  	 //音量加
						OLED_ShowNum(120,0,volume,1,1);
						break;
						}
				case volume_sub:
						{
							if(volume>0)
							{
							volume--;
							}
						VS_WR_Cmd(SPI_VOL,Volume_Buf[volume]);  	 //音量减
						OLED_ShowNum(120,0,volume,1,1);
						break;
						}
				case long_volume_sub:
						{
							if(volume>0)
							{
							volume--;
							}
						VS_WR_Cmd(SPI_VOL,Volume_Buf[volume]);  	 //音量减
						OLED_ShowNum(120,0,volume,1,1);
						break;
						}
				case next:
						{
								SongPlayedNum++;
								MUSIC_state=song_over;
						break;
						}
				case long_next:
						{
							unsigned short int SS_DO_NOT_JUMP;
							//unsigned short int	count = 0;
							//unsigned short int	j = 0;
							//int endFillByte = 0;
							float c=now_process+0.05f;
							short int l=0;
							//VS_WR_Cmd(SPI_WRAMADDR,PAR_END_FILL_BYTE);
							//endFillByte = VS_RD_Reg(SPI_WRAM);
							SS_DO_NOT_JUMP=VS_RD_Reg(SPI_STATUS) >> 15;
							OLED_DrawProcess(9,5);//清空进度条
							while(l<now_process*115)
							{
								l++;
								OLED_Set_Pos(l+10,5);
								OLED_WR_Byte(0x5D,OLED_DATA);
							}
							if( c<1&&(!SS_DO_NOT_JUMP))//没有过尾,VS也没有不允许跳.
							{
								f_lseek(&fsrc,fsrc.fsize*c);
								/*
								while ( count < 12288 )
									{
									if (VS_DQ!=0)
										{
										for ( j=0; j<32; j++ )
												{
												SPI_ReadWriteByte_SPI2(endFillByte);
												count++;
												}
										}
									}*/
							}
						break;
						}
				case back:
						{
						if(SongPlayedNum != 1)//第一首就不减了
						{
							SongPlayedNum--;
							MUSIC_state=song_over;
						}
						break;
					}
				case long_back:
						{
								unsigned short int l=0,SS_DO_NOT_JUMP;
								float c;
								c=now_process-0.05f;
								SS_DO_NOT_JUMP=VS_RD_Reg(SPI_STATUS) >> 15;
								OLED_DrawProcess(9,5);//清空进度条
								while(l<now_process*115)
								{
										l++;
										OLED_Set_Pos(l+10,5);
										OLED_WR_Byte(0x5D,OLED_DATA);
								}
								if(c>0&&(!SS_DO_NOT_JUMP))//没有退过头,VS也没有不允许跳.
								{
									//Send_EndByte();
									f_lseek(&fsrc,fsrc.fsize*c);//偏移到目标地址
								}
						break;
						}
				case play:
						{
							play_state++;
							switch(play_state)
								{
									case 1:        //暂停
										{
										MUSIC_state=song_pause;
										VS_WR_Cmd(SPI_MODE,(VS_RD_Reg(SPI_MODE)+0x2000));//暂停位标志1
										OLED_DrawIcon(0,5,1);
										break;
										}
									case 2:        //播放
										{
										MUSIC_state=song_play;
										VS_WR_Cmd(SPI_MODE,(VS_RD_Reg(SPI_MODE)-0x2000));//暂停位标志0
										OLED_DrawIcon(0,5,0);
										play_state=0;
										break;
										}
								}
						break;
						}
				case long_play:
						{
							temp=(sysfun>>5)&0x03;
							if(temp==0x02)//rock->pop
							{
								VS_WR_Cmd(SPI_BASS,Bass_Buffer[0]);
								sysfun&=~(1<<5);
								sysfun&=~(1<<6);
								OLED_ShowString(0,6,"Normal");
							}
							else if(temp==0x01)//classic->rock
							{
								VS_WR_Cmd(SPI_BASS,Bass_Buffer[1]);
								sysfun&=~(1<<5);
								sysfun|=(1<<6);
								OLED_ShowString(0,6,"Bass  ");
							}
							else//pop->classic
							{
								VS_WR_Cmd(SPI_BASS,Bass_Buffer[2]);
								sysfun&=~(1<<6);
								sysfun|=(1<<5);
								OLED_ShowString(0,6,"Light ");
							}
							break;
						}
			}
    key_value=0;
}


uint8_t creat_file(uint8_t *file_name)	  //创建文件
{
	FIL file;
	FIL *pf = &file;
	FATFS fs;
	uint8_t res;

	res = f_mount(&fs,"0", 1);
	if (res != FR_OK)
	{
		return res;
	}	
	res = f_open(pf,(TCHAR *)file_name,FA_READ | FA_WRITE | FA_CREATE_NEW);
	if (res == FR_OK)
	{
		printf("\r\n创建文件成功!");
		res = f_close(pf);
		if (res != FR_OK)
		{
			printf("\r\n创建文件成功,但关闭文件失败代码%u",res);		
		}				
	}
	else
	{
		printf("\r\n创建文件失败,代码%u",res);
	}
	f_mount(NULL,"0",1);
	return res;
}
uint8_t delete_file(uint8_t *file_name)	  //删除文件
{
	FATFS fs;
//	FATFS *fls = &fs;
	FRESULT res;
	res = f_mount(&fs,"0", 1);
	if (res != FR_OK)
	{
		printf("\r\n挂载文件系统失败,代码%u",res);
		return res;
	}	
	res = f_unlink((TCHAR *)file_name);

	if (res == FR_OK)
	{
		printf("\r\n删除文件成功");
	}
	else if (res == FR_NO_FILE)
	{
		printf("\r\n找不到文件或目录");
	}
	else if (res == FR_NO_PATH)
	{
		printf("\r\n找不到路径");
	}
	else
	{
		printf("\r\n错误代码: %u",res);
	}
	f_mount(NULL,"0",1);
	return res;
}
uint8_t creat_dir(uint8_t *file_name)	  	//创建目录
{
	FATFS fs;        
	FRESULT res;     
//	char path[20];
//	uint8_t length;
	res = f_mount(&fs,"0", 1);
	if (res != FR_OK)
	{
		printf("\r\n挂载文件系统失败,代码%u",res);
		return res;
	}	

	res = f_mkdir("0:/RECORDER");
	if (res == FR_OK)
	{
		printf("\r\n创建文件目录成功!");
	}
	else
	{
		printf("\r\n创建目录失败...");
		printf("\r\n错误代码: %u",res);
	}
	f_mount(NULL,"0",1);
	return res;
}

uint8_t read_file(const char *file_name)	//读取文件
{

	//	FATFS *fls = &fs;
	//	BYTE buffer[512];	这里定义不行！！
	//	char path[20];
	//	char buffer[512] = {0};
	//	uint8_t length;
	FATFS fs;
	FIL	file;
	FRESULT res; 
	DIR dirs;
	FILINFO finfo;
	uint8_t *buffer;
	uint32_t i;
	uint32_t re,files_num = 0;
	res = f_mount(&fs,"0", 1);
	if (res != FR_OK)
	{
		printf("\r\n挂载文件系统失败,代码%u",res);
		return res;
	}
	res = f_opendir(&dirs,"/");
	if (res == FR_OK) 
	{
		//i = strlen(path);
		printf("\r\n------------文件列表------------");
		while ((f_readdir(&dirs, &finfo) == FR_OK) && finfo.fname[0]) 
		{
			if (finfo.fattrib & AM_DIR)//如果是文件夹 
			{ 
				continue;
			} 
			else 								   
			{   
				files_num++;
				//显示文件名,显示文件实际大小 ,文件实际大小采用四舍五入法
				printf("\r\n/%12s%7ld KB ",  &finfo.fname[0],(finfo.fsize+512)/1024);

			}
		}
		if( files_num == 0 )//无文件
		{
			printf("\r\n无文件,请返回先创建文件!");
			return res;
			
		}
	}
	else
	{
		printf("\r\n打开根目录(read_file)失败,代码:%u",res);
	}											 
//	printf("\r\n输入文件全名,以回车结束...");
//	get_file_name((uint8_t *)path,length,FI);
	res = f_open(&file,file_name,FA_READ);
	printf("\r\n正在打开文件,以下是文件数据:\r\n");

	if (res == FR_OK)
	{
		while (1)
		{

			for(i = 0;i < 512;i++)
			{
				buffer[i] = 0x00;
			}
			res = f_read(&file,buffer,512,(UINT*)&re);
			printf("%s",buffer);

			if (res || re == 0)
			{
				printf("\r\n文件读取结束,关闭文件!");
				f_close(&file);
				break;
			}	
		}
	}
	f_mount(NULL,"0",1);
	return res;
}


uint8_t edit_file(uint8_t *file_name,uint8_t * file_buff,uint32_t index)	  		//编辑文件
{
	FATFS fs;
	FIL	file;
	FRESULT res; 
	DIR dirs;
	FILINFO finfo;
	uint32_t reindex = 0x00;
	uint32_t files_num = 0;

	res = f_mount(&fs,"0", 1);
	if (res != FR_OK)
	{
		printf("\r\n挂载文件系统失败,代码%u",res);
		return res;
	}
	res = f_opendir(&dirs,"/");
	printf("\r\n------------文件列表------------");
	if (res == FR_OK) 
	{
		//i = strlen(path);		
		while ((f_readdir(&dirs, &finfo) == FR_OK) && finfo.fname[0]) 
		{
			if (finfo.fattrib & AM_DIR)//如果是文件夹 
			{ 
				continue;
			} 
			else 
			{   
				files_num++;
				//显示文件名,显示文件实际大小 ,文件实际大小采用四舍五入法
				printf("\r\n/%12s%7ld KB ",  &finfo.fname[0],(finfo.fsize+512)/1024);

			}
		}
		if( files_num == 0 )//无文件
		{
			printf("\r\n无文件!");    
		}
	}
	else
	{
		printf("\r\n打开根目录(edit_file)失败,代码:%u",res);
	}
//	printf("\r\n输入要编辑文件全名,以回车结束...");
//	get_file_name((uint8_t *)path,length,FI);

	res = f_open(&file,(const TCHAR*)file_name,FA_READ | FA_WRITE);
	if (res == FR_OK)
	{
	
		printf("\r\n打开文件 %s 成功",file_name);
		printf("\r\n现在是文件写测试,请输入要写入的数据!");		
		printf("\r\n按 ESC 或者 Ctrl+C 结束编辑!\r\n");
				
		printf("\r\n保存数据...");
		res = f_write(&file,file_buff,index,(UINT*)&reindex);
		if ((res == FR_OK) && (reindex == index))
		{
			printf("\r\n保存数据成功!");
			f_close(&file);
			index = 0x00;
			reindex = 0x00;									
		}
		else
		{
			printf("\r\n保存数据失败!");
			printf("\r\n错误代码: %u",res);									
		}

				
	}
	else
	{
		printf("\r\n打开文件失败,代码%u",res);
	}
	return res;
}

uint8_t check_file_name(uint8_t *file_name,uint8_t length)		//检查文件名字是否合法
{
	uint8_t res;
	if (length > 13)
	{
		res = 1;
	}
	else
	{
		if (file_name[length - 4] == '.')
		{
			res = 0;
		}
		else
		{
			res = 2;
		}
	}
	return res;
}

