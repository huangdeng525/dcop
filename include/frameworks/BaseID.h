/// -------------------------------------------------
/// BaseID.h : 基本ID定义公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _BASEID_H_
#define _BASEID_H_


/// -------------------------------------------------
/// 对象索引值的组成
/// -------------------------------------------------
///     高16位 : 系统ID
///     低16位 : 对象ID
/// -------------------------------------------------
/// 所以ID小于0x00010000的对象都是只本地的对象
/// -------------------------------------------------

#define DCOP_OBJECT_NULL                    0               // 无效对象ID
#define DCOP_OBJECT_KERNEL                  1               // 内核(也表示对象基类)
#define DCOP_OBJECT_FACTORY                 2               // 类工厂
#define DCOP_OBJECT_MANAGER                 3               // 对象管理器
#define DCOP_OBJECT_DISPATCH                4               // 消息分发器
#define DCOP_OBJECT_NOTIFY                  5               // 订阅发布器
#define DCOP_OBJECT_CONTROL                 6               // 控制器
#define DCOP_OBJECT_TIMER                   7               // 定时器
#define DCOP_OBJECT_SCHEDULE                8               // 调度器
#define DCOP_OBJECT_STATUS                  9               // 状态机
#define DCOP_OBJECT_RESPONSE                10              // 响应器
#define DCOP_OBJECT_MODEL                   11              // 模型
#define DCOP_OBJECT_DATA                    12              // 数据
#define DCOP_OBJECT_CONNECT                 13              // 连接器
#define DCOP_OBJECT_PROXY                   14              // 代理

#define DCOP_OBJECT_SESSION                 20              // 会话管理
#define DCOP_OBJECT_USER                    21              // 用户管理
#define DCOP_OBJECT_SECURE                  22              // 安全管理
#define DCOP_OBJECT_ACCESS                  23              // 分布式接入
#define DCOP_OBJECT_COMMAND                 24              // 命令行接入
#define DCOP_OBJECT_HTTPD                   25              // 超文本接入
#define DCOP_OBJECT_RESTFUL                 26              // Rest API

#define DCOP_OBJECT_MONITOR                 50              // 系统监控

#define DCOP_OBJECT_CUSTOM                  256             // 用户自定义


/// -------------------------------------------------
/// 内部和外部系统定义(只是标识从外部系统发过来的，但不特指具体哪个系统)
/// -------------------------------------------------

#define DCOP_SYSTEM_INTERNAL                0x0000FFFF
#define DCOP_SYSTEM_EXTERNAL                0xFFFF0000


/// -------------------------------------------------
/// 对象属性值的组成
/// -------------------------------------------------
///     高16位 : 对象ID
///     低16位 : 偏移值
/// -------------------------------------------------

#define DCOP_OBJATTR_SESSION_TABLE          ((DCOP_OBJECT_SESSION   << (16)) | 1)   // 会话表
#define DCOP_OBJATTR_USER_TABLE             ((DCOP_OBJECT_USER      << (16)) | 1)   // 用户表
#define DCOP_OBJATTR_SECURE_TABLE           ((DCOP_OBJECT_SECURE    << (16)) | 1)   // 权限表
#define DCOP_OBJATTR_ACCESS_LOGIN           ((DCOP_OBJECT_ACCESS    << (16)) | 1)   // 接入登录
#define DCOP_OBJATTR_ACCESS_LOGOUT          ((DCOP_OBJECT_ACCESS    << (16)) | 2)   // 退出登录


/// -------------------------------------------------
/// 用户组(类型)定义
/// -------------------------------------------------

#define DCOP_GROUP_VISITOR                  0       // 参观者   : 未登录时默认用户，权限约定受限
#define DCOP_GROUP_USER                     1       // 用户     : 业务层面登录用户，具有具体业务的约定权限
#define DCOP_GROUP_MONITOR                  2       // 监控者   : 系统层面监控者，只能查看收集日志
#define DCOP_GROUP_MAINTAINER               3       // 维护者   : 系统层面维护者，拥有操作权限，可下发维护命令
#define DCOP_GROUP_ADMINISTRATOR            4       // 管理者   : 系统层面管理者，拥有所有权限
#define DCOP_GROUP_ROOT                     5       // 超级用户 : 只在原始启动shell才使用的，具有停止应用的权限


/// -------------------------------------------------
/// 临时访问用户
/// -------------------------------------------------

#define DCOP_USER_UNLOGIN                   0       // 未登录用户


#endif // #ifndef _BASEID_H_

