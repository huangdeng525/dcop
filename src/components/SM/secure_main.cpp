/// -------------------------------------------------
/// secure_main.cpp : 安全管理实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "secure_main.h"
#include "Factory_if.h"
#include "Manager_if.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CSecure, "secure")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CSecure)
    DCOP_IMPLEMENT_INTERFACE(ISecure)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CSecure)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END

/*******************************************************
  函 数 名: CSecure::CSecure
  描    述: CSecure构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CSecure::CSecure(Instance *piParent, int argc, char **argv)
{
    

    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);
}

/*******************************************************
  函 数 名: CSecure::~CSecure
  描    述: CSecure析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CSecure::~CSecure()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CSecure::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSecure::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CSecure::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSecure::Fini()
{
    
}

/*******************************************************
  函 数 名: CSecure::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSecure::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    
}

