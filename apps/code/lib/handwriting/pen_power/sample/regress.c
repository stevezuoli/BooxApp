#define C_SOURCE
#include <stdio.h>
#include "hwr_api.h"
#include "ink.c"
#define NO_CANDIDATE 10
POINT_TYPE RawStroke[1024];

unsigned short code_array[20];
unsigned char WorkBuf[8192];
HWRData theHWRData;
HWRBOX  box;

int main(void)
{
  short i,no_point,flag,no_cand;
  short *ptr_ink;
  struct Global_area1 *ptr_global_area1;
  unsigned short code;
  short char_count;
  unsigned long value;
  int RamSize;

  i=0;
  char_count=0;

  /* 初始化核心之前, 一定要調用本函數 */
  RamSize=PPHWRGetRamSize();
  /*
     利用剛才取得的RamSize來配置工作記憶體;
     如果是採用靜態宣告的陣列, 則可以用來驗證大小
  */
  theHWRData.pPrivate=(signed char*)WorkBuf; //WorkBuf為靜態陣列

  /*
      如果數據庫是與程序碼分開燒錄的時候,
      必須指定數據庫的開始地址.
      目前不用指定
  */
  /* 核心初始化 */
  if(PPHWRInit(&theHWRData)!=STATUS_OK){
    return -2;
  }
  value=ALL_TYPE;
  /* 設定識別字集 */
  PPHWRSetType(&theHWRData,(DWORD)value);

  /* 設定候選字個數 */
  PPHWRSetCandidateNum(&theHWRData,10);

  /* 預設的書寫格大小及位置為 (0,0,60,60)
     如果您的不是這個樣子, 必須改變以下這個函數的參數
  */
  box.left=box.top=0;
  box.right=box.bottom=60;
  PPHWRSetBox(&theHWRData,
	      &box);


  ptr_ink=(short *)&ink_data[0];
  flag=1;
  while(flag)
  {

      no_point=*ptr_ink++;
      if(no_point<=0)break;
      for(i=0;i<no_point;i++)
      {
          RawStroke[i].x=*ptr_ink++;
          RawStroke[i].y=*ptr_ink++;
      }

      if(PPHWRRecognize(&theHWRData,
          (WORD *)RawStroke,
          code_array)!=STATUS_OK){
              printf("Cannot Recognize!\n");
              return -3;
      }
      no_cand=0;
      while(code_array[no_cand]) no_cand++;

      if(no_cand!=*ptr_ink++)
      {
          printf("Candidate Count miss-match %d %d!\n",
              no_cand, *(ptr_ink-1));
          flag=0;
          break;
      }

      for(i=0;i<no_cand;i++)
      {
          code = *ptr_ink++;
          if(code_array[i]!=code)
          {
              printf("Candidate code miss-match %d %04x %04x!\n",
                  i,code, code_array[i]);
              flag=0;
              break;
          }
      }
      if(i!=no_cand)break;

      char_count++;
      printf("Regressioned Character=%d\n", char_count);
  }

  printf("Total Regressioned Character=%d\n",
         char_count);
  return(char_count);
}

