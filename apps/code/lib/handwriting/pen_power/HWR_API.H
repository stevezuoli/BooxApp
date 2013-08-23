/*
FileName:       hwr_api.h
Description:    C header file for handwriting recognition engine interface
*/

#define HWR_API_H

#ifndef BYTE
#define BYTE    unsigned char
#endif

#ifndef WORD
#define WORD    unsigned short
#endif

#ifndef DWORD
#define DWORD   unsigned long
#endif

#define LOWER_ALPHA_TYPE		1
#define UPPER_ALPHA_TYPE		2
#define NUMA_TYPE			4
#define NUMERIC_TYPE		4
#define GENERAL_PUNCT_TYPE		8
#define EXTEND_PUNCT_TYPE		16
#define GENERAL_SYMBOL_TYPE		32
#define EXTEND_SYMBOL_TYPE		64
#define GESTURE_TYPE		128
#define RADICAL_TYPE		256
#define CHINESE_1_TYPE		512
#define CHINESE_2_TYPE		1024
#define SIMPLIFIED_ONLY	          4096
#define TRADITIONAL_ONLY                8192
#define VALID_TYPE          		0x000007FF

#define SYMBOL_TYPE			(GENERAL_PUNCT_TYPE|EXTEND_PUNCT_TYPE|GENERAL_SYMBOL_TYPE|EXTEND_SYMBOL_TYPE)
#define IN_5401_TYPE		CHINESE_1_TYPE
#define IN_13052_TYPE		CHINESE_2_TYPE
#define ALPHA_TYPE			(UPPER_ALPHA_TYPE|LOWER_ALPHA_TYPE)
#define CURSIVE_TYPE		(ALPHA_TYPE|NUMA_TYPE|SYMBOL_TYPE|GESTURE_TYPE)
#define ALL_TYPE			VALID_TYPE

#define TRADITIONAL_PHRASE		0
#define SIMPLIFIED_PHRASE		1

/* Following are the return value definition for the mandatory API functions */
#define STATUS_OK                   0x0000	/* Function finished without error */
#define STATUS_NO_DICT              0x0001	/* No dictionary assigned */
#define STATUS_INVALID_DICT         0x0002	/* Dictionary format wrong */
#define STATUS_INVALID_TYPE         0x0003	/* Required range not in current dictionary */
#define STATUS_INVALID_NUMBER       0x0004	/* Required candidate number is invalid */
#define STATUS_NO_ENOUGH_MEMORY     0x0005	/* Dynamic memory assigned not enough */
#define STATUS_INVALID_HWRDATA_PTR  0x0006      /* NULL HWRDATA Pointer */
#define STATUS_INVALID_PRIVATE_PTR  0x0007      /* NULL Private Data Pointer */
#define STATUS_INVALID_BOX_PTR	    0x0008      /* NULL Box Pointer */
#define STATUS_INVALID_TRACE_PTR    0x0009      /* NULL TRACE PTR */
#define STATUS_INVALID_RESULT_PTR   0x000A      /* NULL RESULT PTR */
#define STATUS_INVALID_DBENDIAN     0x000B      /* Incorrect Database Endian */
#define STATUS_UNKNOWN              0x00FE	/* Unspecified error occured */

/* Following is the data definition for HWR engine */
typedef struct tagHWRData
{
 signed char*	pInternalDict; /* The pointer to standard HWR dictionary */
 signed char*	pExternalDict; /* The pointer to external/customer HWR dictionary, maybe used for user customization */
 signed char*	pPrivate;      /* The pointer to RAM for HWR engine internal usage */
} HWRData;

typedef struct tagHWRBOX
{
  short		left;
  short		top;
  short		right;
  short		bottom;
} HWRBOX;
typedef HWRBOX* LPHWRBOX;

typedef struct tagPOINT_TYPE
{
  short		x;
  short		y;
} POINT_TYPE;
typedef POINT_TYPE* LPPOINT_TYPE;

/* Following are the mandatory API functions */
#ifdef CPP_SOURCE
extern "C"{
unsigned short PPHWRInit(HWRData* pData);
unsigned long  PPHWRGetRamSize(void);	/* Return the size of memory in pPrivate, in bytes. */
unsigned short PPHWRSetType(HWRData* pData,
		        DWORD type);
unsigned short PPHWRSetBox(HWRData* pData,
		       LPHWRBOX pbox);
unsigned short PPHWRSetCandidateNum(HWRData* pData,
		                WORD number);
unsigned short PPHWRRecognize(HWRData* pData,
                              WORD* pTrace,
		          WORD* pResult);

int PPHWRFindPhrase(unsigned short code,
                unsigned short size_of_return_buffer,
                unsigned short *return_buffer,
                signed char area_flag);
/*
參數:
code : 用來搜尋詞庫的鍵值.
size_of_return_buffer : 返回結果暫存區的大小 in bytes.
return_buffer : 返回結果暫存區的指針.
area_flag : 繁體(0)或者是簡體(1)詞庫搜尋

返回值:
在暫存區中的結果的個數(in WORD).
*/
};
#endif


#ifdef C_SOURCE
extern unsigned short PPHWRInit(HWRData* pData);
extern unsigned long  PPHWRGetRamSize(void);	/* Return the size of memory in pPrivate, in bytes. */
extern unsigned short PPHWRSetType(HWRData* pData,
		        DWORD type);
extern unsigned short PPHWRSetBox(HWRData* pData,
		       LPHWRBOX pbox);
extern unsigned short PPHWRSetCandidateNum(HWRData* pData,
		                WORD number);
extern unsigned short PPHWRRecognize(HWRData* pData,
                              WORD* pTrace,
		          WORD* pResult);
extern int PPHWRFindPhrase(unsigned short code,
                       unsigned short size_of_return_buffer,
                       unsigned short *return_buffer,
                       signed char area_flag);
#endif
