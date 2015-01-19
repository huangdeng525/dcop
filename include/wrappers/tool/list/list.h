/// -------------------------------------------------
/// list.h : 链表数据结构公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TOOL_LIST_LIST_H_
#define _TOOL_LIST_LIST_H_


/////////////////////////////////////////////////////
///                List链表说明
/// -------------------------------------------------
/// 来自DHCP的代码，实质是一个单向链表。
/// 比较难理解的可能是LIST_ENTRY中的成员le_prev，它
/// 不是平时双向链表中的前向指针(指向前一个元素)，而
/// 是前向前一个元素的le_next成员的指针(指针的指针)。
/// 好处自然是代码少、效率高。
/// -------------------------------------------------
/// 用法:
/// 节点类型的声明:
///     struct LstNode
///     {
///         LIST_ENTRY(LstNode) m_field;
///         DATETYPE datavalue;
///     };
/// 链表头声明:
///     LIST_HEAD(LstNode) m_head;
/// 初始化链表头:
///     LIST_INIT(&m_head);
/// 初始化链表节点:
///     LIST_NODE_INIT(pNode, m_field);
/// 获取下一个节点:
///     LIST_NEXT(pNode, m_field);
/////////////////////////////////////////////////////

/* 
 * List declarations. 
 */

#define LIST_HEAD(type)                                                         \
    struct {                                                                    \
        type *lh_first;                                                         \
        int count;                                                              \
    }

#define LIST_ENTRY(type)                                                        \
    struct {                                                                    \
        type *le_next;                                                          \
        type **le_prev;                                                         \
    }

/* 
 * List functions. 
 */

#define LIST_EMPTY(head)        (!((head)->lh_first) || !((head)->count))

#define LIST_FIRST(head)        ((head)->lh_first)

#define LIST_COUNT(head)        ((head)->count)

#define LIST_NEXT(elm, field)   ((elm)->field.le_next)

#define LIST_INIT(head)                                                         \
    do {                                                                        \
        LIST_FIRST((head)) = 0;                                                 \
        LIST_COUNT(head) = 0;                                                   \
    } while (0)

#define LIST_NODE_INIT(elm, field)                                              \
    do {                                                                        \
        LIST_NEXT(elm, field) = 0;                                              \
        ((elm)->field.le_prev) = 0;                                             \
    } while (0)

#define LIST_INSERT_HEAD(head, elm, field)                                      \
    do {                                                                        \
        if ((LIST_NEXT((elm), field) = LIST_FIRST((head))) != 0)                \
            LIST_FIRST((head))->field.le_prev = &LIST_NEXT((elm), field);       \
        LIST_FIRST((head)) = (elm);                                             \
        (elm)->field.le_prev = &LIST_FIRST((head));                             \
        LIST_COUNT(head)++;                                                     \
    } while (0)

#define LIST_REMOVE(head, elm, field)                                           \
    do {                                                                        \
        if (LIST_NEXT((elm), field) != 0)                                       \
            LIST_NEXT((elm), field)->field.le_prev = (elm)->field.le_prev;      \
        *((elm)->field.le_prev) = LIST_NEXT((elm), field);                      \
        if ((elm) == LIST_FIRST(head))                                          \
            LIST_FIRST(head) = LIST_NEXT(elm, field);                           \
        LIST_COUNT(head)--;                                                     \
    } while (0)

#define LIST_CLEAR(head, type, field, free)                                     \
    do {                                                                        \
        type *___pNodeLoop = LIST_FIRST(head);                                  \
        while (___pNodeLoop)                                                    \
        {                                                                       \
            type *___pNodeFree = ___pNodeLoop;                                  \
            ___pNodeLoop = LIST_NEXT(___pNodeLoop, field);                      \
            LIST_REMOVE(head, ___pNodeFree, field);                             \
            free(___pNodeFree);                                                 \
        }                                                                       \
        LIST_INIT(head);                                                        \
    } while (0)


#endif // #ifndef _TOOL_LIST_LIST_H_

