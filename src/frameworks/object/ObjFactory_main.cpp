/// -------------------------------------------------
/// ObjFactory_main.cpp : 类厂实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjFactory_main.h"
#include "string/tablestring.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CClassFactory)
    DCOP_IMPLEMENT_INTERFACE(IFactory)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 自动构造
/// -------------------------------------------------
void vConstructFactoryFunc()
{
}

/// -------------------------------------------------
/// 自动析构
/// -------------------------------------------------
void vReleaseFactoryFunc()
{
    if (CClassFactory::sm_pClassFactory)
    {
        (void)CClassFactory::sm_pClassFactory->Release();
        CClassFactory::sm_pClassFactory = 0;
    }
}

/// -------------------------------------------------
/// 自动编译
/// -------------------------------------------------
CPPBUILDUNIT_AUTO(vConstructFactoryFunc, vReleaseFactoryFunc);

/// -------------------------------------------------
/// 全局类管理器
/// -------------------------------------------------
CClassFactory *CClassFactory::sm_pClassFactory = 0;


/*******************************************************
  函 数 名: IFactory::GetInstance
  描    述: 获取类厂单实例
  输    入: 
  输    出: 
  返    回: 
            IFactory *          - 类工厂指针
  修改记录: 
 *******************************************************/
IFactory *IFactory::GetInstance()
{
    if (!CClassFactory::sm_pClassFactory)
    {
        CClassFactory::sm_pClassFactory = new CClassFactory();
        if (!CClassFactory::sm_pClassFactory)
        {
            /// 一开始到这里失败，内存有问题
            return NULL;
        }
    }

    IFactory *piFactoryRc = NULL;
    if (CClassFactory::sm_pClassFactory->QueryInterface(ID_INTF(IFactory), REF_PTR(piFactoryRc)) != SUCCESS)
    {
        /// 这里失败只返回，单件等退出时自动析构
        return NULL;
    }

    return piFactoryRc;
}

/*******************************************************
  函 数 名: CClassFactory::CClassFactory
  描    述: 类工厂实现类构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CClassFactory::CClassFactory()
{
    m_pLock = DCOP_CreateLock();

    DCOP_CONSTRUCT_INSTANCE(NULL);
}

/*******************************************************
  函 数 名: CClassFactory::~CClassFactory
  描    述: 类工厂实现类析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CClassFactory::~CClassFactory()
{
    {
        AutoObjLock(this);
        m_instances.clear();
    }

    if (m_pLock)
    {
        delete m_pLock;
        m_pLock = 0;
    }

    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CClassFactory::InsertClass
  描    述: 插入一个类到类工厂中
  输    入: 
            cpszClassName       - 类名
            fnConstruct         - 类的构建函数
  输    出: 
  返    回: 
            SUCCESS             - 成功
            FAILURE             - 失败
  修改记录: 
 *******************************************************/
DWORD CClassFactory::InsertClass(const char *cszName,
                        DCOP_CREATE_INSTANCE_FUNC fnConstruct)
{
    if (!cszName)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    std::string strKey = cszName;
    IT_INSTANCES it_ins = m_instances.find(cszName);
    if (it_ins != m_instances.end())
    {
        return FAILURE;
    }

    (void)m_instances.insert(
                MAP_INSTANCES::value_type(
                strKey, fnConstruct));

    return SUCCESS;
}

/*******************************************************
  函 数 名: CClassFactory::CreateInstance
  描    述: 从类工厂中创建一个实例
  输    入: 
            cpszClassName       - 要构建的类的名字
            piParent            - 实例容器(父实例)
            argc                - 类的构建参数个数
            argv                - 类的构建参数列表
  输    出: 
  返    回: 
            Instance *          - 创建的实例
  修改记录: 
 *******************************************************/
Instance *CClassFactory::CreateInstance(const char *cszName, 
                        Instance *piParent, 
                        int argc, 
                        char **argv)
{
    if (!cszName)
    {
        return 0;
    }

    AutoObjLock(this);

    std::string strKey = cszName;
    IT_INSTANCES it_ins = m_instances.find(cszName);
    if (it_ins == m_instances.end())
    {
        return 0;
    }

    Instance *pBaseRc = ((*it_ins).second)(piParent, argc, argv);

    return pBaseRc;
}

/*******************************************************
  函 数 名: CClassFactory::Enter
  描    述: 进入类工厂临界区
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CClassFactory::Enter()
{
    if (m_pLock)
    {
        m_pLock->Enter();
    }
}

/*******************************************************
  函 数 名: CClassFactory::Leave
  描    述: 退出类工厂临界区
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CClassFactory::Leave()
{
    if (m_pLock)
    {
        m_pLock->Leave();
    }
}

/*******************************************************
  函 数 名: CClassFactory::Dump
  描    述: Dump入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CClassFactory::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    if (!logPrint) return;

    AutoObjLock(this);
    logPrint("-----------------------------------------------\r\n", logPara);
    logPrint("Factory Dump: \r\n", logPara);

    CTableString tableStr(2, (DWORD)m_instances.size(), "  ");

    for (IT_INSTANCES it_ins = m_instances.begin();
        it_ins != m_instances.end(); ++it_ins)
    {
        tableStr << STR_FORMAT("insName:'%s'", ((*it_ins).first).c_str());
        tableStr << STR_FORMAT("insFunc:'%p'", (*it_ins).second);
    }

    tableStr.Show(logPrint, logPara);

    logPrint(STR_FORMAT("Classes Count: %d\r\n", m_instances.size()), logPara);
    logPrint("-----------------------------------------------\r\n", logPara);
}

