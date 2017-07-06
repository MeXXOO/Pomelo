#include	"fileServerProcess.h"
#include	"fileProtocol.h"
#include	"netSrcData.h"

IME_EXTERN_C	void	TFileServerCheckUserTransferStatus( IMeFileServer* pFileServer , void* pFileSource )
{
	int i;
	int nCount = 0;
	uint32_t dwCurTime;

	while( pFileServer->m_bRunning )
	{
		if( ++nCount > 4 )
		{
			nCount = 0;
			dwCurTime = IMeGetCurrentTime();

			CLock_Lock( pFileServer->m_lockClient );
			
			for( i=CArrayGetSize(pFileServer->m_arrFileSourceClient)-1; i>=0; --i )
			{
				IMeTFileSourceUser* pTFileSourceUser = (IMeTFileSourceUser*)CArrayGetAt(pFileServer->m_arrFileSourceClient,i);
				if( dwCurTime - pTFileSourceUser->m_dwLastRcvDataTime > TFILE_RCV_TIMEOUT && 0 != pTFileSourceUser->m_dwLastRcvDataTime )
				{
					CArrayRemove( pFileServer->m_arrFileSourceClient , i );
					DebugLogString( TRUE , "[TFileServerCheckUserTransferStatus] rcv client data time out %0x!!" , pTFileSourceUser->m_pFileSource );
					IMeTFileSourceUserDestroy( pTFileSourceUser );
				}
			}

			CLock_Unlock( pFileServer->m_lockClient );
		}

		IMeSleep( 1000 );
	}
}

IME_EXTERN_C	int		IMeFileServerSendData( IMeFileServer* pFileServer, void* pFileSource, uint8_t* buffer, int len )
{
	if( pFileServer->m_nTransferProtocolType == TFileProtocol_Tcp )
	{
		IMeSocketTcp* pTcpFileSource = (IMeSocketTcp*)pFileSource;
		return CSocketTcpSend( pTcpFileSource, (char*)buffer, len );
	}
	else if( pFileServer->m_nTransferProtocolType == TFileProtocol_Udp )
	{
		socket_addr_t* addr_user = (socket_addr_t*)pFileSource;
		return CSocketUdpSendByAddr( pFileServer->m_pUdpSocket, (char*)buffer, len, addr_user );
	}

	return 0;
}

IME_EXTERN_C	void	SendTFileClientProtocolFileEndAck( IMeFileServer* pFileServer , IMeTFileInfo* pFileInfo , IMeTFileSourceUser* pFileSourceUser )
{
	uint8_t lpData[1024];
	int nOff = 0;
	
	uint32_t ph;

	const char* fileObjStr;
	json_object* fileObj = json_object_new_object();

	ph = MAKE_TFILEH(TFILE_APP_SERVER,TFILE_DFORMAT_JSON,TFILE_S2C_ADD_FILE_END_ACK);
	memcpy( &lpData[nOff] , &ph , LEN_UINT );	nOff += LEN_UINT;	

	json_object_object_add( fileObj , "fileID" , json_object_new_int(pFileSourceUser->m_fileCurTID) );
	json_object_object_add( fileObj , "fileOffset" , json_object_new_int64(pFileInfo->m_fileOffset) );
	json_object_object_add( fileObj , "fileSize" , json_object_new_int64(pFileInfo->m_fileSize) );
	fileObjStr = json_object_to_json_string( fileObj );

	memcpy( &lpData[nOff] , fileObjStr , strlen(fileObjStr) );	
	nOff += strlen(fileObjStr);

	if( IMeFileServerSendData( pFileServer, pFileSourceUser->m_pFileSource , lpData , nOff ) != nOff )
	{
		DebugLogString( TRUE , "[SendTFileClientProtocolFileEndAck] failed!!" );
	}

	DebugLogString( TRUE , "[SendTFileClientProtocolFileEndAck] success!!" );

	json_object_put( fileObj );	
}

IME_EXTERN_C	void	SendTFileClientProtocolFileStartAck( IMeFileServer* pFileServer , IMeTFileSourceUser* pFileSourceUser )
{
	uint8_t lpData[256];
	int nOff = 0;
	
	uint32_t ph;

	const char* fileObjStr;
	json_object* fileObj = json_object_new_object();

	ph = MAKE_TFILEH(TFILE_APP_SERVER,TFILE_DFORMAT_JSON,TFILE_S2C_ADD_FILE_START_ACK);
	memcpy( &lpData[nOff] , &ph , LEN_UINT );	nOff += LEN_UINT;	

	json_object_object_add( fileObj , "fileID" , json_object_new_int(pFileSourceUser->m_fileCurTID) );
	fileObjStr = json_object_to_json_string( fileObj );

	memcpy( &lpData[nOff] , fileObjStr , strlen(fileObjStr) );	
	nOff += strlen(fileObjStr);

	if( IMeFileServerSendData( pFileServer, pFileSourceUser->m_pFileSource , lpData , nOff ) != nOff )
	{
		DebugLogString( TRUE , "[SendTFileClientProtocolRcvFileInfoAck] failed!!" );
	}

	json_object_put( fileObj );
}

IME_EXTERN_C	void	SendTFileClientProtocolRcvFileInfoAck( IMeFileServer* pFileServer , IMeTFileSourceUser* pFileSourceUser )
{
	uint8_t lpData[256];
	int nOff = 0;
	
	uint32_t ph;

	const char* fileObjStr;
	json_object* fileObj = json_object_new_object();

	ph = MAKE_TFILEH(TFILE_APP_SERVER,TFILE_DFORMAT_JSON,TFILE_S2C_ADD_FILE_INFO_ACK);
	memcpy( &lpData[nOff] , &ph , LEN_UINT );	nOff += LEN_UINT;	

	json_object_object_add( fileObj , "rcvfileInfo" , json_object_new_int(1) );
	fileObjStr = json_object_to_json_string( fileObj );

	memcpy( &lpData[nOff] , fileObjStr , strlen(fileObjStr) );	
	nOff += strlen(fileObjStr);

	if( IMeFileServerSendData( pFileServer, pFileSourceUser->m_pFileSource , lpData , nOff ) != nOff )
	{
		DebugLogString( TRUE , "[SendTFileClientProtocolRcvFileInfoAck] failed!!" );
	}

	json_object_put( fileObj );
}

IME_EXTERN_C	void	SendTFileClientProtocolAccountVerifyAck( IMeFileServer* pFileServer , IMeTFileSourceUser* pFileSourceUser )
{
	uint8_t lpData[1024];
	int nOff = 0;
	
	uint32_t ph;
	
	json_object* account_verify_res;
	const char* account_verify_str;

	ph = MAKE_TFILEH(TFILE_APP_SERVER,TFILE_DFORMAT_JSON,TFILE_S2C_ACCOUNT_VERIFY_ACK);
	memcpy( &lpData[nOff] , &ph , LEN_UINT );	nOff += LEN_UINT;

	account_verify_res = json_object_new_object();
	json_object_object_add( account_verify_res , "verifyRes" , json_object_new_int(pFileSourceUser->m_bVerify) );
	json_object_object_add( account_verify_res , "serverVersion" , json_object_new_string(TFILESERVER_VERSION) );
	account_verify_str = json_object_to_json_string( account_verify_res );
		
	memcpy( &lpData[nOff] , account_verify_str , strlen(account_verify_str) );	
	nOff += strlen(account_verify_str);

	if( IMeFileServerSendData( pFileServer, pFileSourceUser->m_pFileSource , lpData , nOff ) != nOff )
	{
		DebugLogString( TRUE , "[SendTFileClientProtocolAccountVerifyAck] send failed!!" );
	}

	json_object_put( account_verify_res );
}




/************************************************************************/
/*  rcv cmd from client user                                                                     */
/************************************************************************/

IME_EXTERN_C	void	OnTFileServerProtocolRcvFileEnd( IMeFileServer* pFileServer , json_object* tfileEndObj , void* rcvSource )
{
	int nFileID;

	IMeTFileSourceUser* pTFileSourceUser = (IMeTFileSourceUser*)CArrayFindData(pFileServer->m_arrFileSourceClient,(uint32_t)rcvSource);
	if( !pTFileSourceUser )
	{
		DebugLogString( TRUE , "[OnTFileServerProtocolRcvFileEnd] failed find tfile source client!!" );
		return;
	}

	if( json_object_object_get_int( tfileEndObj , "fileID" , &nFileID ) )
	{
		IMeTFileInfo* pTFileInfo = CArrayFindData( pTFileSourceUser->m_listFile , (uint64_t)nFileID );
		if( pTFileInfo )
		{
			if( TFILE_STATUS_OVER != pTFileInfo->m_fileStatus )
			{
				//snd file end ack to client
				SendTFileClientProtocolFileEndAck( pFileServer , pTFileInfo , pTFileSourceUser );

				if( pTFileInfo->m_fileOffset == pTFileInfo->m_fileSize )
				{
					if( pTFileInfo->m_fileCurFd )
					{
						CFileClose( pTFileInfo->m_fileCurFd );
						CFileDestroy( pTFileInfo->m_fileCurFd );
						pTFileInfo->m_fileCurFd = NULL;
					}
                    DebugLogString( TRUE , "[OnTFileServerProtocolRcvFileEnd] fileID:%d fileName:%s" , pTFileInfo->m_fileID, pTFileInfo->m_fileName );
					pTFileInfo->m_fileStatus = TFILE_STATUS_OVER;
					//current file rcv over
					pTFileSourceUser->m_fileCurTID = -1;
				}
				//exception tooltip
				else
				{
					DebugLogString( TRUE , "[OnTFileServerProtocolRcvFileEnd] fileName:%s fileOffset:%I64d no equal fileSize:%I64d exception!!" ,		\
						pTFileInfo->m_fileName , pTFileInfo->m_fileOffset , pTFileInfo->m_fileSize );
				}
			}
			else
			{
				DebugLogString( TRUE , "[OnTFileServerProtocolRcvFileEnd] file:%s fileID:%d transfer status exception!!" , pTFileInfo->m_fileName , nFileID );
			}
		}
	}
}

IME_EXTERN_C	void	OnTFileServerProtocolRcvFileStart( IMeFileServer* pFileServer , json_object* tfileStartObj , void* rcvSource )
{
	int nFileID;

	IMeTFileSourceUser* pTFileSourceUser = (IMeTFileSourceUser*)CArrayFindData(pFileServer->m_arrFileSourceClient,(uint32_t)rcvSource);
	if( !pTFileSourceUser )
	{
		DebugLogString( TRUE , "[OnTFileServerProtocolRcvFileStart] failed find tfile source client!!" );
		return;
	}	

	if( json_object_object_get_int( tfileStartObj , "fileID" , &nFileID ) )
	{
		IMeTFileInfo* pTFileInfo = CArrayFindData( pTFileSourceUser->m_listFile , (uint64_t)nFileID );
		if( pTFileInfo )
		{
			if( TFILE_STATUS_OVER != pTFileInfo->m_fileStatus && pTFileInfo->m_fileCurFd==NULL )
			{
				pTFileInfo->m_fileCurFd = CFileCreate();
				if( pTFileInfo->m_fileCurFd && CFileOpen( pTFileInfo->m_fileCurFd , pTFileInfo->m_fileName , IMeFile_OpenWrite|IMeFile_OpenCreate ) )
				{
					pTFileInfo->m_fileStatus = TFILE_STATUS_RUNNING;
					pTFileSourceUser->m_fileCurTID = nFileID;
					DebugLogString( TRUE , "[OnTFileServerProtocolRcvFileStart] file:%s is running!!" , pTFileInfo->m_fileName );
					
					SendTFileClientProtocolFileStartAck( pFileServer , pTFileSourceUser );
				}
				else
					DebugLogString( TRUE , "[OnTFileServerProtocolRcvFileStart] Open server file:%s failed!!" , pTFileInfo->m_fileName );
			}
			else
			{
				DebugLogString( TRUE , "[OnTFileServerProtocolRcvFileStart] file:%s fileID:%d transfer status exception!!" , pTFileInfo->m_fileName , nFileID );
			}
		}
		else 
		{
			DebugLogString( TRUE , "[OnTFileServerProtocolRcvFileStart] no find file id:%d in server!!" , nFileID );
		}
	}
	else
	{
		DebugLogString( TRUE , "[OnTFileServerProtocolRcvFileStart] no find file-id in client json-string!!" );
	}
}

IME_EXTERN_C	void	OnTFileServerProtocolFilesOver( IMeFileServer* pFileServer , json_object* tfileFilesOverObj , void* rcvSource )
{
	IMeTFileSourceUser* pTFileSourceUser = (IMeTFileSourceUser*)CArrayFindData(pFileServer->m_arrFileSourceClient,(uint32_t)rcvSource);
	if( !pTFileSourceUser )
	{
		DebugLogString( TRUE , "[OnTFileServerProtocolFilesOver] failed find tfile source client!!" );
		return;
	}

    DebugLogString( TRUE , "[OnTFileServerProtocolFilesOver] ======%0x" , pTFileSourceUser->m_pFileSource );

	CArrayRemove( pFileServer->m_arrFileSourceClient , (uint32_t)rcvSource );
	IMeTFileSourceUserDestroy( pTFileSourceUser );
}

IME_EXTERN_C	void	OnTFileServerProtocolFilesCancel( IMeFileServer* pFileServer , json_object* tfileFilesOverObj , void* rcvSource )
{
	IMeTFileSourceUser* pTFileSourceUser = (IMeTFileSourceUser*)CArrayFindData(pFileServer->m_arrFileSourceClient,(uint32_t)rcvSource);
	if( !pTFileSourceUser )
	{
		DebugLogString( TRUE , "[OnTFileServerProtocolFilesCancel] failed find tfile source client!!" );
		return;
	}

	CArrayRemove( pFileServer->m_arrFileSourceClient , (uint32_t)rcvSource );
	IMeTFileSourceUserDestroy( pTFileSourceUser );
}

IME_EXTERN_C	void	OnTFileServerProtocolRcvFilesInfo( IMeFileServer* pFileServer , json_object* tfileFilesInfoObj , void* rcvSource )
{
	int i;
	IMeTFileSourceUser* pTFileSourceUser = (IMeTFileSourceUser*)CArrayFindData(pFileServer->m_arrFileSourceClient,(uint32_t)rcvSource);
	if( !pTFileSourceUser )
	{
		DebugLogString( TRUE , "[OnTFileServerProtocolRcvFilesInfo] failed find tfile source client!!" );
		return;
	}

	for( i = 0; i < json_object_array_length(tfileFilesInfoObj); ++i )
	{
		json_object* fileObj = json_object_array_get_idx(tfileFilesInfoObj,i);
		const char* fileName = json_object_object_get_string( fileObj , "fileName" );
		uint64_t fileSize;
		uint32_t fileID;
		IMeTFileInfo* pTFileInfo;

		char szFileName[256];

		json_object_object_get_int64( fileObj , "fileSize" , (int64_t*)&fileSize );
		json_object_object_get_int( fileObj , "fileID" , (int*)&fileID );
		
		sprintf( szFileName , "%s%s" , pFileServer->m_fileDir , fileName );
		pTFileInfo = IMeTFileInfoCreate( szFileName , fileSize , fileID );
		if( pTFileInfo )
		{
			CArrayAdd( pTFileSourceUser->m_listFile , pTFileInfo , (uint64_t)fileID );
			DebugLogString( TRUE , "[OnTFileServerProtocolRcvFilesInfo] add file:%s fileSize:%I64d fileID:%d!" , szFileName , fileSize , fileID );
		}
	}

	//ack to client
	SendTFileClientProtocolRcvFileInfoAck( pFileServer , pTFileSourceUser );
}

/* tfileserver user check */
IME_EXTERN_C	void	OnTFileServerProtocolRcvAccountVerify( IMeFileServer* pFileServer , json_object* tfileAccountObj , void* rcvSource )
{
	const char* pClientUserName = json_object_object_get_string( tfileAccountObj , "uname" );
	const char* pClientUserPass = json_object_object_get_string( tfileAccountObj , "upass" );
	uint8_t bVerifyRes = TRUE;

	DebugLogString( TRUE , "[OnTFileServerProtocolRcvAccountVerify] UName:%s UPass:%s!!" , pClientUserName?pClientUserName:"NULL" , pClientUserPass?pClientUserPass:"NULL" );
	
	if( bVerifyRes )
	{
		IMeTFileSourceUser* pTFileSourceUser = (IMeTFileSourceUser*)CArrayFindData( pFileServer->m_arrFileSourceClient , (uint32_t)rcvSource );
		if( pTFileSourceUser )
		{
			pTFileSourceUser->m_bVerify = TRUE;
			pTFileSourceUser->m_dwLastRcvDataTime = IMeGetCurrentTime();	//refresh last receive data time 
			SendTFileClientProtocolAccountVerifyAck( pFileServer , pTFileSourceUser );
		}
		else
		{
			DebugLogString( TRUE , "[OnTFileServerProtocolRcvAccountVerify] failed to find file source user error!!" );
		}
	}
	else
	{
		DebugLogString( TRUE , "[OnTFileServerProtocolRcvAccountVerify] TFileServer failed to verify client account!!" );
	}
}

/* json cmd from client */
IME_EXTERN_C	void	OnTFileServerProtocolRcvJsonCmd( IMeFileServer* pFileServer , uint32_t tfilePH , IMeNetSrcData* pNetSrcData )
{
	int nCmd = TFILEH_CMD(tfilePH);
	int nOff = 4; /* ph-4byte */

	json_object* json_object_tfile = json_tokener_parse( (const char*)&pNetSrcData->lpData[nOff] );
	
	if( !json_object_tfile )
	{
		DebugLogString( TRUE , "[OnTFileServerProtocolRcvJsonCmd] json_tokener_parse error!!" );
		return;
	}

	switch( nCmd )
	{
	case TFILE_C2S_ACCOUNT_VERIFY:
		OnTFileServerProtocolRcvAccountVerify( pFileServer , json_object_tfile , pNetSrcData->pDataSrc );
		break;
	case TFILE_C2S_ADD_FILES_INFO:
		OnTFileServerProtocolRcvFilesInfo( pFileServer , json_object_tfile , pNetSrcData->pDataSrc );
		break;
	case TFILE_C2S_ADD_FILE_START:
		OnTFileServerProtocolRcvFileStart( pFileServer , json_object_tfile , pNetSrcData->pDataSrc );
		break;
	case TFILE_C2S_ADD_FILE_END:
		OnTFileServerProtocolRcvFileEnd( pFileServer , json_object_tfile , pNetSrcData->pDataSrc );
		break;
	case TFILE_C2S_ADD_FILES_OVER:
		OnTFileServerProtocolFilesOver( pFileServer , json_object_tfile , pNetSrcData->pDataSrc );
		break;
	case TFILE_C2S_ADD_FILES_CANCEL:
		OnTFileServerProtocolFilesCancel( pFileServer , json_object_tfile , pNetSrcData->pDataSrc );
		break;
	}

	json_object_put( json_object_tfile );
}

/* binary data from client */
IME_EXTERN_C	void	OnTFileServerProtocolRcvBinary( IMeFileServer* pFileServer , uint32_t tfilePH , IMeNetSrcData* pNetSrcData )
{
	int nCmd = TFILEH_CMD(tfilePH);
	if( nCmd == TFILE_C2S_ADD_FILE_DATA )
	{
		int nOff = 4; /* ph-4byte */

		IMeTFileInfo* pTFileInfo = NULL;
		IMeTFileSourceUser* pTFileSourceUser = NULL;
		
		pTFileSourceUser = (IMeTFileSourceUser*)CArrayFindData(pFileServer->m_arrFileSourceClient,(uint32_t)pNetSrcData->pDataSrc);
		if( !pTFileSourceUser )
		{
			DebugLogString( TRUE , "[OnTFileServerProtocolRcvBinary] failed find tfile source client!!" );
			return;
		}
		
		pTFileSourceUser->m_dwLastRcvDataTime = IMeGetCurrentTime();
		
		pTFileInfo = (IMeTFileInfo*)CArrayFindData( pTFileSourceUser->m_listFile , (uint64_t)pTFileSourceUser->m_fileCurTID );
		if( pTFileInfo && pTFileInfo->m_fileCurFd && CFileIsOpen(pTFileInfo->m_fileCurFd) )
		{
			if( pTFileInfo->m_fileOffset < pTFileInfo->m_fileSize )
			{
				CFileWrite( pTFileInfo->m_fileCurFd , &pNetSrcData->lpData[nOff] , pNetSrcData->nLen-nOff );
				pTFileInfo->m_fileOffset += (pNetSrcData->nLen-nOff);
			}
			else
			{
				DebugLogString( TRUE , "[OnTFileServerProtocolRcvBinary] rcv file data exception!!" );
			}
		}
		else
		{
			DebugLogString( TRUE , "[OnTFileServerProtocolRcvBinary] no open file or no find current transfer file id!!" );
		}
	}
}

/* user net blocked , update status */
IME_EXTERN_C	void	OnTFileServerProtocolBlocked( IMeFileServer* pFileServer , void* pDataSourceUser )
{
	IMeTFileSourceUser* pTFileSourceUser;

	pTFileSourceUser = (IMeTFileSourceUser*)CArrayRemove( pFileServer->m_arrFileSourceClient , (uint32_t)pDataSourceUser );

	//check file source status
	if( pTFileSourceUser )
	{
		IMeTFileSourceUserDestroy( pTFileSourceUser );
		DebugLogString( TRUE , "[OnTFileServerProtocolBlocked]" );
	}			
}

IME_EXTERN_C	void	OnTFileServerRcvNetData( IMeFileServer* pFileServer , IMeNetSrcData* pNetSrcData )
{
	CLock_Lock( pFileServer->m_lockClient );

	//rcv data
	if( NULL != pNetSrcData->lpData && pNetSrcData->nLen > 0 )
	{
		int nOff = 0;
		uint32_t ph = *(uint32_t*)&pNetSrcData->lpData[nOff];	
		int nFormat = TFILEH_DFORMAT(ph);
		int nAppClient = TFILEH_APP(ph);

		//json format data
		if( TFILE_DFORMAT_JSON == nFormat && TFILE_APP_CLIENT == nAppClient )
		{
			OnTFileServerProtocolRcvJsonCmd( pFileServer , ph , pNetSrcData );
		}
		//binary format data
		else if( TFILE_DFORMAT_BINARY == nFormat && TFILE_APP_CLIENT == nAppClient )
		{
			OnTFileServerProtocolRcvBinary( pFileServer , ph , pNetSrcData );
		}
		else
		{
			DebugLogString( TRUE , "[OnTFileServerRcvNetData] Can't Be Resolved Data Format From Remote Client!!" );
		}
	}
	//net blocked , check
	else
	{
		OnTFileServerProtocolBlocked( pFileServer , pNetSrcData->pDataSrc );
	}

	CLock_Unlock( pFileServer->m_lockClient );
}

/* data process thread */
IME_EXTERN_C	void	ThreadTFileServerProcessData( IMeFileServer* pFileServer )
{
	IMeNetSrcData* pNetSrcData;
	while( pFileServer->m_bRunning )
	{
		CEventWait( pFileServer->m_evClientData , 1000 );

		//get data packet
		CLock_Lock( pFileServer->m_lockerClientData );
		pNetSrcData = CListRemoveHead( pFileServer->m_listClientData );
		CLock_Unlock( pFileServer->m_lockerClientData );

		//process data packet
		if( pNetSrcData )
		{
            //DebugLogString( TRUE , "[OnFileServerProtocolRcvClientData] Remove SrcData:%x" , pNetSrcData );
			OnTFileServerRcvNetData( pFileServer , pNetSrcData );
			IMeNetSrcDataDestroy( pNetSrcData );
		}		
		
		//whether go-on process data
		if( CListGetCount( pFileServer->m_listClientData ) > 0 )
			CEventSet( pFileServer->m_evClientData );
		else
			CEventReset( pFileServer->m_evClientData );
	}
}


/* receive net data from client */
IME_EXTERN_C	void	OnFileServerProtocolRcvClientData( IMeFileServer* pFileServer , char* lpData , int nLen , void* rcvSource )
{
	IMeNetSrcData* pNetSrcData = IMeNetSrcDataCreate( pFileServer->m_nTransferProtocolType, lpData , nLen , rcvSource );
	if( pNetSrcData )
	{
		CLock_Lock( pFileServer->m_lockerClientData );
		CListAddTail( pFileServer->m_listClientData , pNetSrcData );
		//DebugLogString( TRUE , "[OnFileServerProtocolRcvClientData] Add SrcData:%x" , pNetSrcData );
		CLock_Unlock( pFileServer->m_lockerClientData );
		CEventSet( pFileServer->m_evClientData );
	}
}
