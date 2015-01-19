/// -------------------------------------------------
/// ObjData_if.h : 对象数据公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJDATA_IF_H_
#define _OBJDATA_IF_H_

#include "Object_if.h"
#include "BaseMessage.h"
#include "array/darray.h"


/// -------------------------------------------------
/// 对象数据的定义说明
/// -------------------------------------------------
/// 主要用于托管对象属性中的"数据"
/// 数据托管的意义:
///     可以把对象的数据统一管理起来，直接完成数据表的
///     内存数据操作、对应的数据库存库、恢复等相关操作
///     (表结构按照属性值对应的模型)
/// -------------------------------------------------


/// -------------------------------------------------
/// 定义IData版本号
/// -------------------------------------------------
INTF_VER(IData, 1, 0, 0);


/// -------------------------------------------------
/// 数据接口定义
/// -------------------------------------------------
interface IData : public IObject
{
    /// 句柄类型定义
    enum TYPE
    {
        TYPE_MEM = 0,                       // [0]内存
        TYPE_FILE,                          // [1]文件
        TYPE_MYSQL                          // [2]MySQL数据库
    };

    /// 数据句柄
    typedef void * Handle;

    /// 创建一个数据
    virtual DWORD Create(
                DWORD dwType, 
                DWORD attrID, 
                IObject *pOwner, 
                Handle &hData
                ) = 0;

    /// 销毁一个数据
    virtual DWORD Destroy(
                DWORD attrID
                ) = 0;

    /// 添加一条记录
    virtual DWORD AddRecord(
                Handle hData, 
                DCOP_PARA_NODE *pReqPara, 
                DWORD dwReqParaCount, 
                void *pReqData, 
                DWORD dwReqDataLen, 
                DCOP_PARA_NODE **ppEvtPara = 0, 
                DWORD *pdwEvtParaCount = 0, 
                CDArray *pEvtData = 0
                ) = 0;

    /// 删除一条记录
    virtual DWORD DelRecord(
                Handle hData, 
                BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                DCOP_PARA_NODE **ppEvtPara = 0, 
                DWORD *pdwEvtParaCount = 0, 
                CDArray *pEvtData = 0
                ) = 0;

    /// 编辑一条记录
    virtual DWORD EditRecord(
                Handle hData, 
                BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                DCOP_PARA_NODE *pReqPara, 
                DWORD dwReqParaCount, 
                void *pReqData, 
                DWORD dwReqDataLen, 
                DCOP_PARA_NODE **ppEvtPara = 0, 
                DWORD *pdwEvtParaCount = 0, 
                CDArray *pEvtData = 0
                ) = 0;

    /// 查询多条记录
    virtual DWORD QueryRecord(
                Handle hData, 
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

    /// 统计记录数量
    virtual DWORD CountRecord(
                Handle hData, 
                BYTE byCond, 
                DCOP_PARA_NODE *pCondPara, 
                DWORD dwCondParaCount, 
                void *pCondData, 
                DWORD dwCondDataLen, 
                DWORD &rdwCount
                ) = 0;

    /// 添加关键字索引
    virtual DWORD AddKeyIdx(
                Handle hData, 
                DCOP_PARA_NODE *pIdxPara, 
                DWORD dwIdxParaCount
                ) = 0;

    /// 删除关键字索引
    virtual DWORD DelKeyIdx(
                Handle hData, 
                DCOP_PARA_NODE *pIdxPara, 
                DWORD dwIdxParaCount
                ) = 0;

};


#endif // #ifndef _OBJDATA_IF_H_

