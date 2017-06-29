#ifndef _ME_ARRAY_H_
#define _ME_ARRAY_H_

typedef struct _IMeArray    IMeArray;

#define     SORT_INC    1
#define     SORT_DEC    2
#define     SORT_NULL   3

//keyValue1==keyValue2 ==> 0
//keyValue1 < keyValue2 ==> 1
//keyValue1 > keyValue2 ==> -1
typedef int (*IMeArrayCompareKeyValue)( uint64_t keyValue1, uint64_t keyValue2, void* parameter );

IME_EXTERN_C IMeArray*       IMeArrayCreate( int sortType );

typedef void    (*IMeArrayDestroy)( IMeArray* pIArray );

typedef int     (*IMeArrayAdd)( IMeArray* pIArray , void* pElement , uint64_t keyValue );

typedef void*   (*IMeArrayRemoveAt)( IMeArray* pIArray , int nIndex );
typedef void    (*IMeArrayRemoveAll)( IMeArray* pIArray );
typedef void*   (*IMeArrayRemove)( IMeArray* pIArray , uint64_t keyValue );

typedef void*   (*IMeArrayGetAt)( IMeArray* pIArray , int nIndex );
typedef int     (*IMeArrayFind)( IMeArray* pIArray , uint64_t keyValue );
typedef void*   (*IMeArrayFindData)( IMeArray* pIArray , uint64_t keyValue );

typedef int     (*IMeArrayGetSize)( IMeArray* pIArray );

typedef void    (*IMeArrayCopy)( IMeArray* pIArray, const IMeArray* psrc );

typedef void    (*IMeArraySetCompare)( IMeArray* pIArray, IMeArrayCompareKeyValue compareKeyValue, void* parameter );

struct _IMeArray{
    IMeArraySetCompare  m_pSetCompare;
    IMeArrayAdd m_pAdd;
    IMeArrayRemoveAll   m_pRemoveAll;
    IMeArrayRemoveAt    m_pRemoveAt;
    IMeArrayRemove  m_pRemove;
    IMeArrayFind    m_pFind;
    IMeArrayFindData    m_pFindData;
    IMeArrayGetAt   m_pGetAt;
    IMeArrayGetSize m_pGetSize;
    IMeArrayCopy    m_pCopy;
    IMeArrayDestroy m_pDestroy;
};

#define     CArrayCreate(a)             IMeArrayCreate(a);
#define     CArrayAdd(p,a,b)            (p->m_pAdd(p,a,b))

#define     CArrayRemoveAt(p,a)         (p->m_pRemoveAt(p,a))
#define     CArrayRemoveAll(p)          (p->m_pRemoveAll(p))
#define     CArrayRemove(p,a)           (p->m_pRemove(p,a))

#define     CArrayFind(p,a)             (p->m_pFind(p,a))
#define     CArrayFindData(p,a)         (p->m_pFindData(p,a))

#define     CArrayGetAt(p,a)            (p->m_pGetAt(p,a))
#define     CArrayGetSize(p)            (p->m_pGetSize(p))

#define     CArrayCopy(p,a)             (p->m_pCopy(p,a))

#define     CArrayDestroy(p)            (p->m_pDestroy(p))

#define     CArraySetCompare(p,a,b)     (p->m_pSetCompare(p,a,b))

#endif