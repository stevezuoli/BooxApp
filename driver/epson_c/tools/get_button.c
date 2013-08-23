
/***************************************************************************
 *   Written for gumstix
 *      Holly Gates 7/19/2004
 *   Updated for AM300
 *      Holly Gates 5/08/2008
 *
 * Usage:
 *     o get_button
 *
 * Compile with:
 *        arm-linux-gcc -o get_button get_button.c pxaregs.o
 *
 **************************************************************************/

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define B1_ADDR 0x40E00008
#define B1_PIN 17
#define B2_ADDR 0x40E00008
#define B2_PIN 19
#define B3_ADDR 0x40E00008
#define B3_PIN 18
#define B4_ADDR 0x40E00000
#define B4_PIN 20

#define USE_PAGE_NUMBER_FOR_MMAP 1
#define MAP_SIZE 4096
#define MAP_MASK ( MAP_SIZE - 1 )


void usage(void){
  printf("Usage: get_button <block signal, 0 for non-blocking>\n\r");
  return;
}

int main(int argc, char *argv[])
{

  static int fd = -1;
  unsigned char b_state=0;
  char *map, *map2, *map3, *regaddr;
  unsigned int blocking=0;
  unsigned int val;

  if(argc != 2){
    usage();
    return(1);
  }

  blocking = atoi(*++argv);

  if (fd == -1) {
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd<0) {
      perror("open(\"/dev/mem\")");
      return(1);
    }
  }

  /* do mmap for accessing button GPIOs */
#ifdef USE_PAGE_NUMBER_FOR_MMAP
  map = (char*)mmap(0, MAP_SIZE,
         PROT_READ | PROT_WRITE,
         MAP_SHARED, fd,
         B1_ADDR / MAP_MASK);
#else
  map = (char*)mmap(0, MAP_SIZE,
         PROT_READ | PROT_WRITE,
         MAP_SHARED, fd,
         B1_ADDR & ~MAP_MASK);
#endif
  if (map == (void*)-1 ) {
    perror("mmap()");
    return(1);
  }


  while(b_state == 0){


    regaddr = map + (B1_ADDR & MAP_MASK);
    val = *(unsigned int *) regaddr;
    b_state += ((~val >> B1_PIN) & 0x01);

    regaddr = map + (B2_ADDR & MAP_MASK);
    val = *(unsigned int *) regaddr;
    b_state += ((~val >> B2_PIN) & 0x01)<<1;

    regaddr = map + (B3_ADDR & MAP_MASK);
    val = *(unsigned int *) regaddr;
    b_state += ((~val >> B3_PIN) & 0x01)<<2;

    regaddr = map + (B4_ADDR & MAP_MASK);
    val = *(unsigned int *) regaddr;
    b_state += ((~val >> B4_PIN) & 0x01)<<3;


    if(blocking == 0)
      break;

    if(b_state == 0){
      /*  sched_yield();*/
      usleep(20000);
    }
  }

  printf("%i",b_state);
  munmap(0,MAP_SIZE);

  return(0);

}
