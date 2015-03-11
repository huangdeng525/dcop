/// -------------------------------------------------
/// luax_main.h : lua扩展私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _LUAX_MAIN_H_
#define _LUAX_MAIN_H_

#include "lua.hpp"
#include "luax_if.h"


class CLuaX : public ILuaX
{
public:
    CLuaX(Instance *piParent, int argc, char **argv);
    ~CLuaX();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();
    void Proc(objMsg *msg);
    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

private:
    void LuaInit();
    void LuaFini();
    void LuaError(LOG_PRINT logPrint, LOG_PARA logPara, int errNo);

private:
    lua_State *m_L;
};


#endif // #ifndef _LUAX_MAIN_H_

