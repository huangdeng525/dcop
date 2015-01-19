/// -------------------------------------------------
/// bytes.cpp : 字节相关处理
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "cpu/bytes.h"


/// 字节序类型
enum BYTE_SORT_TYPE
{
    BYTE_SORT_TYPE_LITTLE_ENDIAN = 0,
    BYTE_SORT_TYPE_BIG_ENDIAN
};


/*******************************************************
  函 数 名: Bytes_GetHost
  描    述: 获得当前主机CPU的字节序
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
inline BYTE_SORT_TYPE Bytes_GetHost()
{
    DWORD dwTmp = 0x00000001L;

    if (*(BYTE *)&dwTmp)
    {
        return BYTE_SORT_TYPE_LITTLE_ENDIAN;
    }

    return BYTE_SORT_TYPE_BIG_ENDIAN;
}

/*******************************************************
  函 数 名: Bytes_GetWord
  描    述: 从BUF中获取2字节(字节序转换)
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
WORD Bytes_GetWord(const BYTE *cpbyBuf)
{
    if (Bytes_GetHost() == BYTE_SORT_TYPE_BIG_ENDIAN)
    {
        return *(WORD *)cpbyBuf;
    }

    WORD wRc;
    BYTE *pbyTmp = (BYTE *)&wRc;
    pbyTmp[1] = cpbyBuf[0];
    pbyTmp[0] = cpbyBuf[1];

    return wRc;
}

/*******************************************************
  函 数 名: Bytes_SetWord
  描    述: 把2字节设置到BUF中(字节序转换)
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void Bytes_SetWord(BYTE *pbyBuf, WORD wValue)
{
    if (Bytes_GetHost() == BYTE_SORT_TYPE_BIG_ENDIAN)
    {
        *(WORD *)pbyBuf = wValue;
        return;
    }

    BYTE *pbyTmp = (BYTE *)&wValue;
    pbyBuf[1] = pbyTmp[0];
    pbyBuf[0] = pbyTmp[1];
}

/*******************************************************
  函 数 名: Bytes_GetDword
  描    述: 从BUF中获取4字节(字节序转换)
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD Bytes_GetDword(const BYTE *cpbyBuf)
{
    if (Bytes_GetHost() == BYTE_SORT_TYPE_BIG_ENDIAN)
    {
        return *(DWORD *)cpbyBuf;
    }

    DWORD dwRc;
    BYTE *pbyTmp = (BYTE *)&dwRc;
    pbyTmp[3] = cpbyBuf[0];
    pbyTmp[2] = cpbyBuf[1];
    pbyTmp[1] = cpbyBuf[2];
    pbyTmp[0] = cpbyBuf[3];

    return dwRc;
}

/*******************************************************
  函 数 名: Bytes_SetDword
  描    述: 把4字节设置到BUF中(字节序转换)
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void Bytes_SetDword(BYTE *pbyBuf, DWORD dwValue)
{
    if (Bytes_GetHost() == BYTE_SORT_TYPE_BIG_ENDIAN)
    {
        *(DWORD *)pbyBuf = dwValue;
        return;
    }

    BYTE *pbyTmp = (BYTE *)&dwValue;
    pbyBuf[3] = pbyTmp[0];
    pbyBuf[2] = pbyTmp[1];
    pbyBuf[1] = pbyTmp[2];
    pbyBuf[0] = pbyTmp[3];
}

/*******************************************************
  函 数 名: Bytes_GetDwordValue
  描    述: 根据长度从BUF中获取DWORD值(长度可能为1、2、4，超长只取前4个字节)
  输    入: pbyBuf (不检查是否为NULL，请自行在调用前检查)
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD Bytes_GetDwordValue(const BYTE *cpbyBuf, DWORD dwBufLen)
{
    if (dwBufLen == sizeof(BYTE))
    {
        return (DWORD)*cpbyBuf;
    }
    else if ((dwBufLen >= sizeof(WORD)) && (dwBufLen < sizeof(DWORD)))
    {
        return (DWORD)Bytes_GetWord(cpbyBuf);
    }
    else
    {
        return Bytes_GetDword(cpbyBuf);
    }
}

/*******************************************************
  函 数 名: Bytes_SetDwordValue
  描    述: 根据长度设置DWORD值到BUF中(长度可能为1、2、4，超长只写前4个字节)
  输    入: pbyBuf (不检查是否为NULL，请自行在调用前检查)
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void Bytes_SetDwordValue(DWORD dwValue, BYTE *pbyBuf, DWORD dwBufLen)
{
    if (dwBufLen == sizeof(BYTE))
    {
        *(BYTE *)pbyBuf = (BYTE)dwValue;
    }
    else if ((dwBufLen >= sizeof(WORD)) && (dwBufLen < sizeof(DWORD)))
    {
        Bytes_SetWord(pbyBuf, (WORD)dwValue);
    }
    else
    {
        Bytes_SetDword(pbyBuf, dwValue);
    }
}

/*******************************************************
  函 数 名: Bytes_ChangeOrderByRule
  描    述: 把BUF中的数据按规则批量进行转换
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void Bytes_ChangeOrderByRule(const BYTES_CHANGE_RULE *adwRule,
                    DWORD dwRuleCount,
                    void *pBuf,
                    DWORD dwBufLen)
{
    if (!adwRule || !dwRuleCount || !pBuf || !dwBufLen)
    {
        return;
    }

    /// 测试主机字节序
    DWORD dwTest = 1;
    if (0 == *(BYTE *)&dwTest)
    {
        /// 测试结果为大端字节序(网络字节序)，不用转换
        return;
    }

    /// 'awRule'低3个字节为位置，高1个字节为字节数(2字节或者4字节)
    for (DWORD i = 0; i < dwRuleCount; ++i)
    {
        DWORD dwPos = (DWORD)(adwRule[i].pos);
        if (dwPos >= dwBufLen)
        {
            /// 位置超出BUF长度
            continue;
        }

        BYTE bySize = (BYTE)(adwRule[i].size);
        switch (bySize)
        {
            case sizeof(DWORD):
            {
                DWORD dwTemp = *(DWORD *)((BYTE *)pBuf + dwPos);
                *((BYTE *)pBuf + dwPos)     = *((BYTE *)&dwTemp + 3);
                *((BYTE *)pBuf + dwPos + 1) = *((BYTE *)&dwTemp + 2);
                *((BYTE *)pBuf + dwPos + 2) = *((BYTE *)&dwTemp + 1);
                *((BYTE *)pBuf + dwPos + 3) = *((BYTE *)&dwTemp);
            }
                break;
            case sizeof(WORD):
            {
                WORD wTemp = *(WORD *)((BYTE *)pBuf + dwPos);
                *((BYTE *)pBuf + dwPos)     = *((BYTE *)&wTemp + 1);
                *((BYTE *)pBuf + dwPos + 1) = *((BYTE *)&wTemp);
            }
                break;
            default:
                break;
        }
    }
}

