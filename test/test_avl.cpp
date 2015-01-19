/// -------------------------------------------------
/// test_avl.cpp : Ö÷Òª²âÊÔavl²Ù×÷
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test_avl.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// -------------------------------------------------
/// AVL ²âÊÔÓÃÀý
/// -------------------------------------------------
TEST_SUITE_TABLE(AVL)
    TEST_SUITE_ITEM(CTestSuite_AVL)
        TEST_CASE_ITEM(2)
            "1", "1"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(2)
            "2", "1"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(2)
            "3", "2"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(2)
            "1000", "100"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(2)
            "10000", "9999"
        TEST_CASE_ITEM_END
    #if 0
        TEST_CASE_ITEM(2)
            "100000", "88888"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(2)
            "1000000", "500000"
        TEST_CASE_ITEM_END
    #endif
    TEST_SUITE_ITEM_END
TEST_SUITE_TABLE_END

/// -------------------------------------------------
/// AVL ²âÊÔÌ×
/// -------------------------------------------------
IMPLEMENT_REGTESTSUITE_FUNC(AVL)


CTestAvlNode::CTestAvlNode()
{
    m_dwID = 0;
    (void)memset(m_szName, 0, sizeof(m_szName));
}

CTestAvlNode::CTestAvlNode(DWORD dwID)
{
    m_dwID = dwID;
    (void)memset(m_szName, 0, sizeof(m_szName));
}

CTestAvlNode::~CTestAvlNode()
{
}

int CTestAvlNode::CompFunc(void *pNode1, void *pNode2)
{
    CTestAvlNode *pAvlNode1 = (CTestAvlNode *)pNode1;
    CTestAvlNode *pAvlNode2 = (CTestAvlNode *)pNode2;

    if (!pAvlNode1)
    {
        return -1;
    }

    if (!pAvlNode2)
    {
        return -1;
    }

    if (pAvlNode1->ID() < pAvlNode2->ID())
    {
        return -1;
    }

    if (pAvlNode1->ID() == pAvlNode2->ID())
    {
        return 0;
    }

    return 1;
}

void CTestAvlNode::FreeFunc(void *pNode)
{
    CTestAvlNode *pAvlNode = (CTestAvlNode *)pNode;
    if (!pAvlNode)
    {
        return;
    }

    delete pAvlNode;
}

void CTestAvlNode::Set(DWORD dwID, const char *szName)
{
    m_dwID = dwID;
    strncpy(m_szName, szName, AVL_TEST_NAME_LEN);
    m_szName[AVL_TEST_NAME_LEN - 1] = '\0';
}

CTestSuite_AVL::CTestSuite_AVL()
{
    avl_init(&m_avlTree, CTestAvlNode::CompFunc, CTestAvlNode::FreeFunc);
}

CTestSuite_AVL::~CTestSuite_AVL()
{
    avl_reset(&m_avlTree);
}

int CTestSuite_AVL::TestEntry(int argc, char* argv[])
{
    int iRc;

    if ((argc < 2) || (!argv))
    {
        return -1;
    }

    DWORD dwInsertMax = (DWORD)atoi(argv[0]);
    DWORD dwFindValue = (DWORD)atoi(argv[1]);

    iRc = TestAvl(dwInsertMax, dwFindValue);
    if (iRc)
    {
        return iRc;
    }

    iRc = TestMap(dwInsertMax, dwFindValue);
    if (iRc)
    {
        return iRc;
    }
    
    return 0;
}

int CTestSuite_AVL::TestAvl(DWORD dwInsertMax, DWORD dwFindValue)
{
    TRACE_LOG(STR_FORMAT(" -> AVL Insert Max: %d ...", dwInsertMax));
    for (DWORD i = 1; i <= dwInsertMax; ++i)
    {
        CTestAvlNode *pInsertNode = new CTestAvlNode(i);
        AVLnode *pAvlNode = avl_insert(&m_avlTree, pInsertNode);
        if (!pAvlNode)
        {
            TRACE_LOG(STR_FORMAT(" -> AVL Insert %d Fail!", i));
            delete pInsertNode;
            continue;
        }

        char szName[AVL_TEST_NAME_LEN];
        snprintf(szName, AVL_TEST_NAME_LEN - 1, "_%u_", i);
        pInsertNode->Set(i, szName);
        pAvlNode->cdata = pInsertNode;
    }

    TRACE_LOG(" -> AVL Insert End!");

    TRACE_LOG(STR_FORMAT(" -> AVL Find Value: %d ...", dwFindValue));
    CTestAvlNode tmpNode(dwFindValue);
    AVLnode *pFindNode = avl_lookup(&m_avlTree, &tmpNode);
    if (!pFindNode)
    {
        TRACE_LOG(STR_FORMAT(" -> AVL Found %d Fail!", tmpNode.ID()));
        avl_reset(&m_avlTree);
        return -2;
    }

    TRACE_LOG(" -> AVL Find End!");
    CTestAvlNode *pTmpNode = (CTestAvlNode *)(pFindNode->cdata);
    TRACE_LOG(STR_FORMAT(" -> AVL Found %d:%s.", pTmpNode->ID(), pTmpNode->Name()));

    avl_reset(&m_avlTree);
    return 0;
}

int CTestSuite_AVL::TestMap(DWORD dwInsertMax, DWORD dwFindValue)
{
    typedef std::map<DWORD, CTestAvlNode> MAP_TEST;
    typedef MAP_TEST::iterator IT_TEST;

    MAP_TEST map_test;
    TRACE_LOG(STR_FORMAT(" -> MAP Insert Max: %d ...", dwInsertMax));
    for (DWORD i = 1; i <= dwInsertMax; ++i)
    {
        CTestAvlNode insertNode(i);
        (void)map_test.insert(MAP_TEST::value_type(i, insertNode));
    }

    TRACE_LOG(" -> MAP Insert End!");

    TRACE_LOG(STR_FORMAT(" -> MAP Find Value: %d ...", dwFindValue));
    IT_TEST it = map_test.find(dwFindValue);
    if (it == map_test.end())
    {
        TRACE_LOG(STR_FORMAT(" -> MAP Found %d Fail!", dwFindValue));
        avl_reset(&m_avlTree);
        return -3;
    }

    TRACE_LOG(" -> MAP Find End!");
    CTestAvlNode *pTmpNode = &((*it).second);
    TRACE_LOG(STR_FORMAT(" -> MAP Found %d:%s.", pTmpNode->ID(), pTmpNode->Name()));

    return 0;
}

