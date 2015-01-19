/// -------------------------------------------------
/// network.h : 网数据结构公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_GRAPH_NETWORK_H_
#define _TOOL_GRAPH_NETWORK_H_

#include "graph.h"


/// 网
class CNetwork : public IGraph
{
public:

    /////////////////////////////////////////////////////////////////
    /// [存储说明]为了内存和索引需要，会把一些存储信息都放在数组中(可
    /// 动态变长)；但为了排序方便，同时在数组节点中增加一个下一个节点
    /// 的指针，这个指针指向的下一个是经过排序的，或者根据需要指向空。
    /////////////////////////////////////////////////////////////////

    /// 节点值从1开始编号，位置索引从0开始

    /// 关联关系(Vertex::value为下一个关系)
    class RelationShip : public Vertex
    {
    public:
        IGraph::POS curNode;                    // 本节点
        IGraph::POS connectIdx;                 // 连接索引
        IGraph::POS adjNode;                    // 邻接点
    };

    /// 连接(Vertex::value为权重)
    class Connection :  public Vertex
    {
    public:
        IGraph::POS startNode;                  // 开始节点
        IGraph::POS endNode;                    // 结束节点
        int reverseValue;                       // 反向权重(2->1的值;Vertex::value为正向权重(1->2的值))
    };

    /// 节点(Vertex::value为pos+1的节点值)
    class Node : public Vertex
    {
    public:
        IGraph::POS firstShip;                  // 第一个关联关系
    };

public:
    CNetwork();
    virtual ~CNetwork();

    /// 开始添加关联关系
    bool StartAdd(
                DWORD dwNodeMemLen,             // 节点/连接/关系的初始内存长度
                DWORD dwNodeMemLenStep,         // 节点/连接/关系的初始内存增长步长
                DWORD dwConnectionMemLen,       // 节点/连接/关系的初始内存长度
                DWORD dwConnectionMemLenStep,   // 节点/连接/关系的初始内存增长步长
                DWORD dwRelationShipMemLen,     // 节点/连接/关系的初始内存长度
                DWORD dwRelationShipMemLenStep, // 节点/连接/关系的初始内存增长步长
                bool  bConnectionIndex = false  //以连接作为索引
                );

    /// 添加关联关系
    bool Add(
                int startNodeValue,             // 开始节点
                int endNodeValue,               // 结束节点
                int forwardWeight,              // 正向权重
                int reverseWeight               // 反向权重
                );

    /// 结束添加关联关系
    void EndAdd();

    /// 获取节点的第一个邻接点
    IGraph::POS FirstAdjVexOfNode(const IGraph::POS &parentVex, const IGraph::POS &curVex, const IGraph::POS &dstVex);

    /// 获取节点的下一个邻接点
    IGraph::POS NextAdjVexOfNode(const IGraph::POS &parentVex, const IGraph::POS &curVex, const IGraph::POS &adjVex, const IGraph::POS &dstVex);

    /// 获取连接的第一个邻接点连接
    IGraph::POS FirstAdjVexOfConnection(const IGraph::POS &parentVex, const IGraph::POS &curVex, const IGraph::POS &dstVex);

    /// 获取连接的下一个邻接点连接
    IGraph::POS NextAdjVexOfConnection(const IGraph::POS &parentVex, const IGraph::POS &curVex, const IGraph::POS &adjVex, const IGraph::POS &dstVex);

    /// =============== 重写基类函数 ================
    virtual IGraph::POS FirstAdjVex(const IGraph::POS &parentVex, const IGraph::POS &curVex, const IGraph::POS &dstVex);
    virtual IGraph::POS NextAdjVex(const IGraph::POS &parentVex, const IGraph::POS &curVex, const IGraph::POS &adjVex, const IGraph::POS &dstVex);
    /// =============================================

    /// 获取关联关系
    IGraph::POS GetFirstRelationShip() {return m_posRelationShipFirst;}
    IGraph::POS GetNextRelationShip(IGraph::POS pos) {return m_pRelationShip[pos].value;}
    RelationShip *GetRelationShipNode(IGraph::POS pos) {return &(m_pRelationShip[pos]);}

    /// 获取两个连接中的相同节点
    IGraph::POS GetSameNodeOfConnect(
                IGraph::POS parentNode, 
                IGraph::POS curConnection
                );

    /// 获取连接中的另一个节点
    IGraph::POS GetThatNodeOfConnect(
                IGraph::POS curConnection,
                IGraph::POS thisNode
                );

    /// 空位置判断
    bool NullNodePos(IGraph::POS pos) {return ((pos < 0) || (pos >= (int)m_dwNodeCount))? true : false;}
    bool NullConnectionPos(IGraph::POS pos) {return ((pos < 0) || (pos >= (int)m_dwConnectionCount))? true : false;}
    bool NullRelationShipPos(IGraph::POS pos) {return ((pos < 0) || (pos >= (int)m_dwRelationShipCount))? true : false;}

    /// 调试信息
    void Dump();

private:
    /// 添加节点(返回位置)
    IGraph::POS AddNode(
                int nodeValue                   // 节点值
                );

    /// 添加连接(返回位置)
    IGraph::POS AddConnection(
                IGraph::POS startNode,          // 开始节点
                IGraph::POS endNode,            // 结束节点
                int forwardWeight,              // 正向权重
                int reverseWeight               // 反向权重
                );

    /// 添加关联(返回本节点的第一个关联位置)
    IGraph::POS AddRelationShip(
                IGraph::POS curNode,            // 本节点
                IGraph::POS adjNode,            // 邻接点
                IGraph::POS connectIdx          // 连接索引
                );

    /// 清除内存等
    void Clear();

private:
    Node *m_pNode;                              // 节点列表
    DWORD m_dwNodeMemLen;                       // 节点列表内存长度
    DWORD m_dwNodeCount;                        // 节点个数
    DWORD m_dwNodeMemLenStep;                   // 节点列表内存增长步长

    Connection *m_pConnection;                  // 连接列表
    DWORD m_dwConnectionMemLen;                 // 连接列表内存长度
    DWORD m_dwConnectionCount;                  // 连接个数
    DWORD m_dwConnectionMemLenStep;             // 连接列表内存增长步长

    RelationShip *m_pRelationShip;              // 关联列表
    DWORD m_dwRelationShipMemLen;               // 关联列表内存长度
    DWORD m_dwRelationShipCount;                // 关联个数
    DWORD m_dwRelationShipMemLenStep;           // 关联列表内存增长步长

    IGraph::POS m_posRelationShipFirst;         // 第一个关联位置(最小)
    bool m_bConnectionIndex;                    // 是否将连接作为索引

};


#endif // _TOOL_GRAPH_NETWORK_H_

