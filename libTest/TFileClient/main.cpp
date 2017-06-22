#include    "include.h"
#include	"tfileclient.h"

#ifdef	WIN32

#pragma		comment( lib , "winmm.lib" )

#ifdef  _DEBUG
#pragma		comment( lib , "libSdkD.lib" )
#else
#pragma		comment( lib , "libSdkR.lib" )
#endif

#define		FILECLIENT_UPLOADFILE	"F:\\baidu player\\BaiduPlayerNetSetup_93060008.exe"
//#define		FILECLIENT_UPLOADFILE	"F:\\Media\\十二生肖_BD全集高清中英双字.rmvb"
//#define		FILECLIENT_UPLOADFILE	"F:\\qinxin\\GG\\libTest\\servercourse\\十二生肖_BD全集高清中英双字.rmvb"

#endif

#define		SOCK_MANAGER_GROUP		1

CTFileClient* g_fileClient = NULL;
char g_uploadFile[256];

void	OnRcvMainTFileClientStatus( int nNotifyStatus , void* upApp )
{
	switch( nNotifyStatus )
	{
	case TFileClientNotify_ConnectionBlocked:
        DebugLogString( TRUE , "[OnRcvMainTFileClientStatus] server closed socket!!" );
		break;
	case TFileClientNotify_VerifyFailed:
		break;
	case TFileClientNotify_VerifySuccess:
		{
			DebugLogString( TRUE , "[TFileClientNotify_VerifySuccess] success!!" );
			g_fileClient->StartUploadThead( TRUE );
			g_fileClient->CommitFileInfo( g_uploadFile );
		}
		break;
	}
}


int main(int argc, char* argv[])
{
	int protocolStart;

#ifdef	WIN32

	WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD( 2, 2 );
    err = WSAStartup( wVersionRequested, &wsaData );
    if( err!=0 )    return -1;

#endif

	DebugLogString( TRUE , "[main] usage===>\n client 1[1:udp][2:tcp] /usr/local/filename" );

	protocolStart = TFileProtocol_Udp;
	if( argc >= 2 )
	{
		protocolStart = atoi(argv[1]);
	}

	sprintf( g_uploadFile, "%s", FILECLIENT_UPLOADFILE );
	if( argc >= 3 )
	{
		sprintf( g_uploadFile, "%s", argv[2] );
	}

	g_fileClient = new CTFileClient();
	g_fileClient->SetCallBack( OnRcvMainTFileClientStatus , NULL );
	g_fileClient->Init( protocolStart );

	if( g_fileClient->ConnectServer( "192.168.1.77" , 10000 , AF_INET ) )
	{
		g_fileClient->LoginServer( "TFileClient" , "TFileClient" );
	}

    system( "pause" );
	
	DebugLogString( TRUE , "[main] protocolStart:%d" , protocolStart );

//孙青-PC\孙青
	g_fileClient->StartUploadThead( FALSE );
	g_fileClient->DeInit();

	delete g_fileClient;
	
    return 0;
}