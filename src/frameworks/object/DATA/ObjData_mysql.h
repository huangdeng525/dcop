/// -------------------------------------------------
/// ObjData_mysql.h : MySQL数据库数据私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJDATA_MYSQL_H_
#define _OBJDATA_MYSQL_H_

#include "ObjData_handle.h"
#include "ObjData_tosql.h"


class CDataMySQL : public IDataHandle
{
public:
    static const char *AddrInfo;
    static const char *UserInfo;
    static const char *ConnInfo;
    static const char *BaseInfo;

    static const DWORD ColNameMaxLen = 64;

public:
    CDataMySQL(Instance *piParent, int argc, char **argv);
    ~CDataMySQL();

    DCOP_DECLARE_INSTANCE;

    DWORD Init(DWORD dwAttrID, 
                IObject *pOwner, 
                IModel *piModel);

    void Dump(LOG_PRINT logPrint, LOG_PARA logPara);

    DWORD AddRecord(DCOP_PARA_NODE *pReqPara, 
                DWORD dwReqParaCount, 
                void *pReqData, 
                DWORD dwReqDataLen, 
                DCOP_PARA_NODE **ppEvtPara, 
                DWORD *pdwEvtParaCount, 
                CDArray *pEvtData);

    DWORD DelRecord(BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                DCOP_PARA_NODE **ppEvtPara, 
                DWORD *pdwEvtParaCount, 
                CDArray *pEvtData);

    DWORD EditRecord(BYTE byCond, 
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
                CDArray *pEvtData);

    DWORD QueryRecord(BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                DCOP_PARA_NODE *pReqPara, 
                DWORD dwReqParaCount, 
                DCOP_PARA_NODE *&rpRspPara, 
                DWORD &rdwRspParaCount, 
                CDArray &aRspData);

    DWORD CountRecord(BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                DWORD &rdwCount);

    DWORD AddKeyIdx(DCOP_PARA_NODE *pIdxPara, 
                DWORD dwIdxParaCount);

    DWORD DelKeyIdx(DCOP_PARA_NODE *pIdxPara, 
                DWORD dwIdxParaCount);

    DWORD GetRecCount();

private:
    MYSQL *m_conn_ptr;
    ITlvToSQL *m_piTlvToSQL;
};


#endif // #ifndef _OBJDATA_MYSQL_H_

