/// -------------------------------------------------
/// map.cpp : 图转换为地图实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "graph/map.h"
#include <stdlib.h>
#include <memory.h>


CXYMap::CXYMap()
{
    m_directType = DIRECT_TYPE_4;
    m_weight = 0;
    m_hight = 0;
}

CXYMap::~CXYMap()
{
}

void CXYMap::Init(DWORD dwWeight, DWORD dwHight, int *ipValue)
{
    IGraph::Init(dwWeight * dwHight, ipValue);
    m_weight = dwWeight;
    m_hight = dwHight;
}

void CXYMap::Set(const POS &pos, int value)
{
    if (NullPos(pos))
    {
        return;
    }

    IGraph::Set(Vex(pos), value);
}

int CXYMap::Get(const POS &pos)
{
    if (NullPos(pos))
    {
        return -1;
    }

    return IGraph::Get(Vex(pos));
}

bool CXYMap::NullPos(const POS &pos)
{
    if ((pos.x < 0) || ((DWORD)(pos.x) >= m_weight) ||
        (pos.y < 0) || ((DWORD)(pos.y) >= m_hight))
    {
        return true;
    }

    return false;
}

IGraph::POS CXYMap::Vex(const POS &pos, DIRECT comeDirect)
{
    if (NullPos(pos))
    {
        /// 边的处理

        /// 左上角 + 上边
        if ((pos.y == -1) && ((pos.x >= -1) && (pos.x < (int)m_weight)))
        {
            return (0 - (pos.x + 2));
        }

        /// 右上角 + 右边
        if ((pos.x == (int)m_weight) && ((pos.y >= -1) && (pos.y < (int)m_hight)))
        {
            return (0 - (m_weight + pos.y + 3));
        }

        /// 右下角 + 下边
        if ((pos.y == (int)m_hight) && ((pos.x > -1) && (pos.x <= (int)m_weight)))
        {
            return (0 - (2*m_weight + m_hight + 3 - pos.x));
        }

        /// 左下角 + 左边
        if ((pos.x == -1) && ((pos.y > -1) && (pos.y <= (int)m_hight)))
        {
            return (0 - (2*m_weight + 2*m_hight + 4 - pos.y));
        }

        return IGraph::Null;
    }

    /// 正常的坐标
    return pos.y * m_weight + pos.x;
}

void CXYMap::Pos(const IGraph::POS &vex, POS &pos)
{
    if (IGraph::NullPos(vex))
    {
        /// 边的处理

        /// 左上角 + 上边
        if ((vex > (int)(-2 - m_weight)) && (vex <= -1))
        {
            pos.x = -2 - vex;
            pos.y = -1;
            return;
        }

        /// 右上角 + 右边
        if ((vex > (int)(-3 - m_weight - m_hight)) && (vex <= (int)(-2 - m_weight)))
        {
            pos.x = m_weight;
            pos.y = -3 - m_weight - vex;
            return;
        }

        /// 右下角 + 下边
        if ((vex > (int)(-4 - 2*m_weight - m_hight)) && (vex <= (int)(-3 - m_weight - m_hight)))
        {
            pos.x = vex + 3 + 2*m_weight + m_hight;
            pos.y = m_hight;
            return;
        }

        /// 左下角 + 左边
        if ((vex >= (int)(-4 - 2*m_weight - 2*m_hight)) && (vex <= (int)(-4 - 2*m_weight - m_hight)))
        {
            pos.x = -1;
            pos.y = vex + 4 + 2*m_weight + 2*m_hight;
            return;
        }

        pos.x = -1;
        pos.y = -1;
        return;
    }

    /// 正常的坐标
    pos.x = vex % m_weight;
    pos.y = vex / m_weight;
}

IGraph::POS CXYMap::FirstAdjVex(const IGraph::POS &parentVex, const IGraph::POS &curVex, const IGraph::POS &dstVex)
{
    if (IGraph::NullPos(curVex))
    {
        return IGraph::Null;
    }

    POS parentPos;
    Pos(parentVex, parentPos);

    POS curPos;
    Pos(curVex, curPos);

    POS dstPos;
    Pos(dstVex, dstPos);

    POS nextPos;
    DIRECT firstDirect = GetFirstDirect(curPos, dstPos);
    for (DIRECT direct = firstDirect; XYMAP_AVAILABLE_DIRECT(direct); direct = GetNextDirect(firstDirect, direct))
    {
        GetDirectPos(curPos, direct, nextPos);
        if (CanVisit(parentPos, curPos, nextPos))
        {
            return Vex(nextPos, XYMAP_GET_REVERSE_DIRECT(direct));
        }
    }

    return IGraph::Null;
}

IGraph::POS CXYMap::NextAdjVex(const IGraph::POS &parentVex, const IGraph::POS &curVex, const IGraph::POS &adjVex, const IGraph::POS &dstVex)
{
    POS parentPos;
    Pos(parentVex, parentPos);

    POS curPos;
    Pos(curVex, curPos);

    POS dstPos;
    Pos(dstVex, dstPos);

    POS curAdjPos;
    Pos(adjVex, curAdjPos);

    POS nextAdjPos;
    DIRECT firstDirect = GetFirstDirect(curPos, dstPos);
    for (DIRECT direct = GetNextDirectPos(curPos, curAdjPos, nextAdjPos, firstDirect);
        XYMAP_AVAILABLE_DIRECT(direct);
        direct = GetNextDirectPos(curPos, curAdjPos, nextAdjPos, firstDirect))
    {
        if (CanVisit(parentPos, curPos, nextAdjPos))
        {
            return Vex(nextAdjPos, XYMAP_GET_REVERSE_DIRECT(direct));
        }

        curAdjPos = nextAdjPos;
    }
    
    return IGraph::Null;
}

bool CXYMap::CanVisit(const POS &throughPos, const POS &parentPos, const POS &curPos)
{
    if (NullPos(curPos))
    {
        return false;
    }

    if (Get(curPos))
    {
        return false;
    }

    return true;
}

CXYMap::DIRECT CXYMap::GetFirstDirect(const POS &curPos, const POS &dstPos)
{
    DIRECT direct = GetDirect(curPos, dstPos);
    if (!XYMAP_AVAILABLE_DIRECT(direct))
    {
        return DIRECT_UP;
    }

    if ((DIRECT_TYPE_4 == m_directType) && (direct % 2))
    {
        /// 只支持4个方向时，并且是斜方向时 : 如果X轴距离远, 就向右或者向左; 如果是Y轴远，就向上或者向下
        if (XYMAP_GET_DISTANCE(dstPos.x, curPos.x) > XYMAP_GET_DISTANCE(dstPos.y, curPos.y))
        {
            if ((DIRECT_RIGHTUP == direct) || (DIRECT_RIGHTDOWN == direct))
            {
                direct = DIRECT_RIGHT;
            }
            else
            {
                direct = DIRECT_LEFT;
            }
        }
        else
        {
            if ((DIRECT_RIGHTUP == direct) || (DIRECT_LEFTUP == direct))
            {
                direct = DIRECT_UP;
            }
            else
            {
                direct = DIRECT_DOWN;
            }
        }
    }

    return direct;
}

CXYMap::DIRECT CXYMap::GetNextDirect(DIRECT firstDirect, DIRECT curDirect)
{
    /// 默认只有上下左右四个方向
    if (DIRECT_TYPE_4 == m_directType)
    {
        curDirect = (DIRECT)(curDirect + 2);
    }
    else
    {
        curDirect = (DIRECT)(curDirect + 1);
    }

    if (curDirect >= DIRECT_COUNT)
    {
        curDirect = (DIRECT)(curDirect % DIRECT_COUNT);
    }

    if (curDirect == firstDirect)
    {
        curDirect = DIRECT_NONE;
    }

    return curDirect;
}

CXYMap::DIRECT CXYMap::GetNextDirectPos(const POS &parentPos, const POS &curPos, POS &nextPos, DIRECT firstDirect)
{
    DIRECT curDirect = GetDirect(parentPos, curPos);
    if (!XYMAP_AVAILABLE_DIRECT(curDirect))
    {
        return DIRECT_NONE;
    }

    curDirect = GetNextDirect(firstDirect, curDirect);
    if (!XYMAP_AVAILABLE_DIRECT(curDirect))
    {
        return DIRECT_NONE;
    }

    GetDirectPos(parentPos, curDirect, nextPos);

    return curDirect;
}

CXYMap::DIRECT CXYMap::GetDirect(const POS &curPos, const POS &adjPos)
{
    if ((adjPos.x == curPos.x) && (adjPos.y < curPos.y))
    {
        return DIRECT_UP;
    }

    if ((adjPos.x > curPos.x) && (adjPos.y < curPos.y))
    {
        return DIRECT_RIGHTUP;
    }

    if ((adjPos.x > curPos.x) && (adjPos.y == curPos.y))
    {
        return DIRECT_RIGHT;
    }

    if ((adjPos.x > curPos.x) && (adjPos.y > curPos.y))
    {
        return DIRECT_RIGHTDOWN;
    }

    if ((adjPos.x == curPos.x) && (adjPos.y > curPos.y))
    {
        return DIRECT_DOWN;
    }

    if ((adjPos.x < curPos.x) && (adjPos.y > curPos.y))
    {
        return DIRECT_LEFTDOWN;
    }

    if ((adjPos.x < curPos.x) && (adjPos.y == curPos.y))
    {
        return DIRECT_LEFT;
    }

    if ((adjPos.x < curPos.x) && (adjPos.y < curPos.y))
    {
        return DIRECT_LEFTUP;
    }

    return DIRECT_NONE;
}

void CXYMap::GetDirectPos(const POS &curPos, DIRECT curDirect, POS &adjPos)
{
    switch (curDirect)
    {
        case DIRECT_UP :
        {
            adjPos.x = curPos.x;
            adjPos.y = curPos.y - 1;
        }
            break;
        case DIRECT_RIGHTUP :
        {
            adjPos.x = curPos.x + 1;
            adjPos.y = curPos.y - 1;
        }
            break;
        case DIRECT_RIGHT :
        {
            adjPos.x = curPos.x + 1;
            adjPos.y = curPos.y;
        }
            break;
        case DIRECT_RIGHTDOWN :
        {
            adjPos.x = curPos.x + 1;
            adjPos.y = curPos.y + 1;
        }
            break;
        case DIRECT_DOWN :
        {
            adjPos.x = curPos.x;
            adjPos.y = curPos.y + 1;
        }
            break;
        case DIRECT_LEFTDOWN :
        {
            adjPos.x = curPos.x - 1;
            adjPos.y = curPos.y + 1;
        }
            break;
        case DIRECT_LEFT :
        {
            adjPos.x = curPos.x - 1;
            adjPos.y = curPos.y;
        }
            break;
        case DIRECT_LEFTUP :
        {
            adjPos.x = curPos.x - 1;
            adjPos.y = curPos.y - 1;
        }
            break;
        default:
        {
            adjPos.x = -1;
            adjPos.y = -1;
        }
    }
}

