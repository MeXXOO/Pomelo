#include	"fileCommon.h"

IME_EXTERN_C	IMeTFileInfo*	IMeTFileInfoCreate( const char* pFileName , uint64_t llFileSize , uint32_t nFileID , uint8_t bIsDir )
{
	IMeTFileInfo* pTFileInfo = (IMeTFileInfo*)calloc(1,sizeof(IMeTFileInfo));
	if( pTFileInfo )
	{
		pTFileInfo->m_fileID = nFileID;
		pTFileInfo->m_fileName = strdup(pFileName);
		pTFileInfo->m_fileIsDir = bIsDir;
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

IME_EXTERN_C	void	IMeTFileSourceReleaseFileList( IMeArray* arrFile )
{
	IMeTFileInfo* pTfileInfo;

	while( (pTfileInfo = (IMeTFileInfo*)CArrayRemoveAt(arrFile,0)) )
	{
		IMeTFileInfoDestroy( pTfileInfo );	
	}

	CArrayRemoveAll( arrFile );
}


IME_EXTERN_C    void     IMeTFileSourceStatisticTransferRate( IMeTFileSource* pFileSource )
{
    int i;
    uint64_t totalSize = 0;
    uint32_t dwUseTime;

    for( i = 0; i<CArrayGetSize(pFileSource->m_arrFile); i++ )
    {
        IMeTFileInfo* pTFileInfo = (IMeTFileInfo*)CArrayGetAt(pFileSource->m_arrFile,i);
        totalSize += pTFileInfo->m_fileSize;
    }
    
    dwUseTime = IMeGetCurrentTime() - pFileSource->m_dwStartTransferTime;
    if( dwUseTime < 0 ) dwUseTime = 1;

    if( totalSize >= 10*1024*1024 )
    {
        totalSize = totalSize>>20;
        DebugLogString( TRUE, "[IMeTFileSourceStatisticTransferRate] totalSize:%uMB usetime:%us %.2fMB/s", (uint32_t)totalSize, dwUseTime/1000, ((uint32_t)totalSize*1000)*1.0 / dwUseTime );
    }
    else
    {
        totalSize = totalSize>>10;
        DebugLogString( TRUE, "[IMeTFileSourceStatisticTransferRate] totalSize:%uKB/s usetime:%us %.2fKB/s", (uint32_t)totalSize, dwUseTime/1000, ((uint32_t)totalSize*1000)*1.0 / dwUseTime );
    }
}

IME_EXTERN_C	IMeTFileSource*	IMeTFileSourceCreate( IMeArray* pArrayFile , char* pMainDir )
{
	IMeTFileSource* pTFileSource = (IMeTFileSource*)calloc(1,sizeof(IMeTFileSource));
	
	if( pTFileSource )
	{
		pTFileSource->m_curUploadStatus = TFILE_STATUS_WAITING;
		pTFileSource->m_nCurUploadFileID = -1;
		pTFileSource->m_arrFile = pArrayFile;
		strcpy( pTFileSource->m_szMainDir , pMainDir );
	}

	return pTFileSource;
}

IME_EXTERN_C	void	IMeTFileSourceDestroy( IMeTFileSource* pTFileSource )
{
	IMeTFileSourceReleaseFileList( pTFileSource->m_arrFile );
	
	CArrayDestroy( pTFileSource->m_arrFile );

	free( pTFileSource );
}
