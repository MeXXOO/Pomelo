#include    "../include/include.h"

#ifdef PROJECT_FOR_WINDOWS
#define ACCESS _access  
#define MKDIR(a) _mkdir((a))  
#elif PROJECT_FOR_LINUX  
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
    int iLen;  
    char* pszDir;  
    
    if( NULL == pDir )  
        return FALSE;  
    
    pszDir = strdup(pDir);  
    iLen = strlen(pszDir);  
    
    for( i = 0; i < iLen; i++ )  
    {  
        if( pszDir[i] == FILE_PATH_SEPARATE )  
        {   
            pszDir[i] = '\0';  
            
            //if not exist, create it 
            iRet = ACCESS(pszDir,0);  
            if( iRet != 0 )  
            {  
                iRet = MKDIR(pszDir);  
                if (iRet != 0)  
                    return FALSE;  
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
	uint dwAttr = GetFileAttributes(pPath);

	if( -1 == dwAttr )
		return -1;

	if( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
		return 1;
	else
		return 0;
}

/* 获取可用磁盘空间大小 */
IME_EXTERN_C uint64 IMeGetHardDiskSize( const char* szDiskPath )
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

/* 获取文件大小 */
IME_EXTERN_C uint64  IMeGetFileSize( const char* pFilePathName )
{
	uint64 llSize = 0;

	HANDLE handle = CreateFile( pFilePathName, FILE_READ_EA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );
    if( handle != INVALID_HANDLE_VALUE )
    {
		uint highSize = 0;
        uint lowSize = GetFileSize( handle, &highSize );
		llSize = (uint64)highSize<<32|lowSize;
        CloseHandle(handle);
    }
	
	return llSize;
}

/* 获取一个文件目录的大小 */
IME_EXTERN_C uint64 IMeGetFileDirSize( const char* szFileDir )
{
    char szFilePath[MAX_PATH] = { 0 };
    char szFileFilter[MAX_PATH] = { 0 };
    uint64 llSize = 0;
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
            char szSubPath[MAX_PATH] = { 0 };
            strcpy( szSubPath , szFilePath );
            strcat( szSubPath , fileinfo.cFileName );
            llSize = llSize + IMeGetFileDirSize(szSubPath);
        }
        else
        {
            llSize = llSize + (((uint64)fileinfo.nFileSizeHigh)<<32|fileinfo.nFileSizeLow);
        }
    }while(FindNextFile(hFind,&fileinfo));

    FindClose(hFind);
    return llSize;
}

/* 搜索主目录下的子目录名列表 */
IME_EXTERN_C void   IMeGetSubDirList( const char* szMainDir , IMeArray* arrSubDir )
{
    char szFilePath[MAX_PATH] = { 0 };
    char szFileFilter[MAX_PATH] = { 0 };
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


IME_EXTERN_C void	 IMeGetSubDirFileList( const char* szMainDir , IMeArray* arrSubDirFile , byte bIncludePath )
{
    char szFilePath[MAX_PATH] = { 0 };
    char szFileFilter[MAX_PATH] = { 0 };
    
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
		char szSubPath[MAX_PATH] = { 0 };
		
		strcpy( szSubPath , szFilePath );
		strcat( szSubPath , fileinfo.cFileName );
        
		/* skip . & .. dir */
        if( (fileinfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) 
            && strcmp(fileinfo.cFileName,".") 
            && strcmp(fileinfo.cFileName,"..") )
        {
			IMeGetSubDirFileList( szSubPath , arrSubDirFile , bIncludePath );
        }
        else
        {
			if( bIncludePath )
				CArrayAdd( arrSubDirFile , strdup(szSubPath) , 0 );
			else
				CArrayAdd( arrSubDirFile , strdup(fileinfo.cFileName) , 0 );
        }
    }while(FindNextFile(hFind,&fileinfo));

    FindClose(hFind);
}

IME_EXTERN_C void    IMeCopyDirFile( const char* szSrcDir , const char* szDstDir )
{
    char szFilePath[MAX_PATH] = { 0 };
    char szFileFilter[MAX_PATH] = { 0 };
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
            char szDstFileName[MAX_PATH] = { 0 };
            char szSrcFileName[MAX_PATH] = { 0 };
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

/* 通过进程名或是进程ID杀进程 */
IME_EXTERN_C int IMeKillProcess( const char* szProcessName , uint dwPID )    
{
    /* 创建进程快照(TH32CS_SNAPPROCESS表示创建所有进程的快照) */  
    HANDLE hSnapShot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );  
    /* PROCESSENTRY32进程快照的结构体 */
    PROCESSENTRY32 pe;  
    /* 实例化后使用Process32First获取第一个快照的进程前必做的初始化操作 */
    pe.dwSize = sizeof(PROCESSENTRY32);  
    if( !Process32First(hSnapShot,&pe) )  
    {  
        return 0;  
    }  

    /* 如果句柄有效 则一直获取下一个句柄循环下去 */
    while( Process32Next(hSnapShot,&pe) )  
    {  
        char szExeName[MAX_PATH] = { 0 };
        sprintf( szExeName, "%s", pe.szExeFile );
        
        /* 与当前传入的文件名相同或与当前进程的ID相同 */
        if( !stricmp(szExeName , szProcessName) || (dwPID==pe.th32ProcessID && dwPID!=0) )  
        {  
            //从快照进程中获取该进程的PID(即任务管理器中的PID)  
            DWORD dwProcessID = pe.th32ProcessID;  
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,FALSE,dwProcessID);  
            TerminateProcess(hProcess,0);  
            CloseHandle(hProcess);  
        }  
    }  
    
    return 1;  
}

/* 重启系统 */
IME_EXTERN_C void    IMeRestartSystem()                                                    
{
    char szShellCmd[MAX_PATH] = { 0 };

    sprintf( szShellCmd , "shutdown /r /f /t 0" );
    
    system( szShellCmd );
}

/* 系统关机 */
IME_EXTERN_C void    IMePowerOffSystem()                                                   
{
    char szShellCmd[MAX_PATH] = { 0 };
    
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
    UuidToString( &guid, (uchar**)&pszGuid );
    
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
    RpcStringFree((uchar**)&pszGuid);
}

IME_EXTERN_C uint    IMeGetCurrentTime()
{
    return timeGetTime();
}

IME_EXTERN_C void    IMeSleep( uint dwMiliseconds )
{
    Sleep( dwMiliseconds );
}

#elif       PROJECT_FOR_LINUX   //////////////////////////////////////////////////////////////////////////

IME_EXTERN_C int	IMeFileIsDir( const char* pPath )
{  
    struct stat S_stat;  
    
    /* 取得文件状态 */
    if( lstat(pszName, &S_stat)<0 )  
    {  
        return -1;  
    }  
    
    if( S_ISDIR(S_stat.st_mode) )  
    {  
        return 1;  
    }  
    else  
    {  
        return 0;  
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

IME_EXTERN_C uint64  IMeGetFileSize( const char* pFilePathName )
{
	int fd = 0;
	FILE* pFile = NULL;

	uint64 llSize = 0;

	pFile = fopen( szSubPath, "r" );
	if( pFile && (fd = fileno( pFile )) )
	{
		fstat( fd , &filestat );
		llSize += filestat.st_size;
		fclose(pFile);
	}

	return llSize;
}

IME_EXTERN_C uint64 IMeGetFileDirSize( const char* szFileDir )  
{  
    char szFilePath[MAX_PATH] = { 0 };
    strcpy( szFilePath , szFileDir );
    
    if( szFilePath[strlen(szFilePath)-1] != '/' )
        strcat( szFilePath , "/" );
    
    DIR* pDir = opendir(szFilePath);
    if( pDir == NULL )  return 0;  

    uint64 llSize = 0;
    struct dirent *pFileInfo;  
    struct stat filestat;          /* linux文件详细信息结构定义 */
    while( (pFileInfo=readdir(pDir)) != NULL )  
    {  
        char szSubPath[MAX_PATH] = { 0 };
        strcpy( szSubPath , szFilePath );
        strcat( szSubPath , pFileInfo->d_name );
        if( IMeFileIsDir(szSubPath) == 1
            && strcmp(pFileInfo->d_name, ".") 
            && strcmp(pFileInfo->d_name, ".."))  
        {  
            llSize = llSize + IMeGetFileDirSize(szSubPath);              
        }  
        else
        {
			llSize += IMeGetFileSize( szSubPath );
        }
    }  
    
    closedir(pDir);  
    
    return llSize;  
}  

IME_EXTERN_C void  IMeGetSubDirList( const char* szMainDir , IMeArray* arrSubDir ) 
{  
    char szFilePath[MAX_PATH] = { 0 };
    strcpy( szFilePath , szMainDir );
    
    if( szFilePath[strlen(szFilePath)-1] != '/' )
        strcat( szFilePath , "/" );
    
    DIR* pDir = opendir(szFilePath);
    if( pDir == NULL )  return;  
    
    struct dirent *pFileInfo;  
    while( (pFileInfo=readdir(pDir)) != NULL )  
    {  
        char szSubPath[MAX_PATH] = { 0 };
        strcpy( szSubPath , szFilePath );
        strcat( szSubPath , pFileInfo->d_name );
        if( IMeFileIsDir(szSubPath)==1
            && strcmp(pFileInfo->d_name, ".") 
            && strcmp(pFileInfo->d_name, ".."))  
        {  
            CArrayAdd( arrSubDir , IMeCopyString(pFileInfo->d_name) );
        }  
    }  
    closedir(pDir);  
}  


IME_EXTERN_C void	 IMeGetSubDirFileList( const char* szMainDir , IMeArray* arrSubDirFile , byte bIncludePath )
{
    char szFilePath[MAX_PATH] = { 0 };
    strcpy( szFilePath , szMainDir );
    
    if( szFilePath[strlen(szFilePath)-1] != '/' )
        strcat( szFilePath , "/" );
    
    DIR* pDir = opendir(szFilePath);
    if( pDir == NULL )  return;  

    struct dirent *pFileInfo;  
    struct stat filestat;          /* linux文件详细信息结构定义 */
    while( (pFileInfo=readdir(pDir)) != NULL )  
    {  
        char szSubPath[MAX_PATH] = { 0 };
        
		strcpy( szSubPath , szFilePath );
        strcat( szSubPath , pFileInfo->d_name );

        if( IMeFileIsDir(szSubPath)==1
            && strcmp(pFileInfo->d_name, ".") 
            && strcmp(pFileInfo->d_name, ".."))  
        {  
            IMeGetSubDirFileList( szSubPath , arrSubDirFile , bIncludePath );              
        }  
        else
        {
			if( bIncludePath )
				CArrayAdd( arrSubDirFile , strdup(szSubPath) );
			else
				CArrayAdd( arrSubDirFile , strdup(pFileInfo->d_name) );
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
        if( !IsDirectory(pFileInfo->d_name) )
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
}


IME_EXTERN_C uint64 IMeGetHardDiskSize( const char* szDiskPath )
{   
    struct statfs diskInfo; 
      
    statfs( szDiskPath , &diskInfo ); 
    unsigned long long blocksize = diskInfo.f_bsize; //每个block里包含的字节数 
    unsigned long long totalsize = blocksize * diskInfo.f_blocks; //总的字节数，f_blocks为block的数目 
//     printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n", 
//         totalsize, totalsize>>10, totalsize>>20, totalsize>>30); 
      
    unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //剩余空间的大小 
    unsigned long long availableDisk = diskInfo.f_bavail * blocksize; //可用空间大小 
//     printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n", 
//         freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30); 
    
    return availableDisk>>20; 
}

/* 通过进程名或是进程ID杀进程 */
IME_EXTERN_C int IMeKillProcess( const char* szProcessName , uint dwPID )    
{
    char szShellCmd[MAX_PATH] = { 0 };

    if( !strlen(szProcessName) )
    {
        sprintf( szProcessName , "killall %s" , szProcessName );
        system( szProcessName );
    }
    else if( dwPID != 0 )
    {
        sprintf( szProcessName , "kill -9 %u" , dwPID );
        system( szProcessName );
    }

    return 1;
}

/* 重启系统 */
IME_EXTERN_C void    IMeRestartSystem()                                                    
{
    char szShellCmd[MAX_PATH] = { 0 };
    sprintf( szShellCmd , "init 6" );

    system( szShellCmd );
}

/* 系统关机 */
IME_EXTERN_C void    IMePowerOffSystem()                                                   
{
    char szShellCmd[MAX_PATH] = { 0 };
    sprintf( szShellCmd , "init 0" );

    system( szShellCmd );
}

IME_EXTERN_C uint    IMeGetCurrentTime()
{
    unsigned int uptime = 0;  
    struct timespec on;  
    if( clock_gettime(CLOCK_MONOTONIC, &on) == 0 )  
        uptime = on.tv_sec*1000 + on.tv_nsec/1000000;  
    return uptime;  
}

IME_EXTERN_C void    IMeSleep( uint dwMiliseconds )
{
    usleep( dwMiliseconds*1000 );
}

#endif
