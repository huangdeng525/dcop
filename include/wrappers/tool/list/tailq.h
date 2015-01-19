/// -------------------------------------------------
/// tailq.h : 链表工具类公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_LIST_TAILQ_H_
#define _TOOL_LIST_TAILQ_H_


/////////////////////////////////////////////////////
///          TailQ单向链表(首节点+尾节点)
/// -------------------------------------------------
/// 单链表实现的队列
/// -------------------------------------------------
/// 用法:
/// 节点类型的声明:
///     struct TailNode
///     {
///         TAILQ_ENTRY(TailNode) m_field;
///         DATETYPE datavalue;
///     };
/// 链表头声明:
///     TAILQ_HEAD(TailNode) m_head;
/// 初始化链表头:
///     TAILQ_INIT(&m_head);
/// 初始化链表节点:
///     TAILQ_NODE_INIT(pNode, m_field);
/// 获取下一个节点:
///     TAILQ_NEXT(pNode, m_field);
/////////////////////////////////////////////////////

/* 
 * TailQ declarations. 
 */

#define TAILQ_HEAD(type)                                                        \
    struct {                                                                    \
        type *lh_first;                                                         \
        type *lh_last;                                                          \
        int count;                                                              \
    }

#define TAILQ_ENTRY(type)                                                       \
    struct {                                                                    \
        type *le_next;                                                          \
    }


/* 
 * TailQ functions. 
 */

#define TAILQ_EMPTY(head)   (!((head)->lh_first) || !((head)->lh_last) || !((head)->count))

#define TAILQ_FIRST(head)   ((head)->lh_first)

#define TAILQ_LAST(head)    ((head)->lh_last)

#define TAILQ_COUNT(head)   ((head)->count)

#define TAILQ_NEXT(elm, field)  ((elm)->field.le_next)

#define TAILQ_INIT(head)                                                        \
    do {                                                                        \
        TAILQ_FIRST(head) = 0;                                                  \
        TAILQ_LAST(head) = 0;                                                   \
        TAILQ_COUNT(head) = 0;                                                  \
    } while (0)

#define TAILQ_NODE_INIT(elm, field)                                             \
    do {                                                                        \
        TAILQ_NEXT(elm, field) = 0;                                             \
    } while (0)

#define TAILQ_PUSH_FRONT(head, elm, field)                                      \
    do {                                                                        \
        if (!(TAILQ_NEXT(elm, field) = TAILQ_FIRST(head)))                      \
            TAILQ_LAST(head) = (elm);                                           \
        TAILQ_FIRST(head) = (elm);                                              \
        TAILQ_COUNT(head)++;                                                    \
    } while (0)

#define TAILQ_PUSH_BACK(head, elm, field)                                       \
    do {                                                                        \
        if (TAILQ_EMPTY(head))                                                  \
            TAILQ_FIRST(head) = (elm);                                          \
        else                                                                    \
            TAILQ_NEXT(TAILQ_LAST(head), field) = (elm);                        \
        TAILQ_NEXT(elm, field) = 0;                                             \
        TAILQ_LAST(head) = (elm);                                               \
        TAILQ_COUNT(head)++;                                                    \
    } while (0)

#define TAILQ_POP(head, field)                                                  \
    do {                                                                        \
        if (TAILQ_EMPTY(head)) break;                                           \
        if (!(TAILQ_FIRST(head) = TAILQ_NEXT(TAILQ_FIRST(head), field)    ))    \
            TAILQ_LAST(head) = 0;                                               \
        TAILQ_COUNT(head)--;                                                    \
    } while (0)

#define TAILQ_CLEAR(head, type, field, free)                                    \
    do {                                                                        \
        type *___pNodeLoop = TAILQ_FIRST(head);                                 \
        while (___pNodeLoop)                                                    \
        {                                                                       \
            type *___pNodeFree = ___pNodeLoop;                                  \
            ___pNodeLoop = TAILQ_NEXT(___pNodeLoop, field);                     \
            free(___pNodeFree);                                                 \
        }                                                                       \
        TAILQ_INIT(head);                                                       \
    } while (0)


#endif // #ifndef _TOOL_LIST_TAILQ_H_

