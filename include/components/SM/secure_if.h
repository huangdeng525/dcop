/// -------------------------------------------------
/// secure_if.h : 安全管理公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _SECURE_IF_H_
#define _SECURE_IF_H_

#include "Object_if.h"


/////////////////////////////////////////////////////
///               '安全接口'说明
/// -------------------------------------------------
/// '安全接口'(secure interface)是外部接入时进行安全
/// 认证的验证控制接口。
/// 1. '系统用户和未登录用户': 可直接由当前用户的级别
///    (用户组类型)来决定。
/// 2. '普通用户': 比如某种业务数据的所有者、普通用户
///    和管理用户等，都是属于业务层面，在'用户组级别'
///    都是属于'普通用户'，而他们具体拥有何种权限，是
///    由具体业务对应的权限定义的(可在建模时支持定制)
/////////////////////////////////////////////////////


/// -------------------------------------------------
/// 权限位(第一个DCOP_CTRL_NULL不占bit位)
/// -------------------------------------------------
#define DCOP_SECURE_RIGHT(opt) (1 << (opt - 1))


/// -------------------------------------------------
/// 定义ISecure版本号
/// -------------------------------------------------
INTF_VER(ISecure, 1, 0, 0);


/// -------------------------------------------------
/// 安全管理接口
/// -------------------------------------------------
interface ISecure : public IObject
{
    /// 节点
    struct Node
    {
        DWORD m_ownerAttr;
        DWORD m_ownerField;
        DWORD m_ownerRight;
        DWORD m_visitorRight;
        DWORD m_userAttr;
        DWORD m_userField;
        DWORD m_userRight;
        DWORD m_managerAttr;
        DWORD m_managerField;
        DWORD m_managerRight;
        DWORD m_systemOperator;
    };

    /// 注册安全检查规则
    virtual DWORD RegRule(
                        Node *rules,                // 检查规则
                        DWORD count                 // 规则数量
                        ) = 0;
};


#endif // #ifndef _SECURE_IF_H_

