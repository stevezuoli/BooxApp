#ifndef __SINOVOICE_SPLIT_CHAR_H__
#define __SINOVOICE_SPLIT_CHAR_H__
/************************************************************************/
/*	功能说明：字符切割                                                  */
/************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif //__CPLUSPLUS



/*
 *	jtSplitCharRect	
 *	矩形框类型定义
 */
typedef struct 
{
	short left;			//左侧
    short top;			//上侧
    short right;		//右侧
    short bottom;		//下侧
}jtSplitCharRect;


/*
 *	jtSplitCharBox
 *  切割存储数据结构
 */
typedef struct 
{
	jtSplitCharRect CB_bound;			//笔画矩形框
	unsigned char IsMerged;				//是否合并
	long  nPos;					//切割位置
	long  nstrIdxFirst;					//开始笔画数
	long  nstrIdxLast;					//结束笔画数
	long  nstrNum;						//每个划分的点的个数
}jtSplitCharBox;

/* 
 * 功  能: 将手写点阵数组划分成若干个独立的字
 * 参  数: 
 * pPointArray[in]	:存储手写点阵数组的缓冲区
 * iPointSize[in]	:手写点阵缓冲区的长度(点的个数)
 * pSlitArray[out]	:手写点阵分割的指针, 用来存储划分结果 内存由外部开劈
 * iSplitSize[out]	:划分的个数
 * 返回值			:-2 : 没有通过加密检查
 *					:-1	: 传入的参数错误
 *					:0	:没有进行切割
 *					: >0:返回这段点阵数组切割好的字数
 *
 * 示例:
 * short pPointArray[iPointSize * 2]; 
 * Charbox pSplitArray[iSplitSize]; 
 * SplitPointArray(pPointArray, iPointSize, pSplitArray,iSplitSize);
 */
long jtSplitChar_SplitPointArray(
	short* pPointArray, 
	long iPointSize, 
	jtSplitCharBox* pSplitArray,
	long iSplitSize); 


#ifdef __cplusplus
};
#endif //__CPLUSPLUS
#endif //__SINOVOICE_SPLIT_CHAR_H__