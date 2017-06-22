#ifndef _TFILE_COMMON_H_
#define	_TFILE_COMMON_H_

#include	"include.h"

#define	TFILE_STATUS_WAITING	0	/* waiting transfer */
#define	TFILE_STATUS_RUNNING	1	/* is transfering */
#define	TFILE_STATUS_OVER		2	/* transfer over */
#define TFILE_STATUS_FAILED		3	/* transfer failed */

//single file for transfer
typedef struct _IMeTFileInfo{
	char*	m_fileName;		/* include path */
	uint64	m_fileSize;		/* file size */
	uint64  m_fileOffset;	/* already transfered */
	uint	m_fileID;		/* file id for transfered */
	IMeFile* m_fileCurFd;	/* file fd */
	int		m_fileStatus;	/* file transfered status */
	uint	m_fileLastNotifytime;	/* last notify start transfer file time */
}IMeTFileInfo;

IME_EXTERN_C IMeTFileInfo*	IMeTFileInfoCreate( const char* pFileName , uint64 llFileSize , uint nFileID );
IME_EXTERN_C	void	IMeTFileInfoDestroy( IMeTFileInfo* pTFileInfo );

//single upload file
typedef	struct _IMeTFileSource
{
	/* waiting transfered file */
	IMeArray*	m_arrFile;

	/* current transfer file status */
	byte	m_curUploadStatus;

	/* last commit file info time */
	uint	m_lastCommitInfoTime;

	/* up-level directory in current file info */
	char	m_szMainDir[256];	

	/* current is uploading sub file id */
	int		m_nCurUploadFileID;

    /* statistic transfer data rate */
    uint    m_dwStartTransferTime;
}IMeTFileSource;

IME_EXTERN_C	void	IMeTFileSourceReleaseFileList( IMeArray* arrFile );
IME_EXTERN_C    void    IMeTFileSourceStatisticTransferRate( IMeTFileSource* pFileSource );
IME_EXTERN_C	IMeTFileSource*	IMeTFileSourceCreate( IMeArray* pArrayFile , char* pMainDir );
IME_EXTERN_C	void	IMeTFileSourceDestroy( IMeTFileSource* pFileSource );

#endif	//end _FILE_COMMON_H_
