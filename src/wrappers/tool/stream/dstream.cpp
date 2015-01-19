/// -------------------------------------------------
/// dstream.h : 动态流处理公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "stream/dstream.h"
#include "os.h"
#include <memory.h>
#include <ctype.h>


/*******************************************************
  函 数 名: CDStream::CDStream
  描    述: CDStream构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream::CDStream()
{
    m_pBuffer = 0;
    m_dwMemLen = 0;
    m_dwDataLen = 0;
    m_dwOffset = 0;

    m_dwMemMaxLen = DSTREAM_DMEM_MAX_LEN;
    m_dwMemStepLen = DSTREAM_DMEM_STEP_LEN;

    m_dwRecordLen = 0;
    m_dwRecordCount = 0;
    m_dwRecordIndex = 0;

    m_bInputStrTerminal = false;
}

/*******************************************************
  函 数 名: CDStream::CDStream
  描    述: CDStream构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream::CDStream(DWORD dwNeedMemLen)
{
    m_pBuffer = 0;
    m_dwMemLen = 0;
    m_dwDataLen = 0;
    m_dwOffset = 0;

    m_dwMemMaxLen = DSTREAM_DMEM_MAX_LEN;
    m_dwMemStepLen = DSTREAM_DMEM_STEP_LEN;

    m_dwRecordLen = 0;
    m_dwRecordCount = 0;
    m_dwRecordIndex = 0;

    m_bInputStrTerminal = false;

    if (dwNeedMemLen) (void)SetNeedMemLen(dwNeedMemLen);
}

/*******************************************************
  函 数 名: CDStream::~CDStream
  描    述: CDStream析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream::~CDStream()
{
    Dfree();
}

/*******************************************************
  函 数 名: CDStream::Clear
  描    述: 清除缓存流
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDStream::Clear()
{
    Dfree();

    m_dwMemMaxLen = DSTREAM_DMEM_MAX_LEN;

    m_dwRecordLen = 0;
    m_dwRecordCount = 0;
    m_dwRecordIndex = 0;
}

/*******************************************************
  函 数 名: CDStream::Remove
  描    述: 在指定位置删除指定长度的字符串
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDStream::Remove(DWORD dwOffset, DWORD dwLength)
{
    if (!m_pBuffer || !m_dwDataLen)
    {
        return FAILURE;
    }

    /// 如果偏移超过数据长度，则更新为最后一个字节
    if (dwOffset >= m_dwDataLen)
    {
        dwOffset = m_dwDataLen - 1;
    }

    /// 如果偏移+长度超过数据长度，则更新为最后的数据长度
    if ((dwOffset + dwLength) > m_dwDataLen)
    {
        dwLength = m_dwDataLen - dwOffset;
    }

    /// 获取尾部遗留的数据长度，往前进行移动
    DWORD dwLeftLen = m_dwDataLen - (dwOffset + dwLength);
    if (dwLeftLen)
    {
        (void)memmove((BYTE *)m_pBuffer + dwOffset, 
                        (BYTE *)m_pBuffer + dwOffset + dwLength, 
                        dwLeftLen);
        (void)memset((BYTE *)m_pBuffer + dwOffset + dwLeftLen, 
                        0, 
                        dwLength);
    }

    m_dwDataLen = dwOffset + dwLeftLen;
    if (m_dwOffset > m_dwDataLen)
    {
        m_dwOffset = m_dwDataLen;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDStream::AppendRecord
  描    述: 添加记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDStream::AppendRecord()
{
    if (!m_dwRecordCount)
    {
        return FAILURE;
    }

    /// 第一次添加记录时，把当前偏移设置为记录长度
    if (m_dwRecordCount == 1)
    {
        m_dwRecordLen = m_dwOffset;
    }

    if (!m_dwRecordLen)
    {
        return FAILURE;
    }

    /// 设置新的记录的内存长度，并设置最大不能超过新的记录长度
    DWORD dwRc = SetNeedMemLen(m_dwRecordLen);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    (void)SetMemMaxLen((m_dwRecordCount + 1) * m_dwRecordLen);

    m_dwRecordIndex = m_dwRecordCount;
    m_dwOffset = m_dwRecordIndex * m_dwRecordLen;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDStream::SetCurRecord
  描    述: 设置记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream &CDStream::SetCurRecord(DWORD dwCurIdx)
{
    if (dwCurIdx >= m_dwRecordCount)
    {
        dwCurIdx = m_dwRecordCount - 1;
    }

    m_dwRecordIndex = dwCurIdx;
    m_dwOffset = m_dwRecordIndex * m_dwRecordLen;

    return *this;
}

/*******************************************************
  函 数 名: CDStream::Dalloc
  描    述: 申请动态内存
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CDStream::Dalloc(DWORD dwNeedMemLen)
{
    /// 已有内存且长度满足需要，直接返回成功
    if ( m_pBuffer && ((m_dwOffset + dwNeedMemLen) <= m_dwMemLen) )
    {
        return true;
    }

    /// 申请长度取递增步长和需要的最大值
    DWORD dwTmpCount = m_dwMemLen + m_dwMemStepLen;
    if (dwTmpCount < (m_dwOffset + dwNeedMemLen))
    {
        dwTmpCount = m_dwOffset + dwNeedMemLen;
    }

    /// 不能超过设定的最大长度
    if (dwTmpCount > m_dwMemMaxLen)
    {
        if (m_dwMemMaxLen < (m_dwOffset + dwNeedMemLen))
        {
            return false;
        }

        dwTmpCount = m_dwMemMaxLen;
    }

    /// 无法申请的长度，返回失败
    if (!dwTmpCount)
    {
        return false;
    }

    /// 申请内存，失败返回
    int *pTmp = (int *)DCOP_Malloc(dwTmpCount);
    if (!pTmp)
    {
        return false;
    }

    /// 内存清零
    (void)memset(pTmp, 0, dwTmpCount);

    /// 拷贝已经存在的内存到前面
    if (m_pBuffer)
    {
        (void)memcpy(pTmp, m_pBuffer, m_dwOffset);
        DCOP_Free(m_pBuffer);
    }

    /// 输出新的内存和长度
    m_pBuffer = pTmp;
    m_dwMemLen = dwTmpCount;

    return true;
}

/*******************************************************
  函 数 名: CDStream::Dfree
  描    述: 释放动态内存
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDStream::Dfree()
{
    if (m_pBuffer)
    {
        free(m_pBuffer);
        m_pBuffer = 0;
    }

    m_dwMemLen = 0;
    m_dwDataLen = 0;
    m_dwOffset = 0;
}

/*******************************************************
  函 数 名: CDStream::CheckWriteMem
  描    述: 检查写内存长度
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CDStream::CheckWriteMem(DWORD dwWriteMemLen)
{
    /// 直接使用动态内存，不足会自动申请
    return Dalloc(dwWriteMemLen);
}

/*******************************************************
  函 数 名: CDStream::CheckReadMem
  描    述: 检查读内存长度
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CDStream::CheckReadMem(DWORD dwReadMemLen)
{
    if (!m_pBuffer)
    {
        return false;
    }

    /// 读长度不能超过数据长度
    if ((dwReadMemLen + m_dwOffset) > m_dwDataLen)
    {
        return false;
    }

    return true;
}

/*******************************************************
  函 数 名: CDStream::UpdateWritePos
  描    述: 更新写后位置
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDStream::UpdateWritePos(DWORD dwWriteMemLen)
{
    /// 更新写后偏移
    m_dwOffset += dwWriteMemLen;

    /// 如果写后超过数据长度，更新数据长度
    if (m_dwOffset > m_dwDataLen)
    {
        m_dwDataLen = m_dwOffset;
    }

    /// 如果是新的记录开始，记录数量加一
    if (m_dwRecordIndex == m_dwRecordCount)
    {
        m_dwRecordCount++;
    }
}

/*******************************************************
  函 数 名: CDStream::UpdateReadPos
  描    述: 更新读后位置
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDStream::UpdateReadPos(DWORD dwReadMemLen)
{
    /// 更新读后偏移
    m_dwOffset += dwReadMemLen;
}


/*******************************************************
  函 数 名: CDStream::LoadFile
  描    述: 从文件中读取
  输    入: szFile      - 文件名
            dwOffset    - 偏移
            dwLen       - 指定长度，为0时指偏移后的整个文件内容
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDStream::LoadFile(const char *szFile, DWORD dwOffset, DWORD dwCount)
{
    if (!szFile) return FAILURE;

    DWORD dwFileLen = DCOP_GetFileLen(szFile);
    if (!dwFileLen)
    {
        return FAILURE;
    }

    if (dwOffset >= dwFileLen)
    {
        return FAILURE;
    }

    DWORD dwReadLen = dwFileLen - dwOffset;
    if (dwCount && (dwReadLen > dwCount))
    {
        dwReadLen = dwCount;
    }

    BYTE *pbyBuf = (BYTE *)DCOP_Malloc(dwReadLen);
    if (!pbyBuf)
    {
        return FAILURE;
    }

    DWORD dwRc = DCOP_RestoreFromFile(szFile, pbyBuf, dwReadLen, dwOffset);
    if (dwRc != SUCCESS)
    {
        DCOP_Free(pbyBuf);
        return dwRc;
    }

    if (m_pBuffer) DCOP_Free(m_pBuffer);
    m_pBuffer = pbyBuf;
    m_dwMemLen = dwReadLen;
    m_dwDataLen = dwReadLen;
    m_dwOffset = dwReadLen;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDStream::operator <<
  描    述: 输入BYTE
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream &CDStream::operator <<(BYTE byPara)
{
    if (!CheckWriteMem(sizeof(BYTE)))
    {
        return *this;
    }

    *((BYTE *)m_pBuffer + m_dwOffset) = byPara;

    UpdateWritePos(sizeof(BYTE));
    return *this;
}

/*******************************************************
  函 数 名: CDStream::operator <<
  描    述: 输入WORD
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream &CDStream::operator <<(WORD wPara)
{
    if (!CheckWriteMem(sizeof(WORD)))
    {
        return *this;
    }

    Bytes_SetWord((BYTE *)m_pBuffer + m_dwOffset, wPara);

    UpdateWritePos(sizeof(WORD));
    return *this;
}

/*******************************************************
  函 数 名: CDStream::operator <<
  描    述: 输入DWORD
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream &CDStream::operator <<(DWORD dwPara)
{
    if (!CheckWriteMem(sizeof(DWORD)))
    {
        return *this;
    }

    Bytes_SetDword((BYTE *)m_pBuffer + m_dwOffset, dwPara);

    UpdateWritePos(sizeof(DWORD));
    return *this;
}

/*******************************************************
  函 数 名: CDStream::operator <<
  描    述: 输入字符串
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream &CDStream::operator <<(const char *cpszPara)
{
    if (!cpszPara) return *this;

    DWORD dwStrLen = (DWORD)strlen(cpszPara);
    if (!dwStrLen)
    {
        return *this;
    }

    /// 如果指定了包含结束符，则加上结束符长度
    if (m_bInputStrTerminal)
    {
        dwStrLen++;
    }

    if (!CheckWriteMem(dwStrLen))
    {
        return *this;
    }

    (void)memcpy((char *)m_pBuffer + m_dwOffset, cpszPara, dwStrLen);

    UpdateWritePos(dwStrLen);
    return *this;
}

/*******************************************************
  函 数 名: CDStream::operator <<
  描    述: 输入Buffer
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream &CDStream::operator <<(const CBufferPara &crbufPara)
{
    void *pBuf = crbufPara.GetBuf();
    DWORD dwBufLen = crbufPara.GetBufLen();
    if (!pBuf || !dwBufLen)
    {
        return *this;
    }

    if (!CheckWriteMem(dwBufLen))
    {
        return *this;
    }

    (void)memcpy((BYTE *)m_pBuffer + m_dwOffset, pBuf, dwBufLen);
    Bytes_ChangeOrderByRule(crbufPara.GetRule(), 
                        crbufPara.GetRuleCount(), 
                        (BYTE *)m_pBuffer + m_dwOffset, 
                        dwBufLen);

    UpdateWritePos(dwBufLen);
    return *this;
}

/*******************************************************
  函 数 名: CDStream::operator >>
  描    述: 输出BYTE
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream &CDStream::operator >>(BYTE &rbyPara)
{
    if (!CheckReadMem(sizeof(BYTE)))
    {
        return *this;
    }

    rbyPara = *((BYTE *)m_pBuffer + m_dwOffset);

    UpdateReadPos(sizeof(BYTE));
    return *this;
}

/*******************************************************
  函 数 名: CDStream::operator >>
  描    述: 输出WORD
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream &CDStream::operator >>(WORD &rwPara)
{
    if (!CheckReadMem(sizeof(WORD)))
    {
        return *this;
    }

    rwPara = Bytes_GetWord((BYTE *)m_pBuffer + m_dwOffset);

    UpdateReadPos(sizeof(WORD));
    return *this;
}

/*******************************************************
  函 数 名: CDStream::operator >>
  描    述: 输出DWORD
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream &CDStream::operator >>(DWORD &rdwPara)
{
    if (!CheckReadMem(sizeof(DWORD)))
    {
        return *this;
    }

    rdwPara = Bytes_GetDword((BYTE *)m_pBuffer + m_dwOffset);

    UpdateReadPos(sizeof(DWORD));
    return *this;
}

/*******************************************************
  函 数 名: CDStream::operator >>
  描    述: 输出字符串
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream &CDStream::operator >>(CDString &rstrPara)
{
    rstrPara.Clear();
    if (!CheckReadMem(sizeof(char)))
    {
        return *this;
    }

    rstrPara.Set((char *)m_pBuffer + m_dwOffset, m_dwDataLen - m_dwOffset);

    UpdateReadPos(rstrPara.Length() + 1);
    return *this;
}

/*******************************************************
  函 数 名: CDStream::operator >>
  描    述: 输出Buffer
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDStream &CDStream::operator >>(CBufferPara &rbufPara)
{
    void *pBuf = rbufPara.GetBuf();
    DWORD dwBufLen = rbufPara.GetBufLen();
    if (!pBuf || !dwBufLen)
    {
        return *this;
    }

    if (!CheckReadMem(dwBufLen))
    {
        return *this;
    }

    (void)memcpy(pBuf, (BYTE *)m_pBuffer + m_dwOffset, dwBufLen);
    Bytes_ChangeOrderByRule(rbufPara.GetRule(), 
                        rbufPara.GetRuleCount(), 
                        pBuf, 
                        dwBufLen);

    UpdateReadPos(dwBufLen);
    return *this;
}

/*******************************************************
  函 数 名: CDStream::Output
  描    述: 输出到动态数组
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDStream::Output(CDArray &rArray)
{
    rArray.Clear();
    rArray.SetNodeSize(m_dwRecordLen);

    for (DWORD i = 0; i < m_dwRecordCount; ++i)
    {
        (void)rArray.Append((BYTE *)m_pBuffer + i * m_dwRecordLen);
    }
}

/*******************************************************
  函 数 名: CDStream::ToBuf
  描    述: 转换为内存
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void *CDStream::ToBuf()
{
    void *pBuf = Buffer();
    DWORD dwLen = Length();

    if (!pBuf || !dwLen)
    {
        return NULL;
    }

    void *pTmp = DCOP_Malloc(dwLen);
    if (!pTmp)
    {
        return NULL;
    }

    (void)memcpy(pTmp, pBuf, dwLen);
    return pTmp;
}

/*******************************************************
  函 数 名: CDStream::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDStream::Dump(LOG_PRINT logPrint, LOG_PARA logPara, DWORD recNo)
{
    if (!logPrint) return;

    logPrint("-----------------------------------------------\r\n", logPara);
    logPrint("Stream Dump: \r\n", logPara);

    for (DWORD i = 0; i < m_dwRecordCount; ++i)
    {
        if (recNo && (recNo != (i + 1))) continue;

        PrintBuffer(STR_FORMAT("<Record No:%d> \r\n", i + 1), 
                        (BYTE *)m_pBuffer + i * m_dwRecordLen, m_dwRecordLen, 
                        logPrint, logPara);
    }

    logPrint(STR_FORMAT("[Record Count: %d] \r\n", m_dwRecordCount), logPara);
    logPrint("-----------------------------------------------\r\n", logPara);
}

