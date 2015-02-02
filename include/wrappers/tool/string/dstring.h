/// -------------------------------------------------
/// dstring.h : 动态字符串公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _STRING_DSTRING_H_
#define _STRING_DSTRING_H_

#include "dcop.h"
#include "array/darray.h"


/////////////////////////////////////////////////////
///                动态字符串说明
/// -------------------------------------------------
/// 动态字符串主要用于动态存储和释放(char *)类型字符串
/// 的方法封装的类，为了使用简洁，请务必:
///     * 保持sizeof(CDString)和sizeof(char *)完全一致
///     * 即CDString的成员除char *外，请不添加其它成员
/// -------------------------------------------------
/// 字符串数组 : 和CDArray配合成为字符串数组，CDArray的
/// 每个节点空间，既是CDString的空间，又是(char *)指针
/// 的空间。
/////////////////////////////////////////////////////


/// -------------------------------------------------
/// Buffer字符串
/// -------------------------------------------------
class CBufferString
{
public:
    CBufferString(void *pBuf, DWORD dwBufLen, const char *cszFormat = "%02x", DWORD dwSize = 2)
    {
        m_szStr = NULL;
        if (!pBuf || !dwBufLen) return;

        DWORD dwMemLen = dwBufLen * dwSize + 1;
        m_szStr = (char *)DCOP_Malloc(dwMemLen);
        if (!m_szStr) return;

        BYTE *pbyBuf = (BYTE *)pBuf;
        DWORD dwOffset = 0;
        for (DWORD i = 0; i < dwBufLen; ++i)
        {
            dwOffset += (DWORD)snprintf(m_szStr + dwOffset, 
                        dwMemLen - dwOffset, 
                        cszFormat, 
                        pbyBuf[i]);
        }
        m_szStr[dwMemLen - 1] = '\0';
    }
    ~CBufferString() {if (m_szStr) DCOP_Free(m_szStr); m_szStr = NULL;}

    operator const char *() { return m_szStr;}

private:
    CBufferString() {m_szStr = NULL;}
    char *m_szStr;
};


/// -------------------------------------------------
/// 动态字符串
/// -------------------------------------------------
class CDString
{
public:
    /// 字符串尾部位置
    static const DWORD TAIL = (DWORD)(-1);

    /// 整个字符串长度
    static const DWORD WHOLE = (DWORD)(-1);

public:
    CDString();
    CDString(const char *cpszStr, DWORD dwLen = 0);
    CDString(const CDString &rThis);
    ~CDString();

    /// 获取长度
    DWORD Length();

    /// 清空
    void Clear();

    /// 分割字符串
    void Split(const char *chrList, CDArray &strList, bool bNeedSplitChar = true);

    /// 裁剪
    void Trim(const char *chrList);
    void TrimLeft(const char *chrList, DWORD dwStartPos = 0);
    void TrimRight(const char *chrList);

    /// 插入
    void Insert(DWORD dwStartPos, const char *cpszStr);

    /// 移除
    void Remove(DWORD dwStartPos, DWORD dwLen = 0);

    /// 反向
    void Reverse();

    /// 查找
    DWORD Find(const char *strSub);

    /// 从文件中读取
    DWORD LoadFile(const char *szFile, DWORD dwOffset = 0, DWORD dwCount = 0);

    /// 设置
    void Set(const char *cpszStr, DWORD dwLen = 0)
    {
        if (m_pBuffer) DCOP_Free(m_pBuffer);
        m_pBuffer = Copy(cpszStr, dwLen);
    }

    /// 获取
    char *Get()
    {
        return m_pBuffer;
    }

    /// 获取
    char Get(DWORD dwIdx)
    {
        if (!m_pBuffer) return '\0';
        DWORD dwLen = (DWORD)strlen(m_pBuffer);
        if (!dwLen) return '\0';
        if (dwIdx >= dwLen) dwIdx = dwLen - 1;
        return m_pBuffer[dwIdx];
    }

    /// 赋值操作符
    CDString& operator = (const char *cpszStr)
    {
        Set(cpszStr);
        return *this;
    }

    CDString& operator = (const CDString &rThis)
    {
        *this = (const char *)rThis;
        return *this;
    }

    /// 转换操作符
    operator const char *() const
    {
        return m_pBuffer;
    }

    /// 等号操作符
    bool operator == (const char *cpszStr)
    {
        if (!cpszStr || !m_pBuffer)
        {
            return (!cpszStr && !m_pBuffer)? true : false;
        }

        return (!strcmp(cpszStr, m_pBuffer))? true : false;
    }

    /// 输入操作符
    CDString &operator <<(const char *cpszStr)
    {
        /// 插入在尾部
        Insert(TAIL, cpszStr);
        return *this;
    }

    /// 打印到字符串的回调
    static void Print(const char *info, void *para)
    {
        CDString *pString = (CDString *)para;
        if (!pString) return;
        pString->Insert(TAIL, info);
    }

private:
    char *Copy(const char *cpszStr, DWORD dwLen = 0);
    char *Located(char *pszStr, const char *cpszChr);
    static void StrListFree(DWORD index, void *pNode);
    static void StrListFreeHead(DWORD index, void *pNode);

private:
    char *m_pBuffer;
};

#endif // #ifndef _STRING_DSTRING_H_

