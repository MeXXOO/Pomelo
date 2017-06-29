#include    "../include/include.h"

#define     IMeCArray_DefaultSize    1

typedef struct _CNode{
    uint64_t  keyValue;
    void*   data;
}CNode;

typedef struct _IMeCArray{
    IMeArray    interfacefunction;
    IMeMemory*  m_pMemAllocator;
    CNode*    m_data;
    int m_nSize;        /* 当前存放元素个数 */
    int m_nCaCheSize;   /* 当前分配的缓存大小 */
    int m_nSortType;    /* sort type */

    IMeArrayCompareKeyValue mCompareKeyValue;
	void* mParameter;
}IMeCArray;

int    IMeCArrayCompareKeyValue( IMeCArray* pArray, uint64_t keyValue1, uint64_t keyValue2 )
{
    if( pArray->mCompareKeyValue )
        return pArray->mCompareKeyValue(keyValue1,keyValue2,pArray->mParameter);
    else
    {
        if( keyValue1 == keyValue2 )
            return 0;
        else if( keyValue1 < keyValue2 )
            return 1;
        else
            return -1;
    }
}

int     IMeCArrayInnerFindIndex( IMeCArray* pArray, uint64_t keyValue, int low, int high )
{
    int mid = -1;
    int compareResult;

    //no element or no support sort 
    if( pArray->m_nSize == 0 || pArray->m_nSortType == SORT_NULL )
        return mid;

    //search 
    while( low <= high )
    {  
        mid = (low + high) / 2;  
        
        compareResult = IMeCArrayCompareKeyValue( pArray, pArray->m_data[mid].keyValue, keyValue );

        if( 0 == compareResult )  
        {  
            return mid;  
        }  
        else if( compareResult > 0 )     
        {  
            if( pArray->m_nSortType == SORT_INC )
                low = mid + 1;  
            else
                high = mid - 1;
        }
        else  
        {  
            if( pArray->m_nSortType == SORT_INC )
                high = mid - 1;  
            else
                low = mid + 1;
        }    
    }  
    
    //DebugLogString( TRUE , "[IMeCArrayInnerFind] mid:%d" , mid );

    return -1; 
}

int     IMeCArrayInnerFindInsertPosition( IMeCArray* pArray, uint64_t keyValue, int low, int high )
{
    int mid = -1;
    int compareResult = 0;

    //no element or no support sort 
    if( pArray->m_nSize == 0 || pArray->m_nSortType == SORT_NULL )
        return -1;
    
    //search 
    while( low <= high )
    {  
        mid = (low + high) / 2;  

        compareResult = IMeCArrayCompareKeyValue( pArray, pArray->m_data[mid].keyValue, keyValue );
        
        if ( 0 == compareResult )  
        {  
           return mid;  
        }  
        else if( compareResult > 0 )     
        {  
            if( pArray->m_nSortType == SORT_INC )
                low = mid + 1;  
            else
                high = mid - 1;
        }
        else  
        {  
            if( pArray->m_nSortType == SORT_INC )
                high = mid - 1;  
            else
                low = mid + 1;
        }    
    }  
    
    //DebugLogString( TRUE, "[IMeCArrayInnerFindInsertPosition] low:%d mid:%d high:%d size:%d" , low, mid, high, pArray->m_nSize );

    if( 0 == compareResult )
        return mid;
    else if( pArray->m_nSortType == SORT_INC )
    {
        if( compareResult < 0 )
            return mid;
        else
            return (mid+1>pArray->m_nSize-1 ? -1 : mid+1);
    }
    else if( pArray->m_nSortType == SORT_DEC )
    {
        if( compareResult < 0 )
            return (mid+1>pArray->m_nSize-1 ? -1 : mid+1);
        else 
            return mid;
    }
    
    return -1;
}

int    IMeCArrayInnerSetSize( IMeCArray* pArray, int nNewSize )
{
    //larger
    if( nNewSize > pArray->m_nCaCheSize )
    {
        void* pNode = CMemoryAlloc(pArray->m_pMemAllocator,sizeof(CNode)*pArray->m_nSize*2);
        if( pNode )
        {
            pArray->m_nCaCheSize = pArray->m_nSize*2;
            memcpy( pNode, pArray->m_data, sizeof(CNode)*pArray->m_nSize );
            CMemoryFree( pArray->m_pMemAllocator, pArray->m_data );
            pArray->m_data = pNode;
        }
        else
        {
            DebugLogString( TRUE, "[IMeCArrayInnerAddElement] malloc failed!!" );
            return -1;
        }

        return 0;
    }

    return -1;
}

int    IMeCArrayInnerInsertAt( IMeCArray* pArray, int nIndex, void* pElement, uint64_t keyValue )
{
    int res = 0;

    if( pArray->m_nSize >= pArray->m_nCaCheSize )
        res = IMeCArrayInnerSetSize( pArray, pArray->m_nSize*2 );

    if( res == 0 )
    {
        memmove( &pArray->m_data[nIndex+1], &pArray->m_data[nIndex] , (pArray->m_nSize-nIndex)*sizeof(CNode) );
        pArray->m_data[nIndex].data = pElement;
        pArray->m_data[nIndex].keyValue = keyValue;

        ++pArray->m_nSize;

        return nIndex;
    }

    return -1;
}

int    IMeCArrayInnerAddElement( IMeCArray* pArray, void* pElement, uint64_t keyValue )
{
    int res = 0;

    if( pArray->m_nSize >= pArray->m_nCaCheSize )
        res = IMeCArrayInnerSetSize( pArray, pArray->m_nSize*2 );

    if( res == 0 )
    {
        pArray->m_data[pArray->m_nSize].data = pElement;
        pArray->m_data[pArray->m_nSize].keyValue = keyValue;
        
        ++pArray->m_nSize;
        
        return (pArray->m_nSize-1);
    }

    return -1;
}

int    IMeCArrayAdd( IMeArray* pIArray , void* pElement , uint64_t keyValue )
{
    IMeCArray* pArray = (IMeCArray*)pIArray;

    int nIndex = IMeCArrayInnerFindInsertPosition( pArray, keyValue, 0, pArray->m_nSize-1 );

    if( nIndex >= 0 && nIndex < pArray->m_nSize )
        nIndex = IMeCArrayInnerInsertAt( pArray, nIndex, pElement, keyValue );
    else
        nIndex = IMeCArrayInnerAddElement( pArray, pElement, keyValue );

    return nIndex;
}

int     IMeCArrayGetSize( IMeArray* pIArray )
{
    IMeCArray* pArray = (IMeCArray*)pIArray;

    return pArray->m_nSize;
}

void*   IMeCArrayGetAt( IMeArray* pIArray , int nIndex )
{
    IMeCArray* pArray = (IMeCArray*)pIArray;
    
    if( pArray->m_nSize <= nIndex || nIndex < 0 )  
        return NULL;
    else
        return pArray->m_data[nIndex].data;
}

void    IMeCArrayRemoveAll( IMeArray* pIArray )
{
    IMeCArray* pArray = (IMeCArray*)pIArray;
    
    pArray->m_nSize = 0;
}

void*   IMeCArrayRemoveAt( IMeArray* pIArray , int nIndex )
{
    IMeCArray* pArray = (IMeCArray*)pIArray;
    void* pValue;
    
    if( pArray->m_nSize <= nIndex || nIndex < 0 )  
        return NULL;
    
    pValue = pArray->m_data[nIndex].data;

    memmove( &pArray->m_data[nIndex], &pArray->m_data[nIndex+1], (pArray->m_nSize-nIndex-1)*sizeof(CNode) );
    
    --pArray->m_nSize;

    return pValue;
}

void*    IMeCArrayRemove( IMeArray* pIArray , uint64_t keyValue )
{
    IMeCArray* pArray = (IMeCArray*)pIArray;

    int nIndex = IMeCArrayInnerFindIndex( pArray, keyValue, 0, pArray->m_nSize-1 );

    return IMeCArrayRemoveAt( pIArray, nIndex );
}

int     IMeCArrayFind( IMeArray* pIArray , uint64_t keyValue )
{
    IMeCArray* pArray = (IMeCArray*)pIArray;

    return IMeCArrayInnerFindIndex( pArray, keyValue, 0, pArray->m_nSize-1 );
}

void*   IMeCArrayFindData( IMeArray* pIArray , uint64_t keyValue )
{
    IMeCArray* pArray = (IMeCArray*)pIArray;
    void* pData = NULL;
    int nIndex = IMeCArrayInnerFindIndex( pArray, keyValue, 0, pArray->m_nSize-1 );

    if( nIndex != -1 )
        pData = pArray->m_data[nIndex].data;

    return pData;
}

void    IMeCArrayCopy( IMeArray* pIArray , const IMeArray* psrc )
{
    IMeCArray* pArray = (IMeCArray*)pIArray;
    IMeCArray* pSrcArray = (IMeCArray*)psrc;

    if( psrc != pIArray )
    {
	    IMeCArrayInnerSetSize( pArray, pSrcArray->m_nSize );
        memcpy( pArray->m_data, pSrcArray->m_data, pSrcArray->m_nSize * sizeof(CNode) );
    }
}

void    IMeCArrayDestroy( IMeArray* pIArray )
{
    IMeCArray* pArray = (IMeCArray*)pIArray;
    if( pArray )
    {
        if( pArray->m_data )
            CMemoryFree( pArray->m_pMemAllocator , pArray->m_data );
        CMemoryDestroy( pArray->m_pMemAllocator );
        free( pArray );
    }
}

IME_EXTERN_C    void    IMeCArraySetCompare( IMeArray* pIArray, IMeArrayCompareKeyValue compareKeyValue, void* parameter )
{
    IMeCArray* pArray = (IMeCArray*)pIArray;
    pArray->mCompareKeyValue = compareKeyValue;
	pArray->mParameter = parameter;
}

IME_EXTERN_C IMeArray*    IMeArrayCreate( int sortType )
{
    IMeCArray* pArray = (IMeCArray*)calloc(1,sizeof(IMeCArray));
    if( pArray )
    {
        pArray->m_pMemAllocator = CMemoryCreate( IMeMemory_Default );
        if( !pArray->m_pMemAllocator )
        {
            DebugLogString( TRUE , "[IMeArrayCreate] Allocator created failed!" );
            free( pArray );
            return NULL;
        }

        pArray->m_nSortType = sortType;
        pArray->m_nCaCheSize = IMeCArray_DefaultSize;
        pArray->m_data = (CNode*)CMemoryAlloc( pArray->m_pMemAllocator , pArray->m_nCaCheSize*sizeof(CNode) );
        if( !pArray->m_data )
        {
            DebugLogString( TRUE , "[IMeArrayCreate] Allocator default array buffer failed!" );
            CMemoryDestroy(pArray->m_pMemAllocator);
            free(pArray);
            return NULL;
        }

        ((IMeArray*)pArray)->m_pAdd = IMeCArrayAdd;
        ((IMeArray*)pArray)->m_pSetCompare = IMeCArraySetCompare;
        ((IMeArray*)pArray)->m_pRemoveAll = IMeCArrayRemoveAll;
        ((IMeArray*)pArray)->m_pRemoveAt = IMeCArrayRemoveAt;
        ((IMeArray*)pArray)->m_pDestroy = IMeCArrayDestroy;
        ((IMeArray*)pArray)->m_pGetAt = IMeCArrayGetAt;
        ((IMeArray*)pArray)->m_pGetSize = IMeCArrayGetSize;
        ((IMeArray*)pArray)->m_pRemove = IMeCArrayRemove;
        ((IMeArray*)pArray)->m_pFind = IMeCArrayFind;
        ((IMeArray*)pArray)->m_pFindData = IMeCArrayFindData;
        ((IMeArray*)pArray)->m_pCopy = IMeCArrayCopy;
    }

    return (IMeArray*)pArray;
}