/// -------------------------------------------------
/// ObjData_handle.h : 数据句柄私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJDATA_HANDLE_H_
#define _OBJDATA_HANDLE_H_

#include "ObjModel_if.h"
#include "ObjData_if.h"


INTF_VER(IDataHandle, 1, 0, 0);


interface IDataHandle : public Instance
{
public:
    /// 字段定义
    struct Field : public IModel::Field
    {
        DWORD m_fieldOffset;                // 字段偏移
    };

public:
    IDataHandle();
    virtual ~IDataHandle();

    /// 初始化
    virtual DWORD Init(
                DWORD dwAttrID, 
                IObject *pOwner, 
                IModel *piModel
                ) = 0;

    /// Dump
    virtual void Dump(
                LOG_PRINT logPrint, 
                LOG_PARA logPara
                ) = 0;

    /// 添加记录
    virtual DWORD AddRecord(
                DCOP_PARA_NODE *pReqPara, 
                DWORD dwReqParaCount, 
                void *pReqData, 
                DWORD dwReqDataLen, 
                DCOP_PARA_NODE **ppEvtPara, 
                DWORD *pdwEvtParaCount, 
                CDArray *pEvtData
                ) = 0;

    /// 删除记录
    virtual DWORD DelRecord(
                BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                DCOP_PARA_NODE **ppEvtPara, 
                DWORD *pdwEvtParaCount, 
                CDArray *pEvtData
                ) = 0;

    /// 编辑记录
    virtual DWORD EditRecord(
                BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen,
                DCOP_PARA_NODE *pReqPara, 
                DWORD dwReqParaCount, 
                void *pReqData, 
                DWORD dwReqDataLen, 
                DCOP_PARA_NODE **ppEvtPara, 
                DWORD *pdwEvtParaCount, 
                CDArray *pEvtData
                ) = 0;

    /// 查询记录
    virtual DWORD QueryRecord(
                BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                DCOP_PARA_NODE *pReqPara, 
                DWORD dwReqParaCount, 
                DCOP_PARA_NODE *&rpRspPara, 
                DWORD &rdwRspParaCount, 
                CDArray &aRspData
                ) = 0;

    /// 统计记录
    virtual DWORD CountRecord(
                BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                DWORD &rdwCount
                ) = 0;

    /// 添加索引
    virtual DWORD AddKeyIdx(
                DCOP_PARA_NODE *pIdxPara, 
                DWORD dwIdxParaCount
                ) = 0;

    /// 删除索引
    virtual DWORD DelKeyIdx(
                DCOP_PARA_NODE *pIdxPara, 
                DWORD dwIdxParaCount
                ) = 0;

    /// 获取记录个数
    virtual DWORD GetRecCount(
                ) = 0;

    /// 保存当前ID
    virtual void SaveCurIdentify(DWORD curIdentify) {}

    /// 获取模型对象
    IModel *GetModel() {return m_piModel;}

    /// 获取所属对象
    IObject *GetOwner() {return m_pOwner;}

    /// 获取属性ID
    DWORD GetAttrID() {return m_dwAttrID;}

    /// 获取句柄类型
    DWORD GetType() {return m_dwType;}

    /// 获取数据表名
    const char *GetTableName() {return m_szTableName;}

    /// 获取字段列表
    Field *GetFields() {return m_pFields;}

    /// 获取字段个数
    DWORD GetFieldCount() {return m_dwFieldCount;}

    /// 获取记录大小
    DWORD GetRecSize() {return m_dwRecSize;}

    /// 初始化(子类在重载前面的Init接口中，必须先调用父类的这个接口)
    DWORD Init(DWORD dwType, 
                DWORD dwAttrID, 
                IObject *pOwner, 
                IModel *piModel, 
                IData *piData);

    /// 填充记录
    DWORD FillRecord(Field *pFields, 
                DWORD dwFieldCount, 
                BYTE *pbyRec, 
                DWORD dwRecSize, 
                DCOP_PARA_NODE *pPara, 
                DWORD dwParaCount, 
                void *pData, 
                DWORD dwDataLen);

    /// 拷贝字段
    Field *CopyFields(IModel *piModel, 
                DWORD dwAttrID, 
                DWORD &rdwFieldCount, 
                DWORD &rdwRecSize);

    /// 拷贝关联
    IModel::Relation *CopyShips(IModel *piModel, 
                DWORD dwAttrID, 
                DWORD &rdwShipCount);

    /// 获取输出参数
    DCOP_PARA_NODE *GetOutPara(Field *pFields, 
                DWORD dwFieldCount, 
                DWORD &rdwOutParaCount, 
                DCOP_PARA_NODE *pReqPara = 0, 
                DWORD dwReqParaCount = 0, 
                DWORD *pdwOutDataLen = 0, 
                bool bSpecPara = false);

    /// 拷贝记录
    DWORD CopyRecord(Field *pFields, 
                DWORD dwFieldCount, 
                BYTE *pbyRec, 
                DWORD dwRecSize, 
                DCOP_PARA_NODE *pPara, 
                DWORD dwParaCount, 
                void *pData, 
                DWORD dwDataLen);

    /// 更新记录
    DWORD UpdateRecord(Field *pFields, 
                DWORD dwFieldCount, 
                BYTE *pbyRec, 
                DWORD dwRecSize, 
                DCOP_PARA_NODE *pPara, 
                DWORD dwParaCount, 
                void *pData, 
                DWORD dwDataLen);

    /// 判断字段是否是数字
    bool IsDigital(DWORD dwFieldType);

    /// 判断字段是否是标识
    bool IsIdentify(DWORD dwFieldType);

    /// 设置值
    DWORD SetRecValue(DWORD dwFieldType, 
                void *pDst, 
                DWORD dwDstSize, 
                void *pSrc, 
                DWORD dwSrcSize);

    /// ID生成器(循环+1，直到生成不存在的ID)
    DWORD IdentifyGenerator(DWORD fieldID, DWORD fieldSize, DWORD minValue, DWORD maxValue);

    /// 设置当前ID
    void SetCurIdentify(DWORD curIdentify) {m_curIdentify = curIdentify;}

    /// 获取记录偏移和数量
    void GetOffsetAndLimit(DWORD &rdwOffset, 
                DWORD &rdwLimit, 
                DCOP_PARA_NODE *pPara, 
                DWORD dwParaCount, 
                void *pData, 
                DWORD dwDataLen);

    /// 是否匹配记录
    bool bMatchRecord(Field *pFields, 
                DWORD dwFieldCount, 
                BYTE *pbyRec, 
                BYTE byCond, 
                DCOP_PARA_NODE *pPara, 
                DWORD dwParaCount, 
                void *pData, 
                DWORD dwDataLen);

    /// 是否是数字方式匹配
    bool bDigitalMatch(DWORD dwOpCode, 
                DWORD dwParaValue, 
                DWORD dwRecValue);

    /// 是否是字符串方式匹配
    bool bStringMatch(DWORD dwOpCode, 
                const char *pParaStr, 
                const char *pRecStr);

    /// 是否是Buf方式匹配
    bool bBufferMatch(DWORD dwOpCode, 
                void *pParaBuf, 
                void *pRecBuf, 
                DWORD dwBufLen);

    /// 获取关联的属性和字段
    IModel::Relation *GetRelation(DWORD dwRelID);

    /// 获取关联的参数
    DWORD GetRelPara(DWORD dwRelID,
                DCOP_PARA_NODE &rPara);

    /// 获取关联的记录
    DWORD GetRelRecord(DCOP_PARA_NODE &rPara,
                BYTE *pbyRec, 
                void *pData);

private:
    IData *m_piData;                        // 数据对象
    IModel *m_piModel;                      // 模型对象
    IObject *m_pOwner;                      // 所属对象
    DWORD m_dwAttrID;                       // 属性ID
    DWORD m_dwType;                         // 句柄类型
    char m_szTableName[DCOP_TABLE_NAME_LEN]; // 数据表名
    Field *m_pFields;                       // 字段列表
    DWORD m_dwFieldCount;                   // 字段个数
    IModel::Relation *m_pShips;             // 关联列表
    DWORD m_dwShipCount;                    // 关联个数
    DWORD m_dwRecSize;                      // 记录大小
    DWORD m_curIdentify;                    // 当前标识
};


#endif // #ifndef _OBJDATA_HANDLE_H_

