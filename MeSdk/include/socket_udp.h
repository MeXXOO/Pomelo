#ifndef     _ME_SOCKET_UDP_H_
#define     _ME_SOCKET_UDP_H_

#include	"socket_manager_dispatch.h"

typedef struct _IMeSocketUdp    IMeSocketUdp;

//udp client call , server socket use
typedef void    (*OnUdpSocketCallCallBack)( socket_addr_t* pClientUser , void* upApp );


typedef int     (*IMeSocketUdpSend)( IMeSocketUdp* pISocketUdp , char* lpData , int nLen , char* ipStr , ushort port , ushort family );
typedef int		(*IMeSocketUdpSendByAddr)( IMeSocketUdp* pISocketUdp, char* lpData , int nLen , socket_addr_t* addr );
typedef void    (*IMeSocketUdpDestroy)( IMeSocketUdp* pISocketUdp );
typedef uint8	(*IMeSocketUdpBind)( IMeSocketUdp* pISocketUdp );
typedef void    (*IMeSocketUdpSetOpt)( IMeSocketUdp* pISocketUdp, int opt, int optValue );
typedef void	(*IMeSocketUdpEnableAsTcp)( IMeSocketUdp* pISocketUdp, uint8 bEnable );
typedef void	(*IMeSocketUdpSetClientCallBack)( IMeSocketUdp* pISocketUdp, OnUdpSocketCallCallBack cb, void* upApp );
typedef void	(*IMeSocketUdpCallUser)( IMeSocketUdp* pISocketUdp, char* ipStr, ushort port, ushort family );

struct _IMeSocketUdp
{
    IMeSocketDispatchUser  vtbl;
    IMeSocketUdpSend    m_pSend;
	IMeSocketUdpSendByAddr	m_pSendByAddr;
	IMeSocketUdpBind	m_pBind;
    IMeSocketUdpDestroy m_pDestroy;
    IMeSocketUdpSetOpt  m_pSetOpt;
	IMeSocketUdpEnableAsTcp	m_pEnableAsTcp;
	IMeSocketUdpCallUser	m_pCallUser;
	IMeSocketUdpSetClientCallBack	m_pSetClientCallBack;
};

#define		CSocketUdpBind(p)						(p->m_pBind(p))
#define     CSocketUdpSend(p,a,b,c,d,e)				(p->m_pSend(p,a,b,c,d,e))
#define		CSocketUdpSendByAddr(p,a,b,c)			(p->m_pSendByAddr(p,a,b,c))
#define     CSocketUdpDestroy(p)					(p->m_pDestroy(p))
#define     CSocketUdpSetOpt(p,a,b)					(p->m_pSetOpt(p,a,b))
#define		CSocketUdpEnableAsTcp(p,a)				(p->m_pEnableAsTcp(p,a))
#define		CSocketUdpCallUser(p,a,b,c)				(p->m_pCallUser(p,a,b,c))
#define		CSocketUdpSetClientCallBack(p,a,b)		(p->m_pSetClientCallBack(p,a,b))

#endif  //end _ME_SOCKET_UDP_H_

