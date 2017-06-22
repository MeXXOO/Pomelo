#ifndef     _ME_THREAD_H_
#define     _ME_THREAD_H_

typedef struct _IMeThread  IMeThread;

IME_EXTERN_C IMeThread* IMeThreadCreate( void* func , void* parameter );

typedef void    (*IMeThreadExit)( IMeThread* pIThread , uint dwMilliseconds );
typedef void    (*IMeThreadDestroy)( IMeThread* pIThread );

struct _IMeThread{
    IMeThreadExit   m_pExit;
    IMeThreadDestroy    m_pDestroy;
};

#define     CThreadCreate(a,b)      IMeThreadCreate(a,b)
#define     CThreadExit(p,a)        (p->m_pExit(p,a))
#define     CThreadDestroy(p)       (p->m_pDestroy(p))

#endif
