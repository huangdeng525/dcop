/// -------------------------------------------------
/// mem.cpp : 内存封装实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "mem.h"
#include "task.h"
#include "../log/log.h"


/// -------------------------------------------------
/// 全局内存调试接口
/// -------------------------------------------------
CMemTrack g_mem_track;
int CMemTrack::m_record_status = CMemTrack::record_status_none;
static CLog *sg_pMemLog = 0;


/*******************************************************
  函 数 名: CMemTrack::CMemTrack
  描    述: 内存跟踪构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CMemTrack::CMemTrack()
{
    m_record_status = record_status_none;

    m_alloc_count = 0;
    m_free_count = 0;
    m_repeat_free_count = 0;
    m_over_write_count = 0;
    m_pLock = 0;
    m_track_inside = 0;
    m_record_detail = false;
    m_cur_task_id = 0;
    (void)memset(m_file_name, 0, sizeof(m_file_name));
}

/*******************************************************
  函 数 名: CMemTrack::~CMemTrack
  描    述: 内存跟踪析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CMemTrack::~CMemTrack()
{
    Fini();

    m_record_status = record_status_none;
}

/*******************************************************
  函 数 名: CMemTrack::Init
  描    述: 初始化
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CMemTrack::Init()
{
    if (!m_pLock)
    {
        m_pLock = DCOP_CreateLock();
    }

    if (!sg_pMemLog)
    {
        sg_pMemLog = CLog::Init(LOG_ID_MEMLOG, "mem.log");
    }

    /// test ... 
    ShowCallStack(0, 0, 0);

    m_record_status = record_status_init;
}

/*******************************************************
  函 数 名: CMemTrack::Fini
  描    述: 完成
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CMemTrack::Fini()
{
    /// 防止在dump或者释放时也调入了内存跟踪函数
    m_record_status = record_status_none;

    DumpMemInfo();

    m_track_alloc_info.clear();
    m_track_free_info.clear();
    if (m_pLock)
    {
        delete m_pLock;
    }

    m_record_status = record_status_end;
}

/*******************************************************
  函 数 名: CMemTrack::GetFileName
  描    述: 获取文件名
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
const char *CMemTrack::GetFileName(const char *file)
{
    if (!file) return 0;

    const char *pFileName = strrchr(file, '\\');
    if (!pFileName) pFileName = strrchr(file, '/');
    if (!pFileName) return file;

    return ++pFileName;
}

/*******************************************************
  函 数 名: CMemTrack::AddTrack
  描    述: 添加轨迹
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CMemTrack::AddTrack(void *address, size_t size, const char *file, int line)
{
    AutoLock(m_pLock);

    m_record_status = record_status_start;

    /// 防止递归调用
    if (m_track_inside)
    {
        return;
    }

    m_track_inside = 1;

    file = GetFileName(file);

    if (sg_pMemLog && g_mem_track.GetRecordDetail(file))
    {
        sg_pMemLog->Write(STR_FORMAT(" ADDRESS '%p' (len:%d) Alloc in %s:%d, curTask:%d. \r\n",
                        address,
                        size,
                        file,
                        line,
                        objTask::Current()));
    }

    MEM_INFO info;
    info.address = address;
    info.size = size;
    info.file = file;
    info.line = line;

    /// 尾部校验
    for(int i = 0; i < (int)(MEM_TAIL_CHECK_LEN/sizeof(DWORD)); ++i)
    {
        memcpy((void *)((char *)address + size + i*sizeof(DWORD)), OSBUF_MAGIC, sizeof(DWORD));
    }

    /// 申请记录到map容器
    m_alloc_count++;
    m_track_alloc_info.insert(MAP_ALLOC::value_type(address, info));
    m_track_free_info.erase(address);

    m_track_inside = 0;
};

/*******************************************************
  函 数 名: CMemTrack::RemoveTrack
  描    述: 删除轨迹
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CMemTrack::RemoveTrack(void *address, const char *file, int line)
{
    AutoLock(m_pLock);

    if (record_status_end == m_record_status)
    {
        if (sg_pMemLog)
        {
            sg_pMemLog->Write(STR_FORMAT(" ADDRESS '%p' is removed after record over! Free in %s:%d. \r\n", 
                        address, 
                        GetFileName(file), 
                        line));
            ShowCallStack(CLog::PrintCallBack, sg_pMemLog, 0);
        }
        /// 记录已经结束
        return;
    }

    if (record_status_start != m_record_status)
    {
        /// 还未开始记录
        return;
    }

    /// 防止递归调用
    if (m_track_inside)
    {
        return;
    }

    m_track_inside = 1;

    file = GetFileName(file);

    if (sg_pMemLog && g_mem_track.GetRecordDetail(file))
    {
        sg_pMemLog->Write(STR_FORMAT(" ADDRESS '%p' Free in %s:%d, curTask:%d. \r\n",
                        address,
                        file,
                        line,
                        objTask::Current()));
    }

    /// 从申请容器中查找记录
    IT_ALLOC it = m_track_alloc_info.find(address);
    if (it == m_track_alloc_info.end())
    {
        /// 没有申请，看是否是释放过
        it = m_track_free_info.find(address);
        if (it != m_track_free_info.end())
        {
            /// 释放有记录，说明是重复释放
            m_repeat_free_count++;
            if (sg_pMemLog)
            {
                sg_pMemLog->Write(STR_FORMAT(" ADDRESS '%p' %d bytes doublefreed! Previously free in %s:%d, Currently free in %s:%d. \r\n", 
                        ((*it).second).address, 
                        ((*it).second).size, 
                        ((*it).second).file, 
                        ((*it).second).line, 
                        file, 
                        line));
                ShowCallStack(CLog::PrintCallBack, sg_pMemLog, 0);
            }
        }

        m_track_inside = 0;
        
        return;
    }

    /// 尾部校验
    for(int i = 0; i < (int)(MEM_TAIL_CHECK_LEN/sizeof(DWORD)); ++i)
    {
        if (memcmp((void *)((char *)address + ((*it).second).size + i*sizeof(DWORD)), 
                        OSBUF_MAGIC, sizeof(DWORD)) != 0)
        {
            m_over_write_count++;
            if (sg_pMemLog)
            {
                sg_pMemLog->Write(STR_FORMAT("ADDRESS '%p' %d bytes overwrited! Alloc in %s:%d, Free in %s:%d. \r\n", 
                        ((*it).second).address, 
                        ((*it).second).size, 
                        ((*it).second).file, 
                        ((*it).second).line, 
                        file, 
                        line), 
                        (char *)address + ((*it).second).size, 
                        MEM_TAIL_CHECK_LEN, 
                        0, 
                        0);
                ShowCallStack(CLog::PrintCallBack, sg_pMemLog, 0);
            }
            break;
        }
    }

    /// 将记录更新为当前释放，如果没有文件，就保持申请时的文件和行号
    if (file)
    {
        ((*it).second).file = file;
        ((*it).second).line = line;
    }

    m_free_count++;
    m_track_free_info.insert(MAP_ALLOC::value_type(address, (*it).second));
    m_track_alloc_info.erase(address);

    m_track_inside = 0;
};

/*******************************************************
  函 数 名: CMemTrack::SetRecordDetail
  描    述: 设置详细记录开关
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CMemTrack::SetRecordDetail(bool enable, bool only_cur_task, const char *only_file_name)
{
    m_record_detail = enable;

    if (only_cur_task)
    {
        m_cur_task_id = objTask::Current();
    }
    else
    {
        m_cur_task_id = 0;
    }

    if (only_file_name && (*only_file_name))
    {
        (void)snprintf(m_file_name, sizeof(m_file_name), "%s", only_file_name);
        m_file_name[sizeof(m_file_name) - 1] = '\0';
    }
    else
    {
        (void)memset(m_file_name, 0, sizeof(m_file_name));
    }
}

/*******************************************************
  函 数 名: CMemTrack::GetRecordDetail
  描    述: 获取详细记录开关
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CMemTrack::GetRecordDetail(const char *file)
{
    if (!m_record_detail)
    {
        return false;
    }

    if (m_cur_task_id && (objTask::Current() != m_cur_task_id))
    {
        return false;
    }

    if (*m_file_name)
    {
        if (file && (*file) && strstr(file, m_file_name))
        {
            return true;
        }

        return false;
    }

    return true;
}

/*******************************************************
  函 数 名: CMemTrack::DumpMemInfo
  描    述: 打印内存信息
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CMemTrack::DumpMemInfo()
{
    size_t totalSize = 0;
    char szStr[STR_FORMAT_LEN_MAX];

    AutoLock(m_pLock);

    int pos = GetLogTime(szStr, sizeof(szStr)-1);
    pos += snprintf(szStr+pos, sizeof(szStr)-pos-1, "DumpMemInfo ... \r\n");
    if (sg_pMemLog) sg_pMemLog->Write(szStr);

    for(IT_ALLOC i = m_track_alloc_info.begin(); i != m_track_alloc_info.end(); i++)
    {
        if (sg_pMemLog)
        {
            void *address = ((*i).second).address;
            size_t size = ((*i).second).size;
            const char *file = ((*i).second).file;
            int line = ((*i).second).line;
            sg_pMemLog->Write(STR_FORMAT(" ADDRESS '%p' %d bytes unfreed! Alloc in %s:%d. \r\n", 
                        address, 
                        size, 
                        file, 
                        line), 
                        address, 
                        ((size > 64)? 64 : size));
        }
        totalSize += ((*i).second).size;
    }

    if (sg_pMemLog)
    {
        sg_pMemLog->Write(STR_FORMAT("------------------------------------------- \r\n"));
        sg_pMemLog->Write(STR_FORMAT("    Total Unfreed: %d bytes \r\n", totalSize));
        sg_pMemLog->Write(STR_FORMAT("            Alloc: %d times \r\n", m_alloc_count));
        sg_pMemLog->Write(STR_FORMAT("             Free: %d times \r\n", m_free_count));
        sg_pMemLog->Write(STR_FORMAT("             Leak: %d times \r\n", m_track_alloc_info.size()));
        sg_pMemLog->Write(STR_FORMAT("           Repeat: %d times \r\n", m_repeat_free_count));
        sg_pMemLog->Write(STR_FORMAT("             Over: %d times \r\n", m_over_write_count));
        sg_pMemLog->Write(STR_FORMAT("\r\n"));
    }
}

/*******************************************************
  函 数 名: DCOP_MallocEx
  描    述: 内存申请
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void *DCOP_MallocEx(size_t size, const char *file, int line)
{
    if (!size)
    {
        return 0;
    }

    void *ptr = (void *)malloc(size + MEM_TAIL_CHECK_LEN);
    if (!ptr)
    {
        return 0;
    }

    if (CMemTrack::m_record_status >= CMemTrack::record_status_init)
    {
        g_mem_track.AddTrack(ptr, size, file, line);
    }

    return ptr;
}

/*******************************************************
  函 数 名: DCOP_FreeEx
  描    述: 内存释放
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void DCOP_FreeEx(void *p, const char *file, int line)
{
    if (!p)
    {
        return;
    }

    if (CMemTrack::m_record_status >= CMemTrack::record_status_init)
    {
        g_mem_track.RemoveTrack(p, file, line);
    }

    free(p);
}

/*******************************************************
  函 数 名: DCOP_ReallocEx
  描    述: 内存重申请
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void *DCOP_ReallocEx(void *p, size_t size, const char *file, int line)
{
    if (!size)
    {
        return 0;
    }

    if (p && (CMemTrack::m_record_status >= CMemTrack::record_status_init))
    {
        g_mem_track.RemoveTrack(p, file, line);
    }

    void *ptr = (void *)realloc(p, size + MEM_TAIL_CHECK_LEN);
    if (!ptr)
    {
        return 0;
    }

    if (CMemTrack::m_record_status >= CMemTrack::record_status_init)
    {
        g_mem_track.AddTrack(ptr, size, file, line);
    }

    return ptr;
}

/*******************************************************
  函 数 名: DebugMemStatus
  描    述: 内存跟踪的入口和出口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void DebugMemStatus(int status)
{
    if (status)
    {
        if (CMemTrack::record_status_none == CMemTrack::m_record_status)
        {
            g_mem_track.Init();
        }
    }
    else
    {
        if (CMemTrack::record_status_none != CMemTrack::m_record_status)
        {
            g_mem_track.Fini();
        }
    }
}

/*******************************************************
  函 数 名: OutputMemLog
  描    述: 输出内存日志(到控制台)
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void OutputMemLog(int console)
{
    if (sg_pMemLog)
    {
        sg_pMemLog->OutputToConsole((console)? true : false);
    }
}

/*******************************************************
  函 数 名: RecordMemDetail
  描    述: 详细记录开关
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void RecordMemDetail(int enable, int only_cur_task, const char *only_file_name)
{
    g_mem_track.SetRecordDetail((enable)? true : false, (only_cur_task)? true : false, only_file_name);
}

/*******************************************************
  函 数 名: DumpMemInfo
  描    述: 内存打印接口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void DumpMemInfo()
{
    g_mem_track.DumpMemInfo();
}

