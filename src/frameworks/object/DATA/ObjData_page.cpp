/// -------------------------------------------------
/// ObjData_page.cpp : 内存页实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjData_page.h"


/*******************************************************
  函 数 名: CMemPage::CMemPage
  描    述: CMemPage构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CMemPage::CMemPage()
{
    m_dwPageDefSize = DCOP_DATA_DEF_REC_COUNT;
    m_dwRecSize = 0;
    DLL_INIT(&m_pages);
    DLL_INIT(&m_records);
    m_dwRecCount = 0;
}

/*******************************************************
  函 数 名: CMemPage::~CMemPage
  描    述: CMemPage析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CMemPage::~CMemPage()
{
    ClearPages();
}

/*******************************************************
  函 数 名: CMemPage::NewPage
  描    述: 新建页面
  输    入: dwRecSize
            dwMaxCount
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CMemPage::PageInfo *CMemPage::NewPage(DWORD dwRecSize, DWORD dwMaxCount)
{
    /// 不为空时，记录大小必须和设定的一致
    if (!DLL_EMPTY(&m_pages))
    {
        if (dwRecSize != m_dwRecSize)
        {
            return NULL;
        }
    }

    /// 计算页面实际大小
    DWORD dwMemLen = dwMaxCount * (sizeof(RecordHead) + dwRecSize);
    if (!dwMemLen)
    {
        return NULL;
    }

    /// 申请页面内存
    PageInfo *pPage = (PageInfo *)DCOP_Malloc(sizeof(PageInfo) + dwMemLen);
    if (!pPage)
    {
        return NULL;
    }

    /// 初始化页面
    pPage->m_byMagicWord0 = OSBUF_MAGIC0;
    pPage->m_byMagicWord1 = OSBUF_MAGIC1;
    pPage->m_byMagicWord2 = OSBUF_MAGIC2;
    pPage->m_byMagicWord3 = OSBUF_MAGIC3;
    pPage->m_dwMemCount = dwMaxCount;
    pPage->m_dwRecCount = 0;

    (void)memset(pPage + 1, 0, dwMemLen);

    /// 第一次的输入作为设定值保存下来
    if (DLL_EMPTY(&m_pages))
    {
        m_dwRecSize = dwRecSize;
    }

    /// 添加页面到尾部
    DLL_INSERT_TAIL(&m_pages, pPage, m_field);

    return pPage;
}

/*******************************************************
  函 数 名: CMemPage::AppendRec
  描    述: 添加记录(自动找到已删除的空闲位置，没有空闲位置的话就新建页面)
  输    入: pbyRec
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CMemPage::RecordHead *CMemPage::AppendRec(BYTE *pbyRec)
{
    /// 找到一个空闲记录或者申请一个新的页面
    DWORD dwMemPos = 0;
    PageInfo *pPage = GetIdleRec(dwMemPos);
    if (!pPage)
    {
        pPage = NewPage(m_dwRecSize, m_dwPageDefSize);
        if (!pPage)
        {
            return NULL;
        }

        dwMemPos = 0;
    }

    /// 拷贝记录
    BYTE *pbyRecStart = (BYTE *)(pPage + 1);
    RecordHead *pRecord = (RecordHead *)(pbyRecStart + dwMemPos * (sizeof(RecordHead) + m_dwRecSize));
    (void)memcpy(pRecord + 1, pbyRec, m_dwRecSize);

    /// 把记录挂接到链表末
    DLL_INSERT_TAIL(&m_records, pRecord, m_field);
    pRecord->m_pPage = pPage;
    pRecord->m_dwMemPos = dwMemPos;

    /// 记录数加1
    pPage->m_dwRecCount++;
    m_dwRecCount++;

    return pRecord;
}

/*******************************************************
  函 数 名: CMemPage::DeleteRec
  描    述: 删除记录
  输    入: dwRecPos
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CMemPage::DeleteRec(RecordHead *pRecord)
{
    if (!pRecord)
    {
        return FAILURE;
    }
    
    PageInfo *pPage = pRecord->m_pPage;
    if (!pPage)
    {
        return FAILURE;
    }

    /// 把记录从链表删除并把表头清空
    DLL_REMOVE(&m_records, pRecord, m_field);
    (void)memset(pRecord, 0, sizeof(RecordHead));

    /// 本页面记录数减1
    if (pPage->m_dwRecCount) pPage->m_dwRecCount--;
    if (m_dwRecCount) m_dwRecCount--;

    /// 如果页面记录数为0，需要删除该页面
    if (!pPage->m_dwRecCount)
    {
        DLL_REMOVE(&m_pages, pPage, m_field);
        DCOP_Free(pPage);
        pPage = NULL;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CMemPage::GetRealRec
  描    述: 获取指定有效记录
  输    入: dwRecPos
            rdwMemPos
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CMemPage::PageInfo *CMemPage::GetRealRec(DWORD dwRecPos, DWORD &rdwMemPos)
{
    DWORD dwRecLoop = 0;

    PageInfo *pPageLoop = DLL_FIRST(&m_pages);
    while (pPageLoop)
    {
        PageInfo *pPage = pPageLoop;

        /// 有效记录不在本页内，继续遍历
        if ((dwRecPos - dwRecLoop) < pPage->m_dwRecCount)
        {
            dwRecLoop += pPage->m_dwRecCount;
            pPageLoop = DLL_NEXT_LOOP(&m_pages, pPageLoop, m_field);
            continue;
        }

        /// 遍历当前页面的所有记录
        BYTE *pbyRecStart = (BYTE *)(pPage + 1);
        DWORD dwRecIdx = dwRecLoop;
        DWORD dwMemPos = 0;
        while (dwMemPos < pPage->m_dwMemCount)
        {
            /// 记录头部中的页面指针不为空才是有效记录
            RecordHead *pRecord = (RecordHead *)(pbyRecStart + dwMemPos * (sizeof(RecordHead) + m_dwRecSize));
            if (pRecord->m_pPage)
            {
                /// 找到对应的记录号
                if (dwRecIdx == dwRecPos)
                {
                    rdwMemPos = dwMemPos;
                    return pPage;
                }

                dwRecIdx++;
            }

            dwMemPos++;
        }

        break;
    }

    return NULL;
}

/*******************************************************
  函 数 名: CMemPage::GetIdleRec
  描    述: 找到一个空闲记录
  输    入: rdwMemPos
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CMemPage::PageInfo *CMemPage::GetIdleRec(DWORD &rdwMemPos)
{
    PageInfo *pPageLoop = DLL_FIRST(&m_pages);
    while (pPageLoop)
    {
        /// 遍历所有的页面
        PageInfo *pPage = pPageLoop;
        pPageLoop = DLL_NEXT_LOOP(&m_pages, pPageLoop, m_field);

        /// 如果内存容量和有效记录相等，说明本页面没有空闲位置
        if (pPage->m_dwMemCount == pPage->m_dwRecCount)
        {
            continue;
        }

        /// 当前有空闲页，则遍历当前页面的所有记录
        BYTE *pbyRecStart = (BYTE *)(pPage + 1);
        DWORD dwMemPos = 0;
        while (dwMemPos < pPage->m_dwMemCount)
        {
            /// 记录头部中的页面指针为空才是无效记录
            RecordHead *pRecord = (RecordHead *)(pbyRecStart + dwMemPos * (sizeof(RecordHead) + m_dwRecSize));
            if (!(pRecord->m_pPage))
            {
                rdwMemPos = dwMemPos;
                return pPage;
            }

            dwMemPos++;
        }
    }

    return NULL;
}

/*******************************************************
  函 数 名: CMemPage::ClearPages
  描    述: 清除所有页面
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CMemPage::ClearPages()
{
    PageInfo *pPageLoop = DLL_FIRST(&m_pages);
    while (pPageLoop)
    {
        PageInfo *pPage = pPageLoop;
        pPageLoop = DLL_NEXT_LOOP(&m_pages, pPageLoop, m_field);

        DCOP_Free(pPage);
    }

    DLL_INIT(&m_pages);
    DLL_INIT(&m_records);
}

