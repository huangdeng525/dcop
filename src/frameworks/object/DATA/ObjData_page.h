/// -------------------------------------------------
/// ObjData_page.h : 内存页私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJDATA_PAGE_H_
#define _OBJDATA_PAGE_H_

#include "dcop.h"
#include "list/dllist.h"


/// 默认动态页面的记录数
#define DCOP_DATA_DEF_REC_COUNT             16


/// 内存页类
class CMemPage
{
public:

    /// 内存页信息
    struct PageInfo
    {
        BYTE m_byMagicWord0;                // 头部魔术字(固定为'D')
        BYTE m_byMagicWord1;                // 头部魔术字(固定为'c')
        BYTE m_byMagicWord2;                // 头部魔术字(固定为'o')
        BYTE m_byMagicWord3;                // 头部魔术字(固定为'p')
        DWORD m_dwMemCount;                 // 内存容量数目
        DWORD m_dwRecCount;                 // 有效记录数目
        DLL_ENTRY(PageInfo) m_field;        // 链表域
    };

    /// 记录头
    struct RecordHead
    {
        PageInfo *m_pPage;                  // 内存页
        DWORD m_dwMemPos;                   // 偏移值
        DLL_ENTRY(RecordHead) m_field;      // 链表域
    };

public:
    CMemPage();
    ~CMemPage();

    /// 设置动态申请长度
    void SetPageDefSize(DWORD dwPageDefSize) {m_dwPageDefSize = dwPageDefSize;}

    /// 新建一个页面
    PageInfo *NewPage(DWORD dwRecSize, DWORD dwMaxCount);

    /// 添加记录
    RecordHead *AppendRec(BYTE *pbyRec);

    /// 删除记录
    DWORD DeleteRec(RecordHead *pRecord);

    /// 获取第一条有效记录和下一条有效记录
    RecordHead *GetFirstRec() {return DLL_FIRST(&m_records);}
    RecordHead *GetNextRec(RecordHead *pCurRec) {return DLL_NEXT_LOOP(&m_records, pCurRec, m_field);}

    /// 获取指定有效记录
    PageInfo *GetRealRec(DWORD dwRecPos, DWORD &rdwMemPos);

    /// 找到一个空闲记录
    PageInfo *GetIdleRec(DWORD &rdwMemPos);

    /// 清除所有页面
    void ClearPages();

    /// 获取记录个数
    DWORD GetRecCount() {return m_dwRecCount;}

private:
    DWORD m_dwPageDefSize;                  // 页面默认大小(指容纳的记录条数)
    DWORD m_dwRecSize;                      // 记录大小
    DLL_HEAD(PageInfo) m_pages;             // 页面集合
    DLL_HEAD(RecordHead) m_records;         // 记录集合
    DWORD m_dwRecCount;                     // 记录条数
};


#endif // #ifndef _OBJDATA_PAGE_H_

