/// -------------------------------------------------
/// secure_main.cpp : 安全管理实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "secure_main.h"
#include "Factory_if.h"
#include "Manager_if.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CSecure, "secure")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CSecure)
    DCOP_IMPLEMENT_INTERFACE(ISecure)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CSecure)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END

/// -------------------------------------------------
/// 实现消息分发
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE(CSecure)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_START, OnStart)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_FINISH, OnFinish)
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE_END


/*******************************************************
  函 数 名: CSecure::CSecure
  描    述: CSecure构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CSecure::CSecure(Instance *piParent, int argc, char **argv)
{
    m_piControl = 0;

    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);
}

/*******************************************************
  函 数 名: CSecure::~CSecure
  描    述: CSecure析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CSecure::~CSecure()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CSecure::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSecure::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    /// 查询对象
    DCOP_QUERY_OBJECT_START(root)
        DCOP_QUERY_OBJECT_ITEM(IControl,     DCOP_OBJECT_CONTROL,    m_piControl)
    DCOP_QUERY_OBJECT_END

    return SUCCESS;
}

/*******************************************************
  函 数 名: CSecure::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSecure::Fini()
{
    DCOP_RELEASE_INSTANCE(m_piControl);
}

/*******************************************************
  函 数 名: CSecure::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSecure::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    if (!logPrint) return;

    AutoObjLock(this);

    for (IT_RULES it = m_rules.begin(); it != m_rules.end(); ++it)
    {
        logPrint(STR_FORMAT("ownerAttr: 0x%x \r\n", ((*it).second).m_ownerAttr), logPara);
        logPrint(STR_FORMAT("    ownerField: %d \r\n", ((*it).second).m_ownerField), logPara);
        DumpRight("    ownerRight", logPrint, logPara, ((*it).second).m_ownerRight);
        DumpRight("    visitorRight", logPrint, logPara, ((*it).second).m_visitorRight);
        logPrint(STR_FORMAT("    userAttr: 0x%x \r\n", ((*it).second).m_userAttr), logPara);
        logPrint(STR_FORMAT("    userField: %d \r\n", ((*it).second).m_userField), logPara);
        DumpRight("    userRight", logPrint, logPara, ((*it).second).m_userRight);
        logPrint(STR_FORMAT("    managerAttr: 0x%x \r\n", ((*it).second).m_managerAttr), logPara);
        logPrint(STR_FORMAT("    managerField: %d \r\n", ((*it).second).m_managerField), logPara);
        DumpRight("    managerRight", logPrint, logPara, ((*it).second).m_managerRight);
        logPrint(STR_FORMAT("    systemOperator: %d \r\n", ((*it).second).m_systemOperator), logPara);
    }
}

/*******************************************************
  函 数 名: CSecure::OnStart
  描    述: 开始运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSecure::OnStart(objMsg *msg)
{
    /// 注册安全控制
    if (m_piControl)
    {
        IControl::Node node[] = 
        {
            {0, DCOP_CTRL_NULL, DCOP_REQ, DCOP_GROUP_VISITOR, 0, InputCtrl},
        };
        DWORD dwRc = m_piControl->RegCtrlNode(this, 
                        DCOP_OBJECT_ACCESS, 
                        node, 
                        ARRAY_SIZE(node));
        CHECK_ERRCODE(dwRc, "Reg Ctrl Node To Access");
    }
}

/*******************************************************
  函 数 名: CSecure::OnFinish
  描    述: 结束运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSecure::OnFinish(objMsg *msg)
{
}

/*******************************************************
  函 数 名: CSecure::RegRule
  描    述: 注册安全检查规则
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSecure::RegRule(Node *rules, DWORD count)
{
    if (!rules || !count) return FAILURE;

    AutoObjLock(this);

    for (DWORD i = 0; i < count; ++i)
    {
        IT_RULES it = m_rules.find(rules[i].m_ownerAttr);
        if (it != m_rules.end())
        {
            continue;
        }

        (void)m_rules.insert(MAP_RULES::value_type(rules[i].m_ownerAttr, rules[i]));
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CSecure::InputCtrl
  描    述: 输入控制
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSecure::InputCtrl(objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        IObject *piCtrler)
{
    CSecure *pSecure = (CSecure *)piCtrler;
    if (!pSecure)
    {
        return FAILURE;
    }

    return pSecure->CheckAllRule(pInput, pOutput, bContinue);
}

/*******************************************************
  函 数 名: CSecure::DumpRight
  描    述: Dump权限
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSecure::DumpRight(const char *pcszTitle, 
                        LOG_PRINT logPrint, 
                        LOG_PARA logPara, 
                        DWORD right)
{
    if (!logPrint) return;

    logPrint(STR_FORMAT("%s: ", pcszTitle), logPara);

    static const char *Operation[] = 
    {
        "",
        "Create",
        "Destroy",
        "Add",
        "Del",
        "Set",
        "Get",
        "Dump",
        "Action",
        "Event",
    };

    bool bHaveRight = false;
    for (DWORD i = DCOP_CTRL_CREATE; i <= DCOP_CTRL_EVENT; ++i)
    {
        if (right & DCOP_SECURE_RIGHT(i))
        {
            logPrint(STR_FORMAT("%s ", Operation[i]), logPara);
            bHaveRight = true;
        }
    }

    if (!bHaveRight) logPrint("No Right ", logPara);

    logPrint("\r\n", logPara);
}

/*******************************************************
  函 数 名: CSecure::GetRuleNode
  描    述: 获取检查规则
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
ISecure::Node *CSecure::GetRuleNode(DWORD attrID)
{
    AutoObjLock(this);

    IT_RULES it = m_rules.find(attrID);
    if (it == m_rules.end())
    {
        return 0;
    }

    return &((*it).second);
}

/*******************************************************
  函 数 名: CSecure::CheckMsgOwner
  描    述: 检查消息中的owner字段
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CSecure::CheckMsgOwner(DCOP_SESSION_HEAD *pSessionHead,
                        void *pSessionData,
                        DWORD ownerField)
{
    if (!pSessionHead || !pSessionData || !ownerField)
    {
        return false;
    }

    /// 获取消息中的条件参数
    CDArray aCondHeads;
    IObjectMember::GetMsgHead(pSessionData, pSessionHead->m_type.m_valueLen, 0, &aCondHeads, 0, 0, 0);

    DCOP_CONDITION_HEAD *pCondHead = (DCOP_CONDITION_HEAD *)aCondHeads.Pos(0);
    if (!pCondHead)
    {
        return false;
    }

    /// 首先确认条件字段中的参数必须是同时满足
    if (pCondHead->m_condition == DCOP_CONDITION_ANY)
    {
        return false;
    }

    void *pCondParaData = *(void **)(pCondHead + 1);
    DWORD dwCondParaCount = pCondHead->m_paraCount;
    DWORD dwCondDataLen = pCondHead->m_paraLen;
    if (!pCondParaData || !dwCondParaCount || !dwCondDataLen)
    {
        return false;
    }

    CDStream sCondPara(dwCondParaCount * sizeof(DCOP_PARA_NODE));
    DCOP_PARA_NODE *pCondPara = (DCOP_PARA_NODE *)sCondPara.Buffer();
    if (!pCondPara)
    {
        return false;
    }

    void *pCondData = IObjectMember::GetMsgParaData(pCondParaData, 
                        dwCondParaCount, dwCondDataLen, 
                        pCondPara);
    if (!pCondData)
    {
        return false;
    }

    /// 在条件参数字段中查找owner字段
    DWORD dwParaSize = 0;
    DWORD dwParaOffset = 0;
    for (DWORD i = 0; i < dwCondParaCount; ++i)
    {
        if ((pCondPara[i].m_paraID == ownerField) &&
            (pCondPara[i].m_opCode == DCOP_OPCODE_EQUAL))
        {
            dwParaSize = pCondPara[i].m_paraSize;
            break;
        }

        dwParaOffset += pCondPara[i].m_paraSize;
    }

    if (!dwParaSize)
    {
        return false;
    }

    /// 在条件参数数据中查找owner值
    /// (BYTE *)pCondData + dwParaOffset, dwParaSize

    return true;
}

/*******************************************************
  函 数 名: CSecure::CheckAllRule
  描    述: 检查所有规则
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSecure::CheckAllRule(objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue)
{
    if (!pInput) return FAILURE;

    bool bCheck = false;
    DWORD dwRc = FAILURE;

    /// 获取消息头
    CDArray aSessHeads;
    IObjectMember::GetMsgHead(pInput->GetDataBuf(), pInput->GetDataLen(), &aSessHeads, 0, 0, 0, 0);
    for (DWORD sessIdx = 0; sessIdx < aSessHeads.Count(); ++sessIdx)
    {
        /// 获取会话头
        DCOP_SESSION_HEAD *pSessionHead = (DCOP_SESSION_HEAD *)aSessHeads.Pos(sessIdx);
        if (!pSessionHead)
        {
            continue;
        }

        /// 获取会话数据
        void *pSessionData = *(void **)(pSessionHead + 1);

        /// 根据会话获取规则节点
        ISecure::Node *pRuleNode = GetRuleNode(pSessionHead->m_attribute);
        if (!pRuleNode)
        {
            continue;
        }

        /// '表驱动'中的功能函数列表
        static CHECK_FUNC checkFunc[] = 
        {
            &CSecure::CheckOperatorRule,
            &CSecure::CheckOwnerRule,
            &CSecure::CheckVisitorRule,
            &CSecure::CheckUserRule,
            &CSecure::CheckManagerRule,
        };

        /// 依次检查
        for (DWORD i = 0; i < ARRAY_SIZE(checkFunc); ++i)
        {
            if (!checkFunc[i])
            {
                continue;
            }

            dwRc = (this->*(checkFunc[i]))(pSessionHead,
                        pSessionData,
                        pRuleNode,
                        pInput,
                        pOutput,
                        bContinue,
                        bCheck);
            if (bCheck) break;
        }

        PrintLog(STR_FORMAT("group:%d\r\n session:%d\r\n user:%d\r\n tty:%d\r\n attribute:0x%x\r\n dwRc:%d \r\n",
                        pSessionHead->m_group,
                        pSessionHead->m_session,
                        pSessionHead->m_user,
                        pSessionHead->m_tty,
                        pSessionHead->m_attribute,
                        dwRc), 
                        PrintToConsole, 0);

        /// 已经得到检查结果
        if (bCheck) break;
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CSecure::CheckOperatorRule
  描    述: 检查系统操作者权限
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSecure::CheckOperatorRule(DCOP_SESSION_HEAD *pSessionHead,
                        void *pSessionData,
                        ISecure::Node *pRule,
                        objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        bool &bCheck)
{
    if (!pSessionHead || !pRule)
    {
        return FAILURE;
    }

    /// 系统用户必须是监控者以上的用户
    if (pRule->m_systemOperator < DCOP_GROUP_MONITOR)
    {
        return FAILURE;
    }

    /// 这里就是系统用户检查过的了，后面就不要检查其他用户了
    bCheck = true;

    if (pSessionHead->m_group >= pRule->m_systemOperator)
    {
        return SUCCESS;
    }

    /// 没有权限，中断运行
    bContinue = false;
    return ERRCODE_IO_NO_RIGHT_TO_OPERATE;
}

/*******************************************************
  函 数 名: CSecure::CheckOwnerRule
  描    述: 检查所有者操作权限
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSecure::CheckOwnerRule(DCOP_SESSION_HEAD *pSessionHead,
                        void *pSessionData,
                        ISecure::Node *pRule,
                        objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        bool &bCheck)
{
    if (!pSessionHead || !pRule)
    {
        return FAILURE;
    }

    /// 必须是业务层面的用户
    if (pSessionHead->m_group != DCOP_GROUP_USER)
    {
        return FAILURE;
    }

    return FAILURE;
}

/*******************************************************
  函 数 名: CSecure::CheckVisitorRule
  描    述: 检查参观者操作权限
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSecure::CheckVisitorRule(DCOP_SESSION_HEAD *pSessionHead,
                        void *pSessionData,
                        ISecure::Node *pRule,
                        objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        bool &bCheck)
{
    if (!pSessionHead || !pRule)
    {
        return FAILURE;
    }

    /// 必须是未登录用户
    if (pSessionHead->m_group != DCOP_GROUP_VISITOR)
    {
        return FAILURE;
    }

    /// 检查未登录用户权限设定
    if (pRule->m_visitorRight & DCOP_SECURE_RIGHT(pSessionHead->m_ctrl))
    {
        return SUCCESS;
    }

    /// 没有权限，中断运行
    bContinue = false;
    return ERRCODE_IO_NO_RIGHT_TO_OPERATE;
}

/*******************************************************
  函 数 名: CSecure::CheckUserRule
  描    述: 检查一般用户操作权限
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSecure::CheckUserRule(DCOP_SESSION_HEAD *pSessionHead,
                        void *pSessionData,
                        ISecure::Node *pRule,
                        objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        bool &bCheck)
{
    if (!pSessionHead || !pRule)
    {
        return FAILURE;
    }

    /// 必须是业务层面的用户
    if (pSessionHead->m_group != DCOP_GROUP_USER)
    {
        return FAILURE;
    }

    return FAILURE;
}

/*******************************************************
  函 数 名: CSecure::CheckManagerRule
  描    述: 检查管理用户操作权限
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSecure::CheckManagerRule(DCOP_SESSION_HEAD *pSessionHead,
                        void *pSessionData,
                        ISecure::Node *pRule,
                        objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        bool &bCheck)
{
    if (!pSessionHead || !pRule)
    {
        return FAILURE;
    }

    if (pSessionHead->m_group != DCOP_GROUP_USER)
    {
        return FAILURE;
    }

    return FAILURE;
}

