#ifndef _FATFS_H_
#define _FATFS_H_  1

#include "sys.h"
#include "oled.h" 
#include "ff.h"
#include "diskio.h"
#include <string.h>

extern unsigned int vol;

void keyboard_task(void);			//处理按键
void format_disk(void);				//格式化SD卡
void get_disk_info(void);			//获取SD卡内容 
void list_file(void);					//列出文件列表
void Main_Player(void);				//主播放程序
void Pic_Viewer(void);
void OutPutFile(void);
void OLED_MusicProcess(void);
uint8_t WR_Conf_File(void);
uint8_t VS_recorde_save(void);
uint8_t Song_Player(void);
uint8_t PrePare_File(void);									//准备文件开始播放
uint8_t creat_file(uint8_t *file_name);			//创建文件
uint8_t delete_file(uint8_t *file_name);		//删除文件
uint8_t creat_dir(uint8_t *file_name);			//创建目录
uint8_t read_file(const char *file_name);		//读取文件
uint8_t check_file_name(uint8_t *file_name,uint8_t length); //检查文件名字是否合法
uint8_t edit_file(uint8_t *file_name,uint8_t * file_buff,uint32_t index);	 //编辑文件
/******************************************************************************/
#define BlockSize            512 /* Block Size in Bytes */
#define BufferWordsSize      (BlockSize >> 2)
#define NumberOfBlocks       2  /* For Multi Blocks operation (Read/Write) */
#define MultiBufferWordsSize ((BlockSize * NumberOfBlocks) >> 2)
static
FRESULT scan_files (char* path);
/******************************************************************************/
#endif
