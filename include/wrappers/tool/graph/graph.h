/// -------------------------------------------------
/// graph.h : 图数据结构公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_GRAPH_GRAPH_H_
#define _TOOL_GRAPH_GRAPH_H_

#include "dcop.h"


/// 图
class IGraph
{
public:

    /// 顶点位置
    typedef int POS;

    /// 顶点位置无效值
    static const POS Null = -1;

    /// 顶点类型
    class Vertex
    {
    public:
        int value;                              // 顶点值
    };

public:
    IGraph();
    virtual ~IGraph();

    /////////////////////////////////////////////////////////////////
    /// 初始化操作
    /////////////////////////////////////////////////////////////////
    /// 初始化顶点
    void Init(
                DWORD dwCount,                  // 顶点个数
                int *ipValue = 0,               // 顶点值的初始化列表
                DWORD dwSize = sizeof(int)      // 顶点值的初始化列表长度
                );
    /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    /// 图和顶点位置关联的相关操作
    /////////////////////////////////////////////////////////////////
    /// 是否是无效位置(返回true:无效位置;false:有效位置)
    bool NullPos(
                const POS &pos                  // 顶点位置
                );

    /// 获取顶点数量
    DWORD VexNum();
    /////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////
    /// 顶点自己的相关操作
    /////////////////////////////////////////////////////////////////
    /// 设置顶点值
    void Set(
                const POS &pos,                 // 顶点位置
                int value                       // 顶点值
                );

    /// 获取顶点值
    int Get(
                const POS &pos                  // 顶点位置
                );

    /////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////
    /// 邻接点相关操作
    /////////////////////////////////////////////////////////////////
    /// 获取本顶点的第一个邻接点(子顶点)(返回邻接点位置)
    virtual POS FirstAdjVex(
                const POS &parentVex,           // 父顶点(包括边界进入点)
                const POS &curVex,              // 本顶点
                const POS &dstVex               // 目的顶点
                ) = 0;

    /// 获取下一个邻接点(子顶点)(返回邻接点位置)
    virtual POS NextAdjVex(
                const POS &parentVex,           // 父顶点(包括边界进入点)
                const POS &curVex,              // 本顶点
                const POS &adjVex,              // 当前邻接点
                const POS &dstVex               // 目的顶点
                ) = 0;
    /////////////////////////////////////////////////////////////////


private:
    Vertex     *m_vexs;                         // 顶点列表
    DWORD       m_size;                         // 单个顶点大小
    DWORD       m_count;                        // 顶点数量
};


/// 搜索工具类
class CGraphSearchTool
{
public:

    /// 搜索队列顶点
    typedef struct tagQueNode
    {
        int parent;                             // 父顶点索引
        int child;                              // 本顶点索引(相对父顶点来说是子顶点)
        IGraph::POS childPos;                   // 本顶点位置(相对父顶点来说是子顶点)
    }QueNode;

    /// 目的顶点判断回调
    typedef bool (*IS_DEST_FUNC)(
                IGraph::POS curVex,             // 当前顶点
                IGraph::POS dstVex,             // 目标顶点
                bool &bContinue,                // 找到后是否继续寻找
                CGraphSearchTool *pThis         // 本对象
                );

public:
    CGraphSearchTool();
    ~CGraphSearchTool();


    /////////////////////////////////////////////////////////////////
    /// 图对象的关联相关接口
    /////////////////////////////////////////////////////////////////
    /// 设置图对象
    void SetGraph(
                IGraph *pGraph                  // 图对象
                );

    /// 获取图对象
    IGraph *GetGraph() { return m_graph; }

    /// 获取图的顶点数量
    DWORD GetCount() { return m_count; }
    /////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////
    /// 搜索前的初始化操作
    /////////////////////////////////////////////////////////////////
    /// 初始化访问列表(当需要访问缓存时初始化使用)
    bool InitVisited();

    /// 初始化队列(当需要队列时初始化使用)
    bool InitQueue();

    /// 初始化栈(当需要队列时初始化使用，队列和栈为选择其中一种)
    bool InitStack();

    /// 初始化索引缓存(当需要索引缓存时使用，栈方式往往不会使用索引缓存)
    bool InitIdxBuffer(
                DWORD MemLen,                   // 缓存内存长度
                DWORD dwMemLenStep              // 缓存内存增长步长
                );
    /////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////
    /// 队列操作
    /////////////////////////////////////////////////////////////////
    /// 入队列
    void EnQueue(
                IGraph::POS adjVex,             // 顶点
                int curIdx                      // 当前索引
                );

    /// 出队列
    void DeQueue(
                IGraph::POS &adjVex,            // 顶点(输出)
                int &bufIdx);                   // 当时缓存的索引(输出)
    /////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////
    /// 栈操作
    /////////////////////////////////////////////////////////////////
    /// 入栈
    void PushStack(
                IGraph::POS adjVex,             // 顶点
                int curIdx                      // 当前索引
                );

    /// 出栈
    void PopStack(
                IGraph::POS &adjVex,            // 顶点(输出)
                int &bufIdx                     // 当时缓存的索引(输出)
                );
    /////////////////////////////////////////////////////////////////

    /// 队列是否为空
    bool QueueEmpty() { return (m_front == m_rear)? true : false; }

    /// 获取队列中顶点
    QueNode *GetQueNode(DWORD index)
    {
        if (index < m_length)
            return &(m_queue[index]);

        return 0;
    }

    /// 获取队列中顶点的父顶点
    QueNode *GetParentQueNode(DWORD index)
    {
        if (index < m_length)
            if ((m_queue[index].parent >= 0) && (m_queue[index].parent < (int)m_length))
                return &(m_queue[m_queue[index].parent]);

        return 0;
    }

    /// 获取当前队列尾部索引
    DWORD GetQueRear() { return m_rear; }

    /// 设置访问状态
    void SetVisited(DWORD index, bool state)
    {
        if (index >= m_count)
            return;

        if (state == m_visited[index])
            return;

        m_visited[index] = state;
        if (state)
            m_visitedCount++;
        else
            m_visitedCount--;
    }

    /// 获取访问状态
    bool GetVisited(DWORD index)
    {
        if (index < m_count)
            return m_visited[index];

        return false;
    }

    /// 是否无效的位置
    bool NullPos(DWORD index) { return (index < m_count)? false : true; }

    /// 获取已访问的顶点数量
    DWORD GetVisitedCount() { return m_visitedCount; }

    /// 获取索引缓存
    int *GetIdxBuffer() { return m_pIdxBuffer; }

    /// 获取索引缓存数量
    DWORD GetIdxBufferCount() { return m_dwIdxBufferCount; }

    /// 设置目的判断回调函数
    void SetIsDestFunc(IS_DEST_FUNC fnIsDest) { m_fnIsDest = fnIsDest; }

    /// 是否是目的判断
    bool IsDest(IGraph::POS curVex, int curIdx, IGraph::POS dstVex, bool &bContinue, bool bBufIdx = true);

    /// 获取目的路径
    bool GetDestPath(IGraph::POS *&rpPath, DWORD &rdwPathLen, int iBufIdx = -1);

    /// 搜索完的清理工作
    void Clear();

    /// 动态分配一段内存
    static bool Dalloc(
                void *&rpBuf,                   // 输出内存(输入已分配内存)
                DWORD &rdwBufMemLen,            // 已分配内存长度
                DWORD dwUsedMemLen,             // 已使用内存长度
                DWORD dwNeedMemLen,             // 下次写需要的内存长度
                DWORD dwBufMemLenStep,          // 本次内存增长步长
                DWORD dwBufMemSize              // 内存申请的单位大小
                );

    /// 释放动态内存
    static void Dfree(
                void *&rpBuf,                   // 输出释放后的指针(输入已分配内存)
                DWORD &rdwBufMemLen             // 内存长度
                );

private:
    IGraph *m_graph;                            // 图
    DWORD m_count;                              // 顶点数量

    bool *m_visited;                            // 已访问过
    DWORD m_visitedCount;                       // 已访问过的顶点数量

    QueNode *m_queue;                           // 辅助队列
    DWORD m_length;                             // 队列长度
    DWORD m_front;                              // 队列首
    DWORD m_rear;                               // 队列尾

    int *m_pIdxBuffer;                          // 辅助队列中的顶点索引缓存
    DWORD m_dwIdxBufferCount;                   // 辅助队列中的顶点索引缓存个数
    DWORD m_dwIdxBufferMemLen;                  // 辅助队列中的顶点索引缓存内存长度
    DWORD m_dwIdxBufferMemLenStep;              // 辅助队列中的顶点索引缓存内存增长步长

    IS_DEST_FUNC m_fnIsDest;                    // 目的判断函数

};

/// 图中路径计数增长步长
#define GRAPH_PATH_COUNT_STEP 10

/// 获取最小值和最大值
#define GRAPH_GET_MIN(x, y)     (((x) < (y))? (x) : (y))
#define GRAPH_GET_MAX(x, y)     (((x) > (y))? (x) : (y))


#endif // #ifndef _TOOL_GRAPH_GRAPH_H_

