/// -------------------------------------------------
/// luax_main.cpp : lua扩展实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "luax_main.h"
#include "Factory_if.h"
#include "Manager_if.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CLuaX, "luax")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CLuaX)
    DCOP_IMPLEMENT_INTERFACE(ILuaX)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CLuaX)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END


/*******************************************************
  函 数 名: CLuaX::CLuaX
  描    述: CLuaX构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CLuaX::CLuaX(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);

    m_L = NULL;
}

/*******************************************************
  函 数 名: CLuaX::~CLuaX
  描    述: CLuaX析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CLuaX::~CLuaX()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CLuaX::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CLuaX::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    LuaInit();

    return SUCCESS;
}

/*******************************************************
  函 数 名: CLuaX::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLuaX::Fini()
{
    LuaFini();
}

/*******************************************************
  函 数 名: CLuaX::Proc
  描    述: 消息处理
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLuaX::Proc(objMsg *msg)
{
    if (!msg)
    {
        return;
    }
}

/*******************************************************
  函 数 名: CLuaX::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLuaX::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    if (!logPrint) return;

    if (!m_L)
    {
        logPrint("[Lua Dump] No Lua Interpreter! \r\n", logPara);
        return;
    }

    if ((argc <= 2) || !argv)
    {
        logPrint("[Lua Dump] No Lua File! \r\n", logPara);
        return;
    }

    /// 运行lua脚本
    int rc = luaL_dofile(m_L, (char *)(argv[argc - 1]));
    if (rc) LuaError(logPrint, logPara, rc);
}

/*******************************************************
  函 数 名: CLuaX::LuaInit
  描    述: Lua初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLuaX::LuaInit()
{
    /// Lua解释器的初始化
    m_L = luaL_newstate();
    if (!m_L)
    {
        CHECK_ERRCODE(FAILURE, "Init Lua Interpreter");
        return;
    }

    luaL_openlibs(m_L);
}

/*******************************************************
  函 数 名: CLuaX::LuaFini
  描    述: Lua完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLuaX::LuaFini()
{
    /// 结束Lua解释器
    if (m_L)
    {
        lua_close(m_L);
        m_L = NULL;
    }
}

/*******************************************************
  函 数 名: CLuaX::LuaError
  描    述: Lua错误信息获取
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLuaX::LuaError(LOG_PRINT logPrint, LOG_PARA logPara, int errNo)
{
    if (!m_L || !logPrint || !errNo) return;

    switch (errNo)
    {
        case LUA_ERRSYNTAX:     // 编译时错误
            logPrint("[syntax error during pre-compilation] \r\n", logPara);
            break;
        case LUA_ERRMEM:        // 内存错误
            logPrint("[memory allocation error] \r\n", logPara);
            break;
        case LUA_ERRRUN:        // 运行时错误
            logPrint("[a runtime error] \r\n", logPara);
            break;
        case LUA_YIELD:         // 线程被挂起错误
            logPrint("[Thread has Suspended] \r\n", logPara);
            break;
        case LUA_ERRERR:        // 在进行错误处理时发生错误
            logPrint("[error while running the error handler function] \r\n", logPara);
            break;
        default:
            break;
    }

    /// 打印错误结果
    const char *errInfo = lua_tostring(m_L, -1);
    if (errInfo)
    {
        logPrint(errInfo, logPara);
        logPrint("\r\n", logPara);
    }
    lua_pop(m_L, 1);
}

