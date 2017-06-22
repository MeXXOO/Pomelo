#ifndef _FILE_CLIENT_PROCESS_H_
#define	_FILE_CLIENT_PROCESS_H_

#include	"tfileclient.h"
#include	"fileCommon.h"
#include	"netSrcData.h"


IME_EXTERN_C	void	TFileClientTheadUploadFile( void* parameter );


/************************************************************************/
/* snd cmd to tfile server                                                                     */
/************************************************************************/
IME_EXTERN_C	void	SendTFileServerLoginUserInfo( CTFileClient* pFileClient , const char* pAccount , const char* pPassword );
IME_EXTERN_C	bool	SendTFileServerLocalFileInfo( CTFileClient* pFileClient , IMeTFileSource* pFileSource );

IME_EXTERN_C	void	SendTFileServerLocalFileStart( CTFileClient* pFileClient , IMeTFileInfo* pFileInfo );
IME_EXTERN_C	void	SendTFileServerLocalFileData( CTFileClient* pFileClient , IMeTFileSource* pFileSource , IMeTFileInfo* pFileInfo );
IME_EXTERN_C	void	SendTFileServerLocalFile( CTFileClient* pFileClient , IMeTFileSource* pFileSource , IMeTFileInfo* pFileInfo );

IME_EXTERN_C	void	SendTFileServerLocalFileEnd( CTFileClient* pFileClient , IMeTFileInfo* pFileInfo );
IME_EXTERN_C	void	SendTFileServerAddFilesOver( CTFileClient* pFileClient , IMeTFileSource* pFileSource );
IME_EXTERN_C	void	SendTFileServerAddFilesCancel( CTFileClient* pFileClient , IMeTFileSource* pFileSource );


/************************************************************************/
/* rcv cmd from tfile server                                                                     */
/************************************************************************/
IME_EXTERN_C	void	OnTFileClientProtocolRcvFileEndAck( CTFileClient* pFileClient , json_object* tfileEndAckObj , void* rcvSource );
IME_EXTERN_C	void	OnTFileClientProtocolRcvFileStartAck( CTFileClient* pFileClient , json_object* tfileStartAckObj , void* rcvSource );
IME_EXTERN_C	void	OnTFileClientProtocolRcvFilesInfoAck( CTFileClient* pFileClient , json_object* tfileInfoAckObj , void* rcvSource );
IME_EXTERN_C	void	OnTFileClientProtocolRcvAccountVerifyAck( CTFileClient* pFileClient , json_object* tfileAccountObj , void* rcvSource );


IME_EXTERN_C	void	OnTFileClientProtocolRcvJsonCmd( CTFileClient* pFileClient , uint tfilePH , char* lpData , int nLen );
IME_EXTERN_C	void	OnTFileClientProtocolRcvBinary( CTFileClient* pFileClient , uint tfilePH , char* lpData , int nLen );
IME_EXTERN_C	void	OnTFileClientProtocolBlocked( CTFileClient* pFileClient , void* pDataSourceUser );


IME_EXTERN_C	void	OnFileClientProtocolRcvServerData( CTFileClient* pFileClient , char* lpData , int nLen , void* rcvSource );


#endif	//end _FILE_CLIENT_PROCESS_H_
