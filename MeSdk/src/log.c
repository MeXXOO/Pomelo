#include    "../include/include.h"

IME_EXTERN_C char*	 _ErrorCodeString()
{
	int nErrorCode = IMeSocketGetNetError();
	return IMeSocketConvertErrorCodeToString( nErrorCode );
}

IME_EXTERN_C void	 DebugLogTime( char* buffer )
{
	SYSTEMTIME st; 
	GetLocalTime(&st);
	sprintf( buffer , "%04d-%02d-%02d %02d:%02d:%02d " , st.wYear , st.wMonth , st.wDay , st.wHour , st.wMinute , st.wSecond );
}

IME_EXTERN_C void    DebugLogString( int bOut , char* format , ... )
{
    char szInfo[2*1024] = { 0 };
    int nLen = 0;
    va_list argList;
    va_start(argList, format);

	DebugLogTime( szInfo );

#ifdef	PROJECT_FOR_WINDOWS
    _vsnprintf( szInfo+strlen(szInfo), sizeof(szInfo)-1-strlen(szInfo), format, argList);
#elif   PROJECT_FOR_LINUX
    vsnprintf( szInfo+strlen(szInfo), sizeof(szInfo)-1-strlen(szInfo) , format , argList);
#endif
    va_end(argList);
    
    nLen = strlen(szInfo);
    if( szInfo[nLen] != '\n' )
        szInfo[nLen] = '\n';

    if( bOut )
    {
#ifdef  PROJECT_FOR_WINDOWS
        OutputDebugString( szInfo );
        fprintf( stdout , szInfo );
#elif   PROJECT_FOR_LINUX
        fprintf( stdout , szInfo );
#endif    
    }
}