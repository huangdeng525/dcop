/// -------------------------------------------------
/// config.h : 配置定义公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _DCOP_CONFIG_H_
#define _DCOP_CONFIG_H_


//////////////////////////////////////////////////////
/// 操作系统平台宏(尽量只在封装层使用) - begin
//////////////////////////////////////////////////////
#define DCOP_OS_NONE        0
#define DCOP_OS_WINDOWS     1
#define DCOP_OS_LINUX       2

#if defined(_WINDOWS) || defined(_WINDOWS_) || defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    #define DCOP_OS         DCOP_OS_WINDOWS
#elif defined(__linux) || defined(__linux__)
    #define DCOP_OS         DCOP_OS_LINUX
#else
    #define DCOP_OS         DCOP_OS_NONE
#endif
//////////////////////////////////////////////////////
/// 操作系统平台宏(尽量只在封装层使用) - end
//////////////////////////////////////////////////////



/////////////////////////////////////////////////////
///             stl使用定义
/// -------------------------------------------------
/// 这里使用标准的stl, 所以不使用#include <>, 而使用
/// #include "", 因为编译器可能默认的目录没有标准stl,
///     CB: 默认的是目录是$(BCB)\include下, 并且把
///         包含的标准stl头文件后面加上.h, 所以在CB下
///         要显示指明路进$(BCB)\include\Stlport.
/// 在使用STL的文件开头需要先声明对应的宏,如: INC_MAP
/////////////////////////////////////////////////////
#if defined(INC_STRING)
    #ifdef _MSC_VER
        #pragma warning(disable:4786)
    #endif
    #include "string"
#endif

#if defined(INC_MAP)
    #ifdef _MSC_VER
        #pragma warning(disable:4786)
    #endif
#include "map"
#endif

#if defined(INC_SET)
#include "set"
#endif

#if defined(INC_DEQUE)
#include "deque"
#endif

#if defined(INC_LIST)
#include "list"
#endif
//////////////////////////////////////////////////////
/// stl使用定义 - end
//////////////////////////////////////////////////////


#endif // #ifndef _DCOP_CONFIG_H_

