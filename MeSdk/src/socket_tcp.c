#include    "../include/include.h"

#define		DataPack_Cache_Default      50    //50 data packet
#define     TCP_SOCKET_MAX_BUFFER_SIZE  2*1024*1024

//[tcpH-20][ipH-20] ===>1460    ethernet mtu ===> 1500
//[udpH-8][ipH-20]  ===>1472    ethernet mtu ===> 1500

typedef		struct	_IMeTcpDataPacket{
	char*	lpData;
	int		nLen;
	int		nCacheLen;
	int		nOffset;
}IMeTcpDataPacket;

typedef     struct  _IMeCSocketTcp
{
    IMeSocketTcp    vtbl;

    IMeSocket* m_pSocket;

	IMeSocketDispatchGroup*	m_pSocketDispatchGroup;

    OnSocketRcvDataCallBack m_DataCB;
    void*   m_upApp;
    
    IMeMemory*  m_pMemory;

	//send packet array
    IMeList*	m_pListPacket;
    IMeLock*    m_lockerPacket;
    int m_nSndBufferCnt;
    int m_nMtuSize;
    int m_nSocketRcvBufferCacheSize;

	//receive cache packet
	IMeTcpDataPacket	m_RcvCachePacket;

	uint32_t	m_dispatchUser_extendParameter;

	uint8_t	m_bRunning;
    IMeEvent*   m_evWaitFree;
}IMeCSocketTcp;


/* +++++++++++++++++++++dispatch user base interface++++++++++++++++++ */
/* +++++++++++++++++++++dispatch user base interface++++++++++++++++++ */
/* +++++++++++++++++++++dispatch user base interface++++++++++++++++++ */
void    IMeTcpSocketDispatchUserRemove( IMeSocketDispatchUser* pSocketDispatchUser )
{
    IMeCSocketTcp* pSocketTcp = (IMeCSocketTcp*)pSocketDispatchUser;

    if( pSocketTcp->m_evWaitFree )
        CEventSet( pSocketTcp->m_evWaitFree );

    DebugLogString( TRUE , "[IMeTcpSocketDispatchUserRemove] socket remove from dispatch group!!" );
}

void	IMeTcpSocketDispatchUserSend( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCSocketTcp* pSocketTcp = (IMeCSocketTcp*)pSocketDispatchUser;
	
	IMeTcpDataPacket*	pDataPacket;
	
	POMInteger nRes = 0;
	int nRemainLen;

	do{
		CLock_Lock( pSocketTcp->m_lockerPacket );
		pDataPacket = (IMeTcpDataPacket*)CListGetHead(pSocketTcp->m_pListPacket,NULL);
		CLock_Unlock( pSocketTcp->m_lockerPacket );
		
		if( !pDataPacket )	break;
		
		while( pSocketTcp->m_bRunning )
		{
			nRemainLen = pDataPacket->nLen - pDataPacket->nOffset;

			//write 1KB each time
			nRes = CSocketSend( pSocketTcp->m_pSocket , pDataPacket->lpData+pDataPacket->nOffset , (nRemainLen>pSocketTcp->m_nMtuSize?pSocketTcp->m_nMtuSize:nRemainLen) , 0 );
			
			//write exception
			if( nRes <= 0 )		break;
			
			pDataPacket->nOffset += nRes;
			
			//write over , remove & release 
			if( pDataPacket->nOffset >= pDataPacket->nLen )
			{
				CLock_Lock( pSocketTcp->m_lockerPacket );
				CListRemoveHead( pSocketTcp->m_pListPacket );
				CLock_Unlock( pSocketTcp->m_lockerPacket );
				
				CMemoryFree( pSocketTcp->m_pMemory , pDataPacket->lpData );
				CMemoryFree( pSocketTcp->m_pMemory , pDataPacket );
				break;
			}
		}
	}while( nRes > 0 && pSocketTcp->m_bRunning );

	//no extra data need to send £¬unregister write event
	if( !pDataPacket )
	{
        DebugLogString( TRUE , "[IMeTcpSocketDispatchUserSend] no packet need send && current max cache packet count:%d!!" , pSocketTcp->m_nSndBufferCnt );
        //improve send packet rate, plus 1 packet each time
        ++pSocketTcp->m_nSndBufferCnt;
        CSocketDispatchGroupRemoveEvent( pSocketTcp->m_pSocketDispatchGroup , (IMeSocketDispatchUser*)pSocketTcp , SEvent_Write );
	}
}

////check socket rcv buffer size is enough
// if( nSocketBufferCacheSize >= pSocketTcp->m_nSocketRcvBufferCacheSize && pSocketTcp->m_nSocketRcvBufferCacheSize<TCP_SOCKET_MAX_BUFFER_SIZE )
// {
//     pSocketTcp->m_nSocketRcvBufferCacheSize += 1024;
//     CSocketSetOpt( pSocketTcp->m_pSocket, SOCKET_SO_RCVBUF, pSocketTcp->m_nSocketRcvBufferCacheSize );
//     DebugLogString( TRUE , "[IMeTcpSocketDispatchUserRcv] change socket rcv-buffer size to:%d" , pSocketTcp->m_nSocketRcvBufferCacheSize );
// }

void	IMeTcpSocketDispatchUserRcv( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCSocketTcp* pSocketTcp = (IMeCSocketTcp*)pSocketDispatchUser;
	POMInteger nRes;
	
	IMeTcpDataPacket* pCacheDataPacket = &pSocketTcp->m_RcvCachePacket;
    
    while( pSocketTcp->m_bRunning )
    {
        //read new packet
        if( pCacheDataPacket->nLen == pCacheDataPacket->nOffset )
        {
            int newPacketLen = 0;
            nRes = CSocketRecv( pSocketTcp->m_pSocket , (char*)&newPacketLen , 4 , 0 );
            //no data in buffer or error data
            if( nRes<=0 || newPacketLen==0 )
            {
                //notify connection blocked
                if( nRes < 0 )
                {
                    CSocketDispatchGroupRemoveEvent( pSocketTcp->m_pSocketDispatchGroup , (IMeSocketDispatchUser*)pSocketTcp , SEvent_All );
                    pSocketTcp->m_DataCB( NULL , 0 , pSocketTcp , pSocketTcp->m_upApp );
                }
                //error data
                else if( nRes > 0 && newPacketLen == 0 )
                    DebugLogString( TRUE , "[IMeTcpSocketDispatchUserRcv] receive data length error!!" );
                break;
            }
            
            //realloc new packet
            if( pCacheDataPacket->nCacheLen < newPacketLen )
            {
                if( pCacheDataPacket->lpData )
                {
                    CMemoryFree( pSocketTcp->m_pMemory , pCacheDataPacket->lpData );
                    pCacheDataPacket->nCacheLen = 0;
                }
                
                pCacheDataPacket->lpData = CMemoryAlloc( pSocketTcp->m_pMemory , newPacketLen );
                if( !pCacheDataPacket->lpData )
                {
                    DebugLogString( TRUE , "[IMeTcpSocketDispatchUserRcv] malloc failed!!" );
                    break;
                }
                
                pCacheDataPacket->nCacheLen = newPacketLen;
            }
            
            //init new packet length-info
            pCacheDataPacket->nLen = newPacketLen;
            pCacheDataPacket->nOffset = 0;
        }
        
        nRes = CSocketRecv( pSocketTcp->m_pSocket , pCacheDataPacket->lpData+pCacheDataPacket->nOffset , pCacheDataPacket->nLen-pCacheDataPacket->nOffset , 0 );
        if( nRes > 0 )
        {
            pCacheDataPacket->nOffset += nRes;
            //read data over
            if( pCacheDataPacket->nLen==pCacheDataPacket->nOffset )
            {
                pSocketTcp->m_DataCB( pCacheDataPacket->lpData , pCacheDataPacket->nLen , pSocketTcp , pSocketTcp->m_upApp );
            }
        }
        //notify connection blocked
        else if( nRes < 0 )
        {
            CSocketDispatchGroupRemoveEvent( pSocketTcp->m_pSocketDispatchGroup , (IMeSocketDispatchUser*)pSocketTcp , SEvent_All );
            pSocketTcp->m_DataCB( NULL , 0 , pSocketTcp , pSocketTcp->m_upApp );
        }

        if( nRes <= 0 ) break;
    }
}


void	IMeTcpSocketDispatchUserException( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCSocketTcp* pSocketTcp = (IMeCSocketTcp*)pSocketDispatchUser;
	DebugLogString( TRUE , "[IMeTcpSocketDispatchUserException] socket exception event occur :%0x!!" , (uint32_t)pSocketTcp );
}

void	IMeTcpSocketDispatchUserSetExtendParameter( IMeSocketDispatchUser* pSocketDispatchUser , uint32_t extendParameter )
{
	IMeCSocketTcp* pSocketTcp = (IMeCSocketTcp*)pSocketDispatchUser;
	pSocketTcp->m_dispatchUser_extendParameter = extendParameter;
}

uint32_t	IMeTcpSocketDispatchUserGetExtendParameter( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCSocketTcp* pSocketTcp = (IMeCSocketTcp*)pSocketDispatchUser;
	return pSocketTcp->m_dispatchUser_extendParameter;
}

IMeSocket*	IMeTcpSocketDispatchUserGetSocket( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCSocketTcp* pSocketTcp = (IMeCSocketTcp*)pSocketDispatchUser;
	return pSocketTcp->m_pSocket;
}

uint8_t   IMeCSocketTcpConnect( IMeSocketTcp* pISocketTcp , char* serverAddr , uint16_t serverPort )
{
    IMeCSocketTcp* pSocketTcp = (IMeCSocketTcp*)pISocketTcp;
    return CSocketConnect(pSocketTcp->m_pSocket,serverAddr,serverPort);
}

int     IMeCSocketTcpSend( IMeSocketTcp* pISocketTcp , char* lpData , int nLen )
{
    IMeCSocketTcp* pSocketTcp = (IMeCSocketTcp*)pISocketTcp;
	
    IMeTcpDataPacket*	pDataPacket = NULL;
	int nPacketCacheCnt;
	
	if( nLen==0 || !lpData )	return 0;

	CLock_Lock( pSocketTcp->m_lockerPacket );
	nPacketCacheCnt = CListGetCount(pSocketTcp->m_pListPacket);
	CLock_Unlock( pSocketTcp->m_lockerPacket );

	if( nPacketCacheCnt >= pSocketTcp->m_nSndBufferCnt )
	{
		if( pSocketTcp->m_nSndBufferCnt > DataPack_Cache_Default )
			--pSocketTcp->m_nSndBufferCnt;
		IMeSleep(1);
		return 0;
	}

	pDataPacket = (IMeTcpDataPacket*)CMemoryCalloc( pSocketTcp->m_pMemory , sizeof(IMeTcpDataPacket) );
	if( pDataPacket )
	{
		pDataPacket->lpData = CMemoryAlloc(pSocketTcp->m_pMemory,nLen+4/*data-len*/);
		if( pDataPacket->lpData )
		{
			memcpy( pDataPacket->lpData , &nLen , 4 );
			memcpy( &pDataPacket->lpData[4] , lpData , nLen );
			pDataPacket->nLen = nLen+4;
		}
		else
		{
			CMemoryFree( pSocketTcp->m_pMemory , pDataPacket );
			pDataPacket=  NULL;
		}
	}

	if( pDataPacket )
	{
		CLock_Lock( pSocketTcp->m_lockerPacket );
		CListAddTail( pSocketTcp->m_pListPacket , pDataPacket );
		CLock_Unlock( pSocketTcp->m_lockerPacket );

		CSocketDispatchGroupAddEvent( pSocketTcp->m_pSocketDispatchGroup , (IMeSocketDispatchUser*)pSocketTcp , SEvent_Write );

	    return nLen;
	}

	return 0;
}

void    IMeCSocketTcpSetOpt( IMeSocketTcp* pISocketTcp, int opt, int optValue )
{
    IMeCSocketTcp* pSocketTcp = (IMeCSocketTcp*)pISocketTcp;
    if( pSocketTcp->m_pSocket && opt != SOCKET_SO_NONBLOCK )
        CSocketSetOpt( pSocketTcp->m_pSocket, opt, optValue );
}

void    IMeCSocketTcpDestroy( IMeSocketTcp* pISocketTcp )
{
    IMeCSocketTcp* pSocketTcp = (IMeCSocketTcp*)pISocketTcp;
	
	IMeTcpDataPacket* pDataPacket;
	
    pSocketTcp->m_evWaitFree = CEventCreate(TRUE,FALSE);
    CEventReset( pSocketTcp->m_evWaitFree );

    CSocketDispatchGroupRemoveUser( pSocketTcp->m_pSocketDispatchGroup , (IMeSocketDispatchUser*)pSocketTcp );
	pSocketTcp->m_bRunning = FALSE;

    while( !CEventWait(pSocketTcp->m_evWaitFree,10) );
    CEventDestroy( pSocketTcp->m_evWaitFree );

    //release cache buffer
    CLock_Lock( pSocketTcp->m_lockerPacket );
	
    while( (pDataPacket = CListRemoveHead(pSocketTcp->m_pListPacket)) )
	{
		CMemoryFree( pSocketTcp->m_pMemory , pDataPacket->lpData );
		CMemoryFree( pSocketTcp->m_pMemory , pDataPacket );
	}
	CListDestroy( pSocketTcp->m_pListPacket );

    CLock_Unlock( pSocketTcp->m_lockerPacket );


	CLock_Destroy( pSocketTcp->m_lockerPacket );

	if( pSocketTcp->m_RcvCachePacket.nCacheLen )
		CMemoryFree( pSocketTcp->m_pMemory , pSocketTcp->m_RcvCachePacket.lpData );

	//memory allocator
	CMemoryDestroy( pSocketTcp->m_pMemory );

	free( pSocketTcp );
}

IME_EXTERN_C IMeSocketTcp* IMeSocketTcpCreate( IMeSocketDispatchGroup* pSocketDispatchGroup , IMeSocket* pSocket , OnSocketRcvDataCallBack dataCB , void* upApp )
{
	IMeCSocketTcp*	pSocketTcp = (IMeCSocketTcp*)calloc( 1, sizeof(IMeCSocketTcp) );
	if( pSocketTcp )
	{
		pSocketTcp->m_bRunning = TRUE;
		
		pSocketTcp->m_pSocket = pSocket;
		pSocketTcp->m_pSocketDispatchGroup = pSocketDispatchGroup;

		pSocketTcp->m_DataCB = dataCB;
		pSocketTcp->m_upApp = upApp;

		pSocketTcp->m_pMemory = CMemoryCreate(IMeMemory_Default);
		pSocketTcp->m_pListPacket = CListCreate();
		pSocketTcp->m_lockerPacket = CLock_Create();
        pSocketTcp->m_nSndBufferCnt = DataPack_Cache_Default;
        pSocketTcp->m_nMtuSize = ETHERNET_TCP_MTU_SIZE;
        pSocketTcp->m_nSocketRcvBufferCacheSize = CSocketGetOpt( pSocket, SOCKET_SO_RCVBUF );

		pSocketTcp->vtbl.m_pConnect = IMeCSocketTcpConnect;
		pSocketTcp->vtbl.m_pSend = IMeCSocketTcpSend;
        pSocketTcp->vtbl.m_pSetOpt = IMeCSocketTcpSetOpt;
		pSocketTcp->vtbl.m_pDestroy = IMeCSocketTcpDestroy;

		pSocketTcp->vtbl.vtbl.m_pSetExtendParameter = IMeTcpSocketDispatchUserSetExtendParameter;
		pSocketTcp->vtbl.vtbl.m_pGetExtendParameter = IMeTcpSocketDispatchUserGetExtendParameter;
		pSocketTcp->vtbl.vtbl.m_pGetSocket = IMeTcpSocketDispatchUserGetSocket;
		pSocketTcp->vtbl.vtbl.m_pSend = IMeTcpSocketDispatchUserSend;
		pSocketTcp->vtbl.vtbl.m_pRcv = IMeTcpSocketDispatchUserRcv;
		pSocketTcp->vtbl.vtbl.m_pException = IMeTcpSocketDispatchUserException;
        pSocketTcp->vtbl.vtbl.m_pRemove = IMeTcpSocketDispatchUserRemove;
	}

	return (IMeSocketTcp*)pSocketTcp;
}
