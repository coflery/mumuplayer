#ifndef _FATFS_H_
#define _FATFS_H_  1

#include "sys.h"
#include "oled.h" 
#include "ff.h"
#include "diskio.h"
#include <string.h>

extern unsigned int vol;

void keyboard_task(void);			//������
void format_disk(void);				//��ʽ��SD��
void get_disk_info(void);			//��ȡSD������ 
void list_file(void);					//�г��ļ��б�
void Main_Player(void);				//�����ų���
void Pic_Viewer(void);
void OutPutFile(void);
void OLED_MusicProcess(void);
uint8_t WR_Conf_File(void);
uint8_t VS_recorde_save(void);
uint8_t Song_Player(void);
uint8_t PrePare_File(void);									//׼���ļ���ʼ����
uint8_t creat_file(uint8_t *file_name);			//�����ļ�
uint8_t delete_file(uint8_t *file_name);		//ɾ���ļ�
uint8_t creat_dir(uint8_t *file_name);			//����Ŀ¼
uint8_t read_file(const char *file_name);		//��ȡ�ļ�
uint8_t check_file_name(uint8_t *file_name,uint8_t length); //����ļ������Ƿ�Ϸ�
uint8_t edit_file(uint8_t *file_name,uint8_t * file_buff,uint32_t index);	 //�༭�ļ�
/******************************************************************************/
#define BlockSize            512 /* Block Size in Bytes */
#define BufferWordsSize      (BlockSize >> 2)
#define NumberOfBlocks       2  /* For Multi Blocks operation (Read/Write) */
#define MultiBufferWordsSize ((BlockSize * NumberOfBlocks) >> 2)
static
FRESULT scan_files (char* path);
/******************************************************************************/
#endif
