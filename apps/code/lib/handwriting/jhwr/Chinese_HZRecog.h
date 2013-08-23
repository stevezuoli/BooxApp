#ifdef __cplusplus
extern "C"    {
#endif

#ifdef HZRECOG_EXPORTS
	#define	HZRECOGAPI __declspec( dllexport )
#else
	#define	HZRECOGAPI /*__declspec( dllimport )*/
#endif

#define HZ_VOID void
#define HZ_INT8 char
#define HZ_UINT8 unsigned char
#define HZ_INT16 short
#define HZ_UINT16 unsigned short
#define HZ_INT32 long				
#define HZ_UINT32 unsigned long		

#define HZ_BOOL HZ_INT32
#define HZ_BYTE HZ_UINT8			
#define HZ_WORD HZ_UINT16			
#define HZ_DWORD HZ_UINT32			

// Recognition Kernel Initialization
// call this function before using other functions in handwritting recognition kernel
// the pointer is ROM address for placing HZRecog.dat or is file name string
// Return value :	TRUE	success
//					FALSE	fail
HZRECOGAPI HZ_BOOL  HZInitCharacterRecognition(HZ_VOID* pPointer);

// Recognition Kernel Exit
// call this function before using other functions handwritting recognition kernel no more
// Return value :	TRUE	success
//					FALSE	fail
HZRECOGAPI HZ_BOOL  HZExitCharacterRecognition(HZ_VOID);

// Main Recognition Function
// Return value is recognition result number, its valid value are [-1, MAX_CANDIDATE_NUM]
// pnStrokeBuffer is handwritting buffer of Chinese character.
// (x, y) make up one point, and x and y are short type, their valid value are from -1 to 32767.
// (-1, 0) is end flag of stroke, (-1, -1) is end flag of character.
// pwResultBuffer is recognition result buffer, we recommend its size are MAX_CANDIDATE_NUM+1
// uMatchRange is one of four kinds of match_range or combination of them. Note: it is not null(zero)
// moreover, uMatchRange may include ADAPTATION_MATCH_RANGE only after calling 'HZInitAdaptation'
HZRECOGAPI HZ_INT32  HZCharacterRecognize(HZ_INT16* pnStrokeBuffer, HZ_WORD* pwResult);
HZRECOGAPI HZ_INT32  HZCharacterRecognize2(HZ_INT16* pnStrokeBuffer, HZ_WORD* pwResult, HZ_INT32* piDistance);

#define RECOG_RANGE_NUMBER			0x00000001		// 数字，0~9十个数字
#define RECOG_RANGE_UPPERCASE		0x00000002		// 大写字母，26个
#define RECOG_RANGE_LOWERCASE		0x00000004		// 小写字母，26个
#define RECOG_RANGE_INTERPUNCTION	0x00000008		// 标点符号，43个
#define RECOG_RANGE_GESTURE			0x00000010		// 笔势符号，36个
///// 以下这些定义不要使用
#define RECOG_RANGE_GB1_ONLY		0x00000100
#define RECOG_RANGE_GB1_BIG51		0x00000200
#define RECOG_RANGE_GB1_BIG52		0x00000400
#define RECOG_RANGE_GB2_ONLY		0x00000800
#define RECOG_RANGE_GB2_BIG51		0x00001000
#define RECOG_RANGE_GB2_BIG52		0x00002000
#define RECOG_RANGE_BIG51_ONLY		0x00004000
#define RECOG_RANGE_BIG52_ONLY		0x00008000
#define RECOG_RANGE_GBK_ONLY		0x00010000
#define RECOG_RANGE_GB2_RADICAL		0x00020000
///// 以上这些定义不要使用
#define RECOG_RANGE_ADAPTATION		0x10000000		// 自学习字

// 国标一级汉字，总共3755个汉字
#define RECOG_RANGE_GB1				(RECOG_RANGE_GB1_ONLY | RECOG_RANGE_GB1_BIG51 | RECOG_RANGE_GB1_BIG52)
// 国标二级汉字，总共3008个汉字
#define RECOG_RANGE_GB2				(RECOG_RANGE_GB2_ONLY | RECOG_RANGE_GB2_BIG51 | RECOG_RANGE_GB2_BIG52)
// Big5常用汉字，总共5401个汉字
#define RECOG_RANGE_BIG51			(RECOG_RANGE_GB1_BIG51 | RECOG_RANGE_GB2_BIG51 | RECOG_RANGE_BIG51_ONLY)
// Big5非常用汉字，总共7659个汉字
#define RECOG_RANGE_BIG52			(RECOG_RANGE_GB1_BIG52 | RECOG_RANGE_GB2_BIG52 | RECOG_RANGE_BIG52_ONLY)

// 字母符号，总共52个大小写字母
#define RECOG_RANGE_SYMBOL			(RECOG_RANGE_UPPERCASE | RECOG_RANGE_LOWERCASE)
// 国标一二级汉字，总共6763个汉字
#define RECOG_RANGE_GB				(RECOG_RANGE_GB1 | RECOG_RANGE_GB2)
// Big5汉字，总共13060个汉字
#define RECOG_RANGE_BIG5			(RECOG_RANGE_BIG51 | RECOG_RANGE_BIG52)
// GBK全部汉字(包括偏旁部首)，总共21003个汉字
#define RECOG_RANGE_GBK				(RECOG_RANGE_GB | RECOG_RANGE_BIG5 | RECOG_RANGE_GBK_ONLY | RECOG_RANGE_GB2_RADICAL)
#define RECOG_RANGE_ALL				(RECOG_RANGE_NUMBER | RECOG_RANGE_SYMBOL | RECOG_RANGE_INTERPUNCTION | RECOG_RANGE_GESTURE | RECOG_RANGE_GBK | RECOG_RANGE_ADAPTATION)

// DisplayPage
#define DP_NOCHANGE			0		// 简繁体不做变化
#define DP_TOSIMPLIFIED		1		// 写繁得简
#define DP_TOTRADITIONAL	2		// 写简得繁

// Full or Half Character
#define FH_FULL				0		// 全角字符
#define FH_HALF				1		// 半角字符

// Define Gesture
#define DG_GESTURE_NUM		36 
#define DG_ZERO				0x0000	// 无笔势
#define DG_SPACE			0x0020	// 空格(0x0020)
#define DG_ENTER			0x000D	// 回车(0x000D)
#define DG_BACKSPACE		0x0008	// 回删(0x0008)
#define DG_DELETE			0x001E	// 删除(0x001E)

typedef enum 
{
	PARAM_CANDNUMB,			// 1~100，推荐使用10，缺省是10
	PARAM_RECORANG,			// RECOG_RANGE_xxx，缺省是GB
	PARAM_DISPCODE,			// DP_xxx,缺省是不变
	PARAM_FULLHALF,			// FH_xxx，缺省是全角
	PARAM_DEFGESTURE,		// 传入36维向量（36个WORD），对36个写法进行笔势定义
	PARAM_SPEEDUP,			// 可以获得更快的速度,建议不要设置此参数,缺省是不设置
} HZPARAM;

HZRECOGAPI HZ_VOID HZSetParam(HZPARAM nParam, HZ_UINT32 dwValue);
HZRECOGAPI HZ_UINT32 HZGetParam(HZPARAM nParam);

#ifdef __cplusplus
}
#endif 



