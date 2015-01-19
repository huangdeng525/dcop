/// -------------------------------------------------
/// test_graph.h : 主要测试tool graph操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TEST_SUITE_GRAPH_H_
#define _TEST_SUITE_GRAPH_H_

#include "test.h"

#include "graph/bfs.h"
#include "graph/dfs.h"
#include "graph/map.h"
#include "graph/triangle.h"
#include "graph/network.h"

/// 格子地图测试套
class CTestSuite_MAP : public ITestSuite
{
public:
    CTestSuite_MAP();
    ~CTestSuite_MAP();

    void BeforeTest();

    CXYMap *GetMap() {return m_pMap;}

private:
    CXYMap *m_pMap;
};

/// 广度搜索测试套
class CTestSuite_BFS : public CTestSuite_MAP
{
public:
    void BeforeTest();

    int TestEntry(int argc, char* argv[]);

};

/// 深度搜索测试套
class CTestSuite_DFS : public CTestSuite_MAP
{
public:
    void BeforeTest();

    int TestEntry(int argc, char* argv[]);

};

/// 三角地图测试套
class CTestSuite_TRI : public ITestSuite
{
public:
    CTestSuite_TRI();
    ~CTestSuite_TRI();

    void BeforeTest();

    int TestEntry(int argc, char* argv[]);

private:
    CTriangleMap *m_pMap;
};

/// 连接线搜索测试套
class CTestSuite_LINE :  public ITestSuite
{
public:
    CTestSuite_LINE();
    ~CTestSuite_LINE();

    int TestEntry(int argc, char* argv[]);

    static bool IsDest(IGraph::POS curVex, IGraph::POS dstVex, bool &bContinue, CGraphSearchTool *pThis);

private:
    CNetwork *m_pLine;
};


#endif // #ifndef _TEST_SUITE_GRAPH_H_
