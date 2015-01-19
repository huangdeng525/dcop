/// -------------------------------------------------
/// hashtableindex.h : HASH表索引工具类公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "algo/hashtableindex.h"


CHashTableIdxTool::CHashTableIdxTool()
{
    m_pHashIndexTable = 0;
    m_dwTableLength = HASHTABLE_LENGTH_DEFAULT;
    m_fnHashFunc = 0;
    m_pHashFuncPara = 0;
    m_dwNullValue = HASHTABLE_NULLVALUE_DEFAULT;
}

CHashTableIdxTool::CHashTableIdxTool(DWORD dwHashLength, 
    FUNC_HASH_INDEX fnHashFunc, void *pHashFuncPara, DWORD dwNullValue)
{
    m_pHashIndexTable = 0;
    m_dwTableLength = (!dwHashLength)? HASHTABLE_LENGTH_DEFAULT : dwHashLength;
    m_fnHashFunc = fnHashFunc;
    m_pHashFuncPara = pHashFuncPara;
    m_dwNullValue = dwNullValue;
}

CHashTableIdxTool::~CHashTableIdxTool()
{
    if (m_pHashIndexTable)
    {
        delete [] m_pHashIndexTable;
        m_pHashIndexTable = 0;
    }

    m_dwTableLength = HASHTABLE_LENGTH_DEFAULT;
    m_fnHashFunc = 0;
    m_pHashFuncPara = 0;
    m_dwNullValue = HASHTABLE_NULLVALUE_DEFAULT;
}

void CHashTableIdxTool::vSetTableLength(DWORD dwLength)
{
    if (m_pHashIndexTable)
    {
        return;
    }

    if (!dwLength)
    {
        dwLength = m_dwTableLength;
    }

    m_pHashIndexTable = new DWORD[dwLength];
    if (!m_pHashIndexTable)
    {
        return;
    }

    m_dwTableLength = dwLength;
    for (DWORD i = 0; i < m_dwTableLength; ++i)
    {
        m_pHashIndexTable[i] = m_dwNullValue;
    }
}

DWORD CHashTableIdxTool::dwSetIDToIdx(DWORD dwID)
{
    if (!m_pHashIndexTable)
    {
        vSetTableLength();
    }

    if (dwID == m_dwNullValue)
    {
        return m_dwNullValue;
    }

    DWORD dwIDTmp = (m_fnHashFunc)? (m_fnHashFunc(dwID, m_pHashFuncPara)) : dwID;
    DWORD dwIdx = dwIDTmp % m_dwTableLength;
    DWORD dwRealIdx = dwIdx;
    /// [不等于dwID继续往前查找]
    while (m_pHashIndexTable[dwRealIdx] != dwID)
    {
        dwRealIdx++;

        if (dwRealIdx >= m_dwTableLength)
        {
            dwRealIdx = dwIDTmp % m_dwTableLength;
        }

        if (dwRealIdx == dwIdx)
        {
            /// 找了一圈，都没有空位
            dwRealIdx = m_dwNullValue;
            break;
        }
    }

    if (dwRealIdx != m_dwNullValue)
    {
        m_pHashIndexTable[dwRealIdx] = dwID;
    }

    return dwRealIdx;
}

DWORD CHashTableIdxTool::dwGetIdxByID(DWORD dwID)
{
    if (!m_pHashIndexTable || (dwID == m_dwNullValue))
    {
        return m_dwNullValue;
    }

    DWORD dwIDTmp = (m_fnHashFunc)? (m_fnHashFunc(dwID, m_pHashFuncPara)) : dwID;
    DWORD dwIdx = dwIDTmp % m_dwTableLength;
    DWORD dwRealIdx = dwIdx;
    /// [不等于dwID继续往前查找]
    while (m_pHashIndexTable[dwRealIdx] != dwID)
    {
        dwRealIdx++;

        if (dwRealIdx >= m_dwTableLength)
        {
            dwRealIdx = dwIDTmp % m_dwTableLength;
        }

        if (dwRealIdx == dwIdx)
        {
            /// 找了一圈，都没有空位
            dwRealIdx = m_dwNullValue;
            break;
        }
    }

    return dwRealIdx;
}

DWORD CHashTableIdxTool::dwGetIDByIdx(DWORD dwIdx)
{
    if (!m_pHashIndexTable || (dwIdx >= m_dwTableLength))
    {
        return m_dwNullValue;
    }

    return m_pHashIndexTable[dwIdx];
}

DWORD CHashTableIdxTool::dwClearIdxInID(DWORD dwID)
{
    DWORD dwIdx = dwGetIdxByID(dwID);
    if (dwIdx != m_dwNullValue)
    {
        m_pHashIndexTable[dwIdx] = m_dwNullValue;
    }

    return dwIdx;
}

DWORD CHashTableIdxTool::dwClearIdxInIdx(DWORD dwIdx)
{
    if (!m_pHashIndexTable || (dwIdx >= m_dwTableLength))
    {
        return m_dwNullValue;
    }

    DWORD dwID = m_pHashIndexTable[dwIdx];
    m_pHashIndexTable[dwIdx] = m_dwNullValue;

    return dwID;
}
