#include    "../include/include.h"

IME_EXTERN_C IMeRcvPacket* IMeRcvPacketCreate( IMeMemory* pMemory , char* lpData , int nLen , socket_addr_t* pRSAddr )
{
    IMeRcvPacket* pRcvPacket = (IMeRcvPacket*)CMemoryCalloc( pMemory , sizeof(IMeRcvPacket) );
    if( pRcvPacket )
    {
		pRcvPacket->lpData = CMemoryAlloc( pMemory , nLen );
		if( !pRcvPacket->lpData )
		{
			CMemoryFree( pMemory , pRcvPacket );
			pRcvPacket = NULL;
		}
        memcpy( &pRcvPacket->rcv_addr , pRSAddr , sizeof(socket_addr_t) );
        memcpy( pRcvPacket->lpData , lpData , nLen );
        pRcvPacket->nLen = nLen;
		pRcvPacket->nPacketRef++;
    }
       
    return pRcvPacket;
}

IME_EXTERN_C	void	IMeRcvPacketAddRef( IMeRcvPacket* pRcvPacket )
{
	pRcvPacket->nPacketRef++;
}

IME_EXTERN_C void    IMeRcvPacketDestroy( IMeMemory* pMemory , IMeRcvPacket* pRcvPacket )
{
    /* 该内存未被引用 */
	pRcvPacket->nPacketRef--;
    if( !pRcvPacket->nPacketRef )
    {
		CMemoryFree( pMemory , pRcvPacket->lpData );
        CMemoryFree( pMemory , pRcvPacket );
    }
}




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

IME_EXTERN_C	IMeSndPacket* IMeSndPacketCreate( IMeMemory* pMemory , char* lpData , int nLen )
{
	IMeSndPacket* pSndPacket = (IMeSndPacket*)CMemoryCalloc( pMemory , sizeof(IMeSndPacket) );
    if( pSndPacket )
    {
		pSndPacket->lpData = CMemoryAlloc( pMemory , nLen );
		if( !pSndPacket->lpData )
		{
			CMemoryFree( pMemory , pSndPacket );
			pSndPacket = NULL;
		}
        memcpy( pSndPacket->lpData , lpData , nLen );
        pSndPacket->nLen = nLen;
    }
       
    return pSndPacket;
}

IME_EXTERN_C	void    IMeSndPacketDestroy( IMeMemory* pMemory , IMeSndPacket* pSndPacket )
{
	CMemoryFree( pMemory , pSndPacket->lpData );
	CMemoryFree( pMemory , pSndPacket );
}
