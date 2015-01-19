/// -------------------------------------------------
/// BaseN.c : 查找算法实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include <memory.h>
#include <math.h>


/*******************************************************
  函 数 名: DCOP_BaseN
  描    述: 10进制转换为N进制
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
int DCOP_BaseN(unsigned int N, unsigned int num, unsigned int *array, unsigned int len, unsigned int *high)
{
    unsigned int i = 0;
    unsigned int tmp = num;

    if (!N || !array || !len) return -1;

    (void)memset(array, 0, sizeof(unsigned int) * len);

    while (tmp > 0)
    {
        array[i++] = (tmp % N);
        tmp /= N;
        if (i >= len) break;
    }

    if (high) *high = i;

    return 0;
}

/*******************************************************
  函 数 名: DCOP_BaseN
  描    述: 10进制转换为N进制
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
int DCOP_Base10(unsigned int N, unsigned int *array, unsigned int len, unsigned int *num)
{
    unsigned int i = 0;
    unsigned int tmp = 0;

    if (!N || !array || !len || !num) return -1;

    for (i = 0; i < len; ++i)
    {
        tmp += array[i] * (int)pow(N, i);
    }

    *num = tmp;
    return 0;
}

