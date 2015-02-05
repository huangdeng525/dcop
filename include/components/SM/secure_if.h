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
/////////////////////////////////////////////////////


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

