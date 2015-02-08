/// -------------------------------------------------
/// appbase_main.h : 应用基础私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _APPBASE_MAIN_H_
#define _APPBASE_MAIN_H_

#include "appbase_if.h"
#include "ObjAttribute_if.h"
#include "fs/file.h"


/// -------------------------------------------------
/// 应用基础实现类
/// -------------------------------------------------
class CAppBase : public IAppBase
{
public:
    CAppBase(Instance *piParent, int argc, char **argv);
    ~CAppBase();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DCOP_DECLARE_IOBJECT_MSG_HANDLE;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();
    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

    void OnStart(objMsg *msg);
    void OnFinish(objMsg *msg);
    void OnRequest(objMsg *msg);
    void OnDefault(objMsg *msg);

private:
    DWORD InitModelData();

private:
    char m_szModelCfg[DCOP_FILE_NAME_LEN];          // 模型配置文件

    CObjectMemberIndex m_dataIndex;                 // 数据索引
    CObjectMember<IData *> *m_pDatas;               // 数据成员列表
    CAttribute *m_pDataAttrs;                       // 数据属性列表
    DWORD m_dwDataCount;                            // 数据成员个数

    IModel *m_piModel;                              // 模型管理
    IData *m_piData;                                // 数据中心

    IDispatch *m_piDispatch;                        // 消息分发器
    INotify *m_piNotify;                            // 事件通知器
    INotify::IPool *m_pNotifyPool;                  // 事件缓冲池
};


#endif // #ifndef _APPBASE_MAIN_H_

