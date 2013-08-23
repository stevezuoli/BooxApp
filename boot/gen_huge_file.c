#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <file_name> <size>\n", argv[0]);
		return -1;
	}

	int fd = open(argv[1], O_RDWR);
	if (fd == -1)
	{
		perror("open");
		return -1;
	}

	off_t size = atoi(argv[2]) * 1024 * 1024;
	printf("Change file size to %ld\n", size);
	if (ftruncate(fd, size) != 0)
	{
		perror("ftruncate");
		return -1;
	}

	close(fd);
	return 0;
}

