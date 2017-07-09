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
	char szServer[256];

#ifdef	WIN32

	WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD( 2, 2 );
    err = WSAStartup( wVersionRequested, &wsaData );
    if( err!=0 )    return -1;

#endif

	DebugLogString( TRUE , "[main] usage===>\n client 1[1:udp][2:tcp] 192.168.9.77(server) /usr/local/filename" );

	if( argc != 4 )
	{
		DebugLogString( TRUE , "[main] invalid parameter count!!" );
		return -1;
	}

	protocolStart = TFileProtocol_Udp;
	protocolStart = atoi(argv[1]);

	sprintf( szServer, "%s", argv[2] );	
	sprintf( g_uploadFile, "%s", argv[3] );

	DebugLogString( TRUE , "[main] protocol:%d server:%s uploadFile:%s!!" , protocolStart, szServer, g_uploadFile );

	g_fileClient = new CTFileClient();
	g_fileClient->SetCallBack( OnRcvMainTFileClientStatus , NULL );
	g_fileClient->Init( protocolStart );

	if( g_fileClient->ConnectServer( szServer , 10000 , AF_INET ) )
	{
		g_fileClient->LoginServer( "TFileClient" , "TFileClient" );
	}

#ifdef	PROJECT_FOR_WINDOWS
    system( "pause" );
#else
    while( TRUE )	IMeSleep(1000);
#endif

//孙青-PC\孙青
	g_fileClient->StartUploadThead( FALSE );
	g_fileClient->DeInit();

	delete g_fileClient;
	
    return 0;
}
