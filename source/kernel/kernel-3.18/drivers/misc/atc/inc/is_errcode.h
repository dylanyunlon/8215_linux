/**
 Err Code define for Input Source (IS)
*/


#ifndef _IS_ERRCODE_H_
#define _IS_ERRCODE_H_

#include <windows.h>

/*************************************************************************
 * 
 *                    The size of error_code is 32 bits
 * Error code structure :
 *
 * | 31(1 bit) |30-24(7 bits) |23-16(8 bits) | 15-0(16 bits) |
 * | Indicator |   Reserved   |  Module ID   |      Code     |
 *
 * Indicator  : 1 - ERROR_CODE; 0 - SUCESS_CODE(RET_IS_OK:0X00000000)
 * Reserved   : Reserved bits
 * Module ID  : module ID, defined below
 * Code       : the module's status code

 *************************************************************************/
/* Composer for Module specific error code */
#ifndef _SRESULT_
#define _SRESULT_
typedef UINT32 SRESULT;
#endif

static const char *isErrCodeStr[] = 
{
    "RET_IS_FAIL",                      // 0
    "RET_IS_OUTOFMEMORY",               // 1
    "RET_IS_INVALID_PARAMETER",         // 2
    "RET_IS_BITRATE_NOT_SUPPORT",       // 3
    "RET_IS_SAMPLERATE_NOT_SUPPORT",    // 4
    "RET_IS_RESOLUTION_NOT_SUPPORT"    // 5
}; 


#define COMMON_ERR_CNT  (sizeof(isErrCodeStr)/sizeof(isErrCodeStr[0]))

#define IS_RETURN_LOG_OPEN  1

#define STATEC_ERR          0

#define INTERN_ERR          1

#define EXTERN_ERR          1

#define IS_IS_STATE_ERROR(errcode)  (0x00 == ((errcode >> 24) & 0xFF))


#if IS_RETURN_LOG_OPEN
#define IS_RETURN(err)      IS_RETURN_I((err), __FILE__, __LINE__)
#define IS_RETURN_I(err, file, line)                                                            \
    do                                                                                          \
    {                                                                                           \
        UINT32 u4Err = (err);                                                                   \
        if ((u4Err) != RET_IS_OK)                                                            \
        {                                                                                       \
            if (IS_IS_STATE_ERROR(u4Err))                                                       \
            {                                                                                   \
                RETAILMSG(STATEC_ERR, (TEXT("[0x%x] is returned at line %d in file %s\r\n"),    \
                                (unsigned int)(u4Err), line, file));                            \
            }                                                                                   \
            else if (MOD_ERRCODE_COMMON != ((u4Err & 0x00FFFFFF) >> 16))                        \
            {                                                                                   \
                RETAILMSG(INTERN_ERR, (TEXT("[0x%x] is returned at line %d in file %s\r\n"),    \
                                (unsigned int)(u4Err), line, file));                            \
            }                                                                                   \
            else                                                                                \
            {                                                                                   \
                UINT32 u4ErrIdx = (u4Err & 0x000000FF);                                         \
                if (u4ErrIdx < COMMON_ERR_CNT)                                                  \
                {                                                                               \
                    RETAILMSG(EXTERN_ERR, (TEXT("[%s] is returned at line %d in file %s\r\n"),  \
                                      isErrCodeStr[u4ErrIdx], line, file));                     \
                }                                                                               \
                else                                                                            \
                {                                                                               \
                    RETAILMSG(INTERN_ERR, (TEXT("[0x%x] is incorrectly defined, returned at line %d in file %s\r\n"),    \
                                    (unsigned int)(u4Err), line, file));                        \
                }                                                                               \
            }                                                                                   \
        }                                                                                       \
        return (u4Err);                                                                         \
    }while(0);
#else
#define IS_RETURN(err)    return (err)
#endif


#define MAKE_ERR_CODE(mod, code)       \
    ((UINT32)                         \
     ((UINT32)(0x80000000) |          \
     (UINT32)(((mod) & 0xFF) << 16) | \
     (UINT32)((code) & 0xffff))       \
    )

#define MAKE_STATE_CODE(mod, code)    \
    ((UINT32)                         \
     ((UINT32)(0x00000000) |          \
     (UINT32)(((mod) & 0xFF) << 16) | \
     (UINT32)((code) & 0xffff))       \
    )

////////////////---MODULE_ID----//////////////////////////

// ISORE :  The value range is 0x80010000 ----0x8001FFFFF
#define MOD_ERRCODE_COMMON				0x01L  

// TVD : The value range is 0x80020000 ----0x8002FFFFF
#define MOD_ERRCODE_TVD 				0x02L

// VGA/YPbPr :   The value range is 0x80030000 ----0x8003FFFFF
#define MOD_ERRCODE_VGA					0X03L

// HDMI/MHL:       The value range is 0x80040000 ----0x8004FFFFF
#define MOD_ERRCODE_MHL					0X04L


/*************************************************************************
*
*                  The specific value of msdkcore error code 
*
*          The start value of error_code is 0x80010000(RET_IS_FAIL)
*                                                                                                                            
*************************************************************************/

// Success, or OK
#define RET_IS_OK                              	0X00000000

// Fail, or error, but don't know the real reason
#define RET_IS_FAIL                             MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 0)

// Out of memory, or alloc/realloc fail
#define RET_IS_OUTOFMEMORY                      MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 1)

// Invalid Parameter in function
#define RET_IS_INVALID_PARAMETER                MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 2)

// Bit-rate not support
#define RET_IS_BITRATE_NOT_SUPPORT              MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 3)

// Sample-rate not support
#define RET_IS_SAMPLERATE_NOT_SUPPORT           MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 4)

// Resolution not support
#define RET_IS_RESOLUTION_NOT_SUPPORT           MAKE_ERR_CODE(MOD_ERRCODE_COMMON, 5)



/*************************************************************************
*
*                  The specific value of msdkcore state code 
*
*          The start value of state_code is 0x00010000(RET_IS_STATE_NO_SIGNAL)
*                                                                                                                            
*************************************************************************/

#define RET_IS_STATE_NO_SIGNAL					MAKE_STATE_CODE(MOD_ERRCODE_COMMON, 0)

#define RET_IS_STATE_MODE_CHANGE				MAKE_STATE_CODE(MOD_ERRCODE_COMMON, 1)


#endif

