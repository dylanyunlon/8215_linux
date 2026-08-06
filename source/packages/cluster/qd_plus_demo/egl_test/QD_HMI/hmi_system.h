/*****************************************************************************

File Name        :  hmi_system.h
Organization    :  Zhuli Electronics Co.Ltd in Shanghai (www.shzldz.com)
******************************************************************************/


#ifndef _HMI_SYSTEM_H
#define _HMI_SYSTEM_H

/******* function modual define********/
//#define HMI_TOUCH_PANEL
/*#define HMI_8_DIRECT*/
/*#define HMI_MEMORY_MANAGER*/
/*#define HMI_ACTION_MANAGER*/
/*#define HMI_SCREEN_COORDINATE */ /*no define is touch AD value*/

/*#define HMI_SOFT_TIMER*/
#define HMI_MAX_MIN_VALUE


/*#define HMI_TOUCHPANEL_DRIVER*/
/*#define HMI_RESOURCE_MANAGER_DRIVER*/
/*#define HMI_MEMORY_MANAGER*/
/*#define HMI_MEMORY_ALLOC_FREE*/

/*******OS/MCU define *************/
/*#define	WINDOWS_OS
//#define	WIN32_OS
#define		NO_OS */
//#define	QD_PlUS_RELEASE_		HMI_YES



#if defined( HMI_MCU_RH850_D1LX )    
	typedef unsigned char   UINT8;
	typedef   signed char   SINT8;
	typedef unsigned short  UINT16;
	typedef   signed short  SINT16;
	typedef unsigned int    UINT32_T;
	typedef UINT32_T    	UINT32;
	typedef   signed int    SINT32;
	typedef SINT32			INT32;
	typedef unsigned char   BOOLEAN;
	typedef UINT8			BYTE;
	typedef UINT8			U08;
	typedef UINT8			U8;
	typedef UINT16			U16;
	typedef UINT32_T		U32;
	typedef  float         	float_32;
	typedef  double			double_64;
	typedef float_32		HMI_TIME;
	typedef 	SINT16		INT16;
	typedef unsigned long  int  DWORD;
    #ifndef _WIN32
    #define NEAR
    #define FAR
	#define REENTRANT
	#else	
	#define REENTRANT
	#endif
    #define CONST			const		
#elif defined( HMI_MCU_RH850_D1MX )
	#ifndef __HMI_D1Mx__
	#define  __HMI_D1Mx__
	#endif
	typedef unsigned char   UINT8;
	typedef   signed char   SINT8;
	typedef unsigned short  UINT16;
	typedef   signed short  SINT16;
	typedef unsigned int    UINT32_T;
	#ifndef UINT32
	typedef UINT32_T    	UINT32;
	#endif
	#ifndef SINT32
	typedef   signed int    SINT32;
	#endif
	#ifndef INT32
	typedef SINT32			INT32;
	#endif
	typedef unsigned char   BOOLEAN;

	typedef UINT8			U08;
	typedef UINT8			U8;
	typedef UINT16			U16;
	typedef UINT32_T		U32;
	typedef  float         	float_32;
	typedef  double			double_64;
	typedef float_32		HMI_TIME;
	typedef UINT8			BYTE;
	typedef 	SINT16		INT16;
	typedef unsigned long  int  DWORD;
    #ifndef _WIN32
    #define NEAR
    #define FAR
	#endif
	#ifndef _WIN32
    #define NEAR
    #define FAR
	#define REENTRANT
	#else	
	#define REENTRANT
	#endif
	#define CONST			const
#elif defined( HMI_MCU_RH850_D1HX )
	typedef unsigned char	UINT8;
	typedef   signed char	SINT8;
	typedef unsigned short	UINT16;
	typedef   signed short	SINT16;
	typedef unsigned int	UINT32_T;
	typedef UINT32_T    	UINT32;
	typedef   signed int	SINT32;
	typedef SINT32			INT32;
	typedef unsigned char	BOOLEAN;
	typedef UINT8			BYTE;
	typedef UINT8			U08;
	typedef UINT8			U8;
	typedef UINT16			U16;
	typedef UINT32_T		U32;
	typedef  float			float_32;
	typedef  double 		double_64;
	typedef float_32		HMI_TIME;
	typedef 	SINT16		INT16;
	typedef unsigned long  int  DWORD;
	#ifndef _WIN32
    #define NEAR
    #define FAR
	#define REENTRANT
	#else	
	#define REENTRANT
	#endif
	#define CONST			const
#elif defined( HMI_MCU_S6J3200 )
	typedef unsigned char	UINT8;
	typedef   signed char	SINT8;
	typedef unsigned short	UINT16;
	typedef   signed short	SINT16;
	typedef unsigned int	UINT32_T;
	typedef UINT32_T		UINT32;
	typedef   signed int	SINT32;
	typedef SINT32			INT32;
	typedef unsigned char	BOOLEAN;
	typedef UINT8			BYTE;
	typedef UINT8			U08;
	typedef UINT8			U8;
	typedef UINT16			U16;
	typedef UINT32_T		U32;
	typedef  float			float_32;
	typedef  double 		double_64;
	typedef float_32		HMI_TIME;
	typedef 	SINT16		INT16;
	typedef unsigned long  int  DWORD;
	#ifndef _WIN32
    #define NEAR
    #define FAR
	#define REENTRANT
	#else	
	#define REENTRANT
	#endif
	#define CONST			const
#elif defined( HMI_MCU_RT1170 )|| defined(HMI_MCU_RT1172 )	
	typedef   signed char	SINT8;	
	typedef   signed short	SINT16;
	typedef unsigned int	UINT32_T;
	typedef UINT32_T		UINT32;
	typedef   signed int	SINT32;
#ifndef FF_DEFINED	/*JPG decompress already define INT32,BYTE,DWORD*/
	typedef unsigned char	UINT8;
	typedef unsigned short	UINT16;
	typedef SINT32			INT32;
	typedef UINT8			BYTE;
	typedef unsigned long  int	DWORD;
	typedef 	SINT16		INT16;
#endif
	typedef unsigned char	BOOLEAN;		
	typedef UINT8			U08;
	typedef UINT8			U8;
	typedef UINT16			U16;
	#ifndef _IODEF_AUTOSAR_TYPEDEF_//ofim
	typedef UINT32_T		U32;
	#endif
	typedef  float			float_32;
	typedef  double 		double_64;
	typedef float_32		HMI_TIME;	
	
#ifndef _WIN32
    #define NEAR
    #define FAR
	#define REENTRANT
#else	
	#define REENTRANT
#endif
	#define CONST			const
#elif defined( HMI_MCU_AMT630H )
			typedef   signed char	SINT8;	
			typedef   signed short	SINT16;
			typedef unsigned int	UINT32_T;
			typedef UINT32_T		UINT32;
			typedef   signed int	SINT32;
			typedef unsigned char	UINT8;
			typedef unsigned short	UINT16;
			typedef SINT32			INT32;
			typedef UINT8			BYTE;
			typedef unsigned long  int	DWORD;
			typedef 	SINT16		INT16;

			typedef unsigned char	BOOLEAN;		
			typedef UINT8			U08;
			typedef UINT8			U8;
			typedef UINT16			U16;	
			typedef UINT32_T		U32;	
			typedef  float			float_32;
			typedef  double 		double_64;
			typedef float_32		HMI_TIME;	
			
#ifndef _WIN32
    #define NEAR
    #define FAR
	#define REENTRANT
#else	
	#define REENTRANT
#endif
	#define CONST			const

#elif defined( HMI_MCU_X86 )
		typedef unsigned char	UINT8;
		typedef   signed char	SINT8;
		typedef unsigned short	UINT16;
		typedef   signed short	SINT16;
		typedef unsigned int	UINT32_T;
		typedef UINT32_T		UINT32;
		typedef   signed int	SINT32;
		typedef SINT32			INT32;
		typedef unsigned char	BOOLEAN;
		typedef UINT8			BYTE;
		typedef UINT8			U08;
		typedef UINT8			U8;
		typedef UINT16			U16;
		typedef UINT32_T		U32;
		typedef  float			float_32;
		typedef  double 		double_64;
		typedef float_32		HMI_TIME;
		typedef 	SINT16		INT16;
		typedef unsigned long  int  DWORD;
	#ifndef HMI_WINDOWS
    #define NEAR
    #define FAR
	#define REENTRANT
	#else	
	#define REENTRANT
	#endif
	#define CONST			const
#elif defined( HMI_MCU_IMX6 )
			typedef unsigned char	UINT8;
			typedef   signed char	SINT8;
			typedef unsigned short	UINT16;
			typedef   signed short	SINT16;
			typedef unsigned int	UINT32_T;
			typedef UINT32_T		UINT32;
			typedef   signed int	SINT32;
			typedef SINT32			INT32;
			typedef unsigned char	BOOLEAN;
			typedef UINT8			BYTE;
			typedef UINT8			U08;
			typedef UINT8			U8;
			typedef UINT16			U16;
			typedef UINT32_T		U32;
			typedef  float			float_32;
			typedef  double 		double_64;
			typedef float_32		HMI_TIME;
			//typedef 	UINT32		UINT;
			typedef 	SINT16		INT16;
			typedef unsigned long  int  DWORD;
	#ifndef HMI_WINDOWS
    #define NEAR
    #define FAR
	#define REENTRANT
	#else	
	#define REENTRANT
	#endif
	#define CONST			const
#elif defined( HMI_MCU_R_CAR )
	typedef unsigned char	UINT8;
	typedef   signed char	SINT8;
	typedef unsigned short	UINT16;
	typedef   signed short	SINT16;
	typedef unsigned int	UINT32_T;
	typedef UINT32_T		UINT32;
	typedef   signed int	SINT32;
	typedef SINT32			INT32;
	typedef unsigned char	BOOLEAN;
	typedef UINT8			BYTE;
	typedef UINT8			U08;
	typedef UINT8			U8;
	typedef UINT16			U16;
	typedef UINT32_T		U32;
	typedef  float			float_32;
	typedef  double 		double_64;
	typedef float_32		HMI_TIME;
	//typedef	UINT32		UINT;
	typedef 	SINT16		INT16;
	typedef unsigned long  int	DWORD;
	#ifndef HMI_WINDOWS
    #define NEAR
    #define FAR
	#define REENTRANT
	#else	
	#define REENTRANT
	#endif
	#define CONST			const

#elif defined( HMI_MCU_SH726X )
		typedef unsigned char	UINT8;
		typedef   signed char	SINT8;
		typedef unsigned short	UINT16;
		typedef   signed short	SINT16;
		typedef unsigned int	UINT32_T;
		typedef UINT32_T		UINT32;
		typedef   signed int	SINT32;
		typedef SINT32			INT32;
		typedef unsigned char	BOOLEAN;
	
		typedef UINT8			U08;
		typedef UINT8			U8;
		typedef UINT16			U16;
		typedef UINT32_T		U32;
		typedef  float			float_32;
		typedef  double 		double_64;
		typedef float_32		HMI_TIME;
		typedef UINT8			BYTE;
		typedef 	SINT16		INT16;
		typedef unsigned long  int  DWORD;
		#ifndef _WIN32
	    #define NEAR
	    #define FAR
		#define REENTRANT
		#else	
		#define REENTRANT
		#endif
		#define CONST			const	
#elif defined( HMI_MCU_RL78_D1A )
		typedef unsigned char	UINT8;
		typedef   signed char	SINT8;
		typedef unsigned short	UINT16;
		typedef   signed short	SINT16;
		typedef unsigned long	UINT32_T;
		typedef UINT32_T		UINT32;
		typedef   signed long	SINT32;
		typedef SINT32			INT32;
		typedef unsigned char	BOOLEAN;
	
		typedef UINT8			U08;
		typedef UINT8			U8;
		typedef UINT16			U16;
		typedef UINT32_T		U32;
		typedef  float			float_32;
		typedef  double 		double_64;
		typedef float_32		HMI_TIME;
		typedef UINT8			BYTE;
		typedef 	SINT16		INT16;
		typedef unsigned long  int  DWORD;
		#ifndef _WIN32
	    #define NEAR
	    #define FAR
		#define REENTRANT
		#else	
		#define REENTRANT
		#endif
		#define CONST			const
		
#elif (defined(HMI_MCU_ANY) )
	typedef unsigned char   UINT8;
    typedef   signed char   SINT8;
    typedef unsigned short  UINT16;
    typedef   signed short  SINT16;
    typedef unsigned long    UINT32_T;
	#ifndef UINT32
	//typedef unsigned long    	UINT32;
	#endif
	#ifndef SINT32
    typedef   signed int    	SINT32;
	#endif
	#ifndef INT32
	typedef signed int 			INT32;
	#endif
    typedef unsigned char   	BOOLEAN;
	#ifndef U08 
	typedef unsigned char		U08;
	#endif
	typedef unsigned char		U8;
	#ifndef U16
	typedef unsigned short		U16;
	#endif
	#ifndef U32
	typedef unsigned long		U32;
	#endif
	typedef float         		float_32;
	typedef double				double_64;
	typedef float				HMI_TIME;
	typedef unsigned char		BYTE;
	typedef 	SINT16		INT16;
	typedef unsigned long  int  DWORD;
	
  	#ifndef _WIN32
    //#define NEAR
    //#define FAR
	#define REENTRANT
	#else	
	#define REENTRANT
	#endif
	#define CONST			const

#elif (defined(HMI_MCU_T3) )
		typedef unsigned char	UINT8;
		typedef   signed char	SINT8;
		typedef unsigned short	UINT16;
		typedef   signed short	SINT16;
		typedef unsigned long	UINT32_T;
		typedef unsigned long  int  DWORD;
	#ifndef UINT32
		//typedef unsigned long 	UINT32;
	#endif
	#ifndef SINT32
		typedef   signed int		SINT32;
	#endif
	#ifndef INT32
		typedef signed int			INT32;
	#endif
		typedef unsigned char		BOOLEAN;
	#ifndef U08 
		typedef unsigned char		U08;
	#endif
		typedef unsigned char		U8;
	#ifndef U16
		typedef unsigned short		U16;
	#endif
	#ifndef U32
		typedef unsigned long		U32;
	#endif
		typedef float				float_32;
		typedef double				double_64;
		typedef float				HMI_TIME;
		typedef unsigned char		BYTE;
		typedef 	SINT16			INT16;
		typedef unsigned long  int  DWORD;
  	#ifndef _WIN32
		//#define NEAR
		//#define FAR
	#define REENTRANT
	#else	
	#define REENTRANT
	#endif
	#define CONST			const


#elif (defined( HMI_MCU_TW36 )||defined( HMI_MCU_TW25))
	
	typedef	unsigned char		BYTE;	
	typedef unsigned short int  WORD;
	typedef unsigned long  int  DWORD;
	

	typedef	signed short int		SWORD;
	typedef	signed long  int		SDWORD;
	
	typedef unsigned char			UINT8;
	typedef signed char				SINT8;
	typedef unsigned short int			UINT16;
	typedef	signed short int			SINT16;
	
	typedef unsigned long  int			UINT32_T;
	typedef unsigned long  int			UINT32;
	typedef	signed long  int			SINT32;
	typedef SINT32						INT32;
	typedef unsigned char				BOOLEAN;

	typedef UINT8			U08;
	typedef UINT8			U8;
	typedef UINT16			U16;
	typedef UINT32_T		U32;
	typedef  float			float_32;
	typedef  double 		double_64;
	typedef float_32		HMI_TIME;
	typedef 	SINT16		INT16;
	typedef unsigned long  int  DWORD;
	#ifndef _WIN32
	#define DATA		data
	#define PDATA		pdata
	#define IDATA		idata
	#define XDATA		xdata
	#define CODE		code
	#define CONST		code
    #define NEAR
    #define FAR
	#define CODE_P	
	#define REENTRANT	reentrant 
	#else 
	#define DATA		
	#define PDATA		
	#define IDATA		
	#define XDATA		
	#define CODE		
	#define CONST		const	
    #define NEAR
    #define FAR
	#define CODE_P	
	#define REENTRANT		
	#endif

#elif defined( HMI_MCU_JADE )
	typedef unsigned char	UINT8;
	typedef   signed char	SINT8;
	typedef unsigned short	UINT16;
	typedef   signed short	SINT16;
	typedef unsigned int	UINT32_T;
	typedef UINT32_T    	UINT32;
	typedef   signed int	SINT32;
	typedef SINT32			INT32;
	typedef unsigned char	BOOLEAN;

	typedef UINT8			U08;
	typedef UINT8			U8;
	typedef UINT16			U16;
	typedef UINT32_T		U32;
	typedef  float			float_32;
	typedef  double 		double_64;
	typedef float_32		HMI_TIME;
	typedef 	SINT16		INT16;
	typedef unsigned long  int  DWORD;
	
	#define CONST			const	
	#ifndef _WIN32
    #define NEAR
    #define FAR
	#define REENTRANT
	#else	
	#define REENTRANT
	#endif
#elif defined( CC_GENERIC )
    /*
    //  "Generic" Base Data Types
    */
    typedef unsigned char   UINT8;
    typedef signed char		SINT8;
    typedef unsigned short  UINT16;
    typedef signed short	SINT16;
    typedef unsigned int    UINT32_T;
	typedef UINT32_T    	UINT32;
    typedef signed int		SINT32;
	typedef SINT32			INT32;
    typedef unsigned char   BOOLEAN;
	typedef float         	float_32;
	typedef double			double_64;
	typedef float_32		HMI_TIME;
	typedef 	SINT16		INT16;
	typedef unsigned long  int  DWORD;
    #define CONST			const

#elif defined( IAR_M68HC12_TYPES )
    /*
    //  Base Data Types
    */
    typedef unsigned char   UINT8;
    typedef   signed char   SINT8;
    typedef unsigned short int   UINT16;
    typedef   signed short int   SINT16;
    typedef unsigned long   UINT32_T;
	typedef UINT32_T    	UINT32;
    typedef   signed long   SINT32;
    typedef unsigned char   BOOLEAN;
	typedef  float         	float_32;
	typedef  double			double_64;

	typedef UINT8			U08;
	typedef UINT8			U8;
	typedef UINT16			U16;
	typedef UINT32_T		U32;
	typedef float_32		HMI_TIME;
	typedef unsigned long  int  DWORD;
	#ifndef _WIN32
    #define NEAR
    #define FAR
	#else	
	#define REENTRANT
	#endif
	#define CONST			const
#else
    #error HMI_SYSTEM.H: The compiler, mcu, was not detected
#endif 

/*
//  Derived Types
*/
typedef UINT8   UBYTE;
typedef SINT8   SBYTE;

#ifndef WIN32
#ifndef HMI_SYLIXOS
typedef UINT32_T  ULONG;
#endif
#endif
typedef SINT32  SLONG;

#if !defined( NULL )
    #define NULL    ((void *) 0)
#endif

#define TESTBIT( operand, bit_mask )        (((operand) &  (bit_mask)) != ((bit_mask) - (bit_mask)))
#define SETBIT( operand, bit_mask )         ((operand) |= (bit_mask))
#define CLEARBIT( operand, bit_mask )       ((operand) &= ((UINT32_T)~(bit_mask)))

#if !defined( FALSE )
    #define FALSE   ((BOOLEAN) 0)
#endif

#if !defined( TRUE )
    #define TRUE    ((BOOLEAN) 1)
#endif

#define IS_TRUE(x) (x != FALSE)
#define IS_FALSE(x) (x == FALSE)


/*
 "bit" position mask definitions
*/
#define BIT0    0x01
#define BIT1    0x02
#define BIT2    0x04
#define BIT3    0x08
#define BIT4    0x10
#define BIT5    0x20
#define BIT6    0x40
#define BIT7    0x80
#define BIT8    0x0100
#define BIT9    0x0200
#define BIT10   0x0400
#define BIT11   0x0800
#define BIT12   0x1000
#define BIT13   0x2000
#define BIT14   0x4000
#define BIT15   0x8000
#define BIT16   0x00010000
#define BIT17   0x00020000
#define BIT18   0x00040000
#define BIT19   0x00080000
#define BIT20   0x00100000
#define BIT21   0x00200000
#define BIT22   0x00400000
#define BIT23   0x00800000
#define BIT24   0x01000000
#define BIT25   0x02000000
#define BIT26   0x04000000
#define BIT27   0x08000000
#define BIT28   0x10000000
#define BIT29   0x20000000
#define BIT30   0x40000000
#define BIT31   0x80000000

#define HMI_SUPPORT_FLOAT_ANGEL				HMI_YES//HMI_NO

#endif 

