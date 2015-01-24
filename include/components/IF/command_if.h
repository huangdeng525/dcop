/// -------------------------------------------------
/// command_if.h : 命令行接入公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _COMMAND_IF_H_
#define _COMMAND_IF_H_

#include "Object_if.h"


/// -------------------------------------------------
/// 定义ICommand版本号
/// -------------------------------------------------
INTF_VER(ICommand, 1, 0, 0);


/// -------------------------------------------------
/// 命令行接入接口定义
/// -------------------------------------------------
interface ICommand : public IObject
{
    /// 欢迎信息
    virtual void Welcome(
                        const char *username, 
                        DWORD session = 0
                        ) = 0;

    /// 输入命令行
    virtual void Line(
                        const char *command, 
                        DWORD session = 0
                        ) = 0;

    /// 设置输出
    virtual void Out(
                        LOG_PRINT fnOut, 
                        LOG_PARA  pPara = 0
                        ) = 0;
};


#endif // #ifndef _COMMAND_IF_H_

