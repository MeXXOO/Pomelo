#include	"tfileclient.h"
#include	"fileClientProcess.h"

#define		SOCK_MANAGER_GROUP	1

void    OnTcpClientSocketRcvDataCallBack( char* lpData , int nLen , void* rcvSource , void* upApp )
{
	CTFileClient* pTFileClient = (CTFileClient*)upApp;
	if( pTFileClient->m_bRunning )
		OnFileClientProtocolRcvServerData( pTFileClient , lpData , nLen , rcvSource );
}

void	OnUdpClientSocketRcvDataCallBack( char* lpData , int nLen , void* rcvSource , void* upApp )
{
	CTFileClient* pTFileClient = (CTFileClient*)upApp;
	if( pTFileClient->m_bRunning )
		OnFileClientProtocolRcvServerData( pTFileClient , lpData , nLen , rcvSource );
}

CTFileClient::CTFileClient()
{
	m_bRunning = FALSE;
	m_bLoginSuccess = FALSE;
	m_bConnectSuccess = FALSE;

	m_pSocketManager = NULL;
	m_pTcpSocket = NULL;
	m_pUdpSocket = NULL;

	m_statusCB = NULL;
	m_upApp = NULL;

	m_fileIdIndex = 0;

	m_listFileSource = CListCreate();
	m_lockerListFileSource = CLock_Create();

	m_bUploading = FALSE;
	m_thread = NULL;
	m_evWaitAck = CEventCreate(TRUE,FALSE);
}


CTFileClient::~CTFileClient()
{
	ReleaseFileInfoList();

	if( m_listFileSource )
	{
		CListDestroy(m_listFileSource);
		m_listFileSource = NULL;
	}
	
	if( m_lockerListFileSource )
	{
		CLock_Destroy( m_lockerListFileSource );
		m_lockerListFileSource = NULL;
	}

	if( m_evWaitAck )
	{
		CEventDestroy( m_evWaitAck );
		m_evWaitAck = NULL;
	}
}

void CTFileClient::ReleaseFileInfoList()
{
	IMeTFileSource* pFileSource;

	CLock_Lock( m_lockerListFileSource );

	while( (pFileSource = (IMeTFileSource*)CListRemoveHead(m_listFileSource)) )
		IMeTFileSourceDestroy( pFileSource );

	CLock_Unlock( m_lockerListFileSource );
}

void CTFileClient::SetCallBack( OnRcvTFileClientStatus CB , void* upApp )
{
	m_statusCB = CB;
	m_upApp = upApp;
}

uint8_t CTFileClient::Init( int nTransferProtocolType )
{
	DeInit();

	m_pSocketManager = CSocketManagerCreate(SOCK_MANAGER_GROUP);
	
	if( !m_pSocketManager )
		return FALSE;

	m_bRunning = TRUE;

	m_nTransferProtocolType = nTransferProtocolType;

	return TRUE;
}


void CTFileClient::DeInit()
{
	if( m_bRunning )
	{
		//clear work flag
		m_bConnectSuccess = FALSE;
		m_bLoginSuccess = FALSE;
		m_bRunning = FALSE;

		//socket used by udp protocol
		if( m_pUdpSocket )
		{
			CSocketUdpDestroy(m_pUdpSocket);
			m_pTcpSocket = NULL;
		}

		//socket used by tcp protocol
		if( m_pTcpSocket )
		{
			CSocketTcpDestroy(m_pTcpSocket);
			m_pTcpSocket = NULL;
		}

		//socket manager 
		if( m_pSocketManager )
		{
			CSocketManagerDestroy(m_pSocketManager);
			m_pSocketManager = NULL;
		}
	}
}

void CTFileClient::StartUploadThead( uint8_t bStart )
{
	//start upload
	if( bStart && !m_bUploading )
	{
		m_bUploading = TRUE;
		m_thread = CThreadCreate( (void*)TFileClientTheadUploadFile, this );
	}
	//stop upload
	else if( !bStart && m_bUploading )
	{
		CEventSet( m_evWaitAck );
		m_bUploading = FALSE;
		CThreadExit( m_thread , 2000 );
		CThreadDestroy( m_thread );
		m_thread = NULL;
	}
}

void CTFileClient::DisconnectServer()
{
    StartUploadThead(FALSE);
    if( m_pTcpSocket )
    {
        CSocketTcpDestroy(m_pTcpSocket);
        m_pTcpSocket = NULL;
    }
}

uint8_t CTFileClient::ConnectServer( const char* pServerAddress , uint16_t port , int nAFFamily )
{
	if( !m_bRunning )
		return FALSE;

	if( m_bConnectSuccess || m_bLoginSuccess )	
		return m_bConnectSuccess;

	if( m_nTransferProtocolType == TFileProtocol_Tcp )
	{
		m_pTcpSocket = CSocketManagerCreateTcpSocket( m_pSocketManager , NULL , 0 , nAFFamily , 5000 , OnTcpClientSocketRcvDataCallBack , this );
		if( !m_pTcpSocket )	
		{
			DebugLogString( TRUE , "[CTFileClient::ConnectServer] CSocketManagerCreateTcpSocket failed!!" );
			goto ConnectServerFailed;
		}

		if( !CSocketTcpConnect( m_pTcpSocket , (char*)pServerAddress , port ) )
		{
			DebugLogString( TRUE , "[CTFileClient::ConnectServer] CSocketTcpConnect failed!!" );
			goto ConnectServerFailed;
		}
	}
    else if( m_nTransferProtocolType == TFileProtocol_Udp )
	{
		m_pUdpSocket = CSocketManagerCreateUdpSocket( m_pSocketManager , NULL , 0 , nAFFamily , 5000 , OnUdpClientSocketRcvDataCallBack , this );
		if( !m_pUdpSocket )	
		{
			DebugLogString( TRUE , "[CTFileClient::ConnectServer] CSocketManagerCreateUdpSocket failed!!" );
			goto ConnectServerFailed;
		}

		CSocketUdpSetOpt( m_pUdpSocket , SOCKET_SO_RCVBUF , 1024*1024 );
		CSocketUdpSetOpt( m_pUdpSocket , SOCKET_SO_SNDBUF , 1024*1024 );
		CSocketUdpEnableAsTcp( m_pUdpSocket, 1 );
		CSocketUdpCallUser( m_pUdpSocket, (char*)pServerAddress, port, nAFFamily );
    }

	m_bConnectSuccess = TRUE;

	//init variable
	sprintf( m_serverIpStr , "%s" , pServerAddress );
	m_nPort = port;
	m_nAFFamily = nAFFamily;

	return TRUE;

ConnectServerFailed:
	//failed && release
	if( m_pTcpSocket )
	{
		CSocketTcpDestroy(m_pTcpSocket);
		m_pTcpSocket = NULL;
	}
	
	if( m_pUdpSocket )
	{
		CSocketUdpDestroy( m_pUdpSocket );
		m_pUdpSocket = NULL;
	}

	return FALSE;
}

void CTFileClient::LoginServer( const char* pAccount , const char* pPassword )
{
	SendTFileServerLoginUserInfo( this , pAccount , pPassword );
}

uint8_t CTFileClient::CommitFileInfo( const char* pFilePath )
{
	uint64_t llSize;
	int32_t isDir;
	IMeTFileInfo* pTFileInfo;
	char* pFileName;
	char szUplevelFilePath[256];

	IMeTFileSource* pTFileSource;

	IMeArray* arrFileList;

	if( !m_bLoginSuccess || !pFilePath || (-1==IMeFileIsDir(pFilePath)) )	return FALSE;

	arrFileList = CArrayCreate(SORT_INC);

	memset( szUplevelFilePath , 0 , 256 );
	IMeGetUpLevelFilePath( pFilePath , szUplevelFilePath , 1 );

	//direct raw file
	if( !IMeFileIsDir(pFilePath) )
	{
		llSize = IMeGetFileSize(pFilePath);

		pTFileInfo = IMeTFileInfoCreate( pFilePath+strlen(szUplevelFilePath) , llSize , m_fileIdIndex , FALSE );
		if( pTFileInfo )	CArrayAdd( arrFileList , pTFileInfo , m_fileIdIndex );
		++m_fileIdIndex;
	}
	//file directory
	else
	{
		int i;
		IMeArray* arrFile = CArrayCreate(SORT_NULL);
		
		//add top level directory
		CArrayAdd( arrFile, strdup(pFilePath), 0 );
		IMeGetSubDirFileList( pFilePath , arrFile , TRUE , TRUE );

		for( i=0; i<CArrayGetSize(arrFile); i++ )
		{
			pFileName = (char*)CArrayGetAt(arrFile,i);
			isDir = IMeFileIsDir( pFileName );
			llSize = 0;
			if( isDir==0 )
				llSize = IMeGetFileSize(pFileName);
			
			pTFileInfo = IMeTFileInfoCreate( pFileName+strlen(szUplevelFilePath) , llSize , m_fileIdIndex , (isDir==1) );	
			if( pTFileInfo )	CArrayAdd( arrFileList , pTFileInfo , m_fileIdIndex );
			++m_fileIdIndex;

			free(pFileName);
		}
		CArrayDestroy(arrFile);
	}

	//create file source
	pTFileSource = IMeTFileSourceCreate( arrFileList , szUplevelFilePath );
	if( pTFileSource )
	{
		CLock_Lock( m_lockerListFileSource );
		CListAddTail( m_listFileSource , pTFileSource );
		CLock_Unlock( m_lockerListFileSource );
	}
	else
	{
		IMeTFileSourceReleaseFileList( arrFileList );
		CArrayDestroy( arrFileList );
	}
	
	return TRUE;
}

int	CTFileClient::SendLocalData( char* buffer, int len )
{
	if( m_pTcpSocket )
		return CSocketTcpSend( m_pTcpSocket , buffer , len );
	else if( m_pUdpSocket )
		return CSocketUdpSend( m_pUdpSocket , buffer , len , m_serverIpStr , m_nPort , m_nAFFamily );
	else
		return 0;
}
