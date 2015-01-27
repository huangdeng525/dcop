/// -------------------------------------------------
/// Object_if.h : 对象基类公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJECT_IF_H_
#define _OBJECT_IF_H_

#include "BaseClass.h"
#include "sem.h"
#include "msg.h"


/////////////////////////////////////////////////////
///                   IObject使用说明
/// -------------------------------------------------
/// 1.  h文件CMyClass中进行:
///         DCOP_DECLARE_INSTANCE;
///         DCOP_DECLARE_IOBJECT;
/// -------------------------------------------------
/// 2.  cpp文件CMyClass外进行:
///     DCOP_IMPLEMENT_INSTANCE(CMyClass)
///         ... ...
///         DCOP_IMPLEMENT_INTERFACE(IA)                    // 接口A的实现(表项依次从子类到基类Instance)
///         DCOP_IMPLEMENT_INTERFACE(IObject)
///         DCOP_IMPLEMENT_INTERFACE(Instance)
///     DCOP_IMPLEMENT_INSTANCE_END
/// 
///     DCOP_IMPLEMENT_IOBJECT(CMyClass)
///         DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")   // 从参数-name、-id后面的值获取名字和ID，推荐使用
///         DCOP_IMPLEMENT_IDENTIFY_STATIC("A", 1)          // 也可以硬编码NAME及ID，建议在单实例(单件)时才使用
///         DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")  // 配置线程安全，普通对象基本都是多线程环境(纯子对象建议不配置)
///         DCOP_IMPLEMENT_CUSTOM(argc, argv)               // 用户可以挂接自己的宏处理参数，只是举例
///         ... ...
///     DCOP_IMPLEMENT_IOBJECT_END
/// -------------------------------------------------
/// 3.  CMyClass构造函数开始:
///     CMyClass(Instance *piParent, int argc, char **argv)
///     {
///         ... ...
/// 
///         DCOP_CONSTRUCT_INSTANCE(piParent);
///         DCOP_CONSTRUCT_IOBJECT(argc, argv);
///     }
/// -------------------------------------------------
/// 4.  CMyClass析构函数结束:
///     ~CMyClass()
///     {
///         ... ...
/// 
///         DCOP_DESTRUCT_IOBJECT();
///         DCOP_DESTRUCT_INSTANCE();
///     }
/// -------------------------------------------------
/// 5.  实现类工厂管理对象模板, 位置不固定, cpp中即可
///     DCOP_IMPLEMENT_FACTORY(CMyClass, "A")
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
/// 对象定义说明
/// -------------------------------------------------
/// 1) 对象是一种被动式对象，只按要求被动处理外部操作
/// 2) 对象可提供"函数接口"和"消息接口"，建议必须要本
///    地化的对象可提供"函数接口"外，其他对象应该优先
///    提供"消息接口"，便于分布式部署
/// 3) 对象和外部的交互，需要通过使用"对象管理器"中的
///    其他对象来实现 (管理器在初始化入口以root对象身
///    份传入，代表对象树层次的根一定是管理器)。
///    对象和外部交互有:
///    a) 向外部分发消息(请求或者响应 - 有目的地址)，
///       需要使用IDispatch对象
///    b) 向外部分发事件(内部产生事件 - 无目的地址)，
///       需要使用INotify对象
///    c) 完成其他操作，需要使用其他对象接口 ... ...
/// 4) 对象的保护: 对象实现的入口由对象本身提供保护，
///    可配套使用"自动对象锁"在入口处生成自动锁
/////////////////////////////////////////////////////


/// -------------------------------------------------
/// 定义IObject版本号
/// -------------------------------------------------
INTF_VER(IObject, 1, 0, 0);


/// -------------------------------------------------
/// 对象抽象接口
/// -------------------------------------------------
interface IObject : public Instance
{
    /// 类信息
    virtual const char *Class() = 0;
    virtual DWORD Size() = 0;

    /// 获取是否'线程安全'和'消息并发'
    ///     线程安全: 同步创建互斥信号量，防止对象入口被重入，如果关闭'线程安全'，则无需配置'消息并发'(因为无锁保护)
    ///     消息并发: 对消息入口不进行保护，'Proc(msg)'可被多个线程重入，因此在消息处理中间访问对象其他入口需要锁保护
    virtual bool bThreadSafe() = 0;
    virtual bool bConcurrent() = 0;

    /// 获取父对象和子对象
    ///     父对象: 来源于Instance构造的父对象，只是在IObject构建时重新转换了类型
    ///     子对象: 可由具体对象自己创建管理器来进行子对象的管理并提供查询方法
    virtual IObject *Parent() = 0;
    virtual IObject *Child(DWORD dwID) {return 0;}

    /// 初始化入口
    ///     root对象: 一般用于根对象，可以根据情况传递绝对像根对象或者相对根对象
    ///     变参使用: 类型是地址指针(void**)，不同于构造函数的字符串指针(char**)
    virtual DWORD Init(IObject *root, int argc, void **argv) {return SUCCESS;}

    /// 结束时入口
    virtual void Fini() {}

    /// 消息入口
    ///     消息释放: 对于输入消息，只处理，不释放
    virtual void Proc(objMsg *msg) {}
};


/// -------------------------------------------------
/// 实现类声明IObject模板
/// -------------------------------------------------
#define DCOP_DECLARE_IOBJECT                        \
public:                                             \
    char m_szObjName[OSNAME_LENGTH];                \
    DWORD m_dwObjID;                                \
    bool m_bThreadSafe;                             \
    bool m_bConcurrent;                             \
    objLock *m_pLock;                               \
    IObject *m_piObjParent;                         \
    const char *Class();                            \
    DWORD Size();                                   \
    const char *Name();                             \
    DWORD ID();                                     \
    bool bThreadSafe();                             \
    bool bConcurrent();                             \
    void Enter();                                   \
    void Leave();                                   \
    IObject *Parent();                              \
    void ConstructClass(int argc, char **argv)


/// -------------------------------------------------
/// 实现类实现IObject模板
/// -------------------------------------------------
#define DCOP_CONSTRUCT_IOBJECT(argc, argv)          \
    *m_szObjName = 0;                               \
    m_dwObjID = 0;                                  \
    m_bThreadSafe = false;                          \
    m_bConcurrent = false;                          \
    m_pLock = 0;                                    \
    m_piObjParent = 0;                              \
    ConstructClass(argc, argv);                     \
    if (m_bThreadSafe)                              \
        m_pLock = DCOP_CreateLock()


/// -------------------------------------------------
/// 实现类释放IObject模板
/// -------------------------------------------------
#define DCOP_DESTRUCT_IOBJECT()                     \
    m_piObjParent = 0;                              \
    if (m_pLock)                                    \
        delete m_pLock;                             \
    m_pLock = 0


/// -------------------------------------------------
/// 挂接实现类到IObject模板 - 挂接表头
/// -------------------------------------------------
#define DCOP_IMPLEMENT_IOBJECT(CMyClass)            \
const char *CMyClass::Class()                       \
{                                                   \
    return #CMyClass;                               \
}                                                   \
DWORD CMyClass::Size()                              \
{                                                   \
    return (DWORD)sizeof(CMyClass);                 \
}                                                   \
const char *CMyClass::Name()                        \
{                                                   \
    return m_szObjName;                             \
}                                                   \
DWORD CMyClass::ID()                                \
{                                                   \
    return m_dwObjID;                               \
}                                                   \
bool CMyClass::bThreadSafe()                        \
{                                                   \
    return m_bThreadSafe;                           \
}                                                   \
bool CMyClass::bConcurrent()                        \
{                                                   \
    return m_bConcurrent;                           \
}                                                   \
void CMyClass::Enter()                              \
{                                                   \
    if (m_pLock)                                    \
        m_pLock->Enter();                           \
}                                                   \
void CMyClass::Leave()                              \
{                                                   \
    if (m_pLock)                                    \
        m_pLock->Leave();                           \
}                                                   \
IObject *CMyClass::Parent()                         \
{                                                   \
    return m_piObjParent;                           \
}                                                   \
void CMyClass::ConstructClass(int argc, char **argv) \
{                                                   \
    if (m_piParent && (m_piParent->QueryInterface(  \
            ID_INTF(IObject),                       \
            REF_PTR(m_piObjParent),                 \
            this) == SUCCESS))                      \
    {                                               \
        (void)m_piObjParent->Release(this);         \
    }                                               \
    bool __staticInit = false;                      \
    int __idx = 0;                                  \
    while ((argv && (__idx < argc)) || !__staticInit) \
    {


/// -------------------------------------------------
/// 挂接实现类到IObject模板 - 挂接'字符串配置'表项
/// -------------------------------------------------
#define DCOP_IMPLEMENT_CONFIG_STRING(Config, Value) \
    if (argv && (__idx < argc) &&                   \
        (*(argv[__idx]) == '-') &&                  \
        !strcmp(argv[__idx] + 1, Config) &&         \
        (__idx < (argc - 1)))                       \
    {                                               \
        (void)strncpy(Value, argv[__idx + 1],       \
                        sizeof(Value));             \
        Value[sizeof(Value) - 1] = '\0';            \
        __idx += 2;                                 \
    }


/// -------------------------------------------------
/// 挂接实现类到IObject模板 - 挂接'整数配置'表项
/// -------------------------------------------------
#define DCOP_IMPLEMENT_CONFIG_INTEGER(Config, Value) \
    if (argv && (__idx < argc) &&                   \
        (*(argv[__idx]) == '-') &&                  \
        !strcmp(argv[__idx] + 1, Config) &&         \
        (__idx < (argc - 1)))                       \
    {                                               \
        void *__ptr = (void *)&Value;               \
        if (sizeof(Value) == 1)                     \
        {                                           \
            *(BYTE *)__ptr = (BYTE)atoi(            \
                        argv[__idx + 1]);           \
        }                                           \
        else if (sizeof(Value) == 2)                \
        {                                           \
            *(WORD *)__ptr = (WORD)atoi(            \
                        argv[__idx + 1]);           \
        }                                           \
        else                                        \
        {                                           \
            *(DWORD *)__ptr = (DWORD)atoi(          \
                        argv[__idx + 1]);           \
        }                                           \
        __idx += 2;                                 \
    }


/// -------------------------------------------------
/// 挂接实现类到IObject模板 - 挂接'开关配置'表项
/// -------------------------------------------------
#define DCOP_IMPLEMENT_CONFIG_SWITCH(Config, Value) \
    if (argv && (__idx < argc) &&                   \
        (*(argv[__idx]) == '-') &&                  \
        !strcmp(argv[__idx] + 1, Config))           \
    {                                               \
        if ((__idx < (argc - 1)) &&                 \
                       (*(argv[__idx + 1]) != '-')) \
        {                                           \
            if (!strcmp(argv[__idx + 1], "true"))   \
            {                                       \
                Value = true;                       \
            }                                       \
            __idx += 2;                             \
        }                                           \
        else                                        \
        {                                           \
            Value = true;                           \
            __idx += 1;                             \
        }                                           \
    }


/// -------------------------------------------------
/// 挂接实现类到IObject模板 - 挂接'静态标识配置'表项 (建议单实例使用)
/// -------------------------------------------------
#define DCOP_IMPLEMENT_IDENTIFY_STATIC(NAME, ID)    \
    if (!__staticInit)                              \
    {                                               \
        (void)strncpy(m_szObjName, NAME,            \
                        sizeof(m_szObjName));       \
        m_szObjName[sizeof(m_szObjName) - 1] = '\0'; \
        m_dwObjID = ID;                             \
    }


/// -------------------------------------------------
/// 挂接实现类到IObject模板 - 挂接'动态标识配置'表项 (建议多实例使用)
/// -------------------------------------------------
#define DCOP_IMPLEMENT_IDENTIFY_DYNAMIC(name, id)   \
    DCOP_IMPLEMENT_CONFIG_STRING(name, m_szObjName) \
    DCOP_IMPLEMENT_CONFIG_INTEGER(id, m_dwObjID)


/// -------------------------------------------------
/// 挂接实现类到IObject模板 - 挂接'线程安全标识配置'表项 (普通对象配置;子对象和非多线程环境下无需配置)
/// -------------------------------------------------
#define DCOP_IMPLEMENT_CONFIG_THREADSAFE(ThreadSafe) \
    DCOP_IMPLEMENT_CONFIG_SWITCH(ThreadSafe, m_bThreadSafe)


/// -------------------------------------------------
/// 挂接实现类到IObject模板 - 打开'线程安全'开关 (不配置默认是关闭的状态;打开后就无法使用参数配置)
/// -------------------------------------------------
#define DCOP_IMPLEMENT_CONFIG_THREADSAFE_ENABLE()   \
    if (!__staticInit)                              \
    {                                               \
        m_bThreadSafe = true;                       \
    }


/// -------------------------------------------------
/// 挂接实现类到IObject模板 - 打开'消息并发'开关 (默认关闭;不支持配置参数;对象根据自己的情况选择打开)
/// -------------------------------------------------
#define DCOP_IMPLEMENT_CONFIG_CONCURRENT_ENABLE()   \
    if (!__staticInit)                              \
    {                                               \
        m_bConcurrent = true;                       \
    }


/// -------------------------------------------------
/// 挂接实现类到IObject模板 - 挂接表尾
/// -------------------------------------------------
#define DCOP_IMPLEMENT_IOBJECT_END                  \
        ++__idx;                                    \
        __staticInit = true;                        \
    }                                               \
}


/// -------------------------------------------------
/// 实现类声明IObject消息处理模板
/// -------------------------------------------------
#define DCOP_DECLARE_IOBJECT_MSG_HANDLE             \
    void Proc(objMsg *msg)                          \


/// -------------------------------------------------
/// 挂接实现类到IObject消息处理模板 - 挂接表头
/// -------------------------------------------------
#define DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE(CMyClass) \
    void CMyClass::Proc(objMsg *msg)                \
    {                                               \
        if (!msg) return;                           \
        if (!m_bConcurrent) Enter();                \
        switch (msg->GetMsgType())                  \
        {


/// -------------------------------------------------
/// 挂接实现类到IObject消息处理模板 - 挂接表项
/// -------------------------------------------------
#define DCOP_IMPLEMENT_IOBJECT_MSG_PROC(Msg, OnProc) \
            case Msg:                               \
                OnProc(msg);                        \
                break;


/// -------------------------------------------------
/// 挂接实现类到IObject消息处理模板 - 挂接表尾
/// -------------------------------------------------
#define DCOP_IMPLEMENT_IOBJECT_MSG_DEFAULT(OnDefault) \
            default:                                \
                OnDefault(msg);                     \
                break;                              \


/// -------------------------------------------------
/// 挂接实现类到IObject消息处理模板 - 挂接表尾
/// -------------------------------------------------
#define DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE_END       \
        }                                           \
        if (!m_bConcurrent) Leave();                \
    }


/// -------------------------------------------------
/// 查询一个指定类型的对象 - 对象内引用
/// -------------------------------------------------
#define DCOP_QUERY_OBJECT(Type, ID, Parent, objPtr) \
do {                                                \
    IObject *piObjParent = Parent;                  \
    if (!piObjParent) break;                        \
    IObject *piObject = piObjParent->Child(ID);     \
    if (!piObject) break;                           \
    (void)piObject->QueryInterface(                 \
                        ID_INTF(Type),              \
                        REF_PTR(objPtr),            \
                        this);                      \
} while (0)


/// -------------------------------------------------
/// 查询一个指定类型的对象 - 对象外引用(需指定引用者)
/// -------------------------------------------------
#define DCOP_QUERY_OBJECT_REFER(Type, ID, Parent, Refer, objPtr) \
do {                                                \
    IObject *piObjParent = Parent;                  \
    if (!piObjParent) break;                        \
    IObject *piObject = piObjParent->Child(ID);     \
    if (!piObject) break;                           \
    (void)piObject->QueryInterface(                 \
                        ID_INTF(Type),              \
                        REF_PTR(objPtr),            \
                        Refer);                     \
} while (0)


/// -------------------------------------------------
/// 查询多个指定类型的对象 - 表首
/// -------------------------------------------------
#define DCOP_QUERY_OBJECT_START(Parent)             \
    DWORD __rcQueryObj = SUCCESS;                   \
    IObject *__objParent = Parent;                  \
    if (!__objParent)                               \
    {                                               \
        return FAILURE;                             \
    }                                               \
    do {


/// -------------------------------------------------
/// 查询多个指定类型的对象 - 表项
/// -------------------------------------------------
#define DCOP_QUERY_OBJECT_ITEM(Type, ID, objPtr)    \
    IObject *pi##objPtr = __objParent->Child(ID);   \
    if (!pi##objPtr)                                \
    {                                               \
        __rcQueryObj = FAILURE;                     \
        break;                                      \
    }                                               \
    __rcQueryObj = pi##objPtr->QueryInterface(      \
                        ID_INTF(Type),              \
                        REF_PTR(objPtr),            \
                        this);                      \
    if (__rcQueryObj != SUCCESS)                    \
    {                                               \
        break;                                      \
    }


/// -------------------------------------------------
/// 查询多个指定类型的对象 - 表项
/// -------------------------------------------------
#define DCOP_QUERY_OBJECT_END                       \
    } while (0);                                    \
    if (__rcQueryObj != SUCCESS) return __rcQueryObj;


#endif // ifndef _OBJECT_IF_H_

