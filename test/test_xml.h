/// -------------------------------------------------
/// test_xml.h : 主要测试xml操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TEST_XML_H_
#define _TEST_XML_H_

#include "test.h"


const char * const txtxml[] = 
{
"\
<?xml version=\"1.0\"?>\
<Hello>World</Hello>\
",
"\
<?xml version=\"1.0\"?>\
<scene name=\"Depth\">\
    <node type=\"camera\">\
        <eye>0 10 10</eye>\
        <front>0 0 -1</front>\
        <refUp>0 1 0</refUp>\
        <fov>90</fov>\
    </node>\
    <node type=\"Sphere\">\
        <center>0 10 -10</center>\
        <radius>10</radius>\
    </node>\
    <node type=\"Plane\">\
        <direction>0 10 -10</direction>\
        <distance>10</distance>\
    </node>\
</scene>\
"
};

/// 测试xml操作
class CTestSuite_XML : public ITestSuite
{
public:
    CTestSuite_XML();
    ~CTestSuite_XML();

    int TestEntry(int argc, char* argv[]);

    static bool example1();
    static bool example2();

private:
    bool CreateFile(int iTestNo);
    void DeleteFile();
};


#endif // #ifndef _TEST_XML_H_

