#include    "../include/include.h"

typedef struct _IMeCSocketListen
{
    IMeSocketLsn   vtbl;
    
    IMeSocket*  m_pSocket;

	IMeSocketDispatchGroup*	m_pSocketDispatchGroup;

    OnSocketConnectCallBack m_pNewConnectCB;
    void*   m_upApp;

    uint8_t   m_bRunning;
    IMeEvent*   m_evWaitFree;

    int m_nWaitCount;

	uint32_t	m_dispatchUser_extendParameter;

}IMeCSocketListen;

IME_EXTERN_C uint8_t      IMeCListenSocketListen( IMeCSocketListen* pListenSocket )
{
    if( !CSocketBind( pListenSocket->m_pSocket ) )
    {
        return FALSE;
    }
    
    if( !CSocketListen( pListenSocket->m_pSocket , pListenSocket->m_nWaitCount ) )
    {
        return FALSE;
    }

    pListenSocket->m_bRunning = TRUE;

    return TRUE;
}

IME_EXTERN_C uint8_t    IMeCListenSocketCreateNew( IMeCSocketListen* pListenSocket , uint16_t family , int socket_des , socket_addr_t* pAddr )
{
    IMeSocket* pSocketClient = NULL;

    pSocketClient = CSocketCreateAcceptSocket( socket_des , family , pAddr );
    if( !pSocketClient )
    {
        DebugLogString( TRUE , "[IMeCListenSocketCreateNew] create accept client tcp failed!!" );
        return FALSE;
    }

	/* �ص����׽��� */
    pListenSocket->m_pNewConnectCB( pSocketClient , pListenSocket->m_upApp );

    return TRUE;
}

IME_EXTERN_C void	IMeListenSocketDispatchUserRcv( IMeSocketDispatchUser* pSocketDispatchUser )
{
    socket_addr_t addr_remote;
    int socket_des;

	IMeCSocketListen* pSocketListen = (IMeCSocketListen*)pSocketDispatchUser;

	if( !pSocketListen->m_bRunning )	return;

	/* �������� */
	memset( &addr_remote , 0 , sizeof(socket_addr_t) );
	if( CSocketAccept( pSocketListen->m_pSocket , &addr_remote , &socket_des ) != TRUE )
		return;

	/* ����tcp�׽��ֲ����� */
  if( !IMeCListenSocketCreateNew( pSocketListen , CSocketGetAddrType(pSocketListen->m_pSocket,FALSE) , socket_des , &addr_remote ) ){
#ifdef PROJECT_FOR_IOS
    close(socket_des);
#else
    closesocket(socket_des);
#endif
  }
}

IME_EXTERN_C void	IMeListenSocketDispatchUserSend( IMeSocketDispatchUser* pSocketDispatchUser )
{
    IMeCSocketListen* pSocketListen = (IMeCSocketListen*)pSocketDispatchUser;
    if( !pSocketListen->m_bRunning )    return;
}

IME_EXTERN_C void	IMeListenSocketDispatchUserException( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCSocketListen* pSocketListen = (IMeCSocketListen*)pSocketDispatchUser;
	DebugLogString( TRUE , "[IMeListenSocketDispatchUserException] socket exception event occur %0x!!" , (uint32_t)pSocketListen );
}

IME_EXTERN_C void	IMeListenSocketDispatchUserRemove( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCSocketListen* pSocketListen = (IMeCSocketListen*)pSocketDispatchUser;

    if( pSocketListen->m_evWaitFree )
        CEventSet( pSocketListen->m_evWaitFree );

	DebugLogString( TRUE , "[IMeListenSocketDispatchUserRemove] socket remove from dispatch group!!" );
}

IME_EXTERN_C void	IMeListenSocketDispatchUserSetExtendParameter( IMeSocketDispatchUser* pSocketDispatchUser , uint32_t extendParameter )
{
	IMeCSocketListen* pSocketListen = (IMeCSocketListen*)pSocketDispatchUser;
	pSocketListen->m_dispatchUser_extendParameter = extendParameter;
}

IME_EXTERN_C uint32_t	IMeListenSocketDispatchUserGetExtendParameter( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCSocketListen* pSocketListen = (IMeCSocketListen*)pSocketDispatchUser;
	return pSocketListen->m_dispatchUser_extendParameter;
}

IME_EXTERN_C IMeSocket*	IMeListenSocketDispatchUserGetSocket( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCSocketListen* pSocketListen = (IMeCSocketListen*)pSocketDispatchUser;
	return pSocketListen->m_pSocket;
}

IME_EXTERN_C void	 IMeCListenSocketStop( IMeSocketLsn* pIListenSocket )
{
	IMeCSocketListen* pSocketListen = (IMeCSocketListen*)pIListenSocket;
	CSocketDispatchGroupRemoveUser( pSocketListen->m_pSocketDispatchGroup , (IMeSocketDispatchUser*)pSocketListen );
}

IME_EXTERN_C void    IMeCSocketListenSetOpt( IMeSocketLsn* pIListenSocket, int opt, int optValue )
{
    IMeCSocketListen* pSocketListen = (IMeCSocketListen*)pIListenSocket;
    if( pSocketListen->m_pSocket && opt != SOCKET_SO_NONBLOCK )
        CSocketSetOpt( pSocketListen->m_pSocket, opt, optValue );
}

IME_EXTERN_C void	 IMeCListenSocketStart( IMeSocketLsn* pIListenSocket )
{
	IMeCSocketListen* pSocketListen = (IMeCSocketListen*)pIListenSocket;
	CSocketDispatchGroupAddUser( pSocketListen->m_pSocketDispatchGroup , (IMeSocketDispatchUser*)pSocketListen );
	CSocketDispatchGroupAddEvent( pSocketListen->m_pSocketDispatchGroup , (IMeSocketDispatchUser*)pSocketListen , SEvent_Read );
}

IME_EXTERN_C void    IMeCListenSocketDestroy( IMeSocketLsn* pIListenSocket )
{
    IMeCSocketListen* pSocketListen = (IMeCSocketListen*)pIListenSocket;

    pSocketListen->m_evWaitFree = CEventCreate(TRUE,FALSE);
    CEventReset( pSocketListen->m_evWaitFree );

    IMeCListenSocketStop( pIListenSocket );
	pSocketListen->m_bRunning = FALSE;

    while( !CEventWait(pSocketListen->m_evWaitFree,10) );
    CEventDestroy( pSocketListen->m_evWaitFree );

    free(pSocketListen);
}

IME_EXTERN_C IMeSocketLsn*    IMeSocketLsnCreate( IMeSocketDispatchGroup* pSocketDispatchGroup , IMeSocket* pSocket, OnSocketConnectCallBack pSocketCallBack, void* upApp )
{
    IMeCSocketListen* pSocketListen = (IMeCSocketListen*)calloc(1,sizeof(IMeCSocketListen));
    while( pSocketListen )
    {
        pSocketListen->m_pSocket = pSocket;
		pSocketListen->m_pSocketDispatchGroup = pSocketDispatchGroup;

		//set listen socket nonblock option && listen client connect
		CSocketSetOpt( pSocketListen->m_pSocket , SOCKET_SO_TIMEOUT , 10 );

		if( !IMeCListenSocketListen( pSocketListen ) )
		{
			free(pSocketListen);
			pSocketListen = NULL;
			break;
		}

        pSocketListen->m_pNewConnectCB = pSocketCallBack;
        pSocketListen->m_upApp = upApp;
        pSocketListen->m_nWaitCount = 30;

        pSocketListen->vtbl.m_pDestroy = IMeCListenSocketDestroy;
		pSocketListen->vtbl.m_pStart = IMeCListenSocketStart;
		pSocketListen->vtbl.m_pStop = IMeCListenSocketStop;
        pSocketListen->vtbl.m_pSetOpt = IMeCSocketListenSetOpt;

		pSocketListen->vtbl.vtbl.m_pSetExtendParameter = IMeListenSocketDispatchUserSetExtendParameter;
		pSocketListen->vtbl.vtbl.m_pGetExtendParameter = IMeListenSocketDispatchUserGetExtendParameter;
		pSocketListen->vtbl.vtbl.m_pGetSocket = IMeListenSocketDispatchUserGetSocket;
		pSocketListen->vtbl.vtbl.m_pSend = IMeListenSocketDispatchUserSend;
		pSocketListen->vtbl.vtbl.m_pRcv = IMeListenSocketDispatchUserRcv;
		pSocketListen->vtbl.vtbl.m_pException = IMeListenSocketDispatchUserException;
		pSocketListen->vtbl.vtbl.m_pRemove = IMeListenSocketDispatchUserRemove;

		pSocketListen->m_bRunning = TRUE;
		
        break;
    }

    return (IMeSocketLsn*)pSocketListen;
}
