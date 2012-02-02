/*! \file
\brief Provides portable stdint.h for different compilers on different platforms.

*/

#ifdef _MSC_VER

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;

# ifndef UINT64_C
#  define UINT64_C(v) v ## UI64
# endif
# ifndef INT64_C
#  define  INT64_C(v) v ## I64
# endif
# ifndef PRINTF_INT64_MODIFIER
#  define PRINTF_INT64_MODIFIER "I64"
# endif

#define DISABLEUNUSEDWARNING

#else

# include <stdint.h>

# ifndef UINT64_C
#  define UINT64_C(v) v ## ULL
# endif
# ifndef INT64_C
#  define  INT64_C(v) v ## LL
# endif
# ifndef PRINTF_INT64_MODIFIER
#  define PRINTF_INT64_MODIFIER "ll"
# endif

#define DISABLEUNUSEDWARNING __attribute__(unused)

#endif
