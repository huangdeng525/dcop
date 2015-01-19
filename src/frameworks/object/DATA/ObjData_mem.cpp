/// -------------------------------------------------
/// ObjData_mem.cpp : 内存数据实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjData_mem.h"
#include "Factory_if.h"
#include "cpu/bytes.h"
#include "string/tablestring.h"
#include "sock.h"
#include "ObjTimer_if.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CDataMem, "DataMem")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CDataMem)
    DCOP_IMPLEMENT_INTERFACE(IDataHandle)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END


/*******************************************************
  函 数 名: CDataMem::CDataMem
  描    述: CDataMem构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDataMem::CDataMem(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);

}

/*******************************************************
  函 数 名: CDataMem::~CDataMem
  描    述: CDataMem析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDataMem::~CDataMem()
{

    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CDataMem::Init
  描    述: 初始化
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMem::Init(DWORD dwAttrID, IObject *pOwner, IModel *piModel)
{
    /// 先调用父类的初始化
    DWORD dwRc = IDataHandle::Init(IData::TYPE_MEM, dwAttrID, pOwner, piModel, (IData *)m_piParent);
    if (dwRc)
    {
        return dwRc;
    }

    /// 获取属性的默认记录大小
    DWORD dwDefRecCount = piModel->GetDefRecCount(dwAttrID);
    if (!dwDefRecCount)
    {
        dwDefRecCount = DCOP_DATA_DEF_REC_COUNT;
    }

    /// 新建内存页
    if (m_memPage.NewPage(GetRecSize(), dwDefRecCount) == NULL)
    {
        return FAILURE;
    }

    /// 创建关键字索引
    Field *pFields = GetFields();
    DWORD dwFieldCount = GetFieldCount();
    DCOP_PARA_NODE keyPara;
    for (DWORD i = 0; i < dwFieldCount; ++i)
    {
        if (!pFields[i].m_isKey)
            continue;

        keyPara.m_paraID = i + 1;
        keyPara.m_opCode = 0;
        keyPara.m_paraType = pFields[i].m_fieldType;
        keyPara.m_paraSize = pFields[i].m_fieldSize;
        (void)AddKeyIdx(&keyPara, 1);
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMem::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDataMem::Dump(LOG_PRINT logPrint, LOG_PARA logPara)
{
    if (!logPrint)
    {
        return;
    }

    IObject *pObject = GetOwner();
    if (!pObject)
    {
        return;
    }

    AutoObjLock(pObject);

    Field *pFields = GetFields();
    DWORD dwFieldCount = GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return;
    }

    logPrint(STR_FORMAT("Mem Data Table: ['%s'], Records Count: %d \r\n", GetTableName(), GetRecCount()), logPara);
    CTableString tableStr(dwFieldCount, GetRecCount() + 1, "  ");

    for (DWORD i = 0; i < dwFieldCount; ++i)
    {
        tableStr << pFields[i].m_fieldName;
    }

    DCOP_START_TIME();

    for (CMemPage::RecordHead *pRecord = m_memPage.GetFirstRec();
        pRecord != NULL;
        pRecord = m_memPage.GetNextRec(pRecord))
    {
        BYTE *pbyRec = (BYTE *)(pRecord + 1);
        DWORD dwOffset = 0;
        for (DWORD i = 0; i < dwFieldCount; ++i)
        {
            switch (pFields[i].m_fieldType)
            {
                case IModel::FIELD_BYTE:
                    tableStr << STR_FORMAT("0x%02x", *(pbyRec + dwOffset));
                    break;
                case IModel::FIELD_WORD:
                    tableStr << STR_FORMAT("0x%04x", Bytes_GetWord(pbyRec + dwOffset));
                    break;
                case IModel::FIELD_DWORD:
                    tableStr << STR_FORMAT("0x%08lx", Bytes_GetDword(pbyRec + dwOffset));
                    break;
                case IModel::FIELD_CHAR:
                    tableStr << STR_FORMAT("%c", *(char *)(pbyRec + dwOffset));
                    break;
                case IModel::FIELD_SHORT:
                case IModel::FIELD_INTEGER:
                    tableStr << STR_FORMAT("%d", (int)Bytes_GetDwordValue(pbyRec + dwOffset, pFields[i].m_fieldSize));
                    break;
                case IModel::FIELD_IDENTIFY:
                case IModel::FIELD_NUMBER:
                    tableStr << STR_FORMAT("%d", Bytes_GetDwordValue(pbyRec + dwOffset, pFields[i].m_fieldSize));
                    break;
                case IModel::FIELD_STRING:
                {
                    CDString strTmp((char *)pbyRec + dwOffset, pFields[i].m_fieldSize);
                    tableStr << (const char *)strTmp;
                }
                    break;
                case IModel::FIELD_BUFFER:
                    tableStr << CBufferString(pbyRec + dwOffset, pFields[i].m_fieldSize);
                    break;
                case IModel::FIELD_DATE:
                    break;
                case IModel::FIELD_TIME:
                    break;
                case IModel::FIELD_IP:
                {
                    char szIP[OSSOCK_IPSIZE];
                    (void)memset(szIP, 0, sizeof(szIP));
                    objSock::GetIPStringByValue(*(DWORD *)(pbyRec + dwOffset), szIP);
                    tableStr << szIP;
                }
                    break;
                case IModel::FIELD_PTR:
                    tableStr << STR_FORMAT("%p", *(void **)(pbyRec + dwOffset));
                    break;
                case IModel::FIELD_TIMER:
                {
                    CDString strTimer;
                    ITimer::IWheel::GetString((ITimer::Handle)(pbyRec + dwOffset), strTimer);
                    tableStr << (const char *)strTimer;
                }
                    break;
                case IModel::FIELD_PASS:
                    tableStr << "********";
                    break;
                default:
                    break;
            }

            dwOffset += pFields[i].m_fieldSize;

        }
    }

    DCOP_END_TIME();

    tableStr.Show(logPrint, logPara, "=", "-");

    logPrint(STR_FORMAT("[cost time: %d ms] \r\n", DCOP_COST_TIME()), logPara);
}

/*******************************************************
  函 数 名: CDataMem::AddRecord
  描    述: 添加一条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMem::AddRecord(DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
                        void *pReqData, DWORD dwReqDataLen, 
                        DCOP_PARA_NODE **ppEvtPara, DWORD *pdwEvtParaCount, 
                        CDArray *pEvtData)
{
    /// 获取所属对象
    IObject *pObject = GetOwner();
    if (!pObject)
    {
        return FAILURE;
    }

    /// 获取字段信息
    Field *pFields = GetFields();
    DWORD dwFieldCount = GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    /// 申请记录空间
    BYTE *pbyRec = (BYTE *)DCOP_Malloc(GetRecSize());
    if (!pbyRec)
    {
        return FAILURE;
    }

    /// 使用所属对象的保护锁
    AutoObjLock(pObject);

    /// 填充完整的字段
    DWORD dwRc = FillRecord(pFields, dwFieldCount, 
                        pbyRec, GetRecSize(), 
                        pReqPara, dwReqParaCount, 
                        pReqData, dwReqDataLen);
    if (dwRc)
    {
        DCOP_Free(pbyRec);
        return FAILURE;
    }

    /// 添加记录到内存页面中
    CMemPage::RecordHead *pRecord = m_memPage.AppendRec(pbyRec);
    if (!pRecord)
    {
        DCOP_Free(pbyRec);
        return FAILURE;
    }

    /// 同时触发添加索引(使用内存记录指针)
    CRecIdx::CIdx idx(pRecord);
    dwRc = m_recIdx.OnAddRec(pFields, (BYTE *)(pRecord + 1), idx);
    if (dwRc)
    {
        DCOP_Free(pbyRec);
        (void)m_memPage.DeleteRec(pRecord);
        return FAILURE;
    }

    /// 触发事件
    if (ppEvtPara && pdwEvtParaCount && pEvtData)
    {
        DCOP_PARA_NODE *pEvtPara = 0;
        DWORD dwEvtParaCount = 0;
        pEvtPara = GetOutPara(pFields, dwFieldCount, dwEvtParaCount);
        if (!pEvtPara)
        {
            DCOP_Free(pbyRec);
            return SUCCESS;
        }

        *ppEvtPara = pEvtPara;
        *pdwEvtParaCount = dwEvtParaCount;

        pEvtData->SetNodeSize(GetRecSize());
        (void)pEvtData->Append(pbyRec);
    }

    DCOP_Free(pbyRec);
    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMem::DelRecord
  描    述: 删除一条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMem::DelRecord(BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        DCOP_PARA_NODE **ppEvtPara, DWORD *pdwEvtParaCount, 
                        CDArray *pEvtData)
{
    /// 获取所属对象
    IObject *pObject = GetOwner();
    if (!pObject)
    {
        return FAILURE;
    }

    /// 获取字段信息
    Field *pFields = GetFields();
    DWORD dwFieldCount = GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    /// 申请记录空间
    BYTE *pbyRec = (BYTE *)DCOP_Malloc(GetRecSize());
    if (!pbyRec)
    {
        return FAILURE;
    }

    /// 使用所属对象的保护锁
    AutoObjLock(pObject);

    /// 精确匹配查找(通过关键字索引)
    if (DCOP_CONDITION_ONE == byCond)
    {
        /// 查找记录
        CRecIdx::CIdx idx = m_recIdx.FindRec(pCondPara, dwCondParaCount, pCondData, dwCondDataLen);
        CMemPage::RecordHead *pRecord = (CMemPage::RecordHead *)(void *)idx;
        if (!pRecord)
        {
            DCOP_Free(pbyRec);
            return FAILURE;
        }

        /// 获取记录
        (void)memcpy(pbyRec, (BYTE *)(pRecord + 1), GetRecSize());

        /// 删除记录
        DWORD dwRc = m_memPage.DeleteRec(pRecord);
        if (dwRc)
        {
            DCOP_Free(pbyRec);
            return dwRc;
        }

        /// 删除索引
        m_recIdx.OnDelRec(pFields, pbyRec);

        /// 触发事件
        if (ppEvtPara && pdwEvtParaCount && pEvtData)
        {
            DCOP_PARA_NODE *pEvtPara = 0;
            DWORD dwEvtParaCount = 0;
            pEvtPara = GetOutPara(pFields, dwFieldCount, dwEvtParaCount);
            if (!pEvtPara)
            {
                DCOP_Free(pbyRec);
                return SUCCESS;
            }

            *ppEvtPara = pEvtPara;
            *pdwEvtParaCount = dwEvtParaCount;

            pEvtData->SetNodeSize(GetRecSize());
            (void)pEvtData->Append(pbyRec);
        }

        DCOP_Free(pbyRec);
        return SUCCESS;
    }

    /// 模糊匹配查找(遍历进行匹配)
    DWORD dwDelCount = 0;
    if (pEvtData) pEvtData->SetNodeSize(GetRecSize());
    CMemPage::RecordHead *pRecord = m_memPage.GetFirstRec();
    while (pRecord != NULL)
    {
        CMemPage::RecordHead *pRecordTmp = pRecord;
        pRecord = m_memPage.GetNextRec(pRecord);

        /// 是否匹配当前记录
        bool bRc = bMatchRecord(pFields, dwFieldCount, 
                        (BYTE *)(pRecordTmp + 1), byCond, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen);
        if (!bRc) continue;

        /// 获取记录
        (void)memcpy(pbyRec, (BYTE *)(pRecordTmp + 1), GetRecSize());

        /// 删除页面中的记录
        DWORD dwRc = m_memPage.DeleteRec(pRecordTmp);
        if (dwRc) continue;

        /// 同时触发删除索引
        m_recIdx.OnDelRec(pFields, pbyRec);
        dwDelCount++;

        /// 触发事件
        if (pEvtData) (void)pEvtData->Append(pbyRec);
    }

    DCOP_Free(pbyRec);

    if (!dwDelCount)
    {
        return FAILURE;
    }

    /// 触发事件
    if (ppEvtPara && pdwEvtParaCount && pEvtData)
    {
        DCOP_PARA_NODE *pEvtPara = 0;
        DWORD dwEvtParaCount = 0;
        pEvtPara = GetOutPara(pFields, dwFieldCount, dwEvtParaCount);
        if (!pEvtPara)
        {
            pEvtData->Clear();
            return SUCCESS;
        }

        *ppEvtPara = pEvtPara;
        *pdwEvtParaCount = dwEvtParaCount;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMem::EditRecord
  描    述: 编辑一条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMem::EditRecord(BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
                        void *pReqData, DWORD dwReqDataLen, 
                        DCOP_PARA_NODE **ppEvtPara, DWORD *pdwEvtParaCount, 
                        CDArray *pEvtData)
{
    /// 获取所属对象
    IObject *pObject = GetOwner();
    if (!pObject)
    {
        return FAILURE;
    }

    /// 获取字段信息
    Field *pFields = GetFields();
    DWORD dwFieldCount = GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    /// 获取事件参数
    DCOP_PARA_NODE *pEvtPara = 0;
    DWORD dwEvtParaCount = 0;
    void *pEvtRec = 0;
    DWORD dwEvtDataLen = 0;
    if (ppEvtPara && pdwEvtParaCount)
    {
        pEvtPara = GetOutPara(pFields, dwFieldCount, dwEvtParaCount, 
                        pReqPara, dwReqParaCount, &dwEvtDataLen, true);
        if (dwEvtDataLen)
        {
            pEvtRec = DCOP_Malloc(dwEvtDataLen);
        }
    }

    /// 使用所属对象的保护锁
    AutoObjLock(pObject);

    /// 精确匹配查找(通过关键字索引)
    if (DCOP_CONDITION_ONE == byCond)
    {
        CRecIdx::CIdx idx = m_recIdx.FindRec(pCondPara, dwCondParaCount, pCondData, dwCondDataLen);
        CMemPage::RecordHead *pRecord = (CMemPage::RecordHead *)(void *)idx;
        if (!pRecord) return FAILURE;

        /// 更新记录
        DWORD dwRc = UpdateRecord(pFields, dwFieldCount, 
                        (BYTE *)(pRecord + 1), GetRecSize(), 
                        pReqPara, dwReqParaCount, 
                        pReqData, dwReqDataLen);
        if (dwRc != SUCCESS)
        {
            if (pEvtRec) DCOP_Free(pEvtRec);
            if (pEvtPara) DCOP_Free(pEvtPara);
            return dwRc;
        }

        /// 拷贝记录
        if (pEvtRec && pEvtPara && 
            (CopyRecord(pFields, dwFieldCount, 
                        (BYTE *)(pRecord + 1), GetRecSize(), 
                        pEvtPara, dwEvtParaCount, 
                        pEvtRec, dwEvtDataLen) != SUCCESS))
        {
            DCOP_Free(pEvtRec);
            DCOP_Free(pEvtPara);
            return SUCCESS;
        }

        /// 触发事件
        if (ppEvtPara && pdwEvtParaCount)
        {
            *ppEvtPara = pEvtPara;
            *pdwEvtParaCount = dwEvtParaCount;
        }
        if (pEvtData)
        {
            pEvtData->SetNodeSize(GetRecSize());
            (void)pEvtData->Append(pEvtRec);
        }

        if (pEvtRec) DCOP_Free(pEvtRec);
        return SUCCESS;
    }

    /// 模糊匹配查找(遍历进行匹配)
    DWORD dwEditCount = 0;
    if (pEvtData) pEvtData->SetNodeSize(GetRecSize());
    for (CMemPage::RecordHead *pRecord = m_memPage.GetFirstRec();
        pRecord != NULL;
        pRecord = m_memPage.GetNextRec(pRecord))
    {
        /// 是否匹配当前记录
        bool bRc = bMatchRecord(pFields, dwFieldCount, 
                        (BYTE *)(pRecord + 1), byCond, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen);
        if (!bRc) continue;

        /// 更新记录
        DWORD dwRc = UpdateRecord(pFields, dwFieldCount, 
                        (BYTE *)(pRecord + 1), GetRecSize(), 
                        pReqPara, dwReqParaCount, 
                        pReqData, dwReqDataLen);
        if (dwRc != SUCCESS)
        {
            continue;
        }

        /// 拷贝记录
        if (pEvtRec && pEvtPara && 
            (CopyRecord(pFields, dwFieldCount, 
                        (BYTE *)(pRecord + 1), GetRecSize(), 
                        pEvtPara, dwEvtParaCount, 
                        pEvtRec, dwEvtDataLen) == SUCCESS))
        {
            (void)pEvtData->Append(pEvtRec);
        }

        dwEditCount++;
    }

    if (pEvtRec) DCOP_Free(pEvtRec);

    if (!dwEditCount)
    {
        if (pEvtPara) DCOP_Free(pEvtPara);
        return FAILURE;
    }

    if (ppEvtPara && pdwEvtParaCount)
    {
        *ppEvtPara = pEvtPara;
        *pdwEvtParaCount = dwEvtParaCount;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMem::QueryRecord
  描    述: 查询多条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMem::QueryRecord(BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
                        DCOP_PARA_NODE *&rpRspPara, DWORD &rdwRspParaCount, 
                        CDArray &aRspData)
{
    /// 获取所属对象
    IObject *pObject = GetOwner();
    if (!pObject)
    {
        return FAILURE;
    }

    /// 获取字段信息
    Field *pFields = GetFields();
    DWORD dwFieldCount = GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    /// 获取响应参数
    DWORD dwRspParaCount = 0;
    DWORD dwRspDataLen = 0;
    DCOP_PARA_NODE *pRspPara = GetOutPara(pFields, dwFieldCount, dwRspParaCount, 
                        pReqPara, dwReqParaCount, &dwRspDataLen);
    if (!pRspPara)
    {
        return FAILURE;
    }

    /// 响应数据长度错误
    if (!dwRspDataLen)
    {
        DCOP_Free(pRspPara);
        return FAILURE;
    }

    /// 申请响应数据空间
    void *pRspData = DCOP_Malloc(dwRspDataLen);
    if (!pRspData)
    {
        DCOP_Free(pRspPara);
        return FAILURE;
    }

    aRspData.SetNodeSize(dwRspDataLen);
    aRspData.SetMaxCount(m_memPage.GetRecCount());

    /// 使用所属对象的保护锁
    AutoObjLock(pObject);

    /// 精确匹配查找(通过关键字索引)
    if (DCOP_CONDITION_ONE == byCond)
    {
        CRecIdx::CIdx idx = m_recIdx.FindRec(pCondPara, dwCondParaCount, pCondData, dwCondDataLen);
        CMemPage::RecordHead *pRecord = (CMemPage::RecordHead *)(void *)idx;
        if (!pRecord)
        {
            DCOP_Free(pRspPara);
            DCOP_Free(pRspData);
            return FAILURE;
        }

        DWORD dwRc = CopyRecord(pFields, dwFieldCount, 
                        (BYTE *)(pRecord + 1), GetRecSize(), 
                        pRspPara, dwRspParaCount, 
                        pRspData, dwRspDataLen);
        if (dwRc)
        {
            DCOP_Free(pRspPara);
            DCOP_Free(pRspData);
            return FAILURE;
        }

        dwRc = aRspData.Append(pRspData);
        DCOP_Free(pRspData);
        if (dwRc)
        {
            DCOP_Free(pRspPara);
            return FAILURE;
        }

        rpRspPara = pRspPara;
        rdwRspParaCount = dwRspParaCount;

        return SUCCESS;
    }

    /// 获取记录行
    DWORD dwReqOffset = 0;
    DWORD dwReqLimit = 0;
    GetOffsetAndLimit(dwReqOffset, dwReqLimit, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen);
    if (!dwReqLimit) dwReqLimit = 0xffffffff;

    /// 模糊匹配查找(遍历进行匹配)
    DWORD dwRspOffset = 0;
    DWORD dwRspLimit = 0;
    for (CMemPage::RecordHead *pRecord = m_memPage.GetFirstRec();
        pRecord != NULL;
        pRecord = m_memPage.GetNextRec(pRecord))
    {
        /// 是否匹配当前记录
        bool bRc = bMatchRecord(pFields, dwFieldCount, 
                        (BYTE *)(pRecord + 1), byCond, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen);
        if (!bRc) continue;

        /// 指定记录行
        if ((dwRspOffset < dwReqOffset) || (dwRspLimit >= dwReqLimit))
        {
            dwRspOffset++;
            continue;
        }
        dwRspOffset++;
        dwRspLimit++;

        /// 拷贝输出的记录
        DWORD dwRc = CopyRecord(pFields, dwFieldCount, 
                        (BYTE *)(pRecord + 1), GetRecSize(), 
                        pRspPara, dwRspParaCount, 
                        pRspData, dwRspDataLen);
        if (dwRc) continue;

        (void)aRspData.Append(pRspData);
    }

    DCOP_Free(pRspData);
    if (!aRspData.Count())
    {
        DCOP_Free(pRspPara);
        return FAILURE;
    }

    rpRspPara = pRspPara;
    rdwRspParaCount = dwRspParaCount;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMem::CountRecord
  描    述: 统计记录数量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMem::CountRecord(BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        DWORD &rdwCount)
{
    /// 获取所属对象
    IObject *pObject = GetOwner();
    if (!pObject)
    {
        return FAILURE;
    }

    /// 获取字段信息
    Field *pFields = GetFields();
    DWORD dwFieldCount = GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    /// 使用所属对象的保护锁
    AutoObjLock(pObject);

    /// 精确匹配查找(通过关键字索引)
    if (DCOP_CONDITION_ONE == byCond)
    {
        CRecIdx::CIdx idx = m_recIdx.FindRec(pCondPara, dwCondParaCount, pCondData, dwCondDataLen);
        CMemPage::RecordHead *pRecord = (CMemPage::RecordHead *)(void *)idx;
        rdwCount = (!pRecord)? 0 : 1;
        return SUCCESS;
    }

    /// 模糊匹配查找(遍历进行匹配)
    rdwCount = 0;
    for (CMemPage::RecordHead *pRecord = m_memPage.GetFirstRec();
        pRecord != NULL;
        pRecord = m_memPage.GetNextRec(pRecord))
    {
        /// 是否匹配当前记录
        bool bRc = bMatchRecord(pFields, dwFieldCount, 
                        (BYTE *)(pRecord + 1), byCond, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen);
        if (!bRc) continue;

        rdwCount++;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMem::AddKeyIdx
  描    述: 添加关键字索引
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMem::AddKeyIdx(DCOP_PARA_NODE *pIdxPara, DWORD dwIdxParaCount)
{
    IObject *pObject = GetOwner();
    if (!pObject)
    {
        return FAILURE;
    }

    AutoObjLock(pObject);

    /// 先添加关键字
    DWORD dwRc = m_recIdx.AddKey(pIdxPara, dwIdxParaCount);
    if (dwRc)
    {
        return dwRc;
    }

    /// 再构建索引
    for (CMemPage::RecordHead *pRecord = m_memPage.GetFirstRec();
        pRecord != NULL;
        pRecord = m_memPage.GetNextRec(pRecord))
    {
        /// 构建添加的索引
        CRecIdx::CIdx idx(pRecord);
        dwRc = m_recIdx.BldKeyIdx(pIdxPara, dwIdxParaCount, GetFields(), (BYTE *)(pRecord + 1), idx);
        if (dwRc)
        {
            (void)m_recIdx.DelKey(pIdxPara, dwIdxParaCount);
            return FAILURE;
        }
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMem::DelKeyIdx
  描    述: 删除关键字索引
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMem::DelKeyIdx(DCOP_PARA_NODE *pIdxPara, DWORD dwIdxParaCount)
{
    IObject *pObject = GetOwner();
    if (!pObject)
    {
        return FAILURE;
    }

    AutoObjLock(pObject);

    return m_recIdx.DelKey(pIdxPara, dwIdxParaCount);
}

