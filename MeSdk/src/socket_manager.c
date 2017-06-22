#include    "../include/include.h"

IME_EXTERN_C IMeSocketLsn*	IMeSocketLsnCreate( IMeSocketDispatchGroup* pSocketDispatchGroup , IMeSocket* pSocket , OnSocketConnectCallBack pSocketCallBack, void* upApp );
IME_EXTERN_C IMeSocketTcp*	IMeSocketTcpCreate( IMeSocketDispatchGroup* pSocketDispatchGroup , IMeSocket* pSocket , OnSocketRcvDataCallBack pSocketCallBack , void* upApp );
IME_EXTERN_C IMeSocketUdp*	IMeSocketUdpCreate( IMeSocketDispatchGroup* pSocketDispatchGroup , IMeSocket* pSocket , OnSocketRcvDataCallBack pSocketCallBack , void* upApp );


typedef	struct _IMeCSocketManager{

	IMeSocketManager	vtbl;
	IMeList*	m_pListManagerDispatchGroup;

}IMeCSocketManager;


IME_EXTERN_C	IMeSocketDispatchGroup*	IMeCSocketManagerGetDispatchGroup( IMeCSocketManager* pSocketManager )
{
	int nUserCnt = -1;
	IMeSocketDispatchGroup* pSocketDispatchGroup = NULL;

	lposition listPos;
	IMeSocketDispatchGroup* pFindDispatchGroup = (IMeSocketDispatchGroup*)CListGetHead(pSocketManager->m_pListManagerDispatchGroup,&listPos);
	//add to minimum user dispatch group
	while( pFindDispatchGroup )
	{
		if( nUserCnt==-1 || nUserCnt>CSocketDispatchGroupGetUserCnt(pFindDispatchGroup) )
		{
			nUserCnt = CSocketDispatchGroupGetUserCnt(pFindDispatchGroup);
			pSocketDispatchGroup = pFindDispatchGroup;
		}

		pFindDispatchGroup = (IMeSocketDispatchGroup*)CListGetNext(pSocketManager->m_pListManagerDispatchGroup,&listPos);
	}

	return pSocketDispatchGroup;
}

IME_EXTERN_C	IMeSocketLsn*	IMeCSocketManagerCreateLsnSocket( IMeSocketManager* pISocketManager , char* ipstr , ushort port , ushort s_family , int s_timeout , OnSocketConnectCallBack connectCB , void* upApp )
{
	IMeCSocketManager* pSocketManager = (IMeCSocketManager*)pISocketManager;
	IMeSocketLsn* pSocketListen;

	IMeSocketDispatchGroup* pSocketDispatchGroup = IMeCSocketManagerGetDispatchGroup( pSocketManager );
    
    IMeSocket* pSocket = CSocketCreate( ipstr , port , s_family, s_timeout , SOCKET_TCP );
    //none-blocked socket
	CSocketSetOpt( pSocket , SOCKET_SO_NONBLOCK , 1 );
	
    pSocketListen = IMeSocketLsnCreate( pSocketDispatchGroup , pSocket , connectCB , upApp );
	if( !pSocketListen )
	{
		DebugLogString( TRUE , "[IMeCSocketManagerCreateLsnSocket] create listen socket failed!!" );
		return pSocketListen;
	}
	
	CSocketSetExtendParameter( pSocket , (uint)pSocketListen );
	
	return pSocketListen;
}

IME_EXTERN_C    IMeSocketTcp*	IMeCSocketManagerCreateTcpServerSocket( IMeSocketManager* pISocketManager , IMeSocket* pSocket , OnSocketRcvDataCallBack dataCB , void* upApp )
{
    IMeCSocketManager* pSocketManager = (IMeCSocketManager*)pISocketManager;
    
	IMeSocketDispatchGroup* pSocketDispatchGroup = IMeCSocketManagerGetDispatchGroup( pSocketManager );
    
    IMeSocketTcp* pSocketTcp = IMeSocketTcpCreate( pSocketDispatchGroup , pSocket , dataCB , upApp );
    if( !pSocketTcp )
    {
        DebugLogString( TRUE , "[IMeCSocketManagerCreateTcpServerSocket] create tcp socket failed!!" );
        return pSocketTcp;
    }
    
    //none-blocked socket
	CSocketSetOpt( pSocket , SOCKET_SO_NONBLOCK , 1 );

    CSocketSetExtendParameter( pSocket , (uint)pSocketTcp );
    
    CSocketDispatchGroupAddUser( pSocketDispatchGroup , (IMeSocketDispatchUser*)pSocketTcp );
    CSocketDispatchGroupAddEvent( pSocketDispatchGroup , (IMeSocketDispatchUser*)pSocketTcp , SEvent_Read );
    
	return pSocketTcp;
}

IME_EXTERN_C	IMeSocketTcp*	IMeCSocketManagerCreateTcpSocket( IMeSocketManager* pISocketManager , char* ipstr , ushort port , ushort s_family , int s_timeout , OnSocketRcvDataCallBack dataCB , void* upApp )
{
    IMeSocket* pSocket = CSocketCreate( ipstr , port , s_family, s_timeout , SOCKET_TCP );

    return IMeCSocketManagerCreateTcpServerSocket( pISocketManager, pSocket, dataCB, upApp );
}

IME_EXTERN_C	IMeSocketUdp*	IMeCSocketManagerCreateUdpSocket( IMeSocketManager* pISocketManager , char* ipstr , ushort port , ushort s_family , int s_timeout , OnSocketRcvDataCallBack dataCB , void* upApp )
{
	IMeCSocketManager* pSocketManager = (IMeCSocketManager*)pISocketManager;
    IMeSocketUdp* pSocketUdp;

	IMeSocketDispatchGroup* pSocketDispatchGroup = IMeCSocketManagerGetDispatchGroup( pSocketManager );
    
    IMeSocket* pSocket = CSocketCreate( ipstr , port , s_family, s_timeout , SOCKET_UDP );
    //none-blocked socket
	CSocketSetOpt( pSocket , SOCKET_SO_NONBLOCK , 1 );

	pSocketUdp = IMeSocketUdpCreate( pSocketDispatchGroup , pSocket , dataCB , upApp );
	if( !pSocketUdp )
	{
		DebugLogString( TRUE , "[IMeCSocketManagerCreateUdpSocket] create udp socket failed!!" );
		return pSocketUdp;
	}

	CSocketSetExtendParameter( pSocket , (uint)pSocketUdp );

	CSocketDispatchGroupAddUser( pSocketDispatchGroup , (IMeSocketDispatchUser*)pSocketUdp );
	CSocketDispatchGroupAddEvent( pSocketDispatchGroup , (IMeSocketDispatchUser*)pSocketUdp , SEvent_Read );

	return pSocketUdp;
}

IME_EXTERN_C	void	IMeCSocketManagerDestroy( IMeSocketManager* pISocketManager )
{
	IMeCSocketManager* pSocketManager = (IMeCSocketManager*)pISocketManager;
	IMeSocketDispatchGroup* pSocketDispatchGroup = NULL;

	while( pSocketDispatchGroup=CListRemoveHead(pSocketManager->m_pListManagerDispatchGroup) )
	{
		CSocketDispatchGroupDestroy( pSocketDispatchGroup );
	}
	CListDestroy( pSocketManager->m_pListManagerDispatchGroup );

	free( pSocketManager );
}


IME_EXTERN_C	IMeSocketManager*	IMeSocketManagerCreate( int nDispatchGroupCnt )
{
	int i;
	IMeCSocketManager* pSocketManager = (IMeCSocketManager*)calloc(1,sizeof(IMeCSocketManager));
	
	if( nDispatchGroupCnt<1 )
		nDispatchGroupCnt = 1;

	if( pSocketManager )
	{
		pSocketManager->m_pListManagerDispatchGroup = CListCreate();
		for( i=0; i<nDispatchGroupCnt; i++ )
		{
			IMeSocketDispatchGroup* pDispatchGroup = CSocketDispatchGroupCreate();
			CListAddHead( pSocketManager->m_pListManagerDispatchGroup , pDispatchGroup );
		}

		pSocketManager->vtbl.m_pCreateLsnSocket = IMeCSocketManagerCreateLsnSocket;
		pSocketManager->vtbl.m_pCreateTcpSocket = IMeCSocketManagerCreateTcpSocket;
        pSocketManager->vtbl.m_pCreateTcpServerSocket = IMeCSocketManagerCreateTcpServerSocket;
		pSocketManager->vtbl.m_pCreateUdpSocket = IMeCSocketManagerCreateUdpSocket;
		pSocketManager->vtbl.m_pDestroy = IMeCSocketManagerDestroy;
	}
	
	return (IMeSocketManager*)pSocketManager;
}