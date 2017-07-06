#ifndef _FILE_SERVER_PROCESS_H_
#define	_FILE_SERVER_PROCESS_H_

#include	"server.h"
#include	"fileCommon.h"
#include	"netSrcData.h"


IME_EXTERN_C	void	TFileServerCheckUserTransferStatus( IMeFileServer* pFileServer , void* pSocketClient );

//snd cmd to client
IME_EXTERN_C	void	SendTFileClientProtocolFileEndAck( IMeFileServer* pFileServer , IMeTFileInfo* pFileInfo , IMeTFileSourceUser* pFileSourceUser );
IME_EXTERN_C	void	SendTFileClientProtocolFileStartAck( IMeFileServer* pFileServer , IMeTFileSourceUser* pFileSourceUser );
IME_EXTERN_C	void	SendTFileClientProtocolRcvFileInfoAck( IMeFileServer* pFileServer , IMeTFileSourceUser* pFileSourceUser );
IME_EXTERN_C	void	SendTFileClientProtocolAccountVerifyAck( IMeFileServer* pFileServer , IMeTFileSourceUser* pFileSourceUser );


//rcv cmd from client
IME_EXTERN_C	void	OnTFileServerProtocolFilesCancel( IMeFileServer* pFileServer , json_object* tfileAccountObj , void* rcvSource );
IME_EXTERN_C	void	OnTFileServerProtocolFilesOver( IMeFileServer* pFileServer , json_object* tfileAccountObj , void* rcvSource );
IME_EXTERN_C	void	OnTFileServerProtocolRcvFileEnd( IMeFileServer* pFileServer , json_object* tfileAccountObj , void* rcvSource );
IME_EXTERN_C	void	OnTFileServerProtocolRcvFileStart( IMeFileServer* pFileServer , json_object* tfileAccountObj , void* rcvSource );
IME_EXTERN_C	void	OnTFileServerProtocolRcvFilesInfo( IMeFileServer* pFileServer , json_object* tfileAccountObj , void* rcvSource );
IME_EXTERN_C	void	OnTFileServerProtocolRcvAccountVerify( IMeFileServer* pFileServer , json_object* tfileAccountObj , void* rcvSource );


IME_EXTERN_C	void	OnTFileServerProtocolRcvJsonCmd( IMeFileServer* pFileServer , uint32_t tfilePH , IMeNetSrcData* pNetSrcData );
IME_EXTERN_C	void	OnTFileServerProtocolRcvBinary( IMeFileServer* pFileServer , uint32_t tfilePH , IMeNetSrcData* pNetSrcData );
IME_EXTERN_C	void	OnTFileServerProtocolBlocked( IMeFileServer* pFileServer , void* pDataSourceUser );
IME_EXTERN_C	void	OnTFileServerRcvNetData( IMeFileServer* pFileServer , IMeNetSrcData* pNetSrcData );


IME_EXTERN_C	void	ThreadTFileServerProcessData( IMeFileServer* pFileServer );
IME_EXTERN_C	void	OnFileServerProtocolRcvClientData( IMeFileServer* pFileServer , char* lpData , int nLen , void* rcvSource );


#endif	//end _FILE_SERVER_PROCESS_H_
