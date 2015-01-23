/// -------------------------------------------------
/// ObjControl_main.cpp : 控制器对象实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjControl_main.h"
#include "Factory_if.h"
#include "Manager_if.h"
#include "string/tablestring.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CControl, "control")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CControl)
    DCOP_IMPLEMENT_INTERFACE(IControl)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CControl)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END


/*******************************************************
  函 数 名: CControl::CControl
  描    述: CControl构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CControl::CControl(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);
}

/*******************************************************
  函 数 名: CControl::~CControl
  描    述: CControl析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CControl::~CControl()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CControl::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CControl::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CControl::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CControl::Fini()
{
}

/*******************************************************
  函 数 名: CControl::Proc
  描    述: 消息处理
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CControl::Proc(objMsg *msg)
{
    if (!msg)
    {
        return;
    }
}

/*******************************************************
  函 数 名: CControl::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CControl::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    if (!logPrint) return;

    AutoObjLock(this);

    for (IT_CHAIN it = m_chains.begin(); it != m_chains.end(); ++it)
    {
        ((*it).second).Dump(logPrint, logPara, argc, argv);
    }
}

/*******************************************************
  函 数 名: CControl::CreateChain
  描    述: 创建控制链
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IControl::IChain *CControl::CreateChain(IObject *ctrlee)
{
    if (!ctrlee) return NULL;

    AutoObjLock(this);

    IT_CHAIN it = m_chains.find(ctrlee->ID());
    if (it != m_chains.end())
    {
        return &((*it).second);
    }

    CControlChain chain;
    it = m_chains.insert(m_chains.end(), MAP_CHAIN::value_type(ctrlee->ID(), chain));
    if (it == m_chains.end())
    {
        return NULL;
    }

    CControlChain *pChain = &((*it).second);
    pChain->SetCtrlee(ctrlee);

    return pChain;
}

/*******************************************************
  函 数 名: CControl::DestroyChain
  描    述: 删除控制链
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CControl::DestroyChain(IChain *chain)
{
    if (!chain) return;

    AutoObjLock(this);

    CControlChain *pChain = (CControlChain *)chain;
    IObject *piCtrlee = pChain->GetCtrlee();
    if (!piCtrlee)
    {
        return;
    }

    (void)m_chains.erase(piCtrlee->ID());
}

/*******************************************************
  函 数 名: CControl::RegCtrlNode
  描    述: 注册控制点
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CControl::RegCtrlNode(IObject *ctrler,
                    DWORD ctrlee,
                    Node *ctrls,
                    DWORD count)
{
    AutoObjLock(this);

    IT_CHAIN it = m_chains.find(ctrlee);
    if (it == m_chains.end())
    {
        return FAILURE;
    }

    CControlChain *pChain = &((*it).second);
    return pChain->RegCtrlNode(ctrler, ctrls, count);
}

