#include	"fileClientProcess.h"
#include	"fileProtocol.h"
#include	"netSrcData.h"


IME_EXTERN_C	void	SendTFileServerLoginUserInfo( CTFileClient* pFileClient , const char* pAccount , const char* pPassword )
{
	char lpData[1024];
	int nOff = 0;
	uint32_t ph = MAKE_TFILEH(TFILE_APP_CLIENT,TFILE_DFORMAT_JSON,TFILE_C2S_ACCOUNT_VERIFY);

	const char* pAccountString;
	json_object* account_obj = NULL;
	
	if( !pFileClient->m_bConnectSuccess )	return;

	memcpy( &lpData[nOff] , &ph , LEN_UINT );	nOff += LEN_UINT;

	account_obj = json_object_new_object();
	json_object_object_add( account_obj , "uname" , json_object_new_string(pAccount) );
	json_object_object_add( account_obj , "upass" , json_object_new_string(pPassword) );
	pAccountString = json_object_to_json_string( account_obj );
		
	memcpy( &lpData[nOff] , pAccountString , strlen(pAccountString) );	
	nOff += strlen(pAccountString);
	
	if( pFileClient->SendLocalData( lpData , nOff ) != nOff )
	{
		DebugLogString( TRUE , "[SendTFileServerLoginUserInfo] failed!!" );
	}

	json_object_put( account_obj );
}

IME_EXTERN_C	uint8_t	SendTFileServerLocalFileInfo( CTFileClient* pFileClient , IMeTFileSource* pFileSource )
{
	uint8_t sndRes = FALSE;

	char* lpData;
	int nOff = 0;
	int ph;

	IMeTFileInfo* pFileInfo;
	int i;

	const char* fileObjStr;
	json_object* arrFileObj = json_object_new_array();
	json_object* fileObj;

    for( i=0; i<CArrayGetSize(pFileSource->m_arrFile); i++ )
    {
	    pFileInfo = (IMeTFileInfo*)CArrayGetAt( pFileSource->m_arrFile , i );
		fileObj = json_object_new_object();
		json_object_object_add( fileObj , "fileName" , json_object_new_string(pFileInfo->m_fileName) );
		json_object_object_add( fileObj , "fileSize" , json_object_new_int64(pFileInfo->m_fileSize) );
		json_object_object_add( fileObj , "fileID"   , json_object_new_int(pFileInfo->m_fileID) );
		json_object_object_add( fileObj , "fileIsDir"   , json_object_new_int(pFileInfo->m_fileIsDir) );
		json_object_array_add( arrFileObj , fileObj );
	}

	fileObjStr = json_object_to_json_string( arrFileObj );

	lpData = (char*)malloc( strlen(fileObjStr) + 32 );
	if( lpData )
	{
		ph = MAKE_TFILEH( TFILE_APP_CLIENT , TFILE_DFORMAT_JSON , TFILE_C2S_ADD_FILES_INFO );
		memcpy( &lpData[nOff] , &ph , LEN_INT );	nOff += LEN_INT;
		memcpy( &lpData[nOff] , fileObjStr , strlen(fileObjStr) );	nOff += strlen(fileObjStr); 

		if( pFileClient->SendLocalData( lpData , nOff ) == nOff )
			sndRes = TRUE;
		
		free( lpData );

		CEventWait( pFileClient->m_evWaitAck , 5000 );
	}

	json_object_put( arrFileObj );

	return sndRes;
}

IME_EXTERN_C	void	SendTFileServerLocalFileStart( CTFileClient* pFileClient , IMeTFileInfo* pFileInfo )
{
	char lpData[1024];
	int nOff = 0;
	int ph;

	const char* fileObjStr;
	json_object* fileObj = json_object_new_object();

	json_object_object_add( fileObj , "fileID" , json_object_new_int( pFileInfo->m_fileID ) );
	fileObjStr = json_object_to_json_string( fileObj );

	ph = MAKE_TFILEH( TFILE_APP_CLIENT , TFILE_DFORMAT_JSON , TFILE_C2S_ADD_FILE_START );

	memcpy( &lpData[nOff] , &ph , LEN_INT );	nOff += LEN_INT;
	memcpy( &lpData[nOff] , fileObjStr , strlen(fileObjStr) );	nOff += strlen(fileObjStr); 

		//send success
	if( pFileClient->SendLocalData( lpData , nOff ) == nOff )
	{
		//wait server start ack
		CEventWait( pFileClient->m_evWaitAck, 5000 );
	}

	json_object_put( fileObj );
}

IME_EXTERN_C	void	SendTFileServerLocalFileData( CTFileClient* pFileClient , IMeTFileSource* pFileSource , IMeTFileInfo* pFileInfo )
{
	char lpData[10*1460-4];
	//char lpData[1460];
	int	nReadRes;

	int nSndRes;
	int nOff = 0;
	int ph;

	//open file
	if( pFileInfo->m_fileCurFd == NULL )
	{
		char fileName[256];
		sprintf( fileName , "%s%s" , pFileSource->m_szMainDir , pFileInfo->m_fileName );
		pFileInfo->m_fileCurFd = CFileCreate();
		if( !pFileInfo->m_fileCurFd || CFileOpen( pFileInfo->m_fileCurFd , fileName , IMeFile_OpenRead ) != 1 )
		{
			DebugLogString( TRUE , "[SendTFileServerLocalFileData] open local file MainPath:%s FileName:%s failed!!" , pFileSource->m_szMainDir , pFileInfo->m_fileName );
			return;
		}
	}
	
	//read data from file & send
	ph = MAKE_TFILEH( TFILE_APP_CLIENT , TFILE_DFORMAT_BINARY , TFILE_C2S_ADD_FILE_DATA );
	memcpy( &lpData[nOff] , &ph , LEN_INT );	nOff += LEN_INT;
	if( ( nReadRes = CFileRead( pFileInfo->m_fileCurFd , &lpData[nOff] , 14600-8 ) ) > 0 )
	{
		nSndRes = pFileClient->SendLocalData( lpData , nReadRes+nOff );
		if( nSndRes > 0 )
			pFileInfo->m_fileOffset += nReadRes;
		else
			CFileSeek( pFileInfo->m_fileCurFd , pFileInfo->m_fileOffset , IMeFile_SeekBegin );
	}
}

IME_EXTERN_C	void	SendTFileServerLocalFile( CTFileClient* pFileClient , IMeTFileSource* pFileSource , IMeTFileInfo* pFileInfo )
{
	if( pFileInfo->m_fileStatus == TFILE_STATUS_WAITING )
	{
		SendTFileServerLocalFileStart( pFileClient , pFileInfo );
	}
	else if( pFileInfo->m_fileStatus == TFILE_STATUS_RUNNING )
	{
		//check file is send over
		if( pFileInfo->m_fileOffset == pFileInfo->m_fileSize )
		{
			//send file end to server
			SendTFileServerLocalFileEnd( pFileClient , pFileInfo );
		}
		else
		{
			SendTFileServerLocalFileData( pFileClient , pFileSource , pFileInfo );
		}
	}
}

IME_EXTERN_C	void	SendTFileServerLocalFileEnd( CTFileClient* pFileClient , IMeTFileInfo* pFileInfo )
{
	char lpData[1024];
	int nOff = 0;
	int ph;

	const char* fileObjStr;
	json_object* fileObj = json_object_new_object();

	json_object_object_add( fileObj , "fileID" , json_object_new_int( pFileInfo->m_fileID ) );
	fileObjStr = json_object_to_json_string( fileObj );

	ph = MAKE_TFILEH( TFILE_APP_CLIENT , TFILE_DFORMAT_JSON , TFILE_C2S_ADD_FILE_END );

	memcpy( &lpData[nOff] , &ph , LEN_INT );	nOff += LEN_INT;
	memcpy( &lpData[nOff] , fileObjStr , strlen(fileObjStr) );	nOff += strlen(fileObjStr); 

		//send success
	if( pFileClient->SendLocalData( lpData , nOff ) == nOff )
	{
		CEventWait( pFileClient->m_evWaitAck , 5000 );
	}

	DebugLogString( TRUE , "[SendTFileServerLocalFileEnd] fileID:%d over!!" , pFileInfo->m_fileID );

	json_object_put( fileObj );
}

IME_EXTERN_C	void	SendTFileServerAddFilesOver( CTFileClient* pFileClient , IMeTFileSource* pFileSource )
{
	char lpData[1024];
	int nOff = 0;
	int ph;
	
	const char* fileObjStr;
	json_object* fileObj = json_object_new_object();
	
	json_object_object_add( fileObj , "fileCnt" , json_object_new_int( CArrayGetSize(pFileSource->m_arrFile) ) );
	fileObjStr = json_object_to_json_string( fileObj );
	
	ph = MAKE_TFILEH( TFILE_APP_CLIENT , TFILE_DFORMAT_JSON , TFILE_C2S_ADD_FILES_OVER );
	
	memcpy( &lpData[nOff] , &ph , LEN_INT );	nOff += LEN_INT;
	memcpy( &lpData[nOff] , fileObjStr , strlen(fileObjStr) );	nOff += strlen(fileObjStr); 
	
	pFileClient->SendLocalData( lpData , nOff );

	json_object_put( fileObj );
}

IME_EXTERN_C	void	SendTFileServerAddFilesCancel( CTFileClient* pFileClient , IMeTFileSource* pFileSource )
{
	char lpData[1024];
	int nOff = 0;
	int ph;
	
	const char* fileObjStr;
	json_object* fileObj = json_object_new_object();
	
	json_object_object_add( fileObj , "fileCnt" , json_object_new_int( CArrayGetSize(pFileSource->m_arrFile) ) );
	fileObjStr = json_object_to_json_string( fileObj );
	
	ph = MAKE_TFILEH( TFILE_APP_CLIENT , TFILE_DFORMAT_JSON , TFILE_C2S_ADD_FILES_CANCEL );
	
	memcpy( &lpData[nOff] , &ph , LEN_INT );	nOff += LEN_INT;
	memcpy( &lpData[nOff] , fileObjStr , strlen(fileObjStr) );	nOff += strlen(fileObjStr); 
	
	//send success
	if( pFileClient->SendLocalData( lpData , nOff ) != nOff )
	{
		DebugLogString( TRUE , "[SendTFileServerAddFilesCancel] cancel add local file MainPath:%s FileCnt:%d!!" , pFileSource->m_szMainDir , CArrayGetSize(pFileSource->m_arrFile) );
	}

	json_object_put( fileObj );
}

IME_EXTERN_C	void	TFileClientTheadUploadFile( void* parameter )
{
	CTFileClient* pTFileClient = (CTFileClient*)parameter;
	
	IMeTFileSource* pFileSource;
	uint32_t listPos;

	while( pTFileClient->m_bUploading )
	{
		CLock_Lock( pTFileClient->m_lockerListFileSource );

		pFileSource = (IMeTFileSource*)CListGetHead( pTFileClient->m_listFileSource, &listPos );
		
		CLock_Unlock( pTFileClient->m_lockerListFileSource );

		//wait transfer next file source 
		if( !pFileSource )
		{
			IMeSleep(10);	
			continue;
		}

		//commit file info to server
		if( pFileSource->m_curUploadStatus == TFILE_STATUS_WAITING )
		{
			if( SendTFileServerLocalFileInfo( pTFileClient , pFileSource ) )
			{
				pFileSource->m_lastCommitInfoTime = IMeGetCurrentTime();
			}
		}
		//snd data
		else
		{	
			IMeTFileInfo* pFileInfo = NULL;
			
			if( pFileSource->m_nCurUploadFileID == -1 )
			{
                int i;
                for( i=0; i<CArrayGetSize(pFileSource->m_arrFile); i++ )
                {				    
                    IMeTFileInfo* pFindFileInfo = (IMeTFileInfo*)CArrayGetAt( pFileSource->m_arrFile , i );
				    if( pFindFileInfo->m_fileStatus == TFILE_STATUS_WAITING )
                    {
                        pFileInfo = pFindFileInfo;
					    break;
                    }
                }
			}
			else
				pFileInfo = (IMeTFileInfo*)CArrayFindData( pFileSource->m_arrFile , pFileSource->m_nCurUploadFileID );	
			
			//transfered file in sub-list
			if( pFileInfo )
			{
				SendTFileServerLocalFile( pTFileClient , pFileSource , pFileInfo );
			}
			//this file source transfered over
			else
			{
				//notify server this file source uploaded over
				SendTFileServerAddFilesOver( pTFileClient , pFileSource );
                //output transfer file rate
                IMeTFileSourceStatisticTransferRate( pFileSource );
				//remove local file source in list
				CLock_Lock( pTFileClient->m_lockerListFileSource );
				CListRemoveHead( pTFileClient->m_listFileSource );
				CLock_Unlock( pTFileClient->m_lockerListFileSource );

				IMeTFileSourceDestroy( pFileSource );
			}
		}
	}
}


IME_EXTERN_C	void	OnTFileClientProtocolRcvFileEndAck( CTFileClient* pFileClient , json_object* tfileEndAckObj , void* rcvSource )
{
	uint32_t listPos;

	int nFileID;
	int64_t serverFileOffset;
	int64_t serverFileSize;

	IMeTFileSource* pTFileSource;
	
	CLock_Lock( pFileClient->m_lockerListFileSource );

	pTFileSource = (IMeTFileSource*)CListGetHead(pFileClient->m_listFileSource,&listPos);
	if( pTFileSource )
	{
		if( json_object_object_get_int( tfileEndAckObj , "fileID" , &nFileID ) )
		{
			IMeTFileInfo* pTFileInfo = (IMeTFileInfo*)CArrayFindData( pTFileSource->m_arrFile , (int64_t)nFileID );
			if( pTFileInfo )
			{
				//current file is uploaded , synchronous file upload status with server
				if( pTFileInfo->m_fileOffset == pTFileInfo->m_fileSize )
				{
					json_object_object_get_int64( tfileEndAckObj , "fileOffset" , (int64_t*)&serverFileOffset );
					json_object_object_get_int64( tfileEndAckObj , "fileSize" , (int64_t*)&serverFileSize );

					//server no receive full this file , resend this file
					if( serverFileOffset != serverFileSize ) 
					{
						pTFileInfo->m_fileStatus = TFILE_STATUS_RUNNING;
						pTFileInfo->m_fileOffset = serverFileOffset;
						CFileSeek( pTFileInfo->m_fileCurFd , pTFileInfo->m_fileOffset , IMeFile_SeekBegin );
						DebugLogString( TRUE , "[OnTFileClientProtocolRcvFileEndAck] server no receive this full file!!" );
					}
					//server receive over this file
					else
					{
						if( pTFileInfo->m_fileCurFd )
						{
							CFileClose( pTFileInfo->m_fileCurFd );
							CFileDestroy( pTFileInfo->m_fileCurFd );
							pTFileInfo->m_fileCurFd = NULL;
						}
						pTFileInfo->m_fileStatus = TFILE_STATUS_OVER;
						//try upload next file in this file source
						pTFileSource->m_nCurUploadFileID = -1;

						CEventSet( pFileClient->m_evWaitAck );

						DebugLogString( TRUE , "[OnTFileClientProtocolRcvFileEndAck] file:%s fileID:%d was uploaded over!!" , pTFileInfo->m_fileName , nFileID );
					}
				}
				else
				{
					DebugLogString( TRUE , "[OnTFileClientProtocolRcvFileEndAck] file:%s fileID:%d is uploading , discard this cmd!!" , pTFileInfo->m_fileName , nFileID );
				}
			}
			else
			{
				DebugLogString( TRUE , "[OnTFileClientProtocolRcvFileEndAck] no find file id:%d" , nFileID );
			}
		}
	}
	else
	{
		DebugLogString( TRUE , "[OnTFileClientProtocolRcvFileEndAck] no find File source!!" );
	}

	CLock_Unlock( pFileClient->m_lockerListFileSource );
}

IME_EXTERN_C	void	OnTFileClientProtocolRcvFileStartAck( CTFileClient* pFileClient , json_object* tfileStartAckObj , void* rcvSource )
{
	int nFileID;
	
	uint32_t listPos;
	IMeTFileSource* pTFileSource;
		
	CLock_Lock( pFileClient->m_lockerListFileSource );

	pTFileSource = (IMeTFileSource*)CListGetHead(pFileClient->m_listFileSource,&listPos);
	if( pTFileSource )
	{
		if( json_object_object_get_int( tfileStartAckObj , "fileID" , &nFileID ) )
		{
			IMeTFileInfo* pTFileInfo = (IMeTFileInfo*)CArrayFindData( pTFileSource->m_arrFile , (int64_t)nFileID );
			if( pTFileInfo )
			{
				pTFileSource->m_nCurUploadFileID = nFileID;
				pTFileInfo->m_fileStatus = TFILE_STATUS_RUNNING;
				CEventSet( pFileClient->m_evWaitAck );
				DebugLogString( TRUE , "[OnTFileClientProtocolRcvFileStartAck] change current upload fileid:%d filename:%s is running!!" , pTFileInfo->m_fileID , pTFileInfo->m_fileName );
			}
			else 
			{
				DebugLogString( TRUE , "[OnTFileClientProtocolRcvFileStartAck] no find file id:%d in Client!!" , nFileID );
			}
		}
		else
		{
			DebugLogString( TRUE , "[OnTFileClientProtocolRcvFileStartAck] no find file-id in server json-string!!" );
		}
	}

	CLock_Unlock( pFileClient->m_lockerListFileSource );
}

IME_EXTERN_C	void	OnTFileClientProtocolRcvFilesInfoAck( CTFileClient* pFileClient , json_object* tfileFilesInfoAckObj , void* rcvSource )
{
	IMeTFileSource* pTFileSource;
	uint32_t listPos;

	CLock_Lock( pFileClient->m_lockerListFileSource );

	pTFileSource = (IMeTFileSource*)CListGetHead(pFileClient->m_listFileSource,&listPos);
	if( pTFileSource )
	{
        pTFileSource->m_dwStartTransferTime = IMeGetCurrentTime();
		pTFileSource->m_curUploadStatus = TFILE_STATUS_RUNNING;
		CEventSet( pFileClient->m_evWaitAck );
	}
	
	CLock_Unlock( pFileClient->m_lockerListFileSource );

}

/* tfileClient user check */
IME_EXTERN_C	void	OnTFileClientProtocolRcvAccountVerifyAck( CTFileClient* pFileClient , json_object* tfileAccountObj , void* rcvSource )
{
	int bVerifyRes = 0;
	const char* pServerVersion;
	
	json_object_object_get_int( tfileAccountObj , "verifyRes" , &bVerifyRes );
	pServerVersion = json_object_object_get_string( tfileAccountObj , "serverVersion" );

	DebugLogString( TRUE , "[OnTFileClientProtocolRcvAccountVerifyAck] verifyRes:%d serverVersion:%s!!" , bVerifyRes , pServerVersion?pServerVersion:"NULL" );
	
	if( bVerifyRes )
	{
		pFileClient->m_bLoginSuccess = TRUE;
	}

	//notify up-app i'm ok
	pFileClient->m_statusCB( bVerifyRes?TFileClientNotify_VerifySuccess:TFileClientNotify_VerifyFailed , pFileClient->m_upApp );
}

/* json cmd from client */
IME_EXTERN_C	void	OnTFileClientProtocolRcvJsonCmd( CTFileClient* pFileClient , uint32_t tfilePH , char* lpData , int nLen )
{
	int nCmd = TFILEH_CMD(tfilePH);
	int nOff = 4;	/* ph-4byte */
	json_object* json_object_tfile = NULL;
	
	if( nLen-nOff > 0 )
		json_object_tfile = json_tokener_parse( (const char*)&lpData[nOff] );

	switch( nCmd )
	{
	case TFILE_S2C_ACCOUNT_VERIFY_ACK:
		OnTFileClientProtocolRcvAccountVerifyAck( pFileClient , json_object_tfile , (pFileClient->m_pTcpSocket ? (void*)pFileClient->m_pTcpSocket : (void*)pFileClient->m_pUdpSocket) );
		break;
	case TFILE_S2C_ADD_FILE_INFO_ACK:
		OnTFileClientProtocolRcvFilesInfoAck( pFileClient , json_object_tfile , pFileClient->m_pTcpSocket ? (void*)pFileClient->m_pTcpSocket : (void*)pFileClient->m_pUdpSocket );
		break;
	case TFILE_S2C_ADD_FILE_START_ACK:
		OnTFileClientProtocolRcvFileStartAck( pFileClient , json_object_tfile , pFileClient->m_pTcpSocket ? (void*)pFileClient->m_pTcpSocket : (void*)pFileClient->m_pUdpSocket );
		break;
	case TFILE_S2C_ADD_FILE_END_ACK:
		OnTFileClientProtocolRcvFileEndAck( pFileClient , json_object_tfile , pFileClient->m_pTcpSocket ? (void*)pFileClient->m_pTcpSocket : (void*)pFileClient->m_pUdpSocket );
		break;
	default:
		DebugLogString( TRUE , "[OnTFileClientProtocolRcvJsonCmd] no process json cmd data!!" );
		break;
	}

	if( json_object_tfile )
		json_object_put( json_object_tfile );
}

/* binary data from client */
IME_EXTERN_C	void	OnTFileClientProtocolRcvBinary( CTFileClient* pFileClient , uint32_t tfilePH , char* lpData , int nLen )
{
	//this interface no use for upload file client
}

/* user net blocked , update status */
IME_EXTERN_C	void	OnTFileClientProtocolBlocked( CTFileClient* pFileClient , void* pDataSourceUser )
{
	//update status from remote server
	pFileClient->m_bConnectSuccess = FALSE;
	pFileClient->m_bLoginSuccess = FALSE;

	//notify up my status
	if( pFileClient->m_statusCB )
	{
		pFileClient->m_statusCB( TFileClientNotify_ConnectionBlocked , pFileClient->m_upApp );
	}
}

/* receive net data from client */
IME_EXTERN_C	void	OnFileClientProtocolRcvServerData( CTFileClient* pFileClient , char* lpData , int nLen , void* rcvSource )
{
	//rcv data
	if( NULL != lpData && nLen > 0 )
	{
		int nOff = 0;
		uint32_t ph = *(uint*)&lpData[nOff];	
		int nFormat = TFILEH_DFORMAT(ph);
		int nAppClient = TFILEH_APP(ph);
		
		//json format data
		if( TFILE_DFORMAT_JSON == nFormat && TFILE_APP_SERVER == nAppClient )
		{
			OnTFileClientProtocolRcvJsonCmd( pFileClient , ph , lpData , nLen );
		}
		//binary format data
		else if( TFILE_DFORMAT_BINARY == nFormat && TFILE_APP_SERVER == nAppClient )
		{
			OnTFileClientProtocolRcvBinary( pFileClient , ph , lpData , nLen );
		}
		else
		{
			DebugLogString( TRUE , "[OnFileClientProtocolRcvServerData] Can't Be Resolved Data Format From Remote Server!!" );
		}
	}
	//net blocked , check
	else
	{
		OnTFileClientProtocolBlocked( pFileClient , rcvSource );
	}
}
