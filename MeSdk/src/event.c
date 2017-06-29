#include    "../include/include.h"

#ifdef  PROJECT_FOR_WINDOWS

typedef struct _IMeCEvent{
    IMeEvent    interfacefunction;
    HANDLE  m_hEvent;
}IMeCEvent;

void    IMeCEventSet( IMeEvent* pIEvent )
{
    IMeCEvent* pEvent = (IMeCEvent*)pIEvent;
    if( !pEvent || !pEvent->m_hEvent ) return;

    if( !SetEvent( pEvent->m_hEvent ) )
    {
        DebugLogString( TRUE , "[IMeCEventSet] failed!" );
    }
}

void    IMeCEventReset( IMeEvent* pIEvent )
{
    IMeCEvent* pEvent = (IMeCEvent*)pIEvent;
    if( !pEvent || !pEvent->m_hEvent ) return;
    
    if( !ResetEvent( pEvent->m_hEvent ) )
    {
        DebugLogString( TRUE , "[IMeCEventReset] failed!" );
    }
}

uint8_t   IMeCEventWait( IMeEvent* pIEvent , uint32_t dwMilliseconds )
{
    IMeCEvent* pEvent = (IMeCEvent*)pIEvent;
    uint32_t bWaitRes = WAIT_TIMEOUT;
    
    if( !pEvent || !pEvent->m_hEvent ) return FALSE;

    bWaitRes = WaitForSingleObject( pEvent->m_hEvent , dwMilliseconds );
    
    return bWaitRes==WAIT_OBJECT_0;
}

void    IMeCEventDestroy( IMeEvent* pIEvent )
{
    IMeCEvent* pEvent = (IMeCEvent*)pIEvent;
    
    if( !pEvent || !pEvent->m_hEvent ) return;

    CloseHandle( pEvent->m_hEvent );
    
    free( pEvent );
}

IME_EXTERN_C IMeEvent*   IMeEventCreate( uint8_t bInitialState , uint8_t bReSet )
{
    IMeCEvent* pEvent = (IMeCEvent*)calloc(1,sizeof(IMeCEvent));
    while( pEvent )
    {
        pEvent->m_hEvent = CreateEvent( NULL , bReSet , bInitialState , NULL );
        if( !pEvent->m_hEvent )
        {
            DebugLogString( TRUE , "[IMeEventCreate] Create event failed!!" );
            free( pEvent );
            pEvent = NULL;
            break;
        }

        ((IMeEvent*)pEvent)->m_pDestroy = IMeCEventDestroy;
        ((IMeEvent*)pEvent)->m_pReset = IMeCEventReset;
        ((IMeEvent*)pEvent)->m_pSet = IMeCEventSet;
        ((IMeEvent*)pEvent)->m_pWait = IMeCEventWait;

        break;
    }

    return (IMeEvent*)pEvent;
}

#elif   PROJECT_FOR_LINUX

typedef struct _IMeCEvent{
    IMeEvent    interfacefunction;
    pthread_cond_t thread_cond;      /* ������ */
    pthread_mutex_t cond_mutex;     /* ��������Դ */
    pthread_mutexattr_t mutex_attr; /* ���������� */
    int m_semaphore;    /* �ź��� */
    uint8_t m_bInit;  /* ��ʼ����ʶ */
}IMeCEvent;


void    IMeCEventSet( IMeEvent* pIEvent )
{
    IMeCEvent* pEvent = (IMeCEvent*)pIEvent;
    
    if( !pEvent || !pEvent->m_bInit )   return;

    if( pthread_mutex_lock( &pEvent->cond_mutex ) )
    {
        DebugLogString( TRUE , "[IMeEventSet] pthread_mutex_lock failed!" );
        return;
    }
    
    pEvent->m_semaphore++;

    /* ��signal���ڻ�����֮�� */
    /* ���ź���������,����һ���̴߳ӵȴ�����������,���ȴ����̵߳Ļ�������Դ,�û�������Դ������������,�����̼߳�������ִ�м����ٽ��� */
    if( pthread_cond_signal( &pEvent->thread_cond ) )   
    {
        DebugLogString( TRUE , "[IMeEventSet] pthread_cond_signal failed!" );
    }

    if( pthread_mutex_unlock( &pEvent->cond_mutex ) )
    {
        DebugLogString( TRUE , "[IMeEventSet] pthread_mutex_unlock failed!" );
        return;
    }
}

void    IMeCEventReset( IMeEvent* pIEvent )
{
    IMeCEvent* pEvent = (IMeCEvent*)pIEvent;
    
    if( !pEvent || !pEvent->m_bInit )   return;
    
    if( pthread_mutex_lock( &pEvent->cond_mutex ) )
    {
        DebugLogString( TRUE , "[IMeEventSet] pthread_mutex_lock failed!" );
        return;
    }
    
    pEvent->m_semaphore = 0;
    
    if( pthread_mutex_unlock( &pEvent->cond_mutex ) )
    {
        DebugLogString( TRUE , "[IMeEventSet] pthread_mutex_unlock failed!" );
        return;
    }
}

uint8_t   IMeCEventWait( IMeEvent* pIEvent , uint32_t dwMilliseconds )
{
    IMeCEvent* pEvent = (IMeCEvent*)pIEvent;
    struct timespec abstime;
    /* get the current time */ 
    struct timeval now; 
    int res = ETIMEDOUT;

    if( !pEvent || !pEvent->m_bInit )   return FALSE;

    gettimeofday(&now, NULL); 
    
    /* add the offset to get timeout value */ 
    abstime.tv_nsec = now.tv_usec * 1000 + (dwMilliseconds % 1000) * 1000000; 
    abstime.tv_sec = now.tv_sec + dwMilliseconds / 1000;

     if( pthread_mutex_lock( &pEvent->cond_mutex ) != 0 )
     {
         DebugLogString( TRUE , "[IMeCEventWait] pthread_mutex_lock failed!!" );
         return FALSE;
     }

    if( pEvent->m_semaphore<=0 )
    {
        res=pthread_cond_timedwait( &pEvent->thread_cond , &pEvent->cond_mutex , &abstime );
        if( res!=ETIMEDOUT && res!=0 )
        {
            DebugLogString( TRUE , "[IMeCEventWait] pthread_cond_timewait failed!" );
            if( !pthread_mutex_unlock( &pEvent->cond_mutex ) )
            {
                DebugLogString( TRUE , "[IMeCEventWait] pthread_mutex_unlock failed!" );
            }
            return FALSE;
        }
    }
    
    if( !res )
        pEvent->m_semaphore--;

    if( pthread_mutex_unlock( &pEvent->cond_mutex ) != 0 )
    {
        DebugLogString( TRUE , "[IMeCEventWait] pthread_mutex_unlock failed!!" );
        return FALSE;
    }

    return (res==0);
}

void    IMeCEventDestroy( IMeEvent* pIEvent )
{
    IMeCEvent* pEvent = (IMeCEvent*)pIEvent;
    
    if( !pEvent )   return;

    pthread_cond_broadcast( &pEvent->thread_cond );

    pthread_cond_destroy( &pEvent->thread_cond );
    pthread_mutexattr_destroy( &pEvent->mutex_attr );
    pthread_mutex_destroy( &pEvent->cond_mutex );

    pEvent->m_bInit = FALSE;
    pEvent->m_semaphore = 0;

    free( pEvent );
}


IME_EXTERN_C IMeEvent*   IMeEventCreate( uint8_t bInitialState , uint8_t bReSet )
{
    IMeCEvent* pEvent = (IMeCEvent*)calloc(1,sizeof(IMeCEvent));
    while( pEvent )
    {
        if( pthread_cond_init( &pEvent->thread_cond , NULL ) !=0 )
        {
            DebugLogString( TRUE , "[IMeEventCreate] pthread_cond_init failed!" );
            free(pEvent);
            pEvent = NULL;
            break;
        }
        if( pthread_mutexattr_init( &pEvent->mutex_attr ) !=0 )
        {
            DebugLogString( TRUE , "[IMeEventCreate] pthread_mutexattr_init failed!" );
            pthread_cond_destroy( &pEvent->thread_cond );
            free(pEvent);
            pEvent = NULL;
            break;
        }
        pthread_mutexattr_settype( &pEvent->mutex_attr , PTHREAD_MUTEX_RECURSIVE_NP );    /* ��ͨ�� */
        
        if( !pthread_mutex_init( &pEvent->cond_mutex , &pEvent->mutex_attr ) )
        {
            DebugLogString( TRUE , "[IMeEventCreate] pthread_mutex_init failed!" );
            pthread_cond_destroy( &pEvent->thread_cond );
            pthread_mutexattr_destroy( &pEvent->mutex_attr );
            free(pEvent);
            pEvent = NULL;
            break;
        }

        pEvent->m_bInit = TRUE;
        pEvent->m_semaphore = 0;

        ((IMeEvent*)pEvent)->m_pDestroy = IMeCEventDestroy;
        ((IMeEvent*)pEvent)->m_pReset = IMeCEventReset;
        ((IMeEvent*)pEvent)->m_pSet = IMeCEventSet;
        ((IMeEvent*)pEvent)->m_pWait = IMeCEventWait;

        break;
    }

    return (IMeEvent*)pEvent;
}

/*
linux event ģ��:
thread[1]:   lock ---> ����ȴ����� ---> unlock 
thread[3]:   lock ---> ����ȴ����� ---> unlock
thread[2]:   lock ---> �����ź� ------> �ź��ȵ���thread[1]
thread[1]:   �����ȴ�����  ---> �ȴ�lock(���ȵõ�����Դ)
thread[2]:   unlock ---> ��������ҵ��
thread[4]:   lock ---> ����ȴ����� ---> unlock
thread[1]:   lock ---> unlock ---> ��������ҵ��
*/

#endif
