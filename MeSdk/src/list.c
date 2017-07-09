#include    "../include/include.h"

typedef struct _List_Node_t   list_node_t;
struct _List_Node_t
{
    list_node_t*      next;          
    list_node_t*      pre;               
    void*   data;
};


typedef struct _IMeCList
{
    IMeList     interfacefunction;
    IMeMemory*  m_pMemory;         
    
    list_node_t*    tailNode;   //list tail node
    list_node_t*    headNode;   //list head node
    
    int nNodeCount;             //total node count of the list
}IMeCList;

list_node_t*  list_node_alloc( IMeMemory* pMemory , void* data )
{
    list_node_t* node_t = CMemoryCalloc( pMemory , sizeof(list_node_t) );
    if( !node_t )
    {
        DebugLogString( TRUE , "[list_node_alloc] CMemoryCalloc failed!!" );
        return NULL;
    }
    
    node_t->data = data;
    return node_t;
}

void  list_output_list( list_node_t* head , list_node_t* tail )
{
    while( head!=tail )  
    {
        printf( "%0x->" , (uint32_t)head->data );
        head = head->next; 
    }
    printf( "%0x\n" , (uint32_t)tail->data );
}

int IMeCListGetCount( IMeList* list )
{
    IMeCList*  list_t  =   (IMeCList*)list;
    
    return list_t->nNodeCount;
}

void*   IMeCListGetNext( IMeList* list , uint32_t* position )
{
    IMeCList* list_t = (IMeCList*)list;

    list_node_t* node_t = (list_node_t*)(long)(*position);
    
    if( list_t && node_t )
    {
        node_t = node_t->next;
        *position = (uint32_t)node_t;
        return node_t ? node_t->data : NULL;
    }

    return NULL;
}

void*   IMeCListGetPre( IMeList* list , uint32_t* position )
{
    IMeCList* list_t = (IMeCList*)list;
    list_node_t* node_t = (list_node_t*)(long)(*position);
   
    if( list_t && node_t )
    {
        node_t = node_t->pre;
        *position = (uint32_t)node_t;
        return node_t ? node_t->data : NULL;
    }

    return NULL;
}

void*   IMeCListGetHead( IMeList* list , uint32_t* position )
{
    IMeCList*  list_t  =   (IMeCList*)list;
    list_node_t*  node_t  =   (list_node_t*)list_t->headNode;
    
    if( position )
        *position = (uint32_t)node_t;

    return node_t ? node_t->data : NULL;
}

void*   IMeCListGetTail( IMeList* list , uint32_t* position )
{
    IMeCList* list_t = (IMeCList*)list;
    list_node_t* node_t = (list_node_t*)list_t->tailNode;
    
    if( position )
        *position = (uint32_t)node_t;

    return node_t ? node_t->data : NULL;
}

uint32_t   IMeCListAddHead( IMeList* list , void* data )
{
    IMeCList*  list_t  =   (IMeCList*)list;
    list_node_t*  new_node_t  =  NULL;
    
    new_node_t = list_node_alloc( list_t->m_pMemory , data );    
    
    if( new_node_t )
    {
        new_node_t->next = list_t->headNode;
        
        if( list_t->headNode )
            list_t->headNode->pre = new_node_t;
        
        if( !list_t->tailNode )
            list_t->tailNode = new_node_t;

        list_t->headNode = new_node_t;
        list_t->nNodeCount++;
    }
    
    return (uint32_t)new_node_t;
}

uint32_t   IMeCListAddTail( IMeList* list , void* data )
{
    IMeCList*  list_t  =   (IMeCList*)list;
    list_node_t*  new_node_t  =  NULL;

    new_node_t   =   list_node_alloc( list_t->m_pMemory , data );   

    if( new_node_t )
    {
        new_node_t->pre = list_t->tailNode;
        
        if( list_t->tailNode )
            list_t->tailNode->next = new_node_t;
        
        if( !list_t->headNode )
            list_t->headNode = new_node_t;

        list_t->tailNode = new_node_t;
        list_t->nNodeCount++;
    }

    return (uint32_t)new_node_t;
}

void*   IMeCListRemoveHead( IMeList* list )
{
    IMeCList*  list_t  =   (IMeCList*)list;
    void*    data = NULL;
    list_node_t* head = list_t->headNode;

    if( head )
    {
        data = head->data;
        
        if( head->next )
            head->next->pre = NULL;
        
        list_t->headNode = head->next;
        CMemoryFree( list_t->m_pMemory, head );
        
        if( NULL == list_t->headNode ) 
            list_t->tailNode = NULL;
        
        --list_t->nNodeCount;
    }

    return data;
}

void*   IMeCListRemoveTail( IMeList* list )
{
    IMeCList*  list_t  =   (IMeCList*)list;
    void*    data = NULL;
    list_node_t* tail = list_t->tailNode;
    
    if( tail )
    {
        data = tail->data;
        
        if( tail->pre )
            tail->pre->next = NULL;
        
        list_t->tailNode = tail->pre;
        CMemoryFree( list_t->m_pMemory, tail );
        
        if( NULL == list_t->tailNode ) 
            list_t->headNode = NULL;

        --list_t->nNodeCount;
    }
    
    return data;
}


void    IMeCListRemoveAll( IMeList* list )
{
    IMeCList* list_t = (IMeCList*)list;
    list_node_t* node_t = NULL;
    list_node_t* next = NULL;
    
    next = list_t->headNode;
    while( next != NULL )  
    {
        node_t = next;
        next = next->next;
        CMemoryFree(list_t->m_pMemory,node_t);
    }
    
    list_t->headNode = NULL;
    list_t->tailNode = NULL;
    list_t->nNodeCount = 0;
}

void    IMeCListDestroy( IMeList* list )
{
    IMeCList*  list_t  =   (IMeCList*)list;
    IMeCListRemoveAll(list);
    CMemoryFree(list_t->m_pMemory,list_t);
}

void    IMeCListOutPut( IMeList* list )
{
    IMeCList*  list_t  =   (IMeCList*)list;
    list_output_list( list_t->headNode , list_t->tailNode );
}

IME_EXTERN_C IMeList*    IMeCListCreate()
{
    IMeCList*  list_t = NULL;
    
    list_t = (IMeCList*)calloc(1,sizeof(IMeCList));
    if( !list_t )
    {
        DebugLogString( TRUE , "[IMeCListCreate] calloc failed!" );
        return NULL;
    }

    list_t->m_pMemory = CMemoryCreate(IMeMemory_Default);
    if( !list_t->m_pMemory )
    {
        DebugLogString( TRUE , "[IMeCListCreate] CMemoryCreate failed!" );
        return NULL;
    }

    ((IMeList*)list_t)->m_pAddHead  =   IMeCListAddHead;
    ((IMeList*)list_t)->m_pAddTail  =   IMeCListAddTail;
    ((IMeList*)list_t)->m_pRemoveHead   =   IMeCListRemoveHead;
    ((IMeList*)list_t)->m_pRemoveTail   =   IMeCListRemoveTail;
    ((IMeList*)list_t)->m_pGetHead  =   IMeCListGetHead;
    ((IMeList*)list_t)->m_pGetTail  =   IMeCListGetTail;
    ((IMeList*)list_t)->m_pGetPre   =   IMeCListGetPre;
    ((IMeList*)list_t)->m_pGetNext  =   IMeCListGetNext;
    ((IMeList*)list_t)->m_pGetCount =   IMeCListGetCount;
    ((IMeList*)list_t)->m_pDestroy  =   IMeCListDestroy;
    ((IMeList*)list_t)->m_pOutPut   =   IMeCListOutPut;

    return (IMeList*)list_t;
}
