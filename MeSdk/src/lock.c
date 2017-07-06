#include    "../include/include.h"

#ifdef  PROJECT_FOR_WINDOWS

typedef  struct   _IMeCLock{
    IMeLock interfacefunction;
    CRITICAL_SECTION    cs;
}IMeCLock;

void    IMeCLock_Lock( IMeLock* pILock )
{
    IMeCLock* pLock = (IMeCLock*)pILock;
    if( pLock )
        EnterCriticalSection( &pLock->cs );
}

void    IMeCLock_Unlock( IMeLock* pILock )
{
    IMeCLock* pLock = (IMeCLock*)pILock;
    if( pLock )
        LeaveCriticalSection( &pLock->cs );
}

void    IMeCLock_Destroy( IMeLock* pILock )
{
    IMeCLock* pLock = (IMeCLock*)pILock;
    if( pLock )
    {
        DeleteCriticalSection( &pLock-> cs );
        free( pLock );
    }
}

IME_EXTERN_C IMeLock*    IMeLockCreate()
{
    IMeCLock* pLock = (IMeCLock*)calloc(1,sizeof(IMeCLock));
    if( pLock )
    {
        InitializeCriticalSection(&pLock->cs);

        ((IMeLock*)pLock)->m_pDestroy = IMeCLock_Destroy;
        ((IMeLock*)pLock)->m_pLock = IMeCLock_Lock;
        ((IMeLock*)pLock)->m_pUnlock = IMeCLock_Unlock;
    }

    return (IMeLock*)pLock;
}

#elif   PROJECT_FOR_LINUX

typedef  struct   _IMeCLock{
    IMeLock interfacefunction;
    pthread_mutexattr_t mutex_attr;
    pthread_mutex_t mutex;
}IMeCLock;

void    IMeCLock_Lock( IMeLock* pILock )
{
    IMeCLock* pLock = (IMeCLock*)pILock;
    if( pLock )
        pthread_mutex_lock( &pLock->mutex );
}

void    IMeCLock_Unlock( IMeLock* pILock )
{
    IMeCLock* pLock = (IMeCLock*)pILock;
    if( pLock )
        pthread_mutex_unlock( &pLock->mutex );
}

void    IMeCLock_Destroy( IMeLock* pILock )
{
    IMeCLock* pLock = (IMeCLock*)pILock;
    if( pLock )
    {
        pthread_mutex_destroy( &pLock->mutex );
        pthread_mutexattr_destroy( &pLock->mutex_attr );
        free( pLock );
    }
}

IME_EXTERN_C IMeLock*    IMeLockCreate()
{
    IMeCLock* pLock = (IMeCLock*)calloc(1,sizeof(IMeCLock));
    while( pLock )
    {
        if( pthread_mutexattr_init( &pLock->mutex_attr ) != 0 )
        {
            DebugLogString( TRUE , "[IMeCLockCreate] pthread_mutexattr_init failed!" );
            free(pLock);
            pLock = NULL;
            break;
        }

        pthread_mutexattr_settype( &pLock->mutex_attr , PTHREAD_MUTEX_RECURSIVE_NP );    /* 嵌套锁 */
        
        if( pthread_mutex_init( &pLock->mutex , &pLock->mutex_attr ) != 0 )
        {
            DebugLogString( TRUE , "[IMeCLockCreate] pthread_mutex_init failed!" );
            pthread_mutexattr_destroy( &pLock->mutex_attr );
            free(pLock);
            pLock = NULL;
            break;
        }
        
        ((IMeLock*)pLock)->m_pDestroy = IMeCLock_Destroy;
        ((IMeLock*)pLock)->m_pLock = IMeCLock_Lock;
        ((IMeLock*)pLock)->m_pUnlock = IMeCLock_Unlock;

        break;
    }
    
    return (IMeLock*)pLock;
}

#endif

