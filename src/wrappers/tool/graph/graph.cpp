/// -------------------------------------------------
/// graph.cpp : 图数据结构实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "graph/graph.h"
#include <stdlib.h>
#include <memory.h>


/*******************************************************
  函 数 名: IGraph::IGraph
  描    述: IGraph构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IGraph::IGraph()
{
    m_vexs = 0;
    m_size = sizeof(Vertex);
    m_count = 0;
}

/*******************************************************
  函 数 名: IGraph::~IGraph
  描    述: IGraph析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IGraph::~IGraph()
{
    if (m_vexs)
    {
        free(m_vexs);
    }
}

/*******************************************************
  函 数 名: IGraph::Init
  描    述: 初始化
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void IGraph::Init(DWORD dwCount, int *ipValue, DWORD dwSize)
{
    m_vexs = (Vertex *)malloc(dwCount * dwSize);
    if (!m_vexs)
    {
        return;
    }

    m_count = dwCount;

    if (!ipValue)
    {
        (void)memset(m_vexs, 0, dwCount * dwSize);
        return;
    }

    for (DWORD i = 0; i < dwCount; ++i)
    {
        Vertex *pVex = (Vertex *)((char *)m_vexs + i * dwSize);
        if (pVex) pVex->value = ipValue[i];
    }

}

/*******************************************************
  函 数 名: IGraph::NullPos
  描    述: 是否无效位置
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool IGraph::NullPos(const POS &pos)
{
    if ((pos <= Null) || (pos >= (int)m_count))
    {
        return true;
    }

    return false;
}

/*******************************************************
  函 数 名: IGraph::VexNum
  描    述: 节点数量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD IGraph::VexNum()
{
    return m_count;
}

/*******************************************************
  函 数 名: IGraph::Set
  描    述: 设置节点对应的值
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void IGraph::Set(const POS &pos, int value)
{
    if (NullPos(pos))
    {
        return;
    }

    m_vexs[pos].value = value;
}

/*******************************************************
  函 数 名: IGraph::Get
  描    述: 获取节点对应的值
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
int IGraph::Get(const POS &pos)
{
    if (NullPos(pos))
    {
        return -1;
    }

    return m_vexs[pos].value;
}

/*******************************************************
  函 数 名: CGraphSearchTool::CGraphSearchTool
  描    述: CGraphSearchTool构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CGraphSearchTool::CGraphSearchTool()
{
    m_graph = 0;
    m_count = 0;

    m_visited = 0;
    m_visitedCount = 0;

    m_queue = 0;
    m_length = 0;
    m_front = 0;
    m_rear = 0;

    m_pIdxBuffer = 0;
    m_dwIdxBufferMemLen = GRAPH_PATH_COUNT_STEP;
    m_dwIdxBufferMemLenStep = GRAPH_PATH_COUNT_STEP;
    m_dwIdxBufferCount = 0;

    m_fnIsDest = 0;
}

/*******************************************************
  函 数 名: CGraphSearchTool::~CGraphSearchTool
  描    述: CGraphSearchTool析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CGraphSearchTool::~CGraphSearchTool()
{
    Clear();

    m_graph = 0;
    m_count = 0;
}

/*******************************************************
  函 数 名: CGraphSearchTool::SetGraph
  描    述: 设置图对象
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CGraphSearchTool::SetGraph(IGraph *pGraph)
{
    m_graph = pGraph;
    if (m_graph)
    {
        m_count = m_graph->VexNum();
    }
}

/*******************************************************
  函 数 名: CGraphSearchTool::InitVisited
  描    述: 初始化参观列表
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CGraphSearchTool::InitVisited()
{
    if (!m_count)
    {
        return false;
    }

    if (m_visited)
    {
        free(m_visited);
    }

    m_visited = (bool *)malloc(m_count * sizeof(bool));
    if (!m_visited)
    {
        return false;
    }

    for (DWORD i = 0; i < m_count; ++i)
    {
        m_visited[i] = false;
    }

    m_visitedCount = 0;
    return true;
}

/*******************************************************
  函 数 名: CGraphSearchTool::InitQueue
  描    述: 初始化队列
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CGraphSearchTool::InitQueue()
{
    if (!m_count)
    {
        return false;
    }

    if (m_queue)
    {
        free(m_queue);
    }

    m_queue = (QueNode *)malloc(m_count * sizeof(QueNode));
    if (!m_queue)
    {
        return false;
    }

    m_length = m_count;
    m_front = 0;
    m_rear = 0;

    return true;
}

/*******************************************************
  函 数 名: CGraphSearchTool::InitStack
  描    述: 初始化栈
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CGraphSearchTool::InitStack()
{
    return InitQueue();
}

/*******************************************************
  函 数 名: CGraphSearchTool::InitIdxBuffer
  描    述: 初始化索引缓存
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CGraphSearchTool::InitIdxBuffer(DWORD MemLen, DWORD dwMemLenStep)
{
    m_dwIdxBufferMemLen = (MemLen)? MemLen : GRAPH_PATH_COUNT_STEP;
    m_dwIdxBufferMemLenStep = (dwMemLenStep)? dwMemLenStep : GRAPH_PATH_COUNT_STEP;

    if (m_pIdxBuffer)
    {
        free(m_pIdxBuffer);
    }

    m_pIdxBuffer = (int *)malloc(m_dwIdxBufferMemLen * sizeof(int));
    if (!m_pIdxBuffer)
    {
        return false;
    }

    m_dwIdxBufferCount = 0;

    return true;
}

/*******************************************************
  函 数 名: CGraphSearchTool::EnQueue
  描    述: 把节点放入队列
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CGraphSearchTool::EnQueue(IGraph::POS adjVex, int curIdx)
{
    /// 队列不能为空
    if (!m_queue)
    {
        return;
    }

    /// rear快到内存内存边界时，要么重新申请内存，要么进行绕接
    if (m_rear >= m_length)
    {
        if ((m_rear > m_front) && ((m_rear - m_front) >= m_length))
        {
            m_queue = (QueNode *)realloc(m_queue, (m_length + m_count) * sizeof(QueNode));
            if (!m_queue)
            {
                return;
            }

            m_length += m_count;
        }
        else
        {
            m_rear = 0;
        }
    }

    m_queue[m_rear].parent = curIdx;
    m_queue[m_rear].child = m_rear;
    m_queue[m_rear].childPos = adjVex;
    m_rear++;
}

/*******************************************************
  函 数 名: CGraphSearchTool::DeQueue
  描    述: 从队列中取出节点
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CGraphSearchTool::DeQueue(IGraph::POS &adjVex, int &bufIdx)
{
    /// 队列不能为空
    if (!m_queue)
    {
        return;
    }

    /// 队列取空就返回
    if (m_front == m_rear)
    {
        return;
    }

    /// front快到内存内存边界时，进行绕接
    if (m_front >= m_length)
    {
        m_front = 0;
    }

    adjVex = m_queue[m_front].childPos;
    bufIdx = m_queue[m_front].child;
    m_front++;
}

/*******************************************************
  函 数 名: CGraphSearchTool::PushStack
  描    述: 压栈
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CGraphSearchTool::PushStack(IGraph::POS adjVex, int curIdx)
{
    EnQueue(adjVex, curIdx);
}

/*******************************************************
  函 数 名: CGraphSearchTool::PopStack
  描    述: 出栈
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CGraphSearchTool::PopStack(IGraph::POS &adjVex, int &bufIdx)
{
    /// 队列不能为空
    if (!m_queue)
    {
        return;
    }

    /// 队列取空就返回
    if (!m_rear)
    {
        return;
    }

    adjVex = m_queue[m_rear - 1].childPos;
    bufIdx = m_queue[m_rear - 1].child;
    m_rear--;
}

/*******************************************************
  函 数 名: CGraphSearchTool::IsDest
  描    述: 是否目标值
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CGraphSearchTool::IsDest(IGraph::POS curVex, int curIdx, IGraph::POS dstVex, bool &bContinue, bool bBufIdx)
{
    /// 如果找到目的点，就进行记录
    bool bRc = (m_fnIsDest)? (m_fnIsDest(curVex, dstVex, bContinue, this)) : (curVex == dstVex);
    if ( !bRc )
    {
        return false;
    }

    /// 如果不需要记录索引，直接返回(使用入栈出栈方式的索引不能缓存，因为出栈后失效了)
    if ( !bBufIdx )
    {
        EnQueue(curVex, curIdx);
        return true;
    }

    /// 如果准备记录的内存不足，就需要重新申请
    if ( !Dalloc((void *&)m_pIdxBuffer, m_dwIdxBufferMemLen, m_dwIdxBufferCount, 1, m_dwIdxBufferMemLenStep, (DWORD)(sizeof(int))) )
    {
        return false;
    }

    EnQueue(curVex, curIdx);
    m_pIdxBuffer[m_dwIdxBufferCount++] = GetQueRear();

    return true;
}

/*******************************************************
  函 数 名: CGraphSearchTool::GetDestPath
  描    述: 获取目的路径
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CGraphSearchTool::GetDestPath(IGraph::POS *&rpPath, DWORD &rdwPathLen, int iBufIdx)
{
    if (iBufIdx < 0)
    {
        iBufIdx = GetQueRear() - 1;
    }

    if (iBufIdx < 0)
    {
        return false;
    }

    /// 初始化输出
    rpPath = 0;
    rdwPathLen = 0;

    /// 获取路径长度
    DWORD tmp = 0;
    QueNode *pQueNode = 0;
    for (tmp = iBufIdx; (tmp) && (pQueNode = GetQueNode(tmp)); tmp = pQueNode->parent)
    {
        rdwPathLen++;
    }

    /// 申请输出内存
    DWORD pathIdx = rdwPathLen - 1;
    rpPath = (IGraph::POS *)malloc(rdwPathLen * sizeof(IGraph::POS));
    if (!rpPath)
    {
        rdwPathLen = 0;
        return false;
    }

    /// 逆序获取路径
    for (tmp = iBufIdx; (tmp) && (pQueNode = GetQueNode(tmp)); tmp = pQueNode->parent)
    {
        rpPath[pathIdx--] = pQueNode->childPos;
    }

    return true;
}

/*******************************************************
  函 数 名: CGraphSearchTool::Clear
  描    述: 清除空间
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CGraphSearchTool::Clear()
{
    if (m_visited)
    {
        free(m_visited);
        m_visited = 0;
    }

    m_visitedCount = 0;

    if (m_queue)
    {
        free(m_queue);
        m_queue = 0;
    }

    m_length = 0;
    m_front = 0;
    m_rear = 0;

    if (m_pIdxBuffer)
    {
        free(m_pIdxBuffer);
        m_pIdxBuffer = 0;
    }

    m_dwIdxBufferMemLen = GRAPH_PATH_COUNT_STEP;
    m_dwIdxBufferMemLenStep = GRAPH_PATH_COUNT_STEP;
    m_dwIdxBufferCount = 0;

}

/*******************************************************
  函 数 名: CGraphSearchTool::Dalloc
  描    述: 动态分配内存接口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CGraphSearchTool::Dalloc(void *&rpBuf, DWORD &rdwBufMemLen, DWORD dwUsedMemLen, DWORD dwNeedMemLen, DWORD dwBufMemLenStep, DWORD dwBufMemSize)
{
    /// 已有内存且长度满足需要，直接返回成功
    if ( rpBuf && ((dwUsedMemLen + dwNeedMemLen) <= rdwBufMemLen) )
    {
        return true;
    }

    /// 申请长度取递增步长和需要的最大值
    DWORD dwTmpCount = rdwBufMemLen + dwBufMemLenStep;
    if (dwTmpCount < (dwUsedMemLen + dwNeedMemLen))
    {
        dwTmpCount = dwUsedMemLen + dwNeedMemLen;
    }

    /// 无法申请的长度，返回失败
    if (!dwTmpCount)
    {
        return false;
    }

    /// 申请内存，失败返回
    int *pTmp = (int *)malloc(dwTmpCount * dwBufMemSize);
    if (!pTmp)
    {
        return false;
    }

    /// 内存清零
    (void)memset(pTmp, 0, dwTmpCount * dwBufMemSize);

    /// 拷贝已经存在的内存到前面
    if (rpBuf)
    {
        (void)memcpy(pTmp, rpBuf, dwUsedMemLen * dwBufMemSize);
        free(rpBuf);
    }

    /// 输出新的内存和长度
    rpBuf = pTmp;
    rdwBufMemLen = dwTmpCount;

    return true;
}

/*******************************************************
  函 数 名: CGraphSearchTool::Dfree
  描    述: 动态释放内存接口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CGraphSearchTool::Dfree(void *&rpBuf, DWORD &rdwBufMemLen)
{
    if (rpBuf)
    {
        free(rpBuf);
        rpBuf = 0;
    }

    rdwBufMemLen = 0;
}

