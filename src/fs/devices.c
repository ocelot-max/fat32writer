#include <stdio.h>

struct data_pool
// 数据缓冲池
{
        char            *data;			// 数据
		unsigned char	pool_flag;		// 缓冲属性(R/W)
        unsigned long   data_size;		// 数据大小
        unsigned long   data_start_sector;	// 设备开始缓冲扇区
		struct data_pool *next;			// 链表结构
};

#define POOL_FLAG_READ_ONLY	0x00		// 只读
#define POOL_FLAG_WRITE_RIGHT	0x01		// 立即写入
#define POOL_FLAG_WRITE_FLUSH	0x02		// 缓冲写入

typedef struct data_pool        pool_data;

struct dev_visual
// 虚拟磁盘设备
{
	char		source_path[128];	// 数据源路径
	unsigned int	id;			// 设备id
	pool_data	*data_pool;		// 数据缓冲池
	struct dev_visual *next;
};

typedef struct dev_visual	visual_dev;

static visual_dev	*dev_map;		// 设备表

static void die_error(char *error);
static visual_dev * find_device(unsigned long dev_id);
static char * read_sectors_no_pool(unsigned long dev_id,
		unsigned long start_sector,
		unsigned long sectors);
static unsigned long write_sectors_no_pool(unsigned long dev_id,
                unsigned long start_sector,
                unsigned long sectors,
                const char * data);

void devices_map_init()
// 设备表初始化
{
	dev_map = (visual_dev *) malloc(sizeof(visual_dev));
	dev_map->next = NULL;
	dev_map->data_pool = NULL;
	return;
}

static void die_error(char *error)
{
	fprintf(stderr,"\e[31m%s\n\e[0m",error);
	exit(1);
}

unsigned long loop_device(char *path)
{
	FILE *fp = fopen(path,"rb");
	if(fp==NULL)
		die_error("Error,Can't open source file!");
	visual_dev *dmap = dev_map;
//	srand(time(0));
	unsigned int new_id = rand();
	while(new_id==dmap->id)
		new_id = rand();
	while( dmap->next != NULL )
	{
		if(dmap->id==new_id)
			while(new_id==dmap->id)
				new_id = rand();
		dmap = dmap->next;
	}
	dmap->next = (visual_dev *)malloc(sizeof(visual_dev));
	dmap = dmap->next;
	dmap->id = new_id;
	strcpy(dmap->source_path,path);
	dmap->next = NULL;
	
	return dmap->id;
}

int deloop_device(unsigned long de_id)
{
	visual_dev *dmap = dev_map;
	visual_dev *up;
	visual_dev *down = dmap->next;
	while((dmap!=NULL) && (dmap->id!=de_id))
	{
		up = dmap;
		dmap = dmap->next;
		down = dmap->next;
	}
	if(dmap==NULL)
	{
		die_error("Error,No such Device");
		return -1;
	}
	else
	{
		up->next = down;
		free(dmap);
		return de_id;
	}
}

void list_devices()
{
	visual_dev *dmap = dev_map ->next;
	int i = 1;
	while(dmap!=NULL)
	{
		fprintf(stdin,"[%d] path: %s\tid: %d\n",i,dmap->source_path,dmap->id);
		i++;
		dmap = dmap->next;
	}
	return;
}

char * search_device_by_id(unsigned long s_id)
{
	visual_dev *dmap = dev_map;
	while(dmap!=NULL && dmap->id!=s_id)
		dmap = dmap->next;
	return (dmap==NULL) ? NULL:dmap->source_path;
}

unsigned long search_device_by_path(char *path)
{
	visual_dev *dmap = dev_map;
	while(dmap!=NULL && strcmp(dmap->source_path,path)!=0)
		dmap = dmap->next;
	return (dmap==NULL) ? 0:dmap->id;
}

static visual_dev * find_device(unsigned long dev_id)
{
	visual_dev *dmap = dev_map;
	while(dmap!=NULL && dmap->id!=dev_id)
		dmap = dmap->next;
	return (dmap==NULL) ? NULL:dmap;
}

#define BYTES_PER_SECTOR	512

static char * read_sectors_no_pool(unsigned long dev_id,
		unsigned long start_sector,
		unsigned long sectors)
{
	visual_dev *dev = find_device(dev_id);
	if(dev==NULL)
		return NULL;
	FILE *fp = fopen(dev->source_path,"rb");
	if(fp == NULL)
		return NULL;
	char *buf = (char *)malloc(sectors*BYTES_PER_SECTOR);
	char *save = buf;
	fseek(fp,start_sector*BYTES_PER_SECTOR,SEEK_SET);
	fread(buf,BYTES_PER_SECTOR,sectors,fp);
	fclose(fp);
	return buf;
}

static unsigned long write_sectors_no_pool(unsigned long dev_id,
		unsigned long start_sector,
		unsigned long sectors,
		const char * data)
{
	if(data==NULL | sectors==0)
		return 0;
	visual_dev *dev = find_device(dev_id);
	if(dev==NULL)
		return 0;
	FILE *fp = fopen(dev->source_path,"rb+");
	if(fp==NULL)
		return 0;
	fseek(fp,start_sector*BYTES_PER_SECTOR,SEEK_SET);
	fwrite(data,BYTES_PER_SECTOR,sectors,fp);
	return sectors;
}

/* 注记:
 * 1.缓冲区读写先不写了
 * 2.loop_device里rand()有问题,每次运行数字一样
 */

unsigned long write_sectors(unsigned long dev_id,
		unsigned long start_sector,
		unsigned long sectors,
		const char *data)
{
	return write_sectors_no_pool(dev_id,start_sector,sectors,data);
}

char * read_sectors(unsigned long dev_id,
		unsigned long start_sector,
		unsigned long sectors)
{
	return read_sectors_no_pool(dev_id,start_sector,sectors);
}