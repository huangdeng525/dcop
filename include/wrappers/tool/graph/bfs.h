/// -------------------------------------------------
/// bfs.h : ������������㷨����ͷ�ļ�
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_GRAPH_BFS_H_
#define _TOOL_GRAPH_BFS_H_

/////////////////////////////////////////////////////
///     BFS:������������㷨(BreadthFirstSearch)
/// -------------------------------------------------
/// ������ȱ�����������ȱ������������ڣ�������ȱ���
/// ���Բ�Ϊ˳�򣬽�ĳһ���ϵ����нڵ㶼��������֮���
/// ����һ����������������ȱ����ǽ�ĳһ��֦���ϵ�����
/// �ڵ㶼��������֮�󣬲�ת��������һ��֦���ϵ����н�
/// �㡣
/// -------------------------------------------------
/// ������ȱ�����ĳ��������������ȷ���������㣬Ȼ��
/// �ҳ��շ���������ĵ�һ��δ�����ʵ��ڽ�㣬Ȼ����
/// �Դ��ڽ��Ϊ���㣬������������һ���µĶ�����з��ʣ�
/// �ظ��˲��裬ֱ�����н�㶼��������Ϊֹ��
/// -------------------------------------------------
/// ������ȱ�����ĳ��������������ȷ���������㣬Ȼ��
/// �ҳ������������δ�����ʵ��ڽӵ㣬��������ٷ���
/// ��Щ����е�һ���ڽӵ�����н�㣬�ظ��˷�����ֱ��
/// ���н�㶼��������Ϊֹ��
/// -------------------------------------------------
/// ���Կ������ַ���������������ǰ�ߴӶ���ĵ�һ����
/// �ӵ�һֱ������ȥ�ٷ��ʶ���ĵڶ����ڽӵ㣻���ߴӶ�
/// �㿪ʼ���ʸö���������ڽӵ����������£�һ��һ���
/// ���ʡ�
/// -------------------------------------------------
/////////////////////////////////////////////////////


#include "graph/graph.h"


/// ��������������·��
class CBFS : private CGraphSearchTool
{
public:
    CBFS();
    ~CBFS();

    /// ����ͼ����
    void SetGraph(IGraph *pGraph)
    {
        CGraphSearchTool::SetGraph(pGraph);
    }

    /// �������·��
    bool FindShortestPath(
                    IGraph::POS throughVex, 
                    IGraph::POS srcVex, 
                    IGraph::POS dstVex, 
                    IGraph::POS *&rpPath, 
                    DWORD &rdwPathLen);

    /// �ͷ�·��
    void FreePath(IGraph::POS *pPath);

};


#endif // #ifndef _TOOL_ALGO_BFS_H_
