#ifndef		_I_ME_FILE_SERVER_H_
#define		_I_ME_FILE_SERVER_H_

#include	"include.h"

//major-minor-reversion-build(Ö÷.×Ó.ÐÞÕý.±àÒë)
#define		TFILESERVER_VERSION		"2016-11-26-1.1.0.0"

#define		TFileProtocol_Udp					1
#define		TFileProtocol_Tcp					2

typedef	struct _IMeFileServer
{
	uint8_t	m_bRunning;

	int		m_nTransferProtocolType;

	IMeSocketLsn*	m_pListenSocket;
	IMeSocketUdp*	m_pUdpSocket;
	IMeSocketManager*	m_pSocketManager;

	/* file transform running client count */
	IMeArray*	m_arrFileSourceClient;
	/* lock client user */
	IMeLock*	m_lockClient;

	/* data array from client */
	IMeThread*	m_ThreadClientData;
	IMeEvent*	m_evClientData;
	IMeList*	m_listClientData;
	IMeLock*	m_lockerClientData;

	/* rcv data time out check */
	IMeThread*	m_ThreadCheckClientDataTimeOut;

	/* store file directory */
	char*	m_fileDir;
	
}IMeFileServer;

IME_EXTERN_C	void	IMeFileServerDestroy( IMeFileServer* pFileServer );
IME_EXTERN_C	IMeFileServer*	IMeFileServerCreate( int nTransferProtocolType, char* ipStr , uint16_t port , uint16_t family , char* pFileStoreDir );
IME_EXTERN_C	int		IMeFileServerSendData( IMeFileServer* pFileServer, void* pFileSource, uint8_t* buffer, int len );

#endif		//end _I_ME_FILE_SERVER_H_
