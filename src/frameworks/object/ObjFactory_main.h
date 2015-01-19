/// -------------------------------------------------
/// ObjFactory_main.h : 类厂私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJFACTORY_MAIN_H_
#define _OBJFACTORY_MAIN_H_

#define INC_STRING
#define INC_MAP

#include "Factory_if.h"


class CClassFactory : public IFactory
{
public:
    static CClassFactory *sm_pClassFactory;

    typedef std::map<std::string, DCOP_CREATE_INSTANCE_FUNC> MAP_INSTANCES;
    typedef MAP_INSTANCES::iterator IT_INSTANCES;

    CClassFactory();
    ~CClassFactory();

    DCOP_DECLARE_INSTANCE;

    DWORD InsertClass(const char *cszName,
                        DCOP_CREATE_INSTANCE_FUNC fnConstruct);

    Instance *CreateInstance(const char *cszName,
                        Instance *piParent,
                        int argc,
                        char **argv);

    void Dump(LOG_PRINT logPrint, 
                        LOG_PARA logPara, 
                        int argc, 
                        void **argv);

private:
    MAP_INSTANCES m_instances;
};


#endif // #ifndef _OBJFACTORY_MAIN_H_

