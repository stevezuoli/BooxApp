
#include <stdio.h>
#include "HZRecog.h"

//"“ª"
static short pStroke1[1000] = {0, 50, 100, 50, -1,0,-1,-1};
//"»À"
static short pStroke2[1000] = {50,0,0,50,-1,0,30,25,100,50,-1,0,-1,-1};
//"∞À"
static short pStroke3[1000] = {50,50,0,100,-1,0,60,50,100,100,-1,0,-1,-1};

#define BYTE	unsigned char
BYTE *Address = NULL;

void InitHzRecog()
{
	long lpos;
	FILE *fp = fopen(".\\jHWRDic.dat","rb");
	fseek(fp,0,SEEK_END);
	lpos = ftell(fp);
	Address = (BYTE *)malloc(lpos);
	if(Address == NULL)
	{
		printf("malloc failed!");
		return;
	}
	fseek(fp,0,SEEK_SET);
	fread(Address,1,lpos,fp);
	fclose(fp);
	{
	int bFlag = HZInitCharacterRecognition(Address);
	}
}

void ExitHZRecog()
{
	HZExitCharacterRecognition();
	if (Address != NULL)
		free(Address);
}

#include <time.h>
float tClock, tTotalClock;
int iTotalNum;
int main()
{
	int i, ret;
	unsigned short wResult[21];
	wResult[10] = 0;

 	InitHzRecog();
	iTotalNum = 0;
	tTotalClock = 0.0;
	i = CLOCKS_PER_SEC;
	printf("%d\n", i);
	{
		tClock = clock();
		ret = HZCharacterRecognize(pStroke1,wResult);
		tClock = (clock()-tClock)/CLOCKS_PER_SEC;
		printf("%f\n", tClock);
		if(ret != 10)
			printf("len error!");
 		printf("%d,%s\n",wResult[0],(char *)wResult);
		iTotalNum++;
		tTotalClock += tClock;

		tClock = clock();
		ret = HZCharacterRecognize(pStroke2,wResult);
		tClock = (clock()-tClock)/CLOCKS_PER_SEC;
		printf("%f\n", tClock);
		if(ret != 10)
			printf("len error!");
 		printf("%d,%s\n",wResult[0],(char *)wResult);
		iTotalNum++;
		tTotalClock += tClock;

		tClock = clock();
		ret = HZCharacterRecognize(pStroke3,wResult);
		tClock = (clock()-tClock)/CLOCKS_PER_SEC;
		printf("%f\n", tClock);
		if(ret != 10)
			printf("len error!");
 		printf("%d,%s\n",wResult[0],(char *)wResult);
		iTotalNum++;
		tTotalClock += tClock;
	}
	printf("num = %d total time = %f average time = %f\n", iTotalNum, tTotalClock, tTotalClock/iTotalNum);

	ExitHZRecog();

	return 1;
}

