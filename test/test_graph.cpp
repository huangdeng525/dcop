/// -------------------------------------------------
/// test_graph.cpp : 主要测试tool graph操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test_graph.h"


/// -------------------------------------------------
/// GRAPH 测试用例
/// -------------------------------------------------
TEST_SUITE_TABLE(GRAPH)

    /// 图的BFS搜索测试
    TEST_SUITE_ITEM(CTestSuite_BFS)
        TEST_CASE_ITEM(6)
            "0", "0", "2", "1", "0", "19"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(6)
            "0", "11", "19", "19", "0", "45"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(6)
            "0", "19", "6", "0", "0", "0"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(6)
            "7", "0", "7", "0", "0", "2"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(6)
            "0", "6", "19", "0", "1", "22"
        TEST_CASE_ITEM_END
    TEST_SUITE_ITEM_END

    /// 图的DFS搜索测试
    TEST_SUITE_ITEM(CTestSuite_DFS)
        TEST_CASE_ITEM(7)
            "0", "0", "2", "1", "0", "4", "0"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(7)
            "0", "0", "2", "1", "0", "4", "1"
        TEST_CASE_ITEM_END
    TEST_SUITE_ITEM_END

    /// 三角DFS搜索测试
    TEST_SUITE_ITEM(CTestSuite_TRI)
        TEST_CASE_ITEM(7)
            "-1", "2", "24", "5", "0", "1", "0"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(7)
            "-1", "2", "24", "5", "0", "1", "1"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(7)
            "1", "4", "39", "39", "0", "1", "0"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(7)
            "1", "4", "39", "39", "0", "1", "1"
        TEST_CASE_ITEM_END
    TEST_SUITE_ITEM_END

    /// 连线搜索测试
    TEST_SUITE_ITEM(CTestSuite_LINE)
        TEST_CASE_ITEM(9)
            "1", "2", "1", "3", "2", "3", "4", "3", "1"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(9)
            "1", "2", "1", "3", "2", "3", "1", "4", "1"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(11)
            "1", "2", "1", "3", "2", "3", "1", "4", "4", "3", "1"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(13)
            "1", "2", "1", "3", "2", "3", "1", "4", "4", "3", "4", "2", "0"
        TEST_CASE_ITEM_END
    TEST_SUITE_ITEM_END

TEST_SUITE_TABLE_END

/// -------------------------------------------------
/// GRAPH 测试套
/// -------------------------------------------------
IMPLEMENT_REGTESTSUITE_FUNC(GRAPH)


CTestSuite_MAP::CTestSuite_MAP()
{
    m_pMap = 0;
}

CTestSuite_MAP::~CTestSuite_MAP()
{
    if (m_pMap)
    {
        delete m_pMap;
    }
}

void CTestSuite_MAP::BeforeTest()
{
    m_pMap = new CXYMap();
}

void CTestSuite_BFS::BeforeTest()
{
    int aiMap[] = 
    {
        0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0,
    };

    CTestSuite_MAP::BeforeTest();
    CXYMap *pMap = GetMap();
    if (!pMap)
    {
        return;
    }

    DWORD dwMarginX = 20;
    DWORD dwMarginY = (sizeof(aiMap)/sizeof(int)) / dwMarginX;
    pMap->Init(dwMarginX, dwMarginY, aiMap);

}

int CTestSuite_BFS::TestEntry(int argc, char* argv[])
{
    CXYMap *pMap = GetMap();
    if (!pMap)
    {
        return -1;
    }

    CBFS *pBfs = new CBFS();
    if (!pBfs)
    {
        return -2;
    }

    pBfs->SetGraph(pMap);

    if ((argc < 6) || (!argv))
    {
        delete pBfs;
        return -3;
    }

    /// 输入参数
    int iSrcX = atoi(argv[0]);
    int iSrcY = atoi(argv[1]);
    int iDstX = atoi(argv[2]);
    int iDstY = atoi(argv[3]);
    int iDirectType = atoi(argv[4]);
    int iPathLen = atoi(argv[5]);

    IGraph::POS *pPath = 0;
    DWORD dwPathLen = 0;
    pMap->SetDirectType((CXYMap::DIRECT_TYPE)iDirectType);
    bool bRc = pBfs->FindShortestPath(pMap->Vex(CXYMap::POS(-1, -1)), 
                            pMap->Vex(CXYMap::POS(iSrcX, iSrcY)), 
                            pMap->Vex(CXYMap::POS(iDstX, iDstY)), 
                            pPath, dwPathLen);
    if (!bRc)
    {
        printf("[PATH(None)] : {%d,%d} -> {%d,%d}\n", iSrcX, iSrcY, iDstX, iDstY);

        delete pBfs;
        return (!iPathLen)? 0 : -4;
    }

    printf("[PATH(%u)] : {%d,%d}", dwPathLen, iSrcX, iSrcY);
    for (DWORD i = 0; i < dwPathLen; ++i)
    {
        CXYMap::POS tmpPos;
        pMap->Pos(pPath[i], tmpPos);
        printf(" -> {%d,%d}", tmpPos.x, tmpPos.y);
    }
    printf("\n");

    pBfs->FreePath(pPath);
    delete pBfs;
    return (dwPathLen == (DWORD)iPathLen)? 0 : -5;

}

void CTestSuite_DFS::BeforeTest()
{
    int aiMap[] = 
    {
        0, 1, 0,
        0, 0, 0,
        0, 0, 0,
    };

    CTestSuite_MAP::BeforeTest();
    CXYMap *pMap = GetMap();
    if (!pMap)
    {
        return;
    }

    DWORD dwMarginX = 3;
    DWORD dwMarginY = (sizeof(aiMap)/sizeof(int)) / dwMarginX;
    pMap->Init(dwMarginX, dwMarginY, aiMap);

}

int CTestSuite_DFS::TestEntry(int argc, char* argv[])
{
    CXYMap *pMap = GetMap();
    if (!pMap)
    {
        return -1;
    }

    CDFS *pDfs = new CDFS();
    if (!pDfs)
    {
        return -2;
    }

    pDfs->SetGraph(pMap);

    if ((argc < 7) || (!argv))
    {
        delete pDfs;
        return -3;
    }

    /// 输入参数
    int iSrcX = atoi(argv[0]);
    int iSrcY = atoi(argv[1]);
    int iDstX = atoi(argv[2]);
    int iDstY = atoi(argv[3]);
    int iDirectType = atoi(argv[4]);
    int iPathCount = atoi(argv[5]);
    int iRecur = atoi(argv[6]);

    CDFS::PathNode *pPaths = 0;
    DWORD dwPathCount = 0;
    pMap->SetDirectType((CXYMap::DIRECT_TYPE)iDirectType);
    bool bRc = pDfs->FindAllPath(pMap->Vex(CXYMap::POS(-1, -1)), 
                            pMap->Vex(CXYMap::POS(iSrcX, iSrcY)), 
                            pMap->Vex(CXYMap::POS(iDstX, iDstY)), 
                            pPaths, dwPathCount, (iRecur)?true:false);
    if (!bRc)
    {
        printf("[PATH(None)] : {%d,%d} -> {%d,%d}\n", iSrcX, iSrcY, iDstX, iDstY);

        delete pDfs;
        return (!iPathCount)? 0 : -4;
    }

    for (DWORD l = 0; l < dwPathCount; ++l)
    {
        printf("[PATH(%u)] : {%d,%d}", pPaths[l].pathLen, iSrcX, iSrcY);
        for (DWORD i = 0; i < pPaths[l].pathLen; ++i)
        {
            CXYMap::POS tmpPos;
            pMap->Pos(pPaths[l].path[i], tmpPos);
            printf(" -> {%d,%d}", tmpPos.x, tmpPos.y);
        }
        printf("\n");
    }

    pDfs->FreePaths(pPaths, dwPathCount);
    delete pDfs;
    return (dwPathCount == (DWORD)iPathCount)? 0 : -5;

}

CTestSuite_TRI::CTestSuite_TRI()
{
    m_pMap = 0;
}

CTestSuite_TRI::~CTestSuite_TRI()
{
    if (m_pMap)
    {
        delete m_pMap;
    }
}

void CTestSuite_TRI::BeforeTest()
{
    int aiMap[] = 
    {
        '\\', '/',  '\\', '\\', '\\', '/', 
        '/',  '\\', '/',  '/',  '/',  '/', 
        '\\', '/',  '/',  '\\', '/',  '\\', 
        '\\', '/',  '\\', '/',  '/',  '/', 
        '\\', '\\', '\\', '/',  '/',  '/', 
        '\\', '\\', '\\', '/',  '/',  '/', 
    };

    m_pMap = new CTriangleMap();
    if (!m_pMap)
    {
        return;
    }

    DWORD dwMarginX = 6;
    DWORD dwMarginY = (sizeof(aiMap)/sizeof(int)) / dwMarginX;
    m_pMap->Init(dwMarginX, dwMarginY, aiMap);
    
}

int CTestSuite_TRI::TestEntry(int argc, char* argv[])
{
    CTriangleMap *pMap = m_pMap;
    if (!pMap)
    {
        return -1;
    }

    CDFS *pDfs = new CDFS();
    if (!pDfs)
    {
        return -2;
    }

    pDfs->SetGraph(pMap);

    if ((argc < 6) || (!argv))
    {
        delete pDfs;
        return -3;
    }

    /// 输入参数
    int iThrX = atoi(argv[0]);
    int iThrY = atoi(argv[1]);
    int iSrcId = atoi(argv[2]);
    int iDstId = atoi(argv[3]);
    int iDirectType = atoi(argv[4]);
    int iPathCount = atoi(argv[5]);
    int iRecur = atoi(argv[6]);

    CDFS::PathNode *pPaths = 0;
    DWORD dwPathCount = 0;
    pMap->SetDirectType((CXYMap::DIRECT_TYPE)iDirectType);
    bool bRc = pDfs->FindAllPath(pMap->Vex(CXYMap::POS(iThrX, iThrY)), 
                            iSrcId, 
                            iDstId, 
                            pPaths, dwPathCount, (iRecur)?true:false);
    if (!bRc)
    {
        printf("[PATH(None)] : {%d} -> {%d}\n", iSrcId, iDstId);

        delete pDfs;
        return (!iPathCount)? 0 : -4;
    }

    for (DWORD l = 0; l < dwPathCount; ++l)
    {
        printf("[PATH(%u)] : {%d}", pPaths[l].pathLen, iSrcId);
        for (DWORD i = 0; i < pPaths[l].pathLen; ++i)
        {
        #if 0
            CXYMap::POS tmpPos;
            pMap->Pos(pPaths[l].path[i], tmpPos);
            printf(" -> {%d,%d}", tmpPos.x, tmpPos.y);
        #endif
            printf(" -> {%d}", pPaths[l].path[i]);
        }
        printf("\n");
    }

    pDfs->FreePaths(pPaths, dwPathCount);
    delete pDfs;
    return (dwPathCount == (DWORD)iPathCount)? 0 : -5;
}

CTestSuite_LINE::CTestSuite_LINE()
{
    m_pLine = 0;
}

CTestSuite_LINE::~CTestSuite_LINE()
{
    if (m_pLine)
    {
        delete m_pLine;
    }
}

bool CTestSuite_LINE::IsDest(IGraph::POS curVex, IGraph::POS dstVex, bool &bContinue, CGraphSearchTool *pThis)
{
    if (!pThis)
    {
        return false;
    }

    if (pThis->NullPos(curVex))
    {
        return false;
    }

    if (pThis->GetVisited(curVex))
    {
        return false;
    }

    if (pThis->GetVisitedCount() < (pThis->GetCount() - 1))
    {
        return false;
    }
    
    bContinue = false;
    return true;
}

int CTestSuite_LINE::TestEntry(int argc, char* argv[])
{
    m_pLine = new CNetwork();
    if (!m_pLine)
    {
        return -1;
    }

    /// 输入参数
    m_pLine->StartAdd(0, argc, 0, argc, 0, argc, true);
    for (int i = 0; i < argc; )
    {
        if (i == (argc - 1)) break;

        int iStartNode  = atoi(argv[i++]);
        int iEndNode    = atoi(argv[i++]);
        m_pLine->Add(iStartNode, iEndNode, 1, 1);
    }
    m_pLine->EndAdd();

    /// 打印
    m_pLine->Dump();

    CDFS *pDfs = new CDFS();
    if (!pDfs)
    {
        delete m_pLine;
        m_pLine = 0;
        return -2;
    }

    pDfs->SetGraph(m_pLine);
    pDfs->SetIsDestFunc(IsDest);

    int iPathCount = (argc && (argc%2))? atoi(argv[argc-1]) : 0;

    CDFS::PathNode *pPaths = 0;
    DWORD dwPathCount = 0;
    IGraph::POS ship = m_pLine->GetFirstRelationShip();
    if (IGraph::Null == ship)
    {
        delete pDfs;
        delete m_pLine;
        m_pLine = 0;
        return -3;
    }

    bool bRc = false;
    IGraph::POS posLine = IGraph::Null;
    IGraph::POS throughLine = IGraph::Null;
    while (ship != IGraph::Null)
    {
        CNetwork::RelationShip *pShip = m_pLine->GetRelationShipNode(ship);
        if (!pShip) continue;

        posLine = pShip->connectIdx;
        throughLine = (pShip->curNode < pShip->adjNode)? -1 : -2;
        bRc = pDfs->FindAllPath(throughLine, posLine, posLine, pPaths, dwPathCount);
        if (bRc) break;

        ship = m_pLine->GetNextRelationShip(ship);
    }

    if (!bRc)
    {
        printf("[PATH(None)] \n");

        delete pDfs;
        delete m_pLine;
        m_pLine = 0;
        return (!iPathCount)? 0 : -4;
    }

#if 0
    for (DWORD l = 0; l < dwPathCount; ++l)
    {
        printf("[PATH(%lu)] : {%d} -> {%d}", pPaths[l].pathLen, throughLine, posLine);
        for (DWORD i = 0; i < pPaths[l].pathLen; ++i)
        {
            printf(" -> {%d}", pPaths[l].path[i]);
        }
        printf("\n");
    }
#endif
    for (DWORD l = 0; l < dwPathCount; ++l)
    {
        IGraph::POS posNode = m_pLine->GetSameNodeOfConnect(throughLine, posLine);
        printf("[PATH(%u)] : {%d}", pPaths[l].pathLen, posNode);
        for (DWORD i = 0; i < pPaths[l].pathLen; ++i)
        {
            throughLine = posLine;
            posLine     = pPaths[l].path[i];
            posNode     = m_pLine->GetSameNodeOfConnect(throughLine, posLine);
            printf(" -> {%d}", posNode);
        }
        posNode = m_pLine->GetThatNodeOfConnect(posLine, posNode);
        printf(" -> {%d}\n", posNode);
    }

    pDfs->FreePaths(pPaths, dwPathCount);
    delete pDfs;
    delete m_pLine;
    m_pLine = 0;

    return (dwPathCount == (DWORD)iPathCount)? 0 : -5;
}

