#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FILE *fp;

struct _fat32_disk
{
	char *path;
	unsigned char sectors_per_cul;
	unsigned short save_sectors;
	unsigned char fat_num;
	unsigned int sectors_per_fat;
	unsigned int root_start_sector;
};

typedef struct _fat32_disk fat32_disk;

struct _fat32_files
{
	char *name;
	char *ext;
	unsigned int start_sector;
	unsigned short length;
	struct _fat32_files *next;
};

typedef struct _fat32_files fat32_files;

static fat32_disk disk;

int open_disk(char *path);
int check_fat_flag();
void disk_reset();
int bin2Int(char *buf,int offest);
fat32_files *getGroundFileTree();
static void fillFileDesc(char *buf,int index,fat32_files *ff);

int check_fat_flag()
{
	disk_reset();
	char buf[512];
	fread(buf,512,1,fp);
	char flag[5] = "FAT32";
	int i;
	for(i=0;i<5;i++)
	{
		if(buf[0x52+i]!=flag[i])
		{
			disk_reset();
			return 0;
		}
	}
	disk_reset();
	return 1;
}

void disk_reset()
{
	fseek(fp,0,SEEK_SET);
	return;
}

void readSectors(char *buf,unsigned int num,unsigned long start_sector)
{
	if(num==0|buf==NULL)
		return;
	disk_reset();
	fseek(fp,512*start_sector,SEEK_SET);
	fread(buf,512,num,fp);
	disk_reset();
}

int bin2Int(char *buf,int offest)
{
	buf+=offest;
	return buf[0]|buf[1]<<8|buf[2]<<16|buf[3]<<24;
}

short bin2Short(char *buf,int offest)
{
	buf+=offest;
	return buf[0]|buf[1]<<8;
}

int open_disk(char *path)
{
	fp = fopen(path,"rb+");
	if(fp==NULL)
		return -1;
	if(check_fat_flag()==0)
		return -1;
	disk.path = (char *)malloc(sizeof(char)*strlen(path)+1);
	strcpy(disk.path,path);
	char buf[512];
	readSectors(buf,1,0);
	disk.sectors_per_cul = buf[0xd];
	disk.save_sectors = bin2Short(buf,0x0e);
	disk.fat_num = buf[0x10];
	disk.sectors_per_fat = bin2Int(buf,0x24);
	disk.root_start_sector = bin2Int(buf,0x2c);
}

static void fillFileDesc(char *buf,int index,fat32_files *ff)
{
	if(index>=16)
	{
		ff->length = 0;
		return;
	}
	buf+=index*32;
	ff->start_sector = bin2Short(buf,0x1a)|(bin2Short(buf,0x14))<<16;
	ff->length = bin2Short(buf,0x1c);
	ff->next = (fat32_files *)malloc(sizeof(fat32_files));
	ff->name = (char *)malloc(sizeof(char)*9);	// You see,I'm too lazy to support long name
	char *_name = buf;
	memcpy(ff->name,_name,sizeof(ff->name)-1);
	ff->name[8] = '\0';
	ff->ext = (char *)malloc(sizeof(buf)*4);	// qwq
	char *_ext = buf;
	_ext+=0x08;
	memcpy(ff->ext,_ext,sizeof(ff->ext)-1);
	ff->ext[8] = '\0';
	
}

fat32_files *getGroundFileTree()
{
	char buf[512];
	short i = 0;
	fat32_files *tree = (fat32_files *)malloc(sizeof(fat32_files));
	fat32_files *head = tree;
	do{
		if(i%16==0)
			readSectors(buf,1,disk.save_sectors+disk.sectors_per_fat*disk.fat_num+(i/16)*512);
		fillFileDesc(buf,i%16,tree);
		tree = tree->next;
		i++;
	}while(i<=5);
	/* 明天再做:
	 * bug: Found file:Aa. in clu -1 length:65535B
Found file:A      .TXT  in clu 34 length:83B
Found file:��. in clu -124 length:113B
Found file:坫���~.    in clu 4 length:0B
Found file:�a. in clu -1 length:65535B
Found file:�      .TXT  in clu 33 length:83B
Found file:(null).(null) in clu 0 length:0B
	* 懂?,OK
	*/
	tree->next = NULL;
	return head;
}

void close_disk()
{
	fclose(fp);
}

int main()
{
	if(open_disk("c.img")==-1){
		printf("Open disk failed!\n");
		close_disk();
		return 1;
	}
	printf("%d,%d,%d,%d,%d\n",disk.sectors_per_cul,disk.save_sectors,(int)disk.fat_num,disk.sectors_per_fat,disk.root_start_sector);
	fat32_files * gtree = getGroundFileTree();
	while(gtree!=NULL)
	{
		printf("Found file:%s.%s in clu %d length:%dB\n",gtree->name,gtree->ext,gtree->start_sector,gtree->length);
		gtree = gtree->next;
	}
	close_disk();
	return 0;
}
