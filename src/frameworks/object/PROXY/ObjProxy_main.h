/// -------------------------------------------------
/// ObjProxy_main.h : 代理对象私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJPROXY_MAIN_H_
#define _OBJPROXY_MAIN_H_

#include "ObjProxy_if.h"


class CProxy : public IProxy
{
public:
    CProxy(Instance *piParent, int argc, char **argv);
    ~CProxy();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();
    void Proc(objMsg *msg);
    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

private:
};


#endif // #ifndef _OBJPROXY_MAIN_H_

