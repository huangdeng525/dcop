/// -------------------------------------------------
/// map.h : 图转换为地图公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_GRAPH_MAP_H_
#define _TOOL_GRAPH_MAP_H_

#include "graph.h"


/// XY坐标的地图
class CXYMap : public IGraph
{
public:
    /// 单元坐标, 左上角的坐标是{0,0}, 右下角的坐标是{weight-1,hight-1}
    /// 虚拟边界坐标为负数，和顶点序号转换规则为:
    /// 上边: {-1,-1} ~ {weight+1,-1}               对应顶点: -1 ~ -(weight+2)
    /// 右边: {weight+1,-1} ~ {weight+1,hight+1}    对应顶点: -(weight+2) ~ -(weight+hight+3)
    /// 下边: {-1,hight+1} ~ {weight+1,hight+1}     对应顶点: -(2weight+hight+4) ~ -(weight+hight+3)
    /// 左边: {-1,-1} ~ {weight+1,hight+1}          对应顶点: -(2weight+2hight+4) ~ -(2weight+hight+4)
    typedef struct tagPOS
    {
        int x;
        int y;

        tagPOS() {x=-1;y=-1;}
        tagPOS(int tmpx, int tmpy) {x=tmpx;y=tmpy;}
    }POS;

    /// 方向定义
    typedef enum tagDIRECT
    {
        DIRECT_NONE = -1,

        DIRECT_UP = 0,
        DIRECT_RIGHTUP,
        DIRECT_RIGHT,
        DIRECT_RIGHTDOWN,
        DIRECT_DOWN,
        DIRECT_LEFTDOWN,
        DIRECT_LEFT,
        DIRECT_LEFTUP,

        DIRECT_COUNT
    }DIRECT;

    /// 反方向
    #define XYMAP_GET_REVERSE_DIRECT(direct)    ((DIRECT)((direct + DIRECT_COUNT / 2) % DIRECT_COUNT))

    /// 可用的方向
    #define XYMAP_AVAILABLE_DIRECT(direct)      ((direct > DIRECT_NONE) && (direct < DIRECT_COUNT))

    /// 距离
    #define XYMAP_GET_DISTANCE(a, b)            (((a)>(b))? ((a)-(b)) : ((b)-(a)))

    /// 可选方向类型, 也可重写GetNextDirect获得不同方向
    typedef enum tagDIRECT_TYPE
    {
        DIRECT_TYPE_4 = 0,                      // 四个方向(垂直方向)
        DIRECT_TYPE_8                           // 八个方向(垂直+斜方向)
    }DIRECT_TYPE;


public:
    CXYMap();
    virtual ~CXYMap();

    /// 初始化地图的宽高以及单元的值
    void Init(DWORD dwWeight, DWORD dwHight, int *ipValue = 0);

    /// 设置单元值
    virtual void Set(const POS &pos, int value);

    /// 获取单元值
    virtual int Get(const POS &pos);

    /// 是否是无效坐标
    virtual bool NullPos(const POS &pos);

    /// 将地图坐标转换为图的顶点索引
    virtual IGraph::POS Vex(const POS &pos, DIRECT comeDirect = DIRECT_NONE);

    /// 将图的顶点索引转换为地图坐标
    virtual void Pos(const IGraph::POS &vex, POS &pos);

    /// 设置方向类型(比如4个方向/8个方向)
    void SetDirectType(DIRECT_TYPE directType) { m_directType = directType; }

    /// 设置宽度
    void SetWeight(DWORD dwWeight) { m_weight = dwWeight; }

    /// 设置高度
    void SetHight(DWORD dwHight) { m_hight = dwHight; }

    /// 获取宽度
    DWORD GetWeight() { return m_weight; }

    /// 获取高度
    DWORD GetHight() { return m_hight; }

    /// 是否可以访问
    virtual bool CanVisit(const POS &throughPos, const POS &parentPos, const POS &curPos);

    /// 获取第一个方向
    virtual DIRECT GetFirstDirect(const POS &curPos, const POS &dstPos);

    /// 获取下一个方向
    virtual DIRECT GetNextDirect(DIRECT firstDirect, DIRECT curDirect);

    /// 获取下一个方向的坐标
    virtual DIRECT GetNextDirectPos(const POS &parentPos, const POS &curPos, POS &nextPos, DIRECT firstDirect);

    /// 方向遍历的说明: 总是把向着最趋近目的点的方向作为第一个搜索的方向

    /// =============== 重写基类函数 ================
    virtual IGraph::POS FirstAdjVex(const IGraph::POS &parentVex, const IGraph::POS &curVex, const IGraph::POS &dstVex);
    virtual IGraph::POS NextAdjVex(const IGraph::POS &parentVex, const IGraph::POS &curVex, const IGraph::POS &adjVex, const IGraph::POS &dstVex);
    /// =============================================

public:
    /// 获取当前坐标相对上一坐标的方向
    DIRECT GetDirect(const POS &parentPos, const POS &curPos);

    /// 获取上一坐标上的指定方向的坐标
    void GetDirectPos(const POS &parentPos, DIRECT curDirect, POS &curPos);

private:
    DIRECT_TYPE m_directType;                   // 方向类型
    DWORD m_weight;                             // 宽度(X轴)
    DWORD m_hight;                              // 高度(Y轴)
};


#endif // #ifndef _TOOL_GRAPH_MAP_H_

