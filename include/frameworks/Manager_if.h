/// -------------------------------------------------
/// Manager_if.h : 对象管理公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _MANAGER_IF_H_
#define _MANAGER_IF_H_

#include "Object_if.h"


/// -------------------------------------------------
/// 定义IManager版本号
/// -------------------------------------------------
INTF_VER(IManager, 1, 0, 0);


/// -------------------------------------------------
/// 对象管理器接口
/// -------------------------------------------------
interface IManager : public IObject
{
    /// 插入一个对象
    virtual DWORD InsertObject(
                        IObject *pObject
                        ) = 0;

    /// 删除一个对象
    virtual DWORD DeleteObject(
                        IObject *pObject
                        ) = 0;

    /// 获取所属的系统ID
    virtual DWORD GetSystemID(
                        ) = 0;

    /// 获取所属的系统ID
    virtual const char *GetSystemInfo(
                        ) = 0;

    /// 广播消息到所有对象
    virtual void BroadcastMsg(
                        objMsg *msg
                        ) = 0;
};


/// -------------------------------------------------
/// 'DCOP_EVENT_MANAGER_INITALLOBJECTS' 参数: 
///     Count(DWORD) + 对象ID依次列表(DWORD ... )
/// -------------------------------------------------


#endif // #ifndef _OBJMANAGER_IF_H_

