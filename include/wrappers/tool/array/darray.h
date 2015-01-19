/// -------------------------------------------------
/// darray.h : 动态数组公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _ARRAY_DARRAY_H_
#define _ARRAY_DARRAY_H_

#include "dcop.h"


/// 动态数组默认步数
#define DARRAY_STEP_COUNT           5

/// 动态内存最大数量
#define DARRAY_MAX_COUNT            65535


/// 动态数组
class CDArray
{
public:
    /// 比较回调函数
    typedef bool (*COMPARE)(void *pNode, void *pKey);

    /// 删除回调函数
    typedef void (*DELETE)(DWORD index, void *pNode);

public:
    CDArray();
    CDArray(DWORD dwNodeSize, DWORD dwMaxCount = 0);
    ~CDArray();

    /// 设置相关属性
    void SetStepCount(DWORD dwStepCount) {m_dwBufMemStepCount = dwStepCount;}
    void SetMaxCount(DWORD dwMaxCount) {m_dwNodeMaxCount = dwMaxCount;}
    void SetNodeSize(DWORD dwNodeSize) {m_dwNodeSize = dwNodeSize;}
    void SetNodeDelete(DELETE fnDelete) {m_fnDelete = fnDelete;}

    /// 获取节点大小和节点个数
    DWORD Size() const {return m_dwNodeSize;}
    DWORD Count() const {return m_dwNodeCount;}

    /// 常用操作接口
    DWORD Append(void *pNode, DWORD dwSize = 0);
    void *Pos(DWORD index) const;
    DWORD Set(DWORD index, void *pNode, DWORD dwSize = 0);
    void *Get() const {return m_pBuffer;}
    DWORD Find(void *pNode, COMPARE fnCompare = 0);
    void  Clear();

private:
    /// 动态分配一段内存
    bool Dalloc(DWORD dwNewNodeCount);

    /// 释放动态内存
    void Dfree();

private:
    void *m_pBuffer;                            // 内存地址
    DWORD m_dwBufMemCount;                      // 内存已申请数量
    DWORD m_dwBufMemStepCount;                  // 内存动态递增步数
    DWORD m_dwNodeMaxCount;                     // 最大节点数量
    DWORD m_dwNodeCount;                        // 节点个数
    DWORD m_dwNodeSize;                         // 节点大小
    DELETE m_fnDelete;                          // 节点删除回调函数
};


#endif // #ifndef _ARRAY_DARRAY_H_

