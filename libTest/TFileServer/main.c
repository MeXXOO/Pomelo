#include    "include.h"
#include	"server.h"

//////////////work windows platform//////////////
#ifdef	PROJECT_FOR_WINDOWS

#pragma		comment( lib , "winmm.lib" )

#ifdef  _DEBUG
#pragma		comment( lib , "libSdkD.lib" )
#else
#pragma		comment( lib , "libSdkR.lib" )
#endif

#define		FILE_STORE_DIR	"F:\\qinxin\\GG\\libTest\\servercourse\\"

//////////////work android platform///////////////
#elif		defined(PROJECT_FOR_ANDROID)

#define		FILE_STORE_DIR	"/sdcard"

/////////////work linux platform//////////////////
#else

#define		FILE_STORE_DIR	"/mnt/hgfs/E/servercourse"

#endif


int main( int argc, char* argv[] )
{
	IMeFileServer* tcpServer;
	int protocolStart;
	char szProtocolFileDir[256];
	int8_t bIsDirExist = FALSE;

#ifdef	WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD( 1, 1 );
    err = WSAStartup( wVersionRequested, &wsaData );
    if( err != 0 )    
    {
        DebugLogString( TRUE , "[main] WSAStartup failed!!" );
        return -1;
    }
#endif

	DebugLogString( TRUE , "[main] usage===>\n server 1[1:udp][2:tcp] /usr/local/" );

	protocolStart = TFileProtocol_Udp;
	if( argc >= 2 )
	{
		protocolStart = atoi( argv[1] );
	}

	sprintf( szProtocolFileDir , "%s" , FILE_STORE_DIR );
	if( argc >= 3 )
	{
		sprintf( szProtocolFileDir, "%s", argv[2] );
	}

	DebugLogString( TRUE , "[main] server protocol:%d[1:udp][2:tcp] file save dir:%s!!" , protocolStart , szProtocolFileDir );

	bIsDirExist = (-1 != IMeFileIsDir(szProtocolFileDir));

    if( !bIsDirExist && !IMeCreateDirectory( szProtocolFileDir ) )
    {
        DebugLogString( TRUE , "[main] IMeCreateDirectory failed!!" );
        return -1;
    }
	
	tcpServer = IMeFileServerCreate( protocolStart , "0.0.0.0" , 10000 , AF_INET , szProtocolFileDir );
	if( !tcpServer )	goto END;

#ifdef	PROJECT_FOR_WINDOWS
    system( "pause" );
#else
    while( TRUE ) IMeSleep(1000);
#endif

END:
	IMeFileServerDestroy( tcpServer );
	
    return 0;
}