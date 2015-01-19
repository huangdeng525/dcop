/// -------------------------------------------------
/// triangle.cpp : 图转换为三角地图实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "graph/triangle.h"


void CTriangleMap::Init(DWORD dwWeight, DWORD dwHight, int *ipValue)
{
    IGraph::Init(2 * dwWeight * dwHight);
    SetWeight(dwWeight);
    SetHight(dwHight);

    if (ipValue)
    {
        for (DWORD i = 0; i < dwWeight; ++i)
        {
            for (DWORD j = 0; j < dwHight; ++j)
            {
                Set(CXYMap::POS(i,j), ipValue[j * dwWeight + i]);
            }
        }
    }
}

void CTriangleMap::Set(const CXYMap::POS &pos, int value)
{
    if (NullPos(pos))
    {
        return;
    }

    IGraph::Set(pos.y * GetWeight() * 2 + pos.x * 2, value);
    IGraph::Set(pos.y * GetWeight() * 2 + pos.x * 2 + 1, value);
}

int CTriangleMap::Get(const CXYMap::POS &pos)
{
    if (NullPos(pos))
    {
        return -1;
    }

    return IGraph::Get(pos.y * GetWeight() * 2 + pos.x * 2);
}

IGraph::POS CTriangleMap::Vex(const CXYMap::POS &pos, DIRECT comeDirect)
{
    if (NullPos(pos))
    {
        return CXYMap::Vex(pos, comeDirect);
    }

    if (Get(pos) == '/')
    {
        if ((comeDirect == DIRECT_UP) || (comeDirect == DIRECT_LEFT))
        {
            return pos.y * GetWeight() * 2 + pos.x * 2;
        }

        return pos.y * GetWeight() * 2 + pos.x * 2 + 1;
    }

    if (Get(pos) == '\\')
    {
        if ((comeDirect == DIRECT_LEFT) || (comeDirect == DIRECT_DOWN))
        {
            return pos.y * GetWeight() * 2 + pos.x * 2;
        }

        return pos.y * GetWeight() * 2 + pos.x * 2 + 1;
    }

    return pos.y * GetWeight() * 2 + pos.x * 2;
}

void CTriangleMap::Pos(const IGraph::POS &vex, POS &pos)
{
    if (vex <= IGraph::Null)
    {
        return CXYMap::Pos(vex, pos);
    }

    pos.x = (vex % (GetWeight() * 2)) / 2;
    pos.y = vex / (GetWeight() * 2);
}

bool CTriangleMap::CanVisit(const POS &throughPos, const POS &parentPos, const POS &curPos)
{
    DIRECT comeDirect = GetDirect(parentPos, throughPos);
    DIRECT goDirect = GetDirect(parentPos, curPos);

    if (comeDirect == goDirect)
    {
        return false;
    }

    if (Get(parentPos) == '/')
    {
        if (((comeDirect == DIRECT_UP) || (comeDirect == DIRECT_LEFT)) &&
            ((goDirect == DIRECT_RIGHT) || (goDirect == DIRECT_DOWN)))
        {
            return false;
        }

        if (((comeDirect == DIRECT_RIGHT) || (comeDirect == DIRECT_DOWN)) &&
            ((goDirect == DIRECT_UP) || (goDirect == DIRECT_LEFT)))
        {
            return false;
        }
    }

    if (Get(parentPos) == '\\')
    {
        if (((comeDirect == DIRECT_UP) || (comeDirect == DIRECT_RIGHT)) &&
            ((goDirect == DIRECT_LEFT) || (goDirect == DIRECT_DOWN)))
        {
            return false;
        }

        if (((comeDirect == DIRECT_LEFT) || (comeDirect == DIRECT_DOWN)) &&
            ((goDirect == DIRECT_UP) || (goDirect == DIRECT_RIGHT)))
        {
            return false;
        }
    }

    if (NullPos(curPos))
    {
        return false;
    }

    return true;
}


