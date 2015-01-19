/// -------------------------------------------------
/// file : dllist.h - 双向循环链表头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_LIST_DLLIST_H_
#define _TOOL_LIST_DLLIST_H_


/////////////////////////////////////////////////////
///     DoubleLoopList双向循环链表(首节点+循环)
/// -------------------------------------------------
/// 单链表比双链表容易递归处理。
/// 双向链表用的空间多，插入删除操作比单链表好。
/// -------------------------------------------------
/// 用法:
/// 节点类型的声明:
///     struct DllNode
///     {
///         DLL_ENTRY(DllNode) m_field;
///         DATETYPE datavalue;
///     };
/// 链表头声明:
///     DLL_HEAD(DllNode) m_head;
/// 初始化链表头:
///     DLL_INIT(&m_head);
/// 初始化链表节点:
///     DLL_NODE_INIT(pNode, m_field);
/// 非循环中获取下一个节点 或者 作为左值去设置下一个节点(由于是双向循环, 永远不会返回空):
///     DLL_NEXT(pNode, m_field);
/// 循环中获取下一个节点(只能作为右值, 双向循环到最后一个节点时会返回空, 所以要传入头):
///     DLL_NEXT_LOOP(&m_head, pNode, m_field);
/////////////////////////////////////////////////////

/* 
 * DoubleLoopList declarations. 
 */

#define DLL_HEAD(type)                                                          \
    struct {                                                                    \
        type *lh_first;                                                         \
        int count;                                                              \
    }

#define DLL_ENTRY(type)                                                         \
    struct {                                                                    \
        type *le_next;                                                          \
        type *le_prev;                                                          \
    }

/* 
 * DoubleLoopList functions. 
 */

#define DLL_EMPTY(head)   (!((head)->lh_first) || !((head)->count))

#define DLL_FIRST(head)   ((head)->lh_first)

#define DLL_COUNT(head)   ((head)->count)

#define DLL_NEXT(elm, field)  ((elm)->field.le_next)
#define DLL_PREV(elm, field)  ((elm)->field.le_prev)

#define DLL_NEXT_LOOP(head, elm, field) ((DLL_NEXT(elm, field) == DLL_FIRST(head))? 0 : DLL_NEXT(elm, field))
#define DLL_PREV_LOOP(head, elm, field) ((DLL_PREV(elm, field) == (elm))? 0 : DLL_PREV(elm, field))

#define DLL_INIT(head)                                                          \
    do {                                                                        \
        DLL_FIRST(head) = 0;                                                    \
        DLL_COUNT(head) = 0;                                                    \
    } while (0)

#define DLL_NODE_INIT(elm, field)                                               \
    do {                                                                        \
        DLL_NEXT(elm, field) = 0;                                               \
        DLL_PREV(elm, field) = 0;                                               \
    } while (0)

#define DLL_INSERT_HEAD(head, elm, field)                                       \
    do {                                                                        \
        if (DLL_EMPTY(head)) {                                                  \
            DLL_NEXT(elm, field) = (elm);                                       \
            DLL_PREV(elm, field) = (elm); }                                     \
        else {                                                                  \
            DLL_NEXT(elm, field) = DLL_FIRST(head);                             \
            DLL_PREV(elm, field) = DLL_PREV(DLL_FIRST(head), field));           \
            DLL_NEXT(DLL_PREV(DLL_FIRST(head), field), field) = (elm);          \
            DLL_PREV(DLL_FIRST(head), field) = (elm); }                         \
        DLL_FIRST(head) = (elm);                                                \
        DLL_COUNT(head)++;                                                      \
    } while (0)

#define DLL_INSERT_TAIL(head, elm, field)                                       \
    do {                                                                        \
        if (DLL_EMPTY(head)) {                                                  \
            DLL_FIRST(head) = (elm);                                            \
            DLL_NEXT(elm, field) = (elm);                                       \
            DLL_PREV(elm, field) = (elm); }                                     \
        else {                                                                  \
            DLL_NEXT(elm, field) = DLL_FIRST(head);                             \
            DLL_PREV(elm, field) = DLL_PREV(DLL_FIRST(head), field);            \
            DLL_NEXT(DLL_PREV(DLL_FIRST(head), field), field) = (elm);          \
            DLL_PREV(DLL_FIRST(head), field) = (elm); }                         \
        DLL_COUNT(head)++;                                                      \
    } while (0)

#define DLL_REMOVE(head, elm, field)                                            \
    do {                                                                        \
        if ((DLL_NEXT(elm, field) == elm) ||                                    \
            (DLL_PREV(elm, field) == elm))                                      \
            DLL_FIRST(head) = 0;                                                \
        else {                                                                  \
            DLL_NEXT(DLL_PREV(elm, field), field) =                             \
            DLL_NEXT(elm, field);                                               \
            DLL_PREV(DLL_NEXT(elm, field), field) =                             \
            DLL_PREV(elm, field);                                               \
            if ((elm) == DLL_FIRST(head))                                       \
                DLL_FIRST(head) = DLL_NEXT(elm, field); }                       \
        DLL_COUNT(head)--;                                                      \
    } while (0)

#define DLL_CLEAR(head, type, field, free)                                      \
    do {                                                                        \
        type *___pNodeLoop = DLL_FIRST(head);                                   \
        while (___pNodeLoop)                                                    \
        {                                                                       \
            type *___pNodeFree = ___pNodeLoop;                                  \
            ___pNodeLoop = DLL_NEXT_LOOP(head, ___pNodeLoop, field);            \
            DLL_REMOVE(head, ___pNodeFree, field);                              \
            free(___pNodeFree);                                                 \
        }                                                                       \
        DLL_INIT(head);                                                         \
    } while (0)


#endif // #ifndef _TOOL_LIST_DLLIST_H_

