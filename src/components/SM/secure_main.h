/// -------------------------------------------------
/// secure_main.h : 安全管理私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _SECURE_MAIN_H_
#define _SECURE_MAIN_H_

#define INC_MAP

#include "secure_if.h"
#include "ObjAttribute_if.h"
#include "ObjControl_if.h"


/// -------------------------------------------------
/// 安全管理实现类
/// -------------------------------------------------
class CSecure : public ISecure
{
public:
    typedef DWORD (CSecure::*CHECK_FUNC)(DCOP_SESSION_HEAD *pSessionHead,
                        void *pSessionData,
                        ISecure::Node *pRule,
                        objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        bool &bCheck);

    typedef std::map<DWORD, ISecure::Node> MAP_RULES;
    typedef MAP_RULES::iterator IT_RULES;

public:
    CSecure(Instance *piParent, int argc, char **argv);
    ~CSecure();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DCOP_DECLARE_IOBJECT_MSG_HANDLE;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();

    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

    void OnStart(objMsg *msg);
    void OnFinish(objMsg *msg);

    DWORD RegRule(Node *rules, DWORD count);

private:
    static DWORD InputCtrl(objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        IObject *piCtrler);

    void DumpRight(const char *pcszTitle, 
                        LOG_PRINT logPrint, 
                        LOG_PARA logPara, 
                        DWORD right);

    ISecure::Node *GetRuleNode(DWORD attrID);

    DWORD CheckAllRule(objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue);

    bool CheckMsgOwner(DCOP_SESSION_HEAD *pSessionHead,
                        void *pSessionData,
                        DWORD ownerField);

    DWORD CheckOperatorRule(DCOP_SESSION_HEAD *pSessionHead,
                        void *pSessionData,
                        ISecure::Node *pRule,
                        objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        bool &bCheck);

    DWORD CheckOwnerRule(DCOP_SESSION_HEAD *pSessionHead,
                        void *pSessionData,
                        ISecure::Node *pRule,
                        objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        bool &bCheck);

    DWORD CheckVisitorRule(DCOP_SESSION_HEAD *pSessionHead,
                        void *pSessionData,
                        ISecure::Node *pRule,
                        objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        bool &bCheck);

    DWORD CheckUserRule(DCOP_SESSION_HEAD *pSessionHead,
                        void *pSessionData,
                        ISecure::Node *pRule,
                        objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        bool &bCheck);

    DWORD CheckManagerRule(DCOP_SESSION_HEAD *pSessionHead,
                        void *pSessionData,
                        ISecure::Node *pRule,
                        objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        bool &bCheck);

private:
    MAP_RULES m_rules;
    IControl *m_piControl;
};


#endif // #ifndef _SECURE_MAIN_H_

