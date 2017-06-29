#include    "../include/include.h"


#ifdef      PROJECT_FOR_WINDOWS

typedef struct _IMeCFile{
    IMeFile interfacefunction;
    char*   m_pFileName;
    HANDLE  m_hFile;
}IMeCFile;

int     IMeCFileOpen( IMeFile* pIFile , char* pFileName , int nOpenFlag )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    
    if( !pFile || !pFileName || !nOpenFlag )    return 0;
    
    while( pFile->m_hFile==NULL )
    {
        uint32_t dwOpenAttribute = 0;
        if( nOpenFlag & IMeFile_OpenCreate )
            dwOpenAttribute |= CREATE_ALWAYS;
        else 
            dwOpenAttribute |= OPEN_EXISTING;

        if( nOpenFlag&IMeFile_OpenWrite || nOpenFlag&IMeFile_OpenRead )
            pFile->m_hFile = CreateFile( pFileName , FILE_GENERIC_READ|FILE_GENERIC_WRITE /*FILE_ALL_ACCESS*/ , FILE_SHARE_READ , NULL , dwOpenAttribute , FILE_ATTRIBUTE_NORMAL , NULL );

        if( pFile->m_hFile==INVALID_HANDLE_VALUE )
        {   
            DebugLogString( TRUE , "[IMeFileOpen] Open File %s Failed ErrorString:%s!!" , pFileName , _ErrorCodeString() );
            pFile->m_hFile = NULL;
            break;
        }

        if( nOpenFlag&IMeFile_WriteAppend )
        {
            int lLowSize = 0 , lHightSize = 0;
            SetFilePointer( pFile->m_hFile , lLowSize , &lHightSize , FILE_END );
        }

        pFile->m_pFileName = IMeCopyString(pFileName);
        if( !pFile->m_pFileName )
        {
            CloseHandle(pFile->m_hFile);
            pFile->m_hFile = NULL;
            DebugLogString( TRUE , "[IMeFileOpen] copy file name failed!!" );
            break;
        }
        break;
    }

    return pFile->m_hFile!=NULL;
}

int     IMeCFileIsOpen( IMeFile* pIFile )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    
    if( !pFile )    return 0;

    return pFile->m_hFile!=NULL;
}

uint32_t    IMeCFileRead( IMeFile* pIFile , char* pBuffer , uint32_t nBufferLen )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    int nReadLen = 0;
    
    if( !pFile || !pFile->m_hFile || !pBuffer || !nBufferLen )    return nReadLen;

    if( !ReadFile( pFile->m_hFile , pBuffer , nBufferLen , &nReadLen , NULL ) )
    {
        nReadLen = 0;
        DebugLogString( TRUE , "[IMeFileRead] read file %s failed!!" , pFile->m_pFileName );
    }

    return nReadLen;
}

uint32_t     IMeCFileWrite( IMeFile* pIFile , char* pBuffer , uint32_t nBufferLen )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    uint32_t nWriteLen = 0;

    if( !pFile || !pBuffer || !nBufferLen || !pFile->m_hFile ) return nWriteLen;

    if( !WriteFile( pFile->m_hFile , pBuffer , nBufferLen , &nWriteLen , NULL ) )
    {
        nWriteLen = 0;
        DebugLogString( TRUE , "[IMeFileWrite] write file %s failed!!" , pFile->m_pFileName );
    }

    return nWriteLen;
}

int     IMeCFileSeek( IMeFile* pIFile , int64_t llPos , int nSeekFlag )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;

    int lLoSeekSize = (int)(llPos&0xffffffff);
    int lHiSeekSize = (int)((llPos>>32)&0xffffffff);
    uint32_t nRes = HFILE_ERROR;
    
    if( !pFile || !pFile->m_hFile ) return 0;   
    
    if( nSeekFlag == IMeFile_SeekCur )
    {
        nRes = SetFilePointer( pFile->m_hFile , lLoSeekSize , &lHiSeekSize , FILE_CURRENT );
    }
    else if( nSeekFlag == IMeFile_SeekBegin )
    {
        nRes = SetFilePointer( pFile->m_hFile , lLoSeekSize , &lHiSeekSize , FILE_BEGIN );
    }
    else if( nSeekFlag == IMeFile_SeekEnd )
    {
        nRes = SetFilePointer( pFile->m_hFile , lLoSeekSize , &lHiSeekSize , FILE_END );
    }
    
    if( nRes==HFILE_ERROR )
    {
        DebugLogString( TRUE , "[IMeFileSeek] seek file error!!" );
    }

    return nRes!=HFILE_ERROR;
}

uint64_t  IMeCFileGetSize( IMeFile* pIFile )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    uint32_t lLoSize = 0 , lHiSize = 0;

    if( !pFile || !pFile->m_hFile )     return 0;

    lLoSize = GetFileSize( pFile->m_hFile , &lHiSize );

    return (((uint64_t)lHiSize)<<32|lLoSize);
}

int64_t  IMeCFileGetPosition( IMeFile* pIFile )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    uint32_t lPostion = HFILE_ERROR;
    
    if( !pFile || !pFile->m_hFile )     return 0;

    lPostion = SetFilePointer( pFile->m_hFile , 0 , NULL , FILE_CURRENT );

    if( lPostion==HFILE_ERROR )
    {
        DebugLogString( TRUE , "[IMeFileGetPosition] get file %s position failed!!" , pFile->m_pFileName );
        lPostion = 0;
    }

    return lPostion;
}

void    IMeCFileClose( IMeFile* pIFile )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;

    if( !pFile || !pFile->m_hFile ) return;

    CloseHandle( pFile->m_hFile );
    pFile->m_hFile = NULL;
    free( pFile->m_pFileName );
}

void    IMeCFileDestroy( IMeFile* pIFile )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    
    if( !pFile ) return;

    IMeCFileClose( pIFile );

    free( pFile );
}

IME_EXTERN_C IMeFile*    IMeFileCreate()
{
    IMeCFile* pFile = (IMeCFile*)calloc(1,sizeof(IMeCFile));
    
    if( pFile )
    {
        ((IMeFile*)pFile)->m_pClose = IMeCFileClose;
        ((IMeFile*)pFile)->m_pDestroy = IMeCFileDestroy;
        ((IMeFile*)pFile)->m_pGetPosition = IMeCFileGetPosition;
        ((IMeFile*)pFile)->m_pGetSize = IMeCFileGetSize;
        ((IMeFile*)pFile)->m_pIsOpen = IMeCFileIsOpen;
        ((IMeFile*)pFile)->m_pOpen = IMeCFileOpen;
        ((IMeFile*)pFile)->m_pRead = IMeCFileRead;
        ((IMeFile*)pFile)->m_pSeek = IMeCFileSeek;
        ((IMeFile*)pFile)->m_pWrite = IMeCFileWrite;
    }
    else
    {
        DebugLogString( TRUE , "[IMeFileCreate] calloc failed!!" );
    }

    return (IMeFile*)pFile;
}

#elif       PROJECT_FOR_LINUX

typedef struct _IMeCFile{
    IMeFile interfacefunction;
    char*   m_pFileName;
    int     m_hFile;
}IMeCFile;

int  IMeCFileOpen( IMeFile* pIFile , char* pFileName , int nOpenFlag )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    
    if( !pFile || !pFileName || !nOpenFlag ) return 0;

    while( -1==pFile->m_hFile )
    {
        int nOpenMode = 0;
        
        if( nOpenFlag&IMeFile_OpenWrite && nOpenFlag&IMeFile_OpenRead )
            nOpenMode = O_RDWR;
        else if( nOpenFlag&IMeFile_OpenRead )
            nOpenMode = O_RDONLY;
        else if( nOpenFlag&IMeFile_OpenWrite )
            nOpenMode = O_WRONLY;

        if( nOpenFlag&IMeFile_WriteAppend )
            nOpenMode |= O_APPEND;

        if( nOpenFlag&IMeFile_OpenCreate )
            nOpenMode |= (O_CREAT|O_TRUNC/*文件已存在则删除文件内容*/);
        
        pFile->m_hFile = open( pFileName , nOpenMode );
        if( pFile->m_hFile==-1 )
        {
            DebugLogString( TRUE , "[IMeFileOpen] open File %s failed!" , pFileName );
            pFile->m_hFile = -1;
            break;
        }

        pFile->m_pFileName = IMeCopyString( pFileName );
        if( !pFile->m_pFileName )
        {
            DebugLogString( TRUE , "[IMeFileOpen] copy file name failed!" );
            close( pFile->m_hFile );
            pFile->m_hFile = -1;
        }
        break;
    }

    return pFile->m_hFile != -1;
}

int  IMeCFileIsOpen( IMeFile* pIFile )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    
    if( !pFile )    return 0;

    return pFile->m_hFile!=-1;
}

uint32_t IMeCFileRead( IMeFile* pIFile , char* pBuffer , uint32_t nBufferLen )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    uint32_t nReadLen = 0;
    
    if( !pFile || -1==pFile->m_hFile || !pBuffer || !nBufferLen )  return nReadLen;
    
    nReadLen = read( pFile->m_hFile , (void*)pBuffer , nBufferLen );
    if( nReadLen==-1 )
    {
        DebugLogString( TRUE , "[IMeFileRead] read file %s failed!" , pFile->m_pFileName );
        nReadLen = 0;
    }

    return nReadLen;
}

uint32_t IMeCFileWrite( IMeFile* pIFile , char* pBuffer , uint32_t nBufferLen )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    uint32_t nWriteLen = 0;
    
    if( !pFile || -1==pFile->m_hFile || !pBuffer || !nBufferLen )  return nWriteLen;
    
    nWriteLen = write( pFile->m_hFile , (void*)pBuffer , nBufferLen );
    if( nWriteLen==-1 )
    {
        DebugLogString( TRUE , "[IMeFileWrite] write file %s failed!" , pFile->m_pFileName );
        nWriteLen = 0;
    }
    
    return nWriteLen;
}

int  IMeCFileSeek( IMeFile* pIFile , int64_t llPos , int nSeekFlag )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    int64_t nRes = -1;
    if( !pFile || -1==pFile->m_hFile )    return 0;
    
    if( nSeekFlag==IMeFile_SeekEnd )
    {
        nRes = lseek( pFile->m_hFile , llPos , SEEK_END );
    }
    else if( nSeekFlag==IMeFile_SeekCur )
    {
        nRes = lseek( pFile->m_hFile , llPos , SEEK_CUR );
    }
    else if( nSeekFlag==IMeFile_SeekBegin )
    {
        nRes = lseek( pFile->m_hFile , llPos , SEEK_SET );
    }

    if( nRes==-1 )
    {
        DebugLogString( TRUE , "[IMeFileSeek] seek file %s failed!!" , pFile->m_pFileName );
    }

    return nRes!=-1;
}

uint64_t IMeCFileGetSize( IMeFile* pIFile )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    int64_t llSize = -1;
    if( !pFile || -1==pFile->m_hFile )    return 0;
    
    llSize = lseek( pFile->m_hFile, 0, SEEK_END );
    if( llSize==-1 )
    {
        DebugLogString( TRUE , "[IMeFileGetSize] get file %s size failed!" , pFile->m_pFileName );
        llSize = 0;
    }

    return llSize;
}

int64_t IMeCFileGetPosition( IMeFile* pIFile )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    int64_t llPos = -1;
    if( !pFile || -1==pFile->m_hFile )    return 0;
    
    llPos = lseek( pFile->m_hFile, 0, SEEK_CUR );
    if( llPos==-1 )
    {
        DebugLogString( TRUE , "[IMeCFileGetPosition] get file %s position failed!" , pFile->m_pFileName );
        llPos = 0;
    }
    
    return llPos;
}

void IMeCFileClose( IMeFile* pIFile )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    if( !pFile || -1==pFile->m_hFile )    return;
    close( pFile->m_hFile );
    pFile->m_hFile = 0;
    free( pFile->m_pFileName );
    pFile->m_pFileName = NULL;
}

void IMeCFileDestroy( IMeFile* pIFile )
{
    IMeCFile* pFile = (IMeCFile*)pIFile;
    if( !pFile )    return;
    IMeCFileClose( pIFile );
    free( pFile );
}

IME_EXTERN_C IMeFile*    IMeFileCreate()
{
    IMeCFile* pFile = (IMeCFile*)calloc(1,sizeof(IMeCFile));
    
    if( pFile )
    {
        ((IMeFile*)pFile)->m_pClose = IMeCFileClose;
        ((IMeFile*)pFile)->m_pDestroy = IMeCFileDestroy;
        ((IMeFile*)pFile)->m_pGetPosition = IMeCFileGetPosition;
        ((IMeFile*)pFile)->m_pGetSize = IMeCFileGetSize;
        ((IMeFile*)pFile)->m_pIsOpen = IMeCFileIsOpen;
        ((IMeFile*)pFile)->m_pOpen = IMeCFileOpen;
        ((IMeFile*)pFile)->m_pRead = IMeCFileRead;
        ((IMeFile*)pFile)->m_pSeek = IMeCFileSeek;
        ((IMeFile*)pFile)->m_pWrite = IMeCFileWrite;
    }
    else
    {
        DebugLogString( TRUE , "[IMeFileCreate] calloc failed!!" );
    }
    
    return (IMeFile*)pFile;
}

#endif
