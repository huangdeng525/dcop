/// -------------------------------------------------
/// pythonx_main.h : python扩展私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _PYTHONX_MAIN_H_
#define _PYTHONX_MAIN_H_

#include "pythonx_if.h"
#include "Python.h"


class CPythonX : public IPythonX
{
public:
    CPythonX(Instance *piParent, int argc, char **argv);
    ~CPythonX();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();
    void Proc(objMsg *msg);
    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

private:
    void PythonInit();
    void PythonFini();
};


#endif // #ifndef _PYTHONX_MAIN_H_


