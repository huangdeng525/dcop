/// -------------------------------------------------
/// test_data.cpp : 主要测试数据操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test_data.h"
#include "Factory_if.h"
#include "Manager_if.h"
#include "cpu/bytes.h"
#include "sock.h"


/// -------------------------------------------------
/// DATA 测试用例
/// -------------------------------------------------
TEST_SUITE_TABLE(DATA)
    TEST_SUITE_ITEM(CTestSuite_DATA)
        TEST_CASE_ITEM(1)
            "1"
        TEST_CASE_ITEM_END
    TEST_SUITE_ITEM_END
TEST_SUITE_TABLE_END

/// -------------------------------------------------
/// DATA 测试套
/// -------------------------------------------------
IMPLEMENT_REGTESTSUITE_FUNC(DATA)

/// -------------------------------------------------
/// 用户表名
/// -------------------------------------------------
char* CTestSuite_DATA::USER_TABLE_NAME = "用户表";

/// -------------------------------------------------
/// 用户字段描述
/// -------------------------------------------------
IModel::Field CTestSuite_DATA::UserFields[] = 
{
    {"用户名", IModel::KEY_NO, CTestSuite_DATA::USER_NAME_TYPE, CTestSuite_DATA::USER_NAME_SIZE, 0, 0, 0},
    {"用户ID", IModel::KEY_YES, CTestSuite_DATA::USER_ID_TYPE, CTestSuite_DATA::USER_ID_SIZE, 0, 0, 0},
    {"密码", IModel::KEY_NO, CTestSuite_DATA::USER_PASS_TYPE, CTestSuite_DATA::USER_PASS_SIZE, 0, 0, 0},
    {"等级", IModel::KEY_NO, CTestSuite_DATA::USER_LEVEL_TYPE, CTestSuite_DATA::USER_LEVEL_SIZE, 0, 0, 0},
    {"群组", IModel::KEY_NO, CTestSuite_DATA::USER_GROUP_TYPE, CTestSuite_DATA::USER_GROUP_SIZE, 0, 0, 0},
    {"信息", IModel::KEY_NO, CTestSuite_DATA::USER_INFO_TYPE, CTestSuite_DATA::USER_INFO_SIZE, 0, 0, 0},
};

/// -------------------------------------------------
/// 用户参数描述
/// -------------------------------------------------
DCOP_PARA_NODE CTestSuite_DATA::UserParas[] = 
{
    {CTestSuite_DATA::USER_NAME, 0, CTestSuite_DATA::USER_NAME_TYPE, CTestSuite_DATA::USER_NAME_SIZE},
    {CTestSuite_DATA::USER_ID, 0, CTestSuite_DATA::USER_ID_TYPE, CTestSuite_DATA::USER_ID_SIZE},
    {CTestSuite_DATA::USER_PASS, 0, CTestSuite_DATA::USER_PASS_TYPE, CTestSuite_DATA::USER_PASS_SIZE},
    {CTestSuite_DATA::USER_LEVEL, 0, CTestSuite_DATA::USER_LEVEL_TYPE, CTestSuite_DATA::USER_LEVEL_SIZE},
    {CTestSuite_DATA::USER_GROUP, 0, CTestSuite_DATA::USER_GROUP_TYPE, CTestSuite_DATA::USER_GROUP_SIZE},
    {CTestSuite_DATA::USER_INFO, 0, CTestSuite_DATA::USER_INFO_TYPE, CTestSuite_DATA::USER_INFO_SIZE},
};

/// -------------------------------------------------
/// 会话表名
/// -------------------------------------------------
char* CTestSuite_DATA::SESS_TABLE_NAME = "会话表";

/// -------------------------------------------------
/// 会话字段描述
/// -------------------------------------------------
IModel::Field CTestSuite_DATA::SessFields[] = 
{
    {"会话ID", IModel::KEY_YES, CTestSuite_DATA::SESS_ID_TYPE, CTestSuite_DATA::SESS_ID_SIZE, 0, 0, 0},
    {"用户ID", IModel::KEY_NO, CTestSuite_DATA::SESS_USER_TYPE, CTestSuite_DATA::SESS_USER_SIZE, 0, 0, 0},
    {"终端ID", IModel::KEY_NO, CTestSuite_DATA::SESS_TTY_TYPE, CTestSuite_DATA::SESS_TTY_SIZE, 0, 0, 0},
    {"IP地址", IModel::KEY_NO, CTestSuite_DATA::SESS_IP_TYPE, CTestSuite_DATA::SESS_IP_SIZE, 0, 0, 0},
    {"端口号", IModel::KEY_NO, CTestSuite_DATA::SESS_PORT_TYPE, CTestSuite_DATA::SESS_PORT_SIZE, 0, 0, 0},
    {"信息", IModel::KEY_NO, CTestSuite_DATA::SESS_INFO_TYPE, CTestSuite_DATA::SESS_INFO_SIZE, 0, 0, 0},
};

/// -------------------------------------------------
/// 会话参数描述
/// -------------------------------------------------
DCOP_PARA_NODE CTestSuite_DATA::SessParas[] = 
{
    {CTestSuite_DATA::SESS_ID, 0, CTestSuite_DATA::SESS_ID_TYPE, CTestSuite_DATA::SESS_ID_SIZE},
    {CTestSuite_DATA::SESS_USER, 0, CTestSuite_DATA::SESS_USER_TYPE, CTestSuite_DATA::SESS_USER_SIZE},
    {CTestSuite_DATA::SESS_TTY, 0, CTestSuite_DATA::SESS_TTY_TYPE, CTestSuite_DATA::SESS_TTY_SIZE},
    {CTestSuite_DATA::SESS_IP, 0, CTestSuite_DATA::SESS_IP_TYPE, CTestSuite_DATA::SESS_IP_SIZE},
    {CTestSuite_DATA::SESS_PORT, 0, CTestSuite_DATA::SESS_PORT_TYPE, CTestSuite_DATA::SESS_PORT_SIZE},
    {CTestSuite_DATA::SESS_INFO, 0, CTestSuite_DATA::SESS_INFO_TYPE, CTestSuite_DATA::SESS_INFO_SIZE},
};

/// -------------------------------------------------
/// 用户ID和终端ID作为关键字索引
/// -------------------------------------------------
DCOP_PARA_NODE CTestSuite_DATA::SessKeyUidTei[] = 
{
    {CTestSuite_DATA::SESS_USER, 0, CTestSuite_DATA::SESS_USER_TYPE, CTestSuite_DATA::SESS_USER_SIZE},
    {CTestSuite_DATA::SESS_TTY, 0, CTestSuite_DATA::SESS_TTY_TYPE, CTestSuite_DATA::SESS_TTY_SIZE}
};

/// -------------------------------------------------
/// IP和端口作为关键字索引
/// -------------------------------------------------
DCOP_PARA_NODE CTestSuite_DATA::SessKeyIPPort[] = 
{
    {CTestSuite_DATA::SESS_IP, 0, CTestSuite_DATA::SESS_IP_TYPE, CTestSuite_DATA::SESS_IP_SIZE},
    {CTestSuite_DATA::SESS_PORT, 0, CTestSuite_DATA::SESS_PORT_TYPE, CTestSuite_DATA::SESS_PORT_SIZE}
};


CTestSuite_DATA::CTestSuite_DATA()
{
    m_piManager = 0;
    m_pOwner = 0;
    m_piModel = 0;
    m_piDispatch = 0;
    m_piNotify = 0;
    m_piSchedule = 0;
    m_piData = 0;
    m_hUserData = 0;
    m_hSessData = 0;
}

CTestSuite_DATA::~CTestSuite_DATA()
{
    m_hUserData = 0;
    m_hSessData = 0;

    if (m_piManager) m_piManager->Fini();

    DCOP_RELEASE_INSTANCE_REFER(0, m_piManager);
}

void CTestSuite_DATA::InitModule()
{
    TEST_LOAD_OBJECT(m_piManager, IObject, TA, DCOP_OBJECT_CUSTOM + 1, m_pOwner);
    TEST_LOAD_OBJECT(m_piManager, IModel, model, DCOP_OBJECT_MODEL, m_piModel);
    TEST_LOAD_OBJECT(m_piManager, IData, data, DCOP_OBJECT_DATA, m_piData);
    TEST_LOAD_OBJECT(m_piManager, IDispatch, dispatch, DCOP_OBJECT_DISPATCH, m_piDispatch);
    TEST_LOAD_OBJECT(m_piManager, INotify, notify, DCOP_OBJECT_NOTIFY, m_piNotify);
    TEST_LOAD_OBJECT(m_piManager, ISchedule, schedule, DCOP_OBJECT_SCHEDULE, m_piSchedule);

    (void)m_piManager->Init(NULL, 0, 0);
}

void CTestSuite_DATA::BeforeTest()
{
    DCOP_CREATE_INSTANCE(IManager, "manager", NULL, 0, 0, m_piManager);
    if (!m_piManager)
    {
        return;
    }

    InitModule();

    InitModelData();
}

void CTestSuite_DATA::InitModelData()
{
    if (!m_piModel || !m_piData)
    {
        return;
    }

    /// 注册属性建模
    DWORD dwRc = m_piModel->RegTable(USER_TABLE_NAME, 
                        m_pOwner->ID(), 
                        USER_TABLE_ID, 
                        IModel::TYPE_DATA, 
                        UserFields, 
                        ARRAY_SIZE(UserFields), 
                        USER_REC_DEF_COUNT);
    printf("Reg Table('%s') Rc(0x%x)! \n", USER_TABLE_NAME, dwRc);
    if (dwRc != SUCCESS)
    {
        return;
    }

    /// 注册属性建模
    dwRc = m_piModel->RegTable(SESS_TABLE_NAME, 
                        m_pOwner->ID(), 
                        SESS_TABLE_ID, 
                        IModel::TYPE_DATA, 
                        SessFields, 
                        ARRAY_SIZE(SessFields), 
                        SESS_REC_DEF_COUNT);
    printf("Reg Table('%s') Rc(0x%x)! \n", SESS_TABLE_NAME, dwRc);
    if (dwRc != SUCCESS)
    {
        return;
    }

}

int CTestSuite_DATA::TestEntry(int argc, char* argv[])
{
    if (!m_piModel || !m_piData || !m_pOwner)
    {
        return -1;
    }

    /// 创建用户数据
    DWORD dwRc = m_piData->Create(IData::TYPE_FILE, 
                        USER_TABLE_ID, 
                        m_pOwner, 
                        m_hUserData);
    printf("Create Table(%d) Rc(0x%x)! \n", USER_TABLE_ID, dwRc);
    if (dwRc != SUCCESS)
    {
        return -3;
    }

    if (!m_hUserData)
    {
        return -4;
    }

    /// 创建会话数据
    dwRc = m_piData->Create(IData::TYPE_MEM, 
                        SESS_TABLE_ID, 
                        m_pOwner, 
                        m_hSessData);
    printf("Create Table(%d) Rc(0x%x)! \n", SESS_TABLE_ID, dwRc);
    if (dwRc != SUCCESS)
    {
        return -5;
    }

    if (!m_hSessData)
    {
        return -6;
    }

    /// 添加用户ID和终端ID作为关键字索引
    dwRc = m_piData->AddKeyIdx(m_hSessData, SessKeyUidTei, ARRAY_SIZE(SessKeyUidTei));
    printf("AddKeyIdx Table(%d) Rc(0x%x)! \n", SESS_TABLE_ID, dwRc);
    if (dwRc != SUCCESS)
    {
        return -7;
    }

    /// 添加IP和端口作为关键字索引
    dwRc = m_piData->AddKeyIdx(m_hSessData, SessKeyIPPort, ARRAY_SIZE(SessKeyIPPort));
    printf("AddKeyIdx Table(%d) Rc(0x%x)! \n", SESS_TABLE_ID, dwRc);
    if (dwRc != SUCCESS)
    {
        return -8;
    }

    /// 添加用户数据
    USER_RECORD_NODE addUserData[] = 
    {
        {"刘备", 0, "HaHaHa", 100, 1, "主公"},
        {"诸葛亮", 1, "HaHaHa", 98, 2, "军师"},
        {"关羽", 2, "HaHaHa", 97, 3, "主将"},
        {"张飞", 3, "HaHaHa", 96, 3, "主将"},
        {"赵云", 4, "HaHaHa", 96, 3, "主将"},
    };

    const BYTES_CHANGE_RULE UserRecBORule[] = 
    {
        {SIZE_OF(CTestSuite_DATA::USER_RECORD_NODE, UserID), OFFSET_OF(CTestSuite_DATA::USER_RECORD_NODE, UserID)},
        {SIZE_OF(CTestSuite_DATA::USER_RECORD_NODE, Level), OFFSET_OF(CTestSuite_DATA::USER_RECORD_NODE, Level)},
        {SIZE_OF(CTestSuite_DATA::USER_RECORD_NODE, Group), OFFSET_OF(CTestSuite_DATA::USER_RECORD_NODE, Group)}
    };

    for (DWORD i = 0; i < ARRAY_SIZE(addUserData); ++i)
    {
        Bytes_ChangeOrderByRule(UserRecBORule, ARRAY_SIZE(UserRecBORule), &(addUserData[i]), sizeof(addUserData[i]));
        dwRc = m_piData->AddRecord(m_hUserData, 
                        UserParas, 
                        ARRAY_SIZE(UserParas), 
                        &(addUserData[i]), 
                        sizeof(addUserData[i]));
        printf("Add User '%s' Rc(0x%x)! \n", addUserData[i].UserName, dwRc);
    }

    /// 添加会话数据
    SESS_RECORD_NODE addSessData[] = 
    {
        {0, 0, 0x101, objSock::GetIPValueByString("1.1.1.1"), 200, "在线"},
        {0, 1, 0x102, objSock::GetIPValueByString("1.1.1.2"), 200, "在线"},
        {0, 2, 0x101, objSock::GetIPValueByString("1.1.1.3"), 200, "在线"},
        {0, 3, 0x101, objSock::GetIPValueByString("1.1.1.3"), 201, "在线"},
        {0, 4, 0x103, objSock::GetIPValueByString("1.1.1.3"), 202, "在线"},
    };

    const BYTES_CHANGE_RULE SessRecBORule[] = 
    {
        {SIZE_OF(CTestSuite_DATA::SESS_RECORD_NODE, SessID), OFFSET_OF(CTestSuite_DATA::SESS_RECORD_NODE, SessID)},
        {SIZE_OF(CTestSuite_DATA::SESS_RECORD_NODE, UserID), OFFSET_OF(CTestSuite_DATA::SESS_RECORD_NODE, UserID)},
        {SIZE_OF(CTestSuite_DATA::SESS_RECORD_NODE, TTY), OFFSET_OF(CTestSuite_DATA::SESS_RECORD_NODE, TTY)},
        {SIZE_OF(CTestSuite_DATA::SESS_RECORD_NODE, Port), OFFSET_OF(CTestSuite_DATA::SESS_RECORD_NODE, Port)}
    };

    for (DWORD i = 0; i < ARRAY_SIZE(addUserData); ++i)
    {
        Bytes_ChangeOrderByRule(SessRecBORule, ARRAY_SIZE(SessRecBORule), &(addSessData[i]), sizeof(addSessData[i]));
        dwRc = m_piData->AddRecord(m_hSessData, 
                        SessParas, 
                        ARRAY_SIZE(SessParas), 
                        &(addSessData[i]), 
                        sizeof(addSessData[i]));
        printf("Add Sess '%s' Rc(0x%x)! \n", addUserData[i].UserName, dwRc);
    }

    m_piData->Dump(PrintToConsole, 0, 0, 0);

    BYTE condSessIPPortData[6];
    *(DWORD *)(condSessIPPortData) = objSock::GetIPValueByString("1.1.1.3");
    Bytes_SetWord(condSessIPPortData + 4, 200);
    DCOP_PARA_NODE *pRspPara = 0;
    DWORD dwRspParaCount = 0;
    CDArray aRspData;
    dwRc = m_piData->QueryRecord(m_hSessData, 
                        DCOP_CONDITION_ONE, 
                        SessKeyIPPort, 
                        ARRAY_SIZE(SessKeyIPPort), 
                        condSessIPPortData, 
                        sizeof(condSessIPPortData), 
                        SessParas, 
                        ARRAY_SIZE(SessParas), 
                        pRspPara, 
                        dwRspParaCount, 
                        aRspData);
    if (dwRc)
    {
        return -6;
    }

    for (DWORD i = 0; i < aRspData.Count(); ++i)
    {
        SESS_RECORD_NODE *pSessRec = (SESS_RECORD_NODE *)aRspData.Pos(i);
        Bytes_ChangeOrderByRule(SessRecBORule, ARRAY_SIZE(SessRecBORule), pSessRec, sizeof(SESS_RECORD_NODE));
        char szIP[OSSOCK_IPSIZE];
        (void)memset(szIP, 0, sizeof(szIP));
        objSock::GetIPStringByValue(pSessRec->IP, szIP);
        printf("姓名: %s，ID:%d，会话:%d，终端:0x%x，IP:%s，Port:%d, 信息:%s \n", 
                        addUserData[pSessRec->UserID].UserName, 
                        pSessRec->UserID, 
                        pSessRec->SessID, 
                        pSessRec->TTY, 
                        szIP, 
                        pSessRec->Port, 
                        pSessRec->Info);
    }

    if (pRspPara) DCOP_Free(pRspPara);

    return 0;
}

