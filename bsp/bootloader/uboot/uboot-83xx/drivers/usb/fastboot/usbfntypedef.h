#ifndef X_TYPEDEF_H
#define X_TYPEDEF_H

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

#ifndef VOID
#define VOID void
#endif

#if !defined (_NO_TYPEDEF_BYTE_) && !defined (_TYPEDEF_BYTE_)
typedef unsigned char  BYTE;
#define _TYPEDEF_BYTE_
#endif

#if !defined (_NO_TYPEDEF_UCHAR_) && !defined (_TYPEDEF_UCHAR_)
typedef unsigned char  UCHAR;
#define _TYPEDEF_UCHAR_
#endif

#if !defined (_NO_TYPEDEF_UINT8_) && !defined (_TYPEDEF_UINT8_)
typedef unsigned char  UINT8;
#define _TYPEDEF_UINT8_
#endif

#if !defined (_NO_TYPEDEF_UINT16_) && !defined (_TYPEDEF_UINT16_)
typedef unsigned short  UINT16;
#define _TYPEDEF_UINT16_
#endif

#if !defined (_NO_TYPEDEF_UINT32_) && !defined (_TYPEDEF_UINT32_)
typedef unsigned long  UINT32;
#define _TYPEDEF_UINT32_
#endif

#if !defined (_NO_TYPEDEF_UINT64_) && !defined (_TYPEDEF_UINT64_)
typedef unsigned long long  UINT64;
#define _TYPEDEF_UINT64_
#endif

#if !defined (_NO_TYPEDEF_CHAR_) && !defined (_TYPEDEF_CHAR_)
typedef char  CHAR;     // Debug, should be 'signed char'
#define _TYPEDEF_CHAR_
#endif

#if !defined (_NO_TYPEDEF_INT8_) && !defined (_TYPEDEF_INT8_)
typedef signed char  INT8;
#define _TYPEDEF_INT8_
#endif

#if !defined (_NO_TYPEDEF_INT16_) && !defined (_TYPEDEF_INT16_)
typedef signed short  INT16;
#define _TYPEDEF_INT16_
#endif

#if !defined (_NO_TYPEDEF_INT32_) && !defined (_TYPEDEF_INT32_)
typedef signed long  INT32;
#define _TYPEDEF_INT32_
#endif

#if !defined (_NO_TYPEDEF_INT64_) && !defined (_TYPEDEF_INT64_)
typedef signed long long  INT64;
#define _TYPEDEF_INT64_
#endif


typedef UINT32  BOOL;


/*
#if !defined (_NO_TYPEDEF_FLOAT_) && !defined (_TYPEDEF_FLOAT_)
typedef float  FLOAT;
#define _TYPEDEF_FLOAT_
#endif

#if !defined (_NO_TYPEDEF_DOUBLE_)  && !defined (_TYPEDEF_DOUBLE_)
typedef double  DOUBLE;
#define _TYPEDEF_DOUBLE_
#endif
*/

#ifndef UNUSED
#define UNUSED(x)               (void)x
#endif

#ifndef TRUE
    #define TRUE                (0 == 0)
#endif  // TRUE

#ifndef FALSE
    #define FALSE               (0 != 0)
#endif  // FALSE

#ifndef externC
    #ifdef __cplusplus
        #define externC         extern "C"
    #else
        #define externC         extern
    #endif
#endif  // externC


#ifndef EXTERN
    #ifdef __cplusplus
        #define EXTERN          extern "C"
    #else
        #define EXTERN          extern
    #endif
#endif  // EXTERN
typedef signed char    int8;
typedef signed short   int16;
typedef signed long    int32;
typedef signed int     intx;
typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;
typedef unsigned char  bool;

typedef volatile unsigned char *     P_U8;
typedef volatile signed char *       P_S8;
typedef volatile unsigned short *    P_U16;
typedef volatile signed short *      P_S16;
typedef volatile unsigned int *      P_U32;
typedef volatile signed int *        P_S32;
typedef unsigned long long *         P_U64;
typedef signed long long *           P_S64;


typedef volatile unsigned char      U8;
typedef volatile signed char       S8;
typedef volatile unsigned short     U16;
typedef volatile signed short       S16;
typedef volatile unsigned int       U32;
typedef volatile signed int         S32;
typedef unsigned long long          U64;
typedef signed long long          S64;


typedef unsigned int  DWORD;

typedef unsigned int *  LPDWORD;
typedef unsigned int *  PDWORD;

typedef char *  LPCSTR;
typedef void *  LPVOID;
typedef void *  PVOID;

typedef unsigned int        UINT;
typedef unsigned short      WORD;


#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

typedef unsigned long ULONG;
typedef ULONG *PULONG;
typedef unsigned short USHORT;
typedef USHORT *PUSHORT;

typedef UCHAR *PUCHAR;
typedef char *PSZ;

typedef unsigned char*  PBYTE;
typedef unsigned short TCHAR;
typedef BOOL*  PBOOL;


#define ERROR_SUCCESS 0
#define ERROR_GEN_FAILURE  1

#define HIBYTE(w)  ((BYTE)((w)>>8))


#define LOBYTE(w) ((BYTE)((w)&0x00FF))

#define  min(a,b)  ((a>b)?b:a)

#endif // X_TYPEDEF_H
