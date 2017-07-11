#ifndef		_NET_SRC_DATA_H_
#define		_NET_SRC_DATA_H_

#include	"platdefine.h"

typedef		struct		_IMeNetSrcData{
	int		nProtocolType;
	char*	lpData;
	int		nLen;
	void*	pDataSrc;
}IMeNetSrcData;

IME_EXTERN_C	IMeNetSrcData*	IMeNetSrcDataCreate( int nTransferProtocolType, char* lpData , int nLen , void* pDataSrc );
IME_EXTERN_C	void	IMeNetSrcDataDestroy( IMeNetSrcData* pNetSrcData );

#endif		//end _NET_SRC_DATA_H_
