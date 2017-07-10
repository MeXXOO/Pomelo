#ifndef		_ME_SOCKET_MANAGER_WORKER_H_
#define		_ME_SOCKET_MANAGER_WORKER_H_



/* manager dispatch user */
typedef struct _IMeSocketDispatchUser	IMeSocketDispatchUser;

typedef		void		(*IMeSocketDispatchUserRemove)( IMeSocketDispatchUser* pSocketDispatchUser );
typedef		void		(*IMeSocketDispatchUserSend)( IMeSocketDispatchUser* pSocketDispatchUser );
typedef		void		(*IMeSocketDispatchUserRcv)( IMeSocketDispatchUser* pSocketDispatchUser );
typedef		void		(*IMeSocketDispatchUserException)( IMeSocketDispatchUser* pSocketDispatchUser );
typedef		void		(*IMeSocketDispatchUserSetExtendParameter)( IMeSocketDispatchUser* pSocketDispatchUser , uint32_t extendParameter );
typedef		long	(*IMeSocketDispatchUserGetExtendParameter)( IMeSocketDispatchUser* pSocketDispatchUser );
typedef		IMeSocket*	(*IMeSocketDispatchUserGetSocket)( IMeSocketDispatchUser* pSocketDispatchUser );

struct _IMeSocketDispatchUser{
	IMeSocketDispatchUserGetSocket	m_pGetSocket;
	IMeSocketDispatchUserSend	m_pSend;
	IMeSocketDispatchUserRcv	m_pRcv;
	IMeSocketDispatchUserException	m_pException;
	IMeSocketDispatchUserRemove	m_pRemove;
	IMeSocketDispatchUserGetExtendParameter	m_pGetExtendParameter;
	IMeSocketDispatchUserSetExtendParameter	m_pSetExtendParameter;
};

#define	CSocketDispatchUserRemove(p)				(p->m_pRemove(p))
#define	CSocketDispatchUserSend(p)					(p->m_pSend(p))
#define	CSocketDispatchUserRcv(p)					(p->m_pRcv(p))
#define	CSocketDispatchUserException(p)				(p->m_pException(p))
#define	CSocketDispatchUserGetSocket(p)				(p->m_pGetSocket(p))
#define	CSocketDispatchUserGetExtendParameter(p)	(p->m_pGetExtendParameter(p))
#define	CSocketDispatchUserSetExtendParameter(p,a)	(p->m_pSetExtendParameter(p,a))


/* manager dispatch worker */
typedef	struct _IMeSocketDispatchGroup IMeSocketDispatchGroup;

typedef		int		(*IMeSocketDispatchGroupGetUserCnt)( IMeSocketDispatchGroup* pISocketDispatchGroup );
typedef		void	(*IMeSocketDispatchGroupAddUser)( IMeSocketDispatchGroup* pISocketDispatchGroup , IMeSocketDispatchUser* pISocketDispatchUser );
typedef		void	(*IMeSocketDispatchGroupRemoveUser)( IMeSocketDispatchGroup* pISocketDispatchGroup , IMeSocketDispatchUser* pISocketDispatchUser );
typedef		void	(*IMeSocketDispatchGroupAddEvent)( IMeSocketDispatchGroup* pISocketDispatchGroup , IMeSocketDispatchUser* pISocketDispatchUser , short ev );
typedef		void	(*IMeSocketDispatchGroupRemoveEvent)( IMeSocketDispatchGroup* pISocketDispatchGroup , IMeSocketDispatchUser* pISocketDispatchUser , short ev );
typedef		void	(*IMeSocketDispatchGroupDestroy)( IMeSocketDispatchGroup* pISocketDispatchGroup );


struct _IMeSocketDispatchGroup{
	IMeSocketDispatchGroupRemoveEvent	m_pRemoveEvent;
	IMeSocketDispatchGroupAddEvent	m_pAddEvent;
	IMeSocketDispatchGroupRemoveUser	m_pRemoveUser;
	IMeSocketDispatchGroupAddUser	m_pAddUser;
	IMeSocketDispatchGroupGetUserCnt	m_pGetUserCnt;
	IMeSocketDispatchGroupDestroy	m_pDestroy;
};

IME_EXTERN_C	IMeSocketDispatchGroup*		IMeSocketDispatchGroupCreate();

#define	CSocketDispatchGroupCreate()			IMeSocketDispatchGroupCreate()
#define	CSocketDispatchGroupGetUserCnt(p)		(p->m_pGetUserCnt(p))
#define	CSocketDispatchGroupAddEvent(p,a,b)		(p->m_pAddEvent(p,a,b))
#define	CSocketDispatchGroupRemoveEvent(p,a,b)	(p->m_pRemoveEvent(p,a,b))
#define	CSocketDispatchGroupAddUser(p,a)		(p->m_pAddUser(p,a))
#define	CSocketDispatchGroupRemoveUser(p,a)		(p->m_pRemoveUser(p,a))
#define	CSocketDispatchGroupDestroy(p)			(p->m_pDestroy(p))


#endif		//end _ME_SOCKET_MANAGER_WORKER_H_
