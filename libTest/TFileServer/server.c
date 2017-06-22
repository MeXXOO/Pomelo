#include	"server.h"
#include	"fileServerProcess.h"

#define		SERVER_SOCKET_GROUP_CNT		1

int OnFileSourceUserArrayKeyCompare( uint64 nKey1 , uint64 nKey2 , void* parameter )
{
	IMeFileServer* pFileServer = (IMeFileServer*)parameter;

	if( pFileServer->m_nTransferProtocolType == TFileProtocol_Tcp )
	{
		if( nKey1 == nKey2 )	
			return 0;
		else if( nKey1 > nKey2 )	
			return -1;
		else	
			return 1;
	}
	else 
	{
		socket_addr_t* addr1 = (socket_addr_t*)nKey1;
		socket_addr_t* addr2 = (socket_addr_t*)nKey2;

		if( addr1->family == addr2->family )
		{
			if( addr1->family == AF_INET )
				return memcmp( &addr1->addr_ip4 , &addr2->addr_ip4 , sizeof(struct sockaddr_in) );
			else
				return memcmp( &addr1->addr_ip6 , &addr2->addr_ip6 , sizeof(struct sockaddr_in6) );
		}
		else
			return -1;
	}
}

IME_EXTERN_C	void    OnServerFileSocketRcvDataCallBack( char* lpData , int nLen , void* rcvSource , void* upApp )
{
	IMeFileServer* pFileServer = (IMeFileServer*)upApp;

	OnFileServerProtocolRcvClientData( pFileServer , lpData , nLen , rcvSource );
}

IME_EXTERN_C	void    IMeFileServerListenSocketNewConnectCB( IMeSocket* pSocketClient , void* upApp )
{
	IMeFileServer* pFileServer = (IMeFileServer*)upApp;
	
	DebugLogString( TRUE , "[IMeFileServerListenSocketNewConnectCB] new tcp client connected!!" );

	if( pFileServer->m_bRunning )
	{
		IMeSocketTcp* pTcpClient;
		IMeTFileSourceUser* pTFileSourceUser;

		CSocketSetOpt( pSocketClient , SOCKET_SO_NONBLOCK , 1 );

		pTcpClient = CSocketManagerCreateTcpServerSocket( pFileServer->m_pSocketManager , pSocketClient , OnServerFileSocketRcvDataCallBack , pFileServer );
		if( !pTcpClient )
		{
			DebugLogString( TRUE , "[IMeFileServerListenSocketNewConnectCB] create tcp socket failed!!" );
			return;
		}
		
		pTFileSourceUser = IMeTFileSourceUserCreate( pTcpClient , pFileServer->m_nTransferProtocolType );
		if( pTFileSourceUser )
		{
//             CSocketTcpSetOpt( pTcpClient , SOCKET_TCP_NODELAY , 1 );
            CSocketTcpSetOpt( pTcpClient , SOCKET_SO_SNDBUF , 1024*1024 );
            CSocketTcpSetOpt( pTcpClient , SOCKET_SO_RCVBUF , 1024*1024 );

			CLock_Lock( pFileServer->m_lockClient );
			CArrayAdd( pFileServer->m_arrFileSourceClient , pTFileSourceUser , (uint64)pTcpClient );
			CLock_Unlock( pFileServer->m_lockClient );
		}
	}
	else
	{
		CSocketDestroy(pSocketClient);		
	}
}

IME_EXTERN_C	void    IMeFileServerUdpSocketNewCallCB( socket_addr_t* pUserAddr , void* upApp )
{	
	IMeFileServer* pFileServer = (IMeFileServer*)upApp;
	IMeTFileSourceUser* pTFileSourceUser;

	DebugLogString( TRUE , "[IMeFileServerUdpSocketNewCallCB] new udp client connected!!" );
	
	CLock_Lock( pFileServer->m_lockClient );
	pTFileSourceUser = (IMeTFileSourceUser*)CArrayFindData( pFileServer->m_arrFileSourceClient, (uint64)pUserAddr );
	CLock_Unlock( pFileServer->m_lockClient );

	if( !pTFileSourceUser )
	{
		pTFileSourceUser = IMeTFileSourceUserCreate( pUserAddr , pFileServer->m_nTransferProtocolType );
		if( pTFileSourceUser )
		{	
			CLock_Lock( pFileServer->m_lockClient );
			CArrayAdd( pFileServer->m_arrFileSourceClient , pTFileSourceUser , (uint64)pTFileSourceUser->m_pFileSource );
			CLock_Unlock( pFileServer->m_lockClient );
		}
	}
}

IME_EXTERN_C	void	IMeFileServerReleaseSourceClient( IMeFileServer* pFileServer )
{
	IMeTFileSourceUser* pTFileSourceUser;
    int i;

	CLock_Lock( pFileServer->m_lockClient );
    for( i=0; i<CArrayGetSize(pFileServer->m_arrFileSourceClient); i++ )
    {
	    pTFileSourceUser = (IMeTFileSourceUser*)CArrayGetAt(pFileServer->m_arrFileSourceClient,i);
	    {
		    IMeTFileSourceUserDestroy( pTFileSourceUser );
	    }
    }
    CArrayRemoveAll(pFileServer->m_arrFileSourceClient);
	CLock_Unlock( pFileServer->m_lockClient );
}

IME_EXTERN_C	void	IMeFileServerReleaseClientData( IMeFileServer* pFileServer )
{
	IMeNetSrcData* pNetSrcData;

	CLock_Lock( pFileServer->m_lockerClientData );
	
	while( pNetSrcData = CListRemoveHead(pFileServer->m_listClientData) )
	{
		IMeNetSrcDataDestroy( pNetSrcData );
	}

	CLock_Unlock( pFileServer->m_lockerClientData );
}

IME_EXTERN_C	void	IMeFileServerDestroy( IMeFileServer* pFileServer )
{
	if( pFileServer )
	{
		pFileServer->m_bRunning = FALSE;
		CEventSet( pFileServer->m_evClientData );

		if( pFileServer->m_pListenSocket )
			CSocketListenStop( pFileServer->m_pListenSocket );
		if( pFileServer->m_pUdpSocket )
			CSocketUdpDestroy( pFileServer->m_pUdpSocket );
		IMeFileServerReleaseSourceClient( pFileServer );
		
		if( pFileServer->m_pListenSocket )
			CSocketListenDestroy( pFileServer->m_pListenSocket );			
		CSocketManagerDestroy( pFileServer->m_pSocketManager );

		if( pFileServer->m_ThreadClientData )
		{
			CThreadExit( pFileServer->m_ThreadClientData , 2000 );
			CThreadDestroy( pFileServer->m_ThreadClientData );
			pFileServer->m_ThreadClientData = NULL;
		}

		if( pFileServer->m_ThreadCheckClientDataTimeOut )
		{
			CThreadExit( pFileServer->m_ThreadCheckClientDataTimeOut , 2000 );
			CThreadDestroy( pFileServer->m_ThreadCheckClientDataTimeOut );
			pFileServer->m_ThreadCheckClientDataTimeOut = NULL;
		}

		CLock_Destroy( pFileServer->m_lockClient );
		CEventDestroy( pFileServer->m_evClientData );
		
		IMeFileServerReleaseClientData( pFileServer );
		CListDestroy( pFileServer->m_listClientData );
		CLock_Destroy( pFileServer->m_lockerClientData );

		if( pFileServer->m_fileDir )
			free( pFileServer->m_fileDir );

		free(pFileServer);
	}
}

IME_EXTERN_C	IMeFileServer*	IMeFileServerCreate( int nTransferProtocolType, char* ipStr , ushort port , ushort family , char* pFileStoreDir )
{
	IMeFileServer* pFileServer = (IMeFileServer*)calloc(1,sizeof(IMeFileServer));
	
	while( pFileServer )
	{
		pFileServer->m_pSocketManager = CSocketManagerCreate( SERVER_SOCKET_GROUP_CNT );

		if( nTransferProtocolType == TFileProtocol_Tcp )
		{
			pFileServer->m_pListenSocket = CSocketManagerCreateLsnSocket( pFileServer->m_pSocketManager , ipStr , port , family , 5000 , IMeFileServerListenSocketNewConnectCB , pFileServer );
			if( !pFileServer->m_pListenSocket )
			{
				CSocketManagerDestroy( pFileServer->m_pSocketManager );
				free( pFileServer );
				pFileServer = NULL;
				DebugLogString( TRUE , "[IMeFileServerCreate] start local listen socket failed!!" );
				break;
			}
		}
		else if( nTransferProtocolType == TFileProtocol_Udp )
		{
			pFileServer->m_pUdpSocket = CSocketManagerCreateUdpSocket( pFileServer->m_pSocketManager , ipStr , port , family , 5000 , OnServerFileSocketRcvDataCallBack , pFileServer );
			if( !pFileServer->m_pUdpSocket )
			{
				CSocketManagerDestroy( pFileServer->m_pSocketManager );
				free( pFileServer );
				pFileServer = NULL;
				DebugLogString( TRUE , "[IMeFileServerCreate] start local udp socket failed!!" );
				break;
			}

			CSocketUdpSetOpt( pFileServer->m_pUdpSocket , SOCKET_SO_RCVBUF , 1024*1024 );
			CSocketUdpSetClientCallBack( pFileServer->m_pUdpSocket , IMeFileServerUdpSocketNewCallCB , pFileServer );
			CSocketUdpEnableAsTcp( pFileServer->m_pUdpSocket, 1 );
		}
		else
			break;

        pFileServer->m_fileDir = strdup(pFileStoreDir);
        
        pFileServer->m_arrFileSourceClient = CArrayCreate(SORT_INC);
		CArraySetCompare( pFileServer->m_arrFileSourceClient, OnFileSourceUserArrayKeyCompare, pFileServer );
		
		pFileServer->m_lockClient = CLock_Create();

		pFileServer->m_listClientData = CListCreate();
		pFileServer->m_lockerClientData = CLock_Create();
		pFileServer->m_evClientData = CEventCreate(TRUE,FALSE);

		pFileServer->m_nTransferProtocolType = nTransferProtocolType;

		pFileServer->m_bRunning = TRUE;
		pFileServer->m_ThreadClientData = CThreadCreate( ThreadTFileServerProcessData , pFileServer );
		pFileServer->m_ThreadCheckClientDataTimeOut = CThreadCreate( TFileServerCheckUserTransferStatus , pFileServer );

		//auto start
		if( pFileServer->m_nTransferProtocolType == TFileProtocol_Tcp )
			CSocketListenStart( pFileServer->m_pListenSocket );
		else
			CSocketUdpBind( pFileServer->m_pUdpSocket );

        break;
	}

	return pFileServer;
}
