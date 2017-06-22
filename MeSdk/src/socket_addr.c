#include    "../include/include.h"
//#include	<in6addr.h>

#ifdef  PROJECT_FOR_WINDOWS
#include        <errno.h>
#define     EAFNOSUPPORT    WSAEAFNOSUPPORT
#else

#endif

static char  inet_pton4(const char *src, uchar *dst)
{
	static const char digits[] = "0123456789";
	int saw_digit, octets, ch;
	uchar tmp[IN4ADDRSZ], *tp;

	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr(digits, ch)) != NULL) {
			uint new = *tp * 10 + (pch - digits);

			if (new > 255)
				return (0);
			*tp = new;
			if (! saw_digit) {
				if (++octets > 4)
					return (0);
				saw_digit = 1;
			}
		} else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return (0);
			*++tp = 0;
			saw_digit = 0;
		} else
			return (0);
	}
	if (octets < 4)
		return (0);

	memcpy(dst, tmp, IN4ADDRSZ);
	return (1);
}

static char  inet_pton6(const char *src, uchar *dst)
{
	static const char xdigits_l[] = "0123456789abcdef",
			  xdigits_u[] = "0123456789ABCDEF";
	uchar tmp[IN6ADDRSZ], *tp, *endp, *colonp;
	const char *xdigits, *curtok;
	int ch, saw_xdigit;
	uint val;

	memset((tp = tmp), '\0', IN6ADDRSZ);
	endp = tp + IN6ADDRSZ;
	colonp = NULL;
	/* Leading :: requires some special handling. */
    if (*src == ':')
        if (*++src != ':')
            return (0);
	curtok = src;
	saw_xdigit = 0;
	val = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;
        /* 大小写支持 */
		if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
			pch = strchr((xdigits = xdigits_u), ch);
		if (pch != NULL) {
			val <<= 4; /* 左移4位 */
			val |= (pch - xdigits); /* 地址差刚好对应16进制值(0-15) */
			if (val > 0xffff)   return (0); /* 数据异常 */
			saw_xdigit = 1;
			continue;
		}
		if (ch == ':') {
			curtok = src;
			if (!saw_xdigit) {
				if (colonp) /* ::多个冒号相连 */
					return (0);
				colonp = tp;
				continue;
			}
            /* ipv6长度超过16字节,异常 */
			if (tp + INT16SZ > endp)
				return (0);
			*tp++ = (uchar) (val >> 8) & 0xff;
			*tp++ = (uchar) val & 0xff;
			saw_xdigit = 0;
			val = 0;
			continue;
		}
        /* 可能ipv4地址,存在小数点 */
		if (ch == '.' && ((tp + IN4ADDRSZ) <= endp) &&
		    inet_pton4(curtok, tp) > 0) {
			tp += IN4ADDRSZ;
			saw_xdigit = 0;
			break;	/* '\0' was seen by inet_pton4(). */
		}
		return (0);
	}
	if (saw_xdigit) {
		if (tp + INT16SZ > endp)
			return (0);
		*tp++ = (uchar) (val >> 8) & 0xff;
		*tp++ = (uchar) val & 0xff;
	}
	if (colonp != NULL) {
		/*
		 * Since some memmove()'s erroneously fail to handle
		 * overlapping regions, we'll do the shift by hand.
		 */
		const int n = tp - colonp;
		int i;

		for (i = 1; i <= n; i++) {
			endp[- i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}
	if (tp != endp)
		return (0);
	memcpy(dst, tmp, IN6ADDRSZ);
	return (1);
}


char inet_pton(int af, const char *src, void *dst)
{
    switch (af) {
    case AF_INET:
        return (inet_pton4(src, dst));
    case AF_INET6:
        return (inet_pton6(src, dst));
    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
    /* NOTREACHED */
}

static uint8 inet_ntop4(const uchar *src, char *dst, int size)
{
    const int MIN_SIZE = 16; /* space for 255.255.255.255\0 */
    int n = 0;
    char *next = dst;
    
    if (size < MIN_SIZE) {
        return FALSE;
    }
    do {
        uchar u = *src++;
        if (u > 99) {
            *next++ = '0' + u/100;
            u %= 100;
            *next++ = '0' + u/10;
            u %= 10;
        }
        else if (u > 9) {
            *next++ = '0' + u/10;
            u %= 10;
        }
        *next++ = '0' + u;
        *next++ = '.';
        n++;
    } while (n < 4);
    *--next = 0;

    return TRUE;
}

static uint8 inet_ntop6(const uchar *src, char *dst, int size)
{
    /*
     * Note that int32_t and int16_t need only be "at least" large enough
     * to contain a value of the specified size.  On some systems, like
     * Crays, there is no such thing as an integer variable with 16 bits.
     * Keep this in mind if you think this function should have been coded
     * to use pointer overlays.  All the world's not a VAX.
     */
    char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;
    struct { int base, len; } best = {-1, 0}, cur = {-1, 0};
    uint words[IN6ADDRSZ / INT16SZ];
    int i;
    const uchar *next_src, *src_end;
    uint *next_dest;

    /*
     * Preprocess:
     *	Copy the input (bytewise) array into a wordwise array.
     *	Find the longest run of 0x00's in src[] for :: shorthanding.
     */
    next_src = src;
    src_end = src + IN6ADDRSZ;
    next_dest = words;
    i = 0;
    do {
        uint next_word = (uint)*next_src++;
        next_word <<= 8;
        next_word |= (uint)*next_src++;
        *next_dest++ = next_word;

        if (next_word == 0) {
            if (cur.base == -1) {
                cur.base = i;
                cur.len = 1;
            }
            else {
                cur.len++;
            }
        } else {
            if (cur.base != -1) {
                if (best.base == -1 || cur.len > best.len) {
                    best = cur;
                }
                cur.base = -1;
            }
        }

        i++;
    } while (next_src < src_end);

    if (cur.base != -1) {
        if (best.base == -1 || cur.len > best.len) {
            best = cur;
        }
    }
    if (best.base != -1 && best.len < 2) {
        best.base = -1;
    }

    /*
     * Format the result.
     */
    tp = tmp;
    for (i = 0; i < (IN6ADDRSZ / INT16SZ);) {
        /* Are we inside the best run of 0x00's? */
        if (i == best.base) {
            *tp++ = ':';
            i += best.len;
            continue;
        }
        /* Are we following an initial run of 0x00s or any real hex? */
        if (i != 0) {
            *tp++ = ':';
        }
        /* Is this address an encapsulated IPv4? */
        if (i == 6 && best.base == 0 &&
            (best.len == 6 || (best.len == 5 && words[5] == 0xffff))) {
            if (!inet_ntop4(src+12, tp, sizeof tmp - (tp - tmp))) {
                return FALSE;
            }
            tp += strlen(tp);
            break;
        }
        //QX @2013/07/31 
        tp += _snprintf(tp, sizeof tmp - (tp - tmp), "%x", words[i]);
        i++;
    }
    /* Was it a trailing run of 0x00's? */
    if (best.base != -1 && (best.base + best.len) == (IN6ADDRSZ / INT16SZ)) {
        *tp++ = ':';
    }
    *tp++ = '\0';

    /*
     * Check for overflow, copy, and we're done.
     */
    if ((int)(tp - tmp) > size) {
        errno = ENOSPC;
        return FALSE;
    }

    strcpy(dst, tmp);

    return TRUE;
}

uint8 inet_ntop(int af, const void *src, char *dst, int size)
{
    switch (af) {
    case AF_INET:
        return (inet_ntop4(src, dst, size));
    case AF_INET6:
        return (inet_ntop6(src, dst, size));
    default:
        return FALSE;
    }
    /* NOTREACHED */
}

uint8   socket_addr_init( socket_addr_t* socket_addr , char* ipaddress , ushort port , ushort family )
{
    uint8 bRet = TRUE;
    
	memset( socket_addr, 0, sizeof(socket_addr_t) );
    socket_addr->family = family;
    
    if( family==AF_INET )
    {
        socket_addr->addr_ip4.sin_port      = htons(port);
        socket_addr->addr_ip4.sin_family    = AF_INET; 
        if( ipaddress==NULL )
        {
            socket_addr->addr_ip4.sin_addr.s_addr = INADDR_ANY;
            DebugLogString( TRUE , "[socket_addr_init] set local ipv4 any address!!" );
        }
        else
        {
            if( !inet_pton(AF_INET, ipaddress, &socket_addr->addr_ip4.sin_addr) )
            {
                DebugLogString( TRUE , "[socket_addr_init] inet_pton ipv4 address:%s failed!!" , ipaddress );
            }
            //DebugLogString( TRUE , "[socket_addr_init] set ipv4 address:%s" , ipaddress );
        }
        /* 指向地址缓存,外面获取时直接转成字符串返回 */
        socket_addr->ipstr = &socket_addr->addr_ip4.sin_addr;
    }
    else if( family==AF_INET6 )
    {
        socket_addr->addr_ip6.sin6_port      = htons(port);
        socket_addr->addr_ip6.sin6_family    = AF_INET6; 
        if( ipaddress==NULL )
        {
            //socket_addr->addr_ip6.sin6_addr = in6addr_any;
            DebugLogString( TRUE , "[socket_addr_init] set local ipv6 any address!!" );
        }
        else
        {
            if( !inet_pton(AF_INET6, ipaddress, &socket_addr->addr_ip6.sin6_addr) )
            {
                DebugLogString( TRUE , "[socket_addr_init] inet_pton ipv6 address:%s failed!!" , ipaddress );
            }
            //DebugLogString( TRUE , "[socket_addr_init] set ipv6 address:%s" , ipaddress );
        }
        /* 指向地址缓存,外面获取时直接转成字符串返回 */
        socket_addr->ipstr = &socket_addr->addr_ip6.sin6_addr;
    }
    
    return bRet;
}

uint8     socket_addr_init_byaddr( socket_addr_t* socket_addr , void* addr , ushort family )
{
	socket_addr_t* addr_cur = (socket_addr_t*)addr;

    if( !socket_addr || !addr || family==AF_UNSPEC ) return FALSE;

	memset( socket_addr, 0 , sizeof(socket_addr_t) );
    socket_addr->family = family;
    if( family==AF_INET )
    {
        memcpy( &socket_addr->addr_ip4 , &addr_cur->addr_ip4 , sizeof(struct sockaddr_in) );
        socket_addr->ipstr = &socket_addr->addr_ip4.sin_addr;
    }
    else
    {
        memcpy( &socket_addr->addr_ip6 , &addr_cur->addr_ip6 , sizeof(struct sockaddr_in6) );
        socket_addr->ipstr = &socket_addr->addr_ip6.sin6_addr;
    }

    return TRUE;
}

char*	socket_addr_ipaddr( socket_addr_t* socket_addr )
{
	uint8 bRet = FALSE;

	if( socket_addr->family==AF_INET && sizeof(socket_addr->ipBuf)/sizeof(socket_addr->ipBuf[0]) >= IN4ADDRSTRSZ )
	{
		bRet = inet_ntop( AF_INET , socket_addr->ipstr , socket_addr->ipBuf , 64 );
	}
	else if( socket_addr->family==AF_INET6 && sizeof(socket_addr->ipBuf)/sizeof(socket_addr->ipBuf[0]) >= IN6ADDRSTRSZ )
	{
		bRet = inet_ntop( AF_INET6 , socket_addr->ipstr , socket_addr->ipBuf , 64 );
	}
	
	return bRet ? socket_addr->ipBuf : NULL;
}

int		socket_addr_ipport( socket_addr_t* socket_addr )
{
	return socket_addr->family==AF_INET ? ntohs(socket_addr->addr_ip4.sin_port) : ntohs(socket_addr->addr_ip6.sin6_port);
}