#ifndef     _ME_SOCKET_COMMON_H_
#define     _ME_SOCKET_COMMON_H_

/* rcv raw packet */
typedef struct _IMeRcvPacket
{
    socket_addr_t rcv_addr;      /* 数据包源地址 */

	int     nLen;               /* 数据包数据长度 */
	char*	lpData;				/* 数据 */

	int		nPacketRef;			/* 数据包被引用次数 */
}IMeRcvPacket;


IME_EXTERN_C	IMeRcvPacket* IMeRcvPacketCreate( IMeMemory* pMemory , char* lpData , int nLen , socket_addr_t* pRSAddr );
IME_EXTERN_C	void    IMeRcvPacketDestroy( IMeMemory* pMemory , IMeRcvPacket* pRcvPacket );
IME_EXTERN_C	void	IMeRcvPacketAddRef( IMeRcvPacket* pRcvPacket );



/* snd raw packet */
typedef struct _IMeSndPacket{
	socket_addr_t snd_addr;      /* 数据包目标地址 */

	int     nLen;               /* 数据长度 */
	char*	lpData;				/* 数据 */

    uint8_t   nSndTimes;          /* 数据包累计发送的次数 */
    uint32_t    nLastSndTime;       /* 数据上一次发送的时间 */
}IMeSndPacket;


IME_EXTERN_C	IMeSndPacket* IMeSndPacketCreate( IMeMemory* pMemory , char* lpData , int nLen );
IME_EXTERN_C	void    IMeSndPacketDestroy( IMeMemory* pMemory , IMeSndPacket* pSndPacket );


#endif      //end _ME_SOCKET_COMMON_H_

