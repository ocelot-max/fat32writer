#ifndef _DEVICES_H
#define _DEVICES_H

void devices_map_init();
unsigned long write_sectors(unsigned long dev_id,
		unsigned long start_sector,
		unsigned long sectors,
		const char *data);
char * read_sectors(unsigned long dev_id,
		unsigned long start_sector,
		unsigned long sectors);

#endif