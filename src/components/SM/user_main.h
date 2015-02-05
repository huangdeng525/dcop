/// -------------------------------------------------
/// user_main.h : 用户管理私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _USER_MAIN_H_
#define _USER_MAIN_H_

#include "user_if.h"
#include "ObjAttribute_if.h"
#include "secure_if.h"


/// -------------------------------------------------
/// 用户管理实现类
/// -------------------------------------------------
class CUser : public IUser
{
public:
    /////////////////////////////////////////////////////
    /// 用户表
    /////////////////////////////////////////////////////

    /// 用户表名、属性ID、缺省记录数
    static const char* USER_TABLE_NAME;
    static const DWORD USER_TABLE_ID = DCOP_OBJATTR_USER_TABLE;
    static const DWORD USER_REC_DEF_COUNT = 100;

    /// 用户字段类型
    static const BYTE USER_NAME_TYPE = IModel::FIELD_STRING;
    static const BYTE USER_ID_TYPE = IModel::FIELD_IDENTIFY;
    static const BYTE USER_PASS_TYPE = IModel::FIELD_PASS;
    static const BYTE USER_LEVEL_TYPE = IModel::FIELD_NUMBER;
    static const BYTE USER_GROUP_TYPE = IModel::FIELD_NUMBER;
    static const BYTE USER_INFO_TYPE = IModel::FIELD_STRING;

    /// 用户字段大小
    static const WORD USER_NAME_SIZE = SIZE_OF(NODE, UserName);
    static const WORD USER_ID_SIZE = SIZE_OF(NODE, UserID);
    static const WORD USER_PASS_SIZE = SIZE_OF(NODE, PassText);
    static const WORD USER_LEVEL_SIZE = SIZE_OF(NODE, Level);
    static const WORD USER_GROUP_SIZE = SIZE_OF(NODE, Group);
    static const WORD USER_INFO_SIZE = SIZE_OF(NODE, Info);

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

    /// 用户字段描述
    static IModel::Field UserFields[];

    /// 用户参数描述
    static DCOP_PARA_NODE UserParas[];

    /// 用户名校验文参数描述
    static DCOP_PARA_NODE UserNamePassParas[];

    /// 用户名校验文参数
    struct USER_NAME_PASS
    {
        char UserName[NAME_SIZE];
        char PassText[PASS_SIZE];
    };

    /// 用户名作为关键字索引
    static DCOP_PARA_NODE UserKeyName[];

    /// 用户ID作为关键字索引
    static DCOP_PARA_NODE UserKeyID[];

    /// 用户数据中同一条记录中需要转换字节序的规则
    static void BytesChangeRecord(NODE *pRec);

public:
    CUser(Instance *piParent, int argc, char **argv);
    ~CUser();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DCOP_DECLARE_IOBJECT_MSG_HANDLE;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();

    void OnStart(objMsg *msg);
    void OnFinish(objMsg *msg);

    DWORD CreateUser(const char *cszUserName,
                        const char *cszPassText,
                        DWORD dwLevel,
                        DWORD dwGroup,
                        DWORD &rdwUserID);

    DWORD DeleteUser(DWORD dwUserID);

    DWORD FindUser(const char *cszUserName,
                        NODE &rNode);

    DWORD CheckPass(const char *cszUserName,
                        const char *cszPassText,
                        NODE *pNode = 0);

    DWORD ChangePass(DWORD dwUserID,
                        char szUserName[NAME_SIZE],
                        char szOldPass[PASS_SIZE],
                        char szNewPass[PASS_SIZE]);

    DWORD GetUser(DWORD dwUserID,
                        NODE &rNode);

    DECLARE_ATTRIBUTE_INDEX(userIndex);
    DECLARE_ATTRIBUTE(IData*, users);

private:
    DWORD InitModelData();

private:
    IModel *m_piModel;                              // 模型管理
    IData *m_piData;                                // 数据中心
    IData::TYPE m_userDataType;                     // 数据类型

    IDispatch *m_piDispatch;                        // 消息分发器
    INotify *m_piNotify;                            // 事件通知器
    INotify::IPool *m_pNotifyPool;                  // 事件缓冲池

    ISecure *m_piSecure;                            // 安全管理
};


#endif // #ifndef _USER_MAIN_H_

