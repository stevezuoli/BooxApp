
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bs_cmd.h"



int lut_conflicts;

int main(int argc, char **argv){

  unsigned short wfma;
  unsigned int i;
  char s[16];

  if(argc != 2){
    printf("Usage: bs_set_wfm <wfm address>\n\r");
    return(-1);
  }
  strcpy(s,*++argv);

  wfma = 0;
  i = 0;

  /* convert address */
  if(strncmp(s,"0x",2) == 0){
    while(s[i+2] != 0 && i<4){
      wfma = wfma<<4;
      if(s[i+2]>='0' && s[i+2]<='9') wfma += s[i+2] - '0';
      if(s[i+2]>='a' && s[i+2]<='f') wfma += s[i+2] - 'a' + 10;
      if(s[i+2]>='A' && s[i+2]<='F') wfma += s[i+2] - 'A' + 10;
      i++;
    }
  }
  else
    wfma = atoi(s);

  bs_cmd_flag_hif_mode_cmd(); // required for access to broadsheet commands
  bs_cmd_set_wfm(wfma);       // set up wfm data in broadsheet regs

  return(0);
}
