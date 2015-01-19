/// -------------------------------------------------
/// dfs.h : 广度优先搜索算法公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_GRAPH_DFS_H_
#define _TOOL_GRAPH_DFS_H_

/////////////////////////////////////////////////////
///     DFS:深度优先搜索算法(DepthFirstSearch)
/// -------------------------------------------------
/// 广度优先遍历与深度优先遍历的区别在于：广度优先遍历
/// 是以层为顺序，将某一层上的所有节点都搜索到了之后才
/// 向下一层搜索；而深度优先遍历是将某一条枝桠上的所有
/// 节点都搜索到了之后，才转向搜索另一条枝桠上的所有节
/// 点。
/// -------------------------------------------------
/// 深度优先遍历从某个顶点出发，首先访问这个顶点，然后
/// 找出刚访问这个结点的第一个未被访问的邻结点，然后再
/// 以此邻结点为顶点，继续找它的下一个新的顶点进行访问，
/// 重复此步骤，直到所有结点都被访问完为止。
/// -------------------------------------------------
/// 广度优先遍历从某个顶点出发，首先访问这个顶点，然后
/// 找出这个结点的所有未被访问的邻接点，访问完后再访问
/// 这些结点中第一个邻接点的所有结点，重复此方法，直到
/// 所有结点都被访问完为止。
/// -------------------------------------------------
/// 可以看到两种方法最大的区别在于前者从顶点的第一个邻
/// 接点一直访问下去再访问顶点的第二个邻接点；后者从顶
/// 点开始访问该顶点的所有邻接点再依次向下，一层一层的
/// 访问。
/// -------------------------------------------------
/////////////////////////////////////////////////////

#include "graph/graph.h"



#define DFS_PATH_COUNT_STEP 10

/// 深度优先搜索所有路径
class CDFS : private CGraphSearchTool
{
public:

    /// 路径列表节点
    typedef struct tagPathNode
    {
        IGraph::POS *path;
        DWORD pathLen;
    }PathNode;

public:
    CDFS();
    ~CDFS();

    /// 设置图对象
    void SetGraph(IGraph *pGraph) { CGraphSearchTool::SetGraph(pGraph); }

    /// 设置目的判断回调函数
    void SetIsDestFunc(IS_DEST_FUNC fnIsDest) { CGraphSearchTool::SetIsDestFunc(fnIsDest); }

    /// 查找所有路径
    bool FindAllPath(
                    IGraph::POS throughVex, 
                    IGraph::POS srcVex, 
                    IGraph::POS dstVex, 
                    PathNode *&rpPaths, 
                    DWORD &rdwPathCount, 
                    bool bRecur = false);       // 是否递归

    /// 释放路径
    void FreePaths(PathNode *pPaths, 
                    DWORD dwPathCount);

private:
    /// 递归
    void DFS(IGraph::POS throughVex, 
                    IGraph::POS curVex, 
                    IGraph::POS dstVex, 
                    bool &bContinue, 
                    int curQueIdx);

    /// 非递归
    bool DFS(IGraph::POS throughVex, 
                    IGraph::POS curVex, 
                    IGraph::POS dstVex, 
                    PathNode *&rpPaths, 
                    DWORD &rdwPathCount);

};


#endif // #ifndef _TOOL_GRAPH_DFS_H_

