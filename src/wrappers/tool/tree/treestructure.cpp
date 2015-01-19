/// -------------------------------------------------
/// treestructure.cpp : 树性结构数组工具类实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "tree/treestructure.h"


CTreeStructureTool::TreeNode::TreeNode(DWORD dwID, void *pValue)
{
    m_dwXID = 0;
    m_dwYID = 0;
    m_pFatherNode = 0;
    m_pChildLists = 0;

    m_fnDelNodeProc = 0;
    m_pDelNodePara = 0;

    m_pNodeValue = pValue;
}

CTreeStructureTool::TreeNode::~TreeNode()
{
    if (m_fnDelNodeProc)
    {
        m_fnDelNodeProc(this, m_pDelNodePara);
    }

    LeftFather();
    DelAllChildren();
}

/// 实现子节点的析构处理(实际上和作为父节点的自己是同一类型)
IMPLEMENT_DLVT_DELNODE_PROC_AUTO(CTreeStructureTool::TreeNode, TreeNode)

/// 加入到指定父节点中
void CTreeStructureTool::TreeNode::JoinFather(CTreeStructureTool::TreeNode *pFatherNode)
{
    if (!pFatherNode)
    {
        return;
    }

    m_pFatherNode = pFatherNode;
    m_dwYID = pFatherNode->m_dwYID + 1;
    pFatherNode->AddChild(this);
}

/// 从指定父节点中离开
void CTreeStructureTool::TreeNode::LeftFather()
{
    CTreeStructureTool::TreeNode *pFatherNode = m_pFatherNode;

    if (!pFatherNode)
    {
        return;
    }

    pFatherNode->DelChild(this);
}

/// 加入本节点的子节点
void CTreeStructureTool::TreeNode::AddChild(CTreeStructureTool::TreeNode *pChildNode)
{
    if (!m_pChildLists)
    {
        /// 没有字节点列表创建子节点数组
        m_pChildLists = new CdListVectorTool;
        OSASSERT(m_pChildLists != 0);

        /// 注册子节点的析构处理(实际上和作为父节点的自己是同一类型)
        REG_DLVT_DELNODE_PROC_AUTO(CTreeStructureTool::TreeNode, 
            TreeNode, m_pChildLists);
    }

    pChildNode->m_dwNodeID = m_pChildLists->Append(pChildNode);
}

/// 删除本节点的子节点
void CTreeStructureTool::TreeNode::DelChild(CTreeStructureTool::TreeNode *pChildNode)
{
    if (!m_pChildLists)
    {
        return;
    }

    m_pChildLists->Del(pChildNode->m_dwNodeID);

    /// =================================================
    /// 如果子节点数组中没有节点了如何处理, 每次释放(?)
    /// =================================================
    if (!(m_pChildLists->dwGetNodeNum()))
    {
        /// 子节点数目为空
        delete m_pChildLists;
        m_pChildLists = 0;
    }
}

/// 删除本节点的所有子节点
void CTreeStructureTool::TreeNode::DelAllChildren()
{
    if (!m_pChildLists)
    {
        return;
    }

    delete m_pChildLists;
    m_pChildLists = 0;
}

CTreeStructureTool::DepthListNode::DepthListNode()
{
}

CTreeStructureTool::DepthListNode::~DepthListNode()
{
}

void CTreeStructureTool::DepthListNode::AddNode(TreeNode *pNode)
{
    DWORD dwIdx = Append(pNode);

    pNode->m_dwXID = dwIdx;
}

void CTreeStructureTool::DepthListNode::DelNode(TreeNode *pNode)
{
    (void)Del(pNode->m_dwXID);
}

CTreeStructureTool::CTreeStructureTool()
{
    m_pTreeRootNode = 0;
}

CTreeStructureTool::~CTreeStructureTool()
{
    if (m_pTreeRootNode)
    {
        delete m_pTreeRootNode;
        m_pTreeRootNode = 0;
    }
}

void CTreeStructureTool::onDelNode(
                const void *cpNodeValue, 
                DWORD dwNodeID)
{
    /// 这个节点实际上是一个数组链表指针
    DepthListNode *pDepthNode = (DepthListNode *)cpNodeValue;
    if (pDepthNode)
    {
        delete pDepthNode;
        pDepthNode = 0;
    }
}

void CTreeStructureTool::DelNodeFromDepthList(
                CTreeStructureTool::TreeNode *pNode, 
                void *pPara)
{
    CTreeStructureTool *pThis = (CTreeStructureTool *)pPara;
    OSASSERT(pThis != 0);

    DepthListNode *pDepthNode = (DepthListNode *)(pThis->Get)(pNode->m_dwYID);
    if (pDepthNode)
    {
        pDepthNode->DelNode(pNode);
    }

    /// 如果该深度层中所有子节点都被删除后如何处理(?)
}

/// 在某节点下面加入新的子节点
CTreeStructureTool::TreeNode *CTreeStructureTool::AddNode(
                CTreeStructureTool::TreeNode *pNode, 
                DWORD dwID, 
                void *pValue)
{
    CTreeStructureTool::TreeNode *pFatherNode = pNode;

    /// -------------------------------------------------
    /// 如果pFatherNode为空，新加节点作为根节点
    /// -------------------------------------------------

    if (!pFatherNode && m_pTreeRootNode)
    {
        /// 已经有根节点了
        return 0;
    }

    /// -------------------------------------------------
    /// 如果pFatherNode非空，在非根节点下面加入新节点
    /// -------------------------------------------------

    if (pFatherNode && !m_pTreeRootNode)
    {
        /// 但是还无根节点, 不能添加
        return 0;
    }

    /// 初始化新节点
    CTreeStructureTool::TreeNode *pChildNode = 
                new CTreeStructureTool::TreeNode(dwID, pValue);

    if (!pFatherNode)
    {
        /// 如果父节点为空，作为根节点
        m_pTreeRootNode = pChildNode;
    }
    else
    {
        /// 把新节点加入到父节点下面
        pChildNode->JoinFather(pFatherNode);
    }

    /// 添加到深度列表中
    DepthListNode *pDepthNode = pGetDepthListNode(pChildNode->m_dwYID);
    if (!pDepthNode)
    {
        /// 这一层还未有节点加入
        pDepthNode = new DepthListNode;

        /// 加入到深度列表中
        (void)Add(pChildNode->m_dwYID, pDepthNode);
    }

    /// 给该节点设置删除回调
    pChildNode->SetDelTreeNodeProc(DelNodeFromDepthList, this);

    /// 把该节点加入到深度列表中
    pDepthNode->AddNode(pChildNode);
    

    return pChildNode;

}

/// 删除某节点
void CTreeStructureTool::DelNode(CTreeStructureTool::TreeNode *pNode)
{
    if (!m_pTreeRootNode)
    {
        /// 现在无节点
        return;
    }

    /// -------------------------------------------------
    /// 如果pFatherNode为空，新加节点作为根节点
    /// -------------------------------------------------

    /// -------------------------------------------------
    /// 因为节点自动注册了子节点的析构函数，所以所有树下
    /// 的节点都会遍历析构的
    /// -------------------------------------------------

    if (!pNode)
    {
        /// 清空深度列表
        Clear();

        /// 删除根节点
        delete m_pTreeRootNode;
        m_pTreeRootNode = 0;

        return;
    }

    /// 深度列表的删除已经在节点中设置了删除回调，析构节点时会自行删除的

    delete pNode;
    pNode = 0;

}

CTreeStructureTool::DepthListNode *CTreeStructureTool::pGetDepthListNode(DWORD dwDepthValue)
{
    return (DepthListNode *)Get(dwDepthValue);
}

