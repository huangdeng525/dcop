/// -------------------------------------------------
/// bfs.cpp : 广度优先搜索算法实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "graph/bfs.h"
#include <stdlib.h>
#include <memory.h>


/*******************************************************
  函 数 名: CBFS::CBFS
  描    述: CBFS构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CBFS::CBFS()
{
}

/*******************************************************
  函 数 名: CBFS::~CBFS
  描    述: CBFS析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CBFS::~CBFS()
{
}

/*******************************************************
  函 数 名: CBFS::FindShortestPath
  描    述: 查找最短路径
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CBFS::FindShortestPath(IGraph::POS throughVex, 
                        IGraph::POS srcVex, 
                        IGraph::POS dstVex, 
                        IGraph::POS *&rpPath, 
                        DWORD &rdwPathLen)
{
    if (!GetGraph())
    {
        return false;
    }

    if ((IGraph::Null == srcVex) || (IGraph::Null == dstVex))
    {
        return false;
    }

    if (!InitVisited())
    {
        return false;
    }

    if (!InitQueue())
    {
        return false;
    }

    /// 广度优先进行搜索
    IGraph::POS curVex = srcVex;
    SetVisited(curVex, true);
    EnQueue(curVex, -1);
    int curIdx = 0;
    bool bFound = false;
    while (!QueueEmpty() && !bFound)
    {
        DeQueue(curVex, curIdx);
        QueNode *pParentQueNode = GetParentQueNode(curIdx);
        if (pParentQueNode) throughVex = pParentQueNode->childPos;

        for (IGraph::POS tmpVex = GetGraph()->FirstAdjVex(throughVex, curVex, dstVex);
            tmpVex != IGraph::Null;
            tmpVex = GetGraph()->NextAdjVex(throughVex, curVex, tmpVex, dstVex))
        {
            if (tmpVex == dstVex)
            {
                bFound = true;
                EnQueue(tmpVex, curIdx);
                break;
            }

            if (GetVisited(tmpVex))
            {
                continue;
            }

            /// 和DFS不同, 这里只要访问过的子节点都设置为访问过的
            SetVisited(tmpVex, true);
            EnQueue(tmpVex, curIdx);
        }
    }

    if (!bFound)
    {
        return false;
    }

    /// 获取路径长度
    DWORD tmp = 0;
    QueNode *pQueNode = 0;
    for (tmp = GetQueRear() - 1; (tmp) && (pQueNode = GetQueNode(tmp)); tmp = pQueNode->parent)
    {
        rdwPathLen++;
    }

    DWORD pathIdx = rdwPathLen - 1;
    rpPath = (IGraph::POS *)malloc(rdwPathLen * sizeof(IGraph::POS));
    if(!rpPath)
    {
        rdwPathLen = 0;
        return false;
    }

    /// 逆序获取路径
    for (tmp = GetQueRear() - 1; (tmp) && (pQueNode = GetQueNode(tmp)); tmp = pQueNode->parent)
    {
        rpPath[pathIdx--] = pQueNode->childPos;
    }

    return true;
}

/*******************************************************
  函 数 名: CBFS::FreePath
  描    述: 释放路径
  输    入: 
  输    出: 
  返    回: 
  
  修改记录: 
 *******************************************************/
void CBFS::FreePath(IGraph::POS *pPath)
{
    if (pPath)
    {
        free(pPath);
    }

    Clear();
}


