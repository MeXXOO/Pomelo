#include "netSrcData.h"
#include "server.h"

IME_EXTERN_C	IMeNetSrcData*	IMeNetSrcDataCreate( int nTransferProtocolType, char* lpData , int nLen , void* pDataSrc )
{
	IMeNetSrcData* pNetSrcData = (IMeNetSrcData*)calloc( 1 , sizeof(IMeNetSrcData) );
	while( pNetSrcData )
	{
		if( NULL != lpData && 0 != nLen )
		{
			pNetSrcData->lpData = (char*)malloc(nLen);
			if( !pNetSrcData->lpData )
			{
				free( pNetSrcData );
				pNetSrcData = NULL;
				break;
			}
			memcpy( pNetSrcData->lpData , lpData , nLen );
		}
		pNetSrcData->nProtocolType = nTransferProtocolType;
		pNetSrcData->nLen = nLen;
		if( nTransferProtocolType==TFileProtocol_Tcp )
			pNetSrcData->pDataSrc = pDataSrc;
		else if( nTransferProtocolType==TFileProtocol_Udp )
		{
			pNetSrcData->pDataSrc = calloc( 1 , sizeof(socket_addr_t) );
			memcpy( pNetSrcData->pDataSrc, pDataSrc, sizeof(socket_addr_t) );
		}
		break;
	}

	return pNetSrcData;
}

IME_EXTERN_C	void	IMeNetSrcDataDestroy( IMeNetSrcData* pNetSrcData )
{
	if( pNetSrcData->nProtocolType==TFileProtocol_Udp )
		free( pNetSrcData->pDataSrc );
	if( pNetSrcData->lpData )
		free( pNetSrcData->lpData );
	free( pNetSrcData );
}