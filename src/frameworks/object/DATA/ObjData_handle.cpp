/// -------------------------------------------------
/// ObjData_handle.cpp : 数据句柄实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjData_main.h"
#include "ObjData_handle.h"
#include "cpu/bytes.h"
#include "string/dstring.h"


/*******************************************************
  函 数 名: IDataHandle
  描    述: IDataHandle构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IDataHandle::IDataHandle()
{
    m_piData = NULL;
    m_piModel = NULL;
    m_pOwner = NULL;
    m_dwAttrID = 0;
    m_dwType = IData::TYPE_MEM;
    *m_szTableName = '\0';
    m_pFields = NULL;
    m_dwFieldCount = 0;
    m_pShips = NULL;
    m_dwShipCount = 0;
    m_dwRecSize = 0;
    m_curIdentify = 0;
}

/*******************************************************
  函 数 名: IDataHandle::~IDataHandle
  描    述: IDataHandle析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IDataHandle::~IDataHandle()
{
    m_piData = NULL;
    m_piModel = NULL;
    m_pOwner = NULL;
    m_dwAttrID = 0;
    m_dwType = 0;
    *m_szTableName = '\0';
    if (m_pFields)
    {
        DCOP_Free(m_pFields);
        m_pFields = NULL;
    }
    m_dwFieldCount = 0;
    if (m_pShips)
    {
        DCOP_Free(m_pShips);
        m_pShips = NULL;
    }
    m_dwShipCount = 0;
    m_dwRecSize = 0;
    m_curIdentify = 0;
}

/*******************************************************
  函 数 名: IDataHandle::Init
  描    述: 创建
  输    入: dwType
            dwAttrID
            pOwner
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD IDataHandle::Init(DWORD dwType, 
                DWORD dwAttrID, 
                IObject *pOwner, 
                IModel *piModel, 
                IData *piData)
{
    if (m_pFields)
    {
        /// 已有字段信息，说明已经初始化过了
        return FAILURE;
    }

    if (!pOwner || !piModel)
    {
        /// 输入参数为空，子类初始化时不用重复检查了
        return FAILURE;
    }

    const char *pszTableName = piModel->GetTableName(dwAttrID);
    if (!pszTableName)
    {
        return FAILURE;
    }

    DWORD dwFieldCount = 0;
    DWORD dwRecSize = 0;
    Field *pFields = CopyFields(piModel, dwAttrID, dwFieldCount, dwRecSize);
    if (!pFields)
    {
        /// 拷贝字段失败，只有返回错误了
        return FAILURE;
    }

    m_piData = piData;
    m_piModel = piModel;
    m_pOwner = pOwner;
    m_dwAttrID = dwAttrID;
    m_dwType = dwType;
    (void)memset(m_szTableName, 0, sizeof(m_szTableName));
    (void)snprintf(m_szTableName, sizeof(m_szTableName), "%s", pszTableName);
    m_pFields = pFields;
    m_dwFieldCount = dwFieldCount;
    m_dwRecSize = dwRecSize;

    m_pShips = CopyShips(piModel, dwAttrID, m_dwShipCount);

    return SUCCESS;
}

/*******************************************************
  函 数 名: IDataHandle::FillRecord
  描    述: 填充记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD IDataHandle::FillRecord(Field *pFields, DWORD dwFieldCount,
                        BYTE *pbyRec, DWORD dwRecSize,
                        DCOP_PARA_NODE *pPara, DWORD dwParaCount,
                        void *pData, DWORD dwDataLen)
{
    /// 检查参数
    if (!pFields || !dwFieldCount ||
        !pbyRec || !dwRecSize ||
        !pPara || !dwParaCount ||
        !pData || !dwDataLen)
    {
        return FAILURE;
    }

    /// 使用默认值填充(记录第一个字节是标识位)
    BYTE *pbyRecTmp = pbyRec;
    DWORD dwKeyCount = 0;
    for (DWORD i = 0; i < dwFieldCount; ++i)
    {
        /// 获取缺省值
        DWORD defaultValue = pFields[i].m_defaultValue;

        /// 自动生成IDentify
        if (IsIdentify(pFields[i].m_fieldType))
        {
            defaultValue = IdentifyGenerator(i + 1, 
                        pFields[i].m_fieldSize, 
                        pFields[i].m_minValue, 
                        pFields[i].m_maxValue);
        }
        else
        {
            /// 非自动生成的，统计关键字数量
            if (pFields[i].m_isKey)
            {
                dwKeyCount++;
            }
        }

        /// 设置缺省值
        BYTE byDefValue[4];
        Bytes_SetDword(byDefValue, defaultValue);
        if (SetRecValue(pFields[i].m_fieldType, 
                        pbyRecTmp, pFields[i].m_fieldSize, 
                        byDefValue, sizeof(byDefValue)
                        ) != SUCCESS)
        {
            (void)memset(pbyRecTmp, 0, pFields[i].m_fieldSize);
        }

        pbyRecTmp += pFields[i].m_fieldSize;
    }

    /// 再按照输入值填充
    DWORD dwRc = SUCCESS;
    DWORD dwDataPos = 0;
    DWORD dwKeyCountTmp = 0;
    for (DWORD dwParaIdx = 0; dwParaIdx < dwParaCount; ++dwParaIdx)
    {
        /// 需要判断参数ID的正确性
        if (!pPara[dwParaIdx].m_paraID || (pPara[dwParaIdx].m_paraID > dwFieldCount))
        {
            dwRc = FAILURE;
            break;
        }

        /// 获取字段索引
        DWORD dwFieldIdx = pPara[dwParaIdx].m_paraID - 1;

        /// 自动生成的IDentify不能被覆盖
        if (!IsIdentify(pFields[dwFieldIdx].m_fieldType))
        {
            /// 判断数字的大小范围
            if (IsDigital(pFields[dwFieldIdx].m_fieldType))
            {
                DWORD dwValue = Bytes_GetDwordValue((BYTE *)pData + dwDataPos, pPara[dwParaIdx].m_paraSize);
                DWORD dwMinValue = pFields[dwFieldIdx].m_minValue;
                DWORD dwMaxValue = pFields[dwFieldIdx].m_maxValue;
                if ((dwMinValue != dwMaxValue) && ((dwValue < dwMinValue) || (dwValue > dwMaxValue)))
                {
                    dwRc = FAILURE;
                    break;
                }
            }

            /// 设置输入值
            pbyRecTmp = pbyRec + pFields[dwFieldIdx].m_fieldOffset;
            dwRc = SetRecValue(pFields[dwFieldIdx].m_fieldType,
                            pbyRecTmp, pFields[dwFieldIdx].m_fieldSize,
                            (BYTE *)pData + dwDataPos, 
                            pPara[dwParaIdx].m_paraSize);
            if (dwRc != SUCCESS)
            {
                break;
            }

            if (pFields[dwFieldIdx].m_isKey)
            {
                dwKeyCountTmp++;
            }
        }

        /// 计算偏移
        dwDataPos += pPara[dwParaIdx].m_paraSize;
        if (dwDataPos > dwDataLen)
        {
            dwRc = FAILURE;
            break;
        }

        /// 查找关键字是有重复
        if (pFields[dwFieldIdx].m_isKey)
        {
            void *pKeyData = DCOP_Malloc(pFields[dwFieldIdx].m_fieldSize);
            if (!pKeyData)
            {
                dwRc = FAILURE;
                break;
            }

            (void)memcpy(pKeyData, pbyRecTmp, pFields[dwFieldIdx].m_fieldSize);
            DWORD dwCount = 0;
            dwRc = CountRecord(DCOP_CONDITION_ONE, pPara + dwParaIdx, 1, 
                        pKeyData, pFields[dwFieldIdx].m_fieldSize, 
                        dwCount);
            if (!dwRc && dwCount)
            {
                /// 找到重复记录
                DCOP_Free(pKeyData);
                dwRc = FAILURE;
                break;
            }

            DCOP_Free(pKeyData);
        }
    }

    /// 比较输入关键字是否少于设定关键字
    if (dwKeyCountTmp < dwKeyCount)
    {
        dwRc = FAILURE;
    }

    return dwRc;
}

/*******************************************************
  函 数 名: IDataHandle::CopyFields
  描    述: 拷贝字段
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IDataHandle::Field *IDataHandle::CopyFields(IModel *piModel, DWORD dwAttrID, 
                        DWORD &rdwFieldCount, DWORD &rdwRecSize)
{
    if (!piModel)
    {
        return NULL;
    }

    /// 获取建模中的字段列表
    DWORD dwFieldCount = 0;
    IModel::Field *pFields = piModel->GetFields(dwAttrID, dwFieldCount);
    if (!pFields || !dwFieldCount)
    {
        return NULL;
    }

    /// 创建字段列表的拷贝
    Field *pFieldsCopy = (Field *)DCOP_Malloc(dwFieldCount * sizeof(Field));
    if (!pFieldsCopy)
    {
        return NULL;
    }

    /// 计算单条记录大小
    DWORD dwRecSize = 0;
    for (DWORD i = 0; i < dwFieldCount; ++i)
    {
        (void)memcpy(pFieldsCopy + i, pFields + i, sizeof(IModel::Field));
        pFieldsCopy[i].m_fieldOffset = dwRecSize;
        dwRecSize += pFields[i].m_fieldSize;
    }
    if (!dwRecSize)
    {
        DCOP_Free(pFieldsCopy);
        return NULL;
    }

    rdwFieldCount = dwFieldCount;
    rdwRecSize = dwRecSize;

    return pFieldsCopy;
}

/*******************************************************
  函 数 名: IDataHandle::CopyShips
  描    述: 拷贝关联
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IModel::Relation *IDataHandle::CopyShips(IModel *piModel, 
                DWORD dwAttrID, 
                DWORD &rdwShipCount)
{
    if (!piModel)
    {
        return NULL;
    }

    /// 获取建模中的关联列表
    DWORD dwShipCount = 0;
    IModel::Relation *pShips = piModel->GetShips(dwAttrID, dwShipCount);
    if (!pShips || !dwShipCount)
    {
        return NULL;
    }

    /// 创建关联列表的拷贝
    IModel::Relation *pShipsCopy = (IModel::Relation *)DCOP_Malloc(dwShipCount * sizeof(IModel::Relation));
    if (!pShipsCopy)
    {
        return NULL;
    }

    (void)memcpy(pShipsCopy, pShips, dwShipCount * sizeof(IModel::Relation));

    rdwShipCount = dwShipCount;
    return pShipsCopy;
}

/*******************************************************
  函 数 名: IDataHandle::GetOutPara
  描    述: 获取输出参数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DCOP_PARA_NODE *IDataHandle::GetOutPara(Field *pFields, 
                        DWORD dwFieldCount, 
                        DWORD &rdwOutParaCount, 
                        DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        DWORD *pdwOutDataLen, 
                        bool bSpecPara)
{
    DCOP_PARA_NODE *pOutPara = 0;
    DWORD dwOutParaCount = 0;
    DWORD dwOutDataLen = 0;
    DWORD dwSpecParaCount = 0;

    /// 获取全部字段
    if (!pReqPara || !dwReqParaCount)
    {
        pOutPara = (DCOP_PARA_NODE *)DCOP_Malloc(dwFieldCount * sizeof(DCOP_PARA_NODE));
        if (!pOutPara) return NULL;
    }

    /// 遍历所有字段
    if (!pReqPara || !dwReqParaCount || bSpecPara)
    {
        for (DWORD dwFieldIdx = 0; dwFieldIdx < dwFieldCount; ++dwFieldIdx)
        {
            if (!pReqPara || !dwReqParaCount)
            {
                dwOutDataLen += pFields[dwFieldIdx].m_fieldSize;
                pOutPara[dwOutParaCount].m_paraID = dwFieldIdx + 1;
                pOutPara[dwOutParaCount].m_opCode = DCOP_OPCODE_NONE;
                pOutPara[dwOutParaCount].m_paraType = pFields[dwFieldIdx].m_fieldType;
                pOutPara[dwOutParaCount].m_paraSize = pFields[dwFieldIdx].m_fieldSize;
                dwOutParaCount++;
            }
            else
            {
                if ((pFields[dwFieldIdx].m_fieldType == IModel::FIELD_IDENTIFY) || 
                     (pFields[dwFieldIdx].m_isKey))
                {
                    dwSpecParaCount++;
                }
            }
        }
    }

    /// 获取全部字段
    if (!pReqPara || !dwReqParaCount)
    {
        rdwOutParaCount = dwOutParaCount;
        if (pdwOutDataLen) *pdwOutDataLen = dwOutDataLen;
        return pOutPara;
    }

    pOutPara = (DCOP_PARA_NODE *)DCOP_Malloc((dwSpecParaCount + dwReqParaCount) * sizeof(DCOP_PARA_NODE));
    if (!pOutPara)
    {
        return NULL;
    }

    dwOutParaCount = 0;
    dwOutDataLen = 0;

    /// 获取特殊字段
    if (dwSpecParaCount)
    {
        for (DWORD dwFieldIdx = 0; dwFieldIdx < dwFieldCount; ++dwFieldIdx)
        {
            if ((pFields[dwFieldIdx].m_fieldType == IModel::FIELD_IDENTIFY) || 
                    (pFields[dwFieldIdx].m_isKey))
            {
                dwOutDataLen += pFields[dwFieldIdx].m_fieldSize;
                pOutPara[dwOutParaCount].m_paraID = dwFieldIdx + 1;
                pOutPara[dwOutParaCount].m_opCode = DCOP_OPCODE_NONE;
                pOutPara[dwOutParaCount].m_paraType = pFields[dwFieldIdx].m_fieldType;
                pOutPara[dwOutParaCount].m_paraSize = pFields[dwFieldIdx].m_fieldSize;
                dwOutParaCount++;
                if (dwOutParaCount >= dwSpecParaCount)
                {
                    break;
                }
            }
        }
    }

    /// 指定字段输出
    for (DWORD dwParaIdx = 0; dwParaIdx < dwReqParaCount; ++dwParaIdx)
    {
        if (DCOP_SPECPARA(pReqPara[dwParaIdx].m_paraID, DCOP_FIELD_RELATION))
        {
            if (GetRelPara(pReqPara[dwParaIdx].m_paraID, pOutPara[dwOutParaCount]) == SUCCESS)
            {
                dwOutDataLen += pOutPara[dwOutParaCount].m_paraSize;
                dwOutParaCount++;
            }

            continue;
        }

        if (!pReqPara[dwParaIdx].m_paraID || (pReqPara[dwParaIdx].m_paraID > dwFieldCount))
        {
            continue;
        }

        DWORD dwFieldIdx = pReqPara[dwParaIdx].m_paraID - 1;
        dwOutDataLen += pFields[dwFieldIdx].m_fieldSize;
        pOutPara[dwOutParaCount].m_paraID = dwFieldIdx + 1;
        pOutPara[dwOutParaCount].m_opCode = DCOP_OPCODE_NONE;
        pOutPara[dwOutParaCount].m_paraType = pFields[dwFieldIdx].m_fieldType;
        pOutPara[dwOutParaCount].m_paraSize = pFields[dwFieldIdx].m_fieldSize;
        dwOutParaCount++;
    }

    rdwOutParaCount = dwOutParaCount;
    if (pdwOutDataLen) *pdwOutDataLen = dwOutDataLen;
    return pOutPara;
}

/*******************************************************
  函 数 名: IDataHandle::CopyRecord
  描    述: 拷贝记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD IDataHandle::CopyRecord(Field *pFields, DWORD dwFieldCount,
                        BYTE *pbyRec, DWORD dwRecSize,
                        DCOP_PARA_NODE *pPara, DWORD dwParaCount,
                        void *pData, DWORD dwDataLen)
{
    /// 检查参数
    if (!pFields || !dwFieldCount ||
        !pbyRec || !dwRecSize ||
        !pPara || !dwParaCount ||
        !pData || !dwDataLen)
    {
        return FAILURE;
    }

    /// 从记录中按指定字段进行拷贝
    DWORD dwRc = SUCCESS;
    DWORD dwDataPos = 0;
    for (DWORD dwParaIdx = 0; dwParaIdx < dwParaCount; ++dwParaIdx)
    {
        if (DCOP_SPECPARA(pPara[dwParaIdx].m_paraID, DCOP_FIELD_RELATION))
        {
            dwRc = GetRelRecord(pPara[dwParaIdx], pbyRec, (BYTE *)pData + dwDataPos);
            if (dwRc != SUCCESS)
            {
                break;
            }

            dwDataPos += pPara[dwParaIdx].m_paraSize;
            if (dwDataPos > dwDataLen)
            {
                dwRc = FAILURE;
                break;
            }

            continue;
        }

        if (!pPara[dwParaIdx].m_paraID || (pPara[dwParaIdx].m_paraID > dwFieldCount))
        {
            continue;
        }

        /// 获取字段索引
        DWORD dwFieldIdx = pPara[dwParaIdx].m_paraID - 1;

        /// 设置输入值
        BYTE *pbyRecTmp = pbyRec + pFields[dwFieldIdx].m_fieldOffset;
        dwRc = SetRecValue(pFields[dwFieldIdx].m_fieldType, 
                        (BYTE *)pData + dwDataPos, 
                        pPara[dwParaIdx].m_paraSize, 
                        pbyRecTmp, pFields[dwFieldIdx].m_fieldSize);
        if (dwRc != SUCCESS)
        {
            break;
        }

        dwDataPos += pPara[dwParaIdx].m_paraSize;
        if (dwDataPos > dwDataLen)
        {
            dwRc = FAILURE;
            break;
        }
    }

    return dwRc;
}

/*******************************************************
  函 数 名: IDataHandle::UpdateRecord
  描    述: 更新记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD IDataHandle::UpdateRecord(Field *pFields, DWORD dwFieldCount,
                        BYTE *pbyRec, DWORD dwRecSize,
                        DCOP_PARA_NODE *pPara, DWORD dwParaCount,
                        void *pData, DWORD dwDataLen)
{
    DWORD dwRc = SUCCESS;
    DWORD dwReqDataPos = 0;

    /// 检查参数
    if (!pFields || !dwFieldCount ||
        !pbyRec || !dwRecSize ||
        !pPara || !dwParaCount ||
        !pData || !dwDataLen)
    {
        return FAILURE;
    }

    /// 循环把参数更新到记录上
    for (DWORD dwParaIdx = 0; dwParaIdx < dwParaCount; ++dwParaIdx)
    {
        if (!pPara[dwParaIdx].m_paraID || (pPara[dwParaIdx].m_paraID > dwFieldCount))
        {
            continue;
        }

        DWORD dwFieldIdx = pPara[dwParaIdx].m_paraID - 1;
        BYTE *pbyRecTmp = pbyRec + pFields[dwFieldIdx].m_fieldOffset;
        dwRc = SetRecValue(pFields[dwFieldIdx].m_fieldType, 
                        pbyRecTmp, pFields[dwFieldIdx].m_fieldSize,
                        (BYTE *)pData + dwReqDataPos, 
                        pPara[dwParaIdx].m_paraSize);
        if (dwRc != SUCCESS)
        {
            break;
        }

        dwReqDataPos += pPara[dwParaIdx].m_paraSize;
        if (dwReqDataPos > dwDataLen)
        {
            dwRc = FAILURE;
            break;
        }
    }

    return dwRc;
}

/*******************************************************
  函 数 名: IDataHandle::IsDigital
  描    述: 是否是数字
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool IDataHandle::IsDigital(DWORD dwFieldType)
{
    if ((dwFieldType == IModel::FIELD_IDENTIFY) ||
        (dwFieldType == IModel::FIELD_BYTE) ||
        (dwFieldType == IModel::FIELD_WORD) ||
        (dwFieldType == IModel::FIELD_DWORD) ||
        (dwFieldType == IModel::FIELD_SHORT) ||
        (dwFieldType == IModel::FIELD_INTEGER) ||
        (dwFieldType == IModel::FIELD_NUMBER))
    {
        return true;
    }

    return false;
}

/*******************************************************
  函 数 名: IDataHandle::IsIdentify
  描    述: 是否是标识
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool IDataHandle::IsIdentify(DWORD dwFieldType)
{
    if (dwFieldType == IModel::FIELD_IDENTIFY)
    {
        return true;
    }

    return false;
}

/*******************************************************
  函 数 名: IDataHandle::SetRecValue
  描    述: 设置值
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD IDataHandle::SetRecValue(DWORD dwFieldType, 
                        void *pDst, DWORD dwDstSize, 
                        void *pSrc, DWORD dwSrcSize)
{
    OSASSERT(pDst != NULL);

    if (!pSrc || !dwSrcSize)
    {
        return FAILURE;
    }

    /// 如果是数字的话，先获取到DWORD的变量值中
    DWORD dwValue = 0;
    if (IsDigital(dwFieldType))
    {
        dwValue = Bytes_GetDwordValue((BYTE *)pSrc, dwSrcSize);
    }
    else if (IModel::FIELD_STRING == dwFieldType)
    {
        CDString strTmp((char *)pSrc, dwSrcSize);
        if (strTmp.Length() > dwDstSize)
        {
            return FAILURE;
        }
    }
    else
    {
        if (dwSrcSize > dwDstSize)
        {
            return FAILURE;
        }
    }

    /// 根据类型进行赋值
    if (IsDigital(dwFieldType))
    {
        Bytes_SetDwordValue(dwValue, (BYTE *)pDst, dwDstSize);
    }
    else
    {
        (void)memcpy(pDst, pSrc, dwSrcSize);
        if (dwDstSize > dwSrcSize)
        {
            (void)memset((BYTE *)pDst + dwSrcSize, 0, dwDstSize - dwSrcSize);
        }
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: IDataHandle::IdentifyGenerator
  描    述: ID生成器(循环+1，直到生成不存在的ID)
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD IDataHandle::IdentifyGenerator(DWORD fieldID, DWORD fieldSize, DWORD minValue, DWORD maxValue)
{
    /// 最小值和最大值相等或者反了就是没有设置范围
    if (minValue >= maxValue)
    {
        minValue = 1;
        if (fieldSize == sizeof(BYTE))
        {
            maxValue = 0xff;
        }
        else if (fieldSize == sizeof(WORD))
        {
            maxValue = 0xffff;
        }
        else
        {
            maxValue = 0xffffffff;
        }
    }

    void *pKeyData = DCOP_Malloc(fieldSize);
    if (!pKeyData)
    {
        return 0;
    }

    DCOP_PARA_NODE findPara = {fieldID, DCOP_OPCODE_NONE, IModel::FIELD_IDENTIFY, (WORD)fieldSize};

    for (DWORD i = minValue; i < maxValue; ++i)
    {
        m_curIdentify++;
        if ((m_curIdentify > maxValue) || (m_curIdentify < minValue))
        {
            /// 超出范围都从最小值开始
            m_curIdentify = minValue;
        }

        Bytes_SetDwordValue(m_curIdentify, (BYTE *)pKeyData, fieldSize);
        DWORD dwCount = 0;
        DWORD dwRc = CountRecord(DCOP_CONDITION_ONE, &findPara, 1, 
                    pKeyData, fieldSize, 
                    dwCount);
        if (!dwRc && dwCount)
        {
            /// 找到重复记录
            continue;
        }

        break;
    }

    DCOP_Free(pKeyData);
    
    /// 保存一下当前值
    SaveCurIdentify(m_curIdentify);

    return m_curIdentify;
}

/*******************************************************
  函 数 名: IDataHandle::GetOffsetAndLimit
  描    述: 获取记录偏移和数量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void IDataHandle::GetOffsetAndLimit(DWORD &rdwOffset, 
                DWORD &rdwLimit, 
                DCOP_PARA_NODE *pPara, 
                DWORD dwParaCount, 
                void *pData, 
                DWORD dwDataLen)
{
    rdwOffset = 0;
    rdwLimit = 0;

    DWORD dwDataPos = 0;
    for (DWORD dwParaIdx = 0; dwParaIdx < dwParaCount; ++dwParaIdx)
    {
        /// 获取记录偏移和数量
        if (pPara[dwParaIdx].m_paraID == DCOP_SPECPARA_OFFSET)
        {
            rdwOffset = Bytes_GetDwordValue((BYTE *)pData + dwDataPos, 
                        pPara[dwParaIdx].m_paraSize);
        }
        else if (pPara[dwParaIdx].m_paraID == DCOP_SPECPARA_LIMIT)
        {
            rdwLimit = Bytes_GetDwordValue((BYTE *)pData + dwDataPos, 
                        pPara[dwParaIdx].m_paraSize);
        }
        else
        {
        }

        /// 偏移下一个参数位置
        dwDataPos += pPara[dwParaIdx].m_paraSize;
        if (dwDataPos > dwDataLen)
        {
            break;
        }
    }
}

/*******************************************************
  函 数 名: IDataHandle::bMatchRecord
  描    述: 是否匹配记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool IDataHandle::bMatchRecord(Field *pFields, DWORD dwFieldCount, 
                        BYTE *pbyRec, BYTE byCond, 
                        DCOP_PARA_NODE *pPara, DWORD dwParaCount, 
                        void *pData, DWORD dwDataLen)
{
    if (!pPara && !dwParaCount && !pData && !dwDataLen)
    {
        return true;
    }

    bool bRc = true;
    DWORD dwDataPos = 0;
    for (DWORD dwParaIdx = 0; dwParaIdx < dwParaCount; ++dwParaIdx)
    {
        /// 字段ID不能为0
        if (!pPara[dwParaIdx].m_paraID || (pPara[dwParaIdx].m_paraID > dwFieldCount))
        {
            if (byCond)
            {
                bRc = false;
                break;
            }
            continue;
        }

        /// 获取字段索引
        DWORD dwFieldIdx = pPara[dwParaIdx].m_paraID - 1;
        BYTE *pbyRecTmp = pbyRec + pFields[dwFieldIdx].m_fieldOffset;

        /// 数值匹配
        bool bMatch = false;
        if (IsDigital(pFields[dwFieldIdx].m_fieldType))
        {
            DWORD dwParaValue = Bytes_GetDwordValue((BYTE *)pData + dwDataPos, 
                        pPara[dwParaIdx].m_paraSize);
            DWORD dwRecValue = Bytes_GetDwordValue(pbyRecTmp, 
                        pFields[dwFieldIdx].m_fieldSize);
            bMatch = bDigitalMatch(pPara[dwParaIdx].m_opCode, 
                        dwParaValue, 
                        dwRecValue);
        }
        /// 字符串匹配
        else if (IModel::FIELD_STRING == pFields[dwFieldIdx].m_fieldType)
        {
            CDString strPara((char *)pData + dwDataPos, pPara[dwParaIdx].m_paraSize);
            CDString strRec((char *)pbyRecTmp, pFields[dwFieldIdx].m_fieldSize);
            bMatch = bStringMatch(pPara[dwParaIdx].m_opCode, 
                        (const char *)strPara, 
                        (const char *)strRec);
        }
        /// 其他匹配
        else
        {
            if (pPara[dwParaIdx].m_paraSize == pFields[dwFieldIdx].m_fieldSize)
            {
                bMatch = bBufferMatch(pPara[dwParaIdx].m_opCode, 
                        (char *)pData + dwDataPos, 
                        (char *)pbyRecTmp, 
                        pFields[dwFieldIdx].m_fieldSize);
            }
        }

        /// 完全匹配中，如果当前不匹配，返回false
        if (byCond && !bMatch)
        {
            bRc = false;
            break;
        }

        /// 任意匹配中，如果当前匹配，则返回true
        if (!byCond && bMatch)
        {
            bRc = true;
            break;
        }

        /// 偏移下一个参数位置
        dwDataPos += pPara[dwParaIdx].m_paraSize;
        if (dwDataPos > dwDataLen)
        {
            bRc = false;
            break;
        }

        bRc = bMatch;
    }

    return bRc;
}

/*******************************************************
  函 数 名: IDataHandle::bDigitalMatch
  描    述: 是否是数字方式匹配
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool IDataHandle::bDigitalMatch(DWORD dwOpCode, DWORD dwParaValue, DWORD dwRecValue)
{
    bool bRc = false;
    switch (dwOpCode)
    {
        case DCOP_OPCODE_EQUAL:
            bRc = (dwRecValue == dwParaValue)? true : false;
            break;
        case DCOP_OPCODE_MORE_EQUAL:
            bRc = (dwRecValue >= dwParaValue)? true : false;
            break;
        case DCOP_OPCODE_MORE:
            bRc = (dwRecValue > dwParaValue)? true : false;
            break;
        case DCOP_OPCODE_LESS:
            bRc = (dwRecValue < dwParaValue)? true : false;
            break;
        case DCOP_OPCODE_LESS_EQUAL:
            bRc = (dwRecValue <= dwParaValue)? true : false;
            break;
        case DCOP_OPCODE_NOT_EQUAL:
            bRc = (dwRecValue != dwParaValue)? true : false;
            break;
        default:
            bRc = false;
            break;
    }

    return bRc;
}

/*******************************************************
  函 数 名: IDataHandle::bStringMatch
  描    述: 是否是字符串方式匹配
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool IDataHandle::bStringMatch(DWORD dwOpCode, const char *pParaStr, const char *pRecStr)
{
    bool bRc = false;
    switch (dwOpCode)
    {
        case DCOP_OPCODE_EQUAL:
            bRc = (!strcmp(pRecStr, pParaStr))? true : false;
            break;
        case DCOP_OPCODE_MORE_EQUAL:
            bRc = (strcmp(pRecStr, pParaStr) >= 0)? true : false;
            break;
        case DCOP_OPCODE_MORE:
            bRc = (strcmp(pRecStr, pParaStr) > 0)? true : false;
            break;
        case DCOP_OPCODE_LESS:
            bRc = (strcmp(pRecStr, pParaStr) < 0)? true : false;
            break;
        case DCOP_OPCODE_LESS_EQUAL:
            bRc = (strcmp(pRecStr, pParaStr) <= 0)? true : false;
            break;
        case DCOP_OPCODE_NOT_EQUAL:
            bRc = (strcmp(pRecStr, pParaStr))? true : false;
            break;
        case DCOP_OPCODE_INCLUDE:
            bRc = (strstr(pRecStr, pParaStr))? true : false;
            break;
        case DCOP_OPCODE_NOT_INCLUDE:
            bRc = (!strstr(pRecStr, pParaStr))? true : false;
            break;
        default:
            bRc = false;
            break;
    }

    return bRc;
}

/*******************************************************
  函 数 名: IDataHandle::bBufferMatch
  描    述: 是否是Buf方式匹配
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool IDataHandle::bBufferMatch(DWORD dwOpCode, void *pParaBuf, void *pRecBuf, DWORD dwBufLen)
{
    bool bRc = false;
    switch (dwOpCode)
    {
        case DCOP_OPCODE_EQUAL:
            bRc = (!memcmp(pRecBuf, pParaBuf, dwBufLen))? true : false;
            break;
        case DCOP_OPCODE_MORE_EQUAL:
            bRc = (memcmp(pRecBuf, pParaBuf, dwBufLen) >= 0)? true : false;
            break;
        case DCOP_OPCODE_MORE:
            bRc = (memcmp(pRecBuf, pParaBuf, dwBufLen) > 0)? true : false;
            break;
        case DCOP_OPCODE_LESS:
            bRc = (memcmp(pRecBuf, pParaBuf, dwBufLen) < 0)? true : false;
            break;
        case DCOP_OPCODE_LESS_EQUAL:
            bRc = (memcmp(pRecBuf, pParaBuf, dwBufLen) <= 0)? true : false;
            break;
        case DCOP_OPCODE_NOT_EQUAL:
            bRc = (memcmp(pRecBuf, pParaBuf, dwBufLen))? true : false;
            break;
        default:
            bRc = false;
            break;
    }

    return bRc;
}

/*******************************************************
  函 数 名: IDataHandle::GetRelation
  描    述: 获取关联的属性和字段
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IModel::Relation *IDataHandle::GetRelation(DWORD dwRelID)
{
    if (!DCOP_SPECPARA(dwRelID, DCOP_FIELD_RELATION))
    {
        return NULL;
    }

    if (!m_piModel || !m_dwFieldCount || !m_pShips || !m_dwShipCount)
    {
        return NULL;
    }

    DWORD dwFieldID = (DWORD)((dwRelID & DCOP_FIELD_LOW1) >> 8);
    if (!dwFieldID || (dwFieldID > m_dwFieldCount))
    {
        return NULL;
    }

    IModel::Relation *pShip = NULL;
    for (DWORD i = 0; i < m_dwShipCount; ++i)
    {
        if (m_pShips[i].m_relID == dwFieldID)
        {
            pShip = &(m_pShips[i]);
            break;
        }
    }

    return pShip;
}

/*******************************************************
  函 数 名: IDataHandle::GetRelPara
  描    述: 获取关联的参数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD IDataHandle::GetRelPara(DWORD dwRelID, DCOP_PARA_NODE &rPara)
{
    IModel::Relation *pShip = GetRelation(dwRelID);
    if (!pShip)
    {
        return FAILURE;
    }

    DWORD dwFieldCount = 0;
    IModel::Field *pFields = m_piModel->GetFields(pShip->m_attrID, dwFieldCount);
    if (!pFields || !dwFieldCount)
    {
        return FAILURE;
    }

    DWORD dwRelFieldID = (DWORD)(dwRelID & DCOP_FIELD_LOW0);
    if (!dwRelFieldID || (dwRelFieldID > dwFieldCount))
    {
        return FAILURE;
    }

    rPara.m_paraID = dwRelID;
    rPara.m_opCode = DCOP_OPCODE_NONE;
    rPara.m_paraType = pFields[dwRelFieldID - 1].m_fieldType;
    rPara.m_paraSize = pFields[dwRelFieldID - 1].m_fieldSize;

    return SUCCESS;
}

/*******************************************************
  函 数 名: IDataHandle::GetRelRecord
  描    述: 获取关联的记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD IDataHandle::GetRelRecord(DCOP_PARA_NODE &rPara, BYTE *pbyRec, void *pData)
{
    if (!pbyRec || !pData || !m_piData || !m_pFields)
    {
        return FAILURE;
    }

    IModel::Relation *pShip = GetRelation(rPara.m_paraID);
    if (!pShip)
    {
        return FAILURE;
    }

    if (!pShip->m_relID || (pShip->m_relID > m_dwFieldCount))
    {
        return FAILURE;
    }

    BYTE *pbyRecTmp = pbyRec + m_pFields[pShip->m_relID - 1].m_fieldOffset;
    if (!pbyRecTmp)
    {
        return FAILURE;
    }

    CData *piData = (CData *)m_piData;
    if (!piData)
    {
        return FAILURE;
    }

    /// 获取数据句柄
    IData::Handle hData = piData->GetHandle(pShip->m_attrID);
    if (!hData)
    {
        return FAILURE;
    }

    /// 生成条件字段(关联的字段类型在两个表中是一致的)
    DCOP_PARA_NODE condPara;
    condPara.m_paraID = pShip->m_fieldID;
    condPara.m_opCode = DCOP_OPCODE_NONE;
    condPara.m_paraType = m_pFields[pShip->m_relID - 1].m_fieldType;
    condPara.m_paraSize = m_pFields[pShip->m_relID - 1].m_fieldSize;

    /// 生成请求字段(查询的字段类型已获取，直接输入的)
    DCOP_PARA_NODE reqPara;
    reqPara.m_paraID = (DWORD)(rPara.m_paraID & DCOP_FIELD_LOW0);
    reqPara.m_opCode = DCOP_OPCODE_NONE;
    reqPara.m_paraType = rPara.m_paraType;
    reqPara.m_paraSize = rPara.m_paraSize;

    /// 进行查询(必须是唯一值进行检索)
    DCOP_PARA_NODE *pRspPara = NULL;
    DWORD dwRspParaCount = 0;
    CDArray aRspData;
    DWORD dwRc = piData->QueryRecord(hData, DCOP_CONDITION_ONE, 
                        &condPara, 1, 
                        pbyRecTmp, condPara.m_paraSize, 
                        &reqPara, 1, 
                        pRspPara, dwRspParaCount, 
                        aRspData);
    if (dwRc == SUCCESS)
    {
        if (pRspPara) DCOP_Free(pRspPara);
        dwRc = SetRecValue(rPara.m_paraType, 
                        pData, rPara.m_paraSize, 
                        aRspData.Pos(0), aRspData.Size());
    }
    if (dwRc != SUCCESS)
    {
        (void)memset(pData, 0, rPara.m_paraSize);
    }

    return SUCCESS;
}

