// #include    "../include/include.h"
// 
// void    IMeDataPackFreeList( IMeMemory* pMemory , IMeList* pList , uint8 bSnd )
// {
//     IMeSndPacket* pSndPacket;
// 	IMeRcvPacket* pRcvPacket;
// 	if( bSnd )
// 	{
// 		while( pSndPacket=(IMeSndPacket*)CListRemoveHead(pList) ) 
// 			IMeSndPacketDestroy(pMemory , pSndPacket);
// 	}
// 	else
// 	{
// 		while( pRcvPacket=(IMeRcvPacket*)CListRemoveHead(pList) ) 
// 			IMeRcvPacketDestroy(pMemory , pRcvPacket);
// 	}
// }
// 
// void    IMeDataPackInitHeader( IMePackHeader* pPackHeader , uint nCmdID , uint nPacketID , int nIndex , int nSrcPacketLen , int nSubPacketCounts , int nDataLen )
// {
//     pPackHeader->nCmdID = nCmdID;
//     pPackHeader->nDataLen = nDataLen;
//     pPackHeader->nPacketID = nPacketID;
//     pPackHeader->nPacketIndex = nIndex;
//     pPackHeader->nSrcPacketLen = nSrcPacketLen;
//     pPackHeader->nSubPacketCounts = nSubPacketCounts;
// }
// 
// /* 数据拆包 */
// uint8   IMeDataUnPackInternal( IMeMemory* pMemory , IMeList* pList , uint nCmdID , uint nPacketID , int nPacketMaxSize , char* lpData , int nLen )
// {
//     uint8 bRes = TRUE;
//     int nPacketSize =0 , nPacketIndex = 0;
//     int nSubPacketCounts = (nLen+nPacketMaxSize-1)/nPacketMaxSize;
//     int nFullSize = nPacketMaxSize-sizeof(IMePackHeader);
//     IMeSndPacket* pSndPacket;
//     while( nLen )
//     {
//         nPacketSize = (nLen>nFullSize?nFullSize:nLen);
// 
//         pSndPacket = IMeSndPacketCreate(pMemory, lpData , nPacketSize+sizeof(IMePackHeader));
//         if( !pSndPacket )
//         {
//             DebugLogString( TRUE , "[IMeDataUnPack] create send socket packet failed!!" );
//             IMeDataPackFreeList( pMemory , pList , TRUE );
//             bRes = FALSE;
//             break;
//         }
//         
//         IMeDataPackInitHeader( (IMePackHeader*)(pSndPacket->lpData) , nCmdID , nPacketID , nPacketIndex , nLen , nSubPacketCounts , nPacketSize );
//         memcpy( pSndPacket->lpData+sizeof(IMePackHeader) , lpData , nPacketSize );
//         CListInsert( pList , pSndPacket , nPacketIndex );
// 
//         lpData += nPacketSize;
//         nLen -= nPacketSize;
//         nPacketIndex++;
//     }
// 
//     return bRes;
// }
// 
// /* 数据解包 */
// uint8   IMeDataPackInternal( IMeMemory* pMemory , IMeArray* pList , char** ppData , int* nLen )
// {
//     uint8 bRes = TRUE;
// 
//     IMePackHeader* pHeader;
//     IMeRcvPacket* pRcvPacket;
// 
//     int nSrcPacketSize;
//     
// 	char* pBuffer;
//     int nBufferLen = 0;
// 
//     if( !CArrayGetSize(pList) ) return FALSE;
// 
//     pRcvPacket = (IMeRcvPacket*)CArrayGetAt(pList,0);
// 
//     pHeader = (IMePackHeader*)pRcvPacket->lpData;
//     nSrcPacketSize = pHeader->nSrcPacketLen;
// 
//     pBuffer = (char*)CMemoryAlloc(pMemory,nSrcPacketSize);
//     if( !pBuffer )
//     {
//         DebugLogString( TRUE , "[IMeDataPackInternal] alloc buffer %d size failed!!" , nSrcPacketSize );
//         return FALSE; 
//     }
//     
//     while( pRcvPacket )
//     {
// 		pHeader = (IMePackHeader*)pRcvPacket->lpData;
// 
//         if( nBufferLen>=nSrcPacketSize )
//         {
//             DebugLogString( TRUE , "[IMeDataPackInternal] data error larger than source packet size:%d!!" , nSrcPacketSize );
//             bRes = FALSE;
//             break;
//         }
//         memcpy( pBuffer , pRcvPacket->lpData+sizeof(IMePackHeader) , pHeader->nDataLen );
//         nBufferLen += pHeader->nDataLen;
//         pBuffer += pHeader->nDataLen;
// 
//         pRcvPacket = (IMeRcvPacket*)CListGetNext(pList,&pos);
//     }
//     
//     if( !bRes && pBuffer )
//     {
//         CMemoryFree( pMemory , pBuffer );
//     }
// 	else
// 	{
// 		*ppData = pBuffer;
// 		*nLen = nSrcPacketSize;
// 	}
// 
//     return bRes;
// }
// 
// /* release */
// void    IMeDataDestroyPacket( IMeMemory* pMemory , IMeDataPacket* pDataPacket , uint8 bSnd )
// {
//     IMeDataPackFreeList( pMemory , pDataPacket->mListSubPacket , bSnd );
//     CMemoryFree( pMemory , (char*)pDataPacket );
// }
// 
// /* create */
// IMeDataPacket*  IMeDataCreatePacket( IMeMemory* pMemory , uint nPacketID )
// {
//     uint8 bCreateOk = FALSE;
//     IMeDataPacket* pDataPacket = (IMeDataPacket*)CMemoryAlloc(pMemory,sizeof(IMeDataPacket));
//     while( pDataPacket )
//     {
//         pDataPacket->mListSubPacket = CArrayCreate(SORT_INC);
//         if( !pDataPacket->mListSubPacket )    break;
//         pDataPacket->nPacketID = nPacketID;
//         bCreateOk = TRUE;
//         break;
//     }
//     if( !bCreateOk && pDataPacket )
//     {
//         if( pDataPacket->mListSubPacket )
//             CArrayDestroy( pDataPacket->mListSubPacket );
//         CMemoryFree( pMemory , pDataPacket );
//         pDataPacket = NULL;
//     }
//     
//     return pDataPacket;
// }
// 
// IMeDataPacket*  IMeDataUnPack( IMeMemory* pMemory , uint nPacketID , int nPacketMaxSize , char* lpData , int nLen )
// {
//     IMeDataPacket* pDataPacket = IMeDataCreatePacket( pMemory , nPacketID );
//     if( pDataPacket )
//     {
//         uint8 bRes = IMeDataUnPackInternal( pMemory , pDataPacket->mListSubPacket , PackCmd_Data , nPacketID , nPacketMaxSize , lpData , nLen );
//         if( !bRes )
//         {
//             IMeDataDestroyPacket( pMemory , pDataPacket , TRUE );
//             pDataPacket = NULL;
//         }
//     }
//     
//     return pDataPacket;
// }
// 
// uint8   IMeDataPack( IMeMemory* pMemory , IMeDataPacket* pDataPacket , char** ppData , int* nLen )
// {
//     return IMeDataPackInternal( pMemory , pDataPacket->mListSubPacket , ppData , nLen );
// }
