/// -------------------------------------------------
/// output.h : 输出处理公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_STREAM_OUTPUT_H_
#define _TOOL_STREAM_OUTPUT_H_

#include "type.h"


class COutput
{
public:
    typedef void (*PRINT)(const char *format, ...);

public:
    COutput(PRINT fnPrint = 0);
    ~COutput();

private:
    PRINT m_fnPrintFunc;
    char *m_szPrintBuf;
};


#endif // #ifndef _TOOL_STREAM_OUTPUT_H_

