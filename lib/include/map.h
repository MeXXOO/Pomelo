#ifndef     _ME_MAP_H_
#define     _ME_MAP_H_

typedef     struct _IMeMap  IMeMap;

typedef void    (*IMeMapAdd)( IMeMap* pIMeMap , void* obj , void* key );
typedef void    (*IMeMapRemove)( IMeMap* pIMeMap , void* key );
typedef void*   (*IMeMapContain)( IMeMap* pIMeMap , void* key );
typedef void    (*IMeMapDestroy)( IMeMap* pIMeMap );

struct _IMeMap{
    IMeMapAdd   m_pAdd;
    IMeMapRemove    m_pRemove;
    IMeMapContain   m_pContain;
    IMeMapDestroy   m_pDestroy;
};

IME_EXTERN_C IMeMap*     IMeHashMapCreate();

#define     CHashMapCreate()        IMeHashMapCreate()
#define     CMapAdd(p,a,b)          (p->m_pAdd(p,a,b))
#define     CMapRemove(p,a)         (p->m_pRemove(p,a))
#define     CMapContain(p,a)        (p->m_pContain(p,a))
#define     CMapDestroy(p)          (p->m_pDestroy(p))  

#endif  //end _ME_MAP_H_
