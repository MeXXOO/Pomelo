#ifndef             _ME_LIST_H_
#define             _ME_LIST_H_

#include    "platdefine.h"

typedef struct _IMeList     IMeList;


IME_EXTERN_C IMeList*    IMeCListCreate();

typedef lposition   (*IMeListAddTail)(IMeList* pIList , void* data );
typedef lposition   (*IMeListAddHead)(IMeList* pIList , void* data );
typedef void*       (*IMeListRemoveHead)(IMeList* pIList );
typedef void*       (*IMeListRemoveTail)(IMeList* pIList );
typedef int         (*IMeListGetCount)(IMeList* pIList);
typedef void*       (*IMeListGetNext)(IMeList* pIList , lposition* position );
typedef void*       (*IMeListGetPre)(IMeList* pIList , lposition* position );
typedef void*       (*IMeListGetHead)(IMeList* pIList , lposition* position );
typedef void*       (*IMeListGetTail)(IMeList* pIList , lposition* position );
typedef void        (*IMeListDestroy)(IMeList* pIList);
typedef void        (*IMeListOutPut)(IMeList* pIList);

struct _IMeList{
    IMeListAddTail  m_pAddTail;
    IMeListAddHead  m_pAddHead;
    IMeListRemoveHead   m_pRemoveHead;
    IMeListRemoveTail   m_pRemoveTail;
    IMeListGetCount m_pGetCount;
    IMeListGetNext  m_pGetNext;
    IMeListGetPre   m_pGetPre;
    IMeListGetHead  m_pGetHead;
    IMeListGetTail  m_pGetTail;
    IMeListDestroy  m_pDestroy;
    IMeListOutPut   m_pOutPut;
};

#define     CListCreate()           IMeCListCreate()
#define     CListAddTail(p,a)       (p->m_pAddTail(p,a))
#define     CListAddHead(p,a)       (p->m_pAddHead(p,a))
#define     CListRemoveHead(p)      (p->m_pRemoveHead(p))
#define     CListRemoveTail(p)      (p->m_pRemoveTail(p))
#define     CListInsert(p,a,b)      (p->m_pInsert(p,a,b))
#define     CListRemove(p,a)        (p->m_pRemove(p,a))
#define     CListRemoveAt(p,a)      (p->m_pRemoveAt(p,a))
#define     CListFind(p,a)          (p->m_pFind(p,a))
#define     CListGetPosition(p,a)   (p->m_pGetPosition(p,a))
#define     CListRemoveAll(p)       (p->m_pRemoveAll(p))
#define     CListGetCount(p)        (p->m_pGetCount(p))
#define     CListGetAt(p,a)         (p->m_pGetAt(p,a))
#define     CListGetNext(p,a)       (p->m_pGetNext(p,a))
#define     CListGetPre(p,a)        (p->m_pGetPre(p,a))
#define     CListGetHead(p,a)       (p->m_pGetHead(p,a))
#define     CListGetTail(p,a)       (p->m_pGetTail(p,a))
#define     CListDestroy(p)         (p->m_pDestroy(p))
#define     CListOutPut(p)          (p->m_pOutPut(p))
#define     CListSetCompare(p,a)    (p->m_pSetCompareKey(p,a))

#endif              //END _ME_LIST_H_
