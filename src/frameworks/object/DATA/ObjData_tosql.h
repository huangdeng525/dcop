/// -------------------------------------------------
/// ObjData_tosql.h : 将TLV转换为SQL语句私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJDATA_TOSQL_H_
#define _OBJDATA_TOSQL_H_

#include "ObjData_handle.h"
#include "string/dstring.h"
#include "mysql.h"


INTF_VER(ITlvToSQL, 1, 0, 0);


/// TLV到SQL转换接口
interface ITlvToSQL : public Instance
{
    /// 获取创建参数列表
    virtual DWORD GetCreateFieldList(
                        CDString &strArgList
                        ) = 0;

    /// 获取插入参数列表
    virtual DWORD GetInsertFieldList(
                        DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        void *pReqData, 
                        DWORD dwReqDataLen, 
                        CDString &strArgList
                        ) = 0;

    /// 获取条件参数列表
    virtual DWORD GetCondFieldList(
                        BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, 
                        DWORD dwCondParaCount, 
                        void *pCondData, 
                        DWORD dwCondDataLen, 
                        CDString &strArgList
                        ) = 0;

    /// 获取请求参数列表
    virtual DWORD GetReqFieldList(
                        DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        void *pReqData, 
                        DWORD dwReqDataLen, 
                        CDString &strArgList
                        ) = 0;

    /// 获取参数名字列表
    virtual DWORD GetFieldNameList(
                        DCOP_PARA_NODE *pPara, 
                        DWORD dwParaCount, 
                        CDString &strArgList, 
                        const char *cszSplit = ", ", 
                        CDString *pStrJoinTable = NULL
                        ) = 0;

    /// 获取响应参数列表
    virtual DWORD GetRspFieldList(
                        DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        MYSQL_RES *res, 
                        DCOP_PARA_NODE *&rpRspPara, 
                        DWORD &rdwRspParaCount, 
                        CDArray &aRspData
                        ) = 0;

    /// 获取Identify字段名称
    virtual DWORD GetIdentifyName(
                        CDString &strIdName
                        ) = 0;
};


/// TLV到SQL转换实现
class CTlvToSQL : public ITlvToSQL
{
public:
    /// 参数操作码类型定义
    static const char *ArgOpCode[];

public:
    CTlvToSQL(Instance *piParent, int argc, char **argv);
    ~CTlvToSQL();

    DCOP_DECLARE_INSTANCE;

    DWORD GetCreateFieldList(CDString &strArgList);

    DWORD GetInsertFieldList(DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
                        void *pReqData, DWORD dwReqDataLen, 
                        CDString &strArgList);

    DWORD GetCondFieldList(BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        CDString &strArgList);

    DWORD GetReqFieldList(DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
                        void *pReqData, DWORD dwReqDataLen, 
                        CDString &strArgList);

    DWORD GetFieldNameList(DCOP_PARA_NODE *pPara, 
                        DWORD dwParaCount, 
                        CDString &strArgList, 
                        const char *cszSplit, 
                        CDString *pStrJoinTable);

    DWORD GetRspFieldList(DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        MYSQL_RES *res, 
                        DCOP_PARA_NODE *&rpRspPara, 
                        DWORD &rdwRspParaCount, 
                        CDArray &aRspData);

    DWORD GetIdentifyName(CDString &strIdName);

private:
    DWORD GetCondArgOpCode(BYTE cond, 
                        BYTE opCode, 
                        BYTE paraType, 
                        const char *fieldName, 
                        CDString &strArgList);

    DWORD GetReqArgOpCode(BYTE opCode, 
                        BYTE paraType, 
                        const char *fieldName, 
                        CDString &strArgList);

    DWORD GetRspArgValue(DCOP_PARA_NODE *pRspPara, 
                        DWORD dwRspParaCount, 
                        MYSQL_ROW row, 
                        unsigned long *lens, 
                        void *pRspData, 
                        DWORD dwRspDataLen);

    DWORD GetJoinTable(DWORD dwRelID, 
                        CDString &strFieldName,
                        CDString &strJoinTable);

private:
    IDataHandle *m_piDataHandle;
};


#endif // #ifndef _OBJDATA_TOSQL_H_

