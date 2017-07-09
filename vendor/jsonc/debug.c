/*
 * $Id: debug.c,v 1.5 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#if HAVE_SYSLOG_H
# include <syslog.h>
#endif /* HAVE_SYSLOG_H */

#if HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

#include "debug.h"

static int _jsoncsyslog = 0;

#ifdef MC_MAINTAINER_MODE
static int _jsoncdebug = 0;
#endif

void MC_ERROR(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
#if HAVE_VSYSLOG
    if(_jsoncsyslog) {
		vsyslog(LOG_ERR, msg, ap);
	} else
#endif
		vfprintf(stderr, msg, ap);
  va_end(ap);
}


#ifndef MC_MAINTAINER_MODE

extern void MC_SET_DEBUG(int debug){};
extern int MC_GET_DEBUG(void){ return 0;};

extern void MC_SET_SYSLOG(int syslog){};

extern void MC_DEBUG(const char *msg, ...){};
extern void MC_INFO(const char *msg, ...){};

#else

void MC_SET_DEBUG(int debug) { _jsoncdebug = debug; }
int MC_GET_DEBUG(void) { return _jsoncdebug; }

extern void MC_SET_SYSLOG(int syslog)
{
  _jsoncsyslog = syslog;
}

void MC_DEBUG(const char *msg, ...)
{
  va_list ap;
  if(_jsoncdebug) {
    va_start(ap, msg);
#if HAVE_VSYSLOG
    if(_jsoncsyslog) {
		vsyslog(LOG_DEBUG, msg, ap);
	} else
#endif
		vprintf(msg, ap);
    va_end(ap);
  }
}

void MC_INFO(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
#if HAVE_VSYSLOG
    if(_jsoncsyslog) {
		vsyslog(LOG_INFO, msg, ap);
	} else 
#endif
		vfprintf(stderr, msg, ap);
  va_end(ap);
}

#endif
