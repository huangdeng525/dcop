/// -------------------------------------------------
/// session_main.h : 会话管理私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _SESSION_MAIN_H_
#define _SESSION_MAIN_H_

#include "session_if.h"
#include "ObjAttribute_if.h"
#include "ObjResponse_if.h"
#include "cpu/bytes.h"


/// 会话管理实现类
class CSession : public ISession
{
public:
    /////////////////////////////////////////////////////
    /// 会话表
    /////////////////////////////////////////////////////

    /// 会话表名、属性ID、缺省记录数
    static const char* SESS_TABLE_NAME;
    static const DWORD SESS_TABLE_ID = DCOP_OBJATTR_SESSION_TABLE;
    static const DWORD SESS_REC_DEF_COUNT = 100;
    static const DWORD RELATION_USER = 2;

    /// 会话字段类型
    static const BYTE SESS_ID_TYPE = IModel::FIELD_IDENTIFY;
    static const BYTE SESS_USER_TYPE = IModel::FIELD_NUMBER;
    static const BYTE SESS_GROUP_TYPE = IModel::FIELD_NUMBER;
    static const BYTE SESS_TTY_TYPE = IModel::FIELD_NUMBER;
    static const BYTE SESS_TIMER_TYPE = IModel::FIELD_TIMER;
    static const BYTE SESS_IP_TYPE = IModel::FIELD_IP;
    static const BYTE SESS_PORT_TYPE = IModel::FIELD_NUMBER;
    static const BYTE SESS_INFO_TYPE = IModel::FIELD_STRING;

    /// 会话字段大小
    static const WORD SESS_ID_SIZE = SIZE_OF(NODE, SessID);
    static const WORD SESS_USER_SIZE = SIZE_OF(NODE, UserID);
    static const WORD SESS_GROUP_SIZE = SIZE_OF(NODE, UserGroup);
    static const WORD SESS_TTY_SIZE = SIZE_OF(NODE, TTY);
    static const WORD SESS_TIMER_SIZE = SIZE_OF(NODE, Timer);
    static const WORD SESS_IP_SIZE = SIZE_OF(NODE, IP);
    static const WORD SESS_PORT_SIZE = SIZE_OF(NODE, Port);
    static const WORD SESS_INFO_SIZE = INFO_SIZE;

    /// 会话字段ID
    enum SESS_FIELD_ID
    {
        SESS_ID = 1,
        SESS_USER,
        SESS_GROUP,
        SESS_TTY,
        SESS_TIMER,
        SESS_IP,
        SESS_PORT,
        SESS_INFO
    };

    /// 会话字段描述
    static IModel::Field SessFields[];

    /// 关联字段描述
    static IModel::Relation SessShips[];

    /// 会话参数描述
    static DCOP_PARA_NODE SessParas[];

    /// 会话用户描述
    static DCOP_PARA_NODE SessUser[];

    /// 会话定时器描述
    static DCOP_PARA_NODE SessTimer[];

    /// 会话信息描述
    static DCOP_PARA_NODE SessInfo[];

    /// IP和端口作为关键字索引
    static DCOP_PARA_NODE SessKeyIPPort[];

    /// SessionID作为关键字索引
    static DCOP_PARA_NODE SessKeyID[];

    /// 会话数据中同一条记录中需要转换字节序的规则
    static void BytesChangeRecord(NODE *pRec);

    /// 会话中缓存请求数量
    static const DWORD REQ_POOL_COUNT = 2048;

    /// 定时器轮索引、ID、基值和槽位数量
    static const DWORD WHEEL_S_SEC = 0;
    static const DWORD WHEEL_S_SEC_ID = 1;
    static const DWORD WHEEL_S_HASH_BASE = 1;
    static const DWORD WHEEL_S_SEC_SLOT_COUNT = 32;

    /// TimerValue是注册的定时器轮节点值
    struct TimerValue
    {
        DWORD m_dwSessID;                   // 会话ID
        DWORD m_dwTtyID;                    // 接入ID
    };

    /// 会话超时时间(单位:秒)
    static const DWORD SESS_TIMEOUT = 300;

public:
    CSession(Instance *piParent, int argc, char **argv);
    ~CSession();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DCOP_DECLARE_IOBJECT_MSG_HANDLE;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();

    void OnStart(objMsg *msg);
    void OnFinish(objMsg *msg);
    void OnTimer1s(objMsg *msg);

    DWORD CreateSession(DWORD dwUserID,
                        DWORD dwUserGroup,
                        DWORD dwTTY,
                        DWORD dwRemoteIP,
                        WORD wRemotePort,
                        DWORD &rdwSessionID);

    DWORD DeleteSession(DWORD dwSessionID);

    DWORD UpdateSession(DWORD dwSessionID);

    DWORD UpdateUserID(DWORD dwSessionID,
                        DWORD dwUserID,
                        DWORD dwUserGroup);

    DWORD FindSession(DWORD dwRemoteIP,
                        WORD wRemotePort,
                        NODE &rNode);

    DWORD GetSession(DWORD dwSessionID,
                        NODE &rNode);

    DWORD SetSessionInfo(DWORD dwSessionID,
                        char szInfo[INFO_SIZE]);

    DWORD GetSessionInfo(DWORD dwSessionID,
                        char szInfo[INFO_SIZE]);

    DECLARE_ATTRIBUTE_INDEX(sessionIndex);
    DECLARE_ATTRIBUTE(IData*, sessions);

private:
    DWORD InitModelData();
    DWORD InsertToWheel(DWORD dwTimeOut, ITimer::Handle hTimer);
    void  DelFromWheel(ITimer::Handle hTimer);
    static void OnWheelTimeout(ITimer::Handle handle, void *para);
    IDispatch *GetDispatch() {return m_piDispatch;}

private:
    IModel *m_piModel;                              // 模型管理
    IData *m_piData;                                // 数据中心

    IDispatch *m_piDispatch;                        // 消息分发器
    INotify *m_piNotify;                            // 事件通知器
    INotify::IPool *m_pNotifyPool;                  // 事件缓冲池

    ITimer *m_piTimer;                              // 定时器
    ITimer::Handle m_hTimer1s;
    ITimer::IWheel *m_pTimerWheel;
};



#endif // #ifndef _SESSION_MAIN_H_

