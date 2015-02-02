/// -------------------------------------------------
/// mem.h : 内存封装公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_MEM_MEM_H_
#define _TOOL_MEM_MEM_H_


/// 本头文件只能由mem.cpp包含


/// 说明: 其实是利用了placement new的语法，通过一个简单的宏，就可以把普通的new操作对应到相应的重载上去:
/// new Class; => new (__FILE__, __LINE__) Class;
/// 但是delete因为没有类似于placement new的语法，所以都是走的这个全局函数:
/// void operator delete(void *p);
/// 但是全局实现和c库会冲突，因此定义为内联函数(在外部使用的头文件os.h中)


#include <map>
#include "sem.h"
#include "fs/file.h"
#include "string/dstring.h"


#undef new              /// new只能使用原生实现
#undef malloc           /// malloc只能使用原生实现
#undef free             /// free只能使用原生实现
#undef realloc          /// realloc只能使用原生实现


/// 尾部校验长度
#define MEM_TAIL_CHECK_LEN 16


/// 内存跟踪类
class CMemTrack
{
public:

    /// 内存信息
    typedef struct
    {
        void *address;
        size_t size;
        const char *file;
        int line;
        CDString callstack;
    }MEM_INFO;

    /// 内存信息记录容器
    typedef std::map<void *, MEM_INFO> MAP_ALLOC;
    typedef MAP_ALLOC::iterator IT_ALLOC;

    /// 内存记录状态
    enum record_status
    {
        record_status_none = 0,         // 未初始化
        record_status_init  ,           // 已初始化
        record_status_start,            // 记录开始
        record_status_end               // 记录结束
    };

public:
    CMemTrack();
    ~CMemTrack();

    /// 开始(会重新初始化所有记录容器)
    void Init();

    /// 结束(处于停止记录状态，但是之后会打印后面调过来的内存释放)
    void Fini();

    /// 获取文件路径名中的名字
    static const char *GetFileName(const char *file);

    /// 内存申请时的轨迹
    void AddTrack(void *address, size_t size, const char *file, int line);

    /// 内存释放时的轨迹
    void RemoveTrack(void *address, const char *file, int line);

    /// 设置和获取详细记录开关
    void SetRecordDetail(bool enable, bool only_cur_task, const char *only_file_name);
    bool GetRecordDetail(const char *file);

    /// 设置是否记录分配调用栈
    void SetRecordAllocCallstack(bool enable) {m_record_alloc_callstack = enable;}

    /// 打印内存信息
    void DumpMemInfo();

public:
    static int m_record_status;

private:
    MAP_ALLOC m_track_alloc_info;
    MAP_ALLOC m_track_free_info;
    int m_alloc_count;
    int m_free_count;
    int m_repeat_free_count;
    int m_over_write_count;
    objLock *m_pLock;
    int m_track_inside;
    bool m_record_detail;
    DWORD m_cur_task_id;
    char m_file_name[DCOP_FILE_NAME_LEN];
    bool m_record_alloc_callstack;
};


#endif // #ifndef _TOOL_MEM_MEM_H_

