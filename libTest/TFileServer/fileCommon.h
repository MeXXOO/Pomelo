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
	uint64_t	m_fileSize;		/* file size */
	uint64_t  m_fileOffset;	/* already transfered */
	uint32_t	m_fileID;		/* file id for transfered */
	IMeFile* m_fileCurFd;	/* file fd */
	int		m_fileStatus;	/* file transfered status */
}IMeTFileInfo;

IME_EXTERN_C IMeTFileInfo*	IMeTFileInfoCreate( char* pFileName , uint64_t llFileSize , uint nFileID );
IME_EXTERN_C	void	IMeTFileInfoDestroy( IMeTFileInfo* pTFileInfo );

//tcp file client
typedef	struct _IMeTFileSourceUser
{
	/* remote data source */
	void*	m_pFileSource;
	int		m_nTransferProtocolType;

	/* last receive data time, avoid DDoS */
	uint32_t m_dwLastRcvDataTime;

	/* whether through account check */
	uint8_t	m_bVerify;

	/* waiting transfered file */
	IMeArray*	m_listFile;

	/* current transfer file id */
	uint	m_fileCurTID;

}IMeTFileSourceUser;

IME_EXTERN_C	void	IMeTFileSourceUserReleaseFileList( IMeTFileSourceUser* pFileSourceUser );
IME_EXTERN_C	IMeTFileSourceUser*	IMeTFileSourceUserCreate( void* pFileSource , int nTransferProtocolType );
IME_EXTERN_C	void	IMeTFileSourceUserDestroy( IMeTFileSourceUser* pFileSourceUser );

#endif	//end _FILE_COMMON_H_
