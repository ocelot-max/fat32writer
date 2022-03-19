#include <stdio.h>
#include <devices.h>

extern void a();

int main()
{
    devices_map_init();
    a();
    return 0;
}