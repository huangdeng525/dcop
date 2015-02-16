/// -------------------------------------------------
/// user_main.cpp : 用户管理实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "user_main.h"
#include "Factory_if.h"
#include "Manager_if.h"
#include "md5/md5.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CUser, "user")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CUser)
    DCOP_IMPLEMENT_INTERFACE(IUser)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CUser)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
    IMPLEMENT_CONFIG_DATATYPE("userdatatype", users)
DCOP_IMPLEMENT_IOBJECT_END

/// -------------------------------------------------
/// 实现属性
/// -------------------------------------------------
IMPLEMENT_ATTRIBUTE(CUser, users, CUser::USER_TABLE_ID, IModel::TYPE_DATA)

/// -------------------------------------------------
/// 实现消息分发
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE(CUser)
    IMPLEMENT_ATTRIBUTE_MSG_PROC(userIndex)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_START, OnStart)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_FINISH, OnFinish)
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE_END

/// -------------------------------------------------
/// 用户表名
/// -------------------------------------------------
const char* CUser::USER_TABLE_NAME = "users";

/// -------------------------------------------------
/// 用户字段描述
/// -------------------------------------------------
IModel::Field CUser::UserFields[] = 
{
    {"username", IModel::KEY_NO, CUser::USER_NAME_TYPE, CUser::USER_NAME_SIZE, 0, 0, 0},
    {"userid", IModel::KEY_YES, CUser::USER_ID_TYPE, CUser::USER_ID_SIZE, 0, 0, 0},
    {"password", IModel::KEY_NO, CUser::USER_PASS_TYPE, CUser::USER_PASS_SIZE, 0, 0, 0},
    {"level", IModel::KEY_NO, CUser::USER_LEVEL_TYPE, CUser::USER_LEVEL_SIZE, 0, 0, 0},
    {"usergroup", IModel::KEY_NO, CUser::USER_GROUP_TYPE, CUser::USER_GROUP_SIZE, 0, 0, 0},
    {"info", IModel::KEY_NO, CUser::USER_INFO_TYPE, CUser::USER_INFO_SIZE, 0, 0, 0}
};

/// -------------------------------------------------
/// 用户参数描述
/// -------------------------------------------------
DCOP_PARA_NODE CUser::UserParas[] = 
{
    {CUser::USER_NAME, 0, CUser::USER_NAME_TYPE, CUser::USER_NAME_SIZE},
    {CUser::USER_ID, 0, CUser::USER_ID_TYPE, CUser::USER_ID_SIZE},
    {CUser::USER_PASS, 0, CUser::USER_PASS_TYPE, CUser::USER_PASS_SIZE},
    {CUser::USER_LEVEL, 0, CUser::USER_LEVEL_TYPE, CUser::USER_LEVEL_SIZE},
    {CUser::USER_GROUP, 0, CUser::USER_GROUP_TYPE, CUser::USER_GROUP_SIZE},
    {CUser::USER_INFO, 0, CUser::USER_INFO_TYPE, CUser::USER_INFO_SIZE}
};

/// -------------------------------------------------
/// 用户名校验文参数描述
/// -------------------------------------------------
DCOP_PARA_NODE CUser::UserNamePassParas[] = 
{
    {CUser::USER_NAME, 0, CUser::USER_NAME_TYPE, CUser::USER_NAME_SIZE},
    {CUser::USER_PASS, 0, CUser::USER_PASS_TYPE, CUser::USER_PASS_SIZE}
};

/// -------------------------------------------------
/// 用户名作为关键字索引
/// -------------------------------------------------
DCOP_PARA_NODE CUser::UserKeyName[] = 
{
    {CUser::USER_NAME, 0, CUser::USER_NAME_TYPE, CUser::USER_NAME_SIZE}
};

/// -------------------------------------------------
/// 用户ID作为关键字索引
/// -------------------------------------------------
DCOP_PARA_NODE CUser::UserKeyID[] = 
{
    {CUser::USER_ID, 0, CUser::USER_ID_TYPE, CUser::USER_ID_SIZE}
};


/*******************************************************
  函 数 名: CUser::BytesChangeRecord
  描    述: 转换记录字节序
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CUser::BytesChangeRecord(NODE *pRec)
{
    const BYTES_CHANGE_RULE UserRecBORule[] = 
    {
        {SIZE_OF(IUser::NODE, UserID), OFFSET_OF(IUser::NODE, UserID)},
        {SIZE_OF(IUser::NODE, Level), OFFSET_OF(IUser::NODE, Level)},
        {SIZE_OF(IUser::NODE, Group), OFFSET_OF(IUser::NODE, Group)}
    };
    Bytes_ChangeOrderByRule(UserRecBORule, ARRAY_SIZE(UserRecBORule), pRec, sizeof(NODE));
}

/*******************************************************
  函 数 名: CUser::CUser
  描    述: CUser构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CUser::CUser(Instance *piParent, int argc, char **argv) : 
    m_users(IData::TYPE_MYSQL)
{
    m_piModel = 0;
    m_piData = 0;

    m_piDispatch = 0;
    m_piNotify = 0;
    m_pNotifyPool = 0;

    m_piSecure = 0;

    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);
}

/*******************************************************
  函 数 名: CUser::~CUser
  描    述: CUser析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CUser::~CUser()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CUser::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CUser::Init(IObject *root, int argc, void **argv)
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
        DCOP_QUERY_OBJECT_ITEM(ISecure,      DCOP_OBJECT_SECURE,     m_piSecure)
    DCOP_QUERY_OBJECT_END

    /// 创建缓冲池
    DWORD adwEvents[] = 
    {
        DCOP_OBJATTR_USER_TABLE
    };
    m_pNotifyPool = m_piNotify->CreatePool(this, adwEvents, ARRAY_SIZE(adwEvents));
    if (!m_pNotifyPool)
    {
        return FAILURE;
    }

    /// 初始化属性
    INIT_ATTRIBUTE_START(userIndex, m_piDispatch, m_pNotifyPool)
        INIT_ATTRIBUTE_MEMBER(users)
    INIT_ATTRIBUTE_END

    /// 设置数据对象
    m_users.SetDataObj(m_piData);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CUser::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CUser::Fini()
{
    if (m_piNotify && m_pNotifyPool)
    {
        m_piNotify->DestroyPool(m_pNotifyPool);
        m_pNotifyPool = 0;
    }

    DCOP_RELEASE_INSTANCE(m_piSecure);
    DCOP_RELEASE_INSTANCE(m_piNotify);
    DCOP_RELEASE_INSTANCE(m_piDispatch);
    DCOP_RELEASE_INSTANCE(m_piData);
    DCOP_RELEASE_INSTANCE(m_piModel);
}

/*******************************************************
  函 数 名: CUser::OnStart
  描    述: 开始运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CUser::OnStart(objMsg *msg)
{
    DWORD dwRc = InitModelData();
    CHECK_ERRCODE(dwRc, "Init Model Data");

    if (m_piSecure)
    {
        ISecure::Node node[] = 
        {
            {m_users.GetAttribute()->GetID(), 
                0, 0, 0, 0, 0, 0, 
                DCOP_GROUP_ADMINISTRATOR},
        };
        dwRc = m_piSecure->RegRule(node, ARRAY_SIZE(node));
        CHECK_ERRCODE(dwRc, "Reg Sceure Rule");
    }
}

/*******************************************************
  函 数 名: CUser::OnFinish
  描    述: 结束运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CUser::OnFinish(objMsg *msg)
{
    (void)m_users.Destroy();
}

/*******************************************************
  函 数 名: CUser::CreateUser
  描    述: 创建用户
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CUser::CreateUser(const char *cszUserName,
                        const char *cszPassText,
                        DWORD dwLevel,
                        DWORD dwGroup,
                        DWORD &rdwUserID)
{
    /// 添加记录
    NODE addUserData = 
    {
        "", 0, "", dwLevel, dwGroup, ""
    };
    (void)strncpy(addUserData.UserName, cszUserName, sizeof(addUserData.UserName));
    addUserData.UserName[sizeof(addUserData.UserName) - 1] = '\0';
    (void)strncpy(addUserData.PassText, cszPassText, sizeof(addUserData.PassText));
    addUserData.PassText[sizeof(addUserData.PassText) - 1] = '\0';
    BytesChangeRecord(&addUserData);

    AutoObjLock(this);

    DWORD dwRc = m_users.AddRecord(UserParas, ARRAY_SIZE(UserParas), 
                        &addUserData, sizeof(addUserData));
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 根据用户名查找记录
    dwRc = FindUser(cszUserName, addUserData);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    rdwUserID = addUserData.UserID;
    return SUCCESS;
}

/*******************************************************
  函 数 名: CUser::DeleteUser
  描    述: 删除用户
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CUser::DeleteUser(DWORD dwUserID)
{
    BYTE condData[4];
    Bytes_SetDword(condData, dwUserID);

    AutoObjLock(this);

    return m_users.DelRecord(DCOP_CONDITION_ONE, 
                        UserKeyID, ARRAY_SIZE(UserKeyID), 
                        condData, sizeof(condData));
}

/*******************************************************
  函 数 名: CUser::FindUser
  描    述: 查找用户
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CUser::FindUser(const char *cszUserName,
                        NODE &rNode)
{
    if (!cszUserName)
    {
        return FAILURE;
    }

    char szUserName[NAME_SIZE];
    (void)strncpy(szUserName, cszUserName, sizeof(szUserName));
    szUserName[sizeof(szUserName) - 1] = '\0';
    DCOP_PARA_NODE *pRspPara = 0;
    DWORD dwRspParaCount = 0;
    CDArray aRspData;

    AutoObjLock(this);

    DWORD dwRc = m_piData->QueryRecord(m_users, 
                        DCOP_CONDITION_ONE, 
                        UserKeyName, ARRAY_SIZE(UserKeyName), 
                        szUserName, NAME_SIZE, 
                        UserParas, ARRAY_SIZE(UserParas), 
                        pRspPara, 
                        dwRspParaCount, 
                        aRspData);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 这里不需要响应字段，直接释放
    if (pRspPara) DCOP_Free(pRspPara);

    if (aRspData.Count() == 0)
    {
        return FAILURE;
    }

    NODE *pNode = (NODE *)aRspData.Pos(0);
    if (!pNode)
    {
        return FAILURE;
    }

    BytesChangeRecord(pNode);
    rNode = *pNode;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CUser::CheckPass
  描    述: 检查校验字
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CUser::CheckPass(const char *cszUserName,
                        const char *cszPassText,
                        NODE *pNode)
{
    if (!cszUserName || !cszPassText)
    {
        return FAILURE;
    }

    NODE nodeTmp;

    AutoObjLock(this);

    DWORD dwRc = FindUser(cszUserName, nodeTmp);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    MD5_CTX md5;
    MD5Init(&md5);

    char digest[16];
    MD5Update(&md5, (unsigned  char *)nodeTmp.PassText, 16);
    MD5Update(&md5, (unsigned  char *)cszPassText, (unsigned int)strlen(cszPassText));
    MD5Final(&md5, (unsigned  char *)digest);

    if (memcmp(digest, nodeTmp.PassText + 16, 16) != 0)
    {
        return FAILURE;
    }

    if (pNode)
    {
        (void)memcpy(pNode, &nodeTmp, sizeof(NODE));
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CUser::ChangePass
  描    述: 修改校验字
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CUser::ChangePass(DWORD dwUserID,
                        char szUserName[NAME_SIZE],
                        char szOldPass[PASS_SIZE],
                        char szNewPass[PASS_SIZE])
{
    AutoObjLock(this);

    /// 先进行校验
    DWORD dwRc = CheckPass(szUserName, szOldPass);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 设置新校验字
    BYTE condData[4];
    Bytes_SetDword(condData, dwUserID);

    DCOP_PARA_NODE UserPass[] = 
    {
        {CUser::USER_PASS, 0, CUser::USER_PASS_TYPE, CUser::USER_PASS_SIZE}
    };

    return m_users.EditRecord(DCOP_CONDITION_ONE, 
                        UserKeyID, ARRAY_SIZE(UserKeyID), 
                        condData, sizeof(condData),
                        UserPass, ARRAY_SIZE(UserPass),
                        szNewPass, PASS_SIZE);
}

/*******************************************************
  函 数 名: CUser::GetUser
  描    述: 获取用户
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CUser::GetUser(DWORD dwUserID,
                        NODE &rNode)
{
    BYTE condData[4];
    Bytes_SetDword(condData, dwUserID);

    DCOP_PARA_NODE *pRspPara = 0;
    DWORD dwRspParaCount = 0;
    CDArray aRspData;

    AutoObjLock(this);

    DWORD dwRc = m_piData->QueryRecord(m_users, 
                        DCOP_CONDITION_ONE, 
                        UserKeyID, ARRAY_SIZE(UserKeyID), 
                        condData, sizeof(condData), 
                        UserParas, ARRAY_SIZE(UserParas), 
                        pRspPara, 
                        dwRspParaCount, 
                        aRspData);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 这里不需要响应字段，直接释放
    if (pRspPara) DCOP_Free(pRspPara);

    if (aRspData.Count() == 0)
    {
        return FAILURE;
    }

    NODE *pNode = (NODE *)aRspData.Pos(0);
    if (!pNode)
    {
        return FAILURE;
    }

    BytesChangeRecord(pNode);
    rNode = *pNode;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CUser::InitModelData
  描    述: 初始化模型和数据
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CUser::InitModelData()
{
    if (!m_piModel || !m_piData)
    {
        return FAILURE;
    }

    /// 注册属性建模
    DWORD dwRc = m_piModel->RegTable((char *)USER_TABLE_NAME, 
                        ID(), 
                        m_users.GetAttribute()->GetID(), 
                        m_users.GetModelType(), 
                        UserFields, 
                        ARRAY_SIZE(UserFields), 
                        USER_REC_DEF_COUNT);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 创建数据
    dwRc = m_users.Create();
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 添加用户名作为关键字索引
    dwRc = m_piData->AddKeyIdx(m_users, UserKeyName, ARRAY_SIZE(UserKeyName));
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    return SUCCESS;
}

