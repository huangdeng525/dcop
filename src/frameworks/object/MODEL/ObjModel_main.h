/// -------------------------------------------------
/// ObjModel_main.h : 对象模型建模私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJMODEL_MAIN_H_
#define _OBJMODEL_MAIN_H_

#define INC_MAP

#include "ObjModel_if.h"
#include "ObjDispatch_if.h"
#include "ObjNotify_if.h"


/// 默认记录数
#define DCOP_MODEL_DEF_REC_COUNT            16


class CModel : public IModel
{
public:
    /// 表信息
    struct Table
    {
        char m_szTableName[DCOP_TABLE_NAME_LEN];
        DWORD m_objID;
        DWORD m_attrID;
        TYPE m_attrType;
        Field *m_pFields;
        DWORD m_dwFieldCount;
        DWORD m_dwDefRecCount;
        Relation *m_pShips;
        DWORD m_dwShipCount;

        Table();
        ~Table();

        DWORD Create(const char *cszTableName, 
                        DWORD objID, 
                        DWORD attrID, 
                        TYPE attrType, 
                        Field *pFields, 
                        DWORD dwFieldCount, 
                        DWORD dwDefRecCount, 
                        Relation *pShips, 
                        DWORD dwShipCount);
    };

    typedef std::map<DWORD, Table> MAP_TABLES;
    typedef MAP_TABLES::iterator IT_TABLES;

public:
    CModel(Instance *piParent, int argc, char **argv);
    ~CModel();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DCOP_DECLARE_IOBJECT_MSG_HANDLE;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();

    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

    DWORD RegTable(char tableName[DCOP_TABLE_NAME_LEN],
                        DWORD objID,
                        DWORD attrID,
                        TYPE attrType,
                        Field *pFields,
                        DWORD dwFieldCount,
                        DWORD dwDefRecCount, 
                        Relation *pShips, 
                        DWORD dwShipCount);

    const char *GetTableName(DWORD attrID);

    DWORD GetObjID(DWORD attrID);

    TYPE GetType(DWORD attrID);

    Field *GetFields(DWORD attrID, DWORD &rdwFieldCount);

    DWORD GetDefRecCount(DWORD attrID);

    Relation *GetShips(DWORD attrID, DWORD &rdwShipCount);

    void OnStart(objMsg *msg);
    void OnDefault(objMsg *msg);

private:
    void SendEvent(DWORD attrID = 0);

private:
    MAP_TABLES m_tables;
    IDispatch *m_piDispatch;                        // 消息发送器
    INotify *m_piNotify;                            // 事件通知器
    INotify::IPool *m_pNotifyPool;                  // 事件缓冲池
    bool m_bStartFlag;
};

#endif // #ifndef _OBJMODEL_MAIN_H_

