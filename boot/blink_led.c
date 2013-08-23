#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define GPIO1_BASE_ADDR 0x53FCC000
#define GPIO1_DR        (GPIO1_BASE_ADDR)
#define GPIO1_GDIR      (GPIO1_BASE_ADDR + 0x4)

int main(int argc, char* argv[])
{
	int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mem_fd == -1)
    {
        fprintf(stderr, "Can't open /dev/mem!\n");
        return -1;
    }

    void *p = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, GPIO1_BASE_ADDR);
    if (p == MAP_FAILED)
    {
        fprintf(stderr, "mmap 0x%x failed.\n", GPIO1_BASE_ADDR);
        return -1;
    }

    volatile unsigned int* gpio1_dr = (unsigned int *)((unsigned int)p + 0);
    volatile unsigned int* gpio1_dir = (unsigned int *)((unsigned int)p + 4);

    /* Set MX31_PIN_GPIO1_4 direction */
    *gpio1_dir |= (1 << 4);

    while (1)
    {
        *gpio1_dr |= (1 << 4);
        
        sleep(1);
        
        *gpio1_dr &= ~(1 << 4);
        
        sleep(1);
    }

    return 0;
}
