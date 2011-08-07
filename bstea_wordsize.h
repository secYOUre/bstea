#ifndef __BSTEA_WORDSIZE_H
#define __BSTEA_WORDSIZE_H

/* Determine the wordsize from the preprocessor defines.  */

#if defined __x86_64__ || defined __amd64__ || defined __x86_64 || \
    defined __amd64    || defined _M_X64    || defined __ia64__ || \
    defined __ia64__   || defined __IA64__  || defined __ia64   || \
    defined _M_IA64
# define __BSTEA_WORDSIZE     64
#else
# define __BSTEA_WORDSIZE     32
#endif


#endif /* __BSTEA_WORDSIZE_H */
