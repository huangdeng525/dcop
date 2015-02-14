/// -------------------------------------------------
/// ObjData_file.cpp : 文件数据实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjData_file.h"
#include "Factory_if.h"
#include "fs/file.h"
#include "string/tablestring.h"
#include "sock.h"
#include "ObjTimer_if.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CDataFile, "DataFile")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CDataFile)
    DCOP_IMPLEMENT_INTERFACE(IDataHandle)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END


/*******************************************************
  函 数 名: CDataFile::BytesChangeFileHead
  描    述: 文件头字节序转换
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDataFile::BytesChangeFileHead(FileHead *pFileHead)
{
    const BYTES_CHANGE_RULE FileHeadBORule[] = 
    {
        {SIZE_OF(CDataFile::FileHead, m_headerSize), OFFSET_OF(CDataFile::FileHead, m_headerSize)},
        {SIZE_OF(CDataFile::FileHead, m_objectID), OFFSET_OF(CDataFile::FileHead, m_objectID)},
        {SIZE_OF(CDataFile::FileHead, m_attrID), OFFSET_OF(CDataFile::FileHead, m_attrID)},
        {SIZE_OF(CDataFile::FileHead, m_fieldCount), OFFSET_OF(CDataFile::FileHead, m_fieldCount)},
        {SIZE_OF(CDataFile::FileHead, m_recordCount), OFFSET_OF(CDataFile::FileHead, m_recordCount)},
        {SIZE_OF(CDataFile::FileHead, m_totalCount), OFFSET_OF(CDataFile::FileHead, m_totalCount)},
        {SIZE_OF(CDataFile::FileHead, m_curIdentify), OFFSET_OF(CDataFile::FileHead, m_curIdentify)}
    };
    Bytes_ChangeOrderByRule(FileHeadBORule, ARRAY_SIZE(FileHeadBORule), pFileHead, sizeof(FileHead));
}

/*******************************************************
  函 数 名: CDataFile::BytesChangeFieldInfo
  描    述: 字段信息字节序转换
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDataFile::BytesChangeFieldInfo(IModel::Field *pFileInfo)
{
    const BYTES_CHANGE_RULE FieldInfoBORule[] = 
    {
        {SIZE_OF(IModel::Field, m_fieldSize), OFFSET_OF(IModel::Field, m_fieldSize)},
        {SIZE_OF(IModel::Field, m_defaultValue), OFFSET_OF(IModel::Field, m_defaultValue)},
        {SIZE_OF(IModel::Field, m_minValue), OFFSET_OF(IModel::Field, m_minValue)},
        {SIZE_OF(IModel::Field, m_maxValue), OFFSET_OF(IModel::Field, m_maxValue)}
    };
    Bytes_ChangeOrderByRule(FieldInfoBORule, ARRAY_SIZE(FieldInfoBORule), pFileInfo, sizeof(IModel::Field));
}

/*******************************************************
  函 数 名: CDataFile::CDataFile
  描    述: CDataFile构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDataFile::CDataFile(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);

    (void)memset(&m_head, 0, sizeof(m_head));

    if (argv && (argc > 0))
    {
        m_fileName = (const char *)argv[0];
        if (m_fileName.Length() && (m_fileName.Get(CDString::TAIL) != '/'))
        {
            m_fileName << "/";
        }
    }
}

/*******************************************************
  函 数 名: CDataFile::~CDataFile
  描    述: CDataFile析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDataFile::~CDataFile()
{
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CDataFile::Init
  描    述: 初始化
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataFile::Init(DWORD dwAttrID, IObject *pOwner, IModel *piModel)
{
    /////////////////////////////////////////////////
    /// 先调用父类的初始化
    /////////////////////////////////////////////////
    DWORD dwRc = IDataHandle::Init(IData::TYPE_MEM, dwAttrID, pOwner, piModel, (IData *)m_piParent);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /////////////////////////////////////////////////
    /// 获取字段基本信息(写入文件头的字段是基本信息)
    /////////////////////////////////////////////////
    IModel::Field *pBaseFields = (IModel::Field *)DCOP_Malloc(
                        GetFieldCount() * sizeof(IModel::Field));
    if (!pBaseFields) return FAILURE;
    Field *pFields = GetFields();
    if (!pFields)
    {
        DCOP_Free(pBaseFields);
        return FAILURE;
    }
    for (DWORD i = 0; i < GetFieldCount(); ++i)
    {
        (void)memcpy(&(pBaseFields[i]), &(pFields[i]), sizeof(IModel::Field));
    }

    /////////////////////////////////////////////////
    /// 获取文件名
    /////////////////////////////////////////////////
    const char *cszTableName = GetTableName();
    if (!cszTableName || !(*cszTableName))
    {
        DCOP_Free(pBaseFields);
        return FAILURE;
    }

    m_fileName << cszTableName << ".dbf";

    /////////////////////////////////////////////////
    /// 读取文件头
    /////////////////////////////////////////////////
    dwRc = DCOP_RestoreFromFile(m_fileName, &m_head, sizeof(m_head), 0);
    if (!dwRc)
    {
        /// 读取后转换文件头的字节序
        BytesChangeFileHead(&m_head);

        /// 成功后，进行系列校验
        if (strcmp(m_head.m_tableName, cszTableName) ||
            (m_head.m_objectID != pOwner->ID()) ||
            (m_head.m_attrID != dwAttrID) ||
            (m_head.m_headerSize != sizeof(FileHead)) ||
            (m_head.m_fieldCount != GetFieldCount()))
        {
            DCOP_Free(pBaseFields);
            return FAILURE;
        }

        /// 比较字段是否一致
        IModel::Field *pSavedFields = (IModel::Field *)DCOP_Malloc(
                        GetFieldCount() * sizeof(IModel::Field));
        if (!pSavedFields)
        {
            DCOP_Free(pBaseFields);
            return FAILURE;
        }
        dwRc = DCOP_RestoreFromFile(m_fileName, pSavedFields, 
                        GetFieldCount() * sizeof(IModel::Field), sizeof(m_head));
        if (!dwRc)
        {
            for (DWORD i = 0; i < GetFieldCount(); ++i)
            {
                /// 读取后转换字段的字节序
                BytesChangeFieldInfo(pSavedFields + i);
            }
        }
        if (dwRc || (memcmp(pSavedFields, pBaseFields, 
                        GetFieldCount() * sizeof(IModel::Field)) != 0))
        {
            DCOP_Free(pBaseFields);
            DCOP_Free(pSavedFields);
            return FAILURE;
        }
        DCOP_Free(pBaseFields);
        DCOP_Free(pSavedFields);

        /// 恢复当前ID标识
        SetCurIdentify(m_head.m_curIdentify);

        /// 构建索引
        dwRc = BldAllIdx();
        if (dwRc != SUCCESS) return dwRc;

        return SUCCESS;
    }

    /////////////////////////////////////////////////
    /// 读取失败后，初始化文件
    /////////////////////////////////////////////////
    (void)memset(m_head.m_tableName, 0, sizeof(m_head.m_tableName));
    (void)snprintf(m_head.m_tableName, sizeof(m_head.m_tableName), "%s", cszTableName);
    m_head.m_ver = 1;
    m_head.m_level = 0;
    m_head.m_objectID = pOwner->ID();
    m_head.m_attrID = dwAttrID;
    m_head.m_headerSize = sizeof(FileHead);
    m_head.m_fieldCount = GetFieldCount();
    m_head.m_recordCount = 0;
    m_head.m_totalCount = 0;
    /// 写入前转换文件头的字节序(因为文件头在本地还要使用，所以拷贝一份)
    FileHead headTmp;
    (void)memcpy(&headTmp, &m_head, sizeof(m_head));
    BytesChangeFileHead(&headTmp);
    dwRc = DCOP_SaveToFile(m_fileName, &headTmp, sizeof(headTmp), 0);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /////////////////////////////////////////////////
    /// 写入字段信息
    /////////////////////////////////////////////////
    /// 写入前转换字段信息的字节序(因为字段信息不会再使用了，所以就直接进行转换)
    for (DWORD i = 0; i < GetFieldCount(); ++i)
    {
        /// 读取后转换字段的字节序
        BytesChangeFieldInfo(pBaseFields + i);
    }
    dwRc = DCOP_SaveToFile(m_fileName, pBaseFields, 
                        GetFieldCount() * sizeof(IModel::Field), sizeof(m_head));
    DCOP_Free(pBaseFields);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 构建索引
    (void)BldAllIdx();
    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataFile::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDataFile::Dump(LOG_PRINT logPrint, LOG_PARA logPara)
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

    logPrint(STR_FORMAT("File Data Table: ['%s'], Records Count: %d \r\n", (const char *)m_fileName, GetRecCount()), logPara);
    CTableString tableStr(dwFieldCount, GetRecCount() + 1, "  ");

    for (DWORD i = 0; i < dwFieldCount; ++i)
    {
        tableStr << pFields[i].m_fieldName;
    }

    BYTE *pbyRec = (BYTE *)DCOP_Malloc(GetRecSize());
    if (!pbyRec)
    {
        return;
    }

    DCOP_START_TIME();

    for (DWORD dwRecNo = 1; dwRecNo <= m_head.m_totalCount; ++dwRecNo)
    {
        DWORD dwRc = GetRecord(dwRecNo, pbyRec);
        if (dwRc != SUCCESS) continue;

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

    DCOP_Free(pbyRec);
}

/*******************************************************
  函 数 名: CDataFile::AddRecord
  描    述: 添加一条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataFile::AddRecord(DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
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
    if (dwRc != SUCCESS)
    {
        DCOP_Free(pbyRec);
        return dwRc;
    }

    /// 添加记录到文件中
    DWORD dwRecNo = AddRecord(pbyRec);
    if (!dwRecNo)
    {
        DCOP_Free(pbyRec);
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
  函 数 名: CDataFile::DelRecord
  描    述: 删除一条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataFile::DelRecord(BYTE byCond, 
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
        DWORD dwRecNo = (DWORD)idx;
        if (!dwRecNo)
        {
            DCOP_Free(pbyRec);
            return FAILURE;
        }

        /// 获取记录
        DWORD dwRc = GetRecord(dwRecNo, pbyRec);
        if (dwRc != SUCCESS)
        {
            DCOP_Free(pbyRec);
            return dwRc;
        }

        /// 删除记录
        dwRc = DelRecord(dwRecNo);
        if (dwRc != SUCCESS)
        {
            DCOP_Free(pbyRec);
            return dwRc;
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

    /// 模糊匹配查找(遍历进行匹配)
    DWORD dwDelCount = 0;
    if (pEvtData) pEvtData->SetNodeSize(GetRecSize());
    for (DWORD dwRecNo = 1; dwRecNo <= m_head.m_totalCount; ++dwRecNo)
    {
        /// 获取当前记录
        DWORD dwRc = GetRecord(dwRecNo, pbyRec);
        if (dwRc) continue;

        /// 是否匹配当前记录
        bool bRc = bMatchRecord(pFields, dwFieldCount, 
                        pbyRec, byCond, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen);
        if (!bRc) continue;

        /// 删除记录
        dwRc = DelRecord(dwRecNo);
        if (dwRc) continue;
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
  函 数 名: CDataFile::EditRecord
  描    述: 编辑一条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataFile::EditRecord(BYTE byCond, 
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

    /// 准备复制记录的空间
    BYTE *pbyRec = (BYTE *)DCOP_Malloc(GetRecSize());
    if (!pbyRec)
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
        /// 查找记录
        CRecIdx::CIdx idx = m_recIdx.FindRec(pCondPara, dwCondParaCount, pCondData, dwCondDataLen);
        DWORD dwRecNo = (DWORD)idx;
        if (!dwRecNo)
        {
            DCOP_Free(pbyRec);
            if (pEvtRec) DCOP_Free(pEvtRec);
            if (pEvtPara) DCOP_Free(pEvtPara);
            return FAILURE;
        }

        /// 获取记录
        DWORD dwRc = GetRecord(dwRecNo, pbyRec);
        if (dwRc != SUCCESS)
        {
            DCOP_Free(pbyRec);
            if (pEvtRec) DCOP_Free(pEvtRec);
            if (pEvtPara) DCOP_Free(pEvtPara);
            return dwRc;
        }

        /// 更新记录
        dwRc = UpdateRecord(pFields, dwFieldCount, 
                        pbyRec, GetRecSize(), 
                        pReqPara, dwReqParaCount, 
                        pReqData, dwReqDataLen);
        if (dwRc != SUCCESS)
        {
            DCOP_Free(pbyRec);
            if (pEvtRec) DCOP_Free(pEvtRec);
            if (pEvtPara) DCOP_Free(pEvtPara);
            return dwRc;
        }

        dwRc = SetRecord(dwRecNo, pbyRec);
        if (dwRc != SUCCESS)
        {
            DCOP_Free(pbyRec);
            if (pEvtRec) DCOP_Free(pEvtRec);
            if (pEvtPara) DCOP_Free(pEvtPara);
            return dwRc;
        }

        /// 拷贝记录
        if (pEvtRec && pEvtPara && 
            (CopyRecord(pFields, dwFieldCount, 
                        pbyRec, GetRecSize(), 
                        pEvtPara, dwEvtParaCount, 
                        pEvtRec, dwEvtDataLen) != SUCCESS))
        {
            DCOP_Free(pbyRec);
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

        DCOP_Free(pbyRec);
        if (pEvtRec) DCOP_Free(pEvtRec);
        return SUCCESS;
    }

    /// 模糊匹配查找(遍历进行匹配)
    DWORD dwEditCount = 0;
    if (pEvtData) pEvtData->SetNodeSize(GetRecSize());
    for (DWORD dwRecNo = 1; dwRecNo <= m_head.m_totalCount; ++dwRecNo)
    {
        /// 获取当前记录
        DWORD dwRc = GetRecord(dwRecNo, pbyRec);
        if (dwRc) continue;

        /// 是否匹配当前记录
        bool bRc = bMatchRecord(pFields, dwFieldCount, 
                        pbyRec, byCond, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen);
        if (!bRc) continue;

        /// 更新当前记录
        dwRc = UpdateRecord(pFields, dwFieldCount, 
                        pbyRec, GetRecSize(), 
                        pReqPara, dwReqParaCount, 
                        pReqData, dwReqDataLen);
        if (dwRc != SUCCESS) continue;

        dwRc = SetRecord(dwRecNo, pbyRec);
        if (dwRc != SUCCESS) continue;

        /// 拷贝记录
        if (pEvtRec && pEvtPara && 
            (CopyRecord(pFields, dwFieldCount, 
                        pbyRec, GetRecSize(), 
                        pEvtPara, dwEvtParaCount, 
                        pEvtRec, dwEvtDataLen) == SUCCESS))
        {
            (void)pEvtData->Append(pEvtRec);
        }

        dwEditCount++;
    }

    DCOP_Free(pbyRec);
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
  函 数 名: CDataFile::QueryRecord
  描    述: 查询多条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataFile::QueryRecord(BYTE byCond, 
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
    aRspData.SetMaxCount(m_head.m_recordCount);

    /// 准备复制记录的空间
    BYTE *pbyRec = (BYTE *)DCOP_Malloc(GetRecSize());
    if (!pbyRec)
    {
        DCOP_Free(pRspPara);
        DCOP_Free(pRspData);
        return FAILURE;
    }

    /// 使用所属对象的保护锁
    AutoObjLock(pObject);

    /// 精确匹配查找(通过关键字索引)
    if (DCOP_CONDITION_ONE == byCond)
    {
        /// 查找记录
        CRecIdx::CIdx idx = m_recIdx.FindRec(pCondPara, dwCondParaCount, pCondData, dwCondDataLen);
        DWORD dwRecNo = (DWORD)idx;
        if (!dwRecNo)
        {
            DCOP_Free(pbyRec);
            DCOP_Free(pRspPara);
            DCOP_Free(pRspData);
            return FAILURE;
        }

        /// 获取记录
        DWORD dwRc = GetRecord(dwRecNo, pbyRec);
        if (dwRc)
        {
            DCOP_Free(pbyRec);
            DCOP_Free(pRspPara);
            DCOP_Free(pRspData);
            return FAILURE;
        }

        /// 拷贝记录
        dwRc = CopyRecord(pFields, dwFieldCount, 
                        pbyRec, GetRecSize(), 
                        pRspPara, dwRspParaCount, 
                        pRspData, dwRspDataLen);
        DCOP_Free(pbyRec);
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
    for (DWORD dwRecNo = 1; dwRecNo <= m_head.m_totalCount; ++dwRecNo)
    {
        /// 获取当前记录
        DWORD dwRc = GetRecord(dwRecNo, pbyRec);
        if (dwRc) continue;

        /// 是否匹配当前记录
        bool bRc = bMatchRecord(pFields, dwFieldCount, 
                        pbyRec, byCond, 
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
        dwRc = CopyRecord(pFields, dwFieldCount, 
                        pbyRec, GetRecSize(), 
                        pRspPara, dwRspParaCount, 
                        pRspData, dwRspDataLen);
        if (dwRc) continue;

        (void)aRspData.Append(pRspData);
    }

    DCOP_Free(pbyRec);
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
  函 数 名: CDataFile::CountRecord
  描    述: 统计记录数量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataFile::CountRecord(BYTE byCond, 
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
        /// 查找记录
        CRecIdx::CIdx idx = m_recIdx.FindRec(pCondPara, dwCondParaCount, pCondData, dwCondDataLen);
        DWORD dwRecNo = (DWORD)idx;
        rdwCount = (!dwRecNo)? 0 : 1;
        return SUCCESS;
    }

    /// 准备复制记录的空间
    BYTE *pbyRec = (BYTE *)DCOP_Malloc(GetRecSize());
    if (!pbyRec)
    {
        return FAILURE;
    }

    /// 模糊匹配查找(遍历进行匹配)
    rdwCount = 0;
    for (DWORD dwRecNo = 1; dwRecNo <= m_head.m_totalCount; ++dwRecNo)
    {
        /// 获取当前记录
        DWORD dwRc = GetRecord(dwRecNo, pbyRec);
        if (dwRc) continue;

        /// 是否匹配当前记录
        bool bRc = bMatchRecord(pFields, dwFieldCount, 
                        pbyRec, byCond, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen);
        if (!bRc) continue;

        rdwCount++;
    }

    DCOP_Free(pbyRec);
    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataFile::AddKeyIdx
  描    述: 添加关键字索引
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataFile::AddKeyIdx(DCOP_PARA_NODE *pIdxPara, DWORD dwIdxParaCount)
{
    IObject *pObject = GetOwner();
    if (!pObject)
    {
        return FAILURE;
    }

    AutoObjLock(pObject);

    /// 先添加关键字
    DWORD dwRc = m_recIdx.AddKey(pIdxPara, dwIdxParaCount);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 再构建索引
    BYTE *pbyRec = (BYTE *)DCOP_Malloc(GetRecSize());
    if (!pbyRec) return FAILURE;
    for (DWORD dwRecNo = 1; dwRecNo <= m_head.m_totalCount; ++dwRecNo)
    {
        /// 获取当前记录
        dwRc = GetRecord(dwRecNo, pbyRec);
        if (dwRc != SUCCESS)
        {
            dwRc = SUCCESS;
            continue;
        }

        /// 构建添加的索引
        CRecIdx::CIdx idx(dwRecNo);
        dwRc = m_recIdx.BldKeyIdx(pIdxPara, dwIdxParaCount, GetFields(), pbyRec, idx);
        if (dwRc != SUCCESS)
        {
            (void)m_recIdx.DelKey(pIdxPara, dwIdxParaCount);
            dwRc = FAILURE;
            break;
        }
    }

    DCOP_Free(pbyRec);
    return dwRc;
}

/*******************************************************
  函 数 名: CDataFile::DelKeyIdx
  描    述: 删除关键字索引
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataFile::DelKeyIdx(DCOP_PARA_NODE *pIdxPara, DWORD dwIdxParaCount)
{
    IObject *pObject = GetOwner();
    if (!pObject)
    {
        return FAILURE;
    }

    AutoObjLock(pObject);

    return m_recIdx.DelKey(pIdxPara, dwIdxParaCount);
}

/*******************************************************
  函 数 名: CDataFile::SaveCurIdentify
  描    述: 保存当前ID
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDataFile::SaveCurIdentify(DWORD curIdentify)
{
    /// 更新文件头
    m_head.m_curIdentify = curIdentify;
    /// 写入前转换文件头的字节序(因为文件头在本地还要使用，所以拷贝一份)
    FileHead headTmp;
    (void)memcpy(&headTmp, &m_head, sizeof(m_head));
    BytesChangeFileHead(&headTmp);
    (void)DCOP_SaveToFile(m_fileName, &headTmp, sizeof(headTmp), 0);
}

/*******************************************************
  函 数 名: CDataFile::AddRecord
  描    述: 添加记录(写入整条记录)
  输    入: 
  输    出: 
  返    回: 添加成功时返回记录号(>=1), 否则返回0
  修改记录: 
 *******************************************************/
DWORD CDataFile::AddRecord(BYTE *pbyRec)
{
    if (!pbyRec)
    {
        return 0;
    }

    /// 先触发添加索引
    DWORD dwRecIdx = GetIdleRec();
    CRecIdx::CIdx idx(dwRecIdx + 1);
    DWORD dwRc = m_recIdx.OnAddRec(GetFields(), pbyRec, idx);
    if (dwRc)
    {
        return 0;
    }

    /// 复制记录并置有效标识
    BYTE *pRecord = (BYTE *)DCOP_Malloc(1 + GetRecSize());
    if (!pRecord)
    {
        m_recIdx.OnDelRec(GetFields(), pbyRec);
        return 0;
    }
    *pRecord = REC_FLAG_VALID;
    (void)memcpy(pRecord + 1, pbyRec, GetRecSize());

    /// 把整条记录更新到文件中
    DWORD dwPosIdx = sizeof(FileHead) + sizeof(IModel::Field) * m_head.m_fieldCount;
    dwRc = DCOP_SaveToFile(m_fileName, pRecord, 1 + GetRecSize(), 
                        dwPosIdx + dwRecIdx * (1 + GetRecSize()));
    DCOP_Free(pRecord);
    if (dwRc)
    {
        m_recIdx.OnDelRec(GetFields(), pbyRec);
        return 0;
    }

    /// 更新文件头
    m_head.m_recordCount++;
    if (dwRecIdx >= m_head.m_totalCount) m_head.m_totalCount++;
    /// 写入前转换文件头的字节序(因为文件头在本地还要使用，所以拷贝一份)
    FileHead headTmp;
    (void)memcpy(&headTmp, &m_head, sizeof(m_head));
    BytesChangeFileHead(&headTmp);
    (void)DCOP_SaveToFile(m_fileName, &headTmp, sizeof(headTmp), 0);

    return dwRecIdx + 1;
}

/*******************************************************
  函 数 名: CDataFile::DelRecord
  描    述: 删除记录(只是把标识置为无效)
  输    入: 
  输    出: 
  返    回: 成功或者失败的错误码
  修改记录: 
 *******************************************************/
DWORD CDataFile::DelRecord(DWORD dwRecNo)
{
    if (!dwRecNo || (dwRecNo > m_head.m_totalCount))
    {
        return FAILURE;
    }

    /// 先获取记录并看标识是否已经无效
    BYTE *pRecord = (BYTE *)DCOP_Malloc(1 + GetRecSize());
    if (!pRecord) return FAILURE;
    DWORD dwPosIdx = sizeof(FileHead) + sizeof(IModel::Field) * m_head.m_fieldCount;
    DWORD dwRecIdx = dwRecNo - 1;
    DWORD dwRc = DCOP_RestoreFromFile(m_fileName, pRecord, 1 + GetRecSize(), 
                        dwPosIdx + dwRecIdx * (1 + GetRecSize()));
    if (dwRc || !(*pRecord))
    {
        DCOP_Free(pRecord);
        return FAILURE;
    }

    /// 把记录的标识置为无效
    *pRecord = REC_FLAG_NULL;
    dwRc = DCOP_SaveToFile(m_fileName, pRecord, 1, 
                        dwPosIdx + dwRecIdx * (1 + GetRecSize()));
    if (dwRc)
    {
        DCOP_Free(pRecord);
        return dwRc;
    }

    /// 更新文件头
    if (m_head.m_recordCount) m_head.m_recordCount--;
    /// 写入前转换文件头的字节序(因为文件头在本地还要使用，所以拷贝一份)
    FileHead headTmp;
    (void)memcpy(&headTmp, &m_head, sizeof(m_head));
    BytesChangeFileHead(&headTmp);
    (void)DCOP_SaveToFile(m_fileName, &headTmp, sizeof(headTmp), 0);

    /// 触发删除索引
    m_recIdx.OnDelRec(GetFields(), pRecord + 1);
    DCOP_Free(pRecord);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataFile::SetRecord
  描    述: 设置记录(替换整条记录)[记录必须有效]
  输    入: 
  输    出: 
  返    回: 成功或者失败的错误码
  修改记录: 
 *******************************************************/
DWORD CDataFile::SetRecord(DWORD dwRecNo, BYTE *pbyRec)
{
    if (!dwRecNo || (dwRecNo > m_head.m_totalCount) || (!pbyRec))
    {
        return FAILURE;
    }

    /// 先判断标识是否已经无效
    BYTE byFlag = REC_FLAG_NULL;
    DWORD dwPosIdx = sizeof(FileHead) + sizeof(IModel::Field) * m_head.m_fieldCount;
    DWORD dwRecIdx = dwRecNo - 1;
    DWORD dwRc = DCOP_RestoreFromFile(m_fileName, &byFlag, 1, 
                        dwPosIdx + dwRecIdx * (1 + GetRecSize()));
    if (dwRc || !byFlag)
    {
        return FAILURE;
    }

    /// 复制记录并置有效标识
    BYTE *pRecord = (BYTE *)DCOP_Malloc(1 + GetRecSize());
    if (!pRecord) return FAILURE;
    *pRecord = REC_FLAG_VALID;
    (void)memcpy(pRecord + 1, pbyRec, GetRecSize());

    /// 把整条记录更新到文件中
    dwRc = DCOP_SaveToFile(m_fileName, pRecord, 1 + GetRecSize(), 
                        dwPosIdx + dwRecIdx * (1 + GetRecSize()));
    DCOP_Free(pRecord);

    return dwRc;
}

/*******************************************************
  函 数 名: CDataFile::GetRecord
  描    述: 获取记录(复制整条记录)
  输    入: 
  输    出: 
  返    回: 成功或者失败的错误码
  修改记录: 
 *******************************************************/
DWORD CDataFile::GetRecord(DWORD dwRecNo, BYTE *pbyRec)
{
    if (!dwRecNo || (dwRecNo > m_head.m_totalCount) || (!pbyRec))
    {
        return FAILURE;
    }

    /// 获取整条记录
    BYTE *pRecord = (BYTE *)DCOP_Malloc(1 + GetRecSize());
    if (!pRecord) return FAILURE;
    DWORD dwPosIdx = sizeof(FileHead) + sizeof(IModel::Field) * m_head.m_fieldCount;
    DWORD dwRecIdx = dwRecNo - 1;
    DWORD dwRc = DCOP_RestoreFromFile(m_fileName, pRecord, 1 + GetRecSize(), 
                        dwPosIdx + dwRecIdx * (1 + GetRecSize()));
    if (dwRc)
    {
        DCOP_Free(pRecord);
        return FAILURE;
    }

    /// 如果记录标识无效, 返回错误
    if (!(*pRecord))
    {
        DCOP_Free(pRecord);
        return FAILURE;
    }

    /// 复制获取到的记录
    (void)memcpy(pbyRec, pRecord + 1, GetRecSize());
    DCOP_Free(pRecord);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataFile::GetIdleRec
  描    述: 找到一个空闲记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataFile::GetIdleRec()
{
    DWORD dwRecIdx = m_head.m_totalCount;
    DWORD dwPosIdx = sizeof(FileHead) + sizeof(IModel::Field) * m_head.m_fieldCount;
    for (DWORD i = 0; i < m_head.m_totalCount; ++i)
    {
        BYTE byFlag = REC_FLAG_NULL;
        DWORD dwRc = DCOP_RestoreFromFile(m_fileName, &byFlag, 1, dwPosIdx);
        if (dwRc)
        {
            break;
        }

        if (!byFlag)
        {
            dwRecIdx = i;
            break;
        }

        dwPosIdx += 1 + GetRecSize();
    }

    return dwRecIdx;
}

/*******************************************************
  函 数 名: CDataFile::BldAllIdx
  描    述: 构建所有索引
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataFile::BldAllIdx()
{
    Field *pFields = GetFields();
    DWORD dwFieldCount = GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    /// 添加关键字索引
    for (DWORD i = 0; i < dwFieldCount; ++i)
    {
        if (!pFields[i].m_isKey) continue;

        DCOP_PARA_NODE keyPara = 
        {
            i + 1, // 字段ID为"索引值+1"
            DCOP_OPCODE_NONE,
            pFields[i].m_fieldType,
            pFields[i].m_fieldSize
        };
        (void)m_recIdx.AddKey(&keyPara, 1);
    }

    /// 没有记录，就不用构建索引
    if (!m_head.m_totalCount)
    {
        return SUCCESS;
    }

    /// 为获取记录申请空间
    BYTE *pbyRec = (BYTE *)DCOP_Malloc(GetRecSize());
    if (!pbyRec)
    {
        return FAILURE;
    }

    /// 遍历记录添加记录索引
    for (DWORD dwRecNo = 1; dwRecNo <= m_head.m_totalCount; ++dwRecNo)
    {
        /// 获取当前记录
        DWORD dwRc = GetRecord(dwRecNo, pbyRec);
        if (dwRc) continue;

        CRecIdx::CIdx idx(dwRecNo);
        dwRc = m_recIdx.OnAddRec(pFields, pbyRec, idx);
        if (dwRc)
        {
            DCOP_Free(pbyRec);
            return FAILURE;
        }
    }

    DCOP_Free(pbyRec);
    return SUCCESS;
}

