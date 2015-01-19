/// -------------------------------------------------
/// search.h : 查找算法公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_ALGO_SEARCH_H_
#define _TOOL_ALGO_SEARCH_H_

#ifdef __cplusplus
extern "C" {
#endif


/// 二分法查找的迭代版本
int binary_search(int array[], int n, int v);

/// 二分法查找的递归版本
int binary_search_recurse(int array[], int low, int high, int v);


#ifdef __cplusplus
}
#endif

#endif // #ifndef _TOOL_ALGO_SEARCH_H_

