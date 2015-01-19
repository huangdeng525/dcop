/// -------------------------------------------------
/// test_md5.h : ÷˜“™≤‚ ‘MD5À„∑®ø‚
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TEST_MD5_H_
#define _TEST_MD5_H_

#include "test.h"


/// ≤‚ ‘md5
class CTestSuiteMd5 : public ITestSuite
{
public:
    CTestSuiteMd5();
    ~CTestSuiteMd5();

    char *MD5(const char *szPlainText);

    int TestEntry(int argc, char* argv[]);

private:
    char m_szCipherText[33];
};


#endif // #ifndef _TEST_MD5_H_

