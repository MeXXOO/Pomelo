#ifndef _ME_MEMORY_H_
#define _ME_MEMORY_H_

#define     IMeMemory_Default       1

typedef struct _IMeMemory   IMeMemory;

IME_EXTERN_C IMeMemory*   IMeMemoryCreate( int nType );

typedef void*   (*IMeMemoryAlloc)( IMeMemory* pIMemory , int nSize );
typedef void*   (*IMeMemoryCalloc)( IMeMemory* pIMemory , int nSize );
typedef void*   (*IMeMemoryRealloc)( IMeMemory* pIMemory , void* pMem , int nSize );
typedef void    (*IMeMemoryFree)( IMeMemory* pIMemory , void* pMem );
typedef void    (*IMeMemoryDestroy)( IMeMemory* pIMemory );

struct _IMeMemory{
    IMeMemoryAlloc   m_pAlloc;
    IMeMemoryCalloc  m_pCalloc;
    IMeMemoryRealloc    m_pRealloc;
    IMeMemoryFree  m_pFree;
    IMeMemoryDestroy  m_pDestroy;
};

#define     CMemoryCreate(a)        IMeMemoryCreate(a)
#define     CMemoryAlloc(p,a)       (p->m_pAlloc(p,a))
#define     CMemoryCalloc(p,a)      (p->m_pCalloc(p,a))
#define     CMemoryRealloc(p,a,b)   (p->m_pRealloc(p,a,b))
#define     CMemoryFree(p,a)        (p->m_pFree(p,a))
#define     CMemoryDestroy(p)       (p->m_pDestroy(p))


#endif
