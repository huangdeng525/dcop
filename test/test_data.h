/// -------------------------------------------------
/// test_data.h : 主要测试数据操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TEST_DATA_H_
#define _TEST_DATA_H_

#include "test.h"
#include "ObjData_if.h"
#include "Object_if.h"
#include "Manager_if.h"
#include "ObjModel_if.h"
#include "ObjDispatch_if.h"
#include "ObjNotify_if.h"
#include "ObjSchedule_if.h"
#include "BaseMessage.h"
#include "cpu/bytes.h"


/// 测试Data
class CTestSuite_DATA : public ITestSuite
{
public:
    /////////////////////////////////////////////////////
    /// 用户表
    /////////////////////////////////////////////////////

    /// 用户表名、属性ID、缺省记录数
    static char* USER_TABLE_NAME;
    static const DWORD USER_TABLE_ID = 1;
    static const DWORD USER_REC_DEF_COUNT = 100;

    /// 用户字段类型
    static const BYTE USER_NAME_TYPE = IModel::FIELD_STRING;
    static const BYTE USER_ID_TYPE = IModel::FIELD_NUMBER;
    static const BYTE USER_PASS_TYPE = IModel::FIELD_STRING;
    static const BYTE USER_LEVEL_TYPE = IModel::FIELD_NUMBER;
    static const BYTE USER_GROUP_TYPE = IModel::FIELD_NUMBER;
    static const BYTE USER_INFO_TYPE = IModel::FIELD_STRING;

    /// 用户字段大小
    static const WORD USER_NAME_SIZE = 32;
    static const WORD USER_ID_SIZE = 4;
    static const WORD USER_PASS_SIZE = 32;
    static const WORD USER_LEVEL_SIZE = 4;
    static const WORD USER_GROUP_SIZE = 4;
    static const WORD USER_INFO_SIZE = 64;

    /// 用户字段ID
    enum USER_FIELD_ID
    {
        USER_NAME = 1,
        USER_ID,
        USER_PASS,
        USER_LEVEL,
        USER_GROUP,
        USER_INFO
    };

    /// 用户记录节点
    struct USER_RECORD_NODE
    {
        char UserName[USER_NAME_SIZE];
        DWORD UserID;
        char PassText[USER_PASS_SIZE];
        DWORD Level;
        DWORD Group;
        char Info[USER_INFO_SIZE];
    };

    /// 用户字段描述
    static IModel::Field UserFields[];

    /// 用户参数描述
    static DCOP_PARA_NODE UserParas[];


    /////////////////////////////////////////////////////
    /// 会话表
    /////////////////////////////////////////////////////

    /// 会话表名、属性ID、缺省记录数
    static char* SESS_TABLE_NAME;
    static const DWORD SESS_TABLE_ID = 2;
    static const DWORD SESS_REC_DEF_COUNT = 100;

    /// 会话字段类型
    static const BYTE SESS_ID_TYPE = IModel::FIELD_IDENTIFY;
    static const BYTE SESS_USER_TYPE = IModel::FIELD_NUMBER;
    static const BYTE SESS_TTY_TYPE = IModel::FIELD_DWORD;
    static const BYTE SESS_IP_TYPE = IModel::FIELD_IP;
    static const BYTE SESS_PORT_TYPE = IModel::FIELD_NUMBER;
    static const BYTE SESS_INFO_TYPE = IModel::FIELD_STRING;

    /// 会话字段大小
    static const WORD SESS_ID_SIZE = 4;
    static const WORD SESS_USER_SIZE = 4;
    static const WORD SESS_TTY_SIZE = 4;
    static const WORD SESS_IP_SIZE = 4;
    static const WORD SESS_PORT_SIZE = 2;
    static const WORD SESS_INFO_SIZE = 22;

    /// 会话字段ID
    enum SESS_FIELD_ID
    {
        SESS_ID = 1,
        SESS_USER,
        SESS_TTY,
        SESS_IP,
        SESS_PORT,
        SESS_INFO
    };

    /// 会话记录节点
    struct SESS_RECORD_NODE
    {
        DWORD SessID;
        DWORD UserID;
        DWORD TTY;
        DWORD IP;
        WORD Port;
        char Info[SESS_INFO_SIZE];
    };

    /// 会话字段描述
    static IModel::Field SessFields[];

    /// 会话参数描述
    static DCOP_PARA_NODE SessParas[];

    /// 用户ID和终端ID作为关键字索引
    static DCOP_PARA_NODE SessKeyUidTei[];

    /// IP和端口作为关键字索引
    static DCOP_PARA_NODE SessKeyIPPort[];


public:
    CTestSuite_DATA();
    ~CTestSuite_DATA();

    void BeforeTest();

    int TestEntry(int argc, char* argv[]);

private:
    void InitModule();
    void InitModelData();

private:
    IManager *m_piManager;
    IObject *m_pOwner;
    IModel *m_piModel;
    IDispatch *m_piDispatch;
    INotify *m_piNotify;
    ISchedule *m_piSchedule;
    IData *m_piData;
    IData::Handle m_hUserData;
    IData::Handle m_hSessData;
};


#endif // #ifndef _TEST_DATA_H_

