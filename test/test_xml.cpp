/// -------------------------------------------------
/// test_xml.cpp : Ö÷Òª²âÊÔxml²Ù×÷
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include <iostream>
#include "test_xml.h"
#include "xml/xml.h"


/// -------------------------------------------------
/// XML ²âÊÔÓÃÀý
/// -------------------------------------------------
TEST_SUITE_TABLE(XML)
    TEST_SUITE_ITEM(CTestSuite_XML)
        TEST_CASE_ITEM(1)
            "1"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(1)
            "2"
        TEST_CASE_ITEM_END
    TEST_SUITE_ITEM_END
TEST_SUITE_TABLE_END

/// -------------------------------------------------
/// XML ²âÊÔÌ×
/// -------------------------------------------------
IMPLEMENT_REGTESTSUITE_FUNC(XML)


typedef bool (*XML_RESULT)();
const XML_RESULT funcxml[] = 
{
    CTestSuite_XML::example1,
    CTestSuite_XML::example2,
};

CTestSuite_XML::CTestSuite_XML()
{
    
}

CTestSuite_XML::~CTestSuite_XML()
{
    
}

int CTestSuite_XML::TestEntry(int argc, char* argv[])
{
    if ((argc < 1) || (!argv))
    {
        return -1;
    }

    int iTestNo = atoi(argv[0]);
    if ((iTestNo <= 0) || ((DWORD)iTestNo > (sizeof(txtxml)/sizeof(char *))))
    {
        return -2;
    }

    if (!CreateFile(iTestNo))
    {
        return -3;
    }

    if (!((funcxml[iTestNo - 1])()))
    {
        return -4;
    }

    DeleteFile();
    return 0;
}

bool CTestSuite_XML::example1()
{
    XMLDocument doc;
    doc.LoadFile("test.xml");
    const char* content= doc.FirstChildElement( "Hello" )->GetText();
    if ( !strcmp(content, "World") )
    {
        return true;
    }

    return false;
}

bool CTestSuite_XML::example2()  
{
    bool bRc = false;
    XMLDocument doc;
    doc.LoadFile("test.xml");
    XMLElement *scene=doc.RootElement();
    XMLElement *surface=scene->FirstChildElement("node");
    while (surface)
    {
        XMLElement *surfaceChild=surface->FirstChildElement();
        const char* content;
        const XMLAttribute *attributeOfSurface = surface->FirstAttribute();  
        std::cout << attributeOfSurface->Name() << ":" << attributeOfSurface->Value() << std::endl;
        if ( !strcmp(attributeOfSurface->Name(), "type") )
        {
            bRc = true;
        }
        while (surfaceChild)
        {
            content = surfaceChild->GetText();
            surfaceChild = surfaceChild->NextSiblingElement();
            std::cout << content << std::endl;
        }
        surface = surface->NextSiblingElement();
    }

    return bRc;
}

bool CTestSuite_XML::CreateFile(int iTestNo)
{
    FILE *fp = fopen("test.xml", "w");
    if (!fp)
    {
        return false;
    }

    fprintf(fp, "%s", txtxml[iTestNo - 1]);
    fclose(fp);

    return true;
}

void CTestSuite_XML::DeleteFile()
{
    (void)remove("test.xml");
}

