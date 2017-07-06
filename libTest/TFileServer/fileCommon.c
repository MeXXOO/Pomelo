#include	"fileCommon.h"
#include	"server.h"

IME_EXTERN_C	IMeTFileInfo*	IMeTFileInfoCreate( char* pFileName , uint64_t llFileSize , uint32_t nFileID )
{
	IMeTFileInfo* pTFileInfo = (IMeTFileInfo*)calloc(1,sizeof(IMeTFileInfo));
	if( pTFileInfo )
	{
		pTFileInfo->m_fileID = nFileID;
		pTFileInfo->m_fileName = strdup(pFileName);
		pTFileInfo->m_fileSize = llFileSize;
		pTFileInfo->m_fileStatus = TFILE_STATUS_WAITING;
	}
	return pTFileInfo;
}

IME_EXTERN_C	void	IMeTFileInfoDestroy( IMeTFileInfo* pTFileInfo )
{
	if( pTFileInfo->m_fileName )
		free( pTFileInfo->m_fileName );
	if( pTFileInfo->m_fileCurFd )
	{
		CFileClose(pTFileInfo->m_fileCurFd);
		CFileDestroy(pTFileInfo->m_fileCurFd);
	}
	free( pTFileInfo );
}

IME_EXTERN_C	void	IMeTFileSourceUserReleaseFileList( IMeTFileSourceUser* pTFileSourceUser )
{
    int i;
	IMeTFileInfo* pTfileInfo;

    for( i=0; i<CArrayGetSize(pTFileSourceUser->m_listFile); i++ )
    {
	    pTfileInfo = CArrayGetAt(pTFileSourceUser->m_listFile,i);
		IMeTFileInfoDestroy( pTfileInfo );	
    }
	CArrayRemoveAll( pTFileSourceUser->m_listFile );
}

IME_EXTERN_C	IMeTFileSourceUser*	IMeTFileSourceUserCreate( void* pFileSource , int nTransferProtocolType )
{
	IMeTFileSourceUser* pTFileSourceUser = (IMeTFileSourceUser*)calloc(1,sizeof(IMeTFileSourceUser));
	if( pTFileSourceUser )
	{
		pTFileSourceUser->m_nTransferProtocolType = nTransferProtocolType;
		if( nTransferProtocolType == TFileProtocol_Tcp )
			pTFileSourceUser->m_pFileSource = pFileSource;
		else
		{
			pTFileSourceUser->m_pFileSource = calloc(1,sizeof(socket_addr_t));
			memcpy( pTFileSourceUser->m_pFileSource , pFileSource , sizeof(socket_addr_t) );
		}
		pTFileSourceUser->m_fileCurTID = -1;
		pTFileSourceUser->m_listFile = CArrayCreate(SORT_INC);
	}
	return pTFileSourceUser;
}

IME_EXTERN_C	void	IMeTFileSourceUserDestroy( IMeTFileSourceUser* pTFileSourceUser )
{
	IMeSocketTcp* pSocketTcp;

	IMeTFileSourceUserReleaseFileList( pTFileSourceUser );

	CArrayDestroy( pTFileSourceUser->m_listFile );
    
	if( pTFileSourceUser->m_nTransferProtocolType == TFileProtocol_Tcp )
	{
		pSocketTcp = (IMeSocketTcp*)pTFileSourceUser->m_pFileSource;
		CSocketTcpDestroy( pSocketTcp );
	}
	else
		free( pTFileSourceUser->m_pFileSource );

	free( pTFileSourceUser );
}