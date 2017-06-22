#ifndef     _ME_SOCKET_EVENT_LISTENER_H_
#define     _ME_SOCKET_EVENT_LISTENER_H_

#include    "include.h"

typedef		void	(*IMeSocketEventListenerOnNotify)( short events , IMeSocket* s , void* upApp );


/* socket event listener 类接口表 */
typedef struct _IMeSocketEventListener  IMeSocketEventListener;

typedef void    (*IMeSocketEventListenerDestroy)( IMeSocketEventListener* pIMeSocketEventListener );
typedef int     (*IMeSocketEventListenerAdd)( IMeSocketEventListener* pIMeSocketEventListener , IMeSocket* s , short old , short events );
typedef int     (*IMeSocketEventListenerDel)( IMeSocketEventListener* pIMeSocketEventListener , IMeSocket* s , short old , short events );
typedef int     (*IMeSocketEventListenerDispatch)( IMeSocketEventListener* pIMeSocketEventListener , uint timeMiliseconds );
typedef void    (*IMeSocketEventListenerRegister)( IMeSocketEventListener* pIMeSocketEventListener , IMeSocketEventListenerOnNotify notify , void* upApp );

struct _IMeSocketEventListener{
    IMeSocketEventListenerDestroy   m_pDestroy;
    IMeSocketEventListenerAdd   m_pAdd;
    IMeSocketEventListenerDel   m_pDel;
    IMeSocketEventListenerDispatch  m_pDispatch;
    IMeSocketEventListenerRegister	m_pRegister;
};

IME_EXTERN_C    IMeSocketEventListener* CSocketEventListenerSelectCreate();

#define		CSocketEventListenerCreate()			CSocketEventListenerSelectCreate()
#define     CSocketEventListenerDestroy(p)          (p->m_pDestroy(p))
#define     CSocketEventListenerAdd(p,a,b,c)		(p->m_pAdd(p,a,b,c))
#define     CSocketEventListenerDel(p,a,b,c)		(p->m_pDel(p,a,b,c))
#define     CSocketEventListenerDispatch(p,a)       (p->m_pDispatch(p,a))
#define     CSocketEventListenerRegister(p,a,b)		(p->m_pRegister(p,a,b))

#endif      //end _ME_SOCKET_EVENT_LISTENER_H_