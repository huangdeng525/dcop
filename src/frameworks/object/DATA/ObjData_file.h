/// -------------------------------------------------
/// ObjData_file.h : 文件数据私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJDATA_FILE_H_
#define _OBJDATA_FILE_H_

#include "ObjData_index.h"
#include "ObjData_handle.h"
#include "cpu/bytes.h"
#include "string/dstring.h"


class CDataFile : public IDataHandle
{
public:
    /// 记录有效标识
    enum REC_FLAG
    {
        REC_FLAG_NULL = 0,
        REC_FLAG_VALID
    };

    /// 文件头
    struct FileHead
    {
        char m_tableName[DCOP_TABLE_NAME_LEN];  // 表名
        BYTE m_ver;                             // 版本号
        BYTE m_level;                           // 级别
        WORD m_headerSize;                      // 头部大小
        DWORD m_objectID;                       // 所属对象ID
        DWORD m_attrID;                         // 属性ID
        DWORD m_fieldCount;                     // 字段个数
        DWORD m_recordCount;                    // 记录个数
        DWORD m_totalCount;                     // 总个数(含删除的记录)
        DWORD m_curIdentify;                    // 当前标识
    };

    /// 文件组成 : 文件头信息(FileHead) + 字段信息(Field*N) + 记录信息(M*N)

    /// 文件头和字段信息都需要使用网络字节序保存，字节序转换规则
    static void BytesChangeFileHead(FileHead *pFileHead);
    static void BytesChangeFieldInfo(IModel::Field *pFileInfo);

public:
    CDataFile(Instance *piParent, int argc, char **argv);
    ~CDataFile();

    DCOP_DECLARE_INSTANCE;

    DWORD Init(DWORD dwAttrID, 
                IObject *pOwner, 
                IModel *piModel);

    void Dump(LOG_PRINT logPrint, LOG_PARA logPara = 0);

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

    DWORD GetRecCount() {return m_head.m_recordCount;}

    void SaveCurIdentify(DWORD curIdentify);

private:
    DWORD AddRecord(BYTE *pbyRec);

    DWORD DelRecord(DWORD dwRecNo);

    DWORD SetRecord(DWORD dwRecNo, BYTE *pbyRec);

    DWORD GetRecord(DWORD dwRecNo, BYTE *pbyRec);

    DWORD GetIdleRec();

    DWORD BldAllIdx();

private:
    FileHead m_head;                        // 头部缓存
    CDString m_fileName;                    // 文件名缓存
    CRecIdx m_recIdx;                       // 记录索引
};


#endif // #ifndef _OBJDATA_FILE_H_

