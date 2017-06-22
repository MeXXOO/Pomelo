#ifndef __math_compat_h
#define __math_compat_h

/* Define isnan and isinf on Windows/MSVC */

#ifndef HAVE_DECL_ISNAN
# ifdef HAVE_DECL__ISNAN
#include <float.h>
#define isnan(x) _isnan(x)
# endif
#endif

#ifndef HAVE_DECL_ISINF
# ifdef HAVE_DECL__FINITE
#include <float.h>
#define isinf(x) (!_finite(x))
# endif
#endif

#ifndef HAVE_DECL_NAN
#if	(defined(WIN32) && _MSC_VER<=1700)	//just for VC++ compile no support c99 standard
#define nan(a)	(0.0/0.0000000000000000001)
#else
#error This platform does not have nan()
#endif
#endif

#ifndef HAVE_DECL_INFINITY
#if	(defined(WIN32) && _MSC_VER<=1700)	//just for VC++ compile no support c99 standard
#define		INFINITY		(1.0/0.0000000000000000001)
#else
#error This platform does not have INFINITY
#endif
#endif

#endif
