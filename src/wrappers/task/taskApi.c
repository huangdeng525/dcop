/// -------------------------------------------------
/// taskApi.c : 操作系统task实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "taskApi.h"
#include <assert.h>


TaskFuncs g_TaskFuncs;


void vSetTaskFuncs(const TaskFuncs* pFuncs)
{
    if (pFuncs)
    {
        g_TaskFuncs = *pFuncs;
    }
    else
    {
        (void)memset((void *)&g_TaskFuncs, 0, sizeof(TaskFuncs));
    }
}

void DCOP_Assert(int x, const char *file, int line)
{
    if (!x)
    {
        CheckErrCodeEx(x, !x, "assert", file, line);
        assert(x);
    }
}

void DCOP_Reset(int type, const char *file, int line)
{
    const char *pFileName = strrchr(file, '\\');
    if (!pFileName) pFileName = strrchr(file, '/');
    if (!pFileName) pFileName = file;
    else ++pFileName;
    printf(" ****** System Reset! ****** \n");
    printf(" called reboot on '%s:%d'\n", pFileName, line);

    assert(0);
    exit(type);
}

