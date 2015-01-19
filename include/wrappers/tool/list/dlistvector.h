/// -------------------------------------------------
/// dlistvector.h : 双向链表数组工具类公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_LIST_DLISTVECTOR_H_
#define _TOOL_LIST_DLISTVECTOR_H_

#include "type.h"


#define INVALID_NODEID      0xFFFFFFFFL         // 不可利用的节点ID
#define INVALID_NODEVALUE   0                   // 不可利用的节点值
#define SINGLE_INC_LENGTH   256                 // 单次递增的长度
#define MAX_ARRAY_LENGTH    0xFFFFFFFFL         // 最大数组长度(全8F为不限制)


/// =================================================
///     设  计  说  明
/// -------------------------------------------------
/// 1. 该数组链表的结构:
/// |value|point|
/// +-----+-----+
/// |     |     |
/// | Val | Ptr |--+
/// |     |     |  |
/// +-----+-----+  |
/// |     |     |  |
/// | Val | Ptr |--+
/// |     |     |  |
/// +-----+-----+  |
/// |     |     |  |
/// | Val | Ptr |  |
/// |     |     |  |
/// +-----+-----+  |
/// |     |     |  |
/// | Val | Ptr |--+
/// |     |     |
/// +-----+-----+
/// -------------------------------------------------
/// 2. 链表是个双向链表，数组0元素指向链表的首尾
/// -------------------------------------------------
/// 3. 数组的节点实际上是个指针，如果需要删除处理的话，
///    可以重载删除函数或者设置一个删除过程回调函数
/// -------------------------------------------------
/// 4. 如果用户可以继承，使节点长度超过原始大小，这样
///    在默认的CopyNode函数中是拷贝数据到指针位置，即
///    指针m_pNodeValue就不存在了，而是整个数据长度开
///    始的位置(用户也可以继承重写CopyNode，改变之)
/// =================================================


/// =================================================
///     自动为类加上回调函数的宏
/// -------------------------------------------------
///     申  明: DECLARE_DLVT_DELNODE_PROC_AUTO(c);
/// -------------------------------------------------
#define DECLARE_DLVT_DELNODE_PROC_AUTO(UserClass) \
    static void ___##UserClass##___DelNodeProc( \
        const void *cpNodeValue, \
        DWORD dwNodeID, \
        void *pProcPara)
/// -------------------------------------------------
///     实  现: IMPLEMENT_DLVT_DELNODE_PROC_AUTO(t,c)
/// -------------------------------------------------
#define IMPLEMENT_DLVT_DELNODE_PROC_AUTO(TypeClass, UserClass) \
    void TypeClass::___##UserClass##___DelNodeProc( \
        const void *cpNodeValue, \
        DWORD dwNodeID, \
        void *pProcPara) \
    { \
        (*(TypeClass *)cpNodeValue).~UserClass(); \
    }
/// -------------------------------------------------
///     注  册: REG_DLVT_DELNODE_PROC_AUTO(t,c,o);
/// -------------------------------------------------
#define REG_DLVT_DELNODE_PROC_AUTO(TypeClass, UserClass, UserObjPtr) \
    UserObjPtr->vSetDelNodeProc( \
        TypeClass::___##UserClass##___DelNodeProc, \
        this)
/// =================================================


class CdListVectorTool
{
public:
    /// ID(保存编号)
    struct UserNode
    {
        DWORD m_dwNodeID;
    };

    /// 节点删除过程(回调函数类型定义)
    typedef void (*DEL_NODE_PROC)(
        const void *cpNodeValue,
        DWORD dwNodeID,
        void *pProcPara
        );

    /// 数组链表节点类型
    struct ListNode : UserNode
    {
        ListNode *m_pPrevNode;
        ListNode *m_pNextNode;
        void *m_pNodeValue;                 // 节点值(可能占用该位置并扩展更大内存)
    };

protected:
    ListNode *m_pNodesBuf;                  // 节点列表指针缓冲区
    DWORD m_dwNodeSize;                     // 节点缓冲区的个数值
    DWORD m_dwNodeNum;                      // 节点数目
    DEL_NODE_PROC m_pDelNodeProc;           // 节点删除过程
    void *m_pDelNodeProcPara;               // 节点删除过程用户参数

    DWORD m_dwInvalidID;                    // [继承可改]不可利用的节点ID
    void* m_pInvalidValue;                  // [继承可改]不可利用的节点值
    DWORD m_dwSingleIncLength;              // [继承可改]单次递增的长度
    DWORD m_dwMaxArrayLength;               // [继承可改]最大数组长度(全8F为不限制)
    DWORD m_dwExtendNodeLength;             // [继承可改]扩展节点长度(如果节点不是指针,作为连续空间的话,长度又超过4个字节时)

    ListNode *m_pLoopTmp;                   // 遍历缓冲区
    BOOL m_bDirect;                         // 遍历方向(TRUE为正向,FALSE为反向)

    DWORD m_dwQueFront;                     // 队列头
    DWORD m_dwQueBack;                      // 队列尾

public:
    CdListVectorTool();
    virtual ~CdListVectorTool();

    /// 删除节点事件(调用的是回调;可继承重写)
    virtual void onDelNode(const void *cpNodeValue, DWORD dwNodeID);

    /// 获取节点长度(可继承重写)
    virtual DWORD dwGetNodeLength();

    /// 拷贝节点(可继承重写)
    virtual void vCopyNode(ListNode *pNode, void *pValue);

    /// 获取节点(可继承重写)
    virtual void *pGetNode(ListNode *pNode);

    /// 初始化Value(可继承重写)
    virtual void vInitValue(ListNode *pNode);

    /// 是否可用的Value(可继承重写)
    virtual bool bValidValue(void *pValue);

    /// =================================================
    ///     普通数组操作
    /// -------------------------------------------------
    DWORD Add(DWORD dwIdx, void *pValue);
    DWORD Del(DWORD dwIdx);
    void *Get(DWORD dwIdx);
    DWORD Set(DWORD dwIdx, void *pValue);
    DWORD Append(void *pValue);
    /// =================================================

    /// =================================================
    ///     链表遍历操作(内有一个遍历tmp, 防止重复遍历)
    /// -------------------------------------------------
    ListNode *First();
    ListNode *Prev();
    ListNode *Next();
    ListNode *Cur();
    /// =================================================

    /// =================================================
    ///     队列方式操作(内有队列首位元素标识tmp)
    /// -------------------------------------------------
    void Push_back(void *pValue);           // 在数组最后一个元素后添加
    void Push_front(void *pValue);          // 在数组的第一个元素前添加
    void Pop_back();                        // 删除数组的最后一个节点
    void Pop_front();                       // 删除数组的第一个节点
    void Push(void *pValue);                // 和Push_back一样
    void Pop();                             // 和Pop_front一样
    /// =================================================

    /// 全部清空
    void Clear();

    void    SetLoopDirect(BOOL bDirect) {m_bDirect = bDirect;}
    BOOL    GetLoopDirect() {return m_bDirect;}

    ListNode *pGetRootNode() const {return m_pNodesBuf;}
    DWORD   dwGetNodeSize() const {return m_dwNodeSize;}
    DWORD   dwGetNodeNum() const {return m_dwNodeNum;}

    void vSetDelNodeProc(DEL_NODE_PROC pProc, void *pProcPara)
    {
        m_pDelNodeProc = pProc; m_pDelNodeProcPara = pProcPara;
    }
    void vGetDelNodeProc(DEL_NODE_PROC& rpProc, void *& rpProcPara)
    {
        rpProc = m_pDelNodeProc; rpProcPara = m_pDelNodeProcPara;
    }

    DWORD   dwGetInvalidID() const {return m_dwInvalidID;}
    void*   pGetInvalidValue() const {return m_pInvalidValue;}
    DWORD   dwGetSingleIncLength() const {return m_dwSingleIncLength;}
    DWORD   dwGetMaxArrayLength() const {return m_dwMaxArrayLength;}
    DWORD   dwGetExtendNodeLength() const {return m_dwExtendNodeLength;}

    void    vSetInvalidID(DWORD dwInvalidID) {m_dwInvalidID = dwInvalidID;}
    void    vSetInvalidValue(void *pInvalidValue) {m_pInvalidValue = pInvalidValue;}
    void    vSetSingleIncLength(DWORD dwSingleIncLength) {m_dwSingleIncLength = dwSingleIncLength;}
    void    vSetMaxArrayLength(DWORD dwMaxArrayLength) {m_dwMaxArrayLength = dwMaxArrayLength;}
    void    vSetExtendNodeLength(DWORD dwExtendNodeLength) {m_dwExtendNodeLength = dwExtendNodeLength;}

};


#endif // #ifndef _TOOL_LIST_DLISTVECTOR_H_
