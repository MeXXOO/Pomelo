/*
 * Copyright 2007-2012 Niels Provos and Nick Mathewson
 * Copyright 2000-2007 Niels Provos <provos@citi.umich.edu>
 * Copyright 2003 Michael A. Davis <mike@datanerds.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include    "../include/include.h"

#ifdef  PROJECT_FOR_WINDOWS

#include	<limits.h>

#define XFREE(ptr) do { if (ptr) free(ptr); } while (0)

struct win_fd_set {
    uint32_t fd_count;					/* how many are SET? */
    int  fd_array[FD_SETSIZE];   		/* an array of SOCKETs */
	IMeSocket* socket_array[FD_SETSIZE];
};

/* MSDN says this is required to handle SIGFPE */
volatile double SIGFPE_REQ = 0.0f;

struct idx_info {
	int read_pos_plus1;
	int write_pos_plus1;
};

struct win32op {
	struct win_fd_set *readset_in;
	struct win_fd_set *writeset_in;
	struct win_fd_set *readset_out;
	struct win_fd_set *writeset_out;
};

typedef struct _IMeSocketEventSelectListener {
    IMeSocketEventListener  interfacefunction;
    struct win32op* op;

	IMeSocketEventListenerOnNotify	evNotify;
    void* upApp;
}IMeSocketEventSelectListener;


#define MAX_SECONDS_IN_MSEC_LONG \
(((LONG_MAX) - 999) / 1000)

long	evutil_tv_to_msec(const struct timeval *tv)
{
    if (tv->tv_usec > 1000000 || tv->tv_sec > MAX_SECONDS_IN_MSEC_LONG)
        return -1;
    
    return (tv->tv_sec * 1000) + ((tv->tv_usec + 999) / 1000);
}

static int	do_fd_set(struct win32op *op, struct idx_info *ent, IMeSocket* s, int read)
{
	struct win_fd_set *set = read ? op->readset_in : op->writeset_in;
	
	//already in array
	if (read) {
		if (ent->read_pos_plus1 > 0)
			return (0);
	} else {
		if (ent->write_pos_plus1 > 0)
			return (0);
	}

	if (set->fd_count == FD_SETSIZE) {
		DebugLogString( TRUE , "[do_fd_set] reach max socket count !!" );
		return (-1);
	}

	set->fd_array[set->fd_count] = CSocketGetFd(s);
	set->socket_array[set->fd_count] = s;
	if (read)
		ent->read_pos_plus1 = set->fd_count+1;
	else
		ent->write_pos_plus1 = set->fd_count+1;
	
	return (set->fd_count++);
}

static int	do_fd_clear(struct win32op *op, struct idx_info *ent, int read)
{
	int i;
	struct win_fd_set *set = read ? op->readset_in : op->writeset_in;
	if (read) {
		i = ent->read_pos_plus1 - 1;
		ent->read_pos_plus1 = 0;
	} else {
		i = ent->write_pos_plus1 - 1;
		ent->write_pos_plus1 = 0;
	}
	if (i < 0)
		return (0);

	if (--set->fd_count != (unsigned)i) {
		struct idx_info *ent2;
		IMeSocket* pSocket;

		/* exchange */
		set->fd_array[i] = set->fd_array[set->fd_count];
		pSocket = set->socket_array[i] = set->socket_array[set->fd_count];
		ent2 = (struct idx_info*)CSocketGetEventParameter( pSocket );

		if (!ent2) /* This indicates a bug. */
		{
			DebugLogString( TRUE , "[do_fd_clear] program error!!" );
			return (0);
		}

		if (read)
			ent2->read_pos_plus1 = i+1;
		else
			ent2->write_pos_plus1 = i+1;
	}
	return (0);
}

#define NEVENT 32
void* win32_init(/*struct event_base *_base*/)
{
	struct win32op *winop;
	size_t size;
	if (!(winop = calloc(1, sizeof(struct win32op))))
		return NULL;
	size = sizeof(struct win_fd_set);
	if (!(winop->readset_in = malloc(size)))
		goto err;
	if (!(winop->writeset_in = malloc(size)))
		goto err;
	if (!(winop->readset_out = malloc(size)))
		goto err;
	if (!(winop->writeset_out = malloc(size)))
		goto err;

	winop->readset_in->fd_count = winop->writeset_in->fd_count = 0;
	winop->readset_out->fd_count = winop->writeset_out->fd_count = 0;
	
	return (winop);
 err:
	XFREE(winop->readset_in);
	XFREE(winop->writeset_in);
	XFREE(winop->readset_out);
	XFREE(winop->writeset_out);
	XFREE(winop);
	return (NULL);
}

int	win32_add(IMeSocketEventSelectListener* pSelectListener, IMeSocket* s, short old, short events)
{
	struct win32op *win32op = pSelectListener->op;
	struct idx_info *idx = (struct idx_info *)CSocketGetEventParameter(s);
	uint8_t bfree = FALSE;

	if (!(events & (SEvent_Read|SEvent_Write)))
	{
		DebugLogString( TRUE , "[win32_add] no support event!!" );
		return (0);
	}

	if( !idx )
	{
		idx = (struct idx_info *)calloc(1,sizeof(struct idx_info));
		bfree = TRUE;
	}

	if( !idx )
	{
		DebugLogString( TRUE , "[win32_add] calloc struct idx_info failed!!" );
		return -1;
	}

	if (events & SEvent_Read) {
		if (do_fd_set(win32op, idx, s, 1)<0)
		{
			DebugLogString( TRUE , "[win32_add] do_fd_set read event failed!!" );
			if( bfree )	free(idx);
			return (-1);
		}
	}
	if (events & SEvent_Write) {
		if (do_fd_set(win32op, idx, s, 0)<0)
		{
			DebugLogString( TRUE , "[win32_add] do_fd_set write event failed!!" );
			if( bfree ) free(idx);
			return (-1);
		}
	}
	
	//save index info
	CSocketSetEventParameter(s,(uint32_t)idx);

	return (0);
}

int		win32_del(IMeSocketEventSelectListener* pSelectListener, IMeSocket* s, short old, short events)
{
	struct win32op *win32op = pSelectListener->op;
	struct idx_info *idx = (struct idx_info *)CSocketGetEventParameter(s);

	if( !idx )
	{
		DebugLogString( TRUE , "[win32_del] idx null pointer!!" );
		return -1;
	}

	if (events & SEvent_Read)
		do_fd_clear(win32op, idx, 1);
	if (events & SEvent_Write)
		do_fd_clear(win32op, idx, 0);

	//clear index info
	if( idx->read_pos_plus1==0 && idx->write_pos_plus1==0 )
	{
		free(idx);
		CSocketSetEventParameter(s,(uint32_t)NULL);	
		return 1;
	}

	return 0;
}

static void	fd_set_copy(struct win_fd_set *out, const struct win_fd_set *in )
{
	out->fd_count = in->fd_count;
	memcpy( out->fd_array, in->fd_array, in->fd_count * (sizeof(SOCKET)) );
	memcpy( out->socket_array, in->socket_array, in->fd_count * (sizeof(IMeSocket*)) );
}

int	win32_dispatch( IMeSocketEventSelectListener* pSelectListener, struct timeval *tv)
{
	struct win32op *win32op = pSelectListener->op;
	int res = 0;
	unsigned i;
	int fd_count;
	IMeSocket* s;
	int fd;
	
	fd_set_copy(win32op->readset_out, win32op->readset_in);
	fd_set_copy(win32op->writeset_out, win32op->writeset_in);

	fd_count =
	    (win32op->readset_out->fd_count > win32op->writeset_out->fd_count) ?
	    win32op->readset_out->fd_count : win32op->writeset_out->fd_count;

	if (!fd_count) {
		long msec = tv ? evutil_tv_to_msec(tv) : LONG_MAX;
		/* Sleep's DWORD argument is unsigned long */
		if (msec < 0)
			msec = LONG_MAX;
		/* Windows doesn't like you to call select() with no sockets */
		Sleep(msec);
		return (0);
	}

	res = select( fd_count+1, (struct fd_set*)win32op->readset_out, (struct fd_set*)win32op->writeset_out, NULL, tv );

	//DebugLogString( TRUE , "[win32_dispatch]: select error %d!!" , res );

	if (res <= 0) {
		//DebugLogString( TRUE , "[win32_dispatch]: select error %s", IMeSocketConvertErrorCodeToString(IMeSocketGetNetError()) );
		return res;
	}

	if( !pSelectListener->evNotify )
		return 0;

	//check socket read
	if( win32op->readset_out->fd_count )
	{
		for( i=0; i<win32op->readset_in->fd_count; ++i )
		{
			s = win32op->readset_in->socket_array[i];
			fd = CSocketGetFd(s);
			if( FD_ISSET(fd,win32op->readset_out) )
			{
				pSelectListener->evNotify(SEvent_Read, s, pSelectListener->upApp);
			}
		}
	}

	//check socket write
	if( win32op->writeset_out->fd_count ) 
	{
		for( i=0; i<win32op->writeset_in->fd_count; ++i )
		{
			s = win32op->writeset_in->socket_array[i];
			fd = CSocketGetFd(s);
			if( FD_ISSET(fd,win32op->writeset_out) )
			{
				pSelectListener->evNotify(SEvent_Write, s, pSelectListener->upApp);
			}
		}
	}
	
	return (0);
}

void	win32_dealloc(IMeSocketEventSelectListener* pSelectListener)
{
	struct win32op *win32op = pSelectListener->op;
	uint32_t i;
	IMeSocket* s;
	struct idx_info* idx;

	//evsig_dealloc(_base);
	if (win32op->readset_in)
	{
		for(i=0; i<win32op->readset_in->fd_count; i++)
		{
			s = win32op->readset_in->socket_array[i];
			idx = (struct idx_info*)CSocketGetEventParameter(s);
			if(idx)
			{
				free(idx);
				CSocketSetEventParameter(s,(uint32_t)NULL);
			}	
		}
		free(win32op->readset_in);
	}

	if (win32op->writeset_in)
	{
		for(i=0; i<win32op->writeset_in->fd_count; i++)
		{
			s = win32op->writeset_in->socket_array[i];
			idx = (struct idx_info*)CSocketGetEventParameter(s);
			if(idx)
			{
				free(idx);
				CSocketSetEventParameter(s,(uint32_t)NULL);
			}	
		}
		free(win32op->writeset_in);
	}

	if (win32op->readset_out)
		free(win32op->readset_out);
	if (win32op->writeset_out)
		free(win32op->writeset_out);
	
	/* XXXXX free the tree. */

	memset(win32op, 0, sizeof(*win32op));
	free(win32op);
}


void    IMeSocketEventSelectListenerDestroy( IMeSocketEventListener* pIMeSocketEventListener )
{
    IMeSocketEventSelectListener* pSelectListener = (IMeSocketEventSelectListener*)pIMeSocketEventListener;
    if( pSelectListener )
    {
        win32_dealloc( pSelectListener );
        free(pSelectListener);
    }
}

int    IMeSocketEventSelectListenerAdd( IMeSocketEventListener* pIMeSocketEventListener , IMeSocket* s , short old , short events )
{
    IMeSocketEventSelectListener* pSelectListener = (IMeSocketEventSelectListener*)pIMeSocketEventListener;
    if( pSelectListener )
    {
        return win32_add( pSelectListener , s , old , events );
    }

	return -1;
}

int    IMeSocketEventSelectListenerDel( IMeSocketEventListener* pIMeSocketEventListener , IMeSocket* s , short old , short events )
{
    IMeSocketEventSelectListener* pSelectListener = (IMeSocketEventSelectListener*)pIMeSocketEventListener;
    if( pSelectListener )
    {
        return win32_del( pSelectListener , s , old , events );
    }

	return -1;
}

int     IMeSocketEventSelectListenerDispatch( IMeSocketEventListener* pIMeSocketEventListener , uint32_t timeMiliseconds )
{
    IMeSocketEventSelectListener* pSelectListener = (IMeSocketEventSelectListener*)pIMeSocketEventListener;
    if( pSelectListener )
    {
		struct timeval time;
		time.tv_sec = timeMiliseconds/1000;
		time.tv_usec = (timeMiliseconds%1000)*1000;
        return win32_dispatch( pSelectListener , &time );
    }
    return -1;
}

void    IMeSocketEventSelectListenerRegister( IMeSocketEventListener* pIMeSocketEventListener , IMeSocketEventListenerOnNotify notify , void* upApp )
{
    IMeSocketEventSelectListener* pSelectListener = (IMeSocketEventSelectListener*)pIMeSocketEventListener;
    if( pSelectListener )
    {
		pSelectListener->evNotify = notify;
		pSelectListener->upApp = upApp;
    }
}

IME_EXTERN_C  IMeSocketEventListener*  CSocketEventListenerSelectCreate()
{
    IMeSocketEventSelectListener* pSelectListener = (IMeSocketEventSelectListener*)calloc(1,sizeof(IMeSocketEventSelectListener));
    
    if( pSelectListener )
    {
        pSelectListener->op = win32_init();
        if( !pSelectListener->op )
        {
            DebugLogString( TRUE , "[CSocketEventListenerSelectCreate] invoke win32_init failed!!" );
        }
        ((IMeSocketEventListener*)&pSelectListener->interfacefunction)->m_pAdd = IMeSocketEventSelectListenerAdd;
        ((IMeSocketEventListener*)&pSelectListener->interfacefunction)->m_pDel = IMeSocketEventSelectListenerDel;
        ((IMeSocketEventListener*)&pSelectListener->interfacefunction)->m_pDispatch = IMeSocketEventSelectListenerDispatch;
        ((IMeSocketEventListener*)&pSelectListener->interfacefunction)->m_pDestroy = IMeSocketEventSelectListenerDestroy;
        ((IMeSocketEventListener*)&pSelectListener->interfacefunction)->m_pRegister = IMeSocketEventSelectListenerRegister;
    }

    return (IMeSocketEventListener*)pSelectListener;
}




#elif   PROJECT_FOR_LINUX   /////////////////////////////////////////////////////////////////////////////////////////////



#ifndef _EVENT_HAVE_FD_MASK
/* This type is mandatory, but Android doesn't define it. */
typedef unsigned long fd_mask;
#endif

#ifndef NFDBITS
#define NFDBITS (sizeof(fd_mask)*8)
#endif

/* Divide positive x by y, rounding up. */
#define DIV_ROUNDUP(x, y)   (((x)+((y)-1))/(y))

/* How many bytes to allocate for N fds? */
#define SELECT_ALLOC_SIZE(n)	( DIV_ROUNDUP(n, NFDBITS) * sizeof(fd_mask) )
#define SOCKET_ALLOC_SIZE(n)	( (n-1)*sizeof(IMeSocket*) )	

struct linux_fd_set{
	fd_mask fds_bits[DIV_ROUNDUP(FD_SETSIZE, NFDBITS)];
	IMeSocket* socket_array[1];
};

struct idx_info {
	int read_pos_plus1;
	int write_pos_plus1;
};

struct selectop {
	int event_fds;		/* Highest fd in fd set */
	int event_fdsz;
	int event_socketsz;
	int resize_out_sets;
	struct linux_fd_set *event_readset_in;
	struct linux_fd_set *event_writeset_in;
	struct linux_fd_set *event_readset_out;
	struct linux_fd_set *event_writeset_out;
};

typedef struct _IMeSocketEventSelectListener {
    IMeSocketEventListener  interfacefunction;
    struct selectop* sop;

	IMeSocketEventListenerOnNotify	evNotify;
	void* upApp;
}IMeSocketEventSelectListener;


static int select_resize(struct selectop *sop, int fdsz);
static void select_free_selectop(struct selectop *sop);

static void*	select_init()
{
	struct selectop *sop;

	if(!(sop = calloc(1, sizeof(struct selectop))))
	{
		DebugLogString( TRUE , "[select_init] calloc failed!!" );
		return (NULL);
	}

	if (select_resize(sop, SELECT_ALLOC_SIZE(32 + 1))) 
	{
		select_free_selectop(sop);
		DebugLogString( TRUE , "[select_init] select_resize failed!!" );
		return (NULL);
	}

	return (sop);
}


static int	select_dispatch(IMeSocketEventSelectListener* pSelectListener, struct timeval *tv)
{
	int res=0, i, nfds;
	struct selectop *sop = pSelectListener->sop;

	if( !pSelectListener->evNotify )	return -1;

	if( sop->resize_out_sets ) 
	{
		struct linux_fd_set *readset_out=NULL;
		struct linux_fd_set *writeset_out=NULL;
		
		size_t sz = sop->event_fdsz;

		if (!(readset_out = realloc(sop->event_readset_out, sz)))
			return (-1);
		sop->event_readset_out = readset_out;

		if (!(writeset_out = realloc(sop->event_writeset_out, sz))) 
			return (-1);
		sop->event_writeset_out = writeset_out;

		sop->resize_out_sets = 0;
	}

	memcpy(sop->event_readset_out, sop->event_readset_in, sop->event_fdsz);
	memcpy(sop->event_writeset_out, sop->event_writeset_in, sop->event_fdsz);

	nfds = sop->event_fds+1;

	res = select(nfds, (fd_set*)sop->event_readset_out, (fd_set*)sop->event_writeset_out, NULL, tv);

	if( res <=0 )
	{
		//DebugLogString( TRUE , "[select_dispatch]: select error %s", IMeSocketConvertErrorCodeToString(IMeSocketGetNetError()) );
		return res;
	}

	for( i = 0; i < nfds; ++i ) 
	{
		if( FD_ISSET(i, (fd_set*)sop->event_readset_out) )
		{
			char* pArrayReadSocket = (char*)sop->event_readset_in + sop->event_fdsz;
			IMeSocket* s = (IMeSocket*)(*(uint32_t*)&pArrayReadSocket[i*sizeof(IMeSocket*)]);
			pSelectListener->evNotify( SEvent_Read , s , pSelectListener->upApp );
		}

		if( FD_ISSET(i, (fd_set*)sop->event_writeset_out) )
		{
			char* pArrayWriteSocket = (char*)sop->event_writeset_in + sop->event_fdsz;
			IMeSocket* s = (IMeSocket*)(*(uint32_t*)&pArrayWriteSocket[i*sizeof(IMeSocket*)]);
			pSelectListener->evNotify( SEvent_Write , s , pSelectListener->upApp );
		}
	}

	return (0);
}

static int	select_resize(struct selectop *sop, int fdsz)
{
	struct linux_fd_set *readset_in = NULL;
	struct linux_fd_set *writeset_in = NULL;
	int socket_sz = SOCKET_ALLOC_SIZE(fdsz*8); //each byte can set 8 raw sockets, so multiply 8

	if ((readset_in = realloc(sop->event_readset_in, fdsz+socket_sz)) == NULL)
		goto error;
	sop->event_readset_in = readset_in;

	if ((writeset_in = realloc(sop->event_writeset_in, fdsz+socket_sz)) == NULL) {
		/* Note that this will leave event_readset_in expanded.
		 * That's okay; we wouldn't want to free it, since that would
		 * change the semantics of select_resize from "expand the
		 * readset_in and writeset_in, or return -1" to "expand the
		 * *set_in members, or trash them and return -1."
		 */
		goto error;
	}
	sop->event_writeset_in = writeset_in;

	sop->resize_out_sets = 1;

	//first socket_array copy
	memcpy( (char *)sop->event_readset_in + sop->event_fdsz, (char*)sop->event_readset_in + fdsz, sop->event_socketsz );
	memcpy( (char *)sop->event_writeset_in + sop->event_fdsz, (char*)sop->event_writeset_in + fdsz, sop->event_socketsz );

	//fd_set init
	memset((char *)sop->event_readset_in + sop->event_fdsz, 0, fdsz - sop->event_fdsz);
	memset((char *)sop->event_writeset_in + sop->event_fdsz, 0, fdsz - sop->event_fdsz);

	//socket_array init
	memset((char *)sop->event_readset_in + fdsz + sop->event_socketsz, 0, socket_sz - sop->event_socketsz);
	memset((char *)sop->event_writeset_in + fdsz + sop->event_socketsz, 0, socket_sz - sop->event_socketsz);

	sop->event_fdsz = fdsz;
	sop->event_socketsz = socket_sz;

	return (0);

 error:
	DebugLogString( TRUE , "[select_resize] malloc failed!!" );
	return (-1);
}


static int	select_add(IMeSocketEventSelectListener* pSelectListener, IMeSocket* s, short old, short events)
{
	struct selectop *sop = pSelectListener->sop;
	int fd = CSocketGetFd(s);

    if( !(events&(SEvent_Read|SEvent_Write)) )
	{
		DebugLogString( TRUE , "[select_add] no support signal event!!" );
		return -1;
	}

	/*
	 * Keep track of the highest fd, so that we can calculate the size
	 * of the fd_sets for select(2)
	 */
	if( sop->event_fds < fd ) 
	{
		int fdsz = sop->event_fdsz;

		if( fdsz < (int)sizeof(fd_mask) )
			fdsz = (int)sizeof(fd_mask);

		/* In theory we should worry about overflow here.  In
		 * reality, though, the highest fd on a unixy system will
		 * not overflow here. XXXX */
		while( fdsz < (int)SELECT_ALLOC_SIZE(fd + 1) )
			fdsz *= 2;

		if( fdsz != sop->event_fdsz ) 
		{
			if( select_resize(sop, fdsz) ) 
			{
				DebugLogString( TRUE , "[select_add] select resize failed!!" );
				return (-1);
			}
		}

		sop->event_fds = fd;
	}

	if (events & SEvent_Read)
	{
		char* pArrayReadSocket = (char*)sop->event_readset_in + sop->event_fdsz;
		*(uint32_t*)&pArrayReadSocket[fd*sizeof(IMeSocket*)] = (uint32_t)s;
		FD_SET(fd, (fd_set*)sop->event_readset_in);
	}

	if (events & SEvent_Write)
	{
		char* pArrayWriteSocket = (char*)sop->event_writeset_in + sop->event_fdsz;
		*(uint32_t*)&pArrayWriteSocket[fd*sizeof(IMeSocket*)] = (uint32_t)s;
		FD_SET(fd, (fd_set*)sop->event_writeset_in);
	}

	return (0);
}

/*
 * Nothing to be done here.
 */

static int	select_del(IMeSocketEventSelectListener* pSelectListener, IMeSocket* s, short old, short events)
{
	struct selectop *sop = pSelectListener->sop;
	int fd = CSocketGetFd(s);

	if( !(events&(SEvent_Read|SEvent_Write)) )
	{
		DebugLogString( TRUE , "[select_del] no support event!!" );
		return -1;
	}

	if( sop->event_fds < fd ) 
	{
		DebugLogString( TRUE , "[select_del] error socket!!" );
		return (0);
	}

	if( events & SEvent_Read )
	{
		char* pArrayReadSocket = (char*)sop->event_readset_in + sop->event_fdsz;
		memset( &pArrayReadSocket[fd*sizeof(IMeSocket*)] , 0 , sizeof(IMeSocket*) );
		FD_CLR(fd, (fd_set*)sop->event_readset_in);	
	}

	if( events & SEvent_Write )
	{
		char* pArrayWriteSocket = (char*)sop->event_writeset_in + sop->event_fdsz;
		memset( &pArrayWriteSocket[fd*sizeof(IMeSocket*)] , 0 , sizeof(IMeSocket*) );
		FD_CLR(fd, (fd_set*)sop->event_writeset_in);
	}

	return (0);
}

static void	select_free_selectop(struct selectop *sop)
{
	if (sop->event_readset_in)
		free(sop->event_readset_in);
	if (sop->event_writeset_in)
		free(sop->event_writeset_in);
	if (sop->event_readset_out)
		free(sop->event_readset_out);
	if (sop->event_writeset_out)
		free(sop->event_writeset_out);

	memset(sop, 0, sizeof(struct selectop));
	free(sop);
}

static void	select_dealloc(IMeSocketEventSelectListener* pSelectListener)
{
	select_free_selectop(pSelectListener->sop);
}


void    IMeSocketEventSelectListenerDestroy( IMeSocketEventListener* pIMeSocketEventListener )
{
    IMeSocketEventSelectListener* pSelectListener = (IMeSocketEventSelectListener*)pIMeSocketEventListener;
    if( pSelectListener )
    {
        select_dealloc( pSelectListener );
        free(pSelectListener);
    }
}

int    IMeSocketEventSelectListenerAdd( IMeSocketEventListener* pIMeSocketEventListener , IMeSocket* s , short old , short events )
{
    IMeSocketEventSelectListener* pSelectListener = (IMeSocketEventSelectListener*)pIMeSocketEventListener;
    if( pSelectListener )
    {
        return select_add( pSelectListener , s , old , events );
    }

	return -1;
}

int    IMeSocketEventSelectListenerDel( IMeSocketEventListener* pIMeSocketEventListener , IMeSocket* s , short old , short events )
{
    IMeSocketEventSelectListener* pSelectListener = (IMeSocketEventSelectListener*)pIMeSocketEventListener;
    if( pSelectListener )
    {
        return select_del( pSelectListener , s , old , events );
    }

	return -1;
}

int     IMeSocketEventSelectListenerDispatch( IMeSocketEventListener* pIMeSocketEventListener , uint32_t timeMiliseconds )
{
    IMeSocketEventSelectListener* pSelectListener = (IMeSocketEventSelectListener*)pIMeSocketEventListener;
    if( pSelectListener )
    {
		struct timeval time;
		time.tv_sec = 0;
		time.tv_usec = timeMiliseconds*1000;
        return select_dispatch( pSelectListener , &time );
    }
	
    return -1;
}


void    IMeSocketEventSelectListenerRegister( IMeSocketEventListener* pIMeSocketEventListener , IMeSocketEventListenerOnNotify notify , void* upApp )
{
    IMeSocketEventSelectListener* pSelectListener = (IMeSocketEventSelectListener*)pIMeSocketEventListener;
    if( pSelectListener )
    {
		pSelectListener->evNotify = notify;
		pSelectListener->upApp = upApp;
    }
}

IME_EXTERN_C  IMeSocketEventListener*  CSocketEventListenerSelectCreate()
{
    IMeSocketEventSelectListener* pSelectListener = (IMeSocketEventSelectListener*)calloc(1,sizeof(IMeSocketEventSelectListener));
    
    if( pSelectListener )
    {
        pSelectListener->sop = select_init();
        if( !pSelectListener->sop )
        {
            DebugLogString( TRUE , "[CSocketEventListenerSelectCreate] invoke select_init failed!!" );
			free(pSelectListener);
			return NULL;
        }

        ((IMeSocketEventListener*)&pSelectListener->interfacefunction)->m_pAdd = IMeSocketEventSelectListenerAdd;
        ((IMeSocketEventListener*)&pSelectListener->interfacefunction)->m_pDel = IMeSocketEventSelectListenerDel;
        ((IMeSocketEventListener*)&pSelectListener->interfacefunction)->m_pDispatch = IMeSocketEventSelectListenerDispatch;
        ((IMeSocketEventListener*)&pSelectListener->interfacefunction)->m_pDestroy = IMeSocketEventSelectListenerDestroy;
        ((IMeSocketEventListener*)&pSelectListener->interfacefunction)->m_pRegister = IMeSocketEventSelectListenerRegister;
    }
    
    return (IMeSocketEventListener*)pSelectListener;
}

#endif

