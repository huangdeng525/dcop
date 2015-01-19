/// -------------------------------------------------
/// ObjModel_if.h : 对象模型建模公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJMODEL_IF_H_
#define _OBJMODEL_IF_H_

#include "Object_if.h"


/// -------------------------------------------------
/// 对象模型及建模的定义说明
/// -------------------------------------------------
/// 模型: 对象属性的定义，包括: '数据'、'方法'和'事件'
/// 建模: 就是按照设计关系将这些这些模型描述到内存中
/// -------------------------------------------------


/// -------------------------------------------------
/// 定义IModel版本号
/// -------------------------------------------------
INTF_VER(IModel, 1, 0, 0);


/// 表名最大长度
#define DCOP_TABLE_NAME_LEN                 32

/// 字段名最大长度
#define DCOP_FIELD_NAME_LEN                 16


/// -------------------------------------------------
/// 模型接口定义
/// -------------------------------------------------
interface IModel : public IObject
{
    /// 模型类型
    enum TYPE
    {
        TYPE_DATA = 0,                      // [0]数据
        TYPE_METHOD,                        // [1]方法
        TYPE_EVENT                          // [2]事件
    };

    /// 关键字类型
    enum KEY
    {
        KEY_NO = 0,                         // [0]非关键字
        KEY_YES                             // [1]是关键字
    };

    /// 字段类型
    enum ENUM_FIELD
    {
        FIELD_NULL = 0,                     // [0]无效字段
        FIELD_IDENTIFY,                     // [1]ID (忽略输入，自动生成唯一标识，范围: 1~N十进制)
        FIELD_BYTE,                         // [2]字节 (16进制无符号)
        FIELD_WORD,                         // [3]字 (16进制无符号)
        FIELD_DWORD,                        // [4]双字 (16进制无符号)
        FIELD_SHORT,                        // [5]short (十进制有符号)
        FIELD_INTEGER,                      // [6]int (十进制有符号)
        FIELD_NUMBER,                       // [7]正整数 (范围: 0~N十进制)
        FIELD_BOOL,                         // [8]bool (非0为true;0为false)
        FIELD_CHAR,                         // [9]char (字符)
        FIELD_STRING,                       // [10]字符串
        FIELD_BUFFER,                       // [11]数据段
        FIELD_DATE,                         // [12]日期(年月日)
        FIELD_TIME,                         // [13]时间(时分秒)
        FIELD_IP,                           // [14]IP地址
        FIELD_PTR,                          // [15]指针
        FIELD_TIMER,                        // [16]定时器
        FIELD_PASS,                         // [17]校验字

        FIELD_NUM
    };

    /// 字段基础
    struct Field
    {
        char m_fieldName[DCOP_FIELD_NAME_LEN];
        BYTE m_isKey;                       // 是否关键字(唯一性，可用来创建索引)
        BYTE m_fieldType;                   // 字段类型
        WORD m_fieldSize;                   // 字段大小
        DWORD m_defaultValue;               // 默认值(仅限数值,0表示无默认值)
        DWORD m_minValue;                   // 最小值(仅限数值,0表示无最小值)
        DWORD m_maxValue;                   // 最大值(仅限数值,0表示无最大值)
    };

    /// 字段关联
    struct Relation
    {
        DWORD m_relID;                      // 关联ID(指本表要关联出去的字段ID)(需要关联到唯一值字段)
        DWORD m_attrID;                     // 关联的属性ID
        DWORD m_fieldID;                    // 关联的字段ID
    };

    /// 注册表及字段
    virtual DWORD RegTable(
                    char tableName[DCOP_TABLE_NAME_LEN],
                    DWORD objID,            // 对象ID
                    DWORD attrID,           // 属性ID
                    TYPE attrType,          // 属性类型
                    Field *pFields,         // 字段列表
                    DWORD dwFieldCount,     // 字段个数
                    DWORD dwDefRecCount,    // 默认记录个数
                    Relation *pShips = 0,   // 关联列表
                    DWORD dwShipCount = 0   // 关联个数
                    ) = 0;

    /// 获取表名
    virtual const char *GetTableName(DWORD attrID) = 0;

    /// 获取表所属对象ID
    virtual DWORD GetObjID(DWORD attrID) = 0;

    /// 获取表类型
    virtual TYPE GetType(DWORD attrID) = 0;

    /// 获取字段
    virtual Field *GetFields(DWORD attrID, DWORD &rdwFieldCount) = 0;

    /// 获取默认记录数
    virtual DWORD GetDefRecCount(DWORD attrID) = 0;

    /// 获取关联字段
    virtual Relation *GetShips(DWORD attrID, DWORD &rdwShipCount) = 0;
};


#endif // #ifndef _OBJMODEL_IF_H_

