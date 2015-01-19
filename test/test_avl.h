/// -------------------------------------------------
/// test_avl.h : 主要测试avl操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TEST_AVL_H_
#define _TEST_AVL_H_

#define INC_MAP

#include "test.h"
#include "tree/avl.h"


#define AVL_TEST_NAME_LEN 24

class CTestAvlNode
{
public:
    CTestAvlNode();
    CTestAvlNode(DWORD dwID);
    ~CTestAvlNode();

    static int CompFunc(void *pNode1, void *pNode2);
    static void FreeFunc(void *pNode);

    void Set(DWORD dwID, const char *szName);

    DWORD ID() const {return m_dwID;}
    const char *Name() const {return m_szName;}

private:
    DWORD m_dwID;
    char m_szName[AVL_TEST_NAME_LEN];
};


/// 测试avl操作
class CTestSuite_AVL : public ITestSuite
{
public:
    CTestSuite_AVL();
    ~CTestSuite_AVL();

    int TestEntry(int argc, char* argv[]);

private:
    int TestAvl(DWORD dwInsertMax, DWORD dwFindValue);
    int TestMap(DWORD dwInsertMax, DWORD dwFindValue);

private:
    AVLtree m_avlTree;
};


#endif // #ifndef _TEST_AVL_H_

