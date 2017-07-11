#ifndef     _ME_SOCKET_LISTEN_H_
#define     _ME_SOCKET_LISTEN_H_

#include    "memory.h"
#include	"socket.h"
#include	"socket_manager_dispatch.h"

typedef struct _IMeSocketLsn IMeSocketLsn;

typedef void    (*OnSocketConnectCallBack)( IMeSocket* pSocketClient , void* upApp );

typedef void    (*IMeSocketListenDestroy)( IMeSocketLsn* pIListenSocket );
typedef void	(*IMeSocketListenStart)( IMeSocketLsn* pIListenSocket );
typedef void	(*IMeSocketListenStop)( IMeSocketLsn* pIListenSocket );
typedef void    (*IMeSocketListenSetOpt)( IMeSocketLsn* pIListenSocket, int opt, int optValue );

struct _IMeSocketLsn
{
	IMeSocketDispatchUser  vtbl;
    IMeSocketListenDestroy  m_pDestroy;
	IMeSocketListenStart	m_pStart;
	IMeSocketListenStop		m_pStop;
    IMeSocketListenSetOpt   m_pSetOpt;
};

#define     CSocketListenDestroy(p)         (p->m_pDestroy(p))
#define		CSocketListenStart(p)			(p->m_pStart(p))
#define		CSocketListenStop(p)			(p->m_pStop(p))
#define     CSocketListenSetOpt(p,a,b)      (p->m_pSetOpt(p,a,b))

#endif      //END _ME_SOCKET_LISTEN_H_

