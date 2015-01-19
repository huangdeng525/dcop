/// -------------------------------------------------
/// treestructure.h : 树性结构数组工具类公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TREESTRUCTURE_H_
#define _TREESTRUCTURE_H_

#include "dcop.h"
#include "list/dlistvector.h"

/// =================================================
///     树性结构定义说明
/// -------------------------------------------------
/// 深度列表        树状结构
/// +------+
/// |  0   | -------- Root
/// |      |           |
/// |      |        +--+--+
/// |      |        |     |
/// |  1   | ---- Child Child
/// |      |        |     |
/// |      |     +--+  +--+--+
/// |      |     |     |     |
/// |  2   | - Child Child Child
/// |      |
/// |      |
/// |  .   |    ...   ...   ...
/// +------+
/// =================================================

/// -------------------------------------------------
/// 继承CdListVectorTool是将父类作为深度列表
/// -------------------------------------------------

class CTreeStructureTool : private CdListVectorTool
{
public:

    /// ID(保存编号)
    struct UserNode
    {
        DWORD m_dwNodeID;
    };

    /// 树节点(内部控制类型)
    struct TreeNode : public UserNode
    {
        DWORD m_dwXID;                      // 在同一个深度中的列表ID
        DWORD m_dwYID;                      // 深度ID(DepthValue Idx)
        TreeNode *m_pFatherNode;            // 父节点
        CdListVectorTool *m_pChildLists;    // 子节点列表
        void *m_pNodeValue;                 // 节点值(可能占用该位置并扩展更大内存)

        TreeNode(DWORD dwID = 0, void *pValue = 0);
        ~TreeNode();

        /// 加入到父节点
        void JoinFather(TreeNode *pFatherNode);

        /// 从父节点离开
        void LeftFather();

        /// 加入子节点
        void AddChild(TreeNode *pChildNode);

        /// 删除子节点
        void DelChild(TreeNode *pChildNode);

        /// 删除所有子节点
        void DelAllChildren();

        /// 申明子节点的析构处理(实际上和作为父节点的自己是同一类型)
        DECLARE_DLVT_DELNODE_PROC_AUTO(TreeNode);

        /// 树节点删除回调函数(Node::~Node中调用)
        typedef void (*DEL_TREENODE_PROC)(
            TreeNode *pNode,
            void *pPara
        );

        DEL_TREENODE_PROC m_fnDelNodeProc;
        void *m_pDelNodePara;

        void SetDelTreeNodeProc(
            DEL_TREENODE_PROC fnDelNodeProc,
            void *pProcPara)
        {m_fnDelNodeProc = fnDelNodeProc; m_pDelNodePara = pProcPara;}

    };

    /// 树深度列表节点
    struct DepthListNode : public CdListVectorTool
    {
        DepthListNode();
        ~DepthListNode();

        void AddNode(TreeNode *pNode);
        void DelNode(TreeNode *pNode);

    };

private:
    TreeNode *m_pTreeRootNode;

public:
    CTreeStructureTool();
    ~CTreeStructureTool();

    //////////////////////////////////////////////
    /// 重写深度列表 - begin
    //////////////////////////////////////////////

    /// 删除操作(因为节点是new出来的空间)
    void onDelNode(const void *cpNodeValue, DWORD dwNodeID);

    //////////////////////////////////////////////
    /// 重载深度列表 - end
    //////////////////////////////////////////////

    static void DelNodeFromDepthList(TreeNode *pNode, void *pPara);

    /// 获取整个树的根(返回内部控制节点内型)
    TreeNode *pGetRootNode() const {return m_pTreeRootNode;}

    TreeNode * AddNode(TreeNode *pNode, DWORD dwID, void *pValue);
    void DelNode(TreeNode *pNode);

    DepthListNode *pGetDepthListNode(DWORD dwDepthValue);

};


#endif // #ifndef _TREESTRUCTURE_H_

