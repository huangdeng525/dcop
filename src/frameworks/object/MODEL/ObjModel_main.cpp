/// -------------------------------------------------
/// ObjModel_maincpp : 对象模型建模实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjModel_main.h"
#include "Factory_if.h"
#include "Manager_if.h"
#include "BaseMessage.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CModel, "model")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CModel)
    DCOP_IMPLEMENT_INTERFACE(IModel)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CModel)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END

/// -------------------------------------------------
/// 实现消息分发
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE(CModel)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_START, OnStart)
    DCOP_IMPLEMENT_IOBJECT_MSG_DEFAULT(OnDefault)
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE_END


/*******************************************************
  函 数 名: CModel::Table::Table
  描    述: Table构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CModel::Table::Table()
{
    (void)memset(m_szTableName, 0, sizeof(m_szTableName));

    m_objID = 0;
    m_attrID = 0;
    m_attrType = TYPE_DATA;

    m_pFields = NULL;
    m_dwFieldCount = 0;

    m_dwDefRecCount = DCOP_MODEL_DEF_REC_COUNT;

    m_pShips = NULL;
    m_dwShipCount = 0;
}

/*******************************************************
  函 数 名: CModel::Table::~Table
  描    述: CModel析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CModel::Table::~Table()
{
    m_objID = 0;
    m_attrID = 0;
    m_attrType = TYPE_DATA;

    if (m_pFields)
    {
        DCOP_Free(m_pFields);
    }

    if (m_pShips)
    {
        DCOP_Free(m_pShips);
    }

    m_pFields = NULL;
    m_dwFieldCount = 0;
    m_pShips = NULL;
    m_dwShipCount = 0;
}

/*******************************************************
  函 数 名: CModel::Table::Create
  描    述: 创建
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CModel::Table::Create(const char *cszTableName, DWORD objID, DWORD attrID, TYPE attrType, 
                        Field *pFields, DWORD dwFieldCount, DWORD dwDefRecCount, 
                        Relation *pShips, DWORD dwShipCount)
{
    (void)strncpy(m_szTableName , cszTableName, sizeof(m_szTableName));
    m_szTableName[sizeof(m_szTableName) - 1] = '\0';

    m_objID = objID;
    m_attrID = attrID;
    m_attrType = attrType;

    m_dwDefRecCount = dwDefRecCount;

    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    m_pFields = (Field *)DCOP_Malloc(dwFieldCount * sizeof(Field));
    if (!m_pFields)
    {
        return FAILURE;
    }

    (void)memcpy(m_pFields, pFields, dwFieldCount * sizeof(Field));
    m_dwFieldCount = dwFieldCount;

    if (pShips && dwShipCount)
    {
        m_pShips = (Relation *)DCOP_Malloc(dwShipCount * sizeof(Relation));
        if (!m_pShips)
        {
            return FAILURE;
        }

        (void)memcpy(m_pShips, pShips, dwShipCount * sizeof(Relation));
        m_dwShipCount = dwShipCount;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CModel::CModel
  描    述: CModel构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CModel::CModel(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);

    m_piDispatch = 0;
    m_piNotify = 0;
    m_pNotifyPool = 0;

    m_bStartFlag = false;
}

/*******************************************************
  函 数 名: CModel::~CModel
  描    述: CModel析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CModel::~CModel()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CModel::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CModel::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    /// 查询对象
    DCOP_QUERY_OBJECT_START(root)
        DCOP_QUERY_OBJECT_ITEM(IDispatch,    DCOP_OBJECT_DISPATCH,   m_piDispatch)
        DCOP_QUERY_OBJECT_ITEM(INotify,      DCOP_OBJECT_NOTIFY,     m_piNotify)
    DCOP_QUERY_OBJECT_END

    /// 创建缓冲池
    DWORD adwEvents[] = 
    {
        DCOP_MSG_MODEL_REG
    };
    m_pNotifyPool = m_piNotify->CreatePool(this, adwEvents, ARRAY_SIZE(adwEvents));
    if (!m_pNotifyPool)
    {
        return FAILURE;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CModel::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CModel::Fini()
{
    if (m_piNotify && m_pNotifyPool)
    {
        m_piNotify->DestroyPool(m_pNotifyPool);
        m_pNotifyPool = 0;
    }

    DCOP_RELEASE_INSTANCE(m_piNotify);
    DCOP_RELEASE_INSTANCE(m_piDispatch);
}

/*******************************************************
  函 数 名: CModel::RegTable
  描    述: 注册表及字段
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CModel::RegTable(char tableName[DCOP_TABLE_NAME_LEN],
                        DWORD objID,
                        DWORD attrID,
                        TYPE attrType,
                        Field *pFields,
                        DWORD dwFieldCount, 
                        DWORD dwDefRecCount, 
                        Relation *pShips, 
                        DWORD dwShipCount)
{
    AutoObjLock(this);

    IT_TABLES it_table = m_tables.find(attrID);
    if (it_table != m_tables.end())
    {
        return FAILURE;
    }

    Table table;
    it_table = m_tables.insert(m_tables.end(), MAP_TABLES::value_type(attrID, table));
    if (it_table == m_tables.end())
    {
        return FAILURE;
    }

    DWORD dwRc = ((*it_table).second).Create(tableName, objID, attrID, attrType, 
                        pFields, dwFieldCount, dwDefRecCount, 
                        pShips, dwShipCount);
    if (dwRc)
    {
        (void)m_tables.erase(it_table);
        return FAILURE;
    }

    /// 已经启动了，则单独发送注册事件
    if (m_bStartFlag)
    {
        SendEvent(attrID);
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CModel::GetTableName
  描    述: 获取表名
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
const char *CModel::GetTableName(DWORD attrID)
{
    AutoObjLock(this);

    IT_TABLES it_table = m_tables.find(attrID);
    if (it_table == m_tables.end())
    {
        return NULL;
    }

    return ((*it_table).second).m_szTableName;
}

/*******************************************************
  函 数 名: CModel::GetObjID
  描    述: 获取表所属对象ID
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CModel::GetObjID(DWORD attrID)
{
    AutoObjLock(this);

    IT_TABLES it_table = m_tables.find(attrID);
    if (it_table == m_tables.end())
    {
        return 0;
    }

    return ((*it_table).second).m_objID;
}

/*******************************************************
  函 数 名: CModel::GetType
  描    述: 获取表类型
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IModel::TYPE CModel::GetType(DWORD attrID)
{
    AutoObjLock(this);

    IT_TABLES it_table = m_tables.find(attrID);
    if (it_table == m_tables.end())
    {
        return IModel::TYPE_DATA;
    }

    return ((*it_table).second).m_attrType;
}

/*******************************************************
  函 数 名: CModel::GetFields
  描    述: 获取字段
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IModel::Field *CModel::GetFields(DWORD attrID, DWORD &rdwFieldCount)
{
    AutoObjLock(this);

    IT_TABLES it_table = m_tables.find(attrID);
    if (it_table == m_tables.end())
    {
        return NULL;
    }

    rdwFieldCount = ((*it_table).second).m_dwFieldCount;
    return ((*it_table).second).m_pFields;
}

/*******************************************************
  函 数 名: CModel::GetDefRecCount
  描    述: 获取默认记录数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CModel::GetDefRecCount(DWORD attrID)
{
    AutoObjLock(this);

    IT_TABLES it_table = m_tables.find(attrID);
    if (it_table == m_tables.end())
    {
        return 0;
    }

    return ((*it_table).second).m_dwDefRecCount;
}

/*******************************************************
  函 数 名: CModel::GetShips
  描    述: 获取关联字段
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IModel::Relation *CModel::GetShips(DWORD attrID, DWORD &rdwShipCount)
{
    AutoObjLock(this);

    IT_TABLES it_table = m_tables.find(attrID);
    if (it_table == m_tables.end())
    {
        return NULL;
    }

    rdwShipCount = ((*it_table).second).m_dwShipCount;
    return ((*it_table).second).m_pShips;
}

/*******************************************************
  函 数 名: CModel::OnStart
  描    述: 开始运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CModel::OnStart(objMsg *msg)
{
    m_bStartFlag = true;

    SendEvent();
}

/*******************************************************
  函 数 名: CModel::OnDefault
  描    述: 默认消息处理
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CModel::OnDefault(objMsg *msg)
{
}

/*******************************************************
  函 数 名: CModel::SendEvent
  描    述: 发送事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CModel::SendEvent(DWORD attrID)
{
    if (!m_piDispatch || !m_pNotifyPool)
    {
        return;
    }

    DWORD dwCount = (attrID)? 1 : (DWORD)m_tables.size();
    if (!dwCount)
    {
        return;
    }

    if (m_pNotifyPool->OnPublish(DCOP_MSG_MODEL_REG) != SUCCESS)
    {
        return;
    }

    /// 创建消息
    DWORD dwDataLen = sizeof(DWORD) * (dwCount + 1);
    DWORD dwMsgType = DCOP_MSG_MODEL_REG;
    objMsg *pMsg = DCOP_CreateMsg(dwDataLen,
                        dwMsgType,
                        ID());
    if (!pMsg)
    {
        return;
    }

    DWORD *pdwPara = (DWORD *)pMsg->GetDataBuf();
    if (!pdwPara)
    {
        delete pMsg;
        return;
    }

    /// 填充消息
    pdwPara[0] = dwCount;
    if (attrID)
    {
        pdwPara[1] = attrID;
    }
    else
    {
        DWORD i = 1;
        for (IT_TABLES it_table = m_tables.begin();
            it_table != m_tables.end(); ++it_table)
        {
            pdwPara[i++] = (*it_table).first;
            if (i > dwCount) break;
        }
    }

    /// 通知消息
    pMsg->MsgHead().m_dwDstID = DCOP_OBJECT_NOTIFY;
    DWORD dwRc = m_piDispatch->Send(pMsg);
    if (dwRc != SUCCESS)
    {
        delete pMsg;
        pMsg = 0;
    }
}

