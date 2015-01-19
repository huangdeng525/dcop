/// -------------------------------------------------
/// tablestring.h : 表格字符串公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_STREAM_TABLESTREAM_H_
#define _TOOL_STREAM_TABLESTREAM_H_

#include "dcop.h"
#include "string/dstring.h"

/////////////////////////////////////////////////////
///                表格字符串说明
/// -------------------------------------------------
/// 主要适用于以表格形式显示的字符串
/////////////////////////////////////////////////////


/// -------------------------------------------------
/// 表格字符串
/// -------------------------------------------------
class CTableString
{
public:
    CTableString();
    CTableString(DWORD dwColCount, DWORD dwRowCount, const char *cszAlign = 0);
    ~CTableString();

    /// 设置列和行信息
    DWORD Init(DWORD dwColCount, DWORD dwRowCount);

    /// 设置对齐信息
    void Indent(const char *cpszAlign) {m_cpszAlign = cpszAlign;}

    /// 输入字符串
    CTableString &operator <<(const char *cpszStr);

    /// 获取指定行
    CDString *Line(DWORD dwIdx) const;

    /// 获取行数量
    DWORD Count() const {return m_rowCount;}

    /// 显示表格
    void Show(LOG_PRINT logPrint, LOG_PARA logPara, const char *cpszMargin = 0, const char *cpszTitle = 0) const;

private:
    void Clear();
    void ExpandCol(DWORD dwStrLen);
    DWORD GetColOffset();

private:
    DWORD *m_colNode;
    DWORD m_colCount;

    CDString *m_rowNode;
    DWORD m_rowCount;

    DWORD m_dwColPos;
    DWORD m_dwRowPos;

    const char *m_cpszAlign;
};


#endif // #ifndef _TOOL_STREAM_TABLESTREAM_H_

