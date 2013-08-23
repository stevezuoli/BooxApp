
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bs_cmd.h"


int lut_conflicts;

int main(int argc, char **argv){

  unsigned short ra, val;
  unsigned int i;
  char s[16];

  if(argc != 2){
    printf("Usage: bs_rd_reg <register address>\n\r");
    return(-1);
  }

  strcpy(s,*++argv);

  ra = 0;
  i = 0;
  val = 0;

  /* convert address */
  if(strncmp(s,"0x",2) == 0){
    while(s[i+2] != 0 && i<4){
      ra = ra<<4;
      if(s[i+2]>='0' && s[i+2]<='9') ra += s[i+2] - '0';
      if(s[i+2]>='a' && s[i+2]<='f') ra += s[i+2] - 'a' + 10;
      if(s[i+2]>='A' && s[i+2]<='F') ra += s[i+2] - 'A' + 10;
      i++;
    }
  }
  else
    ra = atoi(s);

  bs_cmd_flag_hif_mode_cmd(); // required for access to broadsheet commands

  val = bs_cmd_rd_reg(ra);
  printf("0x%x\n\r",val);

  return(0);
}
