/// -------------------------------------------------
/// darray.cpp : 动态数组实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "array/darray.h"


/*******************************************************
  函 数 名: CDArray::CDArray
  描    述: CDArray构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDArray::CDArray()
{
    m_pBuffer = 0;
    m_dwBufMemCount = 0;
    m_dwBufMemStepCount = DARRAY_STEP_COUNT;
    m_dwNodeMaxCount = DARRAY_MAX_COUNT;
    m_dwNodeCount = 0;
    m_dwNodeSize = sizeof(DWORD);
    m_fnDelete = 0;
}

/*******************************************************
  函 数 名: CDArray::CDArray
  描    述: CDArray构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDArray::CDArray(DWORD dwNodeSize, DWORD dwMaxCount)
{
    m_pBuffer = 0;
    m_dwBufMemCount = 0;
    m_dwBufMemStepCount = DARRAY_STEP_COUNT;
    m_dwNodeMaxCount = (dwMaxCount)? dwMaxCount : DARRAY_MAX_COUNT;
    m_dwNodeCount = 0;
    m_dwNodeSize = dwNodeSize;
    m_fnDelete = 0;
}

/*******************************************************
  函 数 名: CDArray::~CDArray
  描    述: CDArray析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDArray::~CDArray()
{
    Clear();
}

/*******************************************************
  函 数 名: CDArray::Append
  描    述: 添加节点
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDArray::Append(void *pNode, DWORD dwSize)
{
    bool bRc = Dalloc(1);
    if (!bRc)
    {
        return FAILURE;
    }

    void *pBufTmp = (char *)m_pBuffer + m_dwNodeCount * m_dwNodeSize;
    (void)memset(pBufTmp, 0, m_dwNodeSize);
    DWORD dwCopySize = (!dwSize || (dwSize >= m_dwNodeSize))? m_dwNodeSize : dwSize;
    if (pNode)
    {
        (void)memcpy(pBufTmp, pNode, dwCopySize);
    }

    m_dwNodeCount++;
    return SUCCESS;
}

/*******************************************************
  函 数 名: CDArray::Pos
  描    述: 获取指定位置的节点
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void *CDArray::Pos(DWORD index) const
{
    if (!m_pBuffer || (index >= m_dwNodeCount))
    {
        return NULL;
    }

    return (char *)m_pBuffer + index * m_dwNodeSize;
}

/*******************************************************
  函 数 名: CDArray::Set
  描    述: 设置指定位置的节点
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDArray::Set(DWORD index, void *pNode, DWORD dwSize)
{
    if (!m_pBuffer || (index >= m_dwNodeCount))
    {
        bool bRc = Dalloc(index - m_dwNodeCount + 1);
        if (!bRc)
        {
            return FAILURE;
        }

        m_dwNodeCount = index - m_dwNodeCount + 1;
    }

    void *pBufTmp = (char *)m_pBuffer + index * m_dwNodeSize;
    (void)memset(pBufTmp, 0, m_dwNodeSize);
    DWORD dwCopySize = (!dwSize || (dwSize >= m_dwNodeSize))? m_dwNodeSize : dwSize;
    if (pNode)
    {
        (void)memcpy(pBufTmp, pNode, dwCopySize);
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDArray::Find
  描    述: 查找节点的位置
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDArray::Find(void *pKey, COMPARE fnCompare)
{
    DWORD dwRc = (DWORD)(-1);
    if (!m_pBuffer)
    {
        return dwRc;
    }

    for (DWORD i = 0; i < m_dwNodeCount; ++i)
    {
        bool bFound = false;
        if (fnCompare)
        {
            bFound = (*fnCompare)(Pos(i), pKey);
        }
        else
        {
            bFound = (!memcmp(Pos(i), pKey, m_dwNodeSize))? true : false;
        }

        if (bFound)
        {
            dwRc = i;
            break;
        }
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CDArray::Clear
  描    述: 秦楚
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDArray::Clear()
{
    if (!m_pBuffer)
        return;

    if (m_fnDelete)
    {
        for (DWORD i = 0; i < m_dwNodeCount; ++i)
        {
            void *pNode = Pos(i);
            if (!pNode) continue;

            (*m_fnDelete)(i, pNode);
        }
    }

    Dfree();
}

/*******************************************************
  函 数 名: CDArray::Dalloc
  描    述: 分配新的动态内存，会保留之前的数据
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CDArray::Dalloc(DWORD dwNewNodeCount)
{
    /// 已有内存且长度满足需要，直接返回成功
    if ( m_pBuffer && ((m_dwNodeCount + dwNewNodeCount) <= m_dwBufMemCount) )
    {
        return true;
    }

    /// 申请长度取递增步长和需要的最大值
    DWORD dwTmpCount = m_dwBufMemCount + m_dwBufMemStepCount;
    if (dwTmpCount < (m_dwNodeCount + dwNewNodeCount))
    {
        dwTmpCount = m_dwNodeCount + dwNewNodeCount;
    }

    /// 不能超过设定的最大长度
    if (dwTmpCount > m_dwNodeMaxCount)
    {
        if (m_dwNodeMaxCount < (m_dwNodeCount + dwNewNodeCount))
        {
            return false;
        }

        dwTmpCount = m_dwNodeMaxCount;
    }

    /// 无法申请的长度，返回失败
    if (!dwTmpCount)
    {
        return false;
    }

    /// 申请内存，失败返回
    int *pTmp = (int *)DCOP_Malloc(dwTmpCount * m_dwNodeSize);
    if (!pTmp)
    {
        return false;
    }

    /// 内存清零
    (void)memset(pTmp, 0, dwTmpCount * m_dwNodeSize);

    /// 拷贝已经存在的内存到前面
    if (m_pBuffer)
    {
        (void)memcpy(pTmp, m_pBuffer, m_dwNodeCount * m_dwNodeSize);
        DCOP_Free(m_pBuffer);
    }

    /// 输出新的内存和长度
    m_pBuffer = pTmp;
    m_dwBufMemCount = dwTmpCount;

    return true;
}

/*******************************************************
  函 数 名: CDArray::Dfree
  描    述: 释放动态内存
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDArray::Dfree()
{
    if (m_pBuffer)
    {
        DCOP_Free(m_pBuffer);
        m_pBuffer = 0;
    }

    m_dwBufMemCount = 0;
    m_dwNodeCount = 0;
}

