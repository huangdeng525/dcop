/// -------------------------------------------------
/// command_main.h : 命令行接入私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _COMMAND_MAIN_H_
#define _COMMAND_MAIN_H_

#define INC_STRING
#define INC_MAP

#include "command_if.h"
#include "stream/dstream.h"
#include "string/tablestring.h"
#include "Manager_if.h"
#include "ObjDispatch_if.h"
#include "ObjNotify_if.h"
#include "ObjModel_if.h"
#include "ObjStatus_if.h"
#include "ObjResponse_if.h"
#include "user_if.h"
#include "session_if.h"
#include "access_if.h"
#include "sock.h"


/// -------------------------------------------------
/// 命令行接入管理实现类
/// -------------------------------------------------
class CCommand : public ICommand
{
public:
    /// 命令组成定义
    enum ELEMENT
    {
        ELEMENT_OPERATION = 0,
        ELEMENT_ATTRIBUTE,
        ELEMENT_REQ_ARG_LIST,
        ELEMENT_COND_ARG_LIST,
    };

    /// 命令分隔符列表
    static const char *SplitCharList;

    /// 命令操作类型定义
    static const char *Operation[];

    /// 参数操作码字符集
    static const char *ArgOpCodeCharList;

    /// 参数操作码类型定义
    static const char *ArgOpCode[];

    /// 请求缓存数量
    static const DWORD REQ_POOL_COUNT = 2048;

    /// 本地接入的IP和端口
    static const DWORD LOCAL_IP = 0x7F000001;
    static const WORD LOCAL_PORT = 0;

    /// Telnet Server通道和端口
    static const DWORD TELNET_CHANNEL = 1;
    static const WORD TELNET_PORT = 23;

    /// 网络任务数量
    static const DWORD LAN_TASK_COUNT = 1;

    /// 会话登陆重试最大次数
    static const BYTE SESS_LOGIN_TRY_TIMES_MAX = 10;

    /// 会话状态
    enum SESS_STATE
    {
        SESS_STATE_INPUT_USERNAME,
        SESS_STATE_INPUT_PASSTEXT,
        SESS_STATE_LOGIN_FAILURE,
        SESS_STATE_LOGIN_SUCCESS,
    };

    /// 会话模式
    enum SESS_MODEL
    {
        SESS_MODEL_TEXT,
        SESS_MODEL_CTRL,
        SESS_MODEL_IAC,
    };

    /// 会话节点
    struct SessionNode
    {
        char m_szUserName[IUser::NAME_SIZE];
        DWORD m_dwUserID;
        DWORD m_dwUserGroup;
        DWORD m_dwSessID;
        objSock *m_pSock;
        CDStream m_sInput;
        BYTE m_byState;
        BYTE m_byModel;
        BYTE m_byTryTimes;
        BYTE m_reserved;
        CTableString *m_pRspTable;
    };

    /// 模块列表MAP
    typedef std::map<std::string, IObject *> MAP_MODULES;
    typedef MAP_MODULES::iterator IT_MODULES;

    /// 属性列表MAP
    typedef std::map<std::string, DWORD> MAP_OBJATTRS;
    typedef MAP_OBJATTRS::iterator IT_OBJATTRS;

    /// 会话列表
    typedef std::map<DWORD, SessionNode> MAP_SESSIONS;
    typedef MAP_SESSIONS::iterator IT_SESSIONS;

public:
    static DCOP_PARA_NODE *GetArgList(const CDArray &strArgList, 
                        IModel::Field *pField, 
                        DWORD dwFieldCount, 
                        DWORD &rdwParaCount, 
                        CDStream &rsParaData, 
                        bool bOneValue = true, 
                        IModel *piModel = NULL, 
                        DWORD dwAttrID = 0);

    static BYTE GetArgOpCode(const CDArray &strNameValue);

    static DWORD GetFieldID(IModel::Field *pField, 
                        DWORD dwFieldCount, 
                        const char *cpszFieldName, 
                        BYTE &rbyParaType, 
                        WORD &rwParaSize, 
                        IModel *piModel = NULL, 
                        DWORD dwAttrID = 0);

    static DWORD GetFieldID(IModel::Field *pField, 
                        DWORD dwFieldCount, 
                        DWORD dwFieldIdx, 
                        BYTE &rbyParaType, 
                        WORD &rwParaSize);

    static DWORD GetFieldValue(BYTE &rbyParaType, 
                        WORD &rwParaSize, 
                        const char *cpszParaValue, 
                        CDStream &rsParaData);

    static const char *GetRelFieldName(IModel *piModel, 
                        DWORD dwAttrID, 
                        DWORD dwFieldID,
                        DWORD dwFieldCount);

public:
    CCommand(Instance *piParent, int argc, char **argv);
    ~CCommand();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DCOP_DECLARE_IOBJECT_MSG_HANDLE;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();

    void Welcome(const char *username, DWORD session);
    void Line(const char *command, DWORD session);
    void Out(LOG_PRINT fnOut, LOG_PARA pPara);

    void OnStart(objMsg *msg);
    void OnFinish(objMsg *msg);
    void OnModuleLoad(objMsg *msg);
    void OnModuleUnload(objMsg *msg);
    void OnModelReg(objMsg *msg);
    void OnSessionTimeout(objMsg *msg);
    void OnResponse(objMsg *msg);
    void OnEvent(objMsg *msg);
    void OnDefault(objMsg *msg);

    static void OnLogPrint(const char *cszLogInfo,
                        const char *pFile,
                        DWORD dwLine,
                        void *pUserArg);

    static DWORD OnAccept(DWORD dwChannelID,
                        objSock *pServerSock,
                        objSock *pAcceptSock,
                        void *pUserArg);

    static void OnDisconnect(DWORD dwChannelID,
                        objSock *pSock,
                        const char *cszRemoteIP,
                        WORD wRemotePort,
                        void *pUserArg);

    static void OnRecv(DWORD dwChannelID,
                        objSock *pSock,
                        void *pFrameBuf,
                        DWORD dwFrameLen,
                        const char *cszRemoteIP,
                        WORD wRemotePort,
                        void *pUserArg);

    static int bFrame(void *pBuf,
                        DWORD dwLen,
                        void *pUserArg);

    static void BytesOrder(void *pBuf,
                        DWORD dwLen,
                        void *pUserArg);

    struct OutputPara
    {
        CCommand *m_pThis;
        DWORD m_dwSessID;
    };

    static void Output(const char *text, void *para)
    {
        OutputPara *pPara = (OutputPara *)para;
        if (!pPara || !pPara->m_pThis) return;

        AutoObjLock(pPara->m_pThis);

        pPara->m_pThis->TextOut(text, pPara->m_pThis->FindSession(pPara->m_dwSessID));
    }

private:
    void AddModule(DWORD dwModuleID);
    void DelModule(DWORD dwModuleID);
    void RegModel(DWORD dwAttrID);

    IObject *GetModule(const char *cpszModName);
    BYTE GetOperation(const char *cpszOpName);
    DWORD GetAttribute(const char *cpszAttrName);

    DWORD Analyze(const char *command, SessionNode *pSessNode);
    
    void DumpModObj(const CDArray &strList, SessionNode *pSessNode);
    DWORD GetRspTableTitle(CTableString &tableStr, DWORD dwAttrID, DCOP_PARA_NODE *pPara, DWORD dwParaCount);
    DWORD GetRspTableContent(CTableString &tableStr, const CDArray &aRspHeads, DCOP_PARA_NODE *pPara, DWORD dwParaCount);
    DWORD GetRspTableLine(CTableString &tableStr, DCOP_PARA_NODE *pPara, DWORD dwParaCount, void *pData, DWORD dwDataLen);

    void GetDateTime(char *szStr, int strLen);

    void TextOut(const char *text, SessionNode *pSessNode = NULL);
    void TextEnd(SessionNode *pSessNode = NULL);
    void Help(SessionNode *pSessNode = NULL);

    void StartLanApp();
    void StopLanApp();

    DWORD CreateSession(objSock *pSock);
    void DeleteSession(DWORD dwIP, WORD wPort);
    SessionNode *FindSession(DWORD dwIP, WORD wPort);
    SessionNode *FindSession(DWORD dwSessionID);

    void ProcData(SessionNode &sessNode, void *pFrameBuf, DWORD dwFrameLen);

    void InputUserName(SessionNode &sessNode, const char *cszText);
    void InputPassText(SessionNode &sessNode, const char *cszText);

private:
    MAP_MODULES m_modules;                          // 模块列表
    MAP_OBJATTRS m_objattrs;                        // 属性列表

    WORD m_wTelnetdPort;                            // Telnet服务端口

    IManager *m_piManager;                          // 管理器
    IDispatch *m_piDispatch;                        // 分发器
    INotify *m_piNotify;                            // 订阅器
    IModel *m_piModel;                              // 模型
    IStatus *m_piStatus;                            // 状态

    IResponse *m_piResponse;                        // 响应器
    IResponse::IPool *m_pReqPool;                   // 请求缓冲池

    IUser *m_piUser;                                // 用户管理
    ISession *m_piSession;                          // 会话管理
    IAccess *m_piAccess;                            // 接入管理

    LOG_PRINT m_fnOut;                              // 输出回调
    LOG_PARA  m_pPara;                              // 输出参数

    char m_szSysInfo[OSNAME_LENGTH];                // 系统信息
    char m_szUserName[IUser::NAME_SIZE];            // 默认用户名
    CTableString *m_pRspTable;                      // 默认用户响应表

    objLanApp *m_pLanApp;                           // 网络应用
    MAP_SESSIONS m_sessions;                        // 本地会话信息
};


#endif // #ifndef _COMMAND_MAIN_H_

