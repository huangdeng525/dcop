/// -------------------------------------------------
/// dlistvector.cpp : 双向链表数组工具类实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "list/dlistvector.h"
#include "err.h"
#include <string.h>


CdListVectorTool::CdListVectorTool() : 
    m_pNodesBuf(0), m_dwNodeSize(0), m_dwNodeNum(0),
    m_pDelNodeProc(0), m_pDelNodeProcPara(0)
{
    vSetInvalidID(INVALID_NODEID);
    vSetInvalidValue(INVALID_NODEVALUE);
    vSetSingleIncLength(SINGLE_INC_LENGTH);
    vSetMaxArrayLength(MAX_ARRAY_LENGTH);

    DWORD dwNodeLen = dwGetNodeLength();
    if (dwNodeLen <= sizeof(ListNode))
    {
        vSetExtendNodeLength(0);
    }
    else
    {
        vSetExtendNodeLength(dwNodeLen - sizeof(ListNode));
    }

    m_pLoopTmp = 0;
    m_bDirect = TRUE;

    m_dwQueFront = 0;
    m_dwQueBack = 0;
}

CdListVectorTool::~CdListVectorTool()
{
    Clear();
}

DWORD CdListVectorTool::Add(DWORD dwIdx, void *pValue)
{
    if (dwIdx == m_dwInvalidID || 
        dwIdx > (m_dwMaxArrayLength - 1) || 
        !bValidValue(pValue))
    {
        return FAILURE;
    }

    if (!m_pNodesBuf)
    {
        /// 申请空间时，索引和步长取最大；和最大长度取最小
        m_dwNodeSize = (dwIdx > (m_dwSingleIncLength - 1))?
                (dwIdx + m_dwSingleIncLength) : m_dwSingleIncLength;
        m_dwNodeSize = (m_dwNodeSize > m_dwMaxArrayLength)?
                m_dwMaxArrayLength : m_dwNodeSize;

        m_pNodesBuf = (ListNode *) new BYTE[(m_dwNodeSize * 
                (sizeof(ListNode) + m_dwExtendNodeLength))];
        (void)::memset((void *)m_pNodesBuf, 0, 
                (m_dwNodeSize * (sizeof(ListNode) + m_dwExtendNodeLength)));

        /// 0 为链表的首
        vInitValue(&(m_pNodesBuf[0]));
        m_pNodesBuf[0].m_dwNodeID = m_dwInvalidID;
        m_pNodesBuf[0].m_pPrevNode = &(m_pNodesBuf[0]);
        m_pNodesBuf[0].m_pNextNode = &(m_pNodesBuf[0]);
    }

    if (dwIdx > (m_dwNodeSize - 1))
    {
        /// 空间不足，重新申请数组
        DWORD __len = (dwIdx + m_dwSingleIncLength);
        __len = (__len > m_dwMaxArrayLength)?
                m_dwMaxArrayLength : __len;
        ListNode *__tmp = (ListNode *) new BYTE[(__len * (sizeof(ListNode) + 
                m_dwExtendNodeLength))];
        (void)::memset((void *)__tmp, 0, 
                (__len * (sizeof(ListNode) + m_dwExtendNodeLength)));
        (void)::memcpy((void *)__tmp, (void *)m_pNodesBuf, m_dwNodeSize);
        delete [] m_pNodesBuf;
        m_pNodesBuf = __tmp;
        m_dwNodeSize = __len;
    }

    if (0 == dwIdx)
    {
        /// 0 存放的是链表的首节点, 所以单独处理
        if (m_pNodesBuf[0].m_dwNodeID == m_dwInvalidID)
        {
            vCopyNode(&(m_pNodesBuf[0]), pValue);
            m_pNodesBuf[0].m_dwNodeID = 0;
        }
        else
        {
            return FAILURE;
        }
    }
    else
    {
        if (!(m_pNodesBuf[dwIdx].m_pPrevNode) || 
            !(m_pNodesBuf[dwIdx].m_pNextNode))
        {
            /// 这个位置没被使用
            vCopyNode(&(m_pNodesBuf[dwIdx]), pValue);
            m_pNodesBuf[dwIdx].m_dwNodeID = dwIdx;
            m_pNodesBuf[dwIdx].m_pPrevNode = m_pNodesBuf[0].m_pPrevNode;
            m_pNodesBuf[dwIdx].m_pNextNode = &(m_pNodesBuf[0]);
            m_pNodesBuf[0].m_pPrevNode->m_pNextNode = &(m_pNodesBuf[dwIdx]);
            m_pNodesBuf[0].m_pPrevNode = &(m_pNodesBuf[dwIdx]);
        }
        else
        {
            return FAILURE;
        }
    }

    m_dwNodeNum++;

    return SUCCESS;
}

DWORD CdListVectorTool::Del(DWORD dwIdx)
{
    if (dwIdx == m_dwInvalidID ||
        dwIdx > (m_dwMaxArrayLength - 1) || 
        dwIdx > (m_dwNodeSize - 1))
    {
        return FAILURE;
    }

    void *pNodeValueTmp = 0;
    DWORD dwNodeIDTmp = 0;

    if (0 == dwIdx)
    {
        /// 0 存放的是链表的首节点, 所以单独处理
        if (m_pNodesBuf[0].m_dwNodeID != m_dwInvalidID)
        {
            pNodeValueTmp = m_pNodesBuf[0].m_pNodeValue;
            dwNodeIDTmp = m_pNodesBuf[0].m_dwNodeID;

            vInitValue(&(m_pNodesBuf[0]));
            m_pNodesBuf[0].m_dwNodeID = m_dwInvalidID;
        }
        else
        {
            return FAILURE;
        }
    }
    else
    {
        if ((m_pNodesBuf[dwIdx].m_pPrevNode) &&
            (m_pNodesBuf[dwIdx].m_pNextNode))
        {
            pNodeValueTmp = m_pNodesBuf[dwIdx].m_pNodeValue;
            dwNodeIDTmp = m_pNodesBuf[dwIdx].m_dwNodeID;
            vInitValue(&(m_pNodesBuf[dwIdx]));
            m_pNodesBuf[dwIdx].m_dwNodeID = m_dwInvalidID;
            m_pNodesBuf[dwIdx].m_pPrevNode->m_pNextNode = m_pNodesBuf[dwIdx].m_pNextNode;
            m_pNodesBuf[dwIdx].m_pNextNode->m_pPrevNode = m_pNodesBuf[dwIdx].m_pPrevNode;
            m_pNodesBuf[dwIdx].m_pPrevNode = 0;
            m_pNodesBuf[dwIdx].m_pNextNode = 0;
        }
        else
        {
            return FAILURE;
        }
    }

    m_dwNodeNum--;

    onDelNode(pNodeValueTmp, dwNodeIDTmp);

    return SUCCESS;
}

void *CdListVectorTool::Get(DWORD dwIdx)
{
    if (dwIdx == m_dwInvalidID ||
        dwIdx > (m_dwMaxArrayLength - 1) || 
        dwIdx > (m_dwNodeSize - 1))
    {
        return m_pInvalidValue;
    }

    if (0 == dwIdx)
    {
        /// 0 存放的是链表的首节点, 所以单独处理
        if (m_pNodesBuf[0].m_dwNodeID != m_dwInvalidID)
        {
            return pGetNode(&(m_pNodesBuf[0]));
        }
        else
        {
            return m_pInvalidValue;
        }
    }
    else
    {
        if ((m_pNodesBuf[dwIdx].m_pPrevNode) &&
            (m_pNodesBuf[dwIdx].m_pNextNode))
        {
            /// 这个位置有人使用
            return pGetNode(&(m_pNodesBuf[dwIdx]));
        }
        else
        {
            return m_pInvalidValue;
        }
    }
}

DWORD CdListVectorTool::Set(DWORD dwIdx, void *pValue)
{
    if (dwIdx == m_dwInvalidID || 
        dwIdx > (m_dwMaxArrayLength - 1) || 
        !bValidValue(pValue))
    {
        return FAILURE;
    }

    if (!bValidValue(Get(dwIdx)))
    {
        return Add(dwIdx, pValue);
    }
    else
    {
        vCopyNode(&(m_pNodesBuf[dwIdx]), pValue);
    }

    return SUCCESS;
}

DWORD CdListVectorTool::Append(void *pValue)
{
    /// ---------------------------------------
    /// 数组0元素指向链表的首尾
    /// ---------------------------------------

    if (!m_dwNodeNum)
    {
        /// 没有节点
        Add(0, pValue);
        return 0;
    }

    if (!(pGetRootNode()))
    {
        return FAILURE;
    }

    ListNode *pLastNode = pGetRootNode()->m_pPrevNode;
    DWORD dwThisIdx = pLastNode->m_dwNodeID + 1;
    Add(dwThisIdx, pValue);

    return dwThisIdx;
}

CdListVectorTool::ListNode *CdListVectorTool::First()
{
    if (!m_pNodesBuf)
    {
        return 0;
    }

    m_pLoopTmp = (m_bDirect)? m_pNodesBuf->m_pNextNode : m_pNodesBuf->m_pPrevNode;

    return m_pNodesBuf;
}

CdListVectorTool::ListNode *CdListVectorTool::Prev()
{
    if (!m_pLoopTmp ||
        m_pLoopTmp == m_pNodesBuf)
    {
        m_pLoopTmp = 0;
        return 0;
    }

    m_pLoopTmp = m_pLoopTmp->m_pPrevNode;

    return m_pLoopTmp;
}

CdListVectorTool::ListNode *CdListVectorTool::Next()
{
    if (!m_pLoopTmp ||
        m_pLoopTmp == m_pNodesBuf)
    {
        m_pLoopTmp = 0;
        return 0;
    }

    m_pLoopTmp = m_pLoopTmp->m_pPrevNode;

    return m_pLoopTmp;
}

CdListVectorTool::ListNode *CdListVectorTool::Cur()
{
    return m_pLoopTmp;
}

void CdListVectorTool::Push_back(void *pValue)
{
    if (m_dwQueBack >= (m_dwNodeSize - 1))
    {
        /// 节点已经在最后一个位置了
    }
}

void CdListVectorTool::Push_front(void *pValue)
{
    
}

void CdListVectorTool::Pop_back()
{
    
}

void CdListVectorTool::Pop_front()
{
    
}

void CdListVectorTool::Push(void *pValue)
{
    
}

void CdListVectorTool::Pop()
{
    
}

void CdListVectorTool::Clear()
{
    if (!m_pNodesBuf)
    {
        return;
    }

    for (ListNode *pNode = First(); pNode != 0; pNode = Next())
    {
        onDelNode(pNode->m_pNodeValue, pNode->m_dwNodeID);
    }

    delete [] m_pNodesBuf;
    m_pNodesBuf = 0;
}

void CdListVectorTool::onDelNode(const void *cpNodeValue, DWORD dwNodeID)
{
    if (m_pDelNodeProc)
    {
        m_pDelNodeProc(cpNodeValue, dwNodeID, m_pDelNodeProcPara);
    }
}

DWORD CdListVectorTool::dwGetNodeLength()
{
    return sizeof(ListNode);
}

void CdListVectorTool::vCopyNode(ListNode *pNode, void *pValue)
{
    if (!bValidValue(pValue))
    {
        vInitValue(pNode);
        return;
    }

    if (!m_dwExtendNodeLength)
    {
        pNode->m_pNodeValue = pValue;
    }
    else
    {
        (void)::memcpy((void *)(&(pNode->m_pNodeValue)), pValue, 
                    (m_dwExtendNodeLength + sizeof(void *)));
    }
}

void *CdListVectorTool::pGetNode(ListNode *pNode)
{
    if (!m_dwExtendNodeLength)
    {
        return pNode->m_pNodeValue;
    }
    else
    {
        return (void *)(&(pNode->m_pNodeValue));
    }
}

void CdListVectorTool::vInitValue(ListNode *pNode)
{
    if (!m_dwExtendNodeLength)
    {
        pNode->m_pNodeValue = INVALID_NODEVALUE;
    }
    else
    {
        (void)::memset((void *)(&(pNode->m_pNodeValue)), INVALID_NODEVALUE, 
                    (m_dwExtendNodeLength + sizeof(void *)));
    }
}

bool CdListVectorTool::bValidValue(void *pValue)
{
    if (!m_dwExtendNodeLength)
    {
        if (INVALID_NODEVALUE == pValue)
        {
            return false;
        }

        return true;
    }
    else
    {
        DWORD dwSum = 0;
        BYTE *pBuf = (BYTE *)pValue;
        for (DWORD i = 0; i < (m_dwExtendNodeLength + sizeof(void *)); ++i)
        {
            dwSum += pBuf[i];
        }

        if (INVALID_NODEVALUE == dwSum)
        {
            return false;
        }

        return true;
    }
}

