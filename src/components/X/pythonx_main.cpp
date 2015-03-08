/// -------------------------------------------------
/// pythonx_main.cpp : python扩展实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "pythonx_main.h"
#include "Factory_if.h"
#include "Manager_if.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CPythonX, "pythonx")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CPythonX)
    DCOP_IMPLEMENT_INTERFACE(IPythonX)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CPythonX)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END


/*******************************************************
  函 数 名: CPythonX::CPythonX
  描    述: CPythonX构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CPythonX::CPythonX(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);
}

/*******************************************************
  函 数 名: CPythonX::~CPythonX
  描    述: CPythonX析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CPythonX::~CPythonX()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CPythonX::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CPythonX::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    PythonInit();

    return SUCCESS;
}

/*******************************************************
  函 数 名: CPythonX::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CPythonX::Fini()
{
    PythonFini();
}

/*******************************************************
  函 数 名: CPythonX::Proc
  描    述: 消息处理
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CPythonX::Proc(objMsg *msg)
{
    if (!msg)
    {
        return;
    }
}

/*******************************************************
  函 数 名: CPythonX::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CPythonX::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
}

/*******************************************************
  函 数 名: CPythonX::PythonInit
  描    述: Python初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CPythonX::PythonInit()
{
    /// Python解释器的初始化
    Py_Initialize();

    CHECK_ERRCODE(Py_IsInitialized()? SUCCESS : FAILURE, "Init Python Interpreter");
}

/*******************************************************
  函 数 名: CPythonX::PythonFini
  描    述: Python完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CPythonX::PythonFini()
{
    /// 结束Python解释器
    Py_Finalize();
}

