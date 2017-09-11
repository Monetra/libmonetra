#ifndef __LIBMONETRA_EXP_H__
#define __LIBMONETRA_EXP_H__

#ifdef _WIN32
#  ifdef LIBMONETRA_STATIC
#    define LM_EXPORT
#  else
#    ifdef LIBMONETRA_INTERNAL
#      define LM_EXPORT __declspec(dllexport)
#    else
#      define LM_EXPORT __declspec(dllimport)
#    endif
#  endif
#  ifdef LIBMONETRA_SPEC_STDCALL
#    define LM_SPEC __stdcall
#  else
#    define LM_SPEC __cdecl
#  endif
#else /* !WIN32 */
#  if __GNUC__ >= 4
#    ifdef LIBMONETRA_INTERNAL
#      define LM_EXPORT __attribute__ ((visibility ("default")))
#    else
#      define LM_EXPORT
#    endif
#  else
#    define LM_EXPORT
#  endif
#  define LM_SPEC
#endif /* !WIN32 */

#endif
