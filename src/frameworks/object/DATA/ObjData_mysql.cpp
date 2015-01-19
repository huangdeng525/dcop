/// -------------------------------------------------
/// ObjData_mysql.cpp : MySQL数据库数据实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjData_mysql.h"
#include "Factory_if.h"
#include "string/tablestring.h"
#include "sock.h"
#include "ObjTimer_if.h"

#if DCOP_OS == DCOP_OS_WINDOWS
#pragma comment(lib, "mysqlclient")
#endif


/// -------------------------------------------------
/// 实现类厂(单件)
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CDataMySQL, "DataMySQL")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CDataMySQL)
    DCOP_IMPLEMENT_INTERFACE(IDataHandle)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 连接信息
/// -------------------------------------------------
const char *CDataMySQL::AddrInfo = "127.0.0.1";
const char *CDataMySQL::UserInfo = "root";
const char *CDataMySQL::ConnInfo = "Dcoplatform@31418";
const char *CDataMySQL::BaseInfo = "DcopDataBase";


/*******************************************************
  函 数 名: CDataMySQL::CDataMySQL
  描    述: CDataMySQL构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDataMySQL::CDataMySQL(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);

    m_conn_ptr = NULL;
    m_piTlvToSQL = NULL;
}

/*******************************************************
  函 数 名: CDataMySQL::~CDataMySQL
  描    述: CDataMySQL析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDataMySQL::~CDataMySQL()
{
    if (m_conn_ptr)
    {
        mysql_close(m_conn_ptr);
        m_conn_ptr = NULL;
    }

    DCOP_RELEASE_INSTANCE(m_piTlvToSQL);
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CDataMySQL::Init
  描    述: 初始化
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMySQL::Init(DWORD dwAttrID, IObject *pOwner, IModel *piModel)
{
    /// 因为本实例是单件，所以只能初始化一次，已经初始化了，就直接返回成功
    if (m_conn_ptr)
    {
        return SUCCESS;
    }

    /// 先调用父类的初始化
    DWORD dwRc = IDataHandle::Init(IData::TYPE_MYSQL, dwAttrID, pOwner, piModel, (IData *)m_piParent);
    if (dwRc)
    {
        return dwRc;
    }

    /// 创建SQL语句转换实例
    DCOP_CREATE_INSTANCE(ITlvToSQL, "TLV2SQL", this, 0, 0, m_piTlvToSQL);
    if(!m_piTlvToSQL)
    {
        return FAILURE;
    }

    /// 初始化mysql : mysql_init()输入NULL指针，里面将自动分配新的对象；当调用mysql_close()时将释放该对象
    m_conn_ptr = mysql_init(NULL);
    if(!m_conn_ptr)
    {
        return FAILURE;
    }

    /// 连接数据库 : mysql_connect已经过时，使用mysql_real_connect()取而代之；db为NULL，连接默认的数据库
    MYSQL *conn_ptr = mysql_real_connect(m_conn_ptr, AddrInfo, UserInfo, ConnInfo, 
                        BaseInfo, 0, NULL, 0);
    if(!conn_ptr)
    {
        TRACE_LOG(mysql_error(m_conn_ptr));
        return FAILURE;
    }

    /// 获取创建参数列表
    CDString strCreateArgList;
    dwRc = m_piTlvToSQL->GetCreateFieldList(strCreateArgList);
    if (dwRc)
    {
        return dwRc;
    }

    /// 创建数据表 : mysql_create_db已经过时，使用mysql_query()发出SQL CREATE DATABASE语句来进行创建
    /// CREATE TABLE IF NOT EXISTS table_name (i INT NOT NULL)
    CDString strSQL("CREATE TABLE IF NOT EXISTS ");
    strSQL << GetTableName() << " (" << strCreateArgList << ")";
    int iRc = mysql_query(m_conn_ptr, strSQL);
    if (iRc != 0)
    {
        TRACE_LOG(strSQL);
        TRACE_LOG(mysql_error(m_conn_ptr));
        return FAILURE;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMySQL::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDataMySQL::Dump(LOG_PRINT logPrint, LOG_PARA logPara)
{
    if (!logPrint)
    {
        return;
    }

    Field *pFields = GetFields();
    DWORD dwFieldCount = GetFieldCount();
    if (!pFields || !dwFieldCount)
    {
        return;
    }

    /// SELECT * FROM table_name
    CDString strSQL("SELECT * FROM ");
    strSQL << GetTableName();
    int iRc = mysql_real_query(m_conn_ptr, strSQL, strSQL.Length());
    if (iRc != 0)
    {
        TRACE_LOG(strSQL);
        TRACE_LOG(mysql_error(m_conn_ptr));
        return;
    }

    MYSQL_RES *res = mysql_store_result(m_conn_ptr);
    if (!res)
    {
        TRACE_LOG(mysql_error(m_conn_ptr));
        return;
    }

    DWORD dwRecCount = (DWORD)mysql_num_rows(res);
    logPrint(STR_FORMAT("MySQL Data Table: ['%s'], Records Count: %d \r\n", GetTableName(), dwRecCount), logPara);
    CTableString tableStr(dwFieldCount, dwRecCount + 1, "  ");

    for (DWORD i = 0; i < dwFieldCount; ++i)
    {
        tableStr << pFields[i].m_fieldName;
    }

    DCOP_START_TIME();

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        for (DWORD i = 0; i < dwFieldCount; ++i)
        {
            if (pFields[i].m_fieldType == IModel::FIELD_PASS)
            {
                tableStr << "********";
                continue;
            }

            if (!(row[i]))
            {
                tableStr << "";
                continue;
            }

            if (pFields[i].m_fieldType == IModel::FIELD_BUFFER)
            {
                tableStr << CBufferString((void *)(row[i]), pFields[i].m_fieldSize);
                continue;
            }

            if (pFields[i].m_fieldType == IModel::FIELD_IP)
            {
                char szIP[OSSOCK_IPSIZE];
                (void)memset(szIP, 0, sizeof(szIP));
                objSock::GetIPStringByValue((DWORD)atoi(row[i]), szIP);
                tableStr << szIP;
                continue;
            }

            if (pFields[i].m_fieldType == IModel::FIELD_PTR)
            {
                tableStr << STR_FORMAT("%p", *(void **)(row[i]));
                continue;
            }

            if (pFields[i].m_fieldType == IModel::FIELD_TIMER)
            {
                CDString strTimer;
                ITimer::IWheel::GetString((ITimer::Handle)(row[i]), strTimer);
                tableStr << (const char *)strTimer;
                continue;
            }

            tableStr << row[i];
        }
    }

    DCOP_END_TIME();

    tableStr.Show(logPrint, logPara, "=", "-");

    logPrint(STR_FORMAT("[cost time: %d ms] \r\n", DCOP_COST_TIME()), logPara);

    mysql_free_result(res);
}

/*******************************************************
  函 数 名: CDataMySQL::AddRecord
  描    述: 添加一条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMySQL::AddRecord(DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
                        void *pReqData, DWORD dwReqDataLen, 
                        DCOP_PARA_NODE **ppEvtPara, DWORD *pdwEvtParaCount, 
                        CDArray *pEvtData)
{
    if (!m_conn_ptr || !m_piTlvToSQL)
    {
        return FAILURE;
    }

    CDString strArgList;
    DWORD dwRc = m_piTlvToSQL->GetInsertFieldList(pReqPara, dwReqParaCount, 
                        pReqData, dwReqDataLen, 
                        strArgList);
    if (dwRc)
    {
        return dwRc;
    }

    /// INSERT INTO table_name (a,b,c) VALUES (1,2,3)
    CDString strSQL("INSERT INTO ");
    strSQL << GetTableName() << " " << strArgList;

    int iRc = mysql_query(m_conn_ptr, strSQL);
    if (iRc != 0)
    {
        TRACE_LOG(strSQL);
        TRACE_LOG(mysql_error(m_conn_ptr));
        return FAILURE;
    }

    /// 触发事件
    if (ppEvtPara && pdwEvtParaCount && pEvtData)
    {
        CDString strIdName;
        dwRc = m_piTlvToSQL->GetIdentifyName(strIdName);
        if (dwRc != SUCCESS)
        {
            return SUCCESS;
        }

        CDString strEvtSQL = "SELECT * FROM ";
        strEvtSQL << GetTableName() << " WHERE " << strIdName << "=LAST_INSERT_ID()";
        int iRc = mysql_real_query(m_conn_ptr, strEvtSQL, strEvtSQL.Length());
        if (iRc != 0)
        {
            return SUCCESS;
        }

        MYSQL_RES *res = mysql_store_result(m_conn_ptr);
        if (!res)
        {
            return SUCCESS;
        }

        /// 获取事件数据
        (void)m_piTlvToSQL->GetRspFieldList(0, 0, res, 
                        *ppEvtPara, *pdwEvtParaCount, *pEvtData);
        mysql_free_result(res);
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMySQL::DelRecord
  描    述: 删除一条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMySQL::DelRecord(BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        DCOP_PARA_NODE **ppEvtPara, DWORD *pdwEvtParaCount, 
                        CDArray *pEvtData)
{
    if (!m_conn_ptr || !m_piTlvToSQL)
    {
        return FAILURE;
    }

    CDString strCondArgList;
    DWORD dwRc = m_piTlvToSQL->GetCondFieldList(byCond, 
                        pCondPara, dwCondParaCount, pCondData, dwCondDataLen, 
                        strCondArgList);
    if (dwRc)
    {
        return dwRc;
    }

    /// 触发事件
    if (ppEvtPara && pdwEvtParaCount && pEvtData)
    {
        CDString strEvtSQL = "SELECT * FROM ";
        strEvtSQL << GetTableName() << " WHERE " << strCondArgList;
        int iRc = mysql_real_query(m_conn_ptr, strEvtSQL, strEvtSQL.Length());
        if (iRc != 0)
        {
            return SUCCESS;
        }

        MYSQL_RES *res = mysql_store_result(m_conn_ptr);
        if (!res)
        {
            return SUCCESS;
        }

        /// 获取事件数据
        (void)m_piTlvToSQL->GetRspFieldList(0, 0, res, 
                        *ppEvtPara, *pdwEvtParaCount, *pEvtData);
        mysql_free_result(res);
    }

    /// DELETE FROM table_name WHERE user='jcole'
    CDString strSQL("DELETE FROM ");
    strSQL << GetTableName();
    if (strCondArgList.Length()) strSQL << " WHERE " << strCondArgList;

    int iRc = mysql_query(m_conn_ptr, strSQL);
    if (iRc != 0)
    {
        TRACE_LOG(strSQL);
        TRACE_LOG(mysql_error(m_conn_ptr));

        if (ppEvtPara && pdwEvtParaCount && pEvtData)
        {
            *ppEvtPara = 0;
            *pdwEvtParaCount = 0;
            pEvtData->Clear();
        }

        return FAILURE;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMySQL::EditRecord
  描    述: 编辑一条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMySQL::EditRecord(BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
                        void *pReqData, DWORD dwReqDataLen, 
                        DCOP_PARA_NODE **ppEvtPara, DWORD *pdwEvtParaCount, 
                        CDArray *pEvtData)
{
    if (!m_conn_ptr || !m_piTlvToSQL)
    {
        return FAILURE;
    }

    CDString strCondArgList;
    DWORD dwRc = m_piTlvToSQL->GetCondFieldList(byCond, 
                        pCondPara, dwCondParaCount, pCondData, dwCondDataLen, 
                        strCondArgList);
    if (dwRc)
    {
        return dwRc;
    }

    CDString strReqArgList;
    dwRc = m_piTlvToSQL->GetReqFieldList(pReqPara, dwReqParaCount, 
                        pReqData, dwReqDataLen, strReqArgList);
    if (dwRc)
    {
        return dwRc;
    }

    /// UPDATE table_name SET age=age*2 WHERE id=1
    CDString strSQL("UPDATE ");
    strSQL << GetTableName() << " SET " << strReqArgList;
    if (strCondArgList.Length()) strSQL << " WHERE " << strCondArgList;

    int iRc = mysql_query(m_conn_ptr, strSQL);
    if (iRc != 0)
    {
        TRACE_LOG(strSQL);
        TRACE_LOG(mysql_error(m_conn_ptr));
        return FAILURE;
    }

    /// 触发事件
    if (ppEvtPara && pdwEvtParaCount && pEvtData)
    {
        DWORD dwEvtParaCount = 0;
        DWORD dwEvtDataLen = 0;
        DCOP_PARA_NODE *pEvtPara = GetOutPara(GetFields(), GetFieldCount(), dwEvtParaCount, 
                        pReqPara, dwReqParaCount, &dwEvtDataLen, true);
        if (!pEvtPara)
        {
            return SUCCESS;
        }

        CDString strEvtArgNameList;
        dwRc = m_piTlvToSQL->GetFieldNameList(pEvtPara, dwEvtParaCount, strEvtArgNameList);
        if (dwRc)
        {
            DCOP_Free(pEvtPara);
            return SUCCESS;
        }

        CDString strEvtSQL("SELECT ");
        strEvtSQL << strEvtArgNameList << " FROM " << GetTableName();
        if (strCondArgList.Length()) strEvtSQL << " WHERE " << strCondArgList;
        int iRc = mysql_real_query(m_conn_ptr, strEvtSQL, strEvtSQL.Length());
        if (iRc != 0)
        {
            DCOP_Free(pEvtPara);
            return SUCCESS;
        }

        MYSQL_RES *res = mysql_store_result(m_conn_ptr);
        if (!res)
        {
            DCOP_Free(pEvtPara);
            return SUCCESS;
        }

        /// 获取事件数据
        (void)m_piTlvToSQL->GetRspFieldList(pEvtPara, dwEvtParaCount, res, 
                        *ppEvtPara, *pdwEvtParaCount, *pEvtData);
        DCOP_Free(pEvtPara);
        mysql_free_result(res);
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMySQL::QueryRecord
  描    述: 查询多条记录
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMySQL::QueryRecord(BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        DCOP_PARA_NODE *pReqPara, DWORD dwReqParaCount, 
                        DCOP_PARA_NODE *&rpRspPara, DWORD &rdwRspParaCount, 
                        CDArray &aRspData)
{
    if (!m_conn_ptr || !m_piTlvToSQL)
    {
        return FAILURE;
    }

    /// 获取条件(无条件也会返回成功,所以失败就需要退出)
    CDString strCondArgList;
    DWORD dwRc = m_piTlvToSQL->GetCondFieldList(byCond, 
                        pCondPara, dwCondParaCount, pCondData, dwCondDataLen, 
                        strCondArgList);
    if (dwRc)
    {
        return dwRc;
    }

    /// 获取字段名称列表
    CDString strArgNameList;
    CDString strJoinTable;
    dwRc = m_piTlvToSQL->GetFieldNameList(pReqPara, dwReqParaCount, strArgNameList, ", ", &strJoinTable);
    if (dwRc)
    {
        return dwRc;
    }

    /// SELECT columna, columnb FROM table_name WHERE a='test'
    CDString strSQL("SELECT ");
    strSQL << strArgNameList << " FROM " << GetTableName();
    if (strJoinTable.Length()) strSQL << strJoinTable;
    if (strCondArgList.Length()) strSQL << " WHERE " << strCondArgList;

    /// 获取记录行
    DWORD dwReqOffset = 0;
    DWORD dwReqLimit = 0;
    GetOffsetAndLimit(dwReqOffset, dwReqLimit, 
                        pCondPara, dwCondParaCount, 
                        pCondData, dwCondDataLen);
    if (dwReqOffset || dwReqLimit)
    {
        if (!dwReqLimit) dwReqLimit = 0xffffffff;
        strSQL << STR_FORMAT(" LIMIT %lu,%lu", dwReqOffset, dwReqLimit);
    }

    /// 执行查询(因为可能包含二进制，所以使用'mysql_real_query')
    int iRc = mysql_real_query(m_conn_ptr, strSQL, strSQL.Length());
    if (iRc != 0)
    {
        TRACE_LOG(strSQL);
        TRACE_LOG(mysql_error(m_conn_ptr));
        return FAILURE;
    }

    /// 调用mysql_store_result函数将从Mysql服务器查询的所有数据都存储到客户端，然后读取;
    /// 调用mysql_use_result初始化检索，以便于后面一行一行的读取结果集，而它本身并没有从
    /// 服务器读取任何数据，这种方式较之第一种速度更快且所需内存更少，但它会绑定服务器，
    /// 阻止其他线程更新任何表，而且必须重复执行mysql_fetch_row读取数据，直至返回NULL，否
    /// 则未读取的行会在下一次查询时作为结果的一部分返回，故经常我们使用mysql_store_result.
    MYSQL_RES *res = mysql_store_result(m_conn_ptr);
    if (!res)
    {
        TRACE_LOG(mysql_error(m_conn_ptr));
        return FAILURE;
    }

    /// 获取响应数据
    dwRc = m_piTlvToSQL->GetRspFieldList(pReqPara, dwReqParaCount, 
                        res, rpRspPara, rdwRspParaCount, aRspData);
    mysql_free_result(res);

    return dwRc;
}

/*******************************************************
  函 数 名: CDataMySQL::CountRecord
  描    述: 统计记录数量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMySQL::CountRecord(BYTE byCond, 
                        DCOP_PARA_NODE *pCondPara, DWORD dwCondParaCount, 
                        void *pCondData, DWORD dwCondDataLen, 
                        DWORD &rdwCount)
{
    if (!m_conn_ptr || !m_piTlvToSQL)
    {
        return FAILURE;
    }

    CDString strCondArgList;
    DWORD dwRc = m_piTlvToSQL->GetCondFieldList(byCond, 
                        pCondPara, dwCondParaCount, pCondData, dwCondDataLen, 
                        strCondArgList);
    if (dwRc)
    {
        return dwRc;
    }

    /// SELECT count(*) FROM table_name WHERE a='test'
    CDString strSQL("SELECT count(*) FROM ");
    strSQL << GetTableName();
    if (strCondArgList.Length()) strSQL << " WHERE " << strCondArgList;

    int iRc = mysql_real_query(m_conn_ptr, strSQL, strSQL.Length());
    if (iRc != 0)
    {
        TRACE_LOG(strSQL);
        TRACE_LOG(mysql_error(m_conn_ptr));
        return FAILURE;
    }

    /// 获取结果
    MYSQL_RES *res = mysql_store_result(m_conn_ptr);
    if (!res)
    {
        TRACE_LOG(mysql_error(m_conn_ptr));
        return FAILURE;
    }

    /// 获取数量
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row)
    {
        TRACE_LOG(mysql_error(m_conn_ptr));
        mysql_free_result(res);
        return FAILURE;
    }

    rdwCount = (DWORD)atoi(row[0]);

    mysql_free_result(res);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMySQL::AddKeyIdx
  描    述: 添加关键字索引
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMySQL::AddKeyIdx(DCOP_PARA_NODE *pIdxPara, DWORD dwIdxParaCount)
{
    if (!m_conn_ptr || !m_piTlvToSQL)
    {
        return FAILURE;
    }

    /// 获取索引名(由各个字段名组成)
    CDString strIdxName("idx_");
    DWORD dwRc = m_piTlvToSQL->GetFieldNameList(pIdxPara, dwIdxParaCount, strIdxName, "_");
    if (dwRc)
    {
        return dwRc;
    }

    /// 索引名不能超过最大长度，否则移除超出部分
    if (strIdxName.Length() > ColNameMaxLen)
    {
        strIdxName.Remove(ColNameMaxLen);
    }

    /// 获取字段名称列表
    CDString strArgNameList;
    dwRc = m_piTlvToSQL->GetFieldNameList(pIdxPara, dwIdxParaCount, strArgNameList);
    if (dwRc)
    {
        return dwRc;
    }

    /// CREATE UNIQUE INDEX index_name ON table_name (column_list)
    CDString strSQL("CREATE UNIQUE INDEX ");
    strSQL << strIdxName << " ON " << GetTableName() << "(" << strArgNameList << ")";

    int iRc = mysql_query(m_conn_ptr, strSQL);
    if (iRc != 0)
    {
        TRACE_LOG(strSQL);
        TRACE_LOG(mysql_error(m_conn_ptr));
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMySQL::DelKeyIdx
  描    述: 删除关键字索引
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMySQL::DelKeyIdx(DCOP_PARA_NODE *pIdxPara, DWORD dwIdxParaCount)
{
    if (!m_conn_ptr || !m_piTlvToSQL)
    {
        return FAILURE;
    }

    /// 获取索引名(由各个字段名组成)
    CDString strIdxName("idx_");
    DWORD dwRc = m_piTlvToSQL->GetFieldNameList(pIdxPara, dwIdxParaCount, strIdxName, "_");
    if (dwRc)
    {
        return dwRc;
    }

    /// 索引名不能超过最大长度，否则移除超出部分
    if (strIdxName.Length() > ColNameMaxLen)
    {
        strIdxName.Remove(ColNameMaxLen);
    }

    /// DROP INDEX index_name ON table_name
    CDString strSQL("DROP INDEX ");
    strSQL << strIdxName << " ON " << GetTableName();

    int iRc = mysql_query(m_conn_ptr, strSQL);
    if (iRc != 0)
    {
        TRACE_LOG(strSQL);
        TRACE_LOG(mysql_error(m_conn_ptr));
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDataMySQL::GetRecCount
  描    述: 获取记录数量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDataMySQL::GetRecCount()
{
    if (!m_conn_ptr)
    {
        return 0;
    }

    /// SELECT count(*) FROM table_name
    CDString strSQL("SELECT count(*) FROM ");
    strSQL << GetTableName();
    int iRc = mysql_real_query(m_conn_ptr, strSQL, strSQL.Length());
    if (iRc != 0)
    {
        TRACE_LOG(strSQL);
        TRACE_LOG(mysql_error(m_conn_ptr));
        return 0;
    }

    /// 获取结果
    MYSQL_RES *res = mysql_store_result(m_conn_ptr);
    if (!res)
    {
        TRACE_LOG(mysql_error(m_conn_ptr));
        return 0;
    }

    /// 获取数量
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row)
    {
        TRACE_LOG(mysql_error(m_conn_ptr));
        mysql_free_result(res);
        return 0;
    }

    DWORD dwCount = (DWORD)atoi(row[0]);

    mysql_free_result(res);
    return dwCount;
}

