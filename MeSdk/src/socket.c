#include        "../include/include.h"

#ifdef          PROJECT_FOR_WINDOWS

#include        <errno.h>
#pragma         comment(lib,"ws2_32.lib")

#define         NET_SOCKET_ERROR         SOCKET_ERROR
#define         NET_INVALIDSOCKET        INVALID_SOCKET
#define         GetNetOsError()          WSAGetLastError()
#define         SOCKET_FD(fd)            (fd)
#define         SOCKET_CONNECT_PROGRESS  WSAEWOULDBLOCK

IME_EXTERN_C char*	ConvertErrorCodeToString(DWORD ErrorCode)    
{    
    HLOCAL LocalAddress=NULL;    
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM,  
        NULL,ErrorCode,0,(PTSTR)&LocalAddress,0,NULL);    
    return (char*)LocalAddress;    
}

//check net is broken
char	IsConnectBlocked(DWORD dwErrorCode)
{
	if( dwErrorCode==WSAEWOULDBLOCK )
		return FALSE;
	return TRUE;
}

#elif           PROJECT_FOR_LINUX

#define         GetNetOsError()           errno
#define         SOCKET_FD(fd)             (fd+1)
#define         SOCKET_CONNECT_PROGRESS   EINPROGRESS

IME_EXTERN_C char*	ConvertErrorCodeToString(uint ErrorCode) 
{
    return strerror(ErrorCode);
}

//check net is broken
char	IsConnectBlocked(int nErrorCode)
{
	if( nErrorCode==EWOULDBLOCK 
		|| nErrorCode==EAGAIN
		|| nErrorCode==EINTR)
		return FALSE;
	return TRUE;
}

#endif          // End Plat Difference

#if !defined(EAFNOSUPPORT) && defined(WSAEAFNOSUPPORT)
#define EAFNOSUPPORT WSAEAFNOSUPPORT
#endif


IME_EXTERN_C char*  IMeSocketConvertErrorCodeToString(uint ErrorCode)
{
	return ConvertErrorCodeToString(ErrorCode);
}

IME_EXTERN_C int	IMeSocketGetNetError()
{
	return GetNetOsError();
}

//////////////////////////////////////////////////////////////////////////
//********************** socket address start **************************//
//////////////////////////////////////////////////////////////////////////
/*
    socket struct 
*/
typedef struct _IMeCSocket
{
    IMeSocket   interfacefunction;
    HSOCKET  socket_des;         //socket description
    int s_type;                 //socket type:SOCK_STREAM/SOCK_DGRAM/SOCK_RAM
    int s_protocol;             //socket protocol:UDP/TCP
    int s_timeout;              //socket operation timeout
    int s_mask;                 //socket opt mask
    socket_addr_t*  local_addr;             //local net address
    socket_addr_t*  remote_addr;            //remote net address
	uint s_extendparameter;
	uint s_eventparameter;
}IMeCSocket; 


uint8     IMeCSocketGetAddrStr(IMeSocket* pISocket , uint8 bRemote , char* ipbuf , int nLen)
{
    uint8 bRet = TRUE;
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    
    if( !ipbuf )    return FALSE;
    
    if( bRemote )
    {
        if( pSocket->remote_addr->family==AF_INET && nLen>=IN4ADDRSTRSZ )
        {
            bRet = inet_ntop( AF_INET , pSocket->remote_addr->ipstr , ipbuf , nLen );
        }
        else if( pSocket->remote_addr->family==AF_INET6 && nLen>=IN6ADDRSTRSZ )
        {
            bRet = inet_ntop( AF_INET6 , pSocket->remote_addr->ipstr , ipbuf , nLen );
        }
    }
    else
    {
        if( pSocket->local_addr->family==AF_INET && nLen>=IN4ADDRSTRSZ )
        {
            bRet = inet_ntop( AF_INET , pSocket->local_addr->ipstr , ipbuf , nLen );
        }
        else if( pSocket->local_addr->family==AF_INET6 && nLen>=IN6ADDRSTRSZ )
        {
            bRet = inet_ntop( AF_INET6 , pSocket->local_addr->ipstr , ipbuf , nLen );
        }
    }

    return bRet;
}

uint8         IMeCSocketGetAddrNum(IMeSocket* pISocket , uint8 bRemote , char* bytebufer , int nLen )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    uint8 bRes = FALSE;

    if( !bytebufer )  return bRes;
    
    if( bRemote )
    {
        if( pSocket->remote_addr->family==AF_INET && nLen>=IN4ADDRSZ )
        {
            memcpy( bytebufer , &pSocket->remote_addr->addr_ip4.sin_addr.s_addr , IN4ADDRSZ );
            bRes = TRUE;
        }
        else if( pSocket->remote_addr->family==AF_INET6 && nLen>=IN6ADDRSZ )
        {
            memcpy( bytebufer , pSocket->remote_addr->addr_ip6.sin6_addr.s6_addr , IN6ADDRSZ );
            bRes = TRUE;
        }
    }
    else
    {
        if( pSocket->local_addr->family==AF_INET && nLen>=IN4ADDRSZ )
        {
            memcpy( bytebufer , &pSocket->local_addr->addr_ip4.sin_addr.s_addr , IN4ADDRSZ );
            bRes = TRUE;
        }
        else if( pSocket->local_addr->family==AF_INET6 && nLen>=IN6ADDRSZ )
        {
            memcpy( bytebufer , pSocket->local_addr->addr_ip6.sin6_addr.s6_addr , IN6ADDRSZ );
            bRes = TRUE;
        }
    }
    
    return bRes;
}

ushort      IMeCSocketGetPort( IMeSocket* pISocket , uint8 bRemote )
{
    ushort port = 0;
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;

    if( bRemote )
    {
        if( pSocket->remote_addr->family==AF_INET )
        {
            port = ntohs(pSocket->remote_addr->addr_ip4.sin_port);
        }
        else if( pSocket->remote_addr->family==AF_INET6 )
        {
            port = ntohs(pSocket->remote_addr->addr_ip6.sin6_port);
        }
    }
    else
    {
        if( pSocket->local_addr->family==AF_INET )
        {
            port = ntohs(pSocket->local_addr->addr_ip4.sin_port);
        }    
        else if( pSocket->local_addr->family==AF_INET6 )
        {
            port = ntohs(pSocket->local_addr->addr_ip6.sin6_port);
        }
    }
    
    return port;
}

ushort           IMeCSocketGetAddrType( IMeSocket* pISocket , uint8 bRemote )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;

    return bRemote==FALSE?pSocket->local_addr->family:pSocket->remote_addr->family;
}

//////////////////////////////////////////////////////////////////////////
//********************** socket address stop ***************************//
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//********************** socket attribute opt start ********************//
//////////////////////////////////////////////////////////////////////////
#define         SOCKET_MAX_SECS_TO_LINGER   30  /* 每秒最多接受连接数 */

#define socket_is_option_set(mask, option)  ((mask & option) ==option)
#define socket_set_option(mask, option, on) \
    do {                                 \
    if (on)                          \
    *mask |= option;             \
    else                             \
    *mask &= ~option;            \
    } while (0)


uint8 soblock(HSOCKET sd)
{
    u_long zero = 0;
    
    if(ioctlsocket(sd, FIONBIO, &zero) == NET_SOCKET_ERROR) 
    {
        DebugLogString( TRUE , "[soblock]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
        return FALSE;
    }
    return TRUE;
}

uint8 sononblock(HSOCKET sd)
{
    u_long one = 1;
    
    if(ioctlsocket(sd, FIONBIO, &one) == NET_SOCKET_ERROR) 
    {
        DebugLogString( TRUE , "[sononblock]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
        return FALSE;
    }
    return TRUE;
}

uint8 IMeCSocketSetTimeOut(IMeSocket *pISocket, int new_t)
{   
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    if(new_t == 0) 
    {
        /* Set the socket non-blocking if it was previously blocking */
        if(pSocket->s_timeout != 0) 
        {
            if(sononblock(pSocket->socket_des)!=TRUE)
            {
                DebugLogString( TRUE , "[IMeCSockeSetTimeOut]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
                return FALSE;
            }
        }
    }
    else if(new_t > 0) 
    {
        /* Set the socket to blocking if it was previously non-blocking */
        if(pSocket->s_timeout == 0) 
        {
            if(soblock(pSocket->socket_des)!=TRUE)
            {
                DebugLogString( TRUE , "[IMeCSockeSetTimeOut]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
                return FALSE;
            }
        }
        /* Reset socket timeouts if the new timeout differs from the old timeout */
        if(pSocket->s_timeout != new_t) 
        {
            /* Win32 timeouts are in msec, represented as int */
            pSocket->s_timeout = new_t;
            setsockopt(pSocket->socket_des, SOL_SOCKET, SO_RCVTIMEO, (char *) &pSocket->s_timeout, sizeof(pSocket->s_timeout));
            setsockopt(pSocket->socket_des, SOL_SOCKET, SO_SNDTIMEO, (char *) &pSocket->s_timeout, sizeof(pSocket->s_timeout));
        }
    }
    else if(new_t < 0) 
    {
        int zero = 0;
        /* Set the socket to blocking with infinite timeouts */
        if(soblock(pSocket->socket_des)!=TRUE)
        {
            DebugLogString( TRUE , "[IMeCSockeSetTimeOut]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
            return FALSE;
        }
        setsockopt(pSocket->socket_des, SOL_SOCKET, SO_RCVTIMEO, (char *)&zero, sizeof(zero));
        setsockopt(pSocket->socket_des, SOL_SOCKET, SO_SNDTIMEO, (char *)&zero, sizeof(zero));
    }
    pSocket->s_timeout = new_t;
    return TRUE;
}

int  IMeCSocketGetOpt( IMeSocket* pISocket , int opt )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    int res  = 0;
    int nLen = 4;
    switch (opt) 
    {
    case SOCKET_SO_TIMEOUT: 
        /* XXX: to be deprecated */
        res = pSocket->s_timeout;
        break;
    case SOCKET_SO_RCVBUF:
        getsockopt( pSocket->socket_des , SOL_SOCKET , SO_RCVBUF , (char*)&res , &nLen );
        break;
    case SOCKET_SO_SNDBUF:
        getsockopt( pSocket->socket_des , SOL_SOCKET , SO_SNDBUF , (char*)&res , &nLen );
        break;
    case SOCKET_SO_KEEPALIVE:
    case SOCKET_SO_DEBUG:
    case SOCKET_SO_REUSEADDR:
    case SOCKET_SO_NONBLOCK:
    case SOCKET_SO_LINGER:
    default:
        res = socket_is_option_set(pSocket->s_mask, opt);
    }
    return res;
}

uint8 IMeCSocketGetReadSize( IMeSocket* pISocket , uint* pBufferLen )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
#ifdef  PROJECT_FOR_WINDOWS
    if( NET_SOCKET_ERROR==ioctlsocket( pSocket->socket_des , FIONREAD , (u_long*)&pBufferLen ) )
    {
        DebugLogString( TRUE , "[IMeCSocketGetReadSize] ioctlsocket error %s" , GetNetOsError() );
        return FALSE;
    }
#elif   PROJECT_FOR_LINUX
    if( NET_SOCKET_ERROR==ioctl( pSocket->socket_des , FIONREAD , (void*)pBufferLen ) )
    {
        DebugLogString( TRUE , "[IMeCSocketGetReadSize] ioctlsocket error %s" , GetNetOsError() );
        return FALSE;
    }
#endif
    return TRUE;
}

uint8 IMeCSocketSetOpt( IMeSocket* pISocket , int opt, int on)
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    int one;
    one = on ? 1 : 0;
    switch (opt) {
    case SOCKET_SO_TIMEOUT: 
    {
        /* XXX: To be deprecated */
        return IMeCSocketSetTimeOut(pISocket, on);
    }
    case SOCKET_SO_KEEPALIVE:
        if(on != socket_is_option_set(pSocket->s_mask , SOCKET_SO_KEEPALIVE)) 
        {
            if(setsockopt(pSocket->socket_des, SOL_SOCKET, SO_KEEPALIVE, (void *)&one, sizeof(int)) == NET_SOCKET_ERROR ) 
            {
                DebugLogString( TRUE , "[IMeCSocketSetOpt]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
                return FALSE;
            }
            socket_set_option(&pSocket->s_mask,SOCKET_SO_KEEPALIVE,on);
        }
        break;
    case SOCKET_SO_DEBUG:
        if(on != socket_is_option_set(pSocket->s_mask, SOCKET_SO_DEBUG)) {
            if (setsockopt(pSocket->socket_des, SOL_SOCKET, SO_DEBUG, (void *)&one, sizeof(int)) == NET_SOCKET_ERROR )
            {
                DebugLogString( TRUE , "[IMeCSocketSetOpt]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
                return FALSE;
            }
            socket_set_option(&pSocket->s_mask, SOCKET_SO_DEBUG, on);
        }
        break;
    case SOCKET_SO_SNDBUF:
        if(setsockopt(pSocket->socket_des, SOL_SOCKET, SO_SNDBUF, (void *)&on, sizeof(int)) == NET_SOCKET_ERROR)
        {
            DebugLogString( TRUE , "[IMeCSocketSetOpt]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
            return FALSE;
        }
        break;
    case SOCKET_SO_RCVBUF:
        if(setsockopt(pSocket->socket_des, SOL_SOCKET, SO_RCVBUF, (void *)&on, sizeof(int)) == NET_SOCKET_ERROR)
        {
            DebugLogString( TRUE , "[IMeCSocketSetOpt]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
            return FALSE;
        }
        break;
    case SOCKET_SO_REUSEADDR:
        if(on != socket_is_option_set(pSocket->s_mask, SOCKET_SO_REUSEADDR))
        {
            if(setsockopt(pSocket->socket_des, SOL_SOCKET, SO_REUSEADDR, (void *)&one, sizeof(int)) == NET_SOCKET_ERROR)
            {
                DebugLogString( TRUE , "[IMeCSocketSetOpt]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
                return FALSE;
            }
            socket_set_option(&pSocket->s_mask, SOCKET_SO_REUSEADDR, on);
        }
        break;
    case SOCKET_SO_NONBLOCK:
        if(socket_is_option_set(pSocket->s_mask, SOCKET_SO_NONBLOCK) != on)
        {
            if(on) 
            {
                if(sononblock(pSocket->socket_des)!=TRUE) 
                {
                    DebugLogString( TRUE , "[IMeCSocketSetOpt]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
                    return FALSE;
                }
            }
            else 
            {
                if (soblock(pSocket->socket_des)!=TRUE)
                {
                    DebugLogString( TRUE , "[IMeCSocketSetOpt]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
                    return FALSE;
                }
            }
            socket_set_option(&pSocket->s_mask, SOCKET_SO_NONBLOCK, on);
        }
        break;
    case SOCKET_SO_LINGER:
    {
        if(socket_is_option_set(pSocket->s_mask, SOCKET_SO_LINGER) != on) 
        {
            struct linger li;
            li.l_onoff = on;
            li.l_linger = SOCKET_MAX_SECS_TO_LINGER;
            if (setsockopt(pSocket->socket_des, SOL_SOCKET, SO_LINGER, (char *) &li, sizeof(struct linger)) == NET_SOCKET_ERROR)
            {
                DebugLogString( TRUE , "[IMeCSocketSetOpt]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
                return FALSE;
            }
            socket_set_option(&pSocket->s_mask, SOCKET_SO_LINGER, on);
        }
        break;
    }
    case SOCKET_TCP_NODELAY:
        if(socket_is_option_set(pSocket->s_mask, SOCKET_TCP_NODELAY) != on) 
        {
            int optlevel = IPPROTO_TCP;
            int optname  = TCP_NODELAY;
            if (setsockopt(pSocket->socket_des, optlevel, optname, (void *)&on, sizeof(int)) == NET_SOCKET_ERROR)
            {
                DebugLogString( TRUE , "[IMeCSocketSetOpt]:%s" , ConvertErrorCodeToString(GetNetOsError()) );
                return FALSE;
            }
            socket_set_option(&pSocket->s_mask, SOCKET_TCP_NODELAY, on);
        }
        break;
    default:
        return FALSE;
        break;
    }
    return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//********************* socket attribute opt end ***********************//
//////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////
//************************** socket net opt start **********************//
//////////////////////////////////////////////////////////////////////////
uint8     IMeCSocketConnect( IMeSocket* pISocket , char* ipstr , ushort port )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    const struct sockaddr* pRemoteAddres = NULL;
    int nRemoteAddressLen = 0;

    if( !pSocket || !ipstr )
    {
        DebugLogString( TRUE , "[IMeCSocketConnect] invalid parameter!" );
        return FALSE;
    }
    
	//first init remote addr
	socket_addr_init( pSocket->remote_addr , ipstr , port , pSocket->local_addr->family );

    if( pSocket->remote_addr->family==AF_INET )
    {
        pRemoteAddres = (struct sockaddr*)&pSocket->remote_addr->addr_ip4;
        nRemoteAddressLen = sizeof( struct sockaddr_in );
    }
    else
    {
        pRemoteAddres = (struct sockaddr*)&pSocket->remote_addr->addr_ip6;
        nRemoteAddressLen = sizeof( struct sockaddr_in6 );
    }

    if( connect(pSocket->socket_des , pRemoteAddres , nRemoteAddressLen ) != 0 )
    {
        uint       socket_error = 0;
        struct timeval tv,*ptrTime;
        fd_set write_fdset, exception_fdset;
        
        socket_error = GetNetOsError();
        
        /* 非阻塞连接,调用立即返回连接中状态 */
        if( socket_error != SOCKET_CONNECT_PROGRESS )  
        {
            DebugLogString( TRUE , "[IMeCSocketConnect] connect failed error:%s" , ConvertErrorCodeToString(socket_error) );
            return FALSE;
        }
        
        /* 设置等待时间为零,则直接返回,否则等待连接完成或是超时返回 */
        if( pSocket->s_timeout==0 )
        {
            DebugLogString( TRUE , "[IMeCSocketConnect] connecting ......" );
            return TRUE;
        }
        
        /* 等待连接完成或是连接超时 */
        FD_ZERO(&write_fdset);
        FD_SET( SOCKET_FD(pSocket->socket_des) , &write_fdset );
        FD_ZERO(&exception_fdset);
        FD_SET( SOCKET_FD(pSocket->socket_des) , &exception_fdset );
        
        if( pSocket->s_timeout<0 )
        {
            ptrTime = NULL;
        }
        else
        {
            tv.tv_sec  = pSocket->s_timeout/1000;
            tv.tv_usec = pSocket->s_timeout%1000;
            ptrTime = &tv;
        }
        
        socket_error = select( pSocket->socket_des , NULL , &write_fdset , &exception_fdset , ptrTime );
        /* invoke select function failed */
        if( socket_error==NET_SOCKET_ERROR )
        {
            socket_error = GetNetOsError();
            DebugLogString( TRUE , "[IMeCSocketConnect]:select failed error str:%s" , ConvertErrorCodeToString(socket_error) );
            return FALSE;
        }
        /* get write set flag timeout (connect timeout) */
        else if( socket_error==0 )
        {
            DebugLogString( TRUE , "[IMeCSocketConnect]:connect server in time %u time out" , pSocket->s_timeout );
            return FALSE;
        }
        /* weather exception is occured */
        if(FD_ISSET(pSocket->socket_des, &exception_fdset)) 
        {
            char exception_str[256] = { 0 };            
            int exception_str_len = 255;
            /* get error info failed */
            if( (socket_error=getsockopt(pSocket->socket_des, SOL_SOCKET, SO_ERROR, exception_str, &exception_str_len))==NET_SOCKET_ERROR ) 
            {
                socket_error = GetNetOsError();
                DebugLogString( TRUE , "[IMeCSocketConnect]:getsockopt failed error str:%s" , ConvertErrorCodeToString(socket_error) );
                return FALSE;
            }
            
            DebugLogString( TRUE , "[IMeCSocketConnect]:waiting connect server occur exception with str:%s" , exception_str );
            return FALSE;
        }
    }
    
    return TRUE;
}

uint8     IMeCSocketBind( IMeSocket* pISocket )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    const struct sockaddr* pLocalAddres = NULL;
    int nLocalAddressLen = 0;
    
    if( !pSocket || pSocket->local_addr->family==AF_UNSPEC )
    {
        DebugLogString( TRUE , "[IMeCSocketBind] null pointer or no init local address!" );
        return FALSE;
    }
    
    if( pSocket->local_addr->family==AF_INET )
    {
        pLocalAddres = (struct sockaddr*)&pSocket->local_addr->addr_ip4;
        nLocalAddressLen = sizeof( struct sockaddr_in );
    }
    else
    {
        pLocalAddres = (struct sockaddr*)&pSocket->local_addr->addr_ip6;
        nLocalAddressLen = sizeof( struct sockaddr_in6 );
    }

    if( !pSocket )  return FALSE;

    if( bind( pSocket->socket_des , pLocalAddres , nLocalAddressLen )==NET_SOCKET_ERROR ) 
    {
        DebugLogString( TRUE , "[IMeCSocketBind] bind error str:%s" , ConvertErrorCodeToString(GetNetOsError()) );
        return FALSE;
    }

    return TRUE;
}

uint8    IMeCSocketListen( IMeSocket* pISocket , int backlog )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;

    if(listen(pSocket->socket_des, backlog)==NET_SOCKET_ERROR)
    {
        DebugLogString( TRUE , "[IMeCSocketListen] listen error str:%s" , ConvertErrorCodeToString(GetNetOsError()) );
        return FALSE;
    }

    return TRUE;
}

uint8    IMeCSocketAccept( IMeSocket* pISocket , socket_addr_t* pRemoteAddr , HSOCKET* socket_des )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    uint8 bRes = FALSE;
    int nAddrLen;
    struct sockaddr* pAddr;

    if( pSocket->local_addr->family==AF_UNSPEC || !pRemoteAddr || !socket_des )  
    {
        DebugLogString( TRUE , "[IMeCSocketAccept] null pointer or invalid listen socket!" );
        return bRes;
    }    
    
    /* 接受ipv4的请求 */
    if( pSocket->local_addr->family==AF_INET )
    {
        nAddrLen = sizeof(struct sockaddr_in);
        pAddr = (struct sockaddr*)&pRemoteAddr->addr_ip4;
        pRemoteAddr->family = AF_INET;
        pRemoteAddr->ipstr = &pRemoteAddr->addr_ip4.sin_addr;
    }
    /* 接受ipv6的请求 */
    else
    {
        nAddrLen = sizeof(struct sockaddr_in6);
        pAddr = (struct sockaddr*)&pRemoteAddr->addr_ip6;
        pRemoteAddr->family = AF_INET6;
        pRemoteAddr->ipstr = &pRemoteAddr->addr_ip6.sin6_addr;
    }
    
    *socket_des = accept( pSocket->socket_des , pAddr , &nAddrLen );
    if( *socket_des==NET_SOCKET_ERROR )
    {
		int nErrorCode = GetNetOsError();
		if( IsConnectBlocked(nErrorCode) )
			DebugLogString( TRUE , "[IMeCSocketAccept] accept failed error str:%s errorNum:%d!!" , ConvertErrorCodeToString(nErrorCode) , nErrorCode );
        return FALSE;
    }

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//************************** socket net opt stop **********************//
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
//************************ socket net opt rcv/snd **********************//
//////////////////////////////////////////////////////////////////////////
int     IMeCSocketSend( IMeSocket* pISocket , char* data , int nLen , int flags )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    int   sendStatus = 0;
    uint8  bCheckWrite = TRUE;
    
    sendStatus = send(pSocket->socket_des, data, nLen, flags);
    if( sendStatus==NET_SOCKET_ERROR )
    {
        int netErrorCode = GetNetOsError();
        sendStatus = 0;
        if( IsConnectBlocked(netErrorCode) )
            DebugLogString( TRUE, "[IMeCSocketSend] send data error str:%s errorNum:%d", ConvertErrorCodeToString(netErrorCode), netErrorCode );
    }

    return sendStatus;
}

int    IMeCSocketSendTo( IMeSocket* pISocket , char* szIP , ushort nPort , ushort family , char* data , int nLen , int flags )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    int      sendStatus  = 0;
    uint8     bCheckWrite = TRUE;
    
    struct sockaddr* pRemoteAddress = NULL;
    int nRemoteAddressLen = 0;

    struct sockaddr_in6 sock_addr_ipv6 = { 0 };
    struct sockaddr_in sock_addr_ipv4 = { 0 };

    if( pSocket->local_addr->family==family && family==AF_INET )
    {
        sock_addr_ipv4.sin_family = AF_INET;
        sock_addr_ipv4.sin_port = htons(nPort);
        if( !inet_pton(AF_INET, szIP, &sock_addr_ipv4.sin_addr) )
        {
            DebugLogString( TRUE , "[IMeCSocketSendTo] inet_pton failed!1" );
            return sendStatus;
        }
        pRemoteAddress = (struct sockaddr*)&sock_addr_ipv4;
        nRemoteAddressLen = sizeof(struct sockaddr_in);
    }
    else if( pSocket->local_addr->family==family && family==AF_INET6 )
    {
        sock_addr_ipv6.sin6_family = AF_INET6;
        sock_addr_ipv6.sin6_port = htons(nPort);
        if( !inet_pton(AF_INET6, szIP, &sock_addr_ipv6.sin6_addr) )
        {
            DebugLogString( TRUE , "[IMeCSocketSendTo] inet_pton failed!1" );
            return sendStatus;
        }
        pRemoteAddress = (struct sockaddr*)&sock_addr_ipv6;
        nRemoteAddressLen = sizeof(struct sockaddr_in6);
    }
    else
    {
        DebugLogString( TRUE , "[IMeCSocketSendTo] socket family un_adapter!!" );
        return sendStatus;
    }

    sendStatus = sendto(pSocket->socket_des, data, nLen, flags, pRemoteAddress , nRemoteAddressLen );
    if( sendStatus==NET_SOCKET_ERROR )
    {
        int netErrorCode = GetNetOsError();
        sendStatus = 0;
        if( IsConnectBlocked(netErrorCode) )
            DebugLogString( TRUE , "[IMeCSocketSendTo] send data error str:%s errorNum:%d" , ConvertErrorCodeToString(netErrorCode) , netErrorCode );
    }
    
    return sendStatus;
}

int    IMeCSocketSendToByAddr( IMeSocket* pISocket , socket_addr_t* pSendAddr , char* data , int nLen , int flags )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    int      sendStatus  = 0;
    uint8     bCheckWrite = TRUE;

    struct sockaddr* pRemoteAddress = NULL;
    int nRemoteAddressLen = 0;

    if( pSocket->local_addr->family==AF_INET )
    {
        pRemoteAddress = (struct sockaddr*)&pSendAddr->addr_ip4;
        nRemoteAddressLen = sizeof(struct sockaddr_in);
    }
    else if( pSocket->local_addr->family==AF_INET6 )
    {
        pRemoteAddress = (struct sockaddr*)&pSendAddr->addr_ip6;
        nRemoteAddressLen = sizeof(struct sockaddr_in6);
    }

    sendStatus = sendto( pSocket->socket_des, data, nLen, flags, pRemoteAddress , nRemoteAddressLen );
    if( sendStatus==NET_SOCKET_ERROR )
    {
        int netErrorCode = GetNetOsError();
        sendStatus = 0;
        if( IsConnectBlocked(netErrorCode) )
            DebugLogString( TRUE , "[IMeCSocketSendToByAddr] send data error str:%s errorNum:%d" , ConvertErrorCodeToString(netErrorCode) , netErrorCode );
    }

    return sendStatus;
}

int    IMeCSocketRecv( IMeSocket* pISocket , char* buf , int nLen , int flags )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    int  rcvStatus = 0;
    
    rcvStatus = recv( pSocket->socket_des, buf, nLen, flags );

	if( rcvStatus<=0 )
	{
		if( rcvStatus==NET_SOCKET_ERROR )
		{
			int nErrorCode = GetNetOsError();
			rcvStatus = 0;
			if( IsConnectBlocked(nErrorCode) )	
			{
				rcvStatus = -1;
				DebugLogString( TRUE , "[IMeCSocketRecv] recv data error str:%s errorNum:%d!!" , ConvertErrorCodeToString(nErrorCode) , nErrorCode );
			}
		}
		//connect blocked
		else
			rcvStatus = -1;
	}
    
    return rcvStatus;
}

int    IMeCSocketRecvFrom( IMeSocket* pISocket , char* buf , int nLen , int flags )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    int  rcvStatus = 0;
    
    if( pSocket->local_addr->family==AF_INET )
    {
        int nAddrLen = sizeof(struct sockaddr_in);
        rcvStatus = recvfrom(pSocket->socket_des, buf, nLen, flags, (struct sockaddr*)&pSocket->remote_addr->addr_ip4, &nAddrLen );
    }
    else
    {
        int nAddrLen = sizeof(struct sockaddr_in6);
        rcvStatus = recvfrom(pSocket->socket_des, buf, nLen, flags, (struct sockaddr*)&pSocket->remote_addr->addr_ip6, &nAddrLen );
    }
    
    if( rcvStatus==NET_SOCKET_ERROR )
    {
        int nErrorCode = GetNetOsError();
        rcvStatus = 0;
        if( IsConnectBlocked(nErrorCode) )
            DebugLogString( TRUE , "[IMeCSocketRecvFrom] recv data error str:%s errorNum:%d" , ConvertErrorCodeToString(nErrorCode), nErrorCode );
    }

    return rcvStatus;
}

int    IMeCSocketRecvFromByAddr( IMeSocket* pISocket , char* buf , int nLen , socket_addr_t* pRcvAddr , int flags )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    int  rcvStatus = 0;
    
	pRcvAddr->family = pSocket->local_addr->family;
    if( pSocket->local_addr->family==AF_INET )
    {
        int nAddrLen = sizeof(struct sockaddr_in);
        rcvStatus = recvfrom(pSocket->socket_des, buf, nLen, flags, (struct sockaddr*)&pRcvAddr->addr_ip4, &nAddrLen );
		pRcvAddr->ipstr = &pRcvAddr->addr_ip4.sin_addr;
    }
    else
    {
        int nAddrLen = sizeof(struct sockaddr_in6);
        rcvStatus = recvfrom(pSocket->socket_des, buf, nLen, flags, (struct sockaddr*)&pRcvAddr->addr_ip6, &nAddrLen );
		pRcvAddr->ipstr = &pRcvAddr->addr_ip6.sin6_addr;
    }
    
    if( rcvStatus==NET_SOCKET_ERROR )
    {
        int nErrorCode = GetNetOsError();
        rcvStatus = 0;
        if( IsConnectBlocked(nErrorCode) )
            DebugLogString( TRUE , "[IMeCSocketRecvFromByAddr] recv data error str:%s errorNum:%d" , ConvertErrorCodeToString(nErrorCode) , nErrorCode );
    }
    
    return rcvStatus;
}

//////////////////////////////////////////////////////////////////////////
//************************ socket net opt rcv/snd **********************//
//////////////////////////////////////////////////////////////////////////

void   IMeCSocketDestroy( IMeSocket* pISocket )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    if( pSocket )
    {
        if( pSocket->socket_des!=NET_INVALIDSOCKET )
            closesocket(pSocket->socket_des);
        if( pSocket->local_addr )
            free( pSocket->local_addr );
        if( pSocket->remote_addr )
            free( pSocket->remote_addr );
        free( pSocket );
    }
}

void   IMeCSocketClose( IMeSocket* pISocket )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    if( pSocket->socket_des!=NET_INVALIDSOCKET )
    {
        closesocket(pSocket->socket_des);
        pSocket->socket_des = NET_INVALIDSOCKET;
    }
}

HSOCKET    IMeCSocketGetFd( IMeSocket* pISocket )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    return pSocket->socket_des;
}

int    IMeCSocketGetType( IMeSocket* pISocket )
{
    IMeCSocket* pSocket = (IMeCSocket*)pISocket;
    return pSocket->s_type;
}

void	IMeCSocketSetExtendParameter( IMeSocket* pISocket , uint parameter )
{
	IMeCSocket* pSocket = (IMeCSocket*)pISocket;
	pSocket->s_extendparameter = parameter;
}

uint	IMeCSocketGetExtendParameter( IMeSocket* pISocket )
{
	IMeCSocket* pSocket = (IMeCSocket*)pISocket;
	return pSocket->s_extendparameter;	
}

void	IMeCSocketSetEventParameter( IMeSocket* pISocket , uint parameter  )
{
	IMeCSocket* pSocket = (IMeCSocket*)pISocket;
	pSocket->s_eventparameter = parameter;
}

uint	IMeCSocketGetEventParameter( IMeSocket* pISocket )
{
	IMeCSocket* pSocket = (IMeCSocket*)pISocket;
	return pSocket->s_eventparameter;	
}

#define     IMeCSocketInterfaceRegister(p)       \
    ((IMeSocket*)p)->m_pDestroy = IMeCSocketDestroy; \
    ((IMeSocket*)p)->m_pClose = IMeCSocketClose;    \
    ((IMeSocket*)p)->m_pGetAddrType = IMeCSocketGetAddrType;    \
    ((IMeSocket*)p)->m_pGetAddrStr = IMeCSocketGetAddrStr;  \
    ((IMeSocket*)p)->m_pGetAddrNum = IMeCSocketGetAddrNum;  \
    ((IMeSocket*)p)->m_pGetPort = IMeCSocketGetPort;    \
    ((IMeSocket*)p)->m_pConnect = IMeCSocketConnect;    \
    ((IMeSocket*)p)->m_pBind = IMeCSocketBind;  \
    ((IMeSocket*)p)->m_pListen = IMeCSocketListen;  \
    ((IMeSocket*)p)->m_pAccept = IMeCSocketAccept;  \
    ((IMeSocket*)p)->m_pSend = IMeCSocketSend;  \
    ((IMeSocket*)p)->m_pSendTo = IMeCSocketSendTo;  \
    ((IMeSocket*)p)->m_pSendToByAddr = IMeCSocketSendToByAddr;  \
    ((IMeSocket*)p)->m_pRecv = IMeCSocketRecv;    \
    ((IMeSocket*)p)->m_pRecvFrom = IMeCSocketRecvFrom;  \
    ((IMeSocket*)p)->m_pRcvFromByAddr = IMeCSocketRecvFromByAddr;  \
    ((IMeSocket*)p)->m_pSetTimeOut = IMeCSocketSetTimeOut;  \
    ((IMeSocket*)p)->m_pGetOpt = IMeCSocketGetOpt;  \
    ((IMeSocket*)p)->m_pSetOpt = IMeCSocketSetOpt;  \
    ((IMeSocket*)p)->m_pGetReadSize = IMeCSocketGetReadSize;    \
    ((IMeSocket*)p)->m_pGetFd = IMeCSocketGetFd;    \
    ((IMeSocket*)p)->m_pGetType = IMeCSocketGetType;	\
	((IMeSocket*)p)->m_pSetExtendParameter = IMeCSocketSetExtendParameter;	\
	((IMeSocket*)p)->m_pGetExtendParameter = IMeCSocketGetExtendParameter;	\
	((IMeSocket*)p)->m_pSetEventParameter = IMeCSocketSetEventParameter;	\
	((IMeSocket*)p)->m_pGetEventParameter = IMeCSocketGetEventParameter

/* listen socket tcp use */
IMeSocket*    IMeSocketCreateAcceptSocket( HSOCKET socket_des , ushort family , void* remote_addr )
{
    IMeCSocket* pAcceptSocket = NULL;
    if( !remote_addr )
        return NULL;
    
    pAcceptSocket = (IMeCSocket*)calloc(1,sizeof(IMeCSocket));
    if( !pAcceptSocket )
    {
        DebugLogString( TRUE , "[IMeSocketCreateAcceptSocket] create socket calloc failed!" );
        return NULL;
    }
    
    pAcceptSocket->remote_addr = (socket_addr_t*)calloc(1,sizeof(socket_addr_t));
    if( !pAcceptSocket->remote_addr )
    {
        DebugLogString( TRUE , "[IMeSocketCreateAcceptSocket] remote addr memory calloc failed!" );
        free( pAcceptSocket );
        return NULL;
    }
    socket_addr_init_byaddr( pAcceptSocket->remote_addr , remote_addr , family );
    
    pAcceptSocket->local_addr = (socket_addr_t*)calloc(1,sizeof(socket_addr_t));
    if( !pAcceptSocket->local_addr )
    {
        DebugLogString( TRUE , "[IMeSocketCreateAcceptSocket] local addr memory calloc failed!" );
        free( pAcceptSocket->remote_addr );
        free( pAcceptSocket );
        return NULL;
    }

    pAcceptSocket->s_protocol = IPPROTO_TCP;
    pAcceptSocket->s_type = SOCKET_TCP;
    pAcceptSocket->socket_des = socket_des;
    IMeCSocketInterfaceRegister(pAcceptSocket);
    
    return (IMeSocket*)pAcceptSocket;
}

IME_EXTERN_C IMeSocket*     IMeSocketCreate( char* ipstr , ushort port , ushort s_family , int s_timeout , int s_type )
{
    uint8 bCreaetOk = FALSE;
	int s_protocol;

    IMeCSocket* pSocket = (IMeCSocket*)calloc(1,sizeof(IMeCSocket));
    if( !pSocket )
    {
        DebugLogString( TRUE , "[IMeSocketCreate] socket memory calloc failed!!" );
        return NULL;
    }

	if( s_type==SOCKET_TCP )
	{
		s_type = SOCK_STREAM;
		s_protocol = IPPROTO_TCP;
	}
	else if( s_type==SOCKET_UDP )
	{
		s_type = SOCK_DGRAM;
		s_protocol = IPPROTO_UDP;
	}

    while(1)
    {   
        pSocket->local_addr = (socket_addr_t*)calloc(1,sizeof(socket_addr_t));
        if( !pSocket->local_addr )
        {
            DebugLogString( TRUE , "[IMeSocketCreate] local addr memory calloc failed!" );
            break;
        }
        
        pSocket->remote_addr = (socket_addr_t*)calloc(1,sizeof(socket_addr_t));
        if( !pSocket->remote_addr )
        {
            DebugLogString( TRUE , "[IMeSocketCreate] remote addr memory calloc failed!" );
            break;
        }
        
        if( !socket_addr_init( pSocket->local_addr , ipstr , port , s_family ) )
        {
            DebugLogString( TRUE , "[IMeSocketCreate] init addr failed!!" );
            break;
        }
        
        pSocket->socket_des = socket( s_family , s_type , s_protocol );
        if( pSocket->socket_des==NET_INVALIDSOCKET )
        {
            DebugLogString( TRUE , "[IMeSocketCreate] create socket failed str:%s!!" , ConvertErrorCodeToString(GetNetOsError()) );
            break;
        }
        
        pSocket->s_timeout = s_timeout;
        pSocket->s_protocol = s_protocol;
        pSocket->s_timeout = s_timeout;
        pSocket->s_type = s_type;
        IMeCSocketInterfaceRegister(pSocket);
        bCreaetOk = TRUE;
        break;
    }
    
    if( !bCreaetOk )
    {
        if( pSocket->local_addr )
            free( pSocket->local_addr );
        if( pSocket->remote_addr )
            free( pSocket->remote_addr );
        pSocket = NULL;
    }

    return (IMeSocket*)pSocket;
}
