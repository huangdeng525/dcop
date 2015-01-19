/// -------------------------------------------------
/// ObjData_index.cpp : 记录索引实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjData_index.h"
#include "ObjData_main.h"


/*******************************************************
  函 数 名: CRecIdx::CKey::CKey
  描    述: CKey构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::CKey::CKey() : m_pKey(NULL), m_dwLen(0)
{
}

/*******************************************************
  函 数 名: CRecIdx::CKey::CKey
  描    述: CKey构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::CKey::CKey(DWORD dwLen) : m_pKey(NULL), m_dwLen(0)
{
    if (!dwLen) return;

    void *pKey = DCOP_Malloc(dwLen);
    if (!pKey) return;

    (void)memset(pKey, 0, dwLen);

    m_pKey = pKey;
    m_dwLen = dwLen;
}

/*******************************************************
  函 数 名: CRecIdx::CKey::CKey
  描    述: CKey构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::CKey::CKey(const void *cpKey, DWORD dwLen) : m_pKey(NULL), m_dwLen(0)
{
    if (!dwLen) return;

    void *pKey = DCOP_Malloc(dwLen);
    if (!pKey) return;

    if (!cpKey)
        (void)memset(pKey, 0, dwLen);
    else
        (void)memcpy(pKey, cpKey, dwLen);

    m_pKey = pKey;
    m_dwLen = dwLen;
}

/*******************************************************
  函 数 名: CRecIdx::CKey::CKey
  描    述: CKey构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::CKey::CKey(const CKey &rKey) : m_pKey(NULL), m_dwLen(0)
{
    *this = rKey;
}

/*******************************************************
  函 数 名: CRecIdx::CKey::~CKey
  描    述: CKey析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::CKey::~CKey()
{
    if (m_pKey)
    {
        DCOP_Free(m_pKey);
        m_pKey = NULL;
    }

    m_dwLen = 0;
}

/*******************************************************
  函 数 名: operator=
  描    述: 赋值操作符
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::CKey& CRecIdx::CKey::operator=(const void *cpKey)
{
    if (!m_dwLen)
    {
        return *this;
    }

    if (!m_pKey)
    {
        m_pKey = DCOP_Malloc(m_dwLen);
        if (!m_pKey)
        {
            return *this;
        }
    }

    (void)memcpy(m_pKey, cpKey, m_dwLen);

    return *this;
}

/*******************************************************
  函 数 名: operator=
  描    述: 赋值操作符
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::CKey& CRecIdx::CKey::operator=(const CRecIdx::CKey &rKey)
{
    DWORD dwLen = rKey.GetLen();
    if (!dwLen)
    {
        return *this;
    }

    void *pKey = DCOP_Malloc(dwLen);
    if (!pKey)
    {
        return *this;
    }

    void *pBuf = rKey.GetKey();
    if (!pBuf)
        (void)memset(pKey, 0, dwLen);
    else
        (void)memcpy(pKey, pBuf, dwLen);

    if (m_pKey) DCOP_Free(m_pKey);

    m_pKey = pKey;
    m_dwLen = dwLen;

    return *this;
}

/*******************************************************
  函 数 名: operator<
  描    述: 比较操作符
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CRecIdx::CKey::operator<(const CRecIdx::CKey &rKey) const
{
    if (!GetKey() || !rKey.GetKey())
    {
        return (GetKey() < rKey.GetKey())? true : false;
    }

    if (!GetLen() || !rKey.GetLen() || (GetLen() != rKey.GetLen()))
    {
        return (GetLen() < rKey.GetLen())? true : false;
    }

    if (memcmp(GetKey(), rKey.GetKey(), GetLen()) < 0)
    {
        return true;
    }

    return false;
}

/*******************************************************
  函 数 名: operator==
  描    述: 比较操作符
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CRecIdx::CKey::operator==(const CKey &rKey) const
{
    if (!GetKey() || !rKey.GetKey())
    {
        return (GetKey() == rKey.GetKey())? true : false;
    }

    if (!GetLen() || !rKey.GetLen())
    {
        return (GetLen() == rKey.GetLen())? true : false;
    }

    if (GetLen() != rKey.GetLen())
    {
        return false;
    }

    if (!memcmp(GetKey(), rKey.GetKey(), GetLen()))
    {
        return true;
    }

    return false;
}

/*******************************************************
  函 数 名: CRecIdx::CIdx::CIdx
  描    述: CIdx构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::CIdx::CIdx()
{
    m_idx.m_ptr = NULL;
}

/*******************************************************
  函 数 名: CRecIdx::CIdx::CIdx
  描    述: CIdx构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::CIdx::CIdx(void *ptr)
{
    m_idx.m_ptr = ptr;
}

/*******************************************************
  函 数 名: CRecIdx::CIdx::CIdx
  描    述: CIdx构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::CIdx::CIdx(DWORD pos)
{
    m_idx.m_pos = pos;
}

/*******************************************************
  函 数 名: CRecIdx::CIdx::~CIdx
  描    述: CIdx析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::CIdx::~CIdx()
{
    m_idx.m_ptr = NULL;
}

/*******************************************************
  函 数 名: CRecIdx::CRecIdx
  描    述: CRecIdx构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::CRecIdx()
{
}

/*******************************************************
  函 数 名: CRecIdx::~CRecIdx
  描    述: CRecIdx析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::~CRecIdx()
{
}

/*******************************************************
  函 数 名: CRecIdx::AddKey
  描    述: 添加关键字
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CRecIdx::AddKey(DCOP_PARA_NODE *pPara, DWORD dwParaCount)
{
    if (!pPara || !dwParaCount) return FAILURE;

    CKey fieldKey(pPara, sizeof(DCOP_PARA_NODE) * dwParaCount);
    ClearType(fieldKey);

    IT_FLDIDX it_field = m_idx.find(fieldKey);
    if (it_field != m_idx.end())
    {
        return FAILURE;
    }

    MAP_RECIDX recordMap;
    (void)m_idx.insert(m_idx.end(), MAP_FLDIDX::value_type(fieldKey, recordMap));

    return SUCCESS;
}

/*******************************************************
  函 数 名: CRecIdx::DelKey
  描    述: 删除关键字
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CRecIdx::DelKey(DCOP_PARA_NODE *pPara, DWORD dwParaCount)
{
    if (!pPara || !dwParaCount) return FAILURE;

    CKey fieldKey(pPara, sizeof(DCOP_PARA_NODE) * dwParaCount);
    ClearType(fieldKey);

    IT_FLDIDX it_field = m_idx.find(fieldKey);
    if (it_field == m_idx.end())
    {
        return FAILURE;
    }

    (void)m_idx.erase(it_field);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CRecIdx::OnAddRec
  描    述: 添加整条记录触发的操作
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CRecIdx::OnAddRec(IDataHandle::Field *pFields, BYTE *pbyRec, CIdx recordIdx)
{
    /// 没有索引，直接返回成功
    DWORD dwIdxCount = (DWORD)m_idx.size();
    if (!dwIdxCount)
    {
        return SUCCESS;
    }

    /// 准备复制记录索引的空间
    CKey *pRecordKey = new CKey[dwIdxCount];
    if (!pRecordKey)
    {
        return FAILURE;
    }

    /// 遍历索引，查找索引是否重复
    DWORD dwIdxPos = 0;
    for (IT_FLDIDX it_field = m_idx.begin();
        it_field != m_idx.end(); ++it_field)
    {
        /// 获取参数个数
        CKey fieldKey((*it_field).first);
        DCOP_PARA_NODE *pPara = (DCOP_PARA_NODE *)fieldKey.GetKey();
        DWORD dwParaCount = fieldKey.GetLen() / sizeof(DCOP_PARA_NODE);
        if (!pPara || !dwParaCount)
        {
            delete [] pRecordKey;
            return FAILURE;
        }

        /// 获取数据长度
        DWORD dwDataLen = 0;
        for (DWORD i = 0; i < dwParaCount; ++i)
        {
            dwDataLen += pFields[pPara[i].m_paraID - 1].m_fieldSize;
        }
        if (!dwDataLen)
        {
            delete [] pRecordKey;
            return FAILURE;
        }

        /// 生成数据索引
        BYTE *pData = (BYTE *)DCOP_Malloc(dwDataLen);
        if (!pData)
        {
            delete [] pRecordKey;
            return FAILURE;
        }
        DWORD dwOffset = 0;
        for (DWORD i = 0; i < dwParaCount; ++i)
        {
            (void)memcpy(pData + dwOffset, pbyRec + pFields[pPara[i].m_paraID - 1].m_fieldOffset, 
                    pFields[pPara[i].m_paraID - 1].m_fieldSize);
            dwOffset += pFields[pPara[i].m_paraID - 1].m_fieldSize;
        }

        /// 查找记录索引
        CKey recordKey(pData, dwDataLen);
        DCOP_Free(pData);
        IT_RECIDX it_record = ((*it_field).second).find(recordKey);
        if (it_record != ((*it_field).second).end())
        {
            delete [] pRecordKey;
            return FAILURE;
        }

        pRecordKey[dwIdxPos++] = recordKey;
    }

    /// 最后添加记录索引到对应的字段索引
    dwIdxPos = 0;
    for (IT_FLDIDX it_field = m_idx.begin();
        it_field != m_idx.end(); ++it_field)
    {
        (void)((*it_field).second).insert(MAP_RECIDX::value_type(pRecordKey[dwIdxPos++], recordIdx));
    }

    delete [] pRecordKey;
    return SUCCESS;
}

/*******************************************************
  函 数 名: CRecIdx::OnDelRec
  描    述: 删除整条记录触发的操作
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CRecIdx::OnDelRec(IDataHandle::Field *pFields, BYTE *pbyRec)
{
    /// 遍历索引
    for (IT_FLDIDX it_field = m_idx.begin();
        it_field != m_idx.end();
        ++it_field)
    {
        /// 获取参数个数
        CKey fieldKey((*it_field).first);
        DCOP_PARA_NODE *pPara = (DCOP_PARA_NODE *)fieldKey.GetKey();
        DWORD dwParaCount = fieldKey.GetLen() / sizeof(DCOP_PARA_NODE);
        if (!pPara || !dwParaCount)
        {
            continue;
        }

        /// 获取数据长度
        DWORD dwDataLen = 0;
        for (DWORD i = 0; i < dwParaCount; ++i)
        {
            dwDataLen += pFields[pPara[i].m_paraID - 1].m_fieldSize;
        }
        if (!dwDataLen)
        {
            continue;
        }

        /// 生成数据索引
        BYTE *pData = (BYTE *)DCOP_Malloc(dwDataLen);
        if (!pData) continue;
        DWORD dwOffset = 0;
        for (DWORD i = 0; i < dwParaCount; ++i)
        {
            (void)memcpy(pData + dwOffset, pbyRec + pFields[pPara[i].m_paraID - 1].m_fieldOffset, 
                    pFields[pPara[i].m_paraID - 1].m_fieldSize);
            dwOffset += pFields[pPara[i].m_paraID - 1].m_fieldSize;
        }

        /// 查找记录索引
        CKey recordKey(pData, dwDataLen);
        DCOP_Free(pData);
        IT_RECIDX it_record = ((*it_field).second).find(recordKey);
        if (it_record == ((*it_field).second).end())
        {
            continue;
        }

        (void)((*it_field).second).erase(it_record);
    }
}

/*******************************************************
  函 数 名: CRecIdx::BldKeyIdx
  描    述: 构建关键字索引
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CRecIdx::BldKeyIdx(DCOP_PARA_NODE *pPara, DWORD dwParaCount, 
                        IDataHandle::Field *pFields, BYTE *pbyRec, 
                        CIdx recordIdx)
{
    if (!pPara || !dwParaCount) return FAILURE;

    CKey fieldKey(pPara, sizeof(DCOP_PARA_NODE) * dwParaCount);
    ClearType(fieldKey);

    IT_FLDIDX it_field = m_idx.find(fieldKey);
    if (it_field == m_idx.end())
    {
        return FAILURE;
    }

    /// 获取数据长度
    DWORD dwDataLen = 0;
    for (DWORD i = 0; i < dwParaCount; ++i)
    {
        dwDataLen += pFields[pPara[i].m_paraID - 1].m_fieldSize;
    }
    if (!dwDataLen)
    {
        return FAILURE;
    }

    /// 生成数据索引
    BYTE *pData = (BYTE *)DCOP_Malloc(dwDataLen);
    if (!pData) return FAILURE;
    DWORD dwOffset = 0;
    for (DWORD i = 0; i < dwParaCount; ++i)
    {
        (void)memcpy(pData + dwOffset, pbyRec + pFields[pPara[i].m_paraID - 1].m_fieldOffset, 
                pFields[pPara[i].m_paraID - 1].m_fieldSize);
        dwOffset += pFields[pPara[i].m_paraID - 1].m_fieldSize;
    }

    /// 查找记录索引
    CKey recordKey(pData, dwDataLen);
    DCOP_Free(pData);
    IT_RECIDX it_record = ((*it_field).second).find(recordKey);
    if (it_record != ((*it_field).second).end())
    {
        return FAILURE;
    }

    (void)((*it_field).second).insert(MAP_RECIDX::value_type(recordKey, recordIdx));
    return SUCCESS;
}

/*******************************************************
  函 数 名: CRecIdx::FindRec
  描    述: 查找记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CRecIdx::CIdx CRecIdx::FindRec(DCOP_PARA_NODE *pPara, DWORD dwParaCount, 
                void *pData, DWORD dwDataLen)
{
    CKey fieldKey(pPara, sizeof(DCOP_PARA_NODE) * dwParaCount);
    ClearType(fieldKey);

    CKey recordKey(pData, dwDataLen);

    /// 查找字段索引
    IT_FLDIDX it_field = m_idx.find(fieldKey);
    if (it_field == m_idx.end())
    {
        return CIdx();
    }

    /// 查找记录索引
    IT_RECIDX it_record = ((*it_field).second).find(recordKey);
    if (it_record == ((*it_field).second).end())
    {
        return CIdx();
    }

    return ((*it_record).second);
}

/*******************************************************
  函 数 名: CRecIdx::ClearType
  描    述: 清除关键索引的操作/类型(只按字段ID和大小进行索引)
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CRecIdx::ClearType(CKey &fieldKey)
{
    DCOP_PARA_NODE *pPara = (DCOP_PARA_NODE *)fieldKey.GetKey();
    if (!pPara)
    {
        return;
    }

    DWORD dwCount = fieldKey.GetLen() / sizeof(DCOP_PARA_NODE);
    if (!dwCount)
    {
        return;
    }

    for (DWORD i = 0; i < dwCount; ++i)
    {
        pPara[i].m_opCode = 0;
        pPara[i].m_paraType = 0;
    }
}

