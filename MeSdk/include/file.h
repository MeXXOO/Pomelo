#ifndef     _ME_FILE_H_
#define     _ME_FILE_H_

#define     IMeFile_OpenWrite   1<<0    /* 打开文件写 */
#define     IMeFile_OpenRead    1<<1    /* 打开文件读 */
#define     IMeFile_OpenCreate  1<<2    /* 总是创建新文件写 */
#define     IMeFile_WriteAppend 1<<3    /* 以追加方式写 */

#define     IMeFile_SeekBegin   1   /* 从文件头开始移动文件指针 */
#define     IMeFile_SeekCur     2   /* 从文件指针当前位置移动文件指针 */
#define     IMeFile_SeekEnd     3   /* 从文件未尾开始移动文件指针 */

typedef struct _IMeFile IMeFile;

IME_EXTERN_C IMeFile*    IMeFileCreate();

typedef int  (*IMeFileOpen)( IMeFile* pIFile , char* pFileName , int nOpenFlag );
typedef int  (*IMeFileIsOpen)( IMeFile* pIFile );
typedef uint (*IMeFileRead)( IMeFile* pIFile , char* pBuffer , uint32_t nBufferLen );
typedef uint (*IMeFileWrite)( IMeFile* pIFile , char* pBuffer , uint32_t nBufferLen );
typedef int  (*IMeFileSeek)( IMeFile* pIFile , int64_t llPos , int nSeekFlag );
typedef uint64_t (*IMeFileGetSize)( IMeFile* pIFile );
typedef int64_t (*IMeFileGetPosition)( IMeFile* pIFile );
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
