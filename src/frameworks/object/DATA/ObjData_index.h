/// -------------------------------------------------
/// ObjData_index.h : 记录索引私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJDATA_INDEX_H_
#define _OBJDATA_INDEX_H_

#define INC_MAP

#include "ObjData_handle.h"
#include "BaseMessage.h"


/// 记录索引类
class CRecIdx
{
public:

    /// 关键字
    class CKey
    {
    public:
        CKey();
        CKey(DWORD dwLen);
        CKey(const void *cpKey, DWORD dwLen);
        CKey(const CKey &rKey);
        ~CKey();

        void *GetKey() const {return m_pKey;}
        DWORD GetLen() const {return m_dwLen;}

        CKey& operator=(const void *cpKey);
        CKey& operator=(const CKey &rKey);
        bool operator<(const CKey &rKey) const;
        bool operator==(const CKey &rKey) const;

    private:
        void *m_pKey;
        DWORD m_dwLen;
    };

    class CIdx
    {
    public:
        CIdx();
        CIdx(void *ptr);
        CIdx(DWORD pos);
        ~CIdx();

        operator void *()
        {
            return m_idx.m_ptr;
        }

        operator DWORD()
        {
            return m_idx.m_pos;
        }

    private:
        union
        {
            void *m_ptr;
            DWORD m_pos;
        }m_idx;
    };

    /// 记录索引MAP
    typedef std::map<CKey, CIdx> MAP_RECIDX;
    typedef MAP_RECIDX::iterator IT_RECIDX;

    /// 字段索引MAP
    typedef std::map<CKey, MAP_RECIDX> MAP_FLDIDX;
    typedef MAP_FLDIDX::iterator IT_FLDIDX;

public:
    CRecIdx();
    ~CRecIdx();

    /// 添加关键字(还需要遍历记录并调用BldKeyIdx)
    DWORD AddKey(DCOP_PARA_NODE *pPara, DWORD dwParaCount);

    /// 删除关键字
    DWORD DelKey(DCOP_PARA_NODE *pPara, DWORD dwParaCount);

    /// 添加整条记录触发的操作
    DWORD OnAddRec(IDataHandle::Field *pFields, BYTE *pbyRec, CIdx recordIdx);

    /// 删除整条记录触发的操作
    void OnDelRec(IDataHandle::Field *pFields, BYTE *pbyRec);

    /// 构建关键字索引(构建全部索引请遍历记录并调用OnAddRec)
    DWORD BldKeyIdx(DCOP_PARA_NODE *pPara, DWORD dwParaCount, 
                        IDataHandle::Field *pFields, BYTE *pbyRec, 
                        CIdx recordIdx);

    /// 查找记录
    CIdx FindRec(DCOP_PARA_NODE *pPara, DWORD dwParaCount, 
                        void *pData, DWORD dwDataLen);

    /// 清除关键索引的操作/类型(只按字段ID和大小进行索引)
    void ClearType(CKey &fieldKey);

private:
    MAP_FLDIDX m_idx;
};



#endif // #ifndef _OBJDATA_INDEX_H_

