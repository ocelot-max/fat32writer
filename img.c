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
	unsigned int length;
	unsigned char flag;
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

fat32_files *getGroundFileTree()
{
	fat32_files *tree = (fat32_files *)malloc(sizeof(fat32_files));
	fat32_files *head = tree;
	char *buf = (char *)malloc(512);
	char *ba = buf;
	unsigned long rindex = disk.save_sectors+disk.sectors_per_fat*disk.fat_num;
	int c;
	while(1)
	{
		if(c%16==0)
			readSectors(buf,1,rindex+c/16);
		for(c=0;;c++)
		{
			tree->flag = buf[0x0b];
			tree->start_sector = bin2Short(buf,0x1a)|bin2Short(buf,0x14)<<16;
			tree->length = bin2Int(buf,0x1c);
			if(tree->flag==0&&tree->start_sector==0&&tree->length==0)
				goto ret;
			char *_name = buf;
			char *_ext = buf+0x08;
			tree->name = (char *)malloc(9);
			tree->ext = (char *)malloc(4);
			memcpy(tree->name,_name,8);
			int i;
			for(i=0;i<8;i++)
			{
				if(tree->name[i]==0x20){
					tree->name[i] = '\0';
					break;
				}
			}
			tree->name[8] = '\0';
			memcpy(tree->ext,_ext,3);
			for(i=0;i<3;i++)
			{
				if(tree->ext[i]==0x20){
					tree->ext[i] = '\0';
					break;
				}
			}
			tree->ext[3] = '\0';
			if(buf[0x0]==0xe5||tree->flag!=0x20)
				// 现在只扫描文件
				tree->next = tree;
			else
				tree->next = (fat32_files *)malloc(sizeof(fat32_files));
	
			tree = tree->next;
			buf+=32;
		}
		buf = ba;
	}
ret:
	tree->next = NULL;
	return head;
}

char *readFile(fat32_files *info,unsigned int start_sector,unsigned int sectors)
{
	if(info==NULL||info->length==0||info->flag!=0x20||
			start_sector>info->length/512||start_sector+sectors>info->length/512)
		return NULL;
	char *data = (char *)malloc(512*sectors);
	//(char *)malloc(info->length); 这样写在文件足够大(具体取绝于机器)时会导致段错误
	char *bdata = data;
	char _fat[512];
	char *fat = (char *)&_fat;
	char *ba = fat;
	readSectors(fat,1,disk.save_sectors+info->start_sector/128);
	unsigned long sindex = disk.save_sectors+disk.sectors_per_fat*disk.fat_num+info->start_sector-2;
	readSectors(data,1,sindex);
	int enity = bin2Int(fat,sizeof(int)*info->start_sector);
	int c = 0;
	while(enity<0xfffffff8&&start_sector>c)
	{
		if(c%128==0&&c>0)
			readSectors(fat,1,disk.save_sectors+info->start_sector/128+c/128);
		enity = bin2Int(fat,sizeof(int)*enity);
		readSectors(data,1,sindex+enity);
		c++;
	}
	
	return bdata;
}

fat32_files *new_file(char *name,char *ext)
{

}

void close_disk()
{
	if(fp==NULL)
		fclose(fp);
}

int main()
{
	if(open_disk("a.img")==-1){
		printf("Open disk failed!\n");
		close_disk();
		return 1;
	}
	printf("%d,%d,%d,%d,%d\n",disk.sectors_per_cul,disk.save_sectors,(int)disk.fat_num,disk.sectors_per_fat,disk.root_start_sector);
	fat32_files * gtree = getGroundFileTree();
	int i;
	char *a;
	for(i=0;i<gtree->length/512;i++)
	{
		a = readFile(gtree,i,1);
		sleep(1);
		printf("%s",a);
		fflush(stdout);
	}
	printf("%s",a);
	while(gtree!=NULL)
	{
		printf("Found file:%s.%s in clu %d length:%dB,with flag %d\n",gtree->name,gtree->ext,gtree->start_sector,gtree->length,(int)gtree->flag);
		gtree = gtree->next;
	}
	close_disk();
	return 0;
}
