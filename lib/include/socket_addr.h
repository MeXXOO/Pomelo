#ifndef     _ME_SOCKET_ADDR_H_
#define     _ME_SOCKET_ADDR_H_

#include    "platdefine.h"

#ifdef          PROJECT_FOR_WINDOWS

#include        <WS2tcpip.h>

#else

#endif

#ifndef IN6ADDRSZ
#define IN6ADDRSZ   16
#endif

#ifndef INT16SZ
#define INT16SZ sizeof(short)
#endif

#ifndef IN4ADDRSZ
#define IN4ADDRSZ    4
#endif

#ifndef IN6ADDRSTRSZ
#define IN6ADDRSTRSZ   40
#endif

#ifndef IN4ADDRSTRSZ
#define IN4ADDRSTRSZ   16
#endif

/* socket address ip4/ip6 */
typedef struct _socket_addr_t_   socket_addr_t;
struct _socket_addr_t_                          
{
    char*    hostname;                   //host name
    void*    ipstr;						 //ip address 
    uint16_t   family;                     //address family ip_4 ip_6 ...
	char	 ipBuf[64];					 //save ip string
    union{
        struct sockaddr_in      addr_ip4;
        struct sockaddr_in6     addr_ip6; 
    };                                   //union address
    socket_addr_t*  next;                //more than one net device
};

uint8_t inet_ntop( int af, const void *src, char *dst, int size );
char    inet_pton( int af, const char *src, void *dst );

uint8_t   	socket_addr_init( socket_addr_t* socket_addr , char* ipaddress , uint16_t port , uint16_t family );
uint8_t   	socket_addr_init_byaddr( socket_addr_t* socket_addr , void* addr , uint16_t family );
char*		socket_addr_ipaddr( socket_addr_t* socket_addr );
int			socket_addr_ipport( socket_addr_t* socket_addr );

#endif
