/// -------------------------------------------------
/// dfs.cpp : 广度优先搜索算法实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "graph/dfs.h"
#include <stdlib.h>
#include <memory.h>


/*******************************************************
  函 数 名: CDFS::CDFS
  描    述: CDFS构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDFS::CDFS()
{
}

/*******************************************************
  函 数 名: CDFS::~CDFS
  描    述: CDFS析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDFS::~CDFS()
{
}

/*******************************************************
  函 数 名: CDFS::DFS
  描    述: DFS非递归查找
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CDFS::DFS(IGraph::POS throughVex, 
                        IGraph::POS srcVex, 
                        IGraph::POS dstVex, 
                        PathNode *&rpPaths, 
                        DWORD &rdwPathCount)
{
    /// 深度优先搜索 - 和BFS不同，这里只有父节点才设置为访问的，而且递归结束后返回上一级时要把子节点全部重新标示为未访问
    bool bRc = false;
    bool bContinue = true;
    IGraph::POS startVex = throughVex;
    IGraph::POS curVex = srcVex;
    IGraph::POS tmpVex = IGraph::Null;
    int curIdx = -1;
    while (!GetGraph()->NullPos(curVex) || !QueueEmpty())
    {
        /// 当前节点是有效值就入栈，无效值就出栈一个有效值
        if (!GetGraph()->NullPos(curVex))
        {
            PushStack(curVex, curIdx);
            SetVisited(curVex, true);
            tmpVex = IGraph::Null;
            curIdx = GetQueRear() - 1;
        }
        else
        {
            PopStack(curVex, curIdx);
            SetVisited(curVex, false);
            tmpVex = curVex;
            QueNode *pParentQueNode = GetParentQueNode(curIdx);
            curVex = (pParentQueNode)? pParentQueNode->childPos : IGraph::Null;
            curIdx = (pParentQueNode)? pParentQueNode->child : -1;
            if (GetGraph()->NullPos(curVex))
            {
                continue;
            }
        }

        /// 获取上上级节点
        QueNode *pThroughQueNode = GetParentQueNode(curIdx);
        throughVex = (pThroughQueNode)? pThroughQueNode->childPos : startVex;

        /// 获取下一个子节点
        while (!GetGraph()->NullPos(curVex))
        {
            if (GetGraph()->NullPos(tmpVex))
            {
                tmpVex = GetGraph()->FirstAdjVex(throughVex, curVex, dstVex);
            }
            else
            {
                tmpVex = GetGraph()->NextAdjVex(throughVex, curVex, tmpVex, dstVex);
            }

            if (GetGraph()->NullPos(tmpVex))
            {
                break;
            }

            /// 如果找到目的点，就进行记录
            if (IsDest(tmpVex, curIdx, dstVex, bContinue, false))
            {
                tmpVex = IGraph::Null;

                DWORD dwPathsCount = rdwPathCount;
                if ( !Dalloc( (void *&)rpPaths, dwPathsCount, rdwPathCount, 1, 1, (DWORD)(sizeof(PathNode)) ) )
                {
                    continue;
                }

                bRc = GetDestPath(rpPaths[rdwPathCount].path, rpPaths[rdwPathCount].pathLen);
                if ( !bRc )
                {
                    continue;
                }

                rdwPathCount++;

                /// 找到目标，如果不需要继续寻找，则返回
                if (!bContinue) return true;

                break;
            }

            if (GetVisited(tmpVex))
            {
                continue;
            }

            break;
        }

        curVex = tmpVex;

    }

    return bRc;
}

/*******************************************************
  函 数 名: CDFS::DFS
  描    述: DFS递归查找
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDFS::DFS(IGraph::POS throughVex, 
                        IGraph::POS curVex, 
                        IGraph::POS dstVex, 
                        bool &bContinue, 
                        int curQueIdx)
{
    /// 和BFS不同，这里只有父节点才设置为访问的，而且递归结束后返回上一级时要把子节点全部重新标示为未访问
    SetVisited(curVex, true);

    for (IGraph::POS tmpVex = GetGraph()->FirstAdjVex(throughVex, curVex, dstVex);
            tmpVex != IGraph::Null;
            tmpVex = GetGraph()->NextAdjVex(throughVex, curVex, tmpVex, dstVex))
    {
        /// 如果找到目的点，就进行记录
        if (IsDest(tmpVex, curQueIdx, dstVex, bContinue))
        {
            /// 找到目标，如果不需要继续寻找，则返回
            if (!bContinue) return;

            continue;
        }

        if (GetVisited(tmpVex))
        {
            continue;
        }

        EnQueue(tmpVex, curQueIdx);

        /// 找本tmp节点的邻接点
        DFS(curVex, tmpVex, dstVex, bContinue, GetQueRear() - 1);

        /// 再把本tmp节点设置为未访问
        SetVisited(tmpVex, false);
    }
}

/*******************************************************
  函 数 名: CDFS::FindAllPath
  描    述: DFS查找所有路径
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CDFS::FindAllPath(IGraph::POS throughVex, 
                        IGraph::POS srcVex, 
                        IGraph::POS dstVex, 
                        PathNode *&rpPaths, 
                        DWORD &rdwPathCount, 
                        bool bRecur)
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

    /// 深度优先进行搜索
    if (!bRecur)
    {
        /// 使用非递归方式查找
        return DFS(throughVex, srcVex, dstVex, rpPaths, rdwPathCount);
    }

    /// 使用递归方式查找，先初始化索引缓存
    if (!InitIdxBuffer(DFS_PATH_COUNT_STEP, DFS_PATH_COUNT_STEP))
    {
        return false;
    }

    bool bContinue = true;
    EnQueue(srcVex, -1);
    DFS(throughVex, srcVex, dstVex, bContinue, 0);

    int *pLstDstIdx = GetIdxBuffer();
    DWORD dwDstCount = GetIdxBufferCount();

    if (!dwDstCount)
    {
        return false;
    }

    /// 整理到输出路径中(在原路径中继续添加)
    DWORD dwPathsCount = rdwPathCount;
    if ( !Dalloc( (void *&)rpPaths,  dwPathsCount, rdwPathCount, dwDstCount, dwDstCount, (DWORD)(sizeof(PathNode)) ) )
    {
        return false;
    }

    /// 获取路径
    (void)memset(rpPaths, 0, dwDstCount * sizeof(PathNode));
    for (DWORD i = 0; i < dwDstCount; ++i)
    {
        GetDestPath(rpPaths[rdwPathCount + i].path, rpPaths[rdwPathCount + i].pathLen, pLstDstIdx[i] - 1);
    }

    rdwPathCount += dwDstCount;
    return true;
}

/*******************************************************
  函 数 名: CDFS::FreePaths
  描    述: 释放路径
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDFS::FreePaths(PathNode *pPaths, DWORD dwPathCount)
{
    if (!pPaths)
    {
        return;
    }

    for (DWORD i = 0; i < dwPathCount; ++i)
    {
        if (pPaths[i].path)
        {
            free(pPaths[i].path);
        }
    }

    free(pPaths);

    Clear();
}

