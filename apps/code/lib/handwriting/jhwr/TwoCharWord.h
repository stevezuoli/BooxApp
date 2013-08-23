#ifndef __SINOVOICE_TWO_CHAR_WORD_H__
#define __SINOVOICE_TWO_CHAR_WORD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define	jtHWR_FUNC_CODEPAGE_GBK			936		// GBK (default)
#define jtHWR_FUNC_CODEPAGE_UNICODE		1200	// Unicode (UTF-16) little endien
void jtTwoCharWord_SetParam_CodePage(long iParamValue);

/*
 *  函数名:   jtTwoCharWord_GetHightFrequencyWord
 *  函数功能：获取高频字列表                                             
 *  参数说明：                                                           
 *  szHightFrequencyWords[out]：	输出结果，高频字列表，由用户程序开辟内存空间  
 *  nNumber[in]:		获取的字数，范围介于[1，MAX_HIGHTFREQUENCYWORD_NUMBER]之间   
 *  pContext[in]：	语言模型                                                   
 *  返回值：			-2：加密验证失败
 *						-1：传入错误的参数
 *						 0：传入的汉字不是GB2312中的汉字
 *						其它：实际获得的高频字
 */
long jtTwoCharWord_GetHightFrequencyWord(unsigned short *szHightFrequencyWords, long nNumber, const void *pContext);

/*
 *  函数名:   jtTwoCharWord_GetSuffixAssociateWord
 *  函数功能：获取字的后联想表                                           
 *  参数说明：                                                           
 *  pwSuffixAssociateWords[out]：输出结果，后联想字表，由用户程序开辟内存空间 
 *  nNumber[in]:		获取的字数                                                   
 *  wWord[in]:		汉字的GB码或者Unicode码                                        
 *  pContext[in]：	语言模型                                                   
 *  返回值：			-2：加密验证失败
 *						-1：传入错误的参数
 *						 0：传入的汉字不是GB2312中的汉字
 *						其它：实际获得联想字数，若该字无后联想字，则获得高频字，并返回实际的高频字数                                    
 */

long jtTwoCharWord_GetSuffixAssociateWord(unsigned short  *pwSuffixAssociateWords,long nNumber, unsigned short  wWord, const void *pContext);

/*
 *	函数名:       jtTwoCharWord_AdjustWordFrequency
 *	函数功能：	 调整联想词的位置
 *	参数说明:
 *	wWord[in]:	 汉字的GB码或者Unicode码
 *	iIndex[in]:	 想要移动的汉字在后联想字表中的索引
 *	iForward[in]: 向前前移动的个数
 *	pContext[in]: 语言模型
 *	返回值:	     1：移位成功
 *				 0：输入的汉字不是GB2312中的汉字或没有后联想字
 *				-1：传入错误的参数
 *				-2：加密验证失败
 *				-3：输入的索引大于最大后联想字字数
 */
long jtTwoCharWord_AdjustWordFrequency(
	unsigned short  wWord,
	long iIndex,
	long iForward,
	void *pContext);

#ifdef __cplusplus
}
#endif

#endif