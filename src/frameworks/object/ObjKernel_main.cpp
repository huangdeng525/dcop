/// -------------------------------------------------
/// ObjKernel_main.cpp : object框架核心类实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjKernel_main.h"
#include "Factory_if.h"
#include "BaseID.h"


/// -------------------------------------------------
/// 全局内核资源和实例
/// -------------------------------------------------
CFrameKernel    CFrameKernel::sm_instance;
void          (*g_onInstanceQueryInterface)(
                        Instance *piThis, 
                        Instance *piRefer, 
                        void *pPara) = 0;
void *          g_onInstanceQueryInterfacePara = 0;
void          (*g_onInstanceRelease)(
                        Instance *piThis, 
                        Instance *piRefer, 
                        void *pPara) = 0;
void *          g_onInstanceReleasePara = 0;
void          (*g_onInstanceGetRef)(
                        Instance *piThis, 
                        Instance ***pppiRefers, 
                        DWORD *pdwReferCount, 
                        void *pPara) = 0;
void *          g_onInstanceGetRefPara = 0;


/*******************************************************
  函 数 名: CFrameKernel::CReferNode::OnReferto
  描    述: 被引用时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CFrameKernel::CReferNode::OnReferto(Instance *refer)
{
    IT_COUNT it = m_count.find(refer);
    if (it != m_count.end())
    {
        ((*it).second)++;
        return;
    }

    it = m_count.insert(m_count.end(), MAP_COUNT::value_type(refer, 1));
    if (it == m_count.end())
    {
        return;
    }

    /// 走到这里，表示是新添加了引用，则重新整理数组
    m_refer.Clear();
    m_refcnt.Clear();
    if (!m_count.size())
    {
        return;
    }

    (void)m_refer.Set(m_count.size() - 1, 0);
    (void)m_refcnt.Set(m_count.size() - 1, 0);
    for (it = m_count.begin(); it != m_count.end(); ++it)
    {
        (void)m_refer.Append((void *)&((*it).first));
        (void)m_refcnt.Append((void *)&((*it).second));
    }
}

/*******************************************************
  函 数 名: CFrameKernel::CReferNode::OnRelease
  描    述: 被释放时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CFrameKernel::CReferNode::OnRelease(Instance *refer)
{
    IT_COUNT it = m_count.find(refer);
    if (it == m_count.end())
    {
        return;
    }

    if ((*it).second)
    {
        ((*it).second)--;
    }

    if ((*it).second)
    {
        return;
    }

    (void)m_count.erase(it);

    /// 走到这里，表示是新删除了引用，则重新整理数组
    m_refer.Clear();
    m_refcnt.Clear();
    if (!m_count.size())
    {
        return;
    }

    (void)m_refer.Set(m_count.size() - 1, 0);
    (void)m_refcnt.Set(m_count.size() - 1, 0);
    for (it = m_count.begin(); it != m_count.end(); ++it)
    {
        (void)m_refer.Append((void *)&((*it).first));
        (void)m_refcnt.Append((void *)&((*it).second));
    }
}

/*******************************************************
  函 数 名: CFrameKernel::CReferNode::OnGetRefer
  描    述: 获取引用时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CFrameKernel::CReferNode::OnGetRefer(Instance ***refers, DWORD *count)
{
    if (!refers || !count)
    {
        return;
    }

    *refers = (Instance **)m_refer.Get();
    *count  = m_refer.Count();
}

/*******************************************************
  函 数 名: CFrameKernel::CReferNode::OnGetReferCount
  描    述: 获取引用计数时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CFrameKernel::CReferNode::OnGetReferCount(DWORD **refercount, DWORD *count)
{
    if (!refercount || !count)
    {
        return;
    }

    *refercount = (DWORD *)m_refcnt.Get();
    *count      = m_refcnt.Count();
}

/*******************************************************
  函 数 名: objBase::GetInstance
  描    述: 获取内核实例
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objBase *objBase::GetInstance()
{
    return &CFrameKernel::sm_instance;
}

/*******************************************************
  函 数 名: objBase::~objBase
  描    述: 虚构实现
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objBase::~objBase()
{
}

/*******************************************************
  函 数 名: CFrameKernel::CFrameKernel
  描    述: CFrameKernel构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CFrameKernel::CFrameKernel()
{
    m_pLock = DCOP_CreateLock();
    m_pTask = 0;

    g_onInstanceQueryInterface          = OnInstanceQueryInterface;
    g_onInstanceQueryInterfacePara      = this;
    g_onInstanceRelease                 = OnInstanceRelease;
    g_onInstanceReleasePara             = this;
    g_onInstanceGetRef                  = OnInstanceGetRef;
    g_onInstanceGetRefPara              = this;
}

/*******************************************************
  函 数 名: CFrameKernel::~CFrameKernel
  描    述: CFrameKernel析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CFrameKernel::~CFrameKernel()
{
    if (m_pLock)
    {
        delete m_pLock;
        m_pLock = 0;
    }

    if (m_pTask)
    {
        delete m_pTask;
        m_pTask = 0;
    }

    g_onInstanceQueryInterface          = 0;
    g_onInstanceQueryInterfacePara      = 0;
    g_onInstanceRelease                 = 0;
    g_onInstanceReleasePara             = 0;
    g_onInstanceGetRef                  = 0;
    g_onInstanceGetRefPara              = 0;
}

/*******************************************************
  函 数 名: CFrameKernel::Enter
  描    述: 实现进入对象基类的全局临界区
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CFrameKernel::Enter()
{
    if (m_pLock)
    {
        m_pLock->Enter();
    }
}

/*******************************************************
  函 数 名: CFrameKernel::Leave
  描    述: 实现退出对象基类的全局临界区
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CFrameKernel::Leave()
{
    if (m_pLock)
    {
        m_pLock->Leave();
    }
}

/*******************************************************
  函 数 名: CFrameKernel::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CFrameKernel::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    if (!logPrint) return;

    AutoObjLock(this);

    for (IT_REFERS it = m_refers.begin(); it != m_refers.end(); ++it)
    {
        Instance *piThis = ((*it).first);
        if (!piThis)
        {
            continue;
        }

        logPrint(STR_FORMAT("'%s'(id:%d)[inst:%p] Be Refered(count:%d) By: \r\n", 
                        piThis->Name(), piThis->ID(), piThis, piThis->GetRef()), logPara);

        Instance **ppiRefers = 0;
        DWORD dwReferCount = 0;
        ((*it).second).OnGetRefer(&ppiRefers, &dwReferCount);
        if (!ppiRefers || !dwReferCount)
        {
            logPrint("    No Refered! \r\n", logPara);
            continue;
        }

        DWORD *pdwReferCount = 0;
        DWORD dwCounterCount = 0;
        ((*it).second).OnGetReferCount(&pdwReferCount, &dwCounterCount);

        for (DWORD i = 0; i < dwReferCount; ++i)
        {
            Instance *piRefer = ppiRefers[i];
            if (!piRefer)
            {
                continue;
            }

            logPrint(STR_FORMAT("    '%s'(id:%d)[inst:%p](count:%d) \r\n", 
                        piRefer->Name(), piRefer->ID(), piRefer, 
                        (i < dwCounterCount)? pdwReferCount[i] : 0), logPara);
        }
    }

    osBase::Dump(logPrint, logPara, argc, argv);
}

/*******************************************************
  函 数 名: CFrameKernel::Start
  描    述: 整个应用实例的入口
  输    入: cfgDeploy   - 输入的部署配置文件
  输    出: 
  返    回: 成功或者失败的错误码
  修改记录: 
 *******************************************************/
objBase *CFrameKernel::Start(const char *cfgDeploy)
{
    if (!m_pTask) m_pTask = DCOP_CreateTask("EntryTask", NULL, 0, 0, 0);

    /////////////////////////////////////////////////
    /// 加载管理器内所有对象
    /////////////////////////////////////////////////
    IManager *piManager = LoadAllObjects(cfgDeploy);
    if (!piManager)
    {
        return NULL;
    }

    /////////////////////////////////////////////////
    /// 初始化管理器内所有对象
    /////////////////////////////////////////////////
    DWORD dwRc = piManager->Init(NULL, 0, 0);
    if (dwRc)
    {
        CHECK_RETCODE(dwRc, STR_FORMAT("System(%d) InitAllObjects Fail(0x%x)!", 
                piManager->GetSystemID(), dwRc));
        (void)piManager->Release();
        return NULL;
    }

    TRACE_LOG(STR_FORMAT("System(%d) InitAllObjects OK!", piManager->GetSystemID()));
    piManager->Dump(PrintToConsole, 0, 0, 0);
    Dump(PrintToConsole, 0, 0, 0);

    return piManager;
}

/*******************************************************
  函 数 名: CFrameKernel::End
  描    述: 整个应用实例的出口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CFrameKernel::End(objBase *pBase)
{
    /////////////////////////////////////////////////
    /// 结束管理器内所有对象
    /////////////////////////////////////////////////
    IManager *piManager = (IManager *)pBase;
    if (piManager)
    {
        piManager->Fini();
        DCOP_RELEASE_INSTANCE_REFER(0, piManager);
    }
}

/*******************************************************
  函 数 名: CFrameKernel::LoadAllObjects
  描    述: 加载配置文件中的所有对象
  输    入: xmlFile     - 输入的xml配置文件
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IManager *CFrameKernel::LoadAllObjects(const char *xmlFile)
{
    DWORD dwRc;
    XMLDocument doc;

    /////////////////////////////////////////////////
    /// 加载xml文件
    /////////////////////////////////////////////////
    dwRc = (DWORD)doc.LoadFile(xmlFile);
    if (dwRc)
    {
        CHECK_RETCODE(dwRc, STR_FORMAT("System('%s') LoadFile Fail!", xmlFile));
        return NULL;
    }

    IFactory *piFactory = IFactory::GetInstance();
    if (piFactory)
    {
        piFactory->Dump(PrintToConsole, 0, 0, 0);
        (void)piFactory->Release();
    }

    /////////////////////////////////////////////////
    /// 获取Xml根节点
    /////////////////////////////////////////////////
    XMLElement *pXmlSystem = doc.RootElement();
    if (!pXmlSystem)
    {
        CHECK_RETCODE(FAILURE, STR_FORMAT("System('%s') LoadFile Fail!", xmlFile));
        return NULL;
    }

    TRACE_LOG(STR_FORMAT("System('%s') LoadFile OK!", xmlFile));

    /////////////////////////////////////////////////
    /// 设置系统ID到任务变量中
    /////////////////////////////////////////////////
    objTask *pTask = objTask::Current();
    if (pTask)
    {
        DWORD dwSysID = 0;
        const char *sysID = pXmlSystem->Attribute("id");
        dwSysID = (sysID)? (DWORD)atoi(sysID) : 0;
        dwRc = pTask->SetLocal(TASK_LOCAL_SYSTEM, &dwSysID, sizeof(dwSysID));
        CHECK_ERRCODE(dwRc, "Set Sys ID To Task Local");
        DWORD dwObjID = DCOP_OBJECT_KERNEL;
        dwRc = pTask->SetLocal(TASK_LOCAL_HANDLER, &dwObjID, sizeof(dwObjID));
        CHECK_ERRCODE(dwRc, "Set Obj ID To Task Local");
    }

    /////////////////////////////////////////////////
    /// 实例化对象管理器
    /////////////////////////////////////////////////
    CDArray szArgs(DCOP_STRING_ARG_LEM_MAX, DCOP_SYSTEM_ARG_MAX_COUNT);
    char *argv[DCOP_SYSTEM_ARG_MAX_COUNT];
    DWORD argc = GetXmlAttribute(pXmlSystem, szArgs);
    GetArgList(argc, argv, szArgs);
    IManager *piManager = NULL;
    DCOP_CREATE_INSTANCE(IManager, "manager", NULL, argc, argv, piManager);
    if (!piManager)
    {
        CHECK_RETCODE(FAILURE, STR_FORMAT("System('%s') CreateManager Fail!", xmlFile));
        return NULL;
    }

    TRACE_LOG(STR_FORMAT("System('%s') CreateManager OK!", xmlFile));

    /////////////////////////////////////////////////
    /// 创建管理器下面所有对象
    /////////////////////////////////////////////////
    dwRc = CreateAllObjects(piManager, pXmlSystem);
    if (dwRc)
    {
        CHECK_RETCODE(dwRc, STR_FORMAT("System(%d) CreateAllObjects Fail(0x%x)!", 
                piManager->GetSystemID(), dwRc));
        (void)piManager->Release();
        return NULL;
    }

    TRACE_LOG(STR_FORMAT("System(%d) CreateAllObjects OK!", piManager->GetSystemID()));

    if (pTask)
    {
        DWORD dwObjID = DCOP_OBJECT_KERNEL;
        dwRc = pTask->SetLocal(TASK_LOCAL_HANDLER, &dwObjID, sizeof(dwObjID));
        CHECK_ERRCODE(dwRc, "Set Obj ID To Task Local");
    }

    return piManager;
}

/*******************************************************
  函 数 名: CFrameKernel::Load
  描    述: 加载单个对象
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IObject *CFrameKernel::Load(const char *cszName, IManager *piManager, int argc, char **argv)
{
    IObject *piObject = 0;
    DCOP_CREATE_INSTANCE(IObject, cszName, piManager, argc, argv, piObject);
    if (!piObject)
    {
        CHECK_RETCODE(FAILURE, STR_FORMAT("Create Object: '%s' Fail!", cszName));
        return NULL;
    }

    DWORD dwRc = piManager->InsertObject(piObject);
    if (dwRc != SUCCESS)
    {
        CHECK_RETCODE(dwRc, STR_FORMAT("Insert Object: '%s'|%d Fail(0x%x)!", 
                        piObject->Name(), piObject->ID(), dwRc));
        DCOP_RELEASE_INSTANCE_REFER(piManager, piObject);
        return NULL;
    }

    return piObject;
}

/*******************************************************
  函 数 名: CFrameKernel::DynamicLoad
  描    述: 加载动态库
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objDynamicLoader *CFrameKernel::DynamicLoad(const char *dllFile)
{
    objDynamicLoader *pLoader = DCOP_CreateDynamicLoader();
    if (!pLoader)
    {
        return 0;
    }

    DWORD dwRc = pLoader->Load(dllFile);
    if (dwRc != SUCCESS)
    {
        delete pLoader;
        return 0;
    }

    return pLoader;
}

/*******************************************************
  函 数 名: CFrameKernel::AddRefer
  描    述: 添加引用
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CFrameKernel::AddRefer(Instance *piThis, Instance *piRefer, objDynamicLoader *pLoader)
{
    if (!piThis)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    IT_REFERS it = m_refers.find(piThis);
    if (it == m_refers.end())
    {
        CReferNode refNode;
        it = m_refers.insert(m_refers.end(), MAP_REFERS::value_type(piThis, refNode));
        if (it == m_refers.end())
        {
            return FAILURE;
        }
    }

    if (pLoader)
    {
        ((*it).second).SetLoader(pLoader);
    }
    else
    {
        ((*it).second).OnReferto(piRefer);
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CFrameKernel::DelRefer
  描    述: 删除引用
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CFrameKernel::DelRefer(Instance *piThis, Instance *piRefer)
{
    if (!piThis)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    IT_REFERS it = m_refers.find(piThis);
    if (it == m_refers.end())
    {
        return FAILURE;
    }

    ((*it).second).OnReferto(piRefer);

    if (!piThis->GetRef())
    {
        (void)m_refers.erase(it);
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CFrameKernel::GetRefer
  描    述: 获取引用
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CFrameKernel::GetRefer(Instance *piThis, Instance ***pppiRefers)
{
    if (!piThis)
    {
        return 0;
    }

    AutoObjLock(this);

    IT_REFERS it = m_refers.find(piThis);
    if (it == m_refers.end())
    {
        return 0;
    }

    DWORD dwCount = 0;
    ((*it).second).OnGetRefer(pppiRefers, &dwCount);

    return dwCount;
}

/*******************************************************
  函 数 名: CFrameKernel::GetXmlAttribute
  描    述: CFrameKernel获取XML元素属性值
  输    入: pXMLElement - XML元素
  输    出: rArgs       - 参数数组
  返    回: 配置项的数量
  修改记录: 
 *******************************************************/
DWORD CFrameKernel::GetXmlAttribute(const XMLElement *pXMLElement, CDArray &rArgs)
{
    DWORD dwArgCount = rArgs.Count();
    const XMLAttribute *pXMLAttribute = pXMLElement->FirstAttribute();
    while (pXMLAttribute)
    {
        char szConfig[DCOP_STRING_ARG_LEM_MAX];
        char szValue[DCOP_STRING_ARG_LEM_MAX];

        /// 配置项名称(为空就不配置了)
        const char *pszName = pXMLAttribute->Name();
        if (!pszName || !(*pszName))
        {
            pXMLAttribute = pXMLAttribute->Next();
            continue;
        }

        (void)snprintf(szConfig, sizeof(szConfig), "-%s", pszName);
        szConfig[sizeof(szConfig) - 1] = '\0';
        if (rArgs.Append(szConfig) != SUCCESS)
        {
            pXMLAttribute = pXMLAttribute->Next();
            continue;
        }

        /// 配置项值(为空就视配置项为开关: 只有名称，而没有值)
        const char *pszValue = pXMLAttribute->Value();
        if (!pszValue || !(*pszValue))
        {
            pXMLAttribute = pXMLAttribute->Next();
            continue;
        }

        (void)snprintf(szValue, sizeof(szValue), "%s", pszValue);
        szValue[sizeof(szValue) - 1] = '\0';
        (void)rArgs.Append(szValue);

        pXMLAttribute = pXMLAttribute->Next();
    }

    if (rArgs.Count() < dwArgCount)
    {
        dwArgCount = 0;
    }
    else
    {
        dwArgCount = rArgs.Count() - dwArgCount;
    }

    return dwArgCount;
}

/*******************************************************
  函 数 名: CFrameKernel::GetXmlChildValue
  描    述: CFrameKernel获取XML元素下的子元素值(只限DWORD)
  输    入: pXMLElement - XML元素
  输    出: rArgs       - 参数数组
  返    回: 子元素的数量
  修改记录: 
 *******************************************************/
DWORD CFrameKernel::GetXmlChildValue(const XMLElement *pXMLElement, CDArray &rArgs)
{
    DWORD dwArgCount = rArgs.Count();
    const XMLElement *pXMLChildElement = pXMLElement->FirstChildElement();
    while (pXMLChildElement)
    {
        char szConfig[DCOP_STRING_ARG_LEM_MAX];
        char szValue[DCOP_STRING_ARG_LEM_MAX];

        /// 子元素名称(为空就不配置了)
        const char *pszName = pXMLChildElement->Name();
        if (!pszName || !(*pszName))
        {
            pXMLChildElement = pXMLChildElement->NextSiblingElement();
            continue;
        }

        (void)snprintf(szConfig, sizeof(szConfig), "-%s", pszName);
        szConfig[sizeof(szConfig) - 1] = '\0';
        if (rArgs.Append(szConfig) != SUCCESS)
        {
            pXMLChildElement = pXMLChildElement->NextSiblingElement();
            continue;
        }

        /// 配置项值(为空就视配置项为开关: 只有名称，而没有值)
        const char *pszValue = pXMLChildElement->GetText();
        if (!pszValue || !(*pszValue))
        {
            pXMLChildElement = pXMLChildElement->NextSiblingElement();
            continue;
        }

        (void)snprintf(szValue, sizeof(szValue), "%s", pszValue);
        szValue[sizeof(szValue) - 1] = '\0';
        (void)rArgs.Append(szValue);

        pXMLChildElement = pXMLChildElement->NextSiblingElement();
    }

    if (rArgs.Count() < dwArgCount)
    {
        dwArgCount = 0;
    }
    else
    {
        dwArgCount = rArgs.Count() - dwArgCount;
    }

    return dwArgCount;
}

/*******************************************************
  函 数 名: CFrameKernel::GetArgList
  描    述: CFrameKernel获取char **argv这种参数形式
  输    入: crArgs      - 参数数组(最好把argc置为和crArgs数量相等)
  输    出: argv        - 参数列表
  返    回: 子元素的数量
  修改记录: 
 *******************************************************/
void CFrameKernel::GetArgList(DWORD argc, char **argv, const CDArray &crArgs)
{
    for (DWORD i = 0; i < argc; ++i)
    {
        char *pStr = (char *)crArgs.Pos(i);
        if (!pStr)
        {
            break;
        }

        argv[i] = pStr;
    }
}

/*******************************************************
  函 数 名: CFrameKernel::CreateAllObjects
  描    述: CFrameKernel创建所有XML描述中的对象
  输    入: piManager   - 管理器
            pXMLElement - XML元素
  输    出: 
  返    回: 成功或者失败的错误码
  修改记录: 
 *******************************************************/
DWORD CFrameKernel::CreateAllObjects(IManager *piManager, const XMLElement *pXMLElement)
{
    if (!piManager) return FAILURE;

    const XMLElement *pXmlObjects = pXMLElement->FirstChildElement("objects");
    if (!pXmlObjects)
    {
        return FAILURE;
    }

    const XMLElement *pXmlObject = pXmlObjects->FirstChildElement();
    DWORD dwRc = SUCCESS;
    while (pXmlObject)
    {
        /// 获取配置参数
        CDArray szArgs(DCOP_STRING_ARG_LEM_MAX, DCOP_OBJECT_ARG_MAX_COUNT);
        char *argv[DCOP_OBJECT_ARG_MAX_COUNT];
        DWORD argc = 0;
        argc  = GetXmlAttribute (pXmlObject, szArgs);
        argc += GetXmlChildValue(pXmlObject, szArgs);
        GetArgList(argc, argv, szArgs);

        /// 设置任务变量中
        objTask *pTask = objTask::Current();
        if (pTask)
        {
            DWORD dwObjID = 0;
            const char *objID = pXmlObject->Attribute("id");
            dwObjID = (objID)? (DWORD)atoi(objID) : 0;
            (void)pTask->SetLocal(TASK_LOCAL_HANDLER, &dwObjID, sizeof(dwObjID));
        }

        /// 如果是动态库，则先进行动态加载
        objDynamicLoader *pLoader = 0;
        const char *dllFile = pXmlObject->Attribute("dll");
        if (dllFile)
        {
            pLoader = CFrameKernel::DynamicLoad(dllFile);
        }

        /// 加载单个对象
        const char *cszName = pXmlObject->Attribute("inst");
        if (!cszName) cszName = pXmlObject->Attribute("name");
        IObject *piObject = Load(cszName, piManager, argc, argv);
        if (!piObject)
        {
            delete pLoader;
            pLoader = 0;
            dwRc |= FAILURE;
        }

        /// 添加动态加载句柄
        if (pLoader)
        {
            if (AddRefer(piObject, 0, pLoader) != SUCCESS)
            {
                delete pLoader;
                pLoader = 0;
            }
        }

        /// 到下一个配置元素
        pXmlObject = pXmlObject->NextSiblingElement();
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CFrameKernel::OnInstanceQueryInterface
  描    述: 引用时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CFrameKernel::OnInstanceQueryInterface(
                        Instance *piThis, 
                        Instance *piRefer, 
                        void *pPara)
{
    CFrameKernel *pKernel = (CFrameKernel *)pPara;
    if (!pKernel)
    {
        return;
    }

    (void)pKernel->AddRefer(piThis, piRefer);
}

/*******************************************************
  函 数 名: CFrameKernel::OnInstanceRelease
  描    述: 释放引用时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CFrameKernel::OnInstanceRelease(
                    Instance *piThis, 
                    Instance *piRefer, 
                    void *pPara)
{
    CFrameKernel *pKernel = (CFrameKernel *)pPara;
    if (!pKernel)
    {
        return;
    }

    (void)pKernel->DelRefer(piThis, piRefer);
}

/*******************************************************
  函 数 名: CFrameKernel::OnInstanceGetRef
  描    述: 获取引用时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CFrameKernel::OnInstanceGetRef(
                    Instance *piThis, 
                    Instance ***pppiRefers, 
                    DWORD *pdwReferCount, 
                    void *pPara)
{
    CFrameKernel *pKernel = (CFrameKernel *)pPara;
    if (!pKernel)
    {
        return;
    }

    DWORD dwReferCount = pKernel->GetRefer(piThis, pppiRefers);
    if (pdwReferCount)
    {
        *pdwReferCount = dwReferCount;
    }
}

