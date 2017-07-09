
#include    "../include/include.h"

typedef struct _IMeCMemory{
    IMeMemory   interfacefunction;
}IMeCMemory;

void*   IMeCMemoryAlloc( IMeMemory* pIMemory , int nSize )
{
    return malloc(nSize);
}

void*   IMeCMemoryCalloc( IMeMemory* pIMemory , int nSize )
{
    return calloc( 1 , nSize );
}

void*   IMeCMemoryRealloc( IMeMemory* pIMemory , void* pMem , int nSize )
{
    return realloc( pMem , nSize );
}

void    IMeCMemoryFree( IMeMemory* pIMemory , void* pMem )
{
    if( pMem )  free( pMem );
}

void    IMeCMemoryDestroy( IMeMemory* pIMemory )
{
    if( pIMemory )  free( pIMemory );
}

IME_EXTERN_C IMeMemory*   IMeMemoryCreate( int nType )
{
    IMeCMemory* pMemory = (IMeCMemory*)malloc(sizeof(IMeCMemory));
    if( pMemory )
    {
        ((IMeMemory*)pMemory)->m_pAlloc = IMeCMemoryAlloc;
        ((IMeMemory*)pMemory)->m_pCalloc = IMeCMemoryCalloc;
        ((IMeMemory*)pMemory)->m_pDestroy = IMeCMemoryDestroy;
        ((IMeMemory*)pMemory)->m_pRealloc = IMeCMemoryRealloc;
        ((IMeMemory*)pMemory)->m_pFree = IMeCMemoryFree;
    }

    return (IMeMemory*)pMemory;
}
