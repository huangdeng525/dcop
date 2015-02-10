/// -------------------------------------------------
/// ObjManager_main.cpp : 对象管理实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjManager_main.h"
#include "Factory_if.h"
#include "ObjDispatch_if.h"
#include "msg.h"
#include "BaseMessage.h"
#include "BaseID.h"
#include "string/tablestring.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CObjectManager, "manager")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CObjectManager)
    DCOP_IMPLEMENT_INTERFACE(IManager)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CObjectManager)
    DCOP_IMPLEMENT_IDENTIFY_STATIC("manager", DCOP_OBJECT_MANAGER)
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
    IMPLEMENT_CONFIG_SYSTEM("id", "info")
DCOP_IMPLEMENT_IOBJECT_END


/*******************************************************
  函 数 名: CObjectManager::CObjectManager
  描    述: 对象管理实现类构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CObjectManager::CObjectManager(Instance *piParent, int argc, char **argv)
{
    m_dwSystemID = 0;
    (void)memset(m_szSystemInfo, 0, sizeof(m_szSystemInfo));

    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);

    m_bInit = false;
}

/*******************************************************
  函 数 名: CObjectManager::~CObjectManager
  描    述: 对象管理实现类析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CObjectManager::~CObjectManager()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CObjectManager::Init
  描    述: 初始化入口
  输    入: root - NULL, 管理器一般没有父对象
            argc - 一次性传入的对象个数
            argv - 一次性传入的对象指针
            (注意:多继承可能再转换为void*时丢失dynamic信息)
            (所以:多继承的对象尽可能在初始化前调用InsertObject)
  输    出: 
  返    回: SUCCESS             - 成功
            FAILURE             - 失败
  修改记录: 
 *******************************************************/
DWORD CObjectManager::Init(IObject *root, int argc, void **argv)
{
    AutoObjLock(this);

    if (m_bInit)
    {
        return FAILURE;
    }

    objTask *pTask = objTask::Current();
    DWORD dwObjID = 0;
    if (pTask)
    {
        dwObjID = DCOP_OBJECT_MANAGER;
        (void)pTask->SetLocal(TASK_LOCAL_HANDLER, &dwObjID, sizeof(dwObjID));
    }

    /////////////////////////////////////////////////
    /// 先插入所有的输入对象
    /////////////////////////////////////////////////
    for (int i = 0; i < argc; ++i)
    {
        (void)InsertObject((IObject *)(argv[i]));
    }

    /////////////////////////////////////////////////
    /// 接着初始化所有对象
    /////////////////////////////////////////////////
    /// 为了防止在初始化过程中插入新对象，
    /// 必须先将所有的对象收集到临时数组
    CDArray aObjects(sizeof(IObject *), (DWORD)m_objects.size());
    for (IT_OBJECTS it_obj = m_objects.begin();
        it_obj != m_objects.end(); ++it_obj)
    {
        IObject *pObjTmp = ((*it_obj).second).GetObject();
        (void)aObjects.Append(&pObjTmp);
    }

    /////////////////////////////////////////////////
    /// 置初始化标识，同时进行初始化
    /// (先置标识是为了防止在初始化过程中再插入新对象)
    /////////////////////////////////////////////////
    m_bInit = true;

    IObject **ppiObject = (IObject **)aObjects.Get();
    if (!ppiObject)
    {
        return FAILURE;
    }

    DWORD dwRc = SUCCESS;
    for (DWORD i = 0; i < aObjects.Count(); ++i)
    {
        IObject *pObjTmp = ppiObject[i];
        if (!pObjTmp) continue;

        if (pTask)
        {
            dwObjID = pObjTmp->ID();
            (void)pTask->SetLocal(TASK_LOCAL_HANDLER, &dwObjID, sizeof(dwObjID));
        }

        dwRc = pObjTmp->Init(this, 0, 0);
        if (dwRc)
        {
            CHECK_RETCODE(dwRc, STR_FORMAT("Init Static Object: '%s'(ID:%d) Fail!", 
                        pObjTmp->Name(), pObjTmp->ID()));
            break;
        }
    }

    /////////////////////////////////////////////////
    /// 如果初始化失败返回错误
    /////////////////////////////////////////////////
    if (dwRc)
    {
        return dwRc;
    }

    /////////////////////////////////////////////////
    /// 发送初始化事件
    /////////////////////////////////////////////////
    SendEvent(true);

    if (pTask)
    {
        dwObjID = DCOP_OBJECT_KERNEL;
        (void)pTask->SetLocal(TASK_LOCAL_HANDLER, &dwObjID, sizeof(dwObjID));
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CObjectManager::Fini
  描    述: 结束时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CObjectManager::Fini()
{
    AutoObjLock(this);

    if (!m_bInit)
    {
        return;
    }

    /////////////////////////////////////////////////
    /// 发送结束事件
    /////////////////////////////////////////////////
    SendEvent(false);

    /////////////////////////////////////////////////
    /// 结束所有的对象
    /////////////////////////////////////////////////
    for (IT_OBJECTS it_obj = m_objects.begin();
        it_obj != m_objects.end(); ++it_obj)
    {
        IObject *pObjTmp = ((*it_obj).second).GetObject();
        if (!pObjTmp) continue;

        pObjTmp->Fini();
    }

    /////////////////////////////////////////////////
    /// CObjectNode析构会释放对象引用
    /////////////////////////////////////////////////
    m_objects.clear();

    /////////////////////////////////////////////////
    /// 置初始化标识
    /////////////////////////////////////////////////
    m_bInit = false;
}

/*******************************************************
  函 数 名: CObjectManager::Proc
  描    述: 处理消息
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CObjectManager::Proc(objMsg *msg)
{
    BroadcastMsg(msg);
}

/*******************************************************
  函 数 名: CObjectManager::Dump
  描    述: Dump入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CObjectManager::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    if (!logPrint) return;

    AutoObjLock(this);

    DWORD dwCount = (DWORD)m_objects.size();
    CTableString tableStr(7, dwCount + 4, "  ");
    tableStr << "objKey";
    tableStr << "objID";
    tableStr << "objName";
    tableStr << "className";
    tableStr << "classSize";
    tableStr << "refCount";
    tableStr << "objPtr";

    tableStr << STR_FORMAT("%d", DCOP_OBJECT_KERNEL);
    tableStr << STR_FORMAT("%d", DCOP_OBJECT_KERNEL);
    tableStr << "kernel";
    tableStr << "";
    tableStr << "";
    tableStr << "";
    tableStr << STR_FORMAT("%p", objBase::GetInstance());

    tableStr << STR_FORMAT("%d", DCOP_OBJECT_FACTORY);
    tableStr << STR_FORMAT("%d", DCOP_OBJECT_FACTORY);
    tableStr << "factory";
    tableStr << "";
    tableStr << "";
    tableStr << "";
    tableStr << STR_FORMAT("%p", IFactory::GetInstance());

    tableStr << STR_FORMAT("%d", DCOP_OBJECT_MANAGER);
    tableStr << STR_FORMAT("%d", ID());
    tableStr << Name();
    tableStr << Class();
    tableStr << STR_FORMAT("%d", Size());
    tableStr << STR_FORMAT("%d", GetRef());
    tableStr << STR_FORMAT("%p", this);

    DCOP_START_TIME();

    for (IT_OBJECTS it_obj = m_objects.begin();
        it_obj != m_objects.end(); ++it_obj)
    {
        IObject *pObjTmp = ((*it_obj).second).GetObject();
        if (!pObjTmp) continue;

        tableStr << STR_FORMAT("%d", (*it_obj).first);
        tableStr << STR_FORMAT("%d", pObjTmp->ID());
        tableStr << pObjTmp->Name();
        tableStr << pObjTmp->Class();
        tableStr << STR_FORMAT("%d", pObjTmp->Size());
        tableStr << STR_FORMAT("%d", pObjTmp->GetRef());
        tableStr << STR_FORMAT("%p", pObjTmp);
    }

    DCOP_END_TIME();

    logPrint(STR_FORMAT("Manager Dump: (Objects Count: %d) \r\n", dwCount), logPara);
    tableStr.Show(logPrint, logPara, "=", "-");
    logPrint(STR_FORMAT("[cost time: %d ms] \r\n", DCOP_COST_TIME()), logPara);
}

/*******************************************************
  函 数 名: CObjectManager::Child
  描    述: 从对象管理器中找到一个对象
  输    入: dwID                - 对象ID
  输    出: 
  返    回: 
            IObject *           - 对象指针
  修改记录: 
 *******************************************************/
IObject *CObjectManager::Child(DWORD dwID)
{
    if (dwID == DCOP_OBJECT_MANAGER)
    {
        return (IObject *)this;
    }

    AutoObjLock(this);

    IT_OBJECTS it_obj = m_objects.find(dwID);
    if (it_obj == m_objects.end())
    {
        return NULL;
    }

    return ((*it_obj).second).GetObject();
}

/*******************************************************
  函 数 名: CObjectManager::InsertObject
  描    述: 插入一个对象到对象管理器中
  输    入: pObject             - 对象
  输    出: 
  返    回: 
            SUCCESS             - 成功
            FAILURE             - 失败
  修改记录: 
 *******************************************************/
DWORD CObjectManager::InsertObject(IObject *pObject)
{
    if (!pObject)
    {
        return FAILURE;
    }

    DWORD dwID = pObject->ID();

    AutoObjLock(this);

    /////////////////////////////////////////////////
    /// 查找是否有重复的对象
    /////////////////////////////////////////////////
    IT_OBJECTS it_obj = m_objects.find(dwID);
    if (it_obj != m_objects.end())
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 引用对象
    /////////////////////////////////////////////////
    if (pObject->QueryInterface(ID_INTF(IObject), 0, this) != SUCCESS)
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 把对象加入到MAP容器中
    /////////////////////////////////////////////////
    CObjectNode objNode;
    it_obj = m_objects.insert(m_objects.end(), MAP_OBJECTS::value_type(dwID, objNode));
    if (it_obj == m_objects.end())
    {
        (void)pObject->Release(this);
        return FAILURE;
    }

    ((*it_obj).second).SetObject(pObject, this);

    /////////////////////////////////////////////////
    /// 如果是初始化之前加入的，视为静态对象，等总初始化入口一起初始化
    /////////////////////////////////////////////////
    if (!m_bInit)
    {
        return SUCCESS;
    }

    /////////////////////////////////////////////////
    /// 如果是初始化之后加入的，视为动态对象，这里单独进行初始化
    /////////////////////////////////////////////////
    DWORD dwRc = pObject->Init(this, 0, 0);
    if (dwRc)
    {
        CHECK_RETCODE(dwRc, STR_FORMAT("Init Dynamic Object: '%s'(ID:%d) Fail!", 
                        pObject->Name(), pObject->ID()));
        (void)m_objects.erase(it_obj);
        return dwRc;
    }

    SendEvent(true, pObject);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CObjectManager::DeleteObject
  描    述: 从对象管理器中删除一个对象
  输    入: pObject             - 对象
  输    出: 
  返    回: 
            SUCCESS             - 成功
            FAILURE             - 失败
  修改记录: 
 *******************************************************/
DWORD CObjectManager::DeleteObject(IObject *pObject)
{
    if (!pObject)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    /////////////////////////////////////////////////
    /// 查找是否有对象
    /////////////////////////////////////////////////
    IT_OBJECTS it_obj = m_objects.find(pObject->ID());
    if (it_obj == m_objects.end())
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 已初始化，单独进行结束处理
    /////////////////////////////////////////////////
    if (m_bInit)
    {
        IObject *pObjTmp = ((*it_obj).second).GetObject();
        if (!pObjTmp) return FAILURE;

        pObjTmp->Fini();
        SendEvent(false, pObjTmp);
    }

    /////////////////////////////////////////////////
    /// CObjectNode析构会释放对象引用
    /////////////////////////////////////////////////
    (void)m_objects.erase(it_obj);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CObjectManager::GetSystemID
  描    述: 获取系统ID
  输    入: 
  输    出: 
  返    回: 系统ID
  修改记录: 
 *******************************************************/
DWORD CObjectManager::GetSystemID()
{
    AutoObjLock(this);

    return m_dwSystemID;
}

/*******************************************************
  函 数 名: CObjectManager::GetSystemInfo
  描    述: 获取系统信息
  输    入: 
  输    出: 
  返    回: 系统ID
  修改记录: 
 *******************************************************/
const char *CObjectManager::GetSystemInfo()
{
    AutoObjLock(this);

    return m_szSystemInfo;
}

/*******************************************************
  函 数 名: CObjectManager::BroadcastMsg
  描    述: 广播消息msg
  输    入: msg
  输    出: 
  返    回: 系统ID
  修改记录: 
 *******************************************************/
void CObjectManager::BroadcastMsg(objMsg *msg)
{
    AutoObjLock(this);

    for (IT_OBJECTS it_obj = m_objects.begin();
        it_obj != m_objects.end(); ++it_obj)
    {
        IObject *pObjTmp = ((*it_obj).second).GetObject();
        if (!pObjTmp) continue;

        pObjTmp->Enter();
        pObjTmp->Proc(msg);
        pObjTmp->Leave();
    }
}

/*******************************************************
  函 数 名: CObjectManager::SendEvent
  描    述: 发送初始化事件
  输    入: pDynamicObject - 动态加载的对象
            (动态加载的对象: 在初始化之后插入的对象)
            (0为触发发容器中的所有对象初始化)
  输    出: 
  返    回: 
  备    注: 内部函数不用做保护
  修改记录: 
 *******************************************************/
void CObjectManager::SendEvent(bool bInitOrFini, IObject *pDynamicObject)
{
    DWORD dwCount = (pDynamicObject)? 1 : (DWORD)m_objects.size();
    if (!dwCount)
    {
        return;
    }

    /////////////////////////////////////////////////
    /// 1.创建Init消息
    /////////////////////////////////////////////////
    DWORD dwDataLen = sizeof(DWORD) * (dwCount + 1);
    DWORD dwMsgType = DCOP_MSG_MANAGER_START;
    if (bInitOrFini)
    {
        if (pDynamicObject)
        {
            TRACE_LOG(STR_FORMAT("Send Event: Load Dynamic Object:'%s'(%d)!", pDynamicObject->Name(), pDynamicObject->ID()));
            dwMsgType = DCOP_MSG_MANAGER_LOAD;
        }
        else
        {
            TRACE_LOG(STR_FORMAT("Send Event: Start Static Objects!"));
            dwMsgType = DCOP_MSG_MANAGER_START;
        }
    }
    else
    {
        if (pDynamicObject)
        {
            TRACE_LOG(STR_FORMAT("Send Event: Unload Dynamic Object:'%s'(%d)!", pDynamicObject->Name(), pDynamicObject->ID()));
            dwMsgType = DCOP_MSG_MANAGER_UNLOAD;
        }
        else
        {
            TRACE_LOG(STR_FORMAT("Send Event: Finish Static Objects!"));
            dwMsgType = DCOP_MSG_MANAGER_FINISH;
        }
    }
    objMsg *pMsg = DCOP_CreateMsg(dwDataLen,
                        dwMsgType,
                        ID());
    if (!pMsg)
    {
        CHECK_RETCODE(FAILURE, STR_FORMAT("CreateMsg(Len:%d) Fail!", dwDataLen));
        return;
    }

    DWORD *pdwPara = (DWORD *)pMsg->GetDataBuf();
    OSASSERT(pdwPara != NULL);

    /////////////////////////////////////////////////
    /// 2.填充对象列表到消息
    /////////////////////////////////////////////////
    pdwPara[0] = dwCount;
    if (pDynamicObject)
    {
        pdwPara[1] = pDynamicObject->ID();
    }
    else
    {
        DWORD i = 1;
        for (IT_OBJECTS it_obj = m_objects.begin();
            it_obj != m_objects.end(); ++it_obj)
        {
            IObject *pObjTmp = ((*it_obj).second).GetObject();
            if (!pObjTmp) continue;

            pdwPara[i++] = pObjTmp->ID();
            if (i > dwCount) break;
        }
    }

    /////////////////////////////////////////////////
    /// 3.对管理器里所有对象广播消息
    /////////////////////////////////////////////////
    BroadcastMsg(pMsg);
    delete pMsg;
}

