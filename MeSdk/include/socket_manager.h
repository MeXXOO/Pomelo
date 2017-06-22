#ifndef		_ME_SOCKET_MANAGER_H_
#define		_ME_SOCKET_MANAGER_H_

#include	"../include/include.h"

typedef void    (*OnSocketRcvDataCallBack)( char* lpData , int nLen , void* rcvSource , void* upApp );

typedef	struct _IMeSocketManager	IMeSocketManager;

typedef IMeSocketTcp*	(*IMeSocketManagerCreateTcpServerSocket)( IMeSocketManager* pISocketManager , IMeSocket* pSocket , OnSocketRcvDataCallBack dataCB , void* upApp );
typedef IMeSocketTcp*	(*IMeSocketManagerCreateTcpSocket)( IMeSocketManager* pISocketManager , char* ipstr , ushort port , ushort s_family , int s_timeout , OnSocketRcvDataCallBack dataCB , void* upApp );
typedef IMeSocketUdp*	(*IMeSocketManagerCreateUdpSocket)( IMeSocketManager* pISocketManager , char* ipstr , ushort port , ushort s_family , int s_timeout , OnSocketRcvDataCallBack dataCB , void* upApp );
typedef IMeSocketLsn*	(*IMeSocketManagerCreateLsnSocket)( IMeSocketManager* pISocketManager , char* ipstr , ushort port , ushort s_family , int s_timeout , OnSocketConnectCallBack connectCB , void* upApp );
typedef	void	(*IMeSocketManagerDestroy)( IMeSocketManager* pISocketManager );

struct _IMeSocketManager{
	IMeSocketManagerCreateLsnSocket	m_pCreateLsnSocket;
    IMeSocketManagerCreateTcpServerSocket m_pCreateTcpServerSocket;
	IMeSocketManagerCreateTcpSocket	m_pCreateTcpSocket;
	IMeSocketManagerCreateUdpSocket	m_pCreateUdpSocket;
	IMeSocketManagerDestroy	m_pDestroy;
};

IME_EXTERN_C	IMeSocketManager*	IMeSocketManagerCreate( int nDispatchGroupCnt );

#define     CSocketManagerCreate(a)                         IMeSocketManagerCreate(a)
#define     CSocketManagerCreateLsnSocket(p,a,b,c,d,e,f)    (p->m_pCreateLsnSocket(p,a,b,c,d,e,f))
#define     CSocketManagerCreateTcpServerSocket(p,a,b,c)	(p->m_pCreateTcpServerSocket(p,a,b,c))
#define     CSocketManagerCreateTcpSocket(p,a,b,c,d,e,f)	(p->m_pCreateTcpSocket(p,a,b,c,d,e,f))
#define     CSocketManagerCreateUdpSocket(p,a,b,c,d,e,f)	(p->m_pCreateUdpSocket(p,a,b,c,d,e,f))
#define     CSocketManagerDestroy(p)						(p->m_pDestroy(p))

#endif		//end	_ME_SOCKET_MANAGER_H_
