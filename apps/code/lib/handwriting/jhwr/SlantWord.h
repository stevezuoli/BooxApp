#ifndef __SINOVOICE_SLANT_WORD_H__
#define __SINOVOICE_SLANT_WORD_H__
/************************************************************************/
/* 功能说明：倾斜纠正                                                   */
/************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif 


/*
 *	函数功能：多字倾斜校正，倾斜的笔迹点按一定的角度进行校正后输出
 *  参数:
 *  pnStrokeBufferIn[in]	:要进行校正的笔迹点集合
 *	pnStrokeBufferOut[out]	:校正过后的笔迹点集合
 * 返回值		-2:加密检查失败
 *				-1: 参数错误							
 *				0: 校正失败，输出的校正后的笔迹点为原笔迹点
 *				非0: 校正成功
 */
long jtSlantWord_Correct(
	short *pnStrokeBufferIn,
	short *pnStrokeBufferOut );
 
#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__SINOVOICE_SLANT_WORD_H__