/// -------------------------------------------------
/// appbase_main.cpp : 应用基础实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include <json/json.h>
#include "appbase_main.h"
#include "Factory_if.h"
#include "Manager_if.h"

#if DCOP_OS == DCOP_OS_WINDOWS
#pragma comment(lib, "jsond")
#endif


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CAppBase, "appbase")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CAppBase)
    DCOP_IMPLEMENT_INTERFACE(IAppBase)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CAppBase)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
    DCOP_IMPLEMENT_CONFIG_CONCURRENT_ENABLE()
    DCOP_IMPLEMENT_CONFIG_STRING("datacfg", m_szModelCfg)
DCOP_IMPLEMENT_IOBJECT_END

/// -------------------------------------------------
/// 实现消息分发
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE(CAppBase)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_START, OnStart)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_FINISH, OnFinish)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_OBJECT_REQUEST, OnRequest)
    DCOP_IMPLEMENT_IOBJECT_MSG_DEFAULT(OnDefault)
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE_END


/*******************************************************
  函 数 名: CAppBase::CAppBase
  描    述: CAppBase构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CAppBase::CAppBase(Instance *piParent, int argc, char **argv)
{
    m_pDatas = 0;
    m_pDataAttrs = 0;
    m_dwDataCount = 0;

    (void)memset(m_szModelCfg, 0, sizeof(m_szModelCfg));

    m_piModel = 0;
    m_piData = 0;

    m_piDispatch = 0;
    m_piNotify = 0;
    m_pNotifyPool = 0;

    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);
}

/*******************************************************
  函 数 名: CAppBase::~CAppBase
  描    述: CAppBase析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CAppBase::~CAppBase()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CAppBase::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CAppBase::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    /// 查询对象
    DCOP_QUERY_OBJECT_START(root)
        DCOP_QUERY_OBJECT_ITEM(IModel,       DCOP_OBJECT_MODEL,      m_piModel)
        DCOP_QUERY_OBJECT_ITEM(IData,        DCOP_OBJECT_DATA,       m_piData)
        DCOP_QUERY_OBJECT_ITEM(IDispatch,    DCOP_OBJECT_DISPATCH,   m_piDispatch)
        DCOP_QUERY_OBJECT_ITEM(INotify,      DCOP_OBJECT_NOTIFY,     m_piNotify)
    DCOP_QUERY_OBJECT_END

    return SUCCESS;
}

/*******************************************************
  函 数 名: CAppBase::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CAppBase::Fini()
{
    if (m_piNotify && m_pNotifyPool)
    {
        m_piNotify->DestroyPool(m_pNotifyPool);
        m_pNotifyPool = 0;
    }

    if (m_pDatas)
    {
        delete [] m_pDatas;
        m_pDatas = 0;
    }

    if (m_pDataAttrs)
    {
        delete [] m_pDataAttrs;
        m_pDataAttrs = 0;
    }

    DCOP_RELEASE_INSTANCE(m_piNotify);
    DCOP_RELEASE_INSTANCE(m_piDispatch);
    DCOP_RELEASE_INSTANCE(m_piData);
    DCOP_RELEASE_INSTANCE(m_piModel);
}

/*******************************************************
  函 数 名: CAppBase::Dump
  描    述: DUMP入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CAppBase::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    
}

/*******************************************************
  函 数 名: CAppBase::OnStart
  描    述: 开始运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CAppBase::OnStart(objMsg *msg)
{
    DWORD dwRc = InitModelData();
    if (dwRc != SUCCESS)
    {
        CHECK_ERRCODE(dwRc, "Init Model Data");
        return;
    }

    if (m_pDatas && m_pDataAttrs && m_dwDataCount)
    {
        CDArray aEvents(sizeof(DWORD), m_dwDataCount);
        for (DWORD i = 0; i < m_dwDataCount; ++i)
        {
            DWORD dwEvent = m_pDataAttrs[i].GetID();
            (void)aEvents.Append(&dwEvent);
        }

        m_pNotifyPool = m_piNotify->CreatePool(this, (DWORD *)aEvents.Get(), aEvents.Count());
        if (!m_pNotifyPool)
        {
            CHECK_ERRCODE(FAILURE, "Create Notify Pool");
            return;
        }

        CDArray aMembers(sizeof(IObjectMember *), m_dwDataCount);
        for (DWORD i = 0; i < m_dwDataCount; ++i)
        {
            m_pDatas[i].SetDataObj(m_piData);
            IObjectMember *piMember = m_pDatas[i].Init( this, &(m_pDataAttrs[i]) );
            dwRc = aMembers.Append(&piMember);
            CHECK_ERRCODE(dwRc, "Append Member");
        }

        dwRc = m_dataIndex.Init((IObjectMember **)aMembers.Get(), aMembers.Count(), m_piDispatch, m_pNotifyPool);
        if (dwRc != SUCCESS)
        {
            CHECK_ERRCODE(dwRc, "Init Data Index");
            return;
        }

        for (DWORD i = 0; i < m_dwDataCount; ++i)
        {
            dwRc = m_pDatas[i].Create();
            CHECK_ERRCODE(dwRc, "Create Data");
        }
    }
}

/*******************************************************
  函 数 名: CAppBase::OnFinish
  描    述: 结束运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CAppBase::OnFinish(objMsg *msg)
{
    if (m_pDatas && m_dwDataCount)
    {
        for (DWORD i = 0; i < m_dwDataCount; ++i)
        {
            (void)m_pDatas[i].Destroy();
        }
    }
}

/*******************************************************
  函 数 名: CAppBase::OnRequest
  描    述: 请求消息时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CAppBase::OnRequest(objMsg *msg)
{
    m_dataIndex.Dispatch(msg);
}

/*******************************************************
  函 数 名: CAppBase::OnDefault
  描    述: 默认消息
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CAppBase::OnDefault(objMsg *msg)
{
}

/*******************************************************
  函 数 名: CAppBase::InitModelData
  描    述: 初始化模型和数据
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CAppBase::InitModelData()
{
    if (!m_piModel || !m_szModelCfg || !(*m_szModelCfg))
    {
        return FAILURE;
    }

    CDString strDataCfg;
    DWORD dwRc = strDataCfg.LoadFile(m_szModelCfg);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse((const char *)strDataCfg, (const char *)strDataCfg + strDataCfg.Length(), root, false))
    {
        return FAILURE;
    }

    m_dwDataCount = (DWORD)root.size();
    if (!m_dwDataCount)
    {
        return FAILURE;
    }

    m_pDataAttrs = new CAttribute[m_dwDataCount];
    if (!m_pDataAttrs)
    {
        return FAILURE;
    }

    m_pDatas = new CObjectMember<IData *>[m_dwDataCount];
    if (!m_pDatas)
    {
        return FAILURE;
    }

    /// 获取表信息
    Json::Value table;
    char tableName[DCOP_TABLE_NAME_LEN];
    for (DWORD tableIdx = 0; tableIdx < m_dwDataCount; ++tableIdx)
    {
        table = root[tableIdx];

        const char *cszTableName = table["table_name"].asCString();
        if (!cszTableName)
        {
            return FAILURE;
        }

        (void)memset(tableName, 0, sizeof(tableName));
        (void)snprintf(tableName, sizeof(tableName), "%s", cszTableName);
        tableName[sizeof(tableName) - 1] = '\0';
        m_pDataAttrs[tableIdx].SetName(cszTableName);

        DWORD dwTableID = (DWORD)table["table_id"].asUInt();
        if (!dwTableID)
        {
            return FAILURE;
        }

        DWORD dwAttrID = ( (ID() << 16) | dwTableID );
        m_pDataAttrs[tableIdx].SetID(dwAttrID);

        DWORD dwAttrType = (DWORD)table["attr_type"].asUInt();
        if (dwAttrType > IModel::TYPE_EVENT)
        {
            return FAILURE;
        }

        m_pDataAttrs[tableIdx].SetType(dwAttrType);

        DWORD dwDataType = (DWORD)table["data_type"].asUInt();
        if (dwDataType > IData::TYPE_MYSQL)
        {
            return FAILURE;
        }

        m_pDatas[tableIdx].SetDataType((IData::TYPE)dwDataType);

        /// 获取字段信息
        Json::Value fields = table["fields"];
        DWORD dwFieldCount = (DWORD)fields.size();
        if (!dwFieldCount)
        {
            return FAILURE;
        }

        Json::Value field;
        IModel::Field fieldNode;
        CDArray aFields(sizeof(IModel::Field), dwFieldCount);
        for (DWORD fieldIdx = 0; fieldIdx < dwFieldCount; ++fieldIdx)
        {
            field = fields[fieldIdx];

            const char *cszFieldName = field["field_name"].asCString();
            if (!cszFieldName)
            {
                return FAILURE;
            }

            (void)memset(fieldNode.m_fieldName, 0, sizeof(fieldNode.m_fieldName));
            (void)snprintf(fieldNode.m_fieldName, sizeof(fieldNode.m_fieldName), "%s", cszFieldName);
            fieldNode.m_fieldName[sizeof(fieldNode.m_fieldName) - 1] = '\0';

            fieldNode.m_isKey = (BYTE)field["is_key"].asUInt();
            if (fieldNode.m_isKey > 1)
            {
                return FAILURE;
            }

            fieldNode.m_fieldType = (BYTE)field["field_type"].asUInt();
            if (!fieldNode.m_fieldType || (fieldNode.m_fieldType >= IModel::FIELD_NUM))
            {
                return FAILURE;
            }

            fieldNode.m_fieldSize = (WORD)field["field_size"].asUInt();
            if (!fieldNode.m_fieldSize)
            {
                return FAILURE;
            }

            fieldNode.m_defaultValue = (DWORD)field["field_def_value"].asUInt();
            fieldNode.m_minValue = (DWORD)field["field_min_value"].asUInt();
            fieldNode.m_maxValue = (DWORD)field["field_max_value"].asUInt();

            (void)aFields.Append(&fieldNode);
        }

        if (aFields.Count() != dwFieldCount)
        {
            return FAILURE;
        }

        /// 获取关联信息
        Json::Value relations = table["relations"];
        DWORD dwShipCount = (DWORD)relations.size();
        CDArray aShips(sizeof(IModel::Relation), dwShipCount);
        if (dwShipCount)
        {
            Json::Value ship;
            IModel::Relation shipNode;
            for (DWORD shipIdx = 0; shipIdx < dwShipCount; ++shipIdx)
            {
                ship = relations[shipIdx];

                shipNode.m_relID = (DWORD)ship["rel_id"].asUInt();
                shipNode.m_attrID = (DWORD)ship["rel_attr"].asUInt();
                shipNode.m_fieldID = (DWORD)ship["rel_field"].asUInt();

                if (!shipNode.m_relID || !shipNode.m_attrID || !shipNode.m_fieldID)
                {
                    return FAILURE;
                }

                /// 如果高位16位有值，说明是绝对属性值；否则就是相对属性值，需加上对象ID
                if (shipNode.m_attrID <= 0x0000ffff) shipNode.m_attrID |= ( ID() << 16 );

                (void)aShips.Append(&shipNode);
            }

            if (aShips.Count() != dwShipCount)
            {
                return FAILURE;
            }
        }

        /// 注册模型
        dwRc = m_piModel->RegTable(tableName, ID(), dwAttrID, (IModel::TYPE)dwAttrType, 
                        (IModel::Field *)aFields.Get(), aFields.Count(), 0, 
                        (IModel::Relation *)aShips.Get(), aShips.Count());
        if (dwRc != SUCCESS)
        {
            return dwRc;
        }
    }

    return SUCCESS;
}


