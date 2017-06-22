#ifndef     _ME_PLAT_TYPE_H_
#define     _ME_PLAT_TYPE_H_

/******* if used for windows platform application , MACRO PROJECT_FOR_WINDOWS auto defined below, *******/
/******* else used for other platform , you should set PROJECT_FOR_LINUX MACRO in YOUR makefile *********/
#ifdef	WIN32
#define	PROJECT_FOR_WINDOWS
#endif

//BASE DATA TYPE LENGTH
#define		LEN_INT				(sizeof(int))
#define		LEN_UINT			(sizeof(uint))
#define		LEN_SHORT			(sizeof(short))
#define		LEN_USHORT			(sizeof(ushort))

//BASE DATA TYPE NAME
typedef		int					int32_t;
typedef     unsigned int        uint;
typedef		unsigned int		uint32_t;

typedef		short				int16_t;
typedef     unsigned short      uint16_t;
typedef     unsigned short      ushort;

typedef		unsigned char       uint8_t;
typedef     unsigned char       uchar;
typedef     unsigned char       uint8;
typedef		unsigned char		byte;

typedef     unsigned int        lposition;  /* list use */

#ifdef  __cplusplus
#define IME_EXTERN_C  extern "C"
#else
#define IME_EXTERN_C
#endif  

#ifdef      PROJECT_FOR_WINDOWS

/* 系统头文件 */
#include    <winsock2.h>
#include    <windows.h>
#include    <tlhelp32.h>
#include    <direct.h>  
#include    <io.h> 

/* 数据类型 */
typedef		__int64				int64_t;
typedef     __int64             int64;
typedef     unsigned __int64    uint64_t;
typedef     unsigned __int64    uint64;
typedef     SOCKET              HSOCKET;

#define		FILE_PATH_SEPARATE	'\\'

#elif       PROJECT_FOR_LINUX

/* 系统头文件 */
#include    <netinet/in.h>
#include    <fcntl.h>
#include    <unistd.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <pthead.h>
#include    <time.h>
#include    <stdarg.h>

/* 数据类型 */
typedef     long long           int64;
typedef     unsigned long long  uint64;
typedef     long long           int64_t;
typedef     unsigned long long  uint64_t;
typedef     int                 HSOCKET;

#define     _FILE_OFFSET_BITS		64      /* linux large file more than 4G */
#define		FILE_PATH_SEPARATE		'/'

#endif

#endif  //end _ME_PLAT_TYPE_H_
