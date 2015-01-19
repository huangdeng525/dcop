/// -------------------------------------------------
/// ObjTimer_wheel.h : 定时器的时间轮实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjTimer_wheel.h"


/*******************************************************
  函 数 名: ITimer::IWheel::CreateInstance
  描    述: 创建时间轮实例
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
ITimer::IWheel *ITimer::IWheel::CreateInstance(DWORD dwWheelID,
                        DWORD dwSlotCount,
                        IWheel *pHigherWheel,
                        IWheel *pLowerWheel,
                        DWORD dwHashBase,
                        IWheel::TIMEOUT_PROC fnTimeoutProc,
                        void *pTimeoutPara)
{
    CTimerWheel *pTimerWheel = new CTimerWheel();
    if (!pTimerWheel)
    {
        return NULL;
    }

    pTimerWheel->Init(dwWheelID, dwSlotCount, 
                        (CTimerWheel *)pHigherWheel, (CTimerWheel *)pLowerWheel, 
                        dwHashBase, fnTimeoutProc, pTimeoutPara);
    return pTimerWheel;
}

/*******************************************************
  函 数 名: ITimer::IWheel::~IWheel
  描    述: ITimer::IWheel析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
ITimer::IWheel::~IWheel()
{
}

/*******************************************************
  函 数 名: ITimer::IWheel::Alloc
  描    述: 分配句柄
  输    入: 
  输    出: 
  返    回: 定时器句柄
  修改记录: 
 *******************************************************/
ITimer::Handle ITimer::IWheel::Alloc(void *pValue, DWORD dwLen)
{
    if (!pValue || !dwLen)
    {
        return NULL;
    }

    /// 申请节点空间
    CTimerWheel::TimerNode *pNode = (CTimerWheel::TimerNode *)DCOP_Malloc(sizeof(CTimerWheel::TimerNode) + dwLen);
    if (!pNode)
    {
        return NULL;
    }

    /// 初始化并赋值
    (void)memset(pNode, 0, sizeof(CTimerWheel::TimerNode) + dwLen);
    (void)memcpy(pNode + 1, pValue, dwLen);

    /// 转换定时器句柄
    ITimer::Handle hTimer = (ITimer::Handle)pNode;
    pNode->m_timer = hTimer;

    /// 返回句柄
    return hTimer;
}

/*******************************************************
  函 数 名: ITimer::IWheel::Free
  描    述: 释放句柄
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void ITimer::IWheel::Free(ITimer::Handle handle)
{
    if (!handle)
    {
        return;
    }

    DCOP_Free(handle);
}

/*******************************************************
  函 数 名: CTimerWheel::SetValue
  描    述: 设置用户值
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void ITimer::IWheel::SetValue(ITimer::Handle handle, void *pValue, DWORD dwLen)
{
    CTimerWheel::TimerNode *pNode = (CTimerWheel::TimerNode *)handle;
    if (!pNode)
    {
        return;
    }

    (void)memcpy(pNode + 1, pValue, dwLen);
}

/*******************************************************
  函 数 名: ITimer::IWheel::GetValue
  描    述: 通过句柄获取定时器值
  输    入: handle  - 定时器句柄
  输    出: 
  返    回: 定时器值
  修改记录: 
 *******************************************************/
void *ITimer::IWheel::GetValue(ITimer::Handle handle)
{
    CTimerWheel::TimerNode *pNode = (CTimerWheel::TimerNode *)handle;
    if (!pNode)
    {
        return 0;
    }

    return pNode + 1;
}

/*******************************************************
  函 数 名: ITimer::IWheel::GetWheel
  描    述: 通过句柄获取定时器轮对象
  输    入: handle  - 定时器句柄
  输    出: 
  返    回: 定时器轮对象
  修改记录: 
 *******************************************************/
ITimer::IWheel *ITimer::IWheel::GetWheel(ITimer::Handle handle)
{
    CTimerWheel::TimerNode *pNode = (CTimerWheel::TimerNode *)handle;
    if (!pNode)
    {
        return 0;
    }

    return pNode->m_wheel;
}

/*******************************************************
  函 数 名: ITimer::IWheel::SetTimeBase
  描    述: 设置时间基值
  输    入: handle  - 定时器句柄
            tbase   - 时间基值
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void ITimer::IWheel::SetTimeBase(ITimer::Handle handle, DWORD tbase)
{
    CTimerWheel::TimerNode *pNode = (CTimerWheel::TimerNode *)handle;
    if (!pNode)
    {
        return;
    }

    pNode->m_tbase = tbase;
}

/*******************************************************
  函 数 名: ITimer::IWheel::GetTimeBase
  描    述: 获取时间基值
  输    入: handle  - 定时器句柄
  输    出: 
  返    回: 时间基值
  修改记录: 
 *******************************************************/
DWORD ITimer::IWheel::GetTimeBase(ITimer::Handle handle)
{
    CTimerWheel::TimerNode *pNode = (CTimerWheel::TimerNode *)handle;
    if (!pNode)
    {
        return 0;
    }

    return pNode->m_tbase;
}

/*******************************************************
  函 数 名: ITimer::IWheel::GetIndex
  描    述: 获取时间基值
  输    入: handle  - 定时器句柄
  输    出: 
  返    回: 槽位索引
  修改记录: 
 *******************************************************/
DWORD ITimer::IWheel::GetIndex(ITimer::Handle handle)
{
    CTimerWheel::TimerNode *pNode = (CTimerWheel::TimerNode *)handle;
    if (!pNode)
    {
        return 0;
    }

    return pNode->m_index;
}

/*******************************************************
  函 数 名: ITimer::IWheel::GetTimeBase
  描    述: 获取时间基值
  输    入: handle  - 定时器句柄
  输    出: 
  返    回: 循环次数
  修改记录: 
 *******************************************************/
DWORD ITimer::IWheel::GetCycle(ITimer::Handle handle)
{
    CTimerWheel::TimerNode *pNode = (CTimerWheel::TimerNode *)handle;
    if (!pNode)
    {
        return 0;
    }

    return pNode->m_cycle;
}

/*******************************************************
  函 数 名: ITimer::IWheel::GetHandle
  描    述: 获取实际句柄值(方便复制定时器信息)
  输    入: handle  - 定时器句柄
  输    出: 
  返    回: 实际句柄值(Alloc时的句柄值)
  修改记录: 
 *******************************************************/
ITimer::Handle ITimer::IWheel::GetHandle(ITimer::Handle handle)
{
    CTimerWheel::TimerNode *pNode = (CTimerWheel::TimerNode *)handle;
    if (!pNode)
    {
        return 0;
    }

    return pNode->m_timer;
}

/*******************************************************
  函 数 名: ITimer::IWheel::GetString
  描    述: 获取显示字符串
  输    入: handle  - 定时器句柄
  输    出: 
  返    回: 时间显示字符串
  修改记录: 
 *******************************************************/
void ITimer::IWheel::GetString(ITimer::Handle handle, CDString &str)
{
    ITimer::Node *pTimer = handle;
    if (!pTimer)
    {
        str << "no timer";
        return;
    }

    ITimer::IWheel *pWheel = ITimer::IWheel::GetWheel(handle);
    if (!pWheel)
    {
        str << STR_FORMAT("expired/%d", pTimer->m_tbase);
        return;
    }

    ITimer::Handle hTimer = ITimer::IWheel::GetHandle(handle);
    if (!hTimer)
    {
        str << STR_FORMAT("expired/%d", pTimer->m_tbase);
        return;
    }

    DWORD dwTimeBase = ITimer::IWheel::GetTimeBase(hTimer);
    if (dwTimeBase != hTimer->m_tbase)
    {
        str << STR_FORMAT("expired/%d", pTimer->m_tbase);
        return;
    }

    DWORD dwIndex = ITimer::IWheel::GetIndex(hTimer);
    if (dwIndex != hTimer->m_index)
    {
        str << STR_FORMAT("expired/%d", pTimer->m_tbase);
        return;
    }
    
    DWORD dwCycle = ITimer::IWheel::GetCycle(hTimer);
    DWORD dwSlotCount = pWheel->GetSlotCount();
    DWORD dwScalePoint = pWheel->GetScalePoint();
    DWORD dwLeft = dwCycle * dwSlotCount + ((dwIndex > dwScalePoint)? 
            (dwIndex - dwScalePoint) : (dwSlotCount - dwScalePoint + dwIndex));
    str << STR_FORMAT("%d/%d", dwLeft, dwTimeBase);
}

/*******************************************************
  函 数 名: CTimerWheel::CTimerWheel
  描    述: CTimerWheel构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CTimerWheel::CTimerWheel()
{
    m_pHigherWheel      = 0;
    m_pLowerWheel       = 0;
    m_dwWheelID         = 0;
    m_dwHashBase        = 0;
    m_pSlotTable        = 0;
    m_dwSlotCount       = 0;
    m_dwScalePoint      = 0;
    m_fnTimeoutProc     = 0;
    m_pTimeoutPara      = 0;
}

/*******************************************************
  函 数 名: CTimerWheel::~CTimerWheel
  描    述: CTimerWheel析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CTimerWheel::~CTimerWheel()
{
    m_pHigherWheel      = 0;
    m_pLowerWheel       = 0;
    m_dwWheelID         = 0;
    m_dwHashBase        = 0;

    if (m_pSlotTable)
    {
        for (DWORD i = 0; i < m_dwSlotCount; ++i)
            DLL_CLEAR((&(m_pSlotTable[i])), TimerNode, m_field, DCOP_Free);

        DCOP_Free(m_pSlotTable);
        m_pSlotTable = 0;
    }

    m_dwSlotCount       = 0;
    m_dwScalePoint      = 0;
    m_fnTimeoutProc     = 0;
    m_pTimeoutPara      = 0;
}

/*******************************************************
  函 数 名: CTimerWheel::Init
  描    述: 初始化入口
  输    入: dwWheelID       - 定时器轮ID
            dwSlotCount     - 槽位总数
            pHigherWheel    - 高级别的定时器轮
            pLowerWheel     - 低级别的定时器轮
            dwHashBase      - HASH基值
            fnTimeoutProc   - 超时处理回调
            pTimeoutPara    - 超时回调参数
  输    出: 
  返    回: 成功或者失败的错误码
  修改记录: 
 *******************************************************/
DWORD CTimerWheel::Init(DWORD dwWheelID, DWORD dwSlotCount, 
                    CTimerWheel *pHigherWheel, CTimerWheel *pLowerWheel,
                    DWORD dwHashBase, TIMEOUT_PROC fnTimeoutProc, void *pTimeoutPara)
{
    if (m_pSlotTable)
    {
        return FAILURE;
    }

    if (!dwSlotCount)
    {
        return FAILURE;
    }

    m_pSlotTable = (TimerSlot *)DCOP_Malloc(dwSlotCount * sizeof(TimerSlot));
    if (!m_pSlotTable)
    {
        return FAILURE;
    }

    m_dwWheelID = dwWheelID;
    (void)memset(m_pSlotTable, 0, dwSlotCount * sizeof(TimerSlot));
    m_dwSlotCount       = dwSlotCount;
    m_pHigherWheel      = pHigherWheel;
    m_pLowerWheel       = pLowerWheel;
    m_dwHashBase        = dwHashBase;
    m_fnTimeoutProc     = fnTimeoutProc;
    m_pTimeoutPara      = pTimeoutPara;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CTimerWheel::OnTick
  描    述: 定时器TICK递增
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTimerWheel::OnTick()
{
    if (!m_pSlotTable) return;

    m_dwScalePoint++;
    if (m_dwScalePoint >= m_dwSlotCount)
    {
        m_dwScalePoint %= m_dwSlotCount;
        if (m_pHigherWheel)
        {
            m_pHigherWheel->OnTick();
        }
    }

    if (!DLL_EMPTY(&(m_pSlotTable[m_dwScalePoint])))
    {
        SlotTimeout(&(m_pSlotTable[m_dwScalePoint]));
    }
}

/*******************************************************
  函 数 名: CTimerWheel::Add
  描    述: 添加句柄到定时器轮
            [定时器只能从高级轮往低级轮依次添加]
            [高级轮的'HashBase'==低级轮的'SlotCount*HashBase']
  输    入: handle  - 定时器句柄
  输    出: 
  返    回: 成功或者失败的错误码
  修改记录: 
 *******************************************************/
DWORD CTimerWheel::Add(ITimer::Handle handle)
{
    if (!m_pSlotTable) return FAILURE;

    /////////////////////////////////////////////////
    /// 定时器必有句柄和超时值
    /////////////////////////////////////////////////
    TimerNode *pNode = (TimerNode *)handle;
    if (!pNode || !pNode->m_tbase)
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 本级值加上附加值，再除去高级值
    /////////////////////////////////////////////////
    DWORD dwBase = m_dwSlotCount * m_dwHashBase;
    DWORD dwTime = (pNode->m_tbase + pNode->m_extra) % dwBase;
    DWORD dwLoop = (pNode->m_tbase + pNode->m_extra) / dwBase;

    /////////////////////////////////////////////////
    /// 附加值使用过了就失效处理
    /////////////////////////////////////////////////
    pNode->m_extra = 0;

    /////////////////////////////////////////////////
    /// 如果有高级轮，则循环无效
    /////////////////////////////////////////////////
    if (m_pHigherWheel)
    {
        dwLoop = 0;
    }

    /////////////////////////////////////////////////
    /// 如果精度小于本级轮一个刻度，向低级轮传递
    /////////////////////////////////////////////////
    if (!dwLoop && (dwTime < m_dwHashBase))
    {
        if (dwTime && m_pLowerWheel)
        {
            /// 低级轮处理更小精度
            return m_pLowerWheel->Add(handle);
        }
        else
        {
            /// 无精度更小的低级轮
            if (m_pHigherWheel)
            {
                /// 如果是高级轮过来的，就超时处理并返回成功
                HandleTimeout(handle);
                return SUCCESS;
            }

            /// 直接添加了一个当前精度不满足的时间，只有返回错误了
            return FAILURE;
        }
    }

    /////////////////////////////////////////////////
    /// 低级值的当前位置作为附加值，同时本级值加上该附加值
    /////////////////////////////////////////////////
    if (m_pLowerWheel)
    {
        pNode->m_extra = m_pLowerWheel->GetScalePoint() * m_pLowerWheel->GetHashBase();
        dwTime += pNode->m_extra;
    }

    /////////////////////////////////////////////////
    /// 获取本级轮刻度索引值，插入链表
    /////////////////////////////////////////////////
    DWORD dwIndex = ((dwTime / m_dwHashBase) + m_dwScalePoint) % m_dwSlotCount;
    pNode->m_wheel = this;
    pNode->m_index = dwIndex;
    pNode->m_cycle = dwLoop;
    DLL_INSERT_TAIL((&(m_pSlotTable[dwIndex])), pNode, m_field);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CTimerWheel::Del
  描    述: 从定时器轮中删除句柄
  输    入: handle  - 定时器句柄
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTimerWheel::Del(ITimer::Handle handle)
{
    if (!m_pSlotTable) return;

    TimerNode *pNode = (TimerNode *)handle;
    if (!pNode)
    {
        return;
    }

    if (!pNode->m_wheel)
    {
        return;
    }

    if (pNode->m_wheel->GetWheelID() != m_dwWheelID)
    {
        return;
    }

    if (!DLL_NEXT(pNode, m_field) || !DLL_PREV(pNode, m_field))
    {
        return;
    }

    DLL_REMOVE(&(m_pSlotTable[pNode->m_index]), pNode, m_field);
    pNode->m_wheel = NULL;
}

/*******************************************************
  函 数 名: CTimerWheel::SlotTimeout
  描    述: 槽位超时处理
  输    入: pSlot   - 槽位
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTimerWheel::SlotTimeout(TimerSlot *pSlot)
{
    TimerNode *pNode = DLL_FIRST(pSlot);
    while (pNode)
    {
        TimerNode *pNodeTmp = pNode;
        pNode = DLL_NEXT_LOOP(pSlot, pNode, m_field);
        if (pNodeTmp->m_cycle)
        {
            /// 需要多次循环，继续在链表上保留
            pNodeTmp->m_cycle--;
            continue;
        }

        /// 从链表上移除，往低级轮添加或者超时处理
        DLL_REMOVE(pSlot, pNodeTmp, m_field);
        pNodeTmp->m_wheel = NULL;
        if (m_pLowerWheel)
        {
            (void)m_pLowerWheel->Add((ITimer::Handle)pNodeTmp);
        }
        else
        {
            HandleTimeout((ITimer::Handle)pNodeTmp);
        }
    }

}

/*******************************************************
  函 数 名: CTimerWheel::HandleTimeout
  描    述: 句柄超时处理
  输    入: handle  - 句柄
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTimerWheel::HandleTimeout(ITimer::Handle handle)
{
    CTimerWheel::TimerNode *pNode = (CTimerWheel::TimerNode *)handle;
    if (!pNode)
    {
        return;
    }

    /// 置空临时附加值，防止下次重复使用
    pNode->m_extra = 0;

    if (m_fnTimeoutProc) (*m_fnTimeoutProc)(handle, m_pTimeoutPara);
}

