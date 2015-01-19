/// -------------------------------------------------
/// hashtableindex.h : HASH表索引工具类公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _HASHTABLEINDEX_H_
#define _HASHTABLEINDEX_H_

#include "type.h"


/// 默认HASH表的长度
const DWORD HASHTABLE_LENGTH_DEFAULT = 256;

/// 默认空值
const DWORD HASHTABLE_NULLVALUE_DEFAULT = 0xffffffffL;


/// HASH表索引工具类
class CHashTableIdxTool
{
public:
    /// 自定义HASH算法回调函数原型
    typedef DWORD (*FUNC_HASH_INDEX)(
        DWORD dwID,
        void *pPara
        );

private:
    DWORD *m_pHashIndexTable;               // HASH表地址
    DWORD m_dwTableLength;                  // HASH表长度
    FUNC_HASH_INDEX m_fnHashFunc;           // HASH算法回调函数
    void *m_pHashFuncPara;                  // HASH算法回调函数参数
    DWORD m_dwNullValue;                    // 空值

public:
    CHashTableIdxTool();
    CHashTableIdxTool(DWORD dwHashLength, 
        FUNC_HASH_INDEX fnHashFunc,
        void *pHashFuncPara,
        DWORD dwNullValue);
    ~CHashTableIdxTool();

    void vSetTableLength(DWORD dwLength = 0);

    DWORD dwSetIDToIdx(DWORD dwID);
    DWORD dwGetIdxByID(DWORD dwID);
    DWORD dwGetIDByIdx(DWORD dwIdx);
    DWORD dwClearIdxInID(DWORD dwID);
    DWORD dwClearIdxInIdx(DWORD dwIdx);
};


#endif // #ifndef _HASHTABLEINDEX_H_
