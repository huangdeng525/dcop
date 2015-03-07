/// -------------------------------------------------
/// ObjControl_if.h : 控制器对象公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJCONTROL_IF_H_
#define _OBJCONTROL_IF_H_

#include "Object_if.h"


/// -------------------------------------------------
/// 定义IControl版本号
/// -------------------------------------------------
INTF_VER(IControl, 1, 0, 0);


/// -------------------------------------------------
/// 控制器接口(通过对象预设的接口进行控制)
/// -------------------------------------------------
interface IControl : public IObject
{
    /// 控制回调函数
    typedef DWORD (*ON_CTRL_FUNC)(
                        objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue,
                        IObject *piCtrler,
                        bool bLastNode
                        );

    /// 控制点描述
    struct Node
    {
        DWORD m_attribute;                          // 属性ID
        BYTE m_ctrl;                                // 控制类型
        BYTE m_ack;                                 // 应答类型
        BYTE m_group;                               // 用户组类型
        BYTE m_tty;                                 // 终端类型
        ON_CTRL_FUNC m_function;                    // 回调函数
    };

    /// 控制链接口
    interface IChain
    {
        /// 控制时
        virtual DWORD OnCtrl(
                        objMsg *pInput,
                        objMsg *&pOutput,
                        bool &bContinue
                        ) = 0;
    };

    /// 创建控制链(由'被控制者'在初始化时调用)
    virtual IChain *CreateChain(
                        IObject *ctrlee             // 被控制者
                        ) = 0;

    /// 删除控制链(由'被控制者'在结束时调用)
    virtual void DestroyChain(
                        IChain *chain               // 控制链
                        ) = 0;

    /// 注册控制点(由'控制者'(控制其他对象的对象)在'启动'事件中调用)
    virtual DWORD RegCtrlNode(
                        IObject *ctrler,            // 控制者
                        DWORD ctrlee,               // 被控制者
                        Node *ctrls,                // 控制点列表
                        DWORD count                 // 控制点数量
                        ) = 0;
};


#endif // #ifndef _OBJCONTROL_IF_H_

