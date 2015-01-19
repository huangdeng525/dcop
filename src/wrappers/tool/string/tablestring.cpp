/// -------------------------------------------------
/// tablestring.cpp : 表格字符串实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "string/tablestring.h"


/*******************************************************
  函 数 名: CTableString::CTableString
  描    述: CTableString构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CTableString::CTableString()
{
    m_colNode = 0;
    m_colCount = 0;

    m_rowNode = 0;
    m_rowCount = 0;

    m_dwColPos = 0;
    m_dwRowPos = 0;

    m_cpszAlign = 0;
}

/*******************************************************
  函 数 名: CTableString::CTableString
  描    述: CTableString构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CTableString::CTableString(DWORD dwColCount, DWORD dwRowCount, const char *cpszAlign)
{
    m_colNode = 0;
    m_colCount = 0;

    m_rowNode = 0;
    m_rowCount = 0;

    m_dwColPos = 0;
    m_dwRowPos = 0;

    m_cpszAlign = 0;

    (void)Init(dwColCount, dwRowCount);

    Indent(cpszAlign);
}

/*******************************************************
  函 数 名: CTableString::~CTableString
  描    述: CTableString构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CTableString::~CTableString()
{
    Clear();
}

/*******************************************************
  函 数 名: CTableString::Init
  描    述: 初始化
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTableString::Init(DWORD dwColCount, DWORD dwRowCount)
{
    if (!dwColCount || !dwRowCount)
    {
        return FAILURE;
    }

    DWORD *pColNode = (DWORD *)DCOP_Malloc(dwColCount * sizeof(DWORD));
    if (!pColNode)
    {
        return FAILURE;
    }

    (void)memset(pColNode, 0, dwColCount * sizeof(DWORD));

    CDString *pRowNode = new CDString[dwRowCount];
    if (!pRowNode)
    {
        DCOP_Free(pColNode);
        return FAILURE;
    }

    Clear();

    m_colNode = pColNode;
    m_rowNode = pRowNode;
    m_colCount = dwColCount;
    m_rowCount = dwRowCount;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CTableString::operator <<
  描    述: 输入字符串
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CTableString &CTableString::operator <<(const char *cpszStr)
{
    if ((m_dwColPos >= m_colCount) || 
        (m_dwRowPos >= m_rowCount))
    {
        return *this;
    }

    /// 获取列宽度
    DWORD dwStrLen = (DWORD)strlen(cpszStr) + 2;
    DWORD dwBlankLen = 2;
    if (!m_colNode[m_dwColPos])
    {
        m_colNode[m_dwColPos] = dwStrLen;
    }
    else if (dwStrLen > m_colNode[m_dwColPos])
    {
        ExpandCol(dwStrLen);
    }
    else
    {
        dwBlankLen = m_colNode[m_dwColPos] - dwStrLen + 2;
    }

    /// 起始对齐
    if (!m_dwColPos && m_cpszAlign)
    {
        m_rowNode[m_dwRowPos] << m_cpszAlign;
    }

    /// 正常信息
    m_rowNode[m_dwRowPos] << cpszStr;

    /// 右侧空格
    if (dwBlankLen)
    {
        char *pszBlank = (char *)DCOP_Malloc(dwBlankLen + 1);
        if (!pszBlank)
        {
            return *this;
        }

        (void)memset(pszBlank, ' ', dwBlankLen);
        pszBlank[dwBlankLen] = '\0';

        m_rowNode[m_dwRowPos] << pszBlank;
        DCOP_Free(pszBlank);
    }

    /// 尾部换行
    m_dwColPos++;
    if (m_dwColPos >= m_colCount)
    {
        m_rowNode[m_dwRowPos] << "\r\n";
        m_dwRowPos++;
        m_dwColPos = 0;
    }

    return *this;
}

/*******************************************************
  函 数 名: CTableString::Line
  描    述: 获取指定行
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDString *CTableString::Line(DWORD dwIdx) const
{
    if (!m_rowNode || !m_rowCount)
    {
        return NULL;
    }

    if (dwIdx >= m_rowCount)
    {
        return NULL;
    }

    return &(m_rowNode[dwIdx]);
}

/*******************************************************
  函 数 名: CTableString::Clear
  描    述: 清除
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTableString::Clear()
{
    if (m_colNode)
    {
        DCOP_Free(m_colNode);
        m_colNode = 0;
    }

    if (m_rowNode)
    {
        delete [] m_rowNode;
        m_rowNode = 0;
    }

    m_colCount = 0;
    m_rowCount = 0;
}

/*******************************************************
  函 数 名: CTableString::ExpandCol
  描    述: 扩展列
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTableString::ExpandCol(DWORD dwStrLen)
{
    if (!m_colNode || !m_colCount || 
        !m_rowNode || !m_rowCount || 
        (m_dwColPos >= m_colCount) ||
        (m_dwRowPos >= m_rowCount))
    {
        return;
    }

    if (dwStrLen <= m_colNode[m_dwColPos])
    {
        return;
    }

    DWORD dwBlankLen = dwStrLen - m_colNode[m_dwColPos];
    char *pszBlank = (char *)DCOP_Malloc(dwBlankLen + 1);
    if (!pszBlank)
    {
        return;
    }

    (void)memset(pszBlank, ' ', dwBlankLen);
    pszBlank[dwBlankLen] = '\0';

    DWORD dwOffset = GetColOffset();
    for (DWORD i = 0; i < m_dwRowPos; ++i)
    {
        m_rowNode[i].Insert(dwOffset, pszBlank);
    }

    m_colNode[m_dwColPos] = dwStrLen;
    DCOP_Free(pszBlank);
}

/*******************************************************
  函 数 名: CTableString::GetColOffset
  描    述: 获取列偏移
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTableString::GetColOffset()
{
    if (!m_colNode || !m_colCount) return 0;

    DWORD dwOffset = (m_cpszAlign)? (DWORD)strlen(m_cpszAlign) : 0;
    DWORD dwCurCol = (m_dwColPos >= m_colCount)? (m_colCount - 1) : m_dwColPos;
    for (DWORD i = 0; i <= dwCurCol; ++i)
    {
        dwOffset += m_colNode[i];
    }

    return dwOffset;
}

/*******************************************************
  函 数 名: CTableString::Show
  描    述: 显示表格
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTableString::Show(LOG_PRINT logPrint, LOG_PARA logPara, const char *cpszMargin, const char *cpszTitle) const
{
    if (!logPrint || !m_rowNode) return;

    char *pszMargin = 0;
    char *pszTitle = 0;
    DWORD dwFirstLineLen = m_rowNode[0].Length();
    if (dwFirstLineLen && (cpszMargin || cpszTitle))
    {
        if (cpszMargin)
        {
            pszMargin = (char *)DCOP_Malloc(dwFirstLineLen + 3);
            if (pszMargin)
            {
                (void)memset(pszMargin, *cpszMargin, dwFirstLineLen);
                pszMargin[dwFirstLineLen] = '\r';
                pszMargin[dwFirstLineLen + 1] = '\n';
                pszMargin[dwFirstLineLen + 2] = '\0';
            }
        }

        if (cpszTitle)
        {
            pszTitle = (char *)DCOP_Malloc(dwFirstLineLen + 3);
            if (pszTitle)
            {
                (void)memset(pszTitle, *cpszTitle, dwFirstLineLen);
                pszTitle[dwFirstLineLen] = '\r';
                pszTitle[dwFirstLineLen + 1] = '\n';
                pszTitle[dwFirstLineLen + 2] = '\0';
            }
        }
    }

    /// 显示上边边线
    if (pszMargin) logPrint(pszMargin, logPara);

    /// 显示各行内容
    for (DWORD i = 0; i < Count(); ++i)
    {
        /// 显示内容
        const char *cpszTmp = m_rowNode[i];
        if (!cpszTmp) continue;

        /// 显示标题
        logPrint(cpszTmp, logPara);
        if (!i && pszTitle) logPrint(pszTitle, logPara);
    }

    /// 显示下边边线
    if (pszMargin) logPrint(pszMargin, logPara);

    if (pszMargin) DCOP_Free(pszMargin);
    if (pszTitle) DCOP_Free(pszTitle);
}

