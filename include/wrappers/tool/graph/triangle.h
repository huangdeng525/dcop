/// -------------------------------------------------
/// triangle.h : 图转换为三角地图公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_GRAPH_TRIANGLE_H_
#define _TOOL_GRAPH_TRIANGLE_H_


#include "map.h"


/// 三角地图(每个方格中间有斜线，被分割为两个三角形)
class CTriangleMap : public CXYMap
{
public:

    /// 初始化地图的宽高以及单元的值
    void Init(DWORD dwWeight, DWORD dwHight, int *ipValue = 0);

    /// =============== 重写基类函数 ================
    void Set(const CXYMap::POS &pos, int value);
    int Get(const CXYMap::POS &pos);
    IGraph::POS Vex(const CXYMap::POS &pos, DIRECT comeDirect = DIRECT_NONE);
    void Pos(const IGraph::POS &vex, POS &pos);
    bool CanVisit(const POS &throughPos, const POS &parentPos, const POS &curPos);
    /// =============================================
};


#endif // #ifndef _TOOL_GRAPH_TRIANGLE_H_

