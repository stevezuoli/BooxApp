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
HZRECOGAPI HZ_BOOL  HZInitCharacterRecognition_Multi(HZ_VOID* pPointer);

// Recognition Kernel Exit
// call this function before using other functions handwritting recognition kernel no more
// Return value :	TRUE	success
//					FALSE	fail
HZRECOGAPI HZ_BOOL  HZExitCharacterRecognition_Multi(HZ_VOID);

// Main Recognition Function
// Return value is recognition result number, its valid value are [-1, MAX_CANDIDATE_NUM]
// pnStrokeBuffer is handwritting buffer of Chinese character.
// (x, y) make up one point, and x and y are short type, their valid value are from -1 to 32767.
// (-1, 0) is end flag of stroke, (-1, -1) is end flag of character.
// pwResultBuffer is recognition result buffer, we recommend its size are MAX_CANDIDATE_NUM+1
// uMatchRange is one of four kinds of match_range or combination of them. Note: it is not null(zero)
// moreover, uMatchRange may include ADAPTATION_MATCH_RANGE only after calling 'HZInitAdaptation'
HZRECOGAPI HZ_INT32  HZCharacterRecognize_Multi(HZ_INT16* pnStrokeBuffer, HZ_WORD* pwResult);

#define RECOG_RANGE_NUMBER			0x00000001		// 数字，0~9十个数字
#define RECOG_RANGE_UPPERCASE		0x00000002		// 大写字母，比如在英文中是26个
#define RECOG_RANGE_LOWERCASE		0x00000004		// 小写字母，比如在英文中是26个
#define RECOG_RANGE_INTERPUNCTION	0x00000008		// 标点符号，比如在汉字字典中43个
#define RECOG_RANGE_GESTURE			0x00000010		// 笔势符号，36个

#define RECOG_RANGE_ADAPTATION		0x10000000		// 自学习字

// 字母符号，比如英文总共52个大小写字母
#define RECOG_RANGE_SYMBOL			(RECOG_RANGE_UPPERCASE | RECOG_RANGE_LOWERCASE)
#define RECOG_RANGE_ALL				(RECOG_RANGE_NUMBER | RECOG_RANGE_SYMBOL | RECOG_RANGE_INTERPUNCTION | RECOG_RANGE_GESTURE | RECOG_RANGE_ADAPTATION)

// Define Gesture
#define DG_GESTURE_NUM		36 
#define DG_ZERO				0x0000	// 无笔势
#define DG_SPACE			0x0020	// 空格(0x0020)
#define DG_ENTER			0x000D	// 回车(0x000D)
#define DG_BACKSPACE		0x0008	// 回删(0x0008)
#define DG_DELETE			0x001E	// 删除(0x001E)

typedef enum
{
	HWR_LANGUAGE_BELORUSSIAN,		// 白俄罗斯文
	HWR_LANGUAGE_BULGARIAN,			// 保加利亚文
	HWR_LANGUAGE_CROATIAN,			// 克罗地亚文
	HWR_LANGUAGE_CZECHISH,			// 捷克文
	HWR_LANGUAGE_DENISH,			// 丹麦文
	HWR_LANGUAGE_DUTCH,				// 荷兰文
	HWR_LANGUAGE_ENGLISH,			// 英文
	HWR_LANGUAGE_ESTONIAN,			// 爱沙尼亚文
	HWR_LANGUAGE_FINNISH,			// 芬兰文
	HWR_LANGUAGE_FRENCH,			// 法文
	HWR_LANGUAGE_GERMAN,			// 德文
	HWR_LANGUAGE_GREEK,				// 希腊文
	HWR_LANGUAGE_HUNGARIAN,			// 匈牙利文
	HWR_LANGUAGE_IRISH,				// 爱尔兰文
	HWR_LANGUAGE_ITALIAN,			// 意大利文
	HWR_LANGUAGE_LATVIAN,			// 拉脱维亚文
	HWR_LANGUAGE_LITHUANIAN,		// 立陶宛文
	HWR_LANGUAGE_LUXEMBURG,			// 卢森堡文
	HWR_LANGUAGE_MALTESE,			// 马耳他文
	HWR_LANGUAGE_NORWEGIAN,			// 挪威文
	HWR_LANGUAGE_POLISH,			// 波兰文
	HWR_LANGUAGE_PORTUGUESE,		// 葡萄牙文
	HWR_LANGUAGE_ROMANIAN,			// 罗马尼亚文
	HWR_LANGUAGE_RUSSIAN,			// 俄罗斯文
	HWR_LANGUAGE_SLOVAKIAN,			// 斯洛伐克文
	HWR_LANGUAGE_SLOVENIAN,			// 斯洛文尼亚文
	HWR_LANGUAGE_SPANISH,			// 西班牙文
	HWR_LANGUAGE_SWEDISH,			// 瑞典文
	HWR_LANGUAGE_TURKISH,			// 土耳其文
	HWR_LANGUAGE_UKRAINIAN,			// 乌克兰文

	HWR_LANGUAGE_INDONESIAN,		// 印度尼西亚文
	HWR_LANGUAGE_MALAYSIAN,			// 马来西亚文
	HWR_LANGUAGE_VIETNAMESE,		// 越南文

	HWR_LANGUAGE_AFRIKAANS,			// 南非英文
	
	HWR_LANGUAGE_ARABIC,			// 阿拉伯文
	HWR_LANGUAGE_UYGHUR				// 维吾尔文
} HWRLanguage;

typedef enum 
{
	PARAM_CANDNUMB,			// 1~100，推荐使用10，缺省是10
	PARAM_RECORANG,			// RECOG_RANGE_xxx，汉字引擎缺省是GB，多语种引擎缺省是SYMBOL
	PARAM_DISPCODE,			// DP_xxx,缺省是不变，此设置只对汉字引擎有效
	PARAM_FULLHALF,			// FH_xxx，缺省是全角，此设置只对汉字引擎有效
	PARAM_DEFGESTURE,		// 传入36维向量（36个WORD），对36个写法进行笔势定义
	PARAM_SPEEDUP,			// 可以获得更快的速度,建议不要设置此参数,缺省是不设置，此设置只对汉字引擎有效
	PARAM_LANGUAGE			// 设置当前激活语言，缺省是英文，此设置只对多语种引擎有效
} HZPARAM;

HZRECOGAPI HZ_VOID HZSetParam_Multi(HZPARAM nParam, HZ_UINT32 dwValue);
HZRECOGAPI HZ_UINT32 HZGetParam_Multi(HZPARAM nParam);

#ifdef __cplusplus
}
#endif 



