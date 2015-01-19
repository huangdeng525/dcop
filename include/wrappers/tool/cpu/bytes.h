/// -------------------------------------------------
/// bytes.h : 字节相关处理
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_CPU_BYTES_H_
#define _TOOL_CPU_BYTES_H_

#include "dcop.h"


//////////////////////////////////////////////////////
/// 不同的 CPU 有不同的字节序类型, 这些字节序是指整数在内存中保存的顺序, 这个叫做主机序
/// 最常见的有两种:
/// 1. Little endian:   将低序字节存储在起始地址
/// 2. Big endian:      将高序字节存储在起始地址

/// LE little-endian
/// 最符合人的思维的字节序
/// 地址低位存储值的低位
/// 地址高位存储值的高位
/// 怎么讲是最符合人的思维的字节序，是因为从人的第一观感来说
/// 低位值小，就应该放在内存地址小的地方，也即内存地址低位
/// 反之，高位值就应该放在内存地址大的地方，也即内存地址高位

/// BE big-endian
/// 最直观的字节序
/// 地址低位存储值的高位
/// 地址高位存储值的低位
/// 为什么说直观，不要考虑对应关系
/// 只需要把内存地址从左到右按照由低到高的顺序写出
/// 把值按照通常的高位到低位的顺序写出
/// 两者对照，一个字节一个字节的填充进去

/// [例子] 在内存中双字 0x01020304(DWORD) 的存储方式
/// 内存地址    00  01  02  03
///     LE      04  03  02  01
///     BE      01  02  03  04

/// 网络字节顺序是 TCP/IP 中规定好的一种数据表示格式，它与具体的 CPU 类型、操作系统等无关，
/// 从而可以保证数据在不同主机之间传输时能够被正确解释。网络字节顺序采用 big endian 排序方式。

/// 为了进行转换 bsd socket 提供了转换的函数 有下面四个
/// htons 把 unsigned short 类型从主机序转换到网络序
/// htonl 把 unsigned long 类型从主机序转换到网络序
/// ntohs 把 unsigned short 类型从网络序转换到主机序
/// ntohl 把 unsigned long 类型从网络序转换到主机序

/// 在使用 little endian 的系统中 这些函数会把字节序进行转换
/// 在使用 big endian 类型的系统中 这些函数会定义成空宏

/// 同样 在网络程序开发时 或是跨平台开发时 也应该注意保证只用一种字节序 不然两方的解释不一样就会产生 bug!

/// 备注：
/// 1 、网络与主机字节转换函数 :htons ntohs htonl ntohl (s 就是 short l 是 long h 是 host n 是 network)
/// 2 、不同的 CPU 上运行不同的操作系统，字节序也是不同的，参见下表:
///     处理器      操作系统        字节排序
///     Alpha       全部            Little endian
///     HP-PA       NT              Little endian
///     HP-PA       UNIX            Big endian
///     Intelx86    全部            Little endian   <-----x86 系统是小端字节序系统
///     MIPS        NT              Little endian
///     MIPS        UNIX            Big endian
///     PowerPC     NT              Little endian
///     PowerPC     非 NT           Big endian      <-----PPC 系统是大端字节序系统
///     RS/6000     UNIX            Big endian
///     SPARC       UNIX            Big endian
///     ARM 核心    全部            Little endian   <-----ARM 系统是小端字节序系统

/// [出处] http://blog.csdn.net/zhaojiangwei102/archive/2009/09/08/4532184.aspx
//////////////////////////////////////////////////////


//////////////////////////////////////////////////////
/// [提示] 为了提高效率, 本文的函数不校验输入参数
//////////////////////////////////////////////////////


/// 批量字节序转换规则结构定义
typedef struct tagBYTES_CHANGE_RULE
{
    DWORD size : 8;                         // 字节数
    DWORD pos : 24;                         // 位置
}BYTES_CHANGE_RULE;


#ifdef __cplusplus
extern "C" {
#endif


/// 从BUF中获取2字节(字节序转换)
extern WORD  Bytes_GetWord(const BYTE *cpbyBuf);

/// 把2字节设置到BUF中(字节序转换)
extern void  Bytes_SetWord(BYTE *pbyBuf, WORD wValue);

/// 从BUF中获取4字节(字节序转换)
extern DWORD Bytes_GetDword(const BYTE *cpbyBuf);

/// 把4字节设置到BUF中(字节序转换)
extern void  Bytes_SetDword(BYTE *pbyBuf, DWORD dwValue);

/// 根据长度从BUF中获取DWORD值(长度可能为1、2、4，超长只取前4个字节)
extern DWORD Bytes_GetDwordValue(const BYTE *cpbyBuf, DWORD dwBufLen);

/// 根据长度设置DWORD值到BUF中(长度可能为1、2、4，超长只写前4个字节)
extern void  Bytes_SetDwordValue(DWORD dwValue, BYTE *pbyBuf, DWORD dwBufLen);

/// 把BUF中的数据按规则批量进行转换
extern void  Bytes_ChangeOrderByRule(const BYTES_CHANGE_RULE *adwRule, 
                    DWORD dwRuleCount, 
                    void *pBuf, 
                    DWORD dwBufLen);


#ifdef __cplusplus
}
#endif


#endif // #ifndef _TOOL_CPU_BYTES_H_

