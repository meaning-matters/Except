/*
 *      List.h - doubly linked list library header
 *
 *  DESCRIPTION
 *      This header belongs to "List.c" and must be included by every module
 *      that uses doubly linked lists.
 *
 *  COPYRIGHT
 *      You are free to use, copy or modify this software at your own risk.
 *
 *  AUTHOR
 *      Cornelis van der Bent.  Please let me know if you have comments or find
 *      flaws: cg_vanderbent@mail.com.  Enjoy!
 *
 *  MODIFICATION HISTORY
 *      1999/04/12 vdbent       Thorough test and debugging; beta release.
 *      1998/12/18 vdbent       Conception.
 */

#ifndef _LIST_H
#define _LIST_H

typedef struct _ListNode        ListNode;
struct _ListNode
{
    ListNode *  pNext;          /* next node ('down', 'after') */
    ListNode *  pPrev;          /* next node ('up', 'before') */
    void *      pData;          /* pointer to user data */
};

typedef struct _List    List;   /* doubly linked list */
struct _List
{
    ListNode *  pHead;          /* pointer to dummy list head node */
    ListNode *  pNodeLast;      /* pointer to last accessed node */
    int         count;          /* number of user nodes in list */
};


extern
List * ListCreate(void);

extern
void ListDestroy(
    List *      pList);         /* pointer to list */

extern
void ListDestroyData(
    List *      pList);         /* pointer to list */

extern
void ListAddHead(
    List *      pList,          /* pointer to list */
    void *      pData);         /* data object value */

extern
void ListAddTail(
    List *      pList,          /* pointer to list */
    void *      pData);         /* data object value */

extern
void ListAddBefore(
    List *      pList,          /* pointer to list */
    void *      pData);         /* data object value */

extern
void ListAddAfter(
    List *      pList,          /* pointer to list */
    void *      pData);         /* data object value */

extern
void * ListRemoveHead(
    List *      pList);         /* pointer to list */

extern
void * ListRemoveTail(
    List *      pList);         /* pointer to list */

extern
void * ListRemove(
    List *      pList,          /* pointer to list */
    void *      pData);         /* data object value */

extern
void * ListRemoveLast(
    List *      pList);         /* pointer to list */

extern
void * ListHead(
    List *      pList);         /* pointer to list */

extern
void * ListTail(
    List *      pList);         /* pointer to list */

extern
void * ListLast(
    List *      pList);         /* pointer to list */

extern
void * ListNext(
    List *      pList);         /* pointer to list */

extern
void * ListPrev(
    List *      pList);         /* pointer to list */

extern
int ListCount(
    List *      pList);         /* pointer to list */

extern
void * ListFind(
    List *      pList,          /* pointer to list */
    void *      pData);         /* data object value */

extern
List * ListSplitBefore(
    List *      pListOrg);      /* pointer to original list */

extern
List * ListSplitAfter(
    List *      pListOrg);      /* pointer to original list */

extern
List * ListConcat(
    List *      pListDst,       /* pointer to destination list */
    List *      pListAdd);      /* pointer to list to be added at tail */


#endif  /* _LIST_H */
