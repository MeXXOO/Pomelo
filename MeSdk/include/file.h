#ifndef     _ME_FILE_H_
#define     _ME_FILE_H_

#define     IMeFile_OpenWrite   1<<0    /* ���ļ�д */
#define     IMeFile_OpenRead    1<<1    /* ���ļ��� */
#define     IMeFile_OpenCreate  1<<2    /* ���Ǵ������ļ�д */
#define     IMeFile_WriteAppend 1<<3    /* ��׷�ӷ�ʽд */

#define     IMeFile_SeekBegin   1   /* ���ļ�ͷ��ʼ�ƶ��ļ�ָ�� */
#define     IMeFile_SeekCur     2   /* ���ļ�ָ�뵱ǰλ���ƶ��ļ�ָ�� */
#define     IMeFile_SeekEnd     3   /* ���ļ�δβ��ʼ�ƶ��ļ�ָ�� */

typedef struct _IMeFile IMeFile;

IME_EXTERN_C IMeFile*    IMeFileCreate();

typedef int  (*IMeFileOpen)( IMeFile* pIFile , char* pFileName , int nOpenFlag );
typedef int  (*IMeFileIsOpen)( IMeFile* pIFile );
typedef uint (*IMeFileRead)( IMeFile* pIFile , char* pBuffer , uint nBufferLen );
typedef uint (*IMeFileWrite)( IMeFile* pIFile , char* pBuffer , uint nBufferLen );
typedef int  (*IMeFileSeek)( IMeFile* pIFile , int64 llPos , int nSeekFlag );
typedef uint64 (*IMeFileGetSize)( IMeFile* pIFile );
typedef int64 (*IMeFileGetPosition)( IMeFile* pIFile );
typedef void (*IMeFileClose)( IMeFile* pIFile );
typedef void (*IMeFileDestroy)( IMeFile* pIFile );

struct _IMeFile{
    IMeFileOpen m_pOpen;
    IMeFileIsOpen   m_pIsOpen;
    IMeFileRead m_pRead;
    IMeFileWrite    m_pWrite;
    IMeFileSeek m_pSeek;
    IMeFileGetSize  m_pGetSize;
    IMeFileGetPosition  m_pGetPosition;
    IMeFileClose    m_pClose;
    IMeFileDestroy  m_pDestroy;
};

#define     CFileCreate()       IMeFileCreate()
#define     CFileOpen(p,a,b)    (p->m_pOpen(p,a,b))
#define     CFileIsOpen(p)      (p->m_pIsOpen(p))
#define     CFileRead(p,a,b)    (p->m_pRead(p,a,b))
#define     CFileWrite(p,a,b)   (p->m_pWrite(p,a,b))
#define     CFileSeek(p,a,b)    (p->m_pSeek(p,a,b))
#define     CFileGetSize(p)     (p->m_pGetSize(p))
#define     CFileGetPosition(p) (p->m_pGetPosition(p))
#define     CFileClose(p)       (p->m_pClose(p))
#define     CFileDestroy(p)     (p->m_pDestroy(p))

#endif
