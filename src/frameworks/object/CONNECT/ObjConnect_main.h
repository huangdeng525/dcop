/// -------------------------------------------------
/// ObjConnect_main.h : 连接器对象私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJCONNECT_MAIN_H_
#define _OBJCONNECT_MAIN_H_

#include "ObjConnect_if.h"


class CConnect : public IConnect
{
public:
    CConnect(Instance *piParent, int argc, char **argv);
    ~CConnect();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();
    void Proc(objMsg *msg);
    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

private:
};


#endif // #ifndef _OBJCONNECT_MAIN_H_

