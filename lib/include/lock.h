#ifndef     _ME_LOCK_H_
#define     _ME_LOCK_H_

typedef struct _IMeLock     IMeLock;

IME_EXTERN_C IMeLock*    IMeLockCreate();

typedef void    (*IMeLock_Lock)( IMeLock* pILock );
typedef void    (*IMeLock_Unlock)( IMeLock* pILock );
typedef void    (*IMeLock_Destroy)( IMeLock* pILock );

struct _IMeLock{
    IMeLock_Lock    m_pLock;
    IMeLock_Unlock  m_pUnlock;
    IMeLock_Destroy m_pDestroy;
};

#define     CLock_Create()      IMeLockCreate()
#define     CLock_Lock(p)       (p->m_pLock(p))
#define     CLock_Unlock(p)     (p->m_pUnlock(p))
#define     CLock_Destroy(p)    (p->m_pDestroy(p))

#endif
