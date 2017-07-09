#include    "../include/include.h"

#ifdef  PROJECT_FOR_WINDOWS

typedef struct _IMeCThread
{
    IMeThread   interfacefunction;
    HANDLE m_hThread;
}IMeCThread;

void    IMeCThreadExit( IMeThread* pIThread , uint32_t dwMilliseconds )
{
    IMeCThread* pThread = (IMeCThread*)pIThread;
    uint32_t dwExitCode;
    if( !pThread || !pThread->m_hThread )   return;

    if( WaitForSingleObject( pThread->m_hThread , INFINITE)==false )
    {
        GetExitCodeThread( pThread->m_hThread , &dwExitCode );
        if( 0==TerminateThread( pThread->m_hThread , dwExitCode ) )
        {
            DebugLogString( true , "IMeCThreadExit terminate" );
        }
    }
	CloseHandle( pThread->m_hThread );
	pThread->m_hThread = NULL;
}

void    IMeCThreadDestroy( IMeThread* pIThread )
{
    IMeCThread* pThread = (IMeCThread*)pIThread;
    if( !pThread || !pThread->m_hThread )   return;
    IMeCThreadExit( pIThread , 2000 );
    free( pThread );
}

IME_EXTERN_C IMeThread* IMeThreadCreate( void* func , void* parameter )
{
    IMeCThread* pThread = (IMeCThread*)calloc(1,sizeof(IMeCThread));
    while( pThread )
    {
        pThread->m_hThread = CreateThread( NULL , 0 , func , parameter , 0/*dwCreateFlag */, NULL );
        if( !pThread->m_hThread )
        {
            DebugLogString( true , "[IMeThreadCreate] CreateThread failed!!" );
            free( pThread );
            pThread = NULL;
            break;
        }
        
        ((IMeThread*)pThread)->m_pExit = IMeCThreadExit;
        ((IMeThread*)pThread)->m_pDestroy = IMeCThreadDestroy;

        break;
    }
    return (IMeThread*)pThread;
}


#elif   PROJECT_FOR_LINUX

typedef struct _IMeCThread
{
    IMeThread   interfacefunction;
    pthread_t   m_hThread;
}IMeCThread;

void    IMeCThreadExit( IMeThread* pIThread , uint32_t dwMilliseconds )
{
    IMeCThread* pThread = (IMeCThread*)pIThread;
    //uint32_t dwExitCode;
    if( !pThread || !pThread->m_hThread )   return;
    pthread_join( pThread->m_hThread , NULL );
}

void    IMeCThreadDestroy( IMeThread* pIThread )
{
    IMeCThread* pThread = (IMeCThread*)pIThread;
    if( !pThread || !pThread->m_hThread )   return;
    IMeCThreadExit( pIThread , 2000 );
    close(pThread->m_hThread);
    free( pThread );
}

IME_EXTERN_C IMeThread* IMeThreadCreate( void* func , void* parameter )
{
    IMeCThread* pThread = (IMeCThread*)calloc(1,sizeof(IMeCThread));
    while( pThread )
    {
        if( pthread_create( &pThread->m_hThread , NULL , (void* (*)(void *))func , parameter ) )
        {
            DebugLogString( true , "[IMeThreadCreate] pthread_create failed!!" );
            free( pThread );
            pThread = NULL;
            break;
        }
        
        ((IMeThread*)pThread)->m_pExit = IMeCThreadExit;
        ((IMeThread*)pThread)->m_pDestroy = IMeCThreadDestroy;
        
        break;
    }
    return (IMeThread*)pThread;
}

#endif
