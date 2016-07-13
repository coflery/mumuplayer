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
char *format[6] = {".MP3",".OGG",".WAV",".WMA",".MID",".FLA"};//���ļ���ֻ�ܶ���������չ��
const int Volume_Buf[9] ={0x7777,0x6969,0x6060,0x5252,0x4242,0x3636,0x2727,0x1919,0x1111};
uint8_t volume=6;				//Ҳ����VS1053��DefaultVolume
float now_process;
/* <<<<<<<<<<<<<<<<�ļ�ϵͳ����<<<<<<<<<<<<<<<< */
			FATFS fs;            // Work area (file system object) for logical drive
			FIL fsrc, fdst;      // file objects
			BYTE buffer[512]; 	 // file copy buffer
			UINT br, bw;         // File R/W count
			FRESULT res;
			DIR dirs;
			FILINFO finfo;
/* <<<<<<<<<<<<<<<<�ļ�ϵͳ����<<<<<<<<<<<<<<<< */
					
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
const unsigned char RIFFHeader0[] = { 'R' , 'I' , 'F' , 'F' , // �� ID (RIFF) 
                                      0x70, 0x70, 0x70, 0x70, // �洢���С (¼������) 
                                      'W' , 'A' , 'V' , 'E' , // RIFF �ļ�����
                                      'f' , 'm' , 't' , ' ' , // �� ID (fmt ) 
                                      0x14, 0x00, 0x00, 0x00, // ������ (0x14 = 20 bytes)
                                      0x11, 0x00,             // ��ʽ����(IMA ADPCM) 0x01,��ʾPCM ; 0x11,��ʾIMA ADPCM
                                      0x01, 0x00,             // ������ (2)
                                      0x44, 0xac, 0x00, 0x00, // ������ (0x0c80 = 32.0kHz) 
                                      0x53, 0x57, 0x01, 0x00, // �ֽ�����(16.0khz)
                                      0x00, 0x01,             // ���ݿ��С (512 bytes)  
                                      0x10, 0x00,             // �������� ADPCM ���ݴ�С (4 bits)
	                                    0x02, 0x00,             // ��չ���� (2 bytes) 
                                      0xf9, 0x01,             // ÿ��Ĳ����� (505 samples) 
                                      'f' , 'a' , 'c' , 't' , // �� ID (fact) 
                                      0xc8, 0x01, 0x00, 0x00, // ������ (456 bytes (zeropad!))
                                      0x70, 0x70, 0x70, 0x70  // ������ (¼������) 
																			};
//add 452 zeros after
const unsigned char RIFFHeader504[] = { 'd' , 'a' , 't' , 'a' , // �� ID (data) 
																			  0x70, 0x70, 0x70, 0x70  // ������ (calculate after rec!) 
																			};

/*¼����д�ļ�����Ϊ: 
1��дheader�����䵽512�ֽڣ� 
2��д��Ƶ���ݣ���¼�¶��ٸ�sector��1��sectorΪ512�ֽڣ� 
3������¼���ļ���С�����Ӧ��ֵ��䵽header�� 
(2�����У�ÿ128wordΪһ��block��ÿblock�ĵ��ĸ�����Ӧ����0x00������ͨ������������ж������Ƿ���ȷ�ϴ� )
*/
u8 VS_recorde_save(void)
{
	unsigned char blockNumber=0;
	unsigned long sectorCount=1;
	unsigned long SEC_CAL = 0;
	unsigned int  oneword;
	unsigned char databuffer[512];/* �������ݻ����� */
	unsigned char wavheader[512];
  unsigned int a, temp;
  FRESULT res;
  unsigned char* dataBufPtr;

	VS_Recorde_Init();//��ʼ����ʹVS10XX����¼��״̬ 
  f_mount(&fs,"0", 1);
  f_open(&fdst, "voice.wav", FA_CREATE_ALWAYS | FA_WRITE); 

	//�����headerΪ��дsector���㣬��չΪ512�ֽ�
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
   while (VS_RD_Reg(SPI_HDAT1)>>8);  //���������ڻ�����,�ȴ���������������Ϊ0
   dataBufPtr  = databuffer; 		     //���� dataBufPtr �� filebuffer �Ŀ�ʼ��
   blockNumber = 0;
   while(recording_on==0);
	 OLED_ShowString(0,3,"REC NOW");
	 while(recording_on)
   {
	     if(VS_RD_Reg(SPI_HDAT1) >= 128)
	     {
				if (dataBufPtr>(databuffer+511))
					{
						while(1);//ֹͣ
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

  SEC_CAL=(sectorCount-1)*1010;  //���������� 
	wavheader[48] = (SEC_CAL & 0xff);  
	wavheader[49] = ((SEC_CAL >>8) & 0xff);  
	wavheader[50] = ((SEC_CAL >>16) & 0xff);  
	wavheader[51] = ((SEC_CAL >>24) & 0xff);  

	SEC_CAL = (sectorCount*512)-8; //��Ƶ��С,RIFF���(����wav�ļ���С-8)  
	wavheader[4] = (SEC_CAL & 0xff);  
	wavheader[5] = ((SEC_CAL >>8) & 0xff);  
	wavheader[6] = ((SEC_CAL >>16) & 0xff);  
	wavheader[7] = ((SEC_CAL >>24) & 0xff);  
	
	SEC_CAL = (sectorCount-1)*512; //��ʵ¼�����ݴ�С,Data��� 
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

void format_disk(void)		//��ʽ��SD��
{
	FATFS fs;
	uint8_t res;
	res = f_mount(&fs,"0", 1);
	if (res != FR_OK)
	{
		printf("\r\n�����ļ�ϵͳʧ��,�������: %u",res);
		return;
	}	
	printf("\r\n���ڸ�ʽ������,���Ժ�...");
	res = f_mkfs(0,1,4096);
	if (res == FR_OK)
	{
		printf("\r\n��ʽ���ɹ�...");
	}
	else
	{
		printf("\r\n��ʽ��ʧ�ܴ���: %u",res);
	}
	f_mount(NULL,"0",1);
}
void get_disk_info(void)	//��ȡSD������
{
	FATFS fs;
	FATFS *fls = &fs;
	FRESULT res;
	DWORD clust;
	
	res = f_mount(&fs,"0", 1);
	if (res != FR_OK)
	{
		printf("\r\n�����ļ�ϵͳʧ�ܴ���: %u",res);
		return;
	}
	res = f_getfree("/",&clust,&fls);
	if (res == FR_OK)
	{
		printf("\r\n%d MB ���ÿռ�"
					 "\r\n%d MB �ܴ�С",clust * fls->csize /2/1024,(DWORD)(fls->n_fatent - 2) * fls->csize /2/1024);
		//���еĴ��̿ռ�M=ʣ�����*ÿ�ص�������/2/1024
		//�ܵĴ��̿ռ�M        =���ܴ���-2��*ÿ�ص�������/2/1024=���ô���*ÿ�ص�������/2/1024
		OLED_ShowString(0,1,"Free/Total(MB)");
		OLED_ShowNum(0,2,clust * fls->csize /2/1024,4,12);
		OLED_ShowString(25,2,"/       ");
		OLED_ShowNum(30,2,(DWORD)(fls->n_fatent - 2) * fls->csize /2/1024,4,12);
	}
	
	else
	{
		printf("\r\n��ô洢��Ϣʧ��,����%u",res);
	}
	
	f_mount(NULL,"0",1);

}

void list_file(void)			//�г��ļ��б�
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
		printf("\r\n�����ļ�ϵͳʧ��,����%u",res);
		return;
	}
	res = f_opendir(&dirs,"/");
	printf("\r\n------------�ļ��б�------------");
	if (res == FR_OK)
	{
		while ((f_readdir(&dirs, &finfo) == FR_OK) && finfo.fname[0]) 
		{
			i = strlen(finfo.fname);
			if (finfo.fattrib & AM_DIR)//������ļ��� 
			{
				files_num++;
				printf("\r\n/%s", &finfo.fname[0]);
				switch(i)//���ã�����ļ��������
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
		printf("\r\n�򿪸�Ŀ¼(list_file)ʧ��,����:%u",res);
	}
	res = f_opendir(&dirs, "/");
	if (res == FR_OK) 
	{
		while ((f_readdir(&dirs, &finfo) == FR_OK) && finfo.fname[0])//�ļ������� 
		{
			if (finfo.fattrib & AM_DIR) 
			{ 
				continue;
			} 
			else 
			{   
				files_num++;				
				printf("\r\n/.%12s%7ld KB ",  &finfo.fname[0],(finfo.fsize+512)/1024);//�ļ����ֺʹ�С				
			}
		}
		if( files_num==0 )//���ļ�
		{
			printf("\r\n���ļ�!");    
		}
	}
	else
	{
		printf("\r\n�򿪸�Ŀ¼ʧ��,����%u",res);
	}
	f_mount(NULL,"0",1);
}


uint8_t PrePare_File(void)
{
	//FILINFO finfo;
	uint8_t i;
	uint16_t files_num;
	char pname[25];//��ȫ·�����ļ���
  char path[]= {"0:/MUSIC"}; //MUSICĿ¼
	res = f_mount(&fs,"0", 0);//�����ļ�ϵͳ��0��,����λ0Ϊ��SDȡ���ļ��
	if (res != FR_OK) return 1;//�����ļ�ϵͳʧ��,���ش���
 	if (f_opendir(&dirs, path) == FR_OK)//��MUSICĿ¼
	{
		files_num=0;//��ո����ļ�����
    while (f_readdir(&dirs, &finfo) == FR_OK) //���ζ�ȡ�ļ���
    {
			if(finfo.fname[0]==0)//�������һ���ļ���
			{
				SongPlayedNum=1;//������Ŀ����
				break;
			}
			if((finfo.fattrib & AM_DIR)!=1)//�����ļ���
			{
				for(i=0;i<6;i++)
				{
						if(strstr( finfo.fname, format[i] )!=NULL)//�������ļ�
						{
							files_num++;
							if(files_num==SongPlayedNum)
							{
								strcpy(pname,path);	//����Ŀ¼·��
								strcat(pname,"/");  //��һ����ַ���Ž��ں���
								strcat(pname,finfo.fname); //���ļ������ں���
								//OLED_ShowString(50,0,format[i]);
								OLED_ShowNum(0,2,(finfo.fsize+512)/1024/1024,2,12);OLED_ShowString(14,2,"M");
								//printf("\r\n/.%12s%7ld MB ",  &finfo.fname[0],(finfo.fsize+512)/1024/1024);//���������ļ���С
								OLED_DrawIcon(0,5,0);//������
								OLED_DrawProcess(9,5);//��������
								OLED_ShowNum(0,0,SongPlayedNum,3,12);//��ʾ������Ŀ
								OLED_ShowNum(120,0,volume,1,1);//��ʾ����
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
	VS_WR_Cmd(SPI_MODE,0x0810);//����EarSpeaker������Ч
	OLED_ShowString(0,6,string_bass[0]);
	OLED_DrawIcon(0,5,2);//дֹͣ
	while(1)
	{
		WR_Conf_File();
		while(PrePare_File());
		Song_Player();
		OLED_DrawIcon(0,5,2);//дֹͣ
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
	while((f_mount(&fs,"0", 0))!=FR_OK);//�����ļ�ϵͳ��0��,����λ0Ϊ��SDȡ���ļ��
	for(i = 0;i < 512;i++)
		{
			buffer[i] = 0x00;
		}
	if((MUSIC_state!=song_play) || (f_open(&fsrc,"0:/CONF",FA_READ)==FR_NO_FILE))
		{
			f_open(&fsrc,"CONF",FA_CREATE_ALWAYS | FA_WRITE);
			f_lseek(&fsrc,0);//ƫ�Ƶ��ļ�ͷ
			buffer[0]=SongPlayedNum;
			buffer[1]=volume;
			f_write(&fsrc,buffer,512,&bw);//Ȼ��д������
			f_close(&fsrc);
		}
	if((f_read(&fsrc,buffer,512,&br))==FR_OK)
		{
			SongPlayedNum=buffer[0];
			volume=buffer[1];
			VS_WR_Cmd(SPI_VOL,Volume_Buf[volume]); //��������
			f_close(&fsrc);
			f_mount(NULL,"0",1);//ж�ش洢��
			return 1;
		}
	f_mount(NULL,"0",1);//ж�ش洢��
	return 0;
}


uint8_t Song_Player(void)
{
	uint16_t i,n,re;
		if (res == FR_OK)
		{
			while (1)
			{
				if (MUSIC_state==song_over)//�и��ԭ���Żָ�ģʽ
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
					do                 					//������ѭ��
					{
						if(VS_DQ!=0) 		 					//����ͣ �����ݸ�VS10XX 
						{
							VS_XDCS=0;   						//MP3_DATA_CS=0;
							for(n=0;n<32;n++)
								{
									SPI_ReadWriteByte_SPI2(buffer[i++]);	 		
								}
							VS_XDCS=1;   	//MP3_DATA_CS=1;
						}
					}
				while(i<511);//ѭ������512���ֽ�
				keyboard_task();
				while (MUSIC_state==song_pause)
				{
					keyboard_task();
				}
				if (MUSIC_state==song_over)
				{
					printf("\r\n�и�,�ر��ļ�");
					f_close(&fsrc);
					break;
				}
				if (res || re == 0)
				{
					MUSIC_state=song_over;
					printf("\r\n��������,�ر��ļ�");
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
						VS_WR_Cmd(SPI_VOL,Volume_Buf[volume]);  	 //������
						OLED_ShowNum(120,0,volume,1,1);
						break;
						}
				case long_volume_add:
            {
							if(volume<8)
							{
							volume++;
							}
						VS_WR_Cmd(SPI_VOL,Volume_Buf[volume]);  	 //������
						OLED_ShowNum(120,0,volume,1,1);
						break;
						}
				case volume_sub:
						{
							if(volume>0)
							{
							volume--;
							}
						VS_WR_Cmd(SPI_VOL,Volume_Buf[volume]);  	 //������
						OLED_ShowNum(120,0,volume,1,1);
						break;
						}
				case long_volume_sub:
						{
							if(volume>0)
							{
							volume--;
							}
						VS_WR_Cmd(SPI_VOL,Volume_Buf[volume]);  	 //������
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
							OLED_DrawProcess(9,5);//��ս�����
							while(l<now_process*115)
							{
								l++;
								OLED_Set_Pos(l+10,5);
								OLED_WR_Byte(0x5D,OLED_DATA);
							}
							if( c<1&&(!SS_DO_NOT_JUMP))//û�й�β,VSҲû�в�������.
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
						if(SongPlayedNum != 1)//��һ�׾Ͳ�����
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
								OLED_DrawProcess(9,5);//��ս�����
								while(l<now_process*115)
								{
										l++;
										OLED_Set_Pos(l+10,5);
										OLED_WR_Byte(0x5D,OLED_DATA);
								}
								if(c>0&&(!SS_DO_NOT_JUMP))//û���˹�ͷ,VSҲû�в�������.
								{
									//Send_EndByte();
									f_lseek(&fsrc,fsrc.fsize*c);//ƫ�Ƶ�Ŀ���ַ
								}
						break;
						}
				case play:
						{
							play_state++;
							switch(play_state)
								{
									case 1:        //��ͣ
										{
										MUSIC_state=song_pause;
										VS_WR_Cmd(SPI_MODE,(VS_RD_Reg(SPI_MODE)+0x2000));//��ͣλ��־1
										OLED_DrawIcon(0,5,1);
										break;
										}
									case 2:        //����
										{
										MUSIC_state=song_play;
										VS_WR_Cmd(SPI_MODE,(VS_RD_Reg(SPI_MODE)-0x2000));//��ͣλ��־0
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


uint8_t creat_file(uint8_t *file_name)	  //�����ļ�
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
		printf("\r\n�����ļ��ɹ�!");
		res = f_close(pf);
		if (res != FR_OK)
		{
			printf("\r\n�����ļ��ɹ�,���ر��ļ�ʧ�ܴ���%u",res);		
		}				
	}
	else
	{
		printf("\r\n�����ļ�ʧ��,����%u",res);
	}
	f_mount(NULL,"0",1);
	return res;
}
uint8_t delete_file(uint8_t *file_name)	  //ɾ���ļ�
{
	FATFS fs;
//	FATFS *fls = &fs;
	FRESULT res;
	res = f_mount(&fs,"0", 1);
	if (res != FR_OK)
	{
		printf("\r\n�����ļ�ϵͳʧ��,����%u",res);
		return res;
	}	
	res = f_unlink((TCHAR *)file_name);

	if (res == FR_OK)
	{
		printf("\r\nɾ���ļ��ɹ�");
	}
	else if (res == FR_NO_FILE)
	{
		printf("\r\n�Ҳ����ļ���Ŀ¼");
	}
	else if (res == FR_NO_PATH)
	{
		printf("\r\n�Ҳ���·��");
	}
	else
	{
		printf("\r\n�������: %u",res);
	}
	f_mount(NULL,"0",1);
	return res;
}
uint8_t creat_dir(uint8_t *file_name)	  	//����Ŀ¼
{
	FATFS fs;        
	FRESULT res;     
//	char path[20];
//	uint8_t length;
	res = f_mount(&fs,"0", 1);
	if (res != FR_OK)
	{
		printf("\r\n�����ļ�ϵͳʧ��,����%u",res);
		return res;
	}	

	res = f_mkdir("0:/RECORDER");
	if (res == FR_OK)
	{
		printf("\r\n�����ļ�Ŀ¼�ɹ�!");
	}
	else
	{
		printf("\r\n����Ŀ¼ʧ��...");
		printf("\r\n�������: %u",res);
	}
	f_mount(NULL,"0",1);
	return res;
}

uint8_t read_file(const char *file_name)	//��ȡ�ļ�
{

	//	FATFS *fls = &fs;
	//	BYTE buffer[512];	���ﶨ�岻�У���
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
		printf("\r\n�����ļ�ϵͳʧ��,����%u",res);
		return res;
	}
	res = f_opendir(&dirs,"/");
	if (res == FR_OK) 
	{
		//i = strlen(path);
		printf("\r\n------------�ļ��б�------------");
		while ((f_readdir(&dirs, &finfo) == FR_OK) && finfo.fname[0]) 
		{
			if (finfo.fattrib & AM_DIR)//������ļ��� 
			{ 
				continue;
			} 
			else 								   
			{   
				files_num++;
				//��ʾ�ļ���,��ʾ�ļ�ʵ�ʴ�С ,�ļ�ʵ�ʴ�С�����������뷨
				printf("\r\n/%12s%7ld KB ",  &finfo.fname[0],(finfo.fsize+512)/1024);

			}
		}
		if( files_num == 0 )//���ļ�
		{
			printf("\r\n���ļ�,�뷵���ȴ����ļ�!");
			return res;
			
		}
	}
	else
	{
		printf("\r\n�򿪸�Ŀ¼(read_file)ʧ��,����:%u",res);
	}											 
//	printf("\r\n�����ļ�ȫ��,�Իس�����...");
//	get_file_name((uint8_t *)path,length,FI);
	res = f_open(&file,file_name,FA_READ);
	printf("\r\n���ڴ��ļ�,�������ļ�����:\r\n");

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
				printf("\r\n�ļ���ȡ����,�ر��ļ�!");
				f_close(&file);
				break;
			}	
		}
	}
	f_mount(NULL,"0",1);
	return res;
}


uint8_t edit_file(uint8_t *file_name,uint8_t * file_buff,uint32_t index)	  		//�༭�ļ�
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
		printf("\r\n�����ļ�ϵͳʧ��,����%u",res);
		return res;
	}
	res = f_opendir(&dirs,"/");
	printf("\r\n------------�ļ��б�------------");
	if (res == FR_OK) 
	{
		//i = strlen(path);		
		while ((f_readdir(&dirs, &finfo) == FR_OK) && finfo.fname[0]) 
		{
			if (finfo.fattrib & AM_DIR)//������ļ��� 
			{ 
				continue;
			} 
			else 
			{   
				files_num++;
				//��ʾ�ļ���,��ʾ�ļ�ʵ�ʴ�С ,�ļ�ʵ�ʴ�С�����������뷨
				printf("\r\n/%12s%7ld KB ",  &finfo.fname[0],(finfo.fsize+512)/1024);

			}
		}
		if( files_num == 0 )//���ļ�
		{
			printf("\r\n���ļ�!");    
		}
	}
	else
	{
		printf("\r\n�򿪸�Ŀ¼(edit_file)ʧ��,����:%u",res);
	}
//	printf("\r\n����Ҫ�༭�ļ�ȫ��,�Իس�����...");
//	get_file_name((uint8_t *)path,length,FI);

	res = f_open(&file,(const TCHAR*)file_name,FA_READ | FA_WRITE);
	if (res == FR_OK)
	{
	
		printf("\r\n���ļ� %s �ɹ�",file_name);
		printf("\r\n�������ļ�д����,������Ҫд�������!");		
		printf("\r\n�� ESC ���� Ctrl+C �����༭!\r\n");
				
		printf("\r\n��������...");
		res = f_write(&file,file_buff,index,(UINT*)&reindex);
		if ((res == FR_OK) && (reindex == index))
		{
			printf("\r\n�������ݳɹ�!");
			f_close(&file);
			index = 0x00;
			reindex = 0x00;									
		}
		else
		{
			printf("\r\n��������ʧ��!");
			printf("\r\n�������: %u",res);									
		}

				
	}
	else
	{
		printf("\r\n���ļ�ʧ��,����%u",res);
	}
	return res;
}

uint8_t check_file_name(uint8_t *file_name,uint8_t length)		//����ļ������Ƿ�Ϸ�
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

