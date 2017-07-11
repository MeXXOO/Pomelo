#include	"../include/include.h"

//dispatch user event
#define		MAKE_EVENT_KEY(p)	(((uint64_t)(uint32_t)((p)->pSocket))<<32|(int)((p)->ev)<<16|(p)->bAdd)
typedef	struct _IMeCSocketDispatchUserEvent{
	short	ev;
	uint8_t	bAdd;
	IMeSocket* pSocket;
}IMeCSocketDispatchUserEvent;

typedef struct _IMeCSocketDispatchGroup{
	
	IMeSocketDispatchGroup	vtbl;

	uint8_t	m_bRunning;

	IMeThread*	m_pWorkerThread;
	
	//user info in this group
	int			m_nGrpUserCnt;
	IMeArray*	m_parrGrpUser;
	IMeLock*	m_pLockGrpUser;	

	//user exit
	IMeList*	m_pListReleaseSocket;

	//user register event
	IMeLock*	m_pLockEvent;
	IMeArray*	m_parrEvent;
	IMeList*	m_pListFreeEvent;

	IMeSocketEventListener*	m_pSocketEventLisenter;

}IMeCSocketDispatchGroup;


static void IMeCSocketDispatchGroupReleaseEventList( IMeCSocketDispatchGroup* pSocketDispatchGroup )
{
    int i;
	IMeCSocketDispatchUserEvent* pUserEvent;

	CLock_Lock( pSocketDispatchGroup->m_pLockEvent );
	
	while( (pUserEvent=(IMeCSocketDispatchUserEvent*)CListRemoveHead(pSocketDispatchGroup->m_pListFreeEvent)) )
		free( pUserEvent );

    for( i=0; i<CArrayGetSize(pSocketDispatchGroup->m_parrEvent); i++ )
    {
	    pUserEvent=(IMeCSocketDispatchUserEvent*)CArrayGetAt(pSocketDispatchGroup->m_parrEvent,0);
		free( pUserEvent );
    }
    CArrayRemoveAll( pSocketDispatchGroup->m_parrEvent );

	CLock_Unlock( pSocketDispatchGroup->m_pLockEvent );
}

static void IMeCSocketDispatchGroupReleaseUserList( IMeCSocketDispatchGroup* pSocketDispatchGroup )
{
    int i;
	IMeSocket* pSocket;
    IMeSocketDispatchUser*	pSocketDispatchUser;
	
	CLock_Lock( pSocketDispatchGroup->m_pLockGrpUser );
	
    for( i=0; i<CArrayGetSize(pSocketDispatchGroup->m_parrGrpUser); i++ )
    {
        pSocket=(IMeSocket*)CArrayGetAt(pSocketDispatchGroup->m_parrGrpUser,0);

        pSocketDispatchUser = (IMeSocketDispatchUser*)CSocketGetExtendParameter(pSocket);
        CSocketDispatchUserRemove( pSocketDispatchUser );

        CSocketEventListenerDel( pSocketDispatchGroup->m_pSocketEventLisenter , pSocket , 0 , SEvent_All );
		CSocketDestroy( pSocket );
    }
    CArrayRemoveAll( pSocketDispatchGroup->m_parrGrpUser );
	

	while( (pSocket=(IMeSocket*)CListRemoveHead(pSocketDispatchGroup->m_pListReleaseSocket)) )
	{
        pSocketDispatchUser = (IMeSocketDispatchUser*)CSocketGetExtendParameter(pSocket);
        CSocketDispatchUserRemove( pSocketDispatchUser );

		CSocketEventListenerDel( pSocketDispatchGroup->m_pSocketEventLisenter , pSocket , 0 , SEvent_All );
		CSocketDestroy( pSocket );
	}

	CLock_Unlock( pSocketDispatchGroup->m_pLockGrpUser );
}

static void	IMeCSocketDispatchGroupWorkEventListenerOnNotify( short events , IMeSocket* s , void* upApp )
{
	IMeCSocketDispatchGroup* pSocketDispatchGroup = (IMeCSocketDispatchGroup*)upApp;
	IMeSocketDispatchUser*	pSocketDispatchUser;

	CLock_Lock( pSocketDispatchGroup->m_pLockGrpUser );

	pSocketDispatchUser = (IMeSocketDispatchUser*)CSocketGetExtendParameter(s);

	if( pSocketDispatchUser )
	{	
		//has read event
		if( events&SEvent_Read )
		{
			CSocketDispatchUserRcv(pSocketDispatchUser);
		}
		
		//has write event
		if( events&SEvent_Write )
		{
			CSocketDispatchUserSend(pSocketDispatchUser);
		}
		
		//has exception event
		if( events&SEvent_Exception )
		{
			CSocketDispatchUserException(pSocketDispatchUser);
		}
	}

	CLock_Unlock( pSocketDispatchGroup->m_pLockGrpUser );
}

IME_EXTERN_C void IMeCSocketDispatchGroupCheckUserEvent( IMeCSocketDispatchGroup* pSocketDispatchGroup )
{
	IMeCSocketDispatchUserEvent* pUserEvent;
    int nFreeIndex;

	CLock_Lock( pSocketDispatchGroup->m_pLockEvent );

	while( pSocketDispatchGroup->m_bRunning )
	{   
        pUserEvent = (IMeCSocketDispatchUserEvent*)CArrayRemoveAt(pSocketDispatchGroup->m_parrEvent,0);
        
        if( !pUserEvent )   break;

        CLock_Lock( pSocketDispatchGroup->m_pLockGrpUser );
        nFreeIndex = CArrayFind( pSocketDispatchGroup->m_parrGrpUser , (uint32_t)(pUserEvent->pSocket) );
        CLock_Unlock( pSocketDispatchGroup->m_pLockGrpUser );

		if( nFreeIndex != -1 )
		{
			if( pUserEvent->bAdd )
			{				
				CSocketEventListenerAdd( pSocketDispatchGroup->m_pSocketEventLisenter , pUserEvent->pSocket , 0 , pUserEvent->ev );
			}
			else
			{
				CSocketEventListenerDel( pSocketDispatchGroup->m_pSocketEventLisenter , pUserEvent->pSocket , 0 , pUserEvent->ev );
			}
		}
		 
		CListAddTail( pSocketDispatchGroup->m_pListFreeEvent , pUserEvent );
	}
	
	CLock_Unlock( pSocketDispatchGroup->m_pLockEvent );
}

IME_EXTERN_C void IMeCSocketDispatchGroupCheckReleaseSocket( IMeCSocketDispatchGroup* pSocketDispatchGroup )
{
	IMeSocket* pSocket;
    IMeSocketDispatchUser* pSocketDispatchUser;

	CLock_Lock( pSocketDispatchGroup->m_pLockGrpUser );

	while( pSocketDispatchGroup->m_bRunning )
	{
        pSocket = (IMeSocket*)CListRemoveHead(pSocketDispatchGroup->m_pListReleaseSocket);
        if( !pSocket )  break;
        
        pSocketDispatchUser = (IMeSocketDispatchUser*)CSocketGetExtendParameter(pSocket);
        CSocketDispatchUserRemove( pSocketDispatchUser );

        CSocketEventListenerDel( pSocketDispatchGroup->m_pSocketEventLisenter , pSocket , 0 , SEvent_All );
		CSocketDestroy(pSocket);
	}

	CLock_Unlock( pSocketDispatchGroup->m_pLockGrpUser );
}

static void	IMeCSocketDispatchGroupWorkThread( void* pWorkGroup )
{
	IMeCSocketDispatchGroup* pSocketDispatchGroup = (IMeCSocketDispatchGroup*)pWorkGroup;

	while( pSocketDispatchGroup->m_bRunning )
	{
		CSocketEventListenerDispatch( pSocketDispatchGroup->m_pSocketEventLisenter , 10 );
		IMeCSocketDispatchGroupCheckUserEvent( pSocketDispatchGroup );
		IMeCSocketDispatchGroupCheckReleaseSocket( pSocketDispatchGroup );
	}
}

IME_EXTERN_C	void	IMeCSocketDispatchGroupNewEvent( IMeSocketDispatchGroup* pISocketDispatchGroup , IMeSocketDispatchUser* pISocketDispatchUser , short ev , uint8_t bAdd )
{
	IMeCSocketDispatchGroup* pSocketDispatchGroup = (IMeCSocketDispatchGroup*)pISocketDispatchGroup;
	IMeCSocketDispatchUserEvent stUserEvent, *pUserEvent = &stUserEvent;
	IMeSocket* pSocket = CSocketDispatchUserGetSocket(pISocketDispatchUser);

	stUserEvent.bAdd = bAdd;
	stUserEvent.ev = ev;
	stUserEvent.pSocket = pSocket;

	CLock_Lock( pSocketDispatchGroup->m_pLockEvent );
	if( -1 != CArrayFind( pSocketDispatchGroup->m_parrEvent, MAKE_EVENT_KEY(pUserEvent) ) )
	{
        //DebugLogString( true , "[IMeCSocketDispatchGroupNewEvent] have find event!!" );
		CLock_Unlock( pSocketDispatchGroup->m_pLockEvent );
		return;
	}
	pUserEvent = (IMeCSocketDispatchUserEvent*)CListRemoveHead(pSocketDispatchGroup->m_pListFreeEvent);
	CLock_Unlock( pSocketDispatchGroup->m_pLockEvent );

	if( !pUserEvent )
		pUserEvent = (IMeCSocketDispatchUserEvent*)calloc( 1, sizeof(IMeCSocketDispatchUserEvent) );
	
	if( pUserEvent )
	{
		pUserEvent->bAdd = bAdd;
		pUserEvent->ev = ev;
		pUserEvent->pSocket = pSocket;

		CLock_Lock( pSocketDispatchGroup->m_pLockEvent );
		CArrayAdd( pSocketDispatchGroup->m_parrEvent , pUserEvent , MAKE_EVENT_KEY(pUserEvent) );
		CLock_Unlock( pSocketDispatchGroup->m_pLockEvent );
	}
	else
	{
		DebugLogString( true , "[IMeCSocketDispatchGroupNewEvent] failed socket user:%0x bAdd:%d!!" , pISocketDispatchUser , bAdd );
	}
}

IME_EXTERN_C	void	IMeCSocketDispatchGroupRemoveEvent( IMeSocketDispatchGroup* pISocketDispatchGroup , IMeSocketDispatchUser* pISocketDispatchUser , short ev )
{
	IMeCSocketDispatchGroupNewEvent( pISocketDispatchGroup , pISocketDispatchUser , ev , false );
}

IME_EXTERN_C	void	IMeCSocketDispatchGroupAddEvent( IMeSocketDispatchGroup* pISocketDispatchGroup , IMeSocketDispatchUser* pISocketDispatchUser , short ev )
{
	IMeCSocketDispatchGroupNewEvent( pISocketDispatchGroup , pISocketDispatchUser , ev , true );
}

IME_EXTERN_C	void	IMeCSocketDispatchGroupRemoveUser( IMeSocketDispatchGroup* pISocketDispatchGroup , IMeSocketDispatchUser* pISocketDispatchUser )
{
	IMeCSocketDispatchGroup* pSocketDispatchGroup = (IMeCSocketDispatchGroup*)pISocketDispatchGroup;
	IMeSocket* pSocket;

	pSocket = CSocketDispatchUserGetSocket(pISocketDispatchUser);

	CLock_Lock( pSocketDispatchGroup->m_pLockGrpUser );
	
	pSocket = (IMeSocket*)CArrayRemove( pSocketDispatchGroup->m_parrGrpUser , (uint32_t)pSocket );
	if( pSocket )
	{
		CListAddTail( pSocketDispatchGroup->m_pListReleaseSocket , pSocket );
	}

	CLock_Unlock( pSocketDispatchGroup->m_pLockGrpUser );
}

IME_EXTERN_C	void	IMeCSocketDispatchGroupAddUser( IMeSocketDispatchGroup* pISocketDispatchGroup , IMeSocketDispatchUser* pISocketDispatchUser )
{
	IMeCSocketDispatchGroup* pSocketDispatchGroup = (IMeCSocketDispatchGroup*)pISocketDispatchGroup;
	IMeSocket* pSocket = CSocketDispatchUserGetSocket(pISocketDispatchUser);
	
	CLock_Lock( pSocketDispatchGroup->m_pLockGrpUser );

	if( -1 == CArrayFind( pSocketDispatchGroup->m_parrGrpUser , (uint32_t)pSocket ) )
	{
		CArrayAdd( pSocketDispatchGroup->m_parrGrpUser , pSocket , (uint32_t)pSocket );	
	}

	CLock_Unlock( pSocketDispatchGroup->m_pLockGrpUser );
}

IME_EXTERN_C	int		IMeCSocketDispatchGroupGetUserCnt( IMeSocketDispatchGroup* pISocketDispatchGroup )
{
	IMeCSocketDispatchGroup* pSocketDispatchGroup = (IMeCSocketDispatchGroup*)pISocketDispatchGroup;
	int nUserCnt = 0;

	if( pSocketDispatchGroup->m_bRunning )
    {	
	    CLock_Lock( pSocketDispatchGroup->m_pLockGrpUser );
	    nUserCnt = CArrayGetSize( pSocketDispatchGroup->m_parrGrpUser );
	    CLock_Unlock( pSocketDispatchGroup->m_pLockGrpUser );
    }

	return nUserCnt;
}


IME_EXTERN_C	void	IMeCSocketDispatchGroupDestroy( IMeSocketDispatchGroup* pISocketDispatchGroup )
{
	IMeCSocketDispatchGroup* pSocketDispatchGroup = (IMeCSocketDispatchGroup*)pISocketDispatchGroup;

	pSocketDispatchGroup->m_bRunning = true;

	if( pSocketDispatchGroup->m_pWorkerThread )
	{
		CThreadExit( pSocketDispatchGroup->m_pWorkerThread , 2000 );
		CThreadDestroy( pSocketDispatchGroup->m_pWorkerThread );
		pSocketDispatchGroup->m_pWorkerThread = NULL;
	}

	IMeCSocketDispatchGroupReleaseEventList( pSocketDispatchGroup );
	CArrayDestroy( pSocketDispatchGroup->m_parrEvent );
	CListDestroy( pSocketDispatchGroup->m_pListFreeEvent );

	IMeCSocketDispatchGroupReleaseUserList( pSocketDispatchGroup );
	CArrayDestroy( pSocketDispatchGroup->m_parrGrpUser );
	CListDestroy( pSocketDispatchGroup->m_pListReleaseSocket );

	CSocketEventListenerDestroy( pSocketDispatchGroup->m_pSocketEventLisenter );

	CLock_Destroy( pSocketDispatchGroup->m_pLockGrpUser );
	CLock_Destroy( pSocketDispatchGroup->m_pLockEvent );

	free( pSocketDispatchGroup );
}

IME_EXTERN_C	IMeSocketDispatchGroup*		IMeSocketDispatchGroupCreate()
{
	IMeCSocketDispatchGroup* pSocketDispatchGroup = (IMeCSocketDispatchGroup*)calloc(1,sizeof(IMeCSocketDispatchGroup));
	
	if( pSocketDispatchGroup )
	{	
		pSocketDispatchGroup->m_parrGrpUser = CArrayCreate(SORT_INC);
		pSocketDispatchGroup->m_pListReleaseSocket = CListCreate();
		pSocketDispatchGroup->m_pLockGrpUser = CLock_Create();

		pSocketDispatchGroup->m_pListFreeEvent = CListCreate();
		pSocketDispatchGroup->m_parrEvent = CArrayCreate(SORT_INC);
		pSocketDispatchGroup->m_pLockEvent = CLock_Create();

		pSocketDispatchGroup->m_pSocketEventLisenter = CSocketEventListenerCreate();
		CSocketEventListenerRegister( pSocketDispatchGroup->m_pSocketEventLisenter , IMeCSocketDispatchGroupWorkEventListenerOnNotify , pSocketDispatchGroup );

		pSocketDispatchGroup->m_bRunning = true;
		pSocketDispatchGroup->m_pWorkerThread = CThreadCreate( (void*)IMeCSocketDispatchGroupWorkThread , pSocketDispatchGroup );

		//interface
		pSocketDispatchGroup->vtbl.m_pAddUser = IMeCSocketDispatchGroupAddUser;
		pSocketDispatchGroup->vtbl.m_pRemoveUser = IMeCSocketDispatchGroupRemoveUser;
		pSocketDispatchGroup->vtbl.m_pAddEvent = IMeCSocketDispatchGroupAddEvent;
		pSocketDispatchGroup->vtbl.m_pRemoveEvent = IMeCSocketDispatchGroupRemoveEvent;
		pSocketDispatchGroup->vtbl.m_pGetUserCnt = IMeCSocketDispatchGroupGetUserCnt;
		pSocketDispatchGroup->vtbl.m_pDestroy = IMeCSocketDispatchGroupDestroy;
	}

	return (IMeSocketDispatchGroup*)pSocketDispatchGroup;
}
