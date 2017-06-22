#ifndef		_TFILE_CLIENT_
#define		_TFILE_CLIENT_

#include	"fileCommon.h"

/* 与服务器失去连接 */
#define		TFileClientNotify_ConnectionBlocked	1
/* 帐号密码验证失败 */		
#define		TFileClientNotify_VerifyFailed		2
/* 帐号密码验证成功 */
#define		TFileClientNotify_VerifySuccess		3

#define		TFileProtocol_Udp					1
#define		TFileProtocol_Tcp					2

typedef		void	(*OnRcvTFileClientStatus)( int nNotifyStatus , void* upApp );

class CTFileClient
{
public:
	CTFileClient();
	~CTFileClient();

public:
	bool	m_bRunning;
	bool	m_bConnectSuccess;
	bool	m_bLoginSuccess;

public:
	OnRcvTFileClientStatus	m_statusCB;
	void*	m_upApp;

public:
	IMeSocketManager* m_pSocketManager;
	IMeSocketTcp* m_pTcpSocket;
	IMeSocketUdp* m_pUdpSocket;
	int		m_nTransferProtocolType;

	char	m_serverIpStr[64];
	int		m_nPort;
	int		m_nAFFamily;

public:
	char* m_fileMainDir;
	uint	m_fileIdIndex;
	IMeList*	m_listFileSource;
	IMeLock*	m_lockerListFileSource;

public:
	bool	m_bUploading;
	IMeThread*	m_thread;

public:
	void	ReleaseFileInfoList();

public:
	bool	Init( int nTransferProtocolType );
	void	DeInit();
	void	SetCallBack( OnRcvTFileClientStatus CB , void* upApp );

public:
	void	StartUploadThead( boolean bStart );
    void    DisconnectServer();
	bool	ConnectServer( const char* pServerAddress , ushort port , int nAFFamily );
	void	LoginServer( const char* pAccount , const char* pPassword );
	bool	CommitFileInfo( const char* pFilePath );

public:
	int		SendLocalData( char* buffer, int len );
};

#endif	//end _TFILE_CLIENT_
