/// -------------------------------------------------
/// ObjManager_main.h : 对象管理私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJMANAGER_MAIN_H_
#define _OBJMANAGER_MAIN_H_

#define INC_MAP

#include "Manager_if.h"
#include "Factory_if.h"


/// -------------------------------------------------
/// 挂接实现类到IObject模板 - 配置系统ID
/// -------------------------------------------------
#define IMPLEMENT_CONFIG_SYSTEM(id, info)           \
    DCOP_IMPLEMENT_CONFIG_INTEGER(id, m_dwSystemID)      \
    DCOP_IMPLEMENT_CONFIG_STRING(info, m_szSystemInfo)


/// -------------------------------------------------
/// 对象管理实现
/// -------------------------------------------------
class CObjectManager : public IManager
{
public:
    /// 对象节点
    class CObjectNode
    {
    public:
        CObjectNode() {m_object = 0; m_manager = 0;}
        ~CObjectNode()
        {
            if (m_object)
            {
                TRACE_LOG(STR_FORMAT("Start Release Object:'%s'(%d)", m_object->Name(), m_object->ID()));
                DWORD dwRefCount = m_object->Release(m_manager);
                /// 如果还有引用计数，再多进行一次释放，这样做是为了让管理器具有析构对象的功能
                if (dwRefCount) dwRefCount = m_object->Release(m_manager);
                TRACE_LOG(STR_FORMAT("Endof Release Object Ref Count:%d", dwRefCount));
                m_object = 0;
            }
        }

        void SetObject(IObject *object, IManager *manager) {m_object = object; m_manager = manager;}
        IObject *GetObject() {return m_object;}

    private:
        IObject *m_object;
        IManager *m_manager;
    };

    /// 对象MAP
    typedef std::map<DWORD, CObjectNode> MAP_OBJECTS;
    typedef MAP_OBJECTS::iterator IT_OBJECTS;

public:
    CObjectManager(Instance *piParent, int argc, char **argv);
    ~CObjectManager();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DWORD Init(IObject *root, int argc, void **argv);

    void Fini();

    void Proc(objMsg *msg);

    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

    IObject *Child(DWORD dwID);

    DWORD InsertObject(IObject *pObject);

    DWORD DeleteObject(IObject *pObject);

    DWORD GetSystemID();

    const char *GetSystemInfo();

    void BroadcastMsg(objMsg *msg);

private:
    void SendEvent(bool bInitOrFini, IObject *pDynamicObject = 0);

private:
    /////////////////////////////////////////////////
    /// 是否初始化
    /////////////////////////////////////////////////
    bool m_bInit;

    /////////////////////////////////////////////////
    /// 系统ID(一个系统对应一个运行实例)
    /////////////////////////////////////////////////
    DWORD m_dwSystemID;
    char m_szSystemInfo[OSNAME_LENGTH];

    /////////////////////////////////////////////////
    /// 对象集合
    /////////////////////////////////////////////////
    MAP_OBJECTS m_objects;
};


#endif // #ifndef _OBJMANAGER_MAIN_H_

