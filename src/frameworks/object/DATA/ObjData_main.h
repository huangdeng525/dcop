/// -------------------------------------------------
/// ObjData_main.h : 对象数据私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJDATA_MAIN_H_
#define _OBJDATA_MAIN_H_

#define INC_MAP

#include "ObjData_if.h"
#include "ObjModel_if.h"
#include "ObjData_handle.h"
#include "fs/file.h"


class CData : public IData
{
public:
    /// 记录MAP
    typedef std::map<DWORD, IDataHandle *> MAP_DATA;
    typedef MAP_DATA::iterator IT_DATA;

public:
    CData(Instance *piParent, int argc, char **argv);
    ~CData();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();

    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

    DWORD Create(DWORD dwType, 
                DWORD dwAttrID, 
                IObject *pOwner, 
                Handle &hData);

    DWORD Destroy(DWORD attrID);

    DWORD AddRecord(Handle hData, 
                DCOP_PARA_NODE *pReqPara, 
                DWORD dwReqParaCount, 
                void *pReqData, 
                DWORD dwReqDataLen, 
                DCOP_PARA_NODE **ppEvtPara, 
                DWORD *pdwEvtParaCount, 
                CDArray *pEvtData);

    DWORD DelRecord(Handle hData, 
                BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                DCOP_PARA_NODE **ppEvtPara, 
                DWORD *pdwEvtParaCount, 
                CDArray *pEvtData);

    DWORD EditRecord(Handle hData, 
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
                CDArray *pEvtData);

    DWORD QueryRecord(Handle hData, 
                BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                DCOP_PARA_NODE *pReqPara, 
                DWORD dwReqParaCount, 
                DCOP_PARA_NODE *&rpRspPara, 
                DWORD &rdwRspParaCount, 
                CDArray &aRspData);

    DWORD CountRecord(Handle hData, 
                BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                DWORD &rdwCount
                );

    DWORD AddKeyIdx(
                Handle hData, 
                DCOP_PARA_NODE *pIdxPara, 
                DWORD dwIdxParaCount
                );

    DWORD DelKeyIdx(
                Handle hData, 
                DCOP_PARA_NODE *pIdxPara, 
                DWORD dwIdxParaCount
                );

    Handle GetHandle(DWORD attrID);

private:
    MAP_DATA m_datas;

    char m_szDataFileDir[DCOP_FILE_NAME_LEN];       // DataFile目录

    IModel *m_piModel;
};


#endif // #ifndef _OBJDATA_MAIN_H_

