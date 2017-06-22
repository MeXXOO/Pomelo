
#include    "../include/include.h"

#define     UdpPackCmd_Data		1   /* 数据包中的内容为对方发送的数据 */
#define     UdpPackCmd_Ack		2   /* 数据包中的内容为对方收到该数据包的回应 */
#define		UdpPackCmd_Call		3	/* 数据包中的内容为UDP穿透呼叫数据 */

#define		DataPack_DefaultTimeInterval	100			//minimum packet time interval
#define		DataPack_MaxCacheSize			2*1024*1024	//2MB data packet
#define		DataPack_MaxLimitSize			2*1024*1024	//2MB max size each packet
#define		DataPack_MaxSndCntOneTime		200			//200 packets each time
#define		UdpUserRcvData_TimeOut			30000		//if no receive user's data in 30s , delete this user 


#define     UdpPackHeader_Size          (sizeof(IMeUdpPackHeader))
#define     UdpSubPackNode_Size         (sizeof(IMeUdpSubPackNode))
#define     UdpPacketCount(size,mtu)    ((size)/((mtu)-UdpPackHeader_Size) + ((size)%((mtu)-UdpPackHeader_Size)>0 ? 1 : 0))

#define		UdpPackWaitInfinite			-1
#define		UdpPackSendMaxTimes			5

#define		UdpPackMin(a,b)				(((a)<(b))?(a):(b))

//udp packet
/* packet header */
typedef struct _IMeUdpPackHeader
{
    uint     nCmdID;             /* 命令标识 */
    uint     nPacketID;          /* 数据包ID */
    int      nPacketIndex;       /* 数据包包序 */
    int      nSrcPacketLen;      /* 原始数据包的总长度 */
}IMeUdpPackHeader; 

/* packet sub node */
typedef struct _IMeUdpSubPackNode
{
    byte     mRSTimes;           /* 该数据收发次数 */
    int      mOffset;            /* 该数据包偏移 */
    int      mDataLen;           /* 该数据包长度 */
    byte     mNeedSend;          /* 该数据包是否还需要继续发送 */
    uint     mLastSndTime;       /* 该数据包上一次被发送的时间 */
}IMeUdpSubPackNode;

typedef struct _IMeUdpPacket{
    int mRSCnt; /* send or receive sub-pack count */
    int mCnt;   /* total sub-pack count */
    int mSize;  /* packet total size */
    uint mID;    /* packet id */
    char*   mData;  /* packet data */
    IMeUdpSubPackNode* mNode;   /* packet sub-pack info */
	uint mLastRcvTime;	/* last receive packet time */
}IMeUdpPacket;

//udp user
typedef struct _IMeUdpUser
{
    uint mPacketIndexUsed;        /* count for packet id */
    IMeArray*    mListPacket;    /* sort by each user's packet id */
    socket_addr_t  mUserAddr;
	uint	mLastRcvTime;		 /* last receive packet time */
    /* 每包数据重发时间间隔 */
    uint m_nSndIntervalTime;
	/* 一次udp用户单次发送多少个数据包后停止发送,转而读取数据回应ack */
	int m_SndUserMaxPacksOneTime;
	/* udp发送的总数据包数 */
	int64 m_llSndUserPackCnt;
	/* udp接收一个数据包的超时时间 */
	int m_nRcvPacketTimeOut;
	/* 接收到的udp回应包 */
	int64 m_llRcvPacketAck;
	/* 当前缓存数据大小 */
	uint m_dwCurCacheSize;
}IMeUdpUser;



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


typedef struct _IMeCUdpSocket{
    
    IMeSocketUdp    vtbl;
    IMeMemory*  m_pMemory;
    
    /* socket interface */
    IMeSocket*  m_pSocket;
    IMeSocketDispatchGroup*	m_pSocketDispatchGroup;
    
    /* socket callback interface */
    OnSocketRcvDataCallBack m_pSocketDataCallBack;
    void*   m_upApp;
    
	OnUdpSocketCallCallBack	m_pSocketCallCallBack;
	void*	m_upAppCall;

    uint8   m_bRunning;
    IMeEvent*   m_evWaitFree;
    
    /* 网络mtu大小 */
    int m_nMtuSize;
      
	/* 本地最大发送带宽 */
	int m_nMaxSndBand;
	/* 每包数据最大发送次数 */
    int m_nSndMaxPacketTimes;
    
    /* 数据发送用户列表 */
    IMeArray*   m_listSndUser;
    IMeLock*    m_lockerSndUser;
    
    /* 数据接收用户列表 */
    IMeArray*   m_listRcvUser;
    IMeLock*    m_lockerRcvUser;
    
	/* 数据收发标识锁 */
	IMeLock*	m_lockerFlag;

    uint m_dispatchUser_extendParameter;
    
}IMeCUdpSocket;




//***************************************************************//
//***************************************************************//

IMeUdpPacket*   IMeUdpSendPacketCreate( IMeCUdpSocket* pUdpSocket, char* lpData, int Size, int Id, int nCmd )
{
    IMeUdpPacket* pUdpPacket = (IMeUdpPacket*)CMemoryAlloc( pUdpSocket->m_pMemory, sizeof(IMeUdpPacket) );
    if( pUdpPacket )
    {
        int i;
        IMeUdpPackHeader* pPackHeader;
        IMeUdpSubPackNode* pPackNode;
        
        pUdpPacket->mCnt = UdpPacketCount(Size,pUdpSocket->m_nMtuSize);
        pUdpPacket->mSize = Size + pUdpPacket->mCnt*UdpPackHeader_Size;
        pUdpPacket->mID = Id;
        pUdpPacket->mRSCnt = 0;

        pUdpPacket->mData = (char*)CMemoryAlloc( pUdpSocket->m_pMemory, pUdpPacket->mSize );
        if( !pUdpPacket->mData )
        {
            CMemoryFree( pUdpSocket->m_pMemory, pUdpPacket );
            return NULL;
        }

        pUdpPacket->mNode = (IMeUdpSubPackNode*)CMemoryAlloc( pUdpSocket->m_pMemory, UdpSubPackNode_Size*pUdpPacket->mCnt );
        if( !pUdpPacket->mNode )
        {
            CMemoryFree( pUdpSocket->m_pMemory, pUdpPacket->mData );
            CMemoryFree( pUdpSocket->m_pMemory, pUdpPacket );
            return NULL;
        }

        for( i=0; i<pUdpPacket->mCnt; i++ )
        {
            pPackHeader = (IMeUdpPackHeader*)&pUdpPacket->mData[i*pUdpSocket->m_nMtuSize];           
            pPackHeader->nCmdID = nCmd;
            pPackHeader->nPacketID = Id;
            pPackHeader->nPacketIndex = i;
            pPackHeader->nSrcPacketLen = Size;

            pPackNode = &pUdpPacket->mNode[i];
			pPackNode->mLastSndTime = 0;
            pPackNode->mRSTimes = 0;
            pPackNode->mNeedSend = TRUE;
            pPackNode->mDataLen = pUdpPacket->mSize > (i+1)*pUdpSocket->m_nMtuSize ? pUdpSocket->m_nMtuSize : pUdpPacket->mSize%pUdpSocket->m_nMtuSize; 
            pPackNode->mOffset = i*pUdpSocket->m_nMtuSize;
            memcpy( &pUdpPacket->mData[pPackNode->mOffset+UdpPackHeader_Size], &lpData[i*(pUdpSocket->m_nMtuSize-UdpPackHeader_Size)], pPackNode->mDataLen-UdpPackHeader_Size );
        }
    }

    return pUdpPacket;
}

IMeUdpPacket*   IMeUdpReceivePacketCreate( IMeCUdpSocket* pUdpSocket, int Size, int Id )
{
    IMeUdpPacket* pUdpPacket = (IMeUdpPacket*)CMemoryAlloc( pUdpSocket->m_pMemory, sizeof(IMeUdpPacket) );
    if( pUdpPacket )
    {
        int i;
        IMeUdpSubPackNode* pPackNode;

        pUdpPacket->mCnt = UdpPacketCount(Size,pUdpSocket->m_nMtuSize);
        pUdpPacket->mSize = Size;
        pUdpPacket->mID = Id;
        pUdpPacket->mRSCnt = 0;

        pUdpPacket->mData = (char*)CMemoryAlloc( pUdpSocket->m_pMemory, pUdpPacket->mSize );
        if( !pUdpPacket->mData )
        {
            CMemoryFree( pUdpSocket->m_pMemory, pUdpPacket );
            return NULL;
        }
        
        pUdpPacket->mNode = (IMeUdpSubPackNode*)CMemoryCalloc( pUdpSocket->m_pMemory, UdpSubPackNode_Size*pUdpPacket->mCnt );
        if( !pUdpPacket->mNode )
        {
            CMemoryFree( pUdpSocket->m_pMemory, pUdpPacket->mData );
            CMemoryFree( pUdpSocket->m_pMemory, pUdpPacket );
            return NULL;
        }

        for( i=0; i<pUdpPacket->mCnt; i++ )
        {   
            pPackNode = &pUdpPacket->mNode[i];
            pPackNode->mRSTimes = 0;
            pPackNode->mNeedSend = FALSE;
            pPackNode->mDataLen = (i!=pUdpPacket->mCnt-1) ? (pUdpSocket->m_nMtuSize-UdpPackHeader_Size) : pUdpPacket->mSize%(pUdpSocket->m_nMtuSize-UdpPackHeader_Size);
            pPackNode->mOffset = i*(pUdpSocket->m_nMtuSize-UdpPackHeader_Size);
        }
    }

    return  pUdpPacket;
}

void    IMeUdpPacketDestroy( IMeCUdpSocket* pUdpSocket, IMeUdpPacket* pUdpPacket )
{
    if( pUdpPacket )
    {
        if( pUdpPacket->mData )
            CMemoryFree( pUdpSocket->m_pMemory, pUdpPacket->mData );

        if( pUdpPacket->mNode )
            CMemoryFree( pUdpSocket->m_pMemory, pUdpPacket->mNode );

        CMemoryFree( pUdpSocket->m_pMemory, pUdpPacket );
    }
}

//sort receive or send user array 
int OnUdpRSUserArrayKeyCompare( uint64 nKey1 , uint64 nKey2 , void* parameter )
{
	IMeUdpUser* pUdpUser1 = (IMeUdpUser*)nKey1;
	IMeUdpUser* pUdpUser2 = (IMeUdpUser*)nKey2;

	if( pUdpUser1->mUserAddr.family == pUdpUser2->mUserAddr.family )
	{
        if( pUdpUser1->mUserAddr.family == AF_INET )
		    return memcmp( &pUdpUser1->mUserAddr.addr_ip4 , &pUdpUser2->mUserAddr.addr_ip4 , sizeof(struct sockaddr_in) );
        else
            return memcmp( &pUdpUser1->mUserAddr.addr_ip6 , &pUdpUser2->mUserAddr.addr_ip6 , sizeof(struct sockaddr_in6) );
	}

    return -1;
}

IMeUdpUser* IMeUdpUserCreate( IMeCUdpSocket* pUdpSocket , socket_addr_t* pUserAddr )
{
    IMeUdpUser* pUdpUser = (IMeUdpUser*)CMemoryAlloc( pUdpSocket->m_pMemory , sizeof(IMeUdpUser) );
    if( pUdpUser )
    {
        pUdpUser->mListPacket = CArrayCreate(SORT_INC);
        if( !pUdpUser->mListPacket )
		{
			DebugLogString( TRUE , "[IMeUdpUserCreate] create list packet failed!!" );
            CMemoryFree( pUdpSocket->m_pMemory, pUdpUser );
			return NULL;
		}

		pUdpUser->m_nSndIntervalTime = 10;
		pUdpUser->m_nRcvPacketTimeOut = 4 * pUdpUser->m_nSndIntervalTime * ( (pUdpSocket->m_nSndMaxPacketTimes==UdpPackWaitInfinite) ? UdpPackSendMaxTimes : pUdpSocket->m_nSndMaxPacketTimes);
		pUdpUser->m_dwCurCacheSize = 0;
		pUdpUser->m_SndUserMaxPacksOneTime = 20;

		pUdpUser->m_llRcvPacketAck = 0;
		pUdpUser->m_llSndUserPackCnt = 0;

        pUdpUser->mPacketIndexUsed = 0;
		pUdpUser->mLastRcvTime = 0;
        memcpy( &pUdpUser->mUserAddr , pUserAddr , sizeof(socket_addr_t) );
    }
    
    return pUdpUser;
}

void    IMeUdpUserDestroy( IMeCUdpSocket* pUdpSocket , IMeUdpUser* pUdpUser )
{	
	IMeUdpPacket* pUdpPacket;
	
	while( pUdpPacket = (IMeUdpPacket*)CArrayRemoveAt(pUdpUser->mListPacket,0) )
	{
		IMeUdpPacketDestroy( pUdpSocket , pUdpPacket );
	}
	CArrayDestroy( pUdpUser->mListPacket );

	CMemoryFree( pUdpSocket->m_pMemory , pUdpUser );
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int		IMeCUdpSocketCallBackDataPacket( IMeCUdpSocket* pUdpSocket , IMeUdpUser* pRcvUser , IMeUdpPacket* pDataPacket , socket_addr_t* rcv_addr )
{
	if( pDataPacket->mRSCnt >= pDataPacket->mCnt && pRcvUser->mPacketIndexUsed==pDataPacket->mID )
	{    
		CArrayRemoveAt( pRcvUser->mListPacket, 0 );
		
		/* call back application */
		if( pUdpSocket->m_pSocketDataCallBack )
		{
			pUdpSocket->m_pSocketDataCallBack( pDataPacket->mData , pDataPacket->mSize , rcv_addr , pUdpSocket->m_upApp );
		}
		
//		DebugLogString( TRUE , "[IMeCUdpSocketCallBackDataPacket] pack data [ID:%d SrcSize:%d Cnt:%d] success!" , pDataPacket->mID , pDataPacket->mSize , pDataPacket->mCnt );
		
		IMeUdpPacketDestroy( pUdpSocket , pDataPacket );
		
		//wait next packet
		++pRcvUser->mPacketIndexUsed;

		return 1;
	}

	return 0;
}

/* try generate full packet */
void     IMeCUdpSocketRcvPacketData( IMeCUdpSocket* pUdpSocket , char* pRcvBuffer, int nRcvBufferLen, socket_addr_t* rcv_addr )
{   
    IMeUdpUser* pRcvUser, stRcvUser; 
    IMeUdpPacket* pDataPacket = NULL;
    
    IMeUdpPackHeader* pUdpPackHeader = (IMeUdpPackHeader*)pRcvBuffer;

    CLock_Lock( pUdpSocket->m_lockerRcvUser );

    socket_addr_init_byaddr( &stRcvUser.mUserAddr, rcv_addr, rcv_addr->family );

    pRcvUser = (IMeUdpUser*)CArrayFindData( pUdpSocket->m_listRcvUser, (uint64)&stRcvUser );    
    if( pRcvUser )
    {
		if( pRcvUser->mPacketIndexUsed > (int)pUdpPackHeader->nPacketID )
		{
			DebugLogString( TRUE , "[IMeCUdpSocketRcvPacketData] discard udp packet data id:%d subId:%d" , pUdpPackHeader->nPacketID, pUdpPackHeader->nPacketIndex );
			CLock_Unlock( pUdpSocket->m_lockerRcvUser );			
			return;
		}

        pDataPacket = (IMeUdpPacket*)CArrayFindData( pRcvUser->mListPacket , pUdpPackHeader->nPacketID );
        if( !pDataPacket )
        {
            pDataPacket = IMeUdpReceivePacketCreate( pUdpSocket , pUdpPackHeader->nSrcPacketLen , pUdpPackHeader->nPacketID );
            if( pDataPacket )
            {
                CArrayAdd( pRcvUser->mListPacket , pDataPacket , pUdpPackHeader->nPacketID );
            }
        }

        if( pDataPacket )
        {
            IMeUdpSubPackNode* pPackNode = &pDataPacket->mNode[pUdpPackHeader->nPacketIndex];
			if( pPackNode->mDataLen != (int)(nRcvBufferLen-UdpPackHeader_Size) )
			{
				DebugLogString( TRUE , "[IMeCUdpSocketRcvPacketData] error udp data datalen:%d realLen:%d id:%d subId:%d" , pPackNode->mDataLen, nRcvBufferLen-UdpPackHeader_Size, pUdpPackHeader->nPacketID, pUdpPackHeader->nPacketIndex );
				CLock_Unlock( pUdpSocket->m_lockerRcvUser );
				return;
			}

            //first time receive this sub-packet
            if( pPackNode->mRSTimes==0 )
            {
                memcpy( &pDataPacket->mData[pPackNode->mOffset], pRcvBuffer+UdpPackHeader_Size, nRcvBufferLen-UdpPackHeader_Size );
                pDataPacket->mRSCnt++;
            }

			//last receive packet time
			pRcvUser->mLastRcvTime = pDataPacket->mLastRcvTime = IMeGetCurrentTime();
            pPackNode->mRSTimes++;

			//DebugLogString( TRUE , "[IMeCUdpSocketRcvSubPacket] Rcv Id:%d subID:%d data" , pUdpPackHeader->nPacketID, pUdpPackHeader->nPacketIndex );

			//check packet already receive over
			if( pDataPacket->mCnt==pDataPacket->mRSCnt )
			{
				if( pUdpSocket->m_nSndMaxPacketTimes != UdpPackWaitInfinite )
				{
					pRcvUser->mPacketIndexUsed = pDataPacket->mID;
					IMeCUdpSocketCallBackDataPacket( pUdpSocket, pRcvUser, pDataPacket, rcv_addr );
				}
				else
				{
					if( pRcvUser->mPacketIndexUsed != pDataPacket->mID )
						DebugLogString( TRUE , "[IMeCUdpSocketRcvSubPacket] wait ID:%d ==> cur ID:%d wait####" , pRcvUser->mPacketIndexUsed, pDataPacket->mID );

					pDataPacket = (IMeUdpPacket*)CArrayGetAt( pRcvUser->mListPacket, 0 );
					while( pDataPacket )
					{
						if( 0 == IMeCUdpSocketCallBackDataPacket( pUdpSocket, pRcvUser, pDataPacket, rcv_addr ) )
						{
							break;
						}
						pDataPacket = (IMeUdpPacket*)CArrayGetAt( pRcvUser->mListPacket, 0 );
					}
				}
			}
        }
    }
	else
	{
		DebugLogString( TRUE , "[IMeCUdpSocketRcvSubPacket] no find the user [ip:%s port:%d]!!" , socket_addr_ipaddr(rcv_addr) , socket_addr_ipport(rcv_addr) );
	}

    CLock_Unlock( pUdpSocket->m_lockerRcvUser );
}

/* rcv send packet ack */
void    IMeCUdpSocketRcvPacketAck( IMeCUdpSocket* pUdpSocket , IMeUdpPackHeader* pUdpPackHeader , socket_addr_t* rcv_addr )
{
    IMeUdpUser stSndUser , *pSndUser , *pRcvUser;    
    IMeUdpPacket* pDataPacket;

    socket_addr_init_byaddr( &stSndUser.mUserAddr, rcv_addr, rcv_addr->family );

    CLock_Lock( pUdpSocket->m_lockerSndUser );

    pSndUser = (IMeUdpUser*)CArrayFindData( pUdpSocket->m_listSndUser, (uint64)&stSndUser );
    if( pSndUser )
    {
		CLock_Lock( pUdpSocket->m_lockerFlag );
        pDataPacket = (IMeUdpPacket*)CArrayFindData( pSndUser->mListPacket, pUdpPackHeader->nPacketID );
        if( pDataPacket )
        {
            IMeUdpSubPackNode* pNode = &pDataPacket->mNode[pUdpPackHeader->nPacketIndex];
            if( pNode->mNeedSend )
            {
                pNode->mNeedSend = FALSE;
				++pDataPacket->mRSCnt;
            }
//			DebugLogString( TRUE , "[IMeCUdpSocketRcvPacketAck] Rcv id:%d subId:%d ack" , pUdpPackHeader->nPacketID, pUdpPackHeader->nPacketIndex );
        }
		CLock_Unlock( pUdpSocket->m_lockerFlag );
		//static rcv ack count
		++pSndUser->m_llRcvPacketAck;
		//update current rcv packet time
		pSndUser->mLastRcvTime = IMeGetCurrentTime();
    }

    CLock_Unlock( pUdpSocket->m_lockerSndUser );

	if( !pSndUser )
	{
        DebugLogString( TRUE , "[IMeCUdpSocketRcvPacketAck] no find the user [ip:%s port:%d]!!" , socket_addr_ipaddr(rcv_addr) , socket_addr_ipport(rcv_addr) );
	}

	//change local user rcv data time
	CLock_Lock( pUdpSocket->m_lockerRcvUser );
	pRcvUser = (IMeUdpUser*)CArrayFindData( pUdpSocket->m_listRcvUser, (uint64)&stSndUser );
	if( pRcvUser )	pRcvUser->mLastRcvTime = IMeGetCurrentTime();
	CLock_Unlock( pUdpSocket->m_lockerRcvUser );
}

void     IMeCUdpSocketRcvPacketUserCall( IMeCUdpSocket* pUdpSocket , char* pRcvBuffer, int nRcvBufferLen, socket_addr_t* rcv_addr )
{
	IMeUdpUser* pRcvUser, stUdpRcvUser;
	IMeUdpPackHeader* pUdpPackHeader = (IMeUdpPackHeader*)pRcvBuffer;
	
	/* reply ack info to remote first */
	pUdpPackHeader->nCmdID = UdpPackCmd_Ack;
	if( CSocketSendToByAddr( pUdpSocket->m_pSocket , rcv_addr , pRcvBuffer , UdpPackHeader_Size , 0 ) < UdpPackHeader_Size )
		DebugLogString( TRUE , "[IMeCUdpSocketRcvPacketUserCall] send udp packet Id:%d subId:%d ack failed!!" , pUdpPackHeader->nPacketID , pUdpPackHeader->nPacketIndex );
	
	socket_addr_init_byaddr( &stUdpRcvUser.mUserAddr, rcv_addr, rcv_addr->family );
	
	CLock_Lock( pUdpSocket->m_lockerRcvUser );
	pRcvUser = (IMeUdpUser*)CArrayFindData( pUdpSocket->m_listRcvUser, (uint64)&stUdpRcvUser );
	if( !pRcvUser )
	{
		pRcvUser = IMeUdpUserCreate( pUdpSocket , rcv_addr );
		if( pRcvUser )
		{
			pRcvUser->mPacketIndexUsed = 1;
			CArrayAdd( pUdpSocket->m_listRcvUser , pRcvUser , (uint64)pRcvUser );
		}
	}
	CLock_Unlock( pUdpSocket->m_lockerRcvUser );
	
	DebugLogString( TRUE , "[IMeCUdpSocketRcvPacketUserCall] receive client user [ip:%s port:%d] call server!!" , socket_addr_ipaddr(rcv_addr) , socket_addr_ipport(rcv_addr) );
	
	if( pUdpSocket->m_pSocketCallCallBack )
		pUdpSocket->m_pSocketCallCallBack( rcv_addr , pUdpSocket->m_upAppCall );
}

/* rcv socket packet from net-layer */
void     IMeCUdpSocketRcvDataProcess( IMeCUdpSocket* pUdpSocket , char* pRcvBuffer, int nRcvBufferLen, socket_addr_t* rcv_addr )
{
    IMeUdpPackHeader* pUdpPackHeader = (IMeUdpPackHeader*)pRcvBuffer;

    /* remove receive local send packet's ack */
    if( pUdpPackHeader->nCmdID == UdpPackCmd_Ack )
    {
        IMeCUdpSocketRcvPacketAck( pUdpSocket, pUdpPackHeader, rcv_addr );
    }
    /* receive remote data packet */
    else if( pUdpPackHeader->nCmdID == UdpPackCmd_Data )
    {
		/* reply ack info to remote first */
		pUdpPackHeader->nCmdID = UdpPackCmd_Ack;
        if( CSocketSendToByAddr( pUdpSocket->m_pSocket , rcv_addr , pRcvBuffer , UdpPackHeader_Size , 0 ) != UdpPackHeader_Size )
            DebugLogString( TRUE , "[IMeCUdpSocketRcvDataProcess] send udp packet Id:%d subId:%d ack failed!!" , pUdpPackHeader->nPacketID , pUdpPackHeader->nPacketIndex );

		/* process rcv data to up app */
		pUdpPackHeader->nCmdID = UdpPackCmd_Data;
        IMeCUdpSocketRcvPacketData( pUdpSocket, pRcvBuffer, nRcvBufferLen, rcv_addr );
    }
	/* receive client user call server(UDP NAT) */
	else if( pUdpPackHeader->nCmdID == UdpPackCmd_Call )
	{
		IMeCUdpSocketRcvPacketUserCall( pUdpSocket, pRcvBuffer, nRcvBufferLen, rcv_addr );
	}
}

/* check cache buffer in socket rcv buffer */
void	IMeCUdpSocketCheckCacheBuffer( IMeCUdpSocket* pUdpSocket )
{
	int i,j;
	uint dwCurTime = IMeGetCurrentTime();

	CLock_Lock( pUdpSocket->m_lockerRcvUser );
	
	for( i=CArrayGetSize(pUdpSocket->m_listRcvUser)-1; i>=0; --i )
	{
		IMeUdpUser* pUdpUser = (IMeUdpUser*)CArrayGetAt( pUdpSocket->m_listRcvUser,i);
		
		//check receive user's packet list
		for( j=CArrayGetSize(pUdpUser->mListPacket)-1; j>=0; --j )
		{
			IMeUdpPacket* pUdpPacket = (IMeUdpPacket*)CArrayGetAt( pUdpUser->mListPacket , j );
			if( (int)(dwCurTime - pUdpPacket->mLastRcvTime) > pUdpUser->m_nRcvPacketTimeOut 
				&& pUdpSocket->m_nSndMaxPacketTimes != UdpPackWaitInfinite )
			{
				CArrayRemoveAt( pUdpUser->mListPacket , j );
				DebugLogString( TRUE , "[IMeCUdpSocketCheckCacheBuffer] receive user a packet time out!!" );
				IMeUdpPacketDestroy( pUdpSocket , pUdpPacket );
			}
		}

		//check receive user's data
		if( (int)(dwCurTime - pUdpUser->mLastRcvTime) > UdpUserRcvData_TimeOut && pUdpUser->mLastRcvTime != 0 )
		{
			CArrayRemoveAt( pUdpSocket->m_listRcvUser , i );			
			DebugLogString( TRUE , "[IMeCUdpSocketCheckCacheBuffer] receive user data time out!!" );
			IMeUdpUserDestroy( pUdpSocket , pUdpUser );
		}
		else if( pUdpUser->mLastRcvTime == 0 )
		{
			pUdpUser->mLastRcvTime = dwCurTime;
		}
	}

	CLock_Unlock( pUdpSocket->m_lockerRcvUser );
}

void	IMeCUdpSocketAutoCheckUserNet( IMeCUdpSocket* pUdpSocket , IMeUdpUser* pUdpUser )
{
	double fSndSuccessRate = pUdpUser->m_llRcvPacketAck*1.0/pUdpUser->m_llSndUserPackCnt;

	// more than 80% ack from remote
	if( fSndSuccessRate >= 0.8 )
	{
		pUdpUser->m_SndUserMaxPacksOneTime += pUdpUser->m_SndUserMaxPacksOneTime/5;
		pUdpUser->m_nSndIntervalTime = DataPack_DefaultTimeInterval;
	}
	// more than 60% ack from remote
	else if( fSndSuccessRate >= 0.6  )
	{
		pUdpUser->m_SndUserMaxPacksOneTime += pUdpUser->m_SndUserMaxPacksOneTime/10;
		pUdpUser->m_nSndIntervalTime = DataPack_DefaultTimeInterval;
	}
	// less than 40% ack from remote
	else if( fSndSuccessRate >= 0.4 )
	{
		pUdpUser->m_SndUserMaxPacksOneTime = pUdpUser->m_SndUserMaxPacksOneTime*3/5;
		pUdpUser->m_nSndIntervalTime += 20;
	}
	// less than 20% ack from remote
	else if( fSndSuccessRate >= 0.2 )
	{
		pUdpUser->m_SndUserMaxPacksOneTime = pUdpUser->m_SndUserMaxPacksOneTime*2/5;
		pUdpUser->m_nSndIntervalTime += 30;
	}
	else
	{
		pUdpUser->m_SndUserMaxPacksOneTime = pUdpUser->m_SndUserMaxPacksOneTime/5;
		pUdpUser->m_nSndIntervalTime += 50;
	}

	//minimum send packet count
	if( pUdpUser->m_SndUserMaxPacksOneTime < 5 )
		pUdpUser->m_SndUserMaxPacksOneTime = 5;
	//net send band width control
	else if( pUdpSocket->m_nMaxSndBand > 0 && pUdpUser->m_SndUserMaxPacksOneTime*pUdpSocket->m_nMtuSize > pUdpSocket->m_nMaxSndBand )
		pUdpUser->m_SndUserMaxPacksOneTime = pUdpSocket->m_nMaxSndBand/pUdpSocket->m_nMtuSize;
	//no set send band control, set max 200KB each times
	else if( pUdpSocket->m_nMaxSndBand <= 0 && pUdpUser->m_SndUserMaxPacksOneTime > DataPack_MaxSndCntOneTime )	
		pUdpUser->m_SndUserMaxPacksOneTime = DataPack_MaxSndCntOneTime;
	
	//modify rcv sub packet time out value
	pUdpUser->m_nRcvPacketTimeOut = 4*pUdpUser->m_nSndIntervalTime*UdpPackSendMaxTimes;

	//DebugLogString( TRUE, "[IMeCUdpSocketAutoCheckUserNet] intervalTime:%d maxPacksOneTime:%d fSndSuccessRate:%lf!!" , pUdpUser->m_nSndIntervalTime, pUdpUser->m_SndUserMaxPacksOneTime, fSndSuccessRate );
}

void	IMeCUdpSocketAutoCheckNet( IMeCUdpSocket* pUdpSocket )
{	
    IMeUdpUser* pRSUser;
    int i = 0;

    CLock_Lock( pUdpSocket->m_lockerSndUser );
    pRSUser = (IMeUdpUser*)CArrayGetAt(pUdpSocket->m_listSndUser,i++);
    CLock_Unlock( pUdpSocket->m_lockerSndUser );
    
    while( pRSUser )
    {	
		IMeCUdpSocketAutoCheckUserNet( pUdpSocket, pRSUser );

        CLock_Lock( pUdpSocket->m_lockerSndUser );
        pRSUser = (IMeUdpUser*)CArrayGetAt(pUdpSocket->m_listSndUser,i++);
        CLock_Unlock( pUdpSocket->m_lockerSndUser );
    }
}

void    IMeCUdpSocketRead( IMeCUdpSocket* pUdpSocket )
{   
    int nRes;
    
	socket_addr_t rcv_addr;

	char szBuffer[5*1024];
	IMeUdpPackHeader* pPackHeader = (IMeUdpPackHeader*)szBuffer;

    while( pUdpSocket->m_bRunning )
    {    
		//read packet
		memset( &rcv_addr , 0 , sizeof(socket_addr_t) );
		nRes = CSocketRecvFromByAddr( pUdpSocket->m_pSocket , szBuffer , pUdpSocket->m_nMtuSize , &rcv_addr , 0 );
		if( nRes <= 0 )		break;

		//check packet is valid ?
		if( nRes < UdpPackHeader_Size || nRes > pUdpSocket->m_nMtuSize || pPackHeader->nSrcPacketLen > DataPack_MaxLimitSize )
		{
			DebugLogString( TRUE , "[IMeCUdpSocketRead] error udp data nRes:%d headerSize:%d mtuSize:%d srcLen:%d!!" , nRes, UdpPackHeader_Size, pUdpSocket->m_nMtuSize, pPackHeader->nSrcPacketLen );
			continue;
		}
        
        //process data
        IMeCUdpSocketRcvDataProcess( pUdpSocket, szBuffer, nRes, &rcv_addr );

		//check free no received over cache buffer data
		IMeCUdpSocketCheckCacheBuffer( pUdpSocket );
    }
	
	//auto check net
	IMeCUdpSocketAutoCheckNet( pUdpSocket );
}

//send packet to remote
uint8 IMeCUdpSocketWrite( IMeCUdpSocket* pUdpSocket , byte* lpData , int nLen , socket_addr_t* addr_t )
{
    int nSendLen = CSocketSendToByAddr( pUdpSocket->m_pSocket , addr_t , lpData , nLen , 0 );
	//if send data len less than packet size , resend this packet next time
    return nSendLen==nLen;
}

uint8   IMeCUdpSocketWriteDataPacket( IMeCUdpSocket* pUdpSocket , IMeUdpPacket* pDataPacket , IMeUdpUser* pRSUser , int* nCurSndPackCnt )
{
    uint8 bCanSndOk = TRUE;
    int i;
	IMeUdpSubPackNode* pSubPackNode;
	uint dwCurTime;
	
	int nRealSndCnt = 0;
	int nPacketCnt = UdpPackMin(pRSUser->m_SndUserMaxPacksOneTime , pDataPacket->mCnt);

    for( i=0; i<pDataPacket->mCnt; i++ )
    {
		dwCurTime = IMeGetCurrentTime();

		CLock_Lock( pUdpSocket->m_lockerFlag );
        
		pSubPackNode = &pDataPacket->mNode[i];
        if( pSubPackNode->mNeedSend && (dwCurTime-pSubPackNode->mLastSndTime) >= pRSUser->m_nSndIntervalTime )
        {
			++nRealSndCnt;
			--nPacketCnt;
            bCanSndOk = IMeCUdpSocketWrite( pUdpSocket , &pDataPacket->mData[pSubPackNode->mOffset] , pSubPackNode->mDataLen, &pRSUser->mUserAddr );
            if( bCanSndOk )
            {
                pSubPackNode->mRSTimes++;
                if( pUdpSocket->m_nSndMaxPacketTimes != UdpPackWaitInfinite && pSubPackNode->mRSTimes==pUdpSocket->m_nSndMaxPacketTimes )
                {
                    pSubPackNode->mNeedSend = FALSE;
					++pDataPacket->mRSCnt;
                }
                pSubPackNode->mLastSndTime = dwCurTime;
                
				if( pSubPackNode->mRSTimes > 1 )
					DebugLogString( TRUE , "[IMeCUdpSocketWriteDataPacket] id:%d subId:%d cnt:%d sndTime:%d sndInterval:%d sndUserMaxPacketOneTime:%d!" , pDataPacket->mID, i, pDataPacket->mCnt, pSubPackNode->mRSTimes, pRSUser->m_nSndIntervalTime, pRSUser->m_SndUserMaxPacksOneTime );
            }
        }   
		else
		{
//			DebugLogString( TRUE , "[IMeCUdpSocketWriteDataPacket] id:%d subId:%d needSend:%d curPacketTimeInterval:%d socketTimeInterval:%d" , pDataPacket->mID, i, pSubPackNode->mNeedSend, IMeGetCurrentTime()-pSubPackNode->mLastSndTime, pUdpSocket->m_nSndIntervalTime );
		}

		CLock_Unlock( pUdpSocket->m_lockerFlag );
		
		if( nPacketCnt==0 )	break;
    }
    
	*nCurSndPackCnt = *nCurSndPackCnt + nRealSndCnt;

    return bCanSndOk;
}

uint8   IMeCUdpSocketWriteUserPacket( IMeCUdpSocket* pUdpSocket , IMeUdpUser* pRSUser , int* nRealWriteCnt )
{
    uint8 bCanSndOk = TRUE;
    IMeUdpPacket* pDataPacket;
	int i = 0;
	int nCurSendPackCnt = 0;

    CLock_Lock( pUdpSocket->m_lockerSndUser );
    pDataPacket = (IMeUdpPacket*)CArrayGetAt( pRSUser->mListPacket , i++ );
    CLock_Unlock( pUdpSocket->m_lockerSndUser );

    while( pDataPacket && bCanSndOk )
    {
        bCanSndOk = IMeCUdpSocketWriteDataPacket( pUdpSocket , pDataPacket , pRSUser , &nCurSendPackCnt );
		
//		DebugLogString( TRUE , "[IMeCUdpSocketWriteUserPacket] write id:%d" , pDataPacket->mID );

		if( pDataPacket->mRSCnt >= pDataPacket->mCnt )
        {
//			DebugLogString( TRUE , "[IMeCUdpSocketWriteUserPacket] delete id:%d" , pDataPacket->mID );

            CLock_Lock( pUdpSocket->m_lockerSndUser );
            CArrayRemoveAt( pRSUser->mListPacket, i-1 );
            CLock_Unlock( pUdpSocket->m_lockerSndUser );

			pRSUser->m_dwCurCacheSize -= pDataPacket->mSize;

            IMeUdpPacketDestroy( pUdpSocket , pDataPacket );
			--i;
        }
        
		//if send packets more than send max packets, try next time
		if( nCurSendPackCnt >= pRSUser->m_SndUserMaxPacksOneTime )
			break;

        CLock_Lock( pUdpSocket->m_lockerSndUser );
        pDataPacket = (IMeUdpPacket*)CArrayGetAt( pRSUser->mListPacket , i++ );
        CLock_Unlock( pUdpSocket->m_lockerSndUser );
    }

	*nRealWriteCnt = nCurSendPackCnt;

    return bCanSndOk;
}

/* +++++++++++++++++++++dispatch user base interface++++++++++++++++++ */
/* +++++++++++++++++++++dispatch user base interface++++++++++++++++++ */
/* +++++++++++++++++++++dispatch user base interface++++++++++++++++++ */
void    IMeUdpSocketDispatchUserRemove( IMeSocketDispatchUser* pSocketDispatchUser )
{
    IMeCUdpSocket* pSocketUdp = (IMeCUdpSocket*)pSocketDispatchUser;
    
    if( pSocketUdp->m_evWaitFree )
        CEventSet( pSocketUdp->m_evWaitFree );
    
    DebugLogString( TRUE , "[IMeUdpSocketDispatchUserRemove] socket remove from dispatch group!!" );
}

uint8	IMeCUdpSocketHasSendPackets( IMeCUdpSocket* pSocketUdp )
{
	int i = 0;
	IMeUdpUser* pRSUser;
	uint8 bHasSndPacks = FALSE;
	int nLeftPacketCnt = 0;
	uint dwCurTime = IMeGetCurrentTime();

	CLock_Lock( pSocketUdp->m_lockerSndUser );
    pRSUser = (IMeUdpUser*)CArrayGetAt(pSocketUdp->m_listSndUser,i++);
    CLock_Unlock( pSocketUdp->m_lockerSndUser );
    
    while( pRSUser )
    {
		nLeftPacketCnt = CArrayGetSize( pRSUser->mListPacket );
		if( nLeftPacketCnt > 0 )
		{
			bHasSndPacks = TRUE;
			break;
		}

        CLock_Lock( pSocketUdp->m_lockerSndUser );
        pRSUser = (IMeUdpUser*)CArrayGetAt(pSocketUdp->m_listSndUser,i++);
        CLock_Unlock( pSocketUdp->m_lockerSndUser );
    }

	return bHasSndPacks;
}

void	IMeUdpSocketDispatchUserSend( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCUdpSocket* pSocketUdp = (IMeCUdpSocket*)pSocketDispatchUser;
	
    uint8 bCanSndOk = TRUE;
    IMeUdpUser* pRSUser;
    int i = 0;

	int nCurSendCnt = 0;

    CLock_Lock( pSocketUdp->m_lockerSndUser );
    pRSUser = (IMeUdpUser*)CArrayGetAt(pSocketUdp->m_listSndUser,i++);
    CLock_Unlock( pSocketUdp->m_lockerSndUser );
    
    while( pRSUser && bCanSndOk )
    {
        bCanSndOk = IMeCUdpSocketWriteUserPacket( pSocketUdp , pRSUser , &nCurSendCnt );
		
		pRSUser->m_llSndUserPackCnt += nCurSendCnt;

//		DebugLogString( TRUE , "[IMeUdpSocketDispatchUserSend] CNT:%d totalCnt:%d!" , nCurSendCnt , pRSUser->m_llSndUserPackCnt );

        CLock_Lock( pSocketUdp->m_lockerSndUser );
        pRSUser = (IMeUdpUser*)CArrayGetAt(pSocketUdp->m_listSndUser,i++);
        CLock_Unlock( pSocketUdp->m_lockerSndUser );
    }
	
	
	CLock_Lock( pSocketUdp->m_lockerFlag );

	if( !IMeCUdpSocketHasSendPackets(pSocketUdp) )
	{
		DebugLogString( TRUE, "[IMeUdpSocketDispatchUserSend] no packet need send !!" );
		CSocketDispatchGroupRemoveEvent( pSocketUdp->m_pSocketDispatchGroup , (IMeSocketDispatchUser*)pSocketUdp , SEvent_Write );
	}
	
	CLock_Unlock( pSocketUdp->m_lockerFlag );
}

void	IMeUdpSocketDispatchUserRcv( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCUdpSocket* pSocketUdp = (IMeCUdpSocket*)pSocketDispatchUser;
	IMeCUdpSocketRead( pSocketUdp );
}

void	IMeUdpSocketDispatchUserException( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCUdpSocket* pSocketUdp = (IMeCUdpSocket*)pSocketDispatchUser;
	DebugLogString( TRUE , "[IMeUdpSocketDispatchUserException] socket exception event occur!!" );
}

void	IMeUdpSocketDispatchUserSetExtendParameter( IMeSocketDispatchUser* pSocketDispatchUser , uint extendParameter )
{
	IMeCUdpSocket* pSocketUdp = (IMeCUdpSocket*)pSocketDispatchUser;
	pSocketUdp->m_dispatchUser_extendParameter = extendParameter;
}

uint	IMeUdpSocketDispatchUserGetExtendParameter( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCUdpSocket* pSocketUdp = (IMeCUdpSocket*)pSocketDispatchUser;
	return pSocketUdp->m_dispatchUser_extendParameter;
}

IMeSocket*	IMeUdpSocketDispatchUserGetSocket( IMeSocketDispatchUser* pSocketDispatchUser )
{
	IMeCUdpSocket* pSocketUdp = (IMeCUdpSocket*)pSocketDispatchUser;
	return pSocketUdp->m_pSocket;
}

int		IMeCSocketUdpSendInternel( IMeSocketUdp* pISocketUdp , char* lpData , int nLen , socket_addr_t* addr , int nCmd )
{
	IMeCUdpSocket* pUdpSocket = (IMeCUdpSocket*)pISocketUdp;

	IMeUdpPacket* pUdpPacket = NULL;
	IMeUdpUser *pUdpUser, stUdpUser;
	int nPacketCacheCnt = 0;

	socket_addr_init_byaddr( &stUdpUser.mUserAddr, addr, addr->family );

    CLock_Lock( pUdpSocket->m_lockerSndUser );

    pUdpUser = (IMeUdpUser*)CArrayFindData( pUdpSocket->m_listSndUser, (uint64)&stUdpUser );
    if( !pUdpUser )
    {
        pUdpUser = IMeUdpUserCreate( pUdpSocket, addr );
        if( pUdpUser )  CArrayAdd( pUdpSocket->m_listSndUser, pUdpUser, (uint64)pUdpUser );
    }

    if( pUdpUser )
    {
		if( pUdpUser->m_dwCurCacheSize >= DataPack_MaxCacheSize )
		{
			IMeSleep(1);
			CLock_Unlock( pUdpSocket->m_lockerSndUser );
			return 0;
		}

        pUdpPacket = IMeUdpSendPacketCreate( pUdpSocket, lpData, nLen, pUdpUser->mPacketIndexUsed, nCmd );
        if( pUdpPacket )
		{
			pUdpUser->m_dwCurCacheSize += pUdpPacket->mSize;
			CArrayAdd( pUdpUser->mListPacket, pUdpPacket, pUdpPacket->mID );
			++pUdpUser->mPacketIndexUsed;
		}
    }

    CLock_Unlock( pUdpSocket->m_lockerSndUser );


	//add dispatch group event
	CLock_Lock( pUdpSocket->m_lockerFlag );

	if( pUdpPacket )
	{
		//DebugLogString( TRUE , "[IMeCSocketUdpSendInternel] ID:%d" , pUdpPacket->mID );
		CSocketDispatchGroupAddEvent( pUdpSocket->m_pSocketDispatchGroup , (IMeSocketDispatchUser*)pUdpSocket , SEvent_Write );
	}
	
	CLock_Unlock( pUdpSocket->m_lockerFlag );

    return pUdpPacket ? nLen : 0;
}

int		IMeCSocketUdpSendByAddr( IMeSocketUdp* pISocketUdp, char* lpData , int nLen , socket_addr_t* addr )
{
	IMeCUdpSocket* pUdpSocket = (IMeCUdpSocket*)pISocketUdp;

	if( !pUdpSocket->m_bRunning )   return 0;
	
	return IMeCSocketUdpSendInternel( pISocketUdp, lpData, nLen, addr, UdpPackCmd_Data );
}

/* send data interface for out user use */
int     IMeCSocketUdpSend( IMeSocketUdp* pISocketUdp , char* lpData , int nLen , char* ipStr , ushort port , ushort family )
{
    IMeCUdpSocket* pUdpSocket = (IMeCUdpSocket*)pISocketUdp;
    
    socket_addr_t snd_addr;    

    socket_addr_init( &snd_addr , ipStr , port , family );

	return IMeCSocketUdpSendByAddr( pISocketUdp, lpData, nLen, &snd_addr );
}

void    IMeCSocketUdpSetOpt( IMeSocketUdp* pISocketUdp, int opt, int optValue )
{
    IMeCUdpSocket* pSocketUdp = (IMeCUdpSocket*)pISocketUdp;
    if( pSocketUdp->m_pSocket && opt != SOCKET_SO_NONBLOCK )
        CSocketSetOpt( pSocketUdp->m_pSocket, opt, optValue );
}

void	IMeCSocketUdpEnableAsTcp( IMeSocketUdp* pISocketUdp, uint8 bEnable )
{
	IMeCUdpSocket* pSocketUdp = (IMeCUdpSocket*)pISocketUdp;
	if( bEnable )
		pSocketUdp->m_nSndMaxPacketTimes = UdpPackWaitInfinite;
	else
		pSocketUdp->m_nSndMaxPacketTimes = UdpPackSendMaxTimes;
}

void	IMeCSocketUdpCallUser( IMeSocketUdp* pISocketUdp, char* ipStr, ushort port, ushort family )
{
	IMeCUdpSocket* pSocketUdp = (IMeCUdpSocket*)pISocketUdp;
	if( pSocketUdp->m_pSocket )
	{
		IMeUdpPackHeader stUdpPackHeader;
		IMeUdpUser* pRcvUser, stUser;

		socket_addr_init( &stUser.mUserAddr , ipStr , port , family );
		
		//create rcv user
		CLock_Lock( pSocketUdp->m_lockerRcvUser );
		pRcvUser = (IMeUdpUser*)CArrayFindData( pSocketUdp->m_listRcvUser, (uint64)&stUser );
		if( !pRcvUser )
		{
			pRcvUser = IMeUdpUserCreate( pSocketUdp, &stUser.mUserAddr );
			if( pRcvUser )
				CArrayAdd( pSocketUdp->m_listRcvUser, pRcvUser, (uint64)pRcvUser );
		}
		CLock_Unlock( pSocketUdp->m_lockerRcvUser );

		//send call cmd to remote
		stUdpPackHeader.nCmdID = UdpPackCmd_Call;
		stUdpPackHeader.nSrcPacketLen = sizeof(IMeUdpPackHeader);
		IMeCSocketUdpSendInternel( pISocketUdp, (char*)&stUdpPackHeader, UdpPackHeader_Size, &stUser.mUserAddr, UdpPackCmd_Call );

		DebugLogString( TRUE , "[IMeCSocketUdpCallUser] IpStr:%s port:%d!!" , ipStr, port );
	}
}

uint8		IMeCSocketUdpBind( IMeSocketUdp* pISocketUdp )
{
    IMeCUdpSocket* pSocketUdp = (IMeCUdpSocket*)pISocketUdp;
	uint8 bBindRes = FALSE;
    
	if( pSocketUdp->m_pSocket )
		bBindRes = CSocketBind( pSocketUdp->m_pSocket );

	return bBindRes;
}

void		IMeCSocketUdpSetClientCallBack( IMeSocketUdp* pISocketUdp, OnUdpSocketCallCallBack cb, void* upApp )
{
	IMeCUdpSocket* pSocketUdp = (IMeCUdpSocket*)pISocketUdp;
	pSocketUdp->m_pSocketCallCallBack = cb;
	pSocketUdp->m_upAppCall = upApp;
}

void    IMeCSocketUdpDestroy( IMeSocketUdp* pISocketUdp )
{
    IMeCUdpSocket* pUdpSocket = (IMeCUdpSocket*)pISocketUdp;
    IMeUdpUser* pUdpUser;
    if( pUdpSocket )
    {
        pUdpSocket->m_evWaitFree = CEventCreate(TRUE,FALSE);
        CEventReset( pUdpSocket->m_evWaitFree );

        CSocketDispatchGroupRemoveUser( pUdpSocket->m_pSocketDispatchGroup , (IMeSocketDispatchUser*)pUdpSocket );
        pUdpSocket->m_bRunning = FALSE;

        while( !CEventWait(pUdpSocket->m_evWaitFree,10) );
        CEventDestroy( pUdpSocket->m_evWaitFree );

        /* rcv user */
        while( pUdpUser=(IMeUdpUser*)CArrayRemoveAt(pUdpSocket->m_listRcvUser,0) )
            IMeUdpUserDestroy( pUdpSocket, pUdpUser );
        CArrayDestroy( pUdpSocket->m_listRcvUser );

        /* snd user */
        while( pUdpUser=(IMeUdpUser*)CArrayRemoveAt(pUdpSocket->m_listSndUser,0) )
            IMeUdpUserDestroy( pUdpSocket, pUdpUser );
        CArrayDestroy( pUdpSocket->m_listSndUser );
        
        CLock_Destroy( pUdpSocket->m_lockerRcvUser );
        CLock_Destroy( pUdpSocket->m_lockerSndUser );
        CLock_Destroy( pUdpSocket->m_lockerFlag );

        CMemoryDestroy( pUdpSocket->m_pMemory );

        free(pUdpSocket);    
    }
}

IME_EXTERN_C IMeSocketUdp*   IMeSocketUdpCreate( IMeSocketDispatchGroup* pSocketDispatchGroup , IMeSocket* pSocket , OnSocketRcvDataCallBack dataCB , void* upApp )
{
    IMeCUdpSocket* pUdpSocket = (IMeCUdpSocket*)calloc(1,sizeof(IMeCUdpSocket));
    if( pUdpSocket )
    {
		pUdpSocket->m_bRunning = TRUE;
		
        pUdpSocket->m_pMemory = CMemoryCreate(IMeMemory_Default);

		pUdpSocket->m_lockerFlag = CLock_Create();

        pUdpSocket->m_listRcvUser = CArrayCreate(SORT_INC);
        pUdpSocket->m_lockerRcvUser = CLock_Create();
        CArraySetCompare( pUdpSocket->m_listRcvUser, OnUdpRSUserArrayKeyCompare, pUdpSocket );

        pUdpSocket->m_listSndUser = CArrayCreate(SORT_INC);
        pUdpSocket->m_lockerSndUser = CLock_Create();
        CArraySetCompare( pUdpSocket->m_listSndUser, OnUdpRSUserArrayKeyCompare, pUdpSocket );

        pUdpSocket->m_nSndMaxPacketTimes = UdpPackSendMaxTimes;		
        pUdpSocket->m_nMtuSize = ETHERNET_UDP_MTU_SIZE;
		pUdpSocket->m_nMaxSndBand = 0;

        pUdpSocket->m_pSocket = pSocket;
		pUdpSocket->m_pSocketDispatchGroup = pSocketDispatchGroup;

        pUdpSocket->m_pSocketDataCallBack = dataCB;
        pUdpSocket->m_upApp = upApp;

        ((IMeSocketUdp*)pUdpSocket)->m_pDestroy = IMeCSocketUdpDestroy;
        ((IMeSocketUdp*)pUdpSocket)->m_pSend = IMeCSocketUdpSend;
		((IMeSocketUdp*)pUdpSocket)->m_pSendByAddr = IMeCSocketUdpSendByAddr;
		((IMeSocketUdp*)pUdpSocket)->m_pBind = IMeCSocketUdpBind;
        ((IMeSocketUdp*)pUdpSocket)->m_pSetOpt = IMeCSocketUdpSetOpt;
		((IMeSocketUdp*)pUdpSocket)->m_pEnableAsTcp = IMeCSocketUdpEnableAsTcp;
		((IMeSocketUdp*)pUdpSocket)->m_pCallUser = IMeCSocketUdpCallUser;
		((IMeSocketUdp*)pUdpSocket)->m_pSetClientCallBack = IMeCSocketUdpSetClientCallBack;


		pUdpSocket->vtbl.vtbl.m_pSetExtendParameter = IMeUdpSocketDispatchUserSetExtendParameter;
		pUdpSocket->vtbl.vtbl.m_pGetExtendParameter = IMeUdpSocketDispatchUserGetExtendParameter;
		pUdpSocket->vtbl.vtbl.m_pGetSocket = IMeUdpSocketDispatchUserGetSocket;
		pUdpSocket->vtbl.vtbl.m_pSend = IMeUdpSocketDispatchUserSend;
		pUdpSocket->vtbl.vtbl.m_pRcv = IMeUdpSocketDispatchUserRcv;
		pUdpSocket->vtbl.vtbl.m_pException = IMeUdpSocketDispatchUserException;
        pUdpSocket->vtbl.vtbl.m_pRemove = IMeUdpSocketDispatchUserRemove;

    }

    return (IMeSocketUdp*)pUdpSocket;
}
