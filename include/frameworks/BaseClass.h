/// -------------------------------------------------
/// BaseClass.h : 实例基类公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _BASECLASS_H_
#define _BASECLASS_H_

#include "task.h"


/////////////////////////////////////////////////////
///              Instance使用说明
/// -------------------------------------------------
/// 1.  h文件CMyClass中进行:
///         DCOP_DECLARE_INSTANCE;              // 声明Instance成员
/// -------------------------------------------------
/// 2.  cpp文件中CMyClass外进行:
///     DCOP_IMPLEMENT_INSTANCE(CMyClass)       // 实现Instance成员
///         ... ...
///         DCOP_IMPLEMENT_INTERFACE(IA)        // 接口A的实现(表项依次从子类到基类Instance)
///         DCOP_IMPLEMENT_INTERFACE(Instance)
///     DCOP_IMPLEMENT_INSTANCE_END             // 结束
/// -------------------------------------------------
/// 3.  CMyClass构造函数开始:
///     CMyClass(Instance *piParent, int argc, char **argv)
///     {
///         DCOP_CONSTRUCT_INSTANCE(piParent);  // 传入父对象指针
/// 
///         ... ...
///     }
/// -------------------------------------------------
/// 4.  CMyClass析构函数结束:
///     ~CMyClass()
///     {
///         ... ...
/// 
///         DCOP_DESTRUCT_INSTANCE();
///     }
/// -------------------------------------------------
/// 5.  实现类工厂管理对象模板, 位置不固定, cpp中即可
///     DCOP_IMPLEMENT_FACTORY(CMyClass, "MyName") // 加入实现类到类厂容器
/////////////////////////////////////////////////////


/// -------------------------------------------------
/// 版本声明(接口名,大版本,小版本,微版本)
/// -------------------------------------------------
#define INTF_VER(Interface, major, minor, micro)    \
    const DWORD ___VERSION_##Interface = (          \
        (((major) << 16) & 0xFFFF0000) |            \
        (((minor) <<  8) & 0x0000FF00) |            \
        ((micro)         & 0x000000FF))
/// -------------------------------------------------
#define INTF_VER_MAJOR(Interface)                   \
        ((___VERSION_##Interface >> 16 ) & 0x0000FFFF)
/// -------------------------------------------------
#define INTF_VER_MINOR(Interface)                   \
        ((___VERSION_##Interface >>   8) & 0x000000FF)
/// -------------------------------------------------
#define INTF_VER_MICRO(Interface)                   \
        ((___VERSION_##Interface)        & 0x000000FF)
/// -------------------------------------------------
INTF_VER(Instance, 1, 0, 0);
/// -------------------------------------------------


/// -------------------------------------------------
/// 实例基础类(定义虚析构行为)
/// -------------------------------------------------
class objBase
{
public:
    /// 这里获得基类的全局实例(即:框架内核对象)
    static objBase *GetInstance();

    /// 所有基类均为虚析构
    virtual ~objBase() = 0;

    /// 对象信息: 名字和ID
    ///     Name: 只是用来标识实例类别
    ///     ID  : 用来唯一索引一个实例
    virtual const char *Name() {return 0;}
    virtual DWORD ID() {return 0;}

    /// 进入和离开的互斥保护
    ///     互斥保护: 由实例实现者自己来对入口进行保护(默认为空实现)
    ///     使用方法: 可在入口函数开始使用AutoObjLock(this)来简化操作
    virtual void Enter() {}
    virtual void Leave() {}

    /// Dump入口
    ///     变参使用: 实现时请按约定的顺序获取各个指针参数并输出
    virtual void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv) {}

    /// 部署一个新的系统(使用部署配置文件)[返回该系统根对象(管理器)]
    virtual objBase *Deploy(const char *cfgDeploy) {return 0;}
};


/// -------------------------------------------------
/// 自动对象锁，用于实例入口函数自动加锁和解锁
/// -------------------------------------------------
#define AutoObjLock(x) AutoObjLockLine(x, __LINE__)
#define AutoObjLockLine(x, line) AutoObjLockLineEx(x, line)
#define AutoObjLockLineEx(x, line) AutoObjLockEx __tmp_##line(x)
class AutoObjLockEx
{
public:
    AutoObjLockEx() {m_piBase = 0;}
    AutoObjLockEx(objBase *piBase) {m_piBase = piBase; if (m_piBase) m_piBase->Enter();}
    ~AutoObjLockEx() {if (m_piBase) m_piBase->Leave();}

private:
    objBase *m_piBase;
};


/// -------------------------------------------------
/// 定义查询接口的返回和参数(名字和版本)
/// -------------------------------------------------
struct IntfNameVer
{
    const char *m_interfaceName;        // 接口名
    WORD m_majorVersion;                // 大版本
    BYTE m_minorVersion;                // 小版本
    BYTE m_microVersion;                // 微版本

    IntfNameVer()
    {
        m_interfaceName = 0;
        m_majorVersion  = 0;
        m_minorVersion  = 0;
        m_microVersion  = 0;
    }
    IntfNameVer(const char *cszInterfaceName, DWORD dwVersion)
    {
        m_interfaceName = cszInterfaceName;
        m_majorVersion  = (WORD)((dwVersion >> 16) & 0x0000FFFF);
        m_minorVersion  = (BYTE)((dwVersion >>  8) & 0x000000FF);
        m_microVersion  = (BYTE)((dwVersion)       & 0x000000FF);
    }
};
#define ID_INTF(Interface)  IntfNameVer(#Interface, ___VERSION_##Interface)
#define interface           struct
#define REF_PTR(objPtr)     ((void **)&(objPtr))
#ifdef _MSC_VER
    #pragma warning(disable:4005)
#endif


/// -------------------------------------------------
/// 实例接口基础类(定义引用行为)
/// -------------------------------------------------
interface Instance : public objBase
{
    /// 引用实例
    virtual DWORD QueryInterface(
                        const IntfNameVer& iid,     // 接口ID
                        void **ppv = 0,             // 输出实例
                        Instance *pir = 0           // 输入引用实例
                        ) = 0;

    /// 释放引用
    virtual DWORD Release(
                        Instance *pir = 0           // 输入引用实例
                        ) = 0;

    /// 查询引用
    virtual DWORD GetRef(
                        Instance ***pppirs = 0,     // 输出引用实例列表
                        DWORD *count = 0            // 输出引用实例个数
                        ) = 0;
};


/// -------------------------------------------------
/// 实现类声明Instance模板
/// -------------------------------------------------
#define DCOP_DECLARE_INSTANCE                       \
public:                                             \
    Instance *m_piParent;                           \
    objAtomic::T m_refCount;                        \
    DWORD GetRef(Instance ***pppirs = 0, DWORD *count = 0); \
    DWORD Release(Instance *pir = 0);               \
    DWORD QueryInterface(const IntfNameVer& iid, void **ppv = 0, Instance *pir = 0)


/// -------------------------------------------------
/// 实现类实现Instance模板
/// -------------------------------------------------
#define DCOP_CONSTRUCT_INSTANCE(Parent)             \
    m_piParent = Parent;                            \
    if (m_piParent)                                 \
    {                                               \
        (void)m_piParent->QueryInterface(           \
                            ID_INTF(Instance),      \
                            0,                      \
                            this);                  \
    }                                               \
    (void)objAtomic::Set(m_refCount, 1)


/// -------------------------------------------------
/// 实现类释放Instance模板
/// -------------------------------------------------
#define DCOP_DESTRUCT_INSTANCE()                    \
    m_piParent = 0;                                 \
    (void)objAtomic::Set(m_refCount, 0)


/// -------------------------------------------------
/// 挂接实现类到Instance模板 - 挂接表头
/// -------------------------------------------------
#define DCOP_IMPLEMENT_INSTANCE(CMyClass)           \
extern void (*g_onInstanceQueryInterface)(          \
                    Instance *piThis,               \
                    Instance *piRefer,              \
                    void *pPara);                   \
extern void * g_onInstanceQueryInterfacePara;       \
extern void (*g_onInstanceRelease)(                 \
                    Instance *piThis,               \
                    Instance *piRefer,              \
                    void *pPara);                   \
extern void * g_onInstanceReleasePara;              \
extern void (*g_onInstanceGetRef)(                  \
                    Instance *piThis,               \
                    Instance ***pppiRefers,         \
                    DWORD *pdwReferCount,           \
                    void *pPara);                   \
extern void * g_onInstanceGetRefPara;               \
DWORD CMyClass::GetRef(Instance ***pppirs, DWORD *count) \
{                                                   \
    if (g_onInstanceGetRef && pppirs && count)      \
        g_onInstanceGetRef(this, pppirs, count,     \
        g_onInstanceGetRefPara);                    \
    return (DWORD)m_refCount;                       \
}                                                   \
DWORD CMyClass::Release(Instance *pir)              \
{                                                   \
    if (m_piParent)                                 \
        (void)m_piParent->Release(this);            \
    DWORD dwRefCount = (DWORD)objAtomic::Dec(m_refCount); \
    if (g_onInstanceRelease && pir)                 \
        g_onInstanceRelease(this, pir,              \
        g_onInstanceGetRefPara);                    \
    if (!dwRefCount) delete this;                   \
    return dwRefCount;                              \
}                                                   \
DWORD CMyClass::QueryInterface(const IntfNameVer& iid, void **ppv, Instance *pir) \
{


/// -------------------------------------------------
/// 挂接实现类到Instance模板 - 挂接'接口名'表项
/// -------------------------------------------------
#define DCOP_IMPLEMENT_INTERFACE(Interface)         \
    if (!strcmp(#Interface, iid.m_interfaceName) && \
        (INTF_VER_MAJOR(Interface) >=               \
            iid.m_majorVersion))                    \
    {                                               \
        if (ppv)                                    \
            *ppv = static_cast<Interface *>(this);  \
        if (m_piParent)                             \
            (void)m_piParent->QueryInterface(       \
                        ID_INTF(Instance),          \
                        0,                          \
                        this);                      \
        (void)objAtomic::Inc(m_refCount);           \
        if (g_onInstanceQueryInterface && pir)      \
            g_onInstanceQueryInterface(this, pir,   \
            g_onInstanceQueryInterfacePara);        \
        return SUCCESS;                             \
    }


/// -------------------------------------------------
/// 挂接实现类到Instance模板 - 挂接表尾
/// -------------------------------------------------
#define DCOP_IMPLEMENT_INSTANCE_END                 \
    return FAILURE;                                 \
}


#endif // #ifndef _BASECLASS_H_

