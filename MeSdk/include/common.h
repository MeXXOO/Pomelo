#ifndef     _ME_COMMON_H_
#define     _ME_COMMON_H_

#include    "array.h"

/* 带值字符串拷贝 */
IME_EXTERN_C char*   IMeCopyString( const char* pString );

/* 创建文件路径 */
IME_EXTERN_C char    IMeCreateDirectory( const char* pDir );
/* 获取硬盘空闲空间大小 */
IME_EXTERN_C uint64  IMeGetHardDiskSize( const char* pDiskPath );                              
/* 获取文件目录占用磁盘空间大小 */
IME_EXTERN_C uint64  IMeGetFileDirSize( const char* pFileDir );                                
/* 获取文件大小 */
IME_EXTERN_C uint64  IMeGetFileSize( const char* pFilePathName );

/* 搜索主目录下的第一级子目录名并在列表中返回 */
IME_EXTERN_C void    IMeGetSubDirList( const char* szMainDir , IMeArray* arrSubDir );  
/* 获取目录下的所有文件名 */
IME_EXTERN_C void	 IMeGetSubDirFileList( const char* szMainDir , IMeArray* arrSubDirFile , byte bIncludePath );

/* 获取当前路径的上一级目录全路径
 * @egg: d:\\mypath\curpath nLevel:1 ==> d:\\mypath\
 * @egg: d:\\mypath\curpath.txt nLevel:1 ==> d:\\mypath\
 * @egg: d:\\mypath\curpath.txt nLevel:2 ==> d:\\
 */
IME_EXTERN_C void	 IMeGetUpLevelFilePath( const char* szInputPath , char* szOutputPath  , int nLevel );
//判断是目录还是文件(return 0:file 1:dir -1:failed)
IME_EXTERN_C int	 IMeFileIsDir( const char* pPath );
/* 拷贝目录中的文件(只支持一级并且只拷贝文件) */
IME_EXTERN_C void    IMeCopyDirFile( const char* szSrcDir , const char* szDstDir );                 
/* 拷贝单个文件 */
IME_EXTERN_C void    IMeCopyFile( const char* pSrcFile , const char* pDstFile );

/* 通过进程名或是进程ID杀进程 */      
IME_EXTERN_C int     IMeKillProcess( const char* pProcessName , uint dwPID );
/* 重启系统 */
IME_EXTERN_C void    IMeRestartSystem();                                                    
/* 系统关机 */
IME_EXTERN_C void    IMePowerOffSystem();   

/* 生成GUID字符串 */
IME_EXTERN_C void    IMeCreateGUID( char* szGuid );                                  
/* 获取当前时间(ms为单位) */                                                
IME_EXTERN_C uint    IMeGetCurrentTime();
/* 挂起当前线程 */
IME_EXTERN_C void    IMeSleep( uint dwMiliseconds );  

#endif

