#ifndef _DEVICES_H
#define _DEVICES_H

void devices_map_init();
unsigned long loop_device(char *path);
int deloop_device(unsigned long de_id);
void list_devices();
char * search_device_by_id(unsigned long s_id);
unsigned long search_device_by_path(char *path);
unsigned long write_sectors(unsigned long dev_id,
		unsigned long start_sector,
		unsigned long sectors,
		const char *data);
char * read_sectors(unsigned long dev_id,
		unsigned long start_sector,
		unsigned long sectors);

#endif