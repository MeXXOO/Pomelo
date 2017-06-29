#ifndef     _ME_SOCKET_TCP_H_
#define     _ME_SOCKET_TCP_H_

typedef struct _IMeSocketTcp    IMeSocketTcp;

typedef uint8_t (*IMeSocketTcpConnect)( IMeSocketTcp* pISocketTcp  , char* serverAddr , uint16_t serverPort );
typedef int     (*IMeSocketTcpSend)( IMeSocketTcp* pISocketTcp , char* lpData , int nLen );
typedef void    (*IMeSocketTcpDestroy)( IMeSocketTcp* pISocketTcp );
typedef void    (*IMeSocketTcpSetOpt)( IMeSocketTcp* pISocketTcp, int opt, int optValue );

struct _IMeSocketTcp
{	
	IMeSocketDispatchUser	vtbl;
    IMeSocketTcpConnect m_pConnect;
    IMeSocketTcpSend    m_pSend;
    IMeSocketTcpSetOpt  m_pSetOpt;
    IMeSocketTcpDestroy m_pDestroy;
};

#define     CSocketTcpConnect(p,a,b)		(p->m_pConnect(p,a,b))
#define     CSocketTcpSetOpt(p,a,b)         (p->m_pSetOpt(p,a,b))
#define     CSocketTcpSend(p,a,b)			(p->m_pSend(p,a,b))
#define     CSocketTcpDestroy(p)            (p->m_pDestroy(p))

#endif      //end _ME_TCP_SOCKET_H_

