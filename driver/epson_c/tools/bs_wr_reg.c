
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bs_cmd.h"



int lut_conflicts;

int main(int argc, char **argv){

  unsigned short ra, val;
  unsigned int i;
  char s[16];

  if(argc != 3){
    printf("Usage: bs_wr_reg <register address> <write value>\n\r");
    return(-1);
  }

  ra = 0;
  val = 0;

  /* convert address */
  strcpy(s,*++argv);
  i = 0;
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


  /* convert value */
  strcpy(s,*++argv);
  i = 0;
  if(strncmp(s,"0x",2) == 0){
    while(s[i+2] != 0 && i<4){
      val = val<<4;
      if(s[i+2]>='0' && s[i+2]<='9') val += s[i+2] - '0';
      if(s[i+2]>='a' && s[i+2]<='f') val += s[i+2] - 'a' + 10;
      if(s[i+2]>='A' && s[i+2]<='F') val += s[i+2] - 'A' + 10;
      i++;
    }
  }
  else
    val = atoi(s);


  bs_cmd_flag_hif_mode_cmd(); // required for access to broadsheet commands
  bs_cmd_wr_reg(ra, val);

  return(0);
}
