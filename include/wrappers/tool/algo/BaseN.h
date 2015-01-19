/// -------------------------------------------------
/// BaseN.h : 10进制数和N进制数之间的编码换算头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_ALGO_BASEN_H_
#define _TOOL_ALGO_BASEN_H_

#ifdef __cplusplus
extern "C" {
#endif


/// 10进制转换为N进制
int DCOP_BaseN(unsigned int N, unsigned int num, unsigned int *array, unsigned int len, unsigned int *high);

/// N进制转换为10进制
int DCOP_Base10(unsigned int N, unsigned int *array, unsigned int len, unsigned int *num);


#ifdef __cplusplus
}
#endif

#endif // #ifndef _TOOL_ALGO_BASEN_H_

