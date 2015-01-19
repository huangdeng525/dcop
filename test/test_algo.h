/// -------------------------------------------------
/// test_algo.h : 主要测试算法操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TEST_ALGO_H_
#define _TEST_ALGO_H_

#include "test.h"


/// 算法测试套
class CTestSuiteAlgo : public ITestSuite
{
public:
    CTestSuiteAlgo();
    ~CTestSuiteAlgo();

    int TestEntry(int argc, char* argv[]);

private:
    int BinarySearchEntry();
    int BaseNCase(unsigned int N, unsigned int num);
    int BaseNEntry();
};


#endif // #ifndef _TEST_ALGO_H_

