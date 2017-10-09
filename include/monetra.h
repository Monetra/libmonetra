/* This is the libmonetra legacy header.  It should not be included by modern 
 * programs.  Use "monetra_api.h" instead */
#ifndef __MONETRA_H__
#define __MONETRA_H__

/* Detect if we're using mstdlib.  If we are, it must be included before this file.
 * Otherwise, the libmonetra API needs a typedef that would be provided by mstdlib */
#ifndef __MSTDLIB_H__

#include <stddef.h>

#  ifdef _WIN32
typedef  __int64              M_int64;
#    ifdef _WIN64
typedef  unsigned __int64     M_uintptr;
#    else /* ! _WIN64 */
typedef  unsigned long        M_uintptr;
#    endif
#  else /* ! _WIN32 */
typedef  long long            M_int64;
typedef  unsigned long        M_uintptr;
#  endif

#endif /* __MSTDLIB_H__ */

#define LIBMONETRA_VERSION 0x080000
#include <libmonetra_legacy.h>
#include <libmonetra_deprecated.h>

#endif
