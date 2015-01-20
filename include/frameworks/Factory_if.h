/// -------------------------------------------------
/// Factory_if.h : 类厂公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _FACTORY_IF_H_
#define _FACTORY_IF_H_

#include "BaseClass.h"


/// -------------------------------------------------
/// 实例构造函数原型
/// -------------------------------------------------
typedef Instance* (*DCOP_CREATE_INSTANCE_FUNC)(
                        Instance *piParent, 
                        int argc, 
                        char **argv
                        );


/// -------------------------------------------------
/// 定义IFactory版本号
/// -------------------------------------------------
INTF_VER(IFactory, 1, 0, 0);


/// -------------------------------------------------
/// 工厂容器接口
/// -------------------------------------------------
interface IFactory : public Instance
{
    /// 获取类厂单实例
    static IFactory *GetInstance();

    /// 把类插入到类工厂
    virtual DWORD InsertClass(
                        const char *cszName,
                        DCOP_CREATE_INSTANCE_FUNC fnConstruct
                        ) = 0;

    /// 从类工厂里创建实例
    virtual Instance *CreateInstance(
                        const char *cszName,
                        Instance *piParent = 0,
                        int argc = 0,
                        char **argv = 0
                        ) = 0;
};


/// -------------------------------------------------
/// 实现类工厂导入类模板
/// -------------------------------------------------
#define DCOP_IMPLEMENT_FACTORY(CMyClass, Name)      \
Instance *___CREATE_##CMyClass(                     \
    Instance *piParent, int argc, char **argv)      \
{                                                   \
    CMyClass *pMyClass = new CMyClass(              \
                        piParent, argc, argv);      \
    if (!pMyClass) return 0;                        \
    Instance *pInstnace = 0;                        \
    (void)pMyClass->QueryInterface(ID_INTF(Instance), \
                        REF_PTR(pInstnace));        \
    if (!pInstnace) delete pMyClass;                \
    else (void)pInstnace->Release();                \
    return pInstnace;                               \
}                                                   \
void vReg##CMyClass##ToFactoryFunc()                \
{                                                   \
    IFactory *piFactory = IFactory::GetInstance();  \
    if (!piFactory) return;                         \
    (void)piFactory->InsertClass(Name,              \
                        ___CREATE_##CMyClass);      \
    (void)piFactory->Release();                     \
}                                                   \
CPPBUILDUNIT_AUTO(vReg##CMyClass##ToFactoryFunc, 0);


/// -------------------------------------------------
/// 实现类工厂导入类模板(单件)
/// -------------------------------------------------
#define DCOP_IMPLEMENT_FACTORY_SINGLETON(CMyClass, Name) \
Instance *___CREATE_##CMyClass(                     \
    Instance *piParent, int argc, char **argv)      \
{                                                   \
    bool bCreateFlag = false;                       \
    static CMyClass *s_pMyClass = 0;                \
    if (!s_pMyClass)                                \
    {                                               \
        s_pMyClass = new CMyClass(                  \
                        piParent, argc, argv);      \
        if (!s_pMyClass) return 0;                  \
        bCreateFlag = true;                         \
    }                                               \
    Instance *pInstnace = 0;                        \
    (void)s_pMyClass->QueryInterface(               \
                        ID_INTF(Instance),          \
                        REF_PTR(pInstnace));        \
    if (!pInstnace)                                 \
    {                                               \
        if (bCreateFlag) {delete s_pMyClass;        \
                        s_pMyClass = 0;}            \
        else (void)s_pMyClass->Release();           \
    }                                               \
    else                                            \
    {                                               \
        if (bCreateFlag) (void)pInstnace->Release(); \
    }                                               \
    return pInstnace;                               \
}                                                   \
void vReg##CMyClass##ToFactoryFunc()                \
{                                                   \
    IFactory *piFactory = IFactory::GetInstance();  \
    if (!piFactory) return;                         \
    (void)piFactory->InsertClass(Name,              \
                      ___CREATE_##CMyClass);        \
    (void)piFactory->Release();                     \
}                                                   \
CPPBUILDUNIT_AUTO(vReg##CMyClass##ToFactoryFunc, 0);


/// -------------------------------------------------
/// 创建一个指定类型的实例(请在使用前一定要初始化'Ptr'为NULL值)
/// -------------------------------------------------
#define DCOP_CREATE_INSTANCE(Type, Name, Parent, argc, argv, objPtr) \
TRACE_LOG(STR_FORMAT("START CREATE INSTANCE '%s' : "#objPtr, Name)); \
do {                                                \
    IFactory *piFactory = IFactory::GetInstance();  \
    if (piFactory)                                  \
    {                                               \
        Instance *pInstnace = piFactory->CreateInstance(Name, Parent, argc, argv); \
        (void)piFactory->Release();                 \
        if (!pInstnace) break;                      \
        if (pInstnace->QueryInterface(ID_INTF(Type), REF_PTR(objPtr)) != SUCCESS) \
        {                                           \
            delete pInstnace;                       \
            pInstnace = 0;                          \
            break;                                  \
        }                                           \
        (void)pInstnace->Release();                 \
    }                                               \
} while (0);                                        \
TRACE_LOG(STR_FORMAT("ENDOF CREATE INSTANCE '%s' : "#objPtr" = %p", Name, (void *)objPtr))


/// -------------------------------------------------
/// 释放一个实例(有计数器检查，用于最后一次释放)
/// -------------------------------------------------
#define DCOP_RELEASE_INSTANCE(objPtr)               \
do {                                                \
    if (objPtr)                                     \
    {                                               \
        TRACE_LOG(STR_FORMAT("START RELEASE INSTANCE "#objPtr" = %p", (void *)objPtr)); \
        DWORD refCount = objPtr->Release(this);     \
        TRACE_LOG(STR_FORMAT("ENDOF RELEASE INSTANCE "#objPtr" Ref Count = %lu", refCount)); \
        objPtr = 0;                                 \
    }                                               \
} while (0)


/// -------------------------------------------------
/// 释放一个实例(需指定引用者)
/// -------------------------------------------------
#define DCOP_RELEASE_INSTANCE_REFER(Refer, objPtr)  \
do {                                                \
    if (objPtr)                                     \
    {                                               \
        TRACE_LOG(STR_FORMAT("START RELEASE INSTANCE "#objPtr" = %p", (void *)objPtr)); \
        DWORD refCount = objPtr->Release(Refer);    \
        TRACE_LOG(STR_FORMAT("ENDOF RELEASE INSTANCE "#objPtr" Ref Count = %lu", refCount)); \
        objPtr = 0;                                 \
    }                                               \
} while (0)


#endif // #ifndef _FACTORY_IF_H_

