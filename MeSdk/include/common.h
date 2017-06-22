#ifndef     _ME_COMMON_H_
#define     _ME_COMMON_H_

#include    "array.h"

/* ��ֵ�ַ������� */
IME_EXTERN_C char*   IMeCopyString( const char* pString );

/* �����ļ�·�� */
IME_EXTERN_C char    IMeCreateDirectory( const char* pDir );
/* ��ȡӲ�̿��пռ��С */
IME_EXTERN_C uint64  IMeGetHardDiskSize( const char* pDiskPath );                              
/* ��ȡ�ļ�Ŀ¼ռ�ô��̿ռ��С */
IME_EXTERN_C uint64  IMeGetFileDirSize( const char* pFileDir );                                
/* ��ȡ�ļ���С */
IME_EXTERN_C uint64  IMeGetFileSize( const char* pFilePathName );

/* ������Ŀ¼�µĵ�һ����Ŀ¼�������б��з��� */
IME_EXTERN_C void    IMeGetSubDirList( const char* szMainDir , IMeArray* arrSubDir );  
/* ��ȡĿ¼�µ������ļ��� */
IME_EXTERN_C void	 IMeGetSubDirFileList( const char* szMainDir , IMeArray* arrSubDirFile , byte bIncludePath );

/* ��ȡ��ǰ·������һ��Ŀ¼ȫ·��
 * @egg: d:\\mypath\curpath nLevel:1 ==> d:\\mypath\
 * @egg: d:\\mypath\curpath.txt nLevel:1 ==> d:\\mypath\
 * @egg: d:\\mypath\curpath.txt nLevel:2 ==> d:\\
 */
IME_EXTERN_C void	 IMeGetUpLevelFilePath( const char* szInputPath , char* szOutputPath  , int nLevel );
//�ж���Ŀ¼�����ļ�(return 0:file 1:dir -1:failed)
IME_EXTERN_C int	 IMeFileIsDir( const char* pPath );
/* ����Ŀ¼�е��ļ�(ֻ֧��һ������ֻ�����ļ�) */
IME_EXTERN_C void    IMeCopyDirFile( const char* szSrcDir , const char* szDstDir );                 
/* ���������ļ� */
IME_EXTERN_C void    IMeCopyFile( const char* pSrcFile , const char* pDstFile );

/* ͨ�����������ǽ���IDɱ���� */      
IME_EXTERN_C int     IMeKillProcess( const char* pProcessName , uint dwPID );
/* ����ϵͳ */
IME_EXTERN_C void    IMeRestartSystem();                                                    
/* ϵͳ�ػ� */
IME_EXTERN_C void    IMePowerOffSystem();   

/* ����GUID�ַ��� */
IME_EXTERN_C void    IMeCreateGUID( char* szGuid );                                  
/* ��ȡ��ǰʱ��(msΪ��λ) */                                                
IME_EXTERN_C uint    IMeGetCurrentTime();
/* ����ǰ�߳� */
IME_EXTERN_C void    IMeSleep( uint dwMiliseconds );  

#endif

