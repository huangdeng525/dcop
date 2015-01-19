/// -------------------------------------------------
/// test_mem.h : 主要测试内存操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TEST_MEM_H_
#define _TEST_MEM_H_

#include "test.h"


/// 测试内存申请和释放记录被测类
class CTestMem
{
public:
    CTestMem() : m_iTest(0) {}
    ~CTestMem() {}

    void SetTest(int iTest) { m_iTest = iTest; }
    int GetTest() { return m_iTest; }

private:
    int m_iTest;
};


/// 测试内存申请和释放记录测试套
class CTestSuiteMem : public ITestSuite
{
public:
    CTestSuiteMem();
    ~CTestSuiteMem();

    int TestEntry(int argc, char* argv[]);

};


#endif // #ifndef _TEST_MEM_H_

