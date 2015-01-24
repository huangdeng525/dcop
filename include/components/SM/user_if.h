/// -------------------------------------------------
/// user_if.h : 用户管理公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _USER_IF_H_
#define _USER_IF_H_

#include "Object_if.h"


/// -------------------------------------------------
/// 定义IUser版本号
/// -------------------------------------------------
INTF_VER(IUser, 1, 0, 0);


/// -------------------------------------------------
/// 用户管理接口
/// -------------------------------------------------
interface IUser : public IObject
{
    /// 用户名长度
    static const WORD NAME_SIZE = 32;

    /// 校验字长度
    static const WORD PASS_SIZE = 32;

    /// 信息长度
    static const WORD INFO_SIZE = 64;

    /// 节点
    struct NODE
    {
        char UserName[NAME_SIZE];
        DWORD UserID;
        char PassText[PASS_SIZE];
        DWORD Level;
        DWORD Group;
        char Info[INFO_SIZE];
    };

    /// 创建用户
    virtual DWORD CreateUser(
                        const char *cszUserName,
                        const char *cszPassText,
                        DWORD dwLevel,
                        DWORD dwGroup,
                        DWORD &rdwUserID
                        ) = 0;

    /// 删除用户
    virtual DWORD DeleteUser(
                        DWORD dwUserID
                        ) = 0;

    /// 查找用户
    virtual DWORD FindUser(
                        const char *cszUserName,
                        NODE &rNode
                        ) = 0;

    /// 检查校验字
    virtual DWORD CheckPass(
                        const char *cszUserName,
                        const char *cszPassText,
                        NODE *pNode = 0
                        ) = 0;

    /// 修改校验字
    virtual DWORD ChangePass(
                        DWORD dwUserID,
                        char szUserName[NAME_SIZE],
                        char szOldPass[PASS_SIZE],
                        char szNewPass[PASS_SIZE]
                        ) = 0;

    /// 获取用户
    virtual DWORD GetUser(
                        DWORD dwUserID,
                        NODE &rNode
                        ) = 0;
};


#endif // #ifndef _USER_IF_H_

