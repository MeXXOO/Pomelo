#include "netSrcData.h"

IME_EXTERN_C	IMeNetSrcData*	IMeNetSrcDataCreate( char* lpData , int nLen , void* pDataSrc )
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
		pNetSrcData->nLen = nLen;
		pNetSrcData->pDataSrc = pDataSrc;
		break;
	}

	return pNetSrcData;
}

IME_EXTERN_C	void	IMeNetSrcDataDestroy( IMeNetSrcData* pNetSrcData )
{
	if( pNetSrcData->lpData )
		free( pNetSrcData->lpData );
	free( pNetSrcData );
}