#ifndef __LANMODEL_H__
#define __LANMODEL_H__

#define MAX_SELECT 10					//最多备选汉字的个数
#define	jtHWR_FUNC_CODEPAGE_GBK			936		// GBK (default)
#define jtHWR_FUNC_CODEPAGE_UNICODE		1200	// Unicode (UTF-16) little endien

//节点，每个节点存储的信息
typedef struct GBNode
{
	unsigned short	nGBCode;//汉字的GB吗转成的0---6768之间的码
	unsigned short	nBestPrev;//该节点的最佳前驱（坐标从0开始）
	double	nCost;//代价，概率的乘法进行对数运算后变成代价，使用加法
}GBNode;

typedef enum IsUpdateLangModel
 {
	Update,   //更新语言模型
	NotUpate  //不更新语言模型
}IsUpdateLangModel;

typedef struct RecResultStru
{
	unsigned short nGB; //识别后的备选汉字,国标GB码或UNICODE码
	unsigned long nDis; //识别的后的备选汉字与该汉字的模版的距离
}RecResultStru;

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * 函数功能：设置词言模型接口的编码方式
 * 参数说明：
 * iParamValue[in]:指定为jtHWR_FUNC_CODEPAGE_GBK表示GBK，指定为jtHWR_FUNC_CODEPAGE_UNICODE表示Unicode
 * 返回值：  void
 */
void jtLanModel_SetParam_CodePage(long iParamValue);

/*
 * 函数功能：获取最佳识别结果接口，供手写识别进行后处理使用 
 * 参数说明：                                                
 * pRecResult：手写识别结果产生的被选序列，二维矩阵                
 * nSenLen：该句话的长度                                          
 * pContext：字与字之间的转移概率库，文件指针或内存中的地址         
 * pOut：输出结果，内存由调用开辟，该函数直接使用                    
 * bIsUpdateLangModel：是否更新语言模型，Update更新，NOTUpdate不更新
 * pNode：语言模型操作内存空间，由外部分配
 * 返回值 ： -2：加密检查失败
 *			 -1：参数检查失败
 *			 1： 成功
 * 备注：如果要更新语言模型，推出系统时，请将pContext所指上的语言空间 
 *       进行保存处理。
 */
long jtLanModel_GetBestResult( RecResultStru ** pRecResult, 
								long nSenLen, 
								void * pContext, 
								unsigned short * pOut, 
								IsUpdateLangModel bIsUpdateLangModel,
								GBNode **pNode );

#ifdef __cplusplus
}
#endif

#endif