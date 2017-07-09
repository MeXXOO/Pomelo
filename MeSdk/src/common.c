#include    "../include/include.h"

#ifdef PROJECT_FOR_WINDOWS
#define ACCESS _access  
#define MKDIR(a) _mkdir((a))  
#elif PROJECT_FOR_LINUX  
#define ACCESS access  
#define MKDIR(a) mkdir((a),0755)
#elif PROJECT_FOR_IOS
#define ACCESS access
#define MKDIR(a) mkdir((a),0755)
#endif 

IME_EXTERN_C char*   IMeCopyString( const char* pString )
{
    char* pNewString = NULL;
    
    if( !pString || !strlen(pString) )  return pNewString;
    
    pNewString = calloc( 1 , strlen(pString)+1 );
    
    if( pNewString )
    {
        strcpy( pNewString , pString );
    }
    else
    {
        DebugLogString( TRUE , "[IMeCopyString] calloc failed!!" );
    }

    return pNewString;
}

IME_EXTERN_C void	IMeGetUpLevelFilePath( const char* szInputPath , char* szOutputPath , int nLevel )
{
	char szSrcPath[256];
	char* pFilePos;

	sprintf( szSrcPath , "%s" , szInputPath );

	while( nLevel != 0 && (pFilePos = strrchr(szSrcPath , FILE_PATH_SEPARATE)) != NULL )
	{
		nLevel--;
		*(pFilePos+1) = '\0';
	}
	
	sprintf( szOutputPath , "%s" , szSrcPath );
}

IME_EXTERN_C  char   IMeCreateDirectory( const char* pDir )
{
    int i = 0;  
    int iRet;
#ifdef PROJECT_FOR_IOS
  size_t iLen;
#else
    int iLen;
#endif
    char* pszDir;
    
    if( NULL == pDir )  
        return FALSE;  
    
    pszDir = strdup(pDir);  
    iLen = strlen(pszDir);  
    
    for( i = 0; i < iLen; i++ )  
    {  
        if( pszDir[i] == FILE_PATH_SEPARATE && i != 0/* ignore start pos separate */ )  
        {   
            pszDir[i] = '\0';  
            
            //if not exist, create it 
            iRet = ACCESS(pszDir,0);  
            if( iRet != 0 )  
            {  
                iRet = MKDIR(pszDir);  
                if (iRet != 0)
                {    
                    return FALSE;  
                }
            }  
            //recover  
            pszDir[i] = FILE_PATH_SEPARATE;  
        }   
    }  
    
    iRet = ACCESS(pszDir,0);
    if( iRet != 0 )    
        iRet = MKDIR(pszDir);

    free(pszDir);
    
    return (iRet==0);  
}

#ifdef  PROJECT_FOR_WINDOWS     //////////////////////////////////////////////////////////////////////////

#pragma comment(lib,"Rpcrt4.lib")

IME_EXTERN_C int	IMeFileIsDir( const char* pPath )
{
	uint32_t dwAttr = GetFileAttributes(pPath);

	if( -1 == dwAttr )
		return -1;

	if( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
		return 1;
	else
		return 0;
}

IME_EXTERN_C uint64_t IMeGetHardDiskSize( const char* szDiskPath )
{   
    ULARGE_INTEGER lpuse;   
    ULARGE_INTEGER lptotal;   
    ULARGE_INTEGER lpfree;   
    
    if( !GetDiskFreeSpaceEx( szDiskPath , &lpuse , &lptotal , &lpfree ) )
    {
        DebugLogString( TRUE , "[IMeGetHardDiskSize] Failed to Hard Disk Size!" );
        return 0;
    }
    
    return lpfree.QuadPart>>20;  /* MB */
}

IME_EXTERN_C uint64_t  IMeGetFileSize( const char* pFilePathName )
{
	uint64_t llSize = 0;

	HANDLE handle = CreateFile( pFilePathName, FILE_READ_EA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );
    if( handle != INVALID_HANDLE_VALUE )
    {
		uint32_t highSize = 0;
        uint32_t lowSize = GetFileSize( handle, &highSize );
		llSize = (uint64_t)highSize<<32|lowSize;
        CloseHandle(handle);
    }
	
	return llSize;
}

IME_EXTERN_C uint64_t IMeGetFileDirSize( const char* szFileDir )
{
    char szFilePath[256] = { 0 };
    char szFileFilter[256] = { 0 };
    uint64_t llSize = 0;
    WIN32_FIND_DATA fileinfo;
    HANDLE hFind = NULL; 

    strcpy( szFilePath , szFileDir );

    if( szFilePath[strlen(szFilePath)-1] != '\\' )
        strcat( szFilePath , "\\" );

    strcpy( szFileFilter , szFilePath );
    strcat( szFileFilter , "*.*" );
    
    hFind = FindFirstFile( szFileFilter, &fileinfo );
    do{
        /* skip . & .. dir */
        if( (fileinfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) 
            && strcmp(fileinfo.cFileName,".") 
            && strcmp(fileinfo.cFileName,"..") )
        {
            char szSubPath[256] = { 0 };
            strcpy( szSubPath , szFilePath );
            strcat( szSubPath , fileinfo.cFileName );
            llSize = llSize + IMeGetFileDirSize(szSubPath);
        }
        else
        {
            llSize = llSize + (((uint64_t)fileinfo.nFileSizeHigh)<<32|fileinfo.nFileSizeLow);
        }
    }while(FindNextFile(hFind,&fileinfo));

    FindClose(hFind);
    return llSize;
}


IME_EXTERN_C void   IMeGetSubDirList( const char* szMainDir , IMeArray* arrSubDir )
{
    char szFilePath[256] = { 0 };
    char szFileFilter[256] = { 0 };
    WIN32_FIND_DATA fileinfo;
    HANDLE hFind = NULL;

	if( IMeFileIsDir( szMainDir ) <= 0 )	return;

    strcpy( szFilePath , szMainDir );
    
    if( szFilePath[strlen(szFilePath)-1] != '\\' )
        strcat( szFilePath , "\\" );
    
    strcpy( szFileFilter , szFilePath );
    strcat( szFileFilter , "*.*" );
    
    hFind = FindFirstFile( szFileFilter,&fileinfo );
    do{
        /* skip . & .. dir */
        if( (fileinfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) 
            && strcmp(fileinfo.cFileName,".") 
            && strcmp(fileinfo.cFileName,"..") )
        {
            CArrayAdd( arrSubDir , IMeCopyString(fileinfo.cFileName) , 0 );
        }
    }while(FindNextFile(hFind,&fileinfo));
    
    FindClose(hFind);
}


IME_EXTERN_C void	 IMeGetSubDirFileList( const char* szMainDir , IMeArray* arrSubDirFile , uint8_t bFullPath , uint8_t bIncludeDir )
{
    char szFilePath[256] = { 0 };
    char szFileFilter[256] = { 0 };
    
    WIN32_FIND_DATA fileinfo;
    HANDLE hFind = NULL;

	if( IMeFileIsDir( szMainDir ) <= 0 )	return;

    strcpy( szFilePath , szMainDir );

    if( szFilePath[strlen(szFilePath)-1] != '\\' )
        strcat( szFilePath , "\\" );

    strcpy( szFileFilter , szFilePath );
    strcat( szFileFilter , "*.*" );
    
    hFind = FindFirstFile( szFileFilter, &fileinfo );
    do{
		char szSubPath[256] = { 0 };
		
		strcpy( szSubPath , szFilePath );
		strcat( szSubPath , fileinfo.cFileName );
        
        /* skip . & .. dir */
        if( !stricmp(fileinfo.cFileName,".") || !stricmp(fileinfo.cFileName,"..") )
        {
            continue;
        }
        else if( (fileinfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
        {
            if( bIncludeDir )
            {
                CArrayAdd( arrSubDirFile , strdup(szSubPath) , 0 );
            }
			IMeGetSubDirFileList( szSubPath , arrSubDirFile , bFullPath );
        }
        else
        {
			if( bFullPath )
				CArrayAdd( arrSubDirFile , strdup(szSubPath) , 0 );
			else
				CArrayAdd( arrSubDirFile , strdup(fileinfo.cFileName) , 0 );
        }
    }while(FindNextFile(hFind,&fileinfo));

    FindClose(hFind);
}

IME_EXTERN_C void    IMeCopyDirFile( const char* szSrcDir , const char* szDstDir )
{
    char szFilePath[256] = { 0 };
    char szFileFilter[256] = { 0 };
    WIN32_FIND_DATA fileinfo;
    HANDLE hFind = NULL;

    strcpy( szFilePath , szSrcDir );
    
    if( szFilePath[strlen(szFilePath)-1] != '\\' )
        strcat( szFilePath , "\\" );
    
    strcpy( szFileFilter , szFilePath );
    strcat( szFileFilter , "*.*" );
    
    hFind = FindFirstFile( szFileFilter,&fileinfo );
    do{
        /* skip . & .. dir */
        if( !(fileinfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
        {
            char szDstFileName[256] = { 0 };
            char szSrcFileName[256] = { 0 };
            sprintf( szDstFileName , "%s%s" , szDstDir , fileinfo.cFileName );
            sprintf( szSrcFileName , "%s%s" , szSrcDir , fileinfo.cFileName );
            //DebugLogString( TRUE , "[IMeCopyDirFile] szSrcFileName = %s , szDstFileName = %s\n" , szSrcFileName , szDstFileName );
            IMeCopyFile( szSrcFileName , szDstFileName );
        }
    }while(FindNextFile(hFind,&fileinfo));
    
    FindClose(hFind);
}

IME_EXTERN_C void    IMeCopyFile( const char* pSrcFile , const char* pDstFile )
{
    if( !pSrcFile || !pDstFile )    return;
    CopyFile( pSrcFile , pDstFile , FALSE );
}


IME_EXTERN_C int IMeKillProcess( const char* szProcessName , uint32_t dwPID )    
{
    HANDLE hSnapShot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );  
    PROCESSENTRY32 pe;  
    pe.dwSize = sizeof(PROCESSENTRY32);  
    if( !Process32First(hSnapShot,&pe) )  
    {  
        return 0;  
    }  

    while( Process32Next(hSnapShot,&pe) )  
    {  
        char szExeName[256] = { 0 };
        sprintf( szExeName, "%s", pe.szExeFile );
        
        if( !stricmp(szExeName , szProcessName) || (dwPID==pe.th32ProcessID && dwPID!=0) )  
        {  
            DWORD dwProcessID = pe.th32ProcessID;  
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,FALSE,dwProcessID);  
            TerminateProcess(hProcess,0);  
            CloseHandle(hProcess);  
        }  
    }  
    
    return 1;  
}

IME_EXTERN_C void    IMeRestartSystem()                                                    
{
    char szShellCmd[256] = { 0 };

    sprintf( szShellCmd , "shutdown /r /f /t 0" );
    
    system( szShellCmd );
}

IME_EXTERN_C void    IMePowerOffSystem()                                                   
{
    char szShellCmd[256] = { 0 };
    
    sprintf( szShellCmd , "shutdown /s /f /t 0" );
    
    system( szShellCmd );
}

/* create guid in windows os */
IME_EXTERN_C void    IMeCreateGUID( char* szGuid )
{
    char* pszGuid = NULL;
    UUID guid;
    int i = 0;

    if( !szGuid )   return;
    
    UuidCreate( &guid );
    UuidToString( &guid, (uint8_t**)&pszGuid );
    
    while( pszGuid[i]!='\0' )
    {
        if( isalpha(pszGuid[i]) )
        {
            szGuid[i] = toupper(pszGuid[i]);
        }
        else
            szGuid[i] = pszGuid[i];
        i++;
    }
    RpcStringFree((uint8_t**)&pszGuid);
}

IME_EXTERN_C uint32_t    IMeGetCurrentTime()
{
    return timeGetTime();
}

IME_EXTERN_C void    IMeGetCurrentTimeYMDHMS( IMeTime* pMeTime )
{
    SYSTEMTIME st; 
    GetLocalTime(&st);
    pMeTime->year = st.wYear;
    pMeTime->month = st.wMonth;
    pMeTime->day = st.wDay;
    pMeTime->hour = st.wHour;
    pMeTime->minute = st.wMinute;
    pMeTime->second = st.wSecond;
}

IME_EXTERN_C void    IMeSleep( uint32_t dwMiliseconds )
{
    Sleep( dwMiliseconds );
}

#elif       PROJECT_FOR_LINUX   //////////////////////////////////////////////////////////////////////////

IME_EXTERN_C int	IMeFileIsDir( const char* pPath )
{   
    struct stat S_stat;  
    
    if( lstat(pPath, &S_stat)<0 )  
    {  
        return -1;  
    }  
    
    if( S_ISDIR(S_stat.st_mode) )  
    {  
        return TRUE;  
    }  
    else  
    {  
        return FALSE;  
    }  
}  

IME_EXTERN_C void    IMeCopyFile( const char* pSrcFile , const char* pDstFile )
{
    char cmdline[1024] = { 0 };
    if( pSrcFile && pDstFile )
    {
        sprintf( cmdline , "cp %s %s", pSrcFile , pDstFile );
        system(cmdline);
    }
}

IME_EXTERN_C uint64_t  IMeGetFileSize( const char* pFilePathName )
{
	int fd = 0;
	FILE* pFile = NULL;

	uint64_t llSize = 0;
    struct stat64 filestat;

	pFile = fopen( pFilePathName, "r" );
	if( pFile && (fd = fileno( pFile )) )
	{
		fstat64( fd , &filestat );
		llSize += filestat.st_size;
		fclose(pFile);
	}

	return llSize;
}

IME_EXTERN_C uint64_t IMeGetFileDirSize( const char* szFileDir )  
{  
    char szFilePath[256] = { 0 };
    strcpy( szFilePath , szFileDir );
    
    if( szFilePath[strlen(szFilePath)-1] != '/' )
        strcat( szFilePath , "/" );
    
    DIR* pDir = opendir(szFilePath);
    if( pDir == NULL )  return 0;  

    uint64_t llSize = 0;
    struct dirent *pFileInfo;  
    struct stat64 filestat;          
    while( (pFileInfo=readdir(pDir)) != NULL )  
    {  
        char szSubPath[256] = { 0 };
        strcpy( szSubPath , szFilePath );
        strcat( szSubPath , pFileInfo->d_name );
        if( IMeFileIsDir(szSubPath)
            && strcmp(pFileInfo->d_name, ".") 
            && strcmp(pFileInfo->d_name, "..") )  
        {  
            llSize = llSize + IMeGetFileDirSize(szSubPath);    
        }  
        else if( strcmp(pFileInfo->d_name, ".") 
            && strcmp(pFileInfo->d_name, "..") )
        {
            int fd = -1;
            FILE* pFile = fopen( szSubPath, "r" );
            if( pFile && (fd = fileno( pFile )) )
            {
                fstat64( fd , &filestat );
                llSize += filestat.st_size;
                fclose(pFile);
            }
        }
    }  
    
    closedir(pDir);  
    
    return llSize;  
}  

IME_EXTERN_C void  IMeGetSubDirList( const char* szMainDir , IMeArray* arrSubDir ) 
{  
    char szFilePath[256] = { 0 };
    strcpy( szFilePath , szMainDir );
    
    if( szFilePath[strlen(szFilePath)-1] != '/' )
        strcat( szFilePath , "/" );
    
    DIR* pDir = opendir(szFilePath);
    if( pDir == NULL )  return;  
    
    struct dirent *pFileInfo;  
    while( (pFileInfo=readdir(pDir)) != NULL )  
    {  
        char szSubPath[256] = { 0 };
        strcpy( szSubPath , szFilePath );
        strcat( szSubPath , pFileInfo->d_name );
        if( IMeFileIsDir(szSubPath)==TRUE
            && strcmp(pFileInfo->d_name, ".") 
            && strcmp(pFileInfo->d_name, ".."))  
        {  
            CArrayAdd( arrSubDir , IMeCopyString(pFileInfo->d_name) , 0 );
        }  
    }  
    closedir(pDir);  
}  


IME_EXTERN_C void	 IMeGetSubDirFileList( const char* szMainDir , IMeArray* arrSubDirFile , uint8_t bFullPath , uint8_t bIncludeDir )
{
    char szFilePath[256] = { 0 };
    strcpy( szFilePath , szMainDir );
    
    if( szFilePath[strlen(szFilePath)-1] != '/' )
        strcat( szFilePath , "/" );
    
    DIR* pDir = opendir(szFilePath);
    if( pDir == NULL )  return;  

    struct dirent *pFileInfo;  
    while( (pFileInfo=readdir(pDir)) != NULL )  
    {  
        char szSubPath[256] = { 0 };
        
		strcpy( szSubPath , szFilePath );
        strcat( szSubPath , pFileInfo->d_name );

        if( !strcmp(pFileInfo->d_name, ".") || !strcmp(pFileInfo->d_name, "..") )
        {
           continue;
        }
        else if( IMeFileIsDir(szSubPath)==1 )  
        {  
            if( bIncludeDir )
            {
                CArrayAdd( arrSubDirFile , strdup(szSubPath) , 0 );
            }
            IMeGetSubDirFileList( szSubPath , arrSubDirFile , bFullPath , bIncludeDir );              
        }  
        else
        {
			if( bFullPath )
				CArrayAdd( arrSubDirFile , strdup(szSubPath) , 0 );
			else
				CArrayAdd( arrSubDirFile , strdup(pFileInfo->d_name) , 0 );
        }
    }  
    
    closedir(pDir);  
}

IME_EXTERN_C void    IMeCopyDirFile( const char* szSrcDir , const char* szDstDir )
{
    char szFilePath[256] = { 0 };
    strcpy( szFilePath , szSrcDir );
    
    if( szFilePath[strlen(szFilePath)-1] != '/' )
        strcat( szFilePath , "/" );
    
    DIR* pDir = opendir(szFilePath);
    if( pDir == NULL )  return;  
    
    struct dirent *pFileInfo;  
    while( (pFileInfo=readdir(pDir)) != NULL )  
    {  
        if( !IMeFileIsDir(pFileInfo->d_name) )
        {
            char szDstFileName[256] = { 0 };
            char szSrcFileName[256] = { 0 };
            sprintf( szDstFileName , "%s%s" , szDstDir , pFileInfo->d_name );
            sprintf( szSrcFileName , "%s%s" , szSrcDir , pFileInfo->d_name );
            IMeCopyFile( szSrcFileName , szDstFileName );
        }  
    }  
    closedir(pDir);  
}

/* create guid in linux os */
IME_EXTERN_C void    IMeCreateGUID( char* szGuid )
{

#ifndef  PROJECT_FOR_ANDROID
    if( !szGuid )   return;
    
    uuid_t uuid;
    
    uuid_generate_time(uuid); 
    uuid_unparse(uuid, szGuid);
    
    int i = 0;
    while( szGuid[i]!='\0' )
    {
        if( isalpha(szGuid[i]) )
        {
            szGuid[i] = toupper(szGuid[i]);
        }
        i++;
    }
#endif

}


IME_EXTERN_C uint64_t IMeGetHardDiskSize( const char* szDiskPath )
{   

#ifndef  PROJECT_FOR_ANDROID
    struct statfs diskInfo; 
      
    statfs( szDiskPath , &diskInfo ); 
    uint64_t blocksize = diskInfo.f_bsize;
    uint64_t totalsize = blocksize * diskInfo.f_blocks;
    DebugLogString( TRUE, "[IMeGetHardDiskSize] Total_size = %llu B = %llu KB = %llu MB = %llu GB\n", 
        totalsize, totalsize>>10, totalsize>>20, totalsize>>30 ); 
      
    uint64_t freeDisk = diskInfo.f_bfree * blocksize; 
    uint64_t availableDisk = diskInfo.f_bavail * blocksize; 
    DebugLogString( TRUE, "[IMeGetHardDiskSize] Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n", 
        freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30); 
    
    return availableDisk>>20; 
#endif

    return 0;
}

IME_EXTERN_C int IMeKillProcess( const char* szProcessName , uint32_t dwPID )    
{
    char szShellCmd[256] = { 0 };

    if( szProcessName && !strlen(szProcessName) )
    {
        sprintf( szShellCmd , "killall %s" , szProcessName );
        system( szShellCmd );
    }
    else if( dwPID != 0 )
    {
        sprintf( szShellCmd , "kill -9 %u" , dwPID );
        system( szShellCmd );
    }

    return 1;
}

IME_EXTERN_C void    IMeRestartSystem()                                                    
{
    char szShellCmd[256] = { 0 };
    sprintf( szShellCmd , "init 6" );

    system( szShellCmd );
}

IME_EXTERN_C void    IMePowerOffSystem()                                                   
{
    char szShellCmd[256] = { 0 };
    sprintf( szShellCmd , "init 0" );

    system( szShellCmd );
}

IME_EXTERN_C uint32_t    IMeGetCurrentTime()
{
    uint32_t uptime = 0;  
    struct timespec on;  
    if( clock_gettime(CLOCK_MONOTONIC, &on) == 0 )  
        uptime = on.tv_sec*1000 + on.tv_nsec/1000000;  
    return uptime;  
}

IME_EXTERN_C void    IMeGetCurrentTimeYMDHMS( IMeTime* pMeTime )
{
    time_t  timestamp; 
    struct tm *time_tm; 
    
    time(&timestamp);
    time_tm = localtime(&timestamp);

    pMeTime->year = time_tm->tm_year + 1900;
    pMeTime->month = time_tm->tm_mon + 1;
    pMeTime->day = time_tm->tm_mday;
    pMeTime->hour = time_tm->tm_hour;
    pMeTime->minute = time_tm->tm_min;
    pMeTime->second = time_tm->tm_sec;
}

IME_EXTERN_C void    IMeSleep( uint32_t dwMiliseconds )
{
    usleep( dwMiliseconds*1000 );
}

#endif
