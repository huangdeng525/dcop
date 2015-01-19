/// -------------------------------------------------
/// dstream.h : 动态流处理公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_STREAM_DSTREAM_H_
#define _TOOL_STREAM_DSTREAM_H_

#include "dcop.h"
#include "cpu/bytes.h"
#include "string/dstring.h"
#include "fs/file.h"


/////////////////////////////////////////////////////
///                动态流的3种用法
/// -------------------------------------------------
/// 1. 动态内存: 输入新值超过申请长度后，再安步长(SetMemStepLen)分配新的内存
/// 2. 静态内存: 按指定长度设置需要的内存(SetMemNeedLen)，可进行内存的预申请
/// 3. 字段记录: 可添加记录(AppendRecord)，第一次添加记录时会把当前的数据(即
///    m_dwRecordCount为0时的m_dwOffset的值)设置为单条记录的长度，后面操作过
///    程中会根据记录的长度来增长内存。
/////////////////////////////////////////////////////


#define DSTREAM_FIELD_NAME_LEN      16          // 字段名字长度
#define DSTREAM_DMEM_MAX_LEN        65535       // 内存最大长度
#define DSTREAM_DMEM_STEP_LEN       32          // 内存递增步长


/// -------------------------------------------------
/// Buffer参数
/// -------------------------------------------------
class CBufferPara
{
public:
    CBufferPara()
    {
        m_pBuf = 0;
        m_dwBufLen = 0;
        m_adwRul = 0;
        m_dwRuleCount = 0;
        m_bFree = false;
    }
    CBufferPara(void *pBuf, DWORD dwBufLen, 
                    const BYTES_CHANGE_RULE *adwRule = 0, 
                    DWORD dwRuleCount = 0)
    {
        m_pBuf = pBuf;
        m_dwBufLen = dwBufLen;
        m_adwRul = adwRule;
        m_dwRuleCount = dwRuleCount;
        m_bFree = false;
    }
    CBufferPara(const char *szStr, const char *cszFormat, DWORD dwSize)
    {
        m_pBuf = 0;
        m_dwBufLen = 0;
        m_adwRul = 0;
        m_dwRuleCount = 0;
        m_bFree = false;

        if (!szStr || !(*szStr)) return;

        DWORD dwStrLen = (DWORD)strlen(szStr);
        DWORD dwBufLen = (dwStrLen < dwSize)? 1 : (dwStrLen / dwSize);
        BYTE *pbyBuf = (BYTE *)DCOP_Malloc(dwBufLen);
        if (!pbyBuf) return;

        do
        {
            /// 长度不足一个单位，只输出一个字节的0
            if (dwStrLen < dwSize)
            {
                *pbyBuf = 0;
                break;
            }

            /// 长度足够，则循环进行转换
            DWORD dwOffset = 0;
            DWORD i = 0;
            while (dwOffset < dwStrLen)
            {
                int iRc = sscanf(szStr + dwOffset, cszFormat, &(pbyBuf[i++]));
                if (iRc <= 0) break;

                dwOffset += dwSize;
                if (i >= dwBufLen) break;
            }
        } while (0);

        m_bFree = true;
        m_pBuf = pbyBuf;
        m_dwBufLen = dwBufLen;
        
    }
    CBufferPara(const char *szFile, DWORD dwOffset = 0, DWORD dwCount = 0)
    {
        m_pBuf = 0;
        m_dwBufLen = 0;
        m_adwRul = 0;
        m_dwRuleCount = 0;
        m_bFree = false;

        if (!szFile) return;

        DWORD dwFileLen = DCOP_GetFileLen(szFile);
        if (!dwFileLen)
        {
            return;
        }

        if (dwOffset >= dwFileLen)
        {
            return;
        }

        DWORD dwReadLen = dwFileLen - dwOffset;
        if (dwCount && (dwReadLen > dwCount))
        {
            dwReadLen = dwCount;
        }

        BYTE *pbyBuf = (BYTE *)DCOP_Malloc(dwReadLen);
        if (!pbyBuf)
        {
            return;
        }

        DWORD dwRc = DCOP_RestoreFromFile(szFile, pbyBuf, dwReadLen, dwOffset);
        if (dwRc != SUCCESS)
        {
            DCOP_Free(pbyBuf);
            return;
        }

        m_bFree = true;
        m_pBuf = pbyBuf;
        m_dwBufLen = dwReadLen;

    }
    ~CBufferPara()
    {
        if (m_bFree) DCOP_Free(m_pBuf);
        m_bFree = false;
        m_pBuf = 0;
        m_dwBufLen = 0;
        m_adwRul = 0;
        m_dwRuleCount = 0;
    }

    void *GetBuf() const {return m_pBuf;}
    DWORD GetBufLen() const {return m_dwBufLen;}
    const BYTES_CHANGE_RULE *GetRule() const {return m_adwRul;}
    DWORD GetRuleCount() const {return m_dwRuleCount;}

private:
    void *m_pBuf;
    DWORD m_dwBufLen;
    const BYTES_CHANGE_RULE *m_adwRul;
    DWORD m_dwRuleCount;
    bool m_bFree;
};


/// -------------------------------------------------
/// 动态流
/// -------------------------------------------------
class CDStream
{
public:
    CDStream();
    CDStream(DWORD dwNeedMemLen);
    ~CDStream();

    /// 设置内存最大长度、内存递增步长、指定输入结束符、实时需要一块内存
    CDStream &SetMemMaxLen(DWORD dwMemMaxLen) {m_dwMemMaxLen = (dwMemMaxLen)? dwMemMaxLen : DSTREAM_DMEM_MAX_LEN; return *this;}
    CDStream &SetMemStepLen(DWORD dwMemStepLen) {m_dwMemStepLen = (dwMemStepLen)? dwMemStepLen : DSTREAM_DMEM_STEP_LEN; return *this;}
    CDStream &SetInputStrTerminal(bool bInputStrTerminal) {m_bInputStrTerminal = bInputStrTerminal; return *this;}
    DWORD SetNeedMemLen(DWORD dwNeedMemLen) {return (Dalloc(dwNeedMemLen))? SUCCESS : FAILURE;}

    /// 设置偏移信息、获取偏移信息、根据偏移获取缓存地址、获取长度、清除缓存流
    void Clear();
    DWORD Remove(DWORD dwOffset, DWORD dwLength);
    CDStream &SetOffset(DWORD dwOffset) {m_dwOffset = (dwOffset > m_dwDataLen)? m_dwDataLen : dwOffset; return *this;}
    DWORD GetOffSet() const {return m_dwOffset;}
    void *Buffer(DWORD dwOffset = 0) const {return (dwOffset > m_dwDataLen)? ((BYTE *)m_pBuffer + m_dwDataLen) : ((BYTE *)m_pBuffer + dwOffset);}
    DWORD Length() const {return m_dwDataLen;}
    CDStream &Reset() {SetOffset(0); return *this;}

    /// 添加记录(默认有1条记录了,所以第一次不用调用)、设置当前记录(从0开始)、获取记录总数
    DWORD AppendRecord();
    CDStream &SetCurRecord(DWORD dwCurIdx);
    DWORD GetRecordCount() const {return m_dwRecordCount;}

    /// 从文件中读取
    DWORD LoadFile(const char *szFile, DWORD dwOffset = 0, DWORD dwCount = 0);

    /// 输入
    CDStream &operator <<(BYTE byPara);
    CDStream &operator <<(WORD wPara);
    CDStream &operator <<(DWORD dwPara);
    CDStream &operator <<(const char *cpszPara);
    CDStream &operator <<(const CBufferPara &crbufPara);

    /// 输出
    CDStream &operator >>(BYTE &rbyPara);
    CDStream &operator >>(WORD &rwPara);
    CDStream &operator >>(DWORD &rdwPara);
    CDStream &operator >>(CDString &rstrPara);
    CDStream &operator >>(CBufferPara &rbufPara);

    /// 输出到动态数组
    void Output(CDArray &rArray);

    /// 转换为内存
    void *ToBuf();

    /// Dump
    void Dump(LOG_PRINT logPrint, LOG_PARA logPara = 0, DWORD recNo = 0);

private:
    /// 动态分配一段内存、释放动态内存
    bool Dalloc(DWORD dwNeedMemLen);
    void Dfree();

    /// 检测可写/可读内存
    bool CheckWriteMem(DWORD dwWriteMemLen);
    bool CheckReadMem(DWORD dwReadMemLen);

    /// 更新写后/读后位置
    void UpdateWritePos(DWORD dwWriteMemLen);
    void UpdateReadPos(DWORD dwReadMemLen);

private:
    void *m_pBuffer;                            // 内存地址
    DWORD m_dwMemLen;                           // 内存长度
    DWORD m_dwDataLen;                          // 数据长度
    DWORD m_dwOffset;                           // 操作偏移
    DWORD m_dwMemMaxLen;                        // 内存最大长度
    DWORD m_dwMemStepLen;                       // 内存递增步长
    DWORD m_dwRecordLen;                        // 记录长度
    DWORD m_dwRecordCount;                      // 记录个数
    DWORD m_dwRecordIndex;                      // 记录操作索引
    bool m_bInputStrTerminal;                   // 是否输入字符串结束符
};


#endif // #ifndef _TOOL_STREAM_DSTREAM_H_

