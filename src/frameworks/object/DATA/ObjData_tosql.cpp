/// -------------------------------------------------
/// ObjData_tosql.h : 将TLV转换为SQL语句私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjData_tosql.h"
#include "Factory_if.h"
#include "ObjTimer_if.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CTlvToSQL, "TLV2SQL")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CTlvToSQL)
    DCOP_IMPLEMENT_INTERFACE(ITlvToSQL)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 参数操作码类型定义(和DCOP_OPCODE_TYPE一一对应)
/// -------------------------------------------------
const char *CTlvToSQL::ArgOpCode[] = 
{
    "",
    "+",
    "-",
    "*",
    "/",
    "=",
    ">=",
    ">",
    "<",
    "<=",
    "!=",
    "LIKE",
    "NOT LIKE",
};


/*******************************************************
  函 数 名: CTlvToSQL::CTlvToSQL
  描    述: CTlvToSQL构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CTlvToSQL::CTlvToSQL(Instance *piParent, int argc, char **argv)
{
    /// 绝对子对象，不能引用父对象，而且子对象无生命周期释放引用，父对象又因为引用计数大于1而无法走到析构
    DCOP_CONSTRUCT_INSTANCE(NULL);

    m_piDataHandle = (piParent)? (IDataHandle *)piParent : NULL;
}

/*******************************************************
  函 数 名: CTlvToSQL::~CTlvToSQL
  描    述: CTlvToSQL析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CTlvToSQL::~CTlvToSQL()
{
    m_piDataHandle = NULL;

    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CTlvToSQL::GetCreateFieldList
  描    述: 获取创建参数列表
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTlvToSQL::GetCreateFieldList(CDString &strArgList)
{
    if (!m_piDataHandle) return FAILURE;

    IDataHandle::Field *pFields = m_piDataHandle->GetFields();
    DWORD dwFieldCount = m_piDataHandle->GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    bool bFirstKey = true;
    for (DWORD i = 0; i < dwFieldCount; ++i)
    {
        switch (pFields[i].m_fieldType)
        {
            case IModel::FIELD_IDENTIFY:
                strArgList << STR_FORMAT("%s SERIAL", pFields[i].m_fieldName);
                break;
            case IModel::FIELD_BYTE:
                strArgList << STR_FORMAT("%s TINYINT UNSIGNED DEFAULT %d", pFields[i].m_fieldName, pFields[i].m_defaultValue);
                break;
            case IModel::FIELD_WORD:
                strArgList << STR_FORMAT("%s SMALLINT UNSIGNED DEFAULT %d", pFields[i].m_fieldName, pFields[i].m_defaultValue);
                break;
            case IModel::FIELD_DWORD:
                strArgList << STR_FORMAT("%s INT UNSIGNED DEFAULT %d", pFields[i].m_fieldName, pFields[i].m_defaultValue);
                break;
            case IModel::FIELD_SHORT:
                strArgList << STR_FORMAT("%s SMALLINT DEFAULT %d", pFields[i].m_fieldName, pFields[i].m_defaultValue);
                break;
            case IModel::FIELD_INTEGER:
                strArgList << STR_FORMAT("%s INT DEFAULT %d", pFields[i].m_fieldName, pFields[i].m_defaultValue);
                break;
            case IModel::FIELD_NUMBER:
                strArgList << STR_FORMAT("%s BIGINT UNSIGNED DEFAULT %d", pFields[i].m_fieldName, pFields[i].m_defaultValue);
                break;
            case IModel::FIELD_BOOL:
                strArgList << STR_FORMAT("%s TINYINT", pFields[i].m_fieldName);
                break;
            case IModel::FIELD_CHAR:
                strArgList << STR_FORMAT("%s CHAR(%d)", pFields[i].m_fieldName, pFields[i].m_fieldSize);
                break;
            case IModel::FIELD_STRING:
                strArgList << STR_FORMAT("%s VARCHAR(%d)", pFields[i].m_fieldName, pFields[i].m_fieldSize);
                break;
            case IModel::FIELD_BUFFER:
                strArgList << STR_FORMAT("%s VARBINARY(%d)", pFields[i].m_fieldName, pFields[i].m_fieldSize);
                break;
            case IModel::FIELD_DATE:
                strArgList << STR_FORMAT("%s DATE", pFields[i].m_fieldName);
                break;
            case IModel::FIELD_TIME:
                strArgList << STR_FORMAT("%s TIME", pFields[i].m_fieldName);
                break;
            case IModel::FIELD_IP:
                strArgList << STR_FORMAT("%s INT UNSIGNED DEFAULT 0", pFields[i].m_fieldName);
                break;
            case IModel::FIELD_PTR:
                strArgList << STR_FORMAT("%s VARBINARY(%d) DEFAULT 0x0", pFields[i].m_fieldName, sizeof(void *));
                break;
            case IModel::FIELD_TIMER:
                strArgList << STR_FORMAT("%s VARBINARY(%d) DEFAULT 0x0", pFields[i].m_fieldName, sizeof(ITimer::Node));
                break;
            case IModel::FIELD_PASS:
                strArgList << STR_FORMAT("%s VARBINARY(%d)", pFields[i].m_fieldName, pFields[i].m_fieldSize);
            default:
                break;
        }

        /// 添加关键字标识
        if (pFields[i].m_isKey)
        {
            if (bFirstKey) strArgList << " PRIMARY";
            strArgList << " KEY";
        }

        /// 字段间间隔
        if (i != (dwFieldCount - 1))
        {
            strArgList << ", ";
        }
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CTlvToSQL::GetInsertFieldList
  描    述: 获取插入参数列表
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTlvToSQL::GetInsertFieldList(DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
                        void *pReqData, DWORD dwReqDataLen, 
                        CDString &strArgList)
{
    if (!pReqPara || !dwReqParaCount || 
        !pReqData || !dwReqDataLen)
    {
        return FAILURE;
    }

    if (!m_piDataHandle) return FAILURE;

    IDataHandle::Field *pFields = m_piDataHandle->GetFields();
    DWORD dwFieldCount = m_piDataHandle->GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    CDString strNameList;
    CDString strValueList;
    DWORD dwRc = SUCCESS;
    DWORD dwDataPos = 0;
    for (DWORD dwParaIdx = 0; dwParaIdx < dwReqParaCount; ++dwParaIdx)
    {
        /// 需要判断参数ID的正确性
        if (!pReqPara[dwParaIdx].m_paraID || (pReqPara[dwParaIdx].m_paraID > dwFieldCount))
        {
            dwRc = FAILURE;
            break;
        }

        /// 获取字段索引
        DWORD dwFieldIdx = pReqPara[dwParaIdx].m_paraID - 1;

        /// 自动生成的IDentify不能被覆盖
        if (m_piDataHandle->IsIdentify(pFields[dwFieldIdx].m_fieldType))
        {
            /// 计算偏移
            dwDataPos += pReqPara[dwParaIdx].m_paraSize;
            if (dwDataPos > dwReqDataLen)
            {
                dwRc = FAILURE;
                break;
            }

            continue;
        }

        strNameList << STR_FORMAT("%s, ", pFields[dwFieldIdx].m_fieldName);

        /// 判断数字的大小范围
        if ((m_piDataHandle->IsDigital(pFields[dwFieldIdx].m_fieldType)) || 
            (IModel::FIELD_IP == pFields[dwFieldIdx].m_fieldType))
        {
            DWORD dwValue = Bytes_GetDwordValue((BYTE *)pReqData + dwDataPos, pReqPara[dwParaIdx].m_paraSize);
            DWORD dwMinValue = pFields[dwFieldIdx].m_minValue;
            DWORD dwMaxValue = pFields[dwFieldIdx].m_maxValue;
            if ((dwMinValue != dwMaxValue) && ((dwValue < dwMinValue) || (dwValue > dwMaxValue)))
            {
                dwRc = FAILURE;
                break;
            }

            strValueList << STR_FORMAT("%d, ", dwValue);
        }
        else if (IModel::FIELD_CHAR == pFields[dwFieldIdx].m_fieldType)
        {
            strValueList << STR_FORMAT("'%c', ", *((char *)pReqData + dwDataPos));
        }
        else if (IModel::FIELD_STRING == pFields[dwFieldIdx].m_fieldType)
        {
            CDString strTmp((char *)pReqData + dwDataPos, pReqPara[dwParaIdx].m_paraSize);
            strValueList << STR_FORMAT("'%s', ", (const char *)strTmp);
        }
        else if ((IModel::FIELD_BUFFER == pFields[dwFieldIdx].m_fieldType) || 
            (IModel::FIELD_PTR == pFields[dwFieldIdx].m_fieldType) || 
            (IModel::FIELD_TIMER == pFields[dwFieldIdx].m_fieldType) || 
            (IModel::FIELD_PASS == pFields[dwFieldIdx].m_fieldType))
        {
            strValueList << "0x" << CBufferString((BYTE *)pReqData + dwDataPos, 
                                    pReqPara[dwParaIdx].m_paraSize) << ", ";
        }
        else
        {
        }

        /// 计算偏移
        dwDataPos += pReqPara[dwParaIdx].m_paraSize;
        if (dwDataPos > dwReqDataLen)
        {
            dwRc = FAILURE;
            break;
        }
    }

    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    strNameList.TrimRight(", ");
    strValueList.TrimRight(", ");

    strArgList << STR_FORMAT("(%s) ", (const char *)strNameList) 
               << STR_FORMAT("VALUES (%s)", (const char *)strValueList);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CTlvToSQL::GetCondFieldList
  描    述: 获取条件参数列表
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTlvToSQL::GetCondFieldList(BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        CDString &strArgList)
{
    if (!pCondPara && !dwCondParaCount && !pCondData && !dwCondDataLen)
    {
        return SUCCESS;
    }

    if (!pCondPara || !dwCondParaCount || !pCondData || !dwCondDataLen)
    {
        return FAILURE;
    }

    if (!m_piDataHandle) return FAILURE;

    IDataHandle::Field *pFields = m_piDataHandle->GetFields();
    DWORD dwFieldCount = m_piDataHandle->GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    DWORD dwRc = SUCCESS;
    DWORD dwDataPos = 0;
    for (DWORD dwParaIdx = 0; dwParaIdx < dwCondParaCount; ++dwParaIdx)
    {
        /// 需要判断参数ID的正确性
        if (!pCondPara[dwParaIdx].m_paraID || (pCondPara[dwParaIdx].m_paraID > dwFieldCount))
        {
            continue;
        }

        /// 获取字段索引
        DWORD dwFieldIdx = pCondPara[dwParaIdx].m_paraID - 1;

        /// 获取条件操作码
        dwRc  = GetCondArgOpCode(byCond, 
                        pCondPara[dwParaIdx].m_opCode, 
                        pCondPara[dwParaIdx].m_paraType, 
                        pFields[dwFieldIdx].m_fieldName, 
                        strArgList);
        if (dwRc != SUCCESS)
        {
            break;
        }

        /// 获取各个参数值
        if ((m_piDataHandle->IsDigital(pFields[dwFieldIdx].m_fieldType)) || 
            (IModel::FIELD_IP == pFields[dwFieldIdx].m_fieldType))
        {
            DWORD dwValue = Bytes_GetDwordValue((BYTE *)pCondData + dwDataPos, pCondPara[dwParaIdx].m_paraSize);
            strArgList << STR_FORMAT("%d ", dwValue);
        }
        else if (IModel::FIELD_BOOL == pFields[dwFieldIdx].m_fieldType)
        {
            strArgList << ((*((BYTE *)pCondData + dwDataPos))? "TRUE" : "FALSE");
        }
        else if (IModel::FIELD_CHAR == pFields[dwFieldIdx].m_fieldType)
        {
            strArgList << STR_FORMAT("'%c' ", *((char *)pCondData + dwDataPos));
        }
        else if (IModel::FIELD_STRING == pFields[dwFieldIdx].m_fieldType)
        {
            CDString strTmp((char *)pCondData + dwDataPos, pCondPara[dwParaIdx].m_paraSize);
            strArgList << STR_FORMAT("'%s' ", (const char *)strTmp);
        }
        else if ((IModel::FIELD_BUFFER == pFields[dwFieldIdx].m_fieldType) || 
            (IModel::FIELD_PTR == pFields[dwFieldIdx].m_fieldType) || 
            (IModel::FIELD_TIMER == pFields[dwFieldIdx].m_fieldType) || 
            (IModel::FIELD_PASS == pFields[dwFieldIdx].m_fieldType))
        {
            strArgList << "0x" << CBufferString((BYTE *)pCondData + dwDataPos, 
                                  pCondPara[dwParaIdx].m_paraSize) << " ";
        }
        else
        {
        }

        /// 计算偏移
        dwDataPos += pCondPara[dwParaIdx].m_paraSize;
        if (dwDataPos > dwCondDataLen)
        {
            dwRc = FAILURE;
            break;
        }

        /// 字段间间隔
        if (dwParaIdx != (dwCondParaCount - 1))
        {
            strArgList << ((byCond == DCOP_CONDITION_ANY)? "OR " : "AND ");
        }
    }

    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    strArgList.TrimRight(" ");

    return SUCCESS;
}

/*******************************************************
  函 数 名: CTlvToSQL::GetReqFieldList
  描    述: 获取请求参数列表
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTlvToSQL::GetReqFieldList(DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
                        void *pReqData, DWORD dwReqDataLen, 
                        CDString &strArgList)
{
    if (!pReqPara || !dwReqParaCount || 
        !pReqData || !dwReqDataLen)
    {
        return FAILURE;
    }

    if (!m_piDataHandle) return FAILURE;

    IDataHandle::Field *pFields = m_piDataHandle->GetFields();
    DWORD dwFieldCount = m_piDataHandle->GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    DWORD dwRc = SUCCESS;
    DWORD dwDataPos = 0;
    for (DWORD dwParaIdx = 0; dwParaIdx < dwReqParaCount; ++dwParaIdx)
    {
        /// 需要判断参数ID的正确性
        if (!pReqPara[dwParaIdx].m_paraID || (pReqPara[dwParaIdx].m_paraID > dwFieldCount))
        {
            dwRc = FAILURE;
            break;
        }

        /// 获取字段索引
        DWORD dwFieldIdx = pReqPara[dwParaIdx].m_paraID - 1;

        /// 获取请求操作码
        dwRc  = GetReqArgOpCode(pReqPara[dwParaIdx].m_opCode, 
                        pReqPara[dwParaIdx].m_paraType, 
                        pFields[dwFieldIdx].m_fieldName, 
                        strArgList);
        if (dwRc != SUCCESS)
        {
            break;
        }

        /// 获取各个参数值
        if ((m_piDataHandle->IsDigital(pFields[dwFieldIdx].m_fieldType)) || 
            (IModel::FIELD_IP == pFields[dwFieldIdx].m_fieldType))
        {
            DWORD dwValue = Bytes_GetDwordValue((BYTE *)pReqData + dwDataPos, pReqPara[dwParaIdx].m_paraSize);
            strArgList << STR_FORMAT("%d, ", dwValue);
        }
        else if (IModel::FIELD_BOOL == pFields[dwFieldIdx].m_fieldType)
        {
            strArgList << ((*((BYTE *)pReqData + dwDataPos))? "1" : "0");
        }
        else if (IModel::FIELD_CHAR == pFields[dwFieldIdx].m_fieldType)
        {
            strArgList << STR_FORMAT("'%c', ", *((char *)pReqData + dwDataPos));
        }
        else if (IModel::FIELD_STRING == pFields[dwFieldIdx].m_fieldType)
        {
            CDString strTmp((char *)pReqData + dwDataPos, pReqPara[dwParaIdx].m_paraSize);
            strArgList << STR_FORMAT("'%s', ", (const char *)strTmp);
        }
        else if ((IModel::FIELD_BUFFER == pFields[dwFieldIdx].m_fieldType) || 
            (IModel::FIELD_PTR == pFields[dwFieldIdx].m_fieldType) || 
            (IModel::FIELD_TIMER == pFields[dwFieldIdx].m_fieldType) || 
            (IModel::FIELD_PASS == pFields[dwFieldIdx].m_fieldType))
        {
            strArgList << "0x" << CBufferString((BYTE *)pReqData + dwDataPos, 
                                  pReqPara[dwParaIdx].m_paraSize) << ", ";
        }
        else
        {
        }

        /// 计算偏移
        dwDataPos += pReqPara[dwParaIdx].m_paraSize;
        if (dwDataPos > dwReqDataLen)
        {
            dwRc = FAILURE;
            break;
        }
    }

    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    strArgList.TrimRight(", ");

    return SUCCESS;
}

/*******************************************************
  函 数 名: CTlvToSQL::GetFieldNameList
  描    述: 获取参数名字列表
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTlvToSQL::GetFieldNameList(DCOP_PARA_NODE *pPara, 
                        DWORD dwParaCount, 
                        CDString &strArgList, 
                        const char *cszSplit, 
                        CDString *pStrJoinTable)
{
    if (!pPara || !dwParaCount)
    {
        strArgList << "*";
        return SUCCESS;
    }

    if (!m_piDataHandle) return FAILURE;

    IDataHandle::Field *pFields = m_piDataHandle->GetFields();
    DWORD dwFieldCount = m_piDataHandle->GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    DWORD dwRc = SUCCESS;
    for (DWORD dwParaIdx = 0; dwParaIdx < dwParaCount; ++dwParaIdx)
    {
        /// 获取关联类型的特殊参数
        if (DCOP_SPECPARA(pPara[dwParaIdx].m_paraID, DCOP_FIELD_RELATION) && pStrJoinTable)
        {
            dwRc = GetJoinTable(pPara[dwParaIdx].m_paraID, strArgList, *pStrJoinTable);
            if (dwRc != SUCCESS)
            {
                break;
            }

            strArgList << cszSplit;
            continue;
        }

        /// 需要判断参数ID的正确性
        if (!pPara[dwParaIdx].m_paraID || (pPara[dwParaIdx].m_paraID > dwFieldCount))
        {
            dwRc = FAILURE;
            break;
        }

        /// 获取字段索引
        DWORD dwFieldIdx = pPara[dwParaIdx].m_paraID - 1;

        /// 获取名字
        strArgList << pFields[dwFieldIdx].m_fieldName << cszSplit;
    }

    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    strArgList.TrimRight(cszSplit);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CTlvToSQL::GetRspFieldList
  描    述: 获取响应参数列表
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTlvToSQL::GetRspFieldList(DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        MYSQL_RES *res, 
                        DCOP_PARA_NODE *&rpRspPara, 
                        DWORD &rdwRspParaCount, 
                        CDArray &aRspData)
{
    if (dwReqParaCount)
    {
        if (!pReqPara) return FAILURE;
    }

    if (!m_piDataHandle) return FAILURE;

    /// 获取字段信息
    IDataHandle::Field *pFields = m_piDataHandle->GetFields();
    DWORD dwFieldCount = m_piDataHandle->GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    /// 获取响应参数
    DWORD dwRspParaCount = 0;
    DWORD dwRspDataLen = 0;
    DCOP_PARA_NODE *pRspPara = m_piDataHandle->GetOutPara(pFields, dwFieldCount, dwRspParaCount, 
                        pReqPara, dwReqParaCount, &dwRspDataLen);
    if (!pRspPara)
    {
        return FAILURE;
    }

    /// 响应数据长度错误
    if ((dwRspParaCount != (DWORD)mysql_num_fields(res)) || !dwRspDataLen)
    {
        DCOP_Free(pRspPara);
        return FAILURE;
    }

    void *pRspData = DCOP_Malloc(dwRspDataLen);
    if (!pRspData)
    {
        DCOP_Free(pRspPara);
        return FAILURE;
    }

    (void)memset(pRspData, 0, dwRspDataLen);
    aRspData.SetNodeSize(dwRspDataLen);
    aRspData.SetMaxCount((DWORD)mysql_num_rows(res));

    DWORD dwRc = SUCCESS;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        unsigned long *lens = mysql_fetch_lengths(res);
        dwRc = GetRspArgValue(pRspPara, dwRspParaCount, row, lens, pRspData, dwRspDataLen);
        if (dwRc != SUCCESS)
        {
            break;
        }

        (void)aRspData.Append(pRspData);
    }

    DCOP_Free(pRspData);
    if (!aRspData.Count())
    {
        DCOP_Free(pRspPara);
        return (dwRc) ? dwRc : FAILURE;
    }

    rpRspPara = pRspPara;
    rdwRspParaCount = dwRspParaCount;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CTlvToSQL::GetIdentifyName
  描    述: 获取Identify字段名称
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTlvToSQL::GetIdentifyName(CDString &strIdName)
{
    if (!m_piDataHandle) return FAILURE;

    IDataHandle::Field *pFields = m_piDataHandle->GetFields();
    DWORD dwFieldCount = m_piDataHandle->GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    for (DWORD i = 0; i < dwFieldCount; ++i)
    {
        /// 添加关键字标识
        if (pFields[i].m_fieldType == IModel::FIELD_IDENTIFY)
        {
            strIdName << pFields[i].m_fieldName;
            return SUCCESS;
        }
    }

    return FAILURE;
}

/*******************************************************
  函 数 名: CTlvToSQL::GetCondArgOpCode
  描    述: 获取条件操作码
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTlvToSQL::GetCondArgOpCode(BYTE cond, BYTE opCode, BYTE paraType, const char *fieldName, CDString &strArgList)
{
    if (cond == DCOP_CONDITION_ONE)
    {
        strArgList << fieldName << "=";
        return SUCCESS;
    }

    if (opCode >= ARRAY_SIZE(ArgOpCode))
    {
        return FAILURE;
    }

    /// 等号前面的都是运算操作符，等号后面的(含等号)才是逻辑操作符
    if (opCode < DCOP_OPCODE_EQUAL)
    {
        return FAILURE;
    }

    const char *pcszArgOpCode = ArgOpCode[opCode];

    /// 特殊处理参数类型
    if (paraType == IModel::FIELD_BOOL)
    {
        pcszArgOpCode = "IS";
    }

    strArgList << fieldName << pcszArgOpCode;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CTlvToSQL::GetReqArgOpCode
  描    述: 获取请求操作码
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTlvToSQL::GetReqArgOpCode(BYTE opCode, BYTE paraType, const char *fieldName, CDString &strArgList)
{
    if (opCode >= ARRAY_SIZE(ArgOpCode))
    {
        return FAILURE;
    }

    /// 等号后面的都是逻辑操作符，等号(含等号)前面的才是运算操作符
    if (opCode > DCOP_OPCODE_EQUAL)
    {
        return FAILURE;
    }

    const char *pcszArgOpCode = ArgOpCode[opCode];

    /// 特殊处理参数类型
    if (opCode == DCOP_OPCODE_EQUAL)
    {
        strArgList << fieldName << pcszArgOpCode;
        return SUCCESS;
    }

    strArgList << fieldName << "=" << fieldName << pcszArgOpCode;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CTlvToSQL::GetRspArgValue
  描    述: 获取响应参数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTlvToSQL::GetRspArgValue(DCOP_PARA_NODE *pRspPara, DWORD dwRspParaCount, 
                        MYSQL_ROW row, unsigned long *lens, 
                        void *pRspData, DWORD dwRspDataLen)
{
    DWORD dwRc = SUCCESS;
    DWORD dwDataPos = 0;
    for (DWORD dwParaIdx = 0; dwParaIdx < dwRspParaCount; ++dwParaIdx)
    {
        if (!(row[dwParaIdx]))
        {
            (void)memset((char *)pRspData + dwDataPos, 0, pRspPara[dwParaIdx].m_paraSize);
            dwDataPos += pRspPara[dwParaIdx].m_paraSize;
            if (dwDataPos > dwRspDataLen)
            {
                dwRc = FAILURE;
                break;
            }

            continue;
        }

        /// 获取各个参数值
        if ((m_piDataHandle->IsDigital(pRspPara[dwParaIdx].m_paraType)) || 
            (IModel::FIELD_IP == pRspPara[dwParaIdx].m_paraType))
        {
            Bytes_SetDwordValue((DWORD)atoi(row[dwParaIdx]), (BYTE *)pRspData + dwDataPos, pRspPara[dwParaIdx].m_paraSize);
        }
        else if (IModel::FIELD_BOOL == pRspPara[dwParaIdx].m_paraType)
        {
            *((BYTE *)pRspData + dwDataPos) = (atoi(row[dwParaIdx])? 1 : 0);
        }
        else if (IModel::FIELD_CHAR == pRspPara[dwParaIdx].m_paraType)
        {
            *((char *)pRspData + dwDataPos) = *(row[dwParaIdx]);
        }
        else if (IModel::FIELD_STRING == pRspPara[dwParaIdx].m_paraType)
        {
            (void)strncpy((char *)pRspData + dwDataPos, row[dwParaIdx], pRspPara[dwParaIdx].m_paraSize);
        }
        else if ((IModel::FIELD_BUFFER == pRspPara[dwParaIdx].m_paraType) || 
            (IModel::FIELD_PTR == pRspPara[dwParaIdx].m_paraType) || 
            (IModel::FIELD_TIMER == pRspPara[dwParaIdx].m_paraType) || 
            (IModel::FIELD_PASS == pRspPara[dwParaIdx].m_paraType))
        {
            /// 对于二进制类型，这里查询到的直接是二进制数据，和插入不同:
            /// 插入时，二进制类型需要转换成字符形式的: 0x01020abcc
            /// 所以查询之后的二进制数据直接拷贝，不需要转换成二进制类型.
            DWORD dwCopyDataLen = MIN((DWORD)(pRspPara[dwParaIdx].m_paraSize), (DWORD)(lens[dwParaIdx]));
            if (dwCopyDataLen) (void)memcpy((BYTE *)pRspData + dwDataPos, row[dwParaIdx], dwCopyDataLen);
            DWORD dwLeftBufLen = (DWORD)(pRspPara[dwParaIdx].m_paraSize) - dwCopyDataLen;
            if (dwLeftBufLen) (void)memset((BYTE *)pRspData + dwDataPos + dwCopyDataLen, 0, dwLeftBufLen);
        }
        else
        {
        }

        /// 计算偏移
        dwDataPos += pRspPara[dwParaIdx].m_paraSize;
        if (dwDataPos > dwRspDataLen)
        {
            dwRc = FAILURE;
            break;
        }
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CTlvToSQL::GetJoinTable
  描    述: 获取关联表
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTlvToSQL::GetJoinTable(DWORD dwRelID, 
                        CDString &strFieldName,
                        CDString &strJoinTable)
{
    if (!m_piDataHandle) return FAILURE;

    IModel::Relation *pShip = m_piDataHandle->GetRelation(dwRelID);
    if (!pShip)
    {
        return FAILURE;
    }

    IModel *piModel = m_piDataHandle->GetModel();
    if (!piModel)
    {
        return FAILURE;
    }

    const char *pcszRelTable = piModel->GetTableName(pShip->m_attrID);
    if (!pcszRelTable)
    {
        return FAILURE;
    }

    DWORD dwRelFieldCount = 0;
    IModel::Field *pRelFields = piModel->GetFields(pShip->m_attrID, dwRelFieldCount);
    if (!pRelFields || !dwRelFieldCount)
    {
        return FAILURE;
    }

    if (!pShip->m_fieldID || (pShip->m_fieldID > dwRelFieldCount))
    {
        return FAILURE;
    }

    DWORD dwRelFieldID = (DWORD)(dwRelID & DCOP_FIELD_LOW0);
    if (!dwRelFieldID || (dwRelFieldID > dwRelFieldCount))
    {
        return FAILURE;
    }

    /// 输出字段(格式:'表名.字段名')
    strFieldName << pcszRelTable << "." << pRelFields[dwRelFieldID - 1].m_fieldName;

    /// 进行左联接，如果已经联接就不添加了(所以:如果一个表有多个关联字段，需要把关键联接的字段写到前面)
    CDString strJoinTableTmp = " left Join ";
    strJoinTableTmp << pcszRelTable;
    DWORD dwIdx = strJoinTable.Find((const char *)strJoinTableTmp);
    if (dwIdx == CDString::TAIL)
    {
        IDataHandle::Field *pFields = m_piDataHandle->GetFields();
        DWORD dwFieldCount = m_piDataHandle->GetFieldCount();
        if (!pFields || !dwFieldCount)
        {
            return FAILURE;
        }

        DWORD dwFieldID = pShip->m_relID;
        if (!dwFieldID || (dwFieldID > dwFieldCount))
        {
            return FAILURE;
        }

        strJoinTable << strJoinTableTmp << STR_FORMAT(" on %s.%s=%s.%s", 
                        m_piDataHandle->GetTableName(), pFields[dwFieldID - 1].m_fieldName, 
                        pcszRelTable, pRelFields[pShip->m_fieldID - 1].m_fieldName);
    }

    return SUCCESS;
}

