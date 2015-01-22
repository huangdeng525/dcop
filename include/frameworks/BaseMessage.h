/// -------------------------------------------------
/// BaseMessage.h : 基本消息类型定义公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _BASEMESSAGE_H_
#define _BASEMESSAGE_H_

#include "BaseID.h"
#include "cpu/bytes.h"


/// -------------------------------------------------
/// 本文件主要涉及'消息类型'和'消息结构'定义:
///     '消息类型': 用来分流消息
///     '消息结构': 用来解析消息
/// -------------------------------------------------
/// 关于'消息结构':
///     (没有一个固定的消息结构，而是采用多个消息结构
/// 进行组合，达到多种操作的目的，类似一个语法结构 - 
/// 可参考SQL语句 select 'fieldA fieldB fieldC' from 'table1' where 'fieldD >= 6')
/// -------------------------------------------------


/// -------------------------------------------------
/// 常用'消息结构'组合:
/// -------------------------------------------------
/// '创建'/'销毁'对象属性,'添加'/'请求'的消息参数结构:
///     DCOP_SESSION_HEAD + DCOP_REQUEST_HEAD + Para + Data
/// -------------------------------------------------
/// '删除'/'统计'对象属性记录的消息参数结构:
///     DCOP_SESSION_HEAD + DCOP_CONDITION_HEAD + Para + Data + DCOP_CONDITION_HEAD + Data + ...
/// -------------------------------------------------
/// '编辑'/'查询'对象属性记录的消息参数结构:
///     DCOP_SESSION_HEAD + DCOP_CONDITION_HEAD + Para + Data + DCOP_REQUEST_HEAD + Para + Data
///     DCOP_SESSION_HEAD + DCOP_CONDITION_HEAD + Para + Data + DCOP_CONDITION_HEAD + Data + DCOP_REQUEST_HEAD + Para + Data
/// -------------------------------------------------
/// '响应'的消息参数结构: (多条记录，Para只需要出现一次)
///     DCOP_SESSION_HEAD + DCOP_RESPONSE_HEAD + Para + Data + DCOP_RESPONSE_HEAD + Data + DCOP_RESPONSE_HEAD + Data + ...
/// -------------------------------------------------
/// '事件'的消息参数结构:
///     DCOP_SESSION_HEAD + DCOP_EVENT_HEAD + Para + Data + DCOP_EVENT_HEAD + Data + DCOP_EVENT_HEAD + Data + ...
/// -------------------------------------------------
/// 相同类型的多个消息头合并到一个消息包中:
///     DCOP_SESSION_HEAD + ... + DCOP_SESSION_HEAD + ...
/// -------------------------------------------------


/// -------------------------------------------------
/// '消息结构'和'Para'以及'Value'的关系
/// -------------------------------------------------
///     'Para'用来描述字段，使用'DCOP_PARA_NODE'结构
///     在'消息结构'中使用'paraCount'来描述
/// -------------------------------------------------
///     'Value'是纯二进制数据，必须按照字段列表来解析
///     在'消息结构'中使用'paraLen'来描述
/// -------------------------------------------------
///     [1] 一个消息包中或者记录中至少有一个'Para'描述
///     [2] 相同的数据可使用最开始的'消息结构'中的描述
///     [3] 不同的字段可单独在'消息结构'描述，但必须在
///         同一条记录里面
///     [4] '请求'结构在一个'会话'只能有一个，'条件'、
///         '响应'和'事件'可以有多个
/// -------------------------------------------------


/// -------------------------------------------------
/// 头部类型 : 每一个消息头都有的一个共同成员，用来标识一个消息头部是哪种类型
/// -------------------------------------------------
typedef struct tagDCOP_MSG_HEAD
{
    BYTE m_headType;                        // 头部类型(见'DCOP_MSG_HEAD_TYPE')
    BYTE m_headSize;                        // 头部大小(见'DCOP_MSG_HEAD_SIZE')
    WORD m_valueLen;                        // 头部后面的值的长度
}DCOP_MSG_HEAD;


/// -------------------------------------------------
/// 会话消息头
/// -------------------------------------------------
///     会话 : 用户登录后的会话信息
///     属性 : 对象的属性值(在资源建模时确定)
///     索引 : 消息的序列号
///     控制 : 数据 / 方法 / 事件
/// -------------------------------------------------
typedef struct tagDCOP_SESSION_HEAD
{
    DCOP_MSG_HEAD m_type;                   // 基本类型
    WORD m_ver;                             // 版本(见'DCOP_SESSION_VER')
    WORD m_count;                           // 携带的消息节点数量
    DWORD m_session;                        // 会话号
    DWORD m_user;                           // 用户ID
    DWORD m_tty;                            // 终端类型ID
    DWORD m_attribute;                      // 属性ID
    WORD m_index;                           // 索引值
    BYTE m_ctrl;                            // 控制(见'DCOP_CTRL_TYPE')
    BYTE m_ack;                             // 应答(见'DCOP_ACK_TYPE')
}DCOP_SESSION_HEAD;


/// -------------------------------------------------
/// 条件消息头
/// -------------------------------------------------
typedef struct tagDCOP_CONDITION_HEAD
{
    DCOP_MSG_HEAD m_type;                   // 基本类型
    BYTE m_condition;                       // 条件(见'DCOP_CONDITION_TYPE')
    BYTE m_paraCount;                       // 参数个数
    WORD m_paraLen;                         // 参数长度
}DCOP_CONDITION_HEAD;


/// -------------------------------------------------
/// 请求消息头
/// -------------------------------------------------
typedef struct tagDCOP_REQUEST_HEAD
{
    DCOP_MSG_HEAD m_type;                   // 基本类型
    WORD m_paraCount;                       // 参数个数
    WORD m_paraLen;                         // 参数长度
}DCOP_REQUEST_HEAD;


/// -------------------------------------------------
/// 响应消息头
/// -------------------------------------------------
typedef struct tagDCOP_RESPONSE_HEAD
{
    DCOP_MSG_HEAD m_type;                   // 基本类型
    DWORD m_retCode;                        // 返回码(0为正确,非0为错误码)
    DWORD m_recordCount;                    // 记录条数
    DWORD m_recordIndex;                    // 本次记录索引
    WORD m_paraCount;                       // 参数个数
    WORD m_paraLen;                         // 参数长度
}DCOP_RESPONSE_HEAD;


/// -------------------------------------------------
/// 事件消息头
/// -------------------------------------------------
typedef struct tagDCOP_EVENT_HEAD
{
    DCOP_MSG_HEAD m_type;                   // 基本类型
    DWORD m_recordCount;                    // 记录条数
    DWORD m_recordIndex;                    // 本次记录索引
    WORD m_paraCount;                       // 参数个数
    WORD m_paraLen;                         // 参数长度
}DCOP_EVENT_HEAD;


/// -------------------------------------------------
/// 参数节点
/// -------------------------------------------------
typedef struct tagDCOP_PARA_NODE
{
    DWORD m_paraID;                         // 参数ID(从1开始的编号,0表示所有参数;另外有特殊参数类型,见'DCOP_SPECPARA_TYPE')
    BYTE m_opCode;                          // 操作符(见'DCOP_OPCODE_TYPE')
    BYTE m_paraType;                        // 参数类型(具体定义和实际使用有关，如果使用的对象属性建模，那就是数据字段)
    WORD m_paraSize;                        // 参数大小
}DCOP_PARA_NODE;


/// -------------------------------------------------
/// 头部类型定义
/// -------------------------------------------------
enum DCOP_MSG_HEAD_TYPE
{
    DCOP_MSG_HEAD_SESSION = 0,              // 会话消息头
    DCOP_MSG_HEAD_CONDITION,                // 条件消息头
    DCOP_MSG_HEAD_REQUEST,                  // 请求消息头
    DCOP_MSG_HEAD_RESPONSE,                 // 响应消息头
    DCOP_MSG_HEAD_EVENT,                    // 事件消息头

    DCOP_MSG_HEAD_COUNT                     // 消息头总数目
};


/// -------------------------------------------------
/// 头部版本定义[如有变化请在这里修改]
/// -------------------------------------------------
const BYTE DCOP_MSG_HEAD_SIZE[] = 
{
    sizeof(DCOP_SESSION_HEAD),              // 普通消息头大小
    sizeof(DCOP_CONDITION_HEAD),            // 条件消息头大小
    sizeof(DCOP_REQUEST_HEAD),              // 请求消息头大小
    sizeof(DCOP_RESPONSE_HEAD),             // 响应消息头大小
    sizeof(DCOP_EVENT_HEAD)                 // 事件消息头大小
};


/// -------------------------------------------------
/// SESSION头部字节序转换规则
/// -------------------------------------------------
#define BYTES_CHANGE_SESSION_HEAD_ORDER(pHead)      \
    const BYTES_CHANGE_RULE DCOP_SESSION_HEAD_BORULE[] = \
    { \
        {SIZE_OF(DCOP_MSG_HEAD, m_valueLen), OFFSET_OF(DCOP_MSG_HEAD, m_valueLen)}, \
        {SIZE_OF(DCOP_SESSION_HEAD, m_ver), OFFSET_OF(DCOP_SESSION_HEAD, m_ver)}, \
        {SIZE_OF(DCOP_SESSION_HEAD, m_count), OFFSET_OF(DCOP_SESSION_HEAD, m_count)}, \
        {SIZE_OF(DCOP_SESSION_HEAD, m_session), OFFSET_OF(DCOP_SESSION_HEAD, m_session)}, \
        {SIZE_OF(DCOP_SESSION_HEAD, m_user), OFFSET_OF(DCOP_SESSION_HEAD, m_user)}, \
        {SIZE_OF(DCOP_SESSION_HEAD, m_tty), OFFSET_OF(DCOP_SESSION_HEAD, m_tty)}, \
        {SIZE_OF(DCOP_SESSION_HEAD, m_attribute), OFFSET_OF(DCOP_SESSION_HEAD, m_attribute)}, \
        {SIZE_OF(DCOP_SESSION_HEAD, m_index), OFFSET_OF(DCOP_SESSION_HEAD, m_index)} \
    }; \
    Bytes_ChangeOrderByRule(DCOP_SESSION_HEAD_BORULE, ARRAY_SIZE(DCOP_SESSION_HEAD_BORULE), \
                        pHead, DCOP_MSG_HEAD_SIZE[DCOP_MSG_HEAD_SESSION])


/// -------------------------------------------------
/// CONDITION头部字节序转换规则
/// -------------------------------------------------
#define BYTES_CHANGE_CONDITION_HEAD_ORDER(pHead)    \
    const BYTES_CHANGE_RULE DCOP_CONDITION_HEAD_BORULE[] = \
    { \
        {SIZE_OF(DCOP_MSG_HEAD, m_valueLen), OFFSET_OF(DCOP_MSG_HEAD, m_valueLen)}, \
        {SIZE_OF(DCOP_CONDITION_HEAD, m_paraLen), OFFSET_OF(DCOP_CONDITION_HEAD, m_paraLen)} \
    }; \
    Bytes_ChangeOrderByRule(DCOP_CONDITION_HEAD_BORULE, ARRAY_SIZE(DCOP_CONDITION_HEAD_BORULE), \
                        pHead, DCOP_MSG_HEAD_SIZE[DCOP_MSG_HEAD_CONDITION])


/// -------------------------------------------------
/// REQUEST头部字节序转换规则
/// -------------------------------------------------
#define BYTES_CHANGE_REQUEST_HEAD_ORDER(pHead)      \
    const BYTES_CHANGE_RULE DCOP_REQUEST_HEAD_BORULE[] = \
    { \
        {SIZE_OF(DCOP_MSG_HEAD, m_valueLen), OFFSET_OF(DCOP_MSG_HEAD, m_valueLen)}, \
        {SIZE_OF(DCOP_REQUEST_HEAD, m_paraCount), OFFSET_OF(DCOP_REQUEST_HEAD, m_paraCount)}, \
        {SIZE_OF(DCOP_REQUEST_HEAD, m_paraLen), OFFSET_OF(DCOP_REQUEST_HEAD, m_paraLen)} \
    }; \
    Bytes_ChangeOrderByRule(DCOP_REQUEST_HEAD_BORULE, ARRAY_SIZE(DCOP_REQUEST_HEAD_BORULE), \
                        pHead, DCOP_MSG_HEAD_SIZE[DCOP_MSG_HEAD_REQUEST])


/// -------------------------------------------------
/// RESPONSE头部字节序转换规则
/// -------------------------------------------------
#define BYTES_CHANGE_RESPONSE_HEAD_ORDER(pHead)     \
    const BYTES_CHANGE_RULE DCOP_RESPONSE_HEAD_BORULE[] = \
    { \
        {SIZE_OF(DCOP_MSG_HEAD, m_valueLen), OFFSET_OF(DCOP_MSG_HEAD, m_valueLen)}, \
        {SIZE_OF(DCOP_RESPONSE_HEAD, m_retCode), OFFSET_OF(DCOP_RESPONSE_HEAD, m_retCode)}, \
        {SIZE_OF(DCOP_RESPONSE_HEAD, m_recordCount), OFFSET_OF(DCOP_RESPONSE_HEAD, m_recordCount)}, \
        {SIZE_OF(DCOP_RESPONSE_HEAD, m_recordIndex), OFFSET_OF(DCOP_RESPONSE_HEAD, m_recordIndex)}, \
        {SIZE_OF(DCOP_RESPONSE_HEAD, m_paraCount), OFFSET_OF(DCOP_RESPONSE_HEAD, m_paraCount)}, \
        {SIZE_OF(DCOP_RESPONSE_HEAD, m_paraLen), OFFSET_OF(DCOP_RESPONSE_HEAD, m_paraLen)} \
    }; \
    Bytes_ChangeOrderByRule(DCOP_RESPONSE_HEAD_BORULE, ARRAY_SIZE(DCOP_RESPONSE_HEAD_BORULE), \
                        pHead, DCOP_MSG_HEAD_SIZE[DCOP_MSG_HEAD_RESPONSE])


/// -------------------------------------------------
/// EVENT头部字节序转换规则
/// -------------------------------------------------
#define BYTES_CHANGE_EVENT_HEAD_ORDER(pHead)        \
    const BYTES_CHANGE_RULE DCOP_EVENT_HEAD_BORULE[] = \
    { \
        {SIZE_OF(DCOP_MSG_HEAD, m_valueLen), OFFSET_OF(DCOP_MSG_HEAD, m_valueLen)}, \
        {SIZE_OF(DCOP_EVENT_HEAD, m_recordCount), OFFSET_OF(DCOP_EVENT_HEAD, m_recordCount)}, \
        {SIZE_OF(DCOP_EVENT_HEAD, m_recordIndex), OFFSET_OF(DCOP_EVENT_HEAD, m_recordIndex)}, \
        {SIZE_OF(DCOP_EVENT_HEAD, m_paraCount), OFFSET_OF(DCOP_EVENT_HEAD, m_paraCount)}, \
        {SIZE_OF(DCOP_EVENT_HEAD, m_paraLen), OFFSET_OF(DCOP_EVENT_HEAD, m_paraLen)} \
    }; \
    Bytes_ChangeOrderByRule(DCOP_EVENT_HEAD_BORULE, ARRAY_SIZE(DCOP_EVENT_HEAD_BORULE), \
                        pHead, DCOP_MSG_HEAD_SIZE[DCOP_MSG_HEAD_EVENT])


/// -------------------------------------------------
/// PARA节点字节序转换规则
/// -------------------------------------------------
#define BYTES_CHANGE_PARA_NODE_ORDER(pNode, Count)  \
    const BYTES_CHANGE_RULE DCOP_PARA_NODE_BORULE[] = \
    { \
        {SIZE_OF(DCOP_PARA_NODE, m_paraID), OFFSET_OF(DCOP_PARA_NODE, m_paraID)}, \
        {SIZE_OF(DCOP_PARA_NODE, m_paraSize), OFFSET_OF(DCOP_PARA_NODE, m_paraSize)}, \
    }; \
    for (DWORD i = 0; i < Count; ++i) \
    Bytes_ChangeOrderByRule(DCOP_PARA_NODE_BORULE, ARRAY_SIZE(DCOP_PARA_NODE_BORULE), \
                        &(pNode[i]), sizeof(DCOP_PARA_NODE))


/// -------------------------------------------------
/// 会话版本
/// -------------------------------------------------
#define DCOP_SESSION_VER 1


/// -------------------------------------------------
/// 控制定义
/// -------------------------------------------------
enum DCOP_CTRL_TYPE
{
    DCOP_CTRL_NULL = 0,                     // 无效操作
    DCOP_CTRL_CREATE,                       // 创建操作
    DCOP_CTRL_DESTROY,                      // 销毁操作
    DCOP_CTRL_ADD,                          // 添加操作
    DCOP_CTRL_DEL,                          // 删除操作
    DCOP_CTRL_SET,                          // 设置操作
    DCOP_CTRL_GET,                          // 获取操作
    DCOP_CTRL_DUMP,                         // DUMP操作
    DCOP_CTRL_METHOD,                       // '方法'
    DCOP_CTRL_EVENT,                        // '事件'(这种事件是无源事件;由于上面的操作引起的事件是有源事件)
};


/// -------------------------------------------------
/// 应答定义
/// -------------------------------------------------
enum DCOP_ACK_TYPE
{
    DCOP_REQ = 0,                           // 请求     (0b00)
    DCOP_EVT,                               // 事件     (0b01)
    DCOP_RSP_CON,                           // 响应持续 (0b10)
    DCOP_RSP_END                            // 响应结束 (0b11)
};
#define DCOP_RSP(ack) (((ack) & DCOP_RSP_CON) != 0)


/// -------------------------------------------------
/// 条件消息的条件类型
/// -------------------------------------------------
enum DCOP_CONDITION_TYPE
{
    DCOP_CONDITION_ANY = 0,                 // 满足任意字段的条件匹配
    DCOP_CONDITION_ALL,                     // 满足所有字段的条件匹配
    DCOP_CONDITION_ONE                      // 所有字段当作一个字段进行精确匹配
};


/// -------------------------------------------------
/// 特殊参数类型
/// -------------------------------------------------
/// 关联字段是一个表的字段关联到另外一张表上，然后可以直接获取关联表其他字段，组成: 
///     "高位16位(固定值) + 本身字段(最大255) + 关联字段(最大255)"
/// [注] 正常情况的字段ID一般不超过255。
/// -------------------------------------------------
enum DCOP_SPECPARA_TYPE
{
    DCOP_FIELD_BASE = 0,
    DCOP_FIELD_SPECPARA,
    DCOP_FIELD_RELATION,

    DCOP_FIELD_LOW0 = 0x000000ff,
    DCOP_FIELD_LOW1 = 0x0000ff00,
    DCOP_FIELD_HIGH = 0xffff0000,

    DCOP_SPECPARA_COUNT = (DCOP_FIELD_SPECPARA << 16),      // 记录统计
    DCOP_SPECPARA_OFFSET,                                   // 记录偏移
    DCOP_SPECPARA_LIMIT,                                    // 记录限制
};
#define DCOP_SPECPARA(fieldID, specType) (((fieldID) & DCOP_FIELD_HIGH) == ((specType) << 16))


/// -------------------------------------------------
/// 参数操作符
/// -------------------------------------------------
enum DCOP_OPCODE_TYPE
{
    DCOP_OPCODE_NONE = 0,                   // 无操作
    DCOP_OPCODE_ADD,                        // 加       - '+'
    DCOP_OPCODE_SUB,                        // 减       - '-'
    DCOP_OPCODE_MUL,                        // 乘       - '*'
    DCOP_OPCODE_DIV,                        // 除       - '/'
    DCOP_OPCODE_EQUAL,                      // 等于     - '='
    DCOP_OPCODE_MORE_EQUAL,                 // 大于等于 - '>='
    DCOP_OPCODE_MORE,                       // 大于     - '>'
    DCOP_OPCODE_LESS,                       // 小于     - '<'
    DCOP_OPCODE_LESS_EQUAL,                 // 小于等于 - '<='
    DCOP_OPCODE_NOT_EQUAL,                  // 不等于   - '!='
    DCOP_OPCODE_INCLUDE,                    // 包含     - '#'
    DCOP_OPCODE_NOT_INCLUDE                 // 不包含   - '!#'
};


/// -------------------------------------------------
/// 任何消息类型
/// -------------------------------------------------
#define DCOP_MSG_ANY                                (0x00000000)


/// -------------------------------------------------
/// 对象消息类型(主要用于对象三大属性:'数据'/'方法'/'事件')
/// -------------------------------------------------
#define DCOP_MSG_OBJECT                             (0x00000001)
#define DCOP_MSG_OBJECT_REQUEST                     ((DCOP_MSG_OBJECT) + 1)        // 0x00000002 : 请求操作对象
#define DCOP_MSG_OBJECT_RESPONSE                    ((DCOP_MSG_OBJECT) + 2)        // 0x00000003 : 操作对象响应
#define DCOP_MSG_OBJECT_EVENT                       ((DCOP_MSG_OBJECT) + 3)        // 0x00000004 : 对象通知事件


/// -------------------------------------------------
/// 其他消息类型的组成
/// -------------------------------------------------
///     高16位 : 对象ID
///     低16位 : 偏移值
/// -------------------------------------------------


/// -------------------------------------------------
/// 状态机消息类型
/// -------------------------------------------------
#define DCOP_MSG_MANAGER                            ((DCOP_OBJECT_MANAGER) << 16)   // 0x00030000
#define DCOP_MSG_MANAGER_START                      ((DCOP_MSG_MANAGER) + 1)        // 0x00030001 : 静态开始运行
#define DCOP_MSG_MANAGER_FINISH                     ((DCOP_MSG_MANAGER) + 2)        // 0x00030002 : 静态结束运行
#define DCOP_MSG_MANAGER_LOAD                       ((DCOP_MSG_MANAGER) + 3)        // 0x00030003 : 动态加载对象
#define DCOP_MSG_MANAGER_UNLOAD                     ((DCOP_MSG_MANAGER) + 4)        // 0x00030004 : 动态卸载对象


/// -------------------------------------------------
/// 状态机消息类型
/// -------------------------------------------------
#define DCOP_MSG_STATUS                             ((DCOP_OBJECT_STATUS) << 16)    // 0x00080000
#define DCOP_MSG_STATUS_START                       ((DCOP_MSG_STATUS) + 1)         // 0x00080001 : 状态机开始
#define DCOP_MSG_STATUS_NEW                         ((DCOP_MSG_STATUS) + 2)         // 0x00080002 : 状态机迁移
#define DCOP_MSG_STATUS_TIMEOUT                     ((DCOP_MSG_STATUS) + 3)         // 0x00080003 : 状态机超时
#define DCOP_MSG_STATUS_STOP                        ((DCOP_MSG_STATUS) + 4)         // 0x00080004 : 状态机结束


/// -------------------------------------------------
/// 响应器消息类型
/// -------------------------------------------------
#define DCOP_MSG_RESPONSE                           ((DCOP_OBJECT_RESPONSE) << 16)  // 0x00090000
#define DCOP_MSG_RESPONSE_TIMER_1S                  ((DCOP_MSG_RESPONSE) + 1)       // 0x00090001 : 响应器1S定时器消息


/// -------------------------------------------------
/// 模型消息类型
/// -------------------------------------------------
#define DCOP_MSG_MODEL                              ((DCOP_OBJECT_MODEL) << 16)     // 0x000a0000
#define DCOP_MSG_MODEL_REG                          ((DCOP_MSG_MODEL) + 1)          // 0x000a0001 : 模型注册消息


/// -------------------------------------------------
/// 会话消息类型
/// -------------------------------------------------
#define DCOP_MSG_SESSION                            ((DCOP_OBJECT_SESSION) << 16)   // 0x00140000
#define DCOP_MSG_SESSION_TIMER_1S                   ((DCOP_MSG_SESSION) + 1)        // 0x00140001 : 会话1S定时器消息
#define DCOP_MSG_SESSION_TIMEOUT                    ((DCOP_MSG_SESSION) + 2)        // 0x00140002 : 会话超时消息


/// -------------------------------------------------
/// 超文本消息类型
/// -------------------------------------------------
#define DCOP_MSG_HTTPD                              ((DCOP_OBJECT_HTTPD) << 16)     // 0x00180000
#define DCOP_MSG_HTTPD_REQUEST                      ((DCOP_MSG_HTTPD) + 1)          // 0x00180001 : Http请求消息
#define DCOP_MSG_HTTPD_PROCESS                      ((DCOP_MSG_HTTPD) + 2)          // 0x00180002 : Http处理消息
#define DCOP_MSG_HTTPD_RESPONSE                     ((DCOP_MSG_HTTPD) + 3)          // 0x00180003 : Http响应消息


/// -------------------------------------------------
/// 外部自定义消息类型
/// -------------------------------------------------
#define DCOP_MSG_CUSTOM                             ((DCOP_OBJECT_CUSTOM) << 16) // 0x00FF0000


#endif // #ifndef _BASEMESSAGE_H_

