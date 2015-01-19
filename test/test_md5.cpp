/// -------------------------------------------------
/// test_md5.cpp : ÷˜“™≤‚ ‘MD5À„∑®ø‚
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test_md5.h"
#include "md5/md5.h"


/// -------------------------------------------------
/// ≤‚ ‘”√¿˝
/// -------------------------------------------------
TEST_SUITE_TABLE(MD5)
    TEST_SUITE_ITEM(CTestSuiteMd5)
        TEST_CASE_ITEM(1)
            "1"
        TEST_CASE_ITEM_END
    TEST_SUITE_ITEM_END
TEST_SUITE_TABLE_END

/// -------------------------------------------------
/// ≤‚ ‘Ã◊
/// -------------------------------------------------
IMPLEMENT_REGTESTSUITE_FUNC(MD5)


CTestSuiteMd5::CTestSuiteMd5()
{
}

CTestSuiteMd5::~CTestSuiteMd5()
{
}

char *CTestSuiteMd5::MD5(const char *szPlainText)
{
    MD5_CTX md5;

    char digest[16];
    (void)memset(digest, 0, 16);
    (void)memset(m_szCipherText, 0, 33);

    MD5Init(&md5);
    MD5Update(&md5, (unsigned  char *)szPlainText, (unsigned int)strlen((char *)szPlainText));
    MD5Final(&md5, (unsigned  char *)digest);

    DWORD dwOffset = 0;
    for(DWORD i = 0; i < 16; i++)
    {
        dwOffset += (DWORD)snprintf(m_szCipherText + dwOffset, 
                        sizeof(m_szCipherText) - dwOffset, 
                        "%02x", 
                        (BYTE)digest[i]);
    }

    printf("MD5(\"%s\") = %s \r\n", szPlainText, m_szCipherText);

    return m_szCipherText;
}

int CTestSuiteMd5::TestEntry(int argc, char* argv[])
{
    int rc = 0;

    TEST_CHECK(0, strcmp(MD5(""), "d41d8cd98f00b204e9800998ecf8427e"), rc);
    TEST_CHECK(0, strcmp(MD5("a"), "0cc175b9c0f1b6a831c399e269772661"), rc);
    TEST_CHECK(0, strcmp(MD5("abc"), "900150983cd24fb0d6963f7d28e17f72"), rc);
    TEST_CHECK(0, strcmp(MD5("admin"), "21232f297a57a5a743894a0e4a801fc3"), rc);
    TEST_CHECK(0, strcmp(MD5("123456"), "e10adc3949ba59abbe56e057f20f883e"), rc);
    TEST_CHECK(0, strcmp(MD5("admin888"), "7fef6171469e80d32c0559f88b377245"), rc);
    TEST_CHECK(0, strcmp(MD5("abcdefghijklmnopqrstuvwxyz"), "c3fcd3d76192e4007dfb496cca67e13b"), rc);
    TEST_CHECK(0, strcmp(MD5("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"), "f29939a25efabaef3b87e2cbfe641315"), rc);

    return rc;
}

