#ifndef         _ME_SOCKET_H_
#define         _ME_SOCKET_H_

#include        "platdefine.h"

/* socket attribute set/get */
#define         SOCKET_SO_LINGER        1    /**what to do when data not all send out closesocket opt occur*/
#define         SOCKET_SO_KEEPALIVE     2    /**< Keepalive */
#define         SOCKET_SO_DEBUG         4    /**< Debug */
#define         SOCKET_SO_NONBLOCK      8    /**< Non-blocking IO */
#define         SOCKET_SO_REUSEADDR     16   /**< Reuse addresses */
#define         SOCKET_SO_TIMEOUT       32   /**< Timeout */
#define         SOCKET_SO_SNDBUF        64   /**< Send buffer */
#define         SOCKET_SO_RCVBUF        128  /**< Receive buffer */
#define         SOCKET_SO_DISCONNECTED  256  /**< Disconnected */
#define         SOCKET_TCP_NODELAY      512  /**< For SCTP sockets, this is mapped*/


/* mtu define */
#define     ETHERNET_TCP_MTU_SIZE    1460
#define     ETHERNET_UDP_MTU_SIZE    1472

/* socket type */
#define         SOCKET_UDP      1
#define         SOCKET_TCP      2

typedef struct _IMeSocket   IMeSocket;

IME_EXTERN_C char*	IMeSocketConvertErrorCodeToString(uint32_t ErrorCode);
IME_EXTERN_C int	IMeSocketGetNetError();	

IME_EXTERN_C IMeSocket*     IMeSocketCreate( char* ipstr , uint16_t port , uint16_t s_family , int s_timeout , int s_type );
IME_EXTERN_C IMeSocket*     IMeSocketCreateAcceptSocket( int socket_des , uint16_t family , void* remote_addr );

typedef void          (*IMeSocketDestroy)( IMeSocket* pISocket );
typedef void          (*IMeSocketClose)( IMeSocket* pISocket );

typedef uint16_t        (*IMeSocketGetAddrType)(IMeSocket* pISocket , uint8_t bRemote );
typedef uint8_t         (*IMeSocketGetAddrStr)(IMeSocket* pISocket , uint8_t bRemote , char* strbuf , int nLen/* ipv4-16B | ipv6-40B */ );
typedef uint8_t         (*IMeSocketGetAddrNum)(IMeSocket* pISocket , uint8_t bRemote , char* bytebufer , int nLen/* ipv4-4B | ipv6-16B */ );
typedef uint16_t        (*IMeSocketGetPort)(IMeSocket* pISocket , uint8_t bRemote );

typedef uint8_t         (*IMeSocketConnect)( IMeSocket* pISocket , char* ipstr , uint16_t port );
typedef uint8_t         (*IMeSocketBind)( IMeSocket* pISocket );
typedef uint8_t         (*IMeSocketListen)( IMeSocket* pISocket , int backlog );
typedef uint8_t         (*IMeSocketAccept)( IMeSocket* pISocket , socket_addr_t* pRemoteAddr , int* socket_des );

typedef POMInteger           (*IMeSocketSend)( IMeSocket* pISocket , char* data , int nLen , int flags );
typedef POMInteger           (*IMeSocketSendToByAddr)( IMeSocket* pISocket , socket_addr_t* pSendAddr , char* data , int nLen , int flags );
typedef POMInteger           (*IMeSocketSendTo)( IMeSocket* pISocket , char* szIP , uint16_t nPort , uint16_t family , char* data , int nLen , int flags );
typedef POMInteger           (*IMeSocketRecv)( IMeSocket* pISocket , char* buf , int nLen , int flags );
typedef POMInteger           (*IMeSocketRecvFrom)( IMeSocket* pISocket , char* buf , int nLen , int flags );
typedef POMInteger           (*IMeSocketRecvFromByAddr)( IMeSocket* pISocket , char* buf , int nLen , socket_addr_t* pRcvAddr , int flags );

typedef uint8_t         (*IMeSockeSetTimeOut)(IMeSocket *socket, int new_t);
typedef int           (*IMeSocketGetOpt)( IMeSocket* pISocket , int opt );
typedef uint8_t         (*IMeSocketSetOpt)( IMeSocket* pISocket , int opt , int on);
typedef uint8_t         (*IMeSocketGetReadSize)( IMeSocket* pISocket , uint32_t* pBufferLen );

typedef int       (*IMeSocketGetFd)( IMeSocket* pISocket );
typedef int           (*IMeSocketGetType)( IMeSocket* pISocket );

typedef	void		  (*IMeSocketSetEventParameter)( IMeSocket* pISocket , uint32_t parameter  );
typedef long		  (*IMeSocketGetEventParameter)( IMeSocket* pISocket );

typedef void		  (*IMeSocketSetExtendParameter)( IMeSocket* pISocket , uint32_t parameter );
typedef long		  (*IMeSocketGetExtendParameter)( IMeSocket* pISocket );

struct _IMeSocket{
    IMeSocketDestroy    m_pDestroy;
    IMeSocketClose  m_pClose;

    IMeSocketGetAddrType    m_pGetAddrType;
    IMeSocketGetAddrStr   m_pGetAddrStr;
    IMeSocketGetAddrNum   m_pGetAddrNum;
    IMeSocketGetPort  m_pGetPort;
    
    IMeSocketConnect    m_pConnect;
    IMeSocketBind   m_pBind;
    IMeSocketListen m_pListen;
    IMeSocketAccept m_pAccept;
    
    IMeSocketSend   m_pSend;
    IMeSocketSendTo m_pSendTo;
    IMeSocketSendToByAddr   m_pSendToByAddr;
    IMeSocketRecv   m_pRecv;
    IMeSocketRecvFrom   m_pRecvFrom;
    IMeSocketRecvFromByAddr m_pRcvFromByAddr;
    
    IMeSockeSetTimeOut  m_pSetTimeOut;
    IMeSocketGetOpt m_pGetOpt;
    IMeSocketSetOpt m_pSetOpt;
    IMeSocketGetReadSize    m_pGetReadSize;

    IMeSocketGetFd  m_pGetFd;
    IMeSocketGetType    m_pGetType;

	IMeSocketSetEventParameter	m_pSetEventParameter;
	IMeSocketGetEventParameter	m_pGetEventParameter;

	IMeSocketSetExtendParameter	m_pSetExtendParameter;
	IMeSocketGetExtendParameter m_pGetExtendParameter;
};

#define             CSocketCreate(a,b,c,d,e)				IMeSocketCreate(a,b,c,d,e)
#define             CSocketCreateAcceptSocket(a,b,c)		IMeSocketCreateAcceptSocket(a,b,c)

#define             CSocketDestroy(p)                       (p->m_pDestroy(p))
#define             CSocketClose(p)                         (p->m_pClose(p))

#define             CSocketGetAddrType(p,a)                 (p->m_pGetAddrType(p,a))
#define             CSocketGetAddrStr(p,a,b,c)              (p->m_pGetAddrStr(p,a,b,c))
#define             CSocketGetAddrNum(p,a,b,c)              (p->m_pGetAddrNum(p,a,b,c))
#define             CSocketGetPort(p,a)                     (p->m_pGetPort(p,a))

#define             CSocketConnect(p,a,b)                   (p->m_pConnect(p,a,b))
#define             CSocketBind(p)                          (p->m_pBind(p))
#define             CSocketListen(p,a)                      (p->m_pListen(p,a))
#define             CSocketAccept(p,a,b)                    (p->m_pAccept(p,a,b))

#define             CSocketSend(p,a,b,c)                    (p->m_pSend(p,a,b,c))
#define             CSocketSendTo(p,a,b,c,d,e,f)            (p->m_pSendTo(p,a,b,c,d,e,f))
#define             CSocketSendToByAddr(p,a,b,c,d)          (p->m_pSendToByAddr(p,a,b,c,d))
#define             CSocketRecv(p,a,b,c)                    (p->m_pRecv(p,a,b,c))
#define             CSocketRecvFrom(p,a,b,c)                (p->m_pRecvFrom(p,a,b,c))
#define             CSocketRecvFromByAddr(p,a,b,c,d)        (p->m_pRcvFromByAddr(p,a,b,c,d))

#define             CSocketSetTimeOut(p,a)                  (p->m_pSetTimeOut(p,a))
#define             CSocketGetOpt(p,a)                      (p->m_pGetOpt(p,a))
#define             CSocketSetOpt(p,a,b)                    (p->m_pSetOpt(p,a,b))
#define             CSocketGetReadSize(p,a)                 (p->m_pGetReadSize(p,a))

#define             CSocketGetFd(p)                         (p->m_pGetFd(p))
#define             CSocketGetType(p)                       (p->m_pGetType(p))

#define				CSocketSetEventParameter(p,a)			(p->m_pSetEventParameter(p,a))
#define				CSocketGetEventParameter(p)				(p->m_pGetEventParameter(p))

#define             CSocketSetExtendParameter(p,a)          (p->m_pSetExtendParameter(p,a))
#define             CSocketGetExtendParameter(p)			(p->m_pGetExtendParameter(p))

#endif      //END _ME_SOCKET_H_
