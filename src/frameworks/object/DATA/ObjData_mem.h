/// -------------------------------------------------
/// ObjData_mem.h : 内存数据私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJDATA_MEM_H_
#define _OBJDATA_MEM_H_

#include "ObjData_index.h"
#include "ObjData_page.h"
#include "ObjData_handle.h"


class CDataMem : public IDataHandle
{
public:
    CDataMem(Instance *piParent, int argc, char **argv);
    ~CDataMem();

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

    DWORD GetRecCount() {return m_memPage.GetRecCount();}

private:
    CMemPage m_memPage;                     // 内存页
    CRecIdx m_recIdx;                       // 记录索引
};

#endif // #ifndef _OBJDATA_MEM_H_

