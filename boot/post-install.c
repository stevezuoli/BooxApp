#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define SYSCONFIG_BLKDEV "/dev/mtd1"
#define LANG_CFGFILE     "/root/Settings/language"
#define SYSCONFIG_SIZE   2048

struct SysConfig
{
	char         version[16];
	unsigned int program_time;
	char         serial_no[16];
	char         dev_id[16];
	unsigned int screen_size;    /* 60, 80, 97 */
	unsigned int x_pixels;
	unsigned int y_pixels;
	unsigned int grayscale;      /* 8, 16 */
	char         language[16];   /* Default language */
};

int main(int argc, char* argv[])
{
    int fd = open(SYSCONFIG_BLKDEV, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Can't open %s for reading!\n", SYSCONFIG_BLKDEV);
        return -1;
    }

	char sys_config[SYSCONFIG_SIZE];

    if (pread(fd, sys_config, SYSCONFIG_SIZE, 0xFF800) != SYSCONFIG_SIZE)
    {
        perror("pread");
        return -1;
    }
    
    close(fd);
    
    struct SysConfig* p = (struct SysConfig *)sys_config;
    
    FILE* fp = fopen(LANG_CFGFILE, "w");
    if (fp == NULL)
	{
		fprintf(stderr, "Can't open %s for writing!\n", LANG_CFGFILE);
		return -1;
	}
	
	char tmp[32] = "export LANG=";
	strcat(tmp, p->language);
	fwrite(tmp, 1, strlen(tmp), fp);
	fclose(fp);
	return 0; 
}
