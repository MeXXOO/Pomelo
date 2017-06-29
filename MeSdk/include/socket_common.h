#ifndef     _ME_SOCKET_COMMON_H_
#define     _ME_SOCKET_COMMON_H_

/* rcv raw packet */
typedef struct _IMeRcvPacket
{
    socket_addr_t rcv_addr;      /* ���ݰ�Դ��ַ */

	int     nLen;               /* ���ݰ����ݳ��� */
	char*	lpData;				/* ���� */

	int		nPacketRef;			/* ���ݰ������ô��� */
}IMeRcvPacket;


IME_EXTERN_C	IMeRcvPacket* IMeRcvPacketCreate( IMeMemory* pMemory , char* lpData , int nLen , socket_addr_t* pRSAddr );
IME_EXTERN_C	void    IMeRcvPacketDestroy( IMeMemory* pMemory , IMeRcvPacket* pRcvPacket );
IME_EXTERN_C	void	IMeRcvPacketAddRef( IMeRcvPacket* pRcvPacket );



/* snd raw packet */
typedef struct _IMeSndPacket{
	socket_addr_t snd_addr;      /* ���ݰ�Ŀ���ַ */

	int     nLen;               /* ���ݳ��� */
	char*	lpData;				/* ���� */

    uint8_t   nSndTimes;          /* ���ݰ��ۼƷ��͵Ĵ��� */
    uint32_t    nLastSndTime;       /* ������һ�η��͵�ʱ�� */
}IMeSndPacket;


IME_EXTERN_C	IMeSndPacket* IMeSndPacketCreate( IMeMemory* pMemory , char* lpData , int nLen );
IME_EXTERN_C	void    IMeSndPacketDestroy( IMeMemory* pMemory , IMeSndPacket* pSndPacket );


#endif      //end _ME_SOCKET_COMMON_H_

