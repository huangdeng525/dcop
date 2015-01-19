/// -------------------------------------------------
/// ObjData_main.cpp : 对象数据实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjData_main.h"
#include "Factory_if.h"
#include "Manager_if.h"
#include "list/dlistvector.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CData, "data")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CData)
    DCOP_IMPLEMENT_INTERFACE(IData)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CData)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
    DCOP_IMPLEMENT_CONFIG_STRING("datafiledir", m_szDataFileDir)
DCOP_IMPLEMENT_IOBJECT_END


/*******************************************************
  函 数 名: CData::CData
  描    述: CData构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CData::CData(Instance *piParent, int argc, char **argv)
{
    (void)memset(m_szDataFileDir, 0, sizeof(m_szDataFileDir));
    m_piModel = 0;

    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);
}

/*******************************************************
  函 数 名: CData::~CData
  描    述: CData析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CData::~CData()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CData::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CData::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    /// 查询对象
    DCOP_QUERY_OBJECT_START(root)
        DCOP_QUERY_OBJECT_ITEM(IModel,       DCOP_OBJECT_MODEL,      m_piModel)
    DCOP_QUERY_OBJECT_END

    return SUCCESS;
}

/*******************************************************
  函 数 名: CData::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CData::Fini()
{
    /// 释放容器中的数据句柄
    Enter();
    for (IT_DATA it_data = m_datas.begin(); 
        it_data != m_datas.end(); 
        ++it_data)
    {
        DCOP_RELEASE_INSTANCE((*it_data).second);
    }
    Leave();

    DCOP_RELEASE_INSTANCE(m_piModel);
}

/*******************************************************
  函 数 名: CData::Dump
  描    述: Dump入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CData::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    AutoObjLock(this);

    if (!argc)
    {
        for (IT_DATA it_data = m_datas.begin();
            it_data != m_datas.end(); ++it_data)
        {
            ((*it_data).second)->Dump(logPrint, logPara);
        }

        return;
    }

    for (int i = 0; i < argc; ++i)
    {
        if (!(argv[i])) continue;

        IT_DATA it_data = m_datas.find(*(DWORD *)(argv[i]));
        if (it_data == m_datas.end()) continue;

        ((*it_data).second)->Dump(logPrint, logPara);
    }
}

/*******************************************************
  函 数 名: CData::Create
  描    述: 创建一个数据
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CData::Create(DWORD dwType, DWORD dwAttrID, IObject *pOwner, Handle &hData)
{
    AutoObjLock(this);

    /// 查找是否有重复的属性
    IT_DATA it_data = m_datas.find(dwAttrID);
    if (it_data != m_datas.end())
    {
        return FAILURE;
    }

    /// 创建新的句柄
    IDataHandle *pDataHandle = 0;
    switch (dwType)
    {
        case TYPE_MEM:
            DCOP_CREATE_INSTANCE(IDataHandle, "DataMem", this, 0, 0, pDataHandle);
            break;
        case TYPE_FILE:
        {
            char *argDataFile[] = {m_szDataFileDir};
            DCOP_CREATE_INSTANCE(IDataHandle, "DataFile", this, ARRAY_SIZE(argDataFile), argDataFile, pDataHandle);
        }
            break;
        case TYPE_MYSQL:
            DCOP_CREATE_INSTANCE(IDataHandle, "DataMySQL", this, 0, 0, pDataHandle);
            break;
        default:
            break;
    }
    if (!pDataHandle)
    {
        return FAILURE;
    }

    /// 初始化句柄
    DWORD dwRc = pDataHandle->Init(dwAttrID, pOwner, m_piModel);
    if (dwRc)
    {
        DCOP_RELEASE_INSTANCE(pDataHandle);
        return dwRc;
    }

    /// 添加到容器中
    it_data = m_datas.insert(m_datas.end(), MAP_DATA::value_type(dwAttrID, pDataHandle));
    if (it_data == m_datas.end())
    {
        DCOP_RELEASE_INSTANCE(pDataHandle);
        return FAILURE;
    }

    /// 输出数据句柄
    hData = (Handle)((*it_data).second);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CData::Destroy
  描    述: 销毁一个数据句柄
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CData::Destroy(DWORD dwAttrID)
{
    AutoObjLock(this);

    IT_DATA it_data = m_datas.find(dwAttrID);
    if (it_data == m_datas.end())
    {
        return FAILURE;
    }

    IDataHandle *pHandle = ((*it_data).second);
    if (pHandle)
    {
        const char *pcszTableName = pHandle->GetTableName();
        CStrFormat strLog("-> [DATA] Release '%s' Handle Ref Count", ((pcszTableName)? pcszTableName : "none"));
        DWORD dwRefCount = pHandle->Release();
        TRACE_LOG(STR_FORMAT("%s:%d", (const char *)strLog, dwRefCount));
    }

    (void)m_datas.erase(it_data);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CData::AddRecord
  描    述: 添加一条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CData::AddRecord(Handle hData, 
                        DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
                        void *pReqData, DWORD dwReqDataLen, 
                        DCOP_PARA_NODE **ppEvtPara, 
                        DWORD *pdwEvtParaCount, 
                        CDArray *pEvtData)
{
    IDataHandle *pDataHandle = dynamic_cast<IDataHandle *>((Instance *)hData);
    if (!pDataHandle)
    {
        return FAILURE;
    }

    /// 这里不使用data本身的锁保护，因为里面会使用owner对象的锁保护

    return pDataHandle->AddRecord(pReqPara, dwReqParaCount, 
                        pReqData, dwReqDataLen, 
                        ppEvtPara, pdwEvtParaCount, 
                        pEvtData);
}

/*******************************************************
  函 数 名: CData::DelRecord
  描    述: 删除一条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CData::DelRecord(Handle hData, BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        DCOP_PARA_NODE **ppEvtPara, 
                        DWORD *pdwEvtParaCount, 
                        CDArray *pEvtData)
{
    IDataHandle *pDataHandle = dynamic_cast<IDataHandle *>((Instance *)hData);
    if (!pDataHandle)
    {
        return FAILURE;
    }

    /// 这里不使用data本身的锁保护，因为里面会使用owner对象的锁保护

    return pDataHandle->DelRecord(byCond, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen, 
                        ppEvtPara, pdwEvtParaCount, 
                        pEvtData);
}

/*******************************************************
  函 数 名: CData::EditRecord
  描    述: 编辑一条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CData::EditRecord(Handle hData, BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
                        void *pReqData, DWORD dwReqDataLen, 
                        DCOP_PARA_NODE **ppEvtPara, 
                        DWORD *pdwEvtParaCount, 
                        CDArray *pEvtData)
{
    IDataHandle *pDataHandle = dynamic_cast<IDataHandle *>((Instance *)hData);
    if (!pDataHandle)
    {
        return FAILURE;
    }

    /// 这里不使用data本身的锁保护，因为里面会使用owner对象的锁保护

    return pDataHandle->EditRecord(byCond, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen, 
                        pReqPara, dwReqParaCount, 
                        pReqData, dwReqDataLen, 
                        ppEvtPara, pdwEvtParaCount, 
                        pEvtData);
}

/*******************************************************
  函 数 名: CData::QueryRecord
  描    述: 查询多条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CData::QueryRecord(Handle hData, BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
                        DCOP_PARA_NODE *&rpRspPara, DWORD &rdwRspParaCount, 
                        CDArray &aRspData)
{
    IDataHandle *pDataHandle = dynamic_cast<IDataHandle *>((Instance *)hData);
    if (!pDataHandle)
    {
        return FAILURE;
    }

    /// 这里不使用data本身的锁保护，因为里面会使用owner对象的锁保护

    return pDataHandle->QueryRecord(byCond, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen, 
                        pReqPara, dwReqParaCount, 
                        rpRspPara, rdwRspParaCount, 
                        aRspData);
}

/*******************************************************
  函 数 名: CData::CountRecord
  描    述: 统计记录数量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CData::CountRecord(Handle hData, BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        DWORD &rdwCount)
{
    IDataHandle *pDataHandle = dynamic_cast<IDataHandle *>((Instance *)hData);
    if (!pDataHandle)
    {
        return FAILURE;
    }

    /// 这里不使用data本身的锁保护，因为里面会使用owner对象的锁保护

    return pDataHandle->CountRecord(byCond, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen, 
                        rdwCount);
}

/*******************************************************
  函 数 名: CData::AddKeyIdx
  描    述: 添加关键字索引
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CData::AddKeyIdx(Handle hData, DCOP_PARA_NODE *pIdxPara, DWORD dwIdxParaCount)
{
    IDataHandle *pDataHandle = dynamic_cast<IDataHandle *>((Instance *)hData);
    if (!pDataHandle)
    {
        return FAILURE;
    }

    /// 这里不使用data本身的锁保护，因为里面会使用owner对象的锁保护

    return pDataHandle->AddKeyIdx(pIdxPara, dwIdxParaCount);
}

/*******************************************************
  函 数 名: CData::DelKeyIdx
  描    述: 删除关键字索引
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CData::DelKeyIdx(Handle hData, DCOP_PARA_NODE *pIdxPara, DWORD dwIdxParaCount)
{
    IDataHandle *pDataHandle = dynamic_cast<IDataHandle *>((Instance *)hData);
    if (!pDataHandle)
    {
        return FAILURE;
    }

    /// 这里不使用data本身的锁保护，因为里面会使用owner对象的锁保护

    return pDataHandle->DelKeyIdx(pIdxPara, dwIdxParaCount);
}

/*******************************************************
  函 数 名: CData::GetHandle
  描    述: 获取属性值对应的数据句柄
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IData::Handle CData::GetHandle(DWORD dwAttrID)
{
    AutoObjLock(this);

    IT_DATA it_data = m_datas.find(dwAttrID);
    if (it_data == m_datas.end())
    {
        return NULL;
    }

    return (Handle)((*it_data).second);
}

