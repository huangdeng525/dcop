/// -------------------------------------------------
/// os.h : OS定义公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _DCOP_OS_H_
#define _DCOP_OS_H_

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#if DCOP_OS == DCOP_OS_LINUX
#include <stdint.h>
#endif
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <time.h>
#ifdef __cplusplus
#include <new>
#endif


//////////////////////////////////////////////////////
/// OS句柄
//////////////////////////////////////////////////////
typedef void               *OSHANDLE;


//////////////////////////////////////////////////////
/// 永远等待
//////////////////////////////////////////////////////
#define OSWAIT_FOREVER      0xFFFFFFFFL


//////////////////////////////////////////////////////
/// NAME长度
//////////////////////////////////////////////////////
#define OSNAME_LENGTH       16


//////////////////////////////////////////////////////
/// BUF头部魔术字
//////////////////////////////////////////////////////
#define OSBUF_MAGIC         "Dcop"
#define OSBUF_MAGIC0        'D'
#define OSBUF_MAGIC1        'c'
#define OSBUF_MAGIC2        'o'
#define OSBUF_MAGIC3        'p'


//////////////////////////////////////////////////////
/// 使用下面的检查必须在函数结尾处申明"ERROR_LABEL"
///     标签, 并进行错误处理
//////////////////////////////////////////////////////
#define ERROR_CHECK(exp)        \
    do                          \
    {                           \
        if (!(exp))             \
        {                       \
            goto ERROR_LABEL;   \
        }                       \
    }while(0)


//////////////////////////////////////////////////////
/// 回调函数调用宏(有返回值的一般调用)
/// 用法示例: dwRc = BACKFUNC_CALL(callback)(...);
///     相当于调用 dwRc = callback(...);
//////////////////////////////////////////////////////
#define BACKFUNC_CALL(pFunc) \
    (!(pFunc))? (FAILURE) : (pFunc)

//////////////////////////////////////////////////////
/// 回调函数调用宏(有返回值的一般调用)
/// 用法示例: b = BBACKFUNC_CALL(callback)(...);
///     相当于调用 b = callback(...);
//////////////////////////////////////////////////////
#define BBACKFUNC_CALL(pFunc) \
    (!(pFunc))? (false) : (pFunc)

//////////////////////////////////////////////////////
/// 回调函数调用宏(无返回值的一般调用)
/// 用法示例: VDBACKFUNC_CALL(callback)(...);
///     相当于调用 callback(...);
//////////////////////////////////////////////////////
#define VDBACKFUNC_VDCALL(pFunc) \
    if (pFunc) (pFunc)

//////////////////////////////////////////////////////
/// 回调函数调用宏(有返回值的void调用)
/// 用法示例: BACKFUNC_VDCALL(callback)(...);
///     相当于调用 (void)callback(...);
//////////////////////////////////////////////////////
#define BACKFUNC_VDCALL(pFunc) \
    if (pFunc) (void)(pFunc)


/////////////////////////////////////////////////////
/// 重载内存接口，方便定位内存问题
/////////////////////////////////////////////////////
/// 先屏蔽掉别的地方的宏定义
#ifdef __cplusplus
    #ifdef new
        #undef new
    #endif
    #ifdef delete
        #undef delete
    #endif
#endif

#ifdef malloc
    #undef malloc
#endif

#ifdef free
    #undef free
#endif


/////////////////////////////////////////////////////
///                 内存申请和释放
/// -------------------------------------------------
/// 内存申请和释放请使用宏:
///     DCOP_Malloc
///     DCOP_Free
/// 打开和关闭内存跟踪状态:
///     DebugMemStatus(1);  // 输出到文件中
///     DebugMemStatus(2);  // 以printf方式输出
///     DebugMemStatus(3);  // 同时以文件+printf输出
///     DebugMemStatus(0);  // 关闭输出
/// 打印内存调试信息:
///     DumpMemInfo();
/////////////////////////////////////////////////////
#ifdef __cplusplus
    extern "C" {
#endif
extern void *               DCOP_MallocEx(size_t size, const char *file, int line);
extern void                 DCOP_FreeEx(void *p, const char *file, int line);
extern void *               DCOP_ReallocEx(void *p, size_t size, const char *file, int line);
extern void                 DebugMemStatus(int status);
extern void                 OutputMemLog(int console);
extern void                 RecordMemDetail(int enable, int only_cur_task, const char *only_file_name);
extern void                 RecordAllocCallstack(int enable);
extern void                 DumpMemInfo();
extern size_t               GetMemAllocCount();
extern size_t               GetMemFreeCount();
extern size_t               GetMemDoubleFreeCount();
extern size_t               GetMemOverWriteCount();
extern size_t               GetMemTotalSize();
#ifdef __cplusplus
    }
#endif
#ifdef __cplusplus
    #ifdef _MSC_VER
        #pragma warning(disable:4290)
    #endif
    inline void *operator   new(size_t size) throw (std::bad_alloc) {return DCOP_MallocEx(size, 0, 0);}
    inline void *operator   new[](size_t size) throw (std::bad_alloc) {return DCOP_MallocEx(size, 0, 0);}
    inline void *operator   new(size_t size, const char *file, int line) throw (std::bad_alloc) {return DCOP_MallocEx(size, file, line);}
    inline void *operator   new[](size_t size, const char *file, int line) throw (std::bad_alloc) {return DCOP_MallocEx(size, file, line);}
    inline void operator    delete(void *p) throw() {DCOP_FreeEx(p, 0, 0);}
    inline void operator    delete[](void *p) throw() {DCOP_FreeEx(p, 0, 0);}
    inline void operator    delete(void *p, const char *file, int line) throw() {DCOP_FreeEx(p, file, line);}
    inline void operator    delete[](void *p, const char *file, int line) throw() {DCOP_FreeEx(p, file, line);}
#endif

/////////////////////////////////////////////////////
/// 在需要检测的文件里，重定义new
/////////////////////////////////////////////////////
#ifdef __cplusplus
    #define new             new(__FILE__, __LINE__)
#endif

/////////////////////////////////////////////////////
/// 主动内存申请和释放接口
/////////////////////////////////////////////////////
#define DCOP_Malloc(size)   DCOP_MallocEx((size_t)size, __FILE__, __LINE__)
#define DCOP_Free(ptr)      DCOP_FreeEx(ptr, __FILE__, __LINE__)
#define DCOP_Realloc(ptr,size) DCOP_ReallocEx(ptr, (size_t)size, __FILE__, __LINE__)
#define malloc(size)        DCOP_MallocEx((size_t)size, __FILE__, __LINE__)
#define free(ptr)           DCOP_FreeEx(ptr, __FILE__, __LINE__)
#define realloc(ptr,size)   DCOP_ReallocEx(ptr, (size_t)size, __FILE__, __LINE__)


/////////////////////////////////////////////////////
///              日志接口使用说明
/// -------------------------------------------------
/// 记录跟踪信息:
///     TRACE_LOG("跟踪信息");
///     TRACE_LOG(STR_FORMAT("跟踪信息:%s", str));
/// 检查错误信息:
///     CHECK_ERRCODE(RC, "错误信息");
///     CHECK_ERRCODE(RC, STR_FORMAT("错误信息:%s", str));
/// 打开和关闭调试日志开关:
///     DebugLogStatus(1);
///     DebugLogStatus(0);
/////////////////////////////////////////////////////

/// [注意] 字符串格式化后最大长度不超过256(包括'\0')
#define STR_FORMAT_LEN_MAX          256

/// 记录跟踪信息(有开关控制的调试信息)
#define TRACE_LOG(Info)             TraceLogEx(Info, __FILE__, __LINE__)
#define TRACE_BUF(Info, Buf, Len)   TraceBufEx(Info, Buf, Len, __FILE__, __LINE__)

/// 检查返回码(不等于SUCESS生效;记录错误日志但不会复位)
#define CHECK_RETCODE(Rc, Info)     CheckRetCodeEx((int)(Rc), (int)SUCCESS, (const char *)(Info), __FILE__, __LINE__)

/// 检查错误码(不等于SUCESS生效;记录错误日志并且会复位)
#define CHECK_ERRCODE(Rc, Info)     CheckErrCodeEx((int)(Rc), (int)SUCCESS, (const char *)(Info), __FILE__, __LINE__)

/// 日志打印回调
typedef void (*LOG_PRINT)(const char *info, void *para);
typedef void * LOG_PARA;

/// 打印日志
#define PrintLog(Info, Print, Para) PrintLogEx((const char *)(Info), (LOG_PRINT)(Print), (LOG_PARA)(Para), __FILE__, __LINE__)

/// 打印内存
#define PrintBuffer(Info, Buf, Len, Print, Para) PrintBufferEx((const char *)(Info), (const void *)(Buf), (size_t)(Len), (LOG_PRINT)(Print), (LOG_PARA)(Para), __FILE__, __LINE__)

/// 跟踪信息和错误检查
#ifdef __cplusplus
    extern "C" {
#endif
extern void ShowCallStack(LOG_PRINT print, LOG_PARA para, int depth);
extern void TraceLogEx(const char *info, const char *file, int line);
extern void TraceBufEx(const char *info, const void *buf, size_t len, const char *file, int line);
extern void CheckRetCodeEx(int rc, int expect, const char *info, const char *file, int line);
extern void CheckErrCodeEx(int rc, int expect, const char *info, const char *file, int line);
extern void DebugLogStatus(int status);
extern int  GetLogTime(char *szStr, int strLen);
extern int  GetLogFileNameAndLine(char *szStr, int strLen, const char *file, int line);
extern void PrintLogEx(const char *info, LOG_PRINT print, LOG_PARA para, const char *file, int line);
extern void PrintBufferEx(const char *info, const void *buf, size_t len, LOG_PRINT print, LOG_PARA para, const char *file, int line);
extern void PrintToConsole(const char *info, void *para);
#ifdef __cplusplus
    }
#endif


/////////////////////////////////////////////////////
/// 格式化变参字符串[用法:TRACE_LOG(STR_FORMAT("xxx:%s", yyy));]
/// -------------------------------------------------
#ifdef __cplusplus
#define STR_FORMAT (const char *)CStrFormat
class CStrFormat
{
public:
    CStrFormat(const char *format, ...)
    {
        va_list args;

        va_start(args, format);
        (void)vsnprintf(m_szStr, sizeof(m_szStr), format, args);
        va_end(args);

        m_szStr[sizeof(m_szStr) - 1] = '\0';
    }
    ~CStrFormat() {}

    operator const char *() { return m_szStr;}

private:
    CStrFormat() {}
    char m_szStr[STR_FORMAT_LEN_MAX];
};
#endif
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
/// cpp编译单元析构tmp类自动执行
/// 利用全局类对象自动运行其构造函数的原理
/// -------------------------------------------------
#ifdef __cplusplus
#define CPPBUILDUNIT_AUTO(constructFunc, destroyFunc) \
class CBuildUnitAutoTmp##constructFunc##destroyFunc \
{                                                   \
public:                                             \
    CBuildUnitAutoTmp##constructFunc##destroyFunc() \
    {                                               \
        void (*__fn__)() = constructFunc;           \
        if (__fn__) __fn__();                       \
    }                                               \
    ~CBuildUnitAutoTmp##constructFunc##destroyFunc() \
    {                                               \
        void (*__fn__)() = destroyFunc;             \
        if (__fn__) __fn__();                       \
    }                                               \
} __tmp##constructFunc##destroyFunc
#endif // #ifdef __cplusplus
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
/// 用于.a或者忽略全局对象的.o中的强制连接
/// 无法产生全局对象就无法自动执行上面的函数
/// -------------------------------------------------
#ifdef __cplusplus
#define CPPBUILDUNIT_FORCELINK(constructFunc, destroyFunc) \
class CBuildUnitAutoTmp##constructFunc##destroyFunc; \
extern CBuildUnitAutoTmp##constructFunc##destroyFunc __tmp##constructFunc##destroyFunc; \
void *__forcelink##constructFunc##destroyFunc = (void *)&__tmp##constructFunc##destroyFunc
#endif // #ifdef __cplusplus
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////
/// ASSERT定义宏
//////////////////////////////////////////////////////
#ifdef __cplusplus
    extern "C" {
#endif
extern void DCOP_Assert(int x, const char *file, int line);
#ifdef __cplusplus
    }
#endif
#define OSASSERT(x) DCOP_Assert((int)(x), __FILE__, __LINE__)


/////////////////////////////////////////////////////
/// 复位处理
/////////////////////////////////////////////////////
#ifdef __cplusplus
    extern "C" {
#endif
extern void DCOP_Reset(int type, const char *file, int line);
#ifdef __cplusplus
    }
#endif
#define OSREBOOT(type) DCOP_Reset((int)(type), __FILE__, __LINE__)


/////////////////////////////////////////////////////
/// 不同os下的C库差异
/////////////////////////////////////////////////////
#if DCOP_OS == DCOP_OS_WINDOWS
    #define snprintf    _snprintf
    #define stricmp     _stricmp
    #define strnicmp    _strnicmp
#endif
#if DCOP_OS == DCOP_OS_LINUX
    #include <strings.h>
    #define stricmp     strcasecmp
    #define strnicmp    strncasecmp
#endif


/////////////////////////////////////////////////////
/// 获取消耗时间
/////////////////////////////////////////////////////
#define DCOP_START_TIME()   clock_t __start_time=clock()
#define DCOP_END_TIME()     clock_t __end_time=clock()
#define DCOP_COST_TIME()    int((__end_time-__start_time)/(CLOCKS_PER_SEC/1000))


#endif // #ifndef _DCOP_OS_H_

