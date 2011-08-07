#ifndef __BSTEA_H
#define __BSTEA_H

#include <stdint.h>
#include <limits.h>

#include "bstea_wordsize.h"

#define TEA_ROUNDS      32

#define TEA_BLOCK_SIZE  64
#define TEA_KEY_SIZE    128

#if __BSTEA_WORDSIZE == 64
typedef uint64_t vector_width_t;  /* 64-way bit-level vectorization */
#define VECTOR_AT_ONE	0xffffffffffffffff
#define VECTOR_AT_ZERO	0x0000000000000000
#elif __BSTEA_WORDSIZE == 32
typedef uint32_t vector_width_t;  /* 32-way bit-level vectorization */
#define VECTOR_AT_ONE	0xffffffff
#define VECTOR_AT_ZERO	0x00000000
#elif __BSTEA_WORDSIZE == 16
typedef uint32_t vector_width_t;  /* 16-way bit-level vectorization */
#define VECTOR_AT_ONE	0xffff
#define VECTOR_AT_ZERO	0x0000
#elif __BSTEA_WORDSIZE == 8
typedef uint32_t vector_width_t;  /* 8-way bit-level vectorization */
#define VECTOR_AT_ONE	0xff
#define VECTOR_AT_ZERO	0x00
#else 
typedef unsigned long int vector_width_t;  /* word-way bit-level vectorization */
#define VECTOR_AT_ONE	ULONG_MAX
#define VECTOR_AT_ZERO	0
#endif


typedef vector_width_t parallel_blocks_t[TEA_BLOCK_SIZE];
typedef vector_width_t parallel_keys_t[TEA_KEY_SIZE];


/* __P is a macro used to wrap function prototypes, so that compilers
   that don't understand ANSI C prototypes still work, and ANSI C
   compilers can issue warnings about type mismatches. */
#undef __P
#if defined (__STDC__) || defined (_AIX) || (defined (__mips) && defined (_SYSTYPE_SVR4)) || defined(WIN32) || defined(__cplusplus)
 # define __P(protos)    protos
#else
 # define __P(protos)    ()
#endif


/* __BEGIN_DECLS should be used at the beginning of your declarations,
   so that C++ compilers don't mangle their names.  Use __END_DECLS at
   the end of C declarations. */
#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC. If you don't need the mixed model,
 * just define FAR to be empty.
 */
#ifdef SYS16BIT
#  if defined(M_I86SM) || defined(M_I86MM)
     /* MSC small or medium model */
#    define SMALL_MEDIUM
#    ifdef _MSC_VER
#      define FAR _far
#    else
#      define FAR far
#    endif
#  endif
#  if (defined(__SMALL__) || defined(__MEDIUM__))
     /* Turbo C small or medium model */
#    define SMALL_MEDIUM
#    ifdef __BORLANDC__
#      define FAR _far
#    else
#      define FAR far
#    endif
#  endif
#endif


#if defined(WINDOWS) || defined(WIN32)
#  ifdef BSTEA_DLL
#    if defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#      ifdef BSTEA_INTERNAL
#        define BSTEA_EXTERN extern __declspec(dllexport)
#      else
#        define BSTEA_EXTERN extern __declspec(dllimport)
#      endif
#    endif
#  endif  /* BSTEA_DLL */

   /* If building or using bstea with the WINAPI/WINAPIV calling convention,
    * define BSTEA_WINAPI.
    * Caution: the standard BSTEA.DLL is NOT compiled using BSTEA_WINAPI.
    */
#  ifdef BSTEA_WINAPI
#    ifdef FAR
#      undef FAR
#    endif
#    include <windows.h>
     /* No need for _export, use BSTEA_LIB.DEF instead. */
     /* For complete Windows compatibility, use WINAPI, not __stdcall. */
#    define BSTEA_EXPORT WINAPI
#    ifdef WIN32
#      define BSTEA_EXPORTVA WINAPIV
#    else
#      define BSTEA_EXPORTVA FAR CDECL
#    endif
#  endif

#else
# include <stdbool.h>
#endif

#ifndef BSTEA_EXTERN
#  define BSTEA_EXTERN extern
#endif
#ifndef BSTEA_EXPORT
#  define BSTEA_EXPORT
#endif
#ifndef BSTEA_EXPORTVA
#  define BSTEA_EXPORTVA
#endif

#ifndef FAR
# define FAR
#endif

__BEGIN_DECLS

BSTEA_EXTERN  void encrypt __P((parallel_blocks_t, const parallel_keys_t, unsigned int));
BSTEA_EXTERN  void decrypt __P((parallel_blocks_t, const parallel_keys_t, unsigned int));

__END_DECLS

#endif /* __BSTEA_H */
