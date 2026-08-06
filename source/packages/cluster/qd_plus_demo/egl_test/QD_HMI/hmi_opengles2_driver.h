
#ifndef _HMI_QD_RGL_DRIVER_H
#define _HMI_QD_RGL_DRIVER_H
#define Z_TRANS         1.75f

#define SZ				1.0f     

#define FB_SX			HMI_MAX_WIDTH
#define FB_SY			HMI_MAX_HEIGHT

#define	Z_ACCEL 		0.001f

#define HMI_CHAR_SPACE		0
#define HMI_CHAR_LINE_SPACE	0
#define HMI_CHAR_HEIGHT		0

#define HMI_LOAD_TICK_VALUE	1
#define HMI_DRAW_TICK_VALUE	0
#define HMI_MAX_TICK_VALUE		0xffffffff


#define CUBE_BASE_ANGLE		0.25f

#define HMI_INVALID_CHAR 	0x3F

#define HMI_GB2312_HB_MAX 	0xF7
#define HMI_GB2312_HB_MIN 	0xA1
#define HMI_GB2312_LB_MAX 	0xFE
#define HMI_GB2312_LB_MIN 	0xA1

#define HMI_GBK_HB_MAX 		0xFE
#define HMI_GBK_HB_MIN 		0x81
#define HMI_GBK_LB_MAX 		0xFE
#define HMI_GBK_LB_MIN 		0x40
#define HMI_GBK_LB_INVALID 	0x7F

#define HMI_BIG5_HB_MAX 	0xFE
#define HMI_BIG5_HB_MIN 	0xA0
#define HMI_BIG5_LB_MAX 	0xFE
#define HMI_BIG5_LB_MID1 	0xA1
#define HMI_BIG5_LB_MID2 	0x7E
#define HMI_BIG5_LB_MIN 	0x40

#define HMI_SJIS_HB_MAX 	0xFC
#define HMI_SJIS_HB_MID1 	0xE0
#define HMI_SJIS_HB_MID2 	0x9F
#define HMI_SJIS_HB_MIN 	0x81
#define HMI_SJIS_LB_MAX 	0xFC
#define HMI_SJIS_LB_MID1 	0x80
#define HMI_SJIS_LB_MID2 	0x7E
#define HMI_SJIS_LB_MIN 	0x40

#define HMI_SJIS_ASCII_MAX	0xDF
#define HMI_SJIS_ASCII_MID1	0xA0
#define HMI_SJIS_ASCII_MID2	0x7F
#define HMI_SJIS_ASCII_MIN	0x00
#define HMI_INVALIATE_FONT_ID	0xff

#define HMI_FLOAT_TOLERANCE	0.000001
#define HMI_SCALE_1			1.0f

#define HMI_GET_BUFFER_CNT	200
#define HMI_MAX_LOOP_CNT	200

//#define HMI_TEXT_SCALE_ENABLE			HMI_NO
#define HMI_TEXT_SCALE_LINE_DIS		0
#define HMI_TEXT_ALIGN_CENTER_SCROLL	HMI_NO
#define HMI_TEXT_CYCLE_SCROLL_BIT		14
#define HMI_TEXT_CYCLE_SCROLL_TIMES_BIT	12
#define HMI_TEXT_CYCLE_SCROLL
#define HMI_TEXT_CYCLE_RANGE		5

#define HMI_DMA_ENABLE				HMI_NO
#define HMI_LAYER_MAX_CNT				1
#define HMI_VIDEO_CAPTURE_WM_BOTTOM	1
#define HMI_STRIDE(W)					(((W+127)>>7)<<7)
#define HMI_STRIDE_256(W)				(((W+255)>>8)<<8)
#define HMI_STRIDE_8(H)					(((H+7)>>3)<<3)
#define HMI_STRIDE_4(H)					(((H+3)>>2)<<2)


#define HMI_VIDEO_STRIDE(W)				HMI_STRIDE(W)
#define HMI_TEXTURE_STRIDE(W)			(((W+3)>>2)<<2)

#define HMI_MAX_RGL_32BIT				0xffffffff
#define HMI_MAX_RGL_OVERFLOW			0xffffffff
#define HMI_OPAQUE						255
#define HMI_WHITE_COLOR_ARGB			0xffffff	

//#define HMI_MEMORY_LONG_TIME_RELEASE	HMI_YES

#define HMI_LAYER_RLE					0
#define HMI_LAYER_SPRITE				1
#define HMI_LAYER_BUFFER				2			
#define HMI_LAYER_VIDEOIN				3

#define	HMI_RGB_LEN						3
#define HMI_ARGB_LEN					4
#define HMI_RGL_WND_BURST_LEN			128
#define	HMI_TRAIL_SEGMENT				2
#define HMI_TRAIL_TOLERATION			0.5

#define ROM_FS_SCRIPT_SWZ				0
#define ROM_FS_WIREMESH01				1
#define LOC_BYTE_PER_PIXEL				4

#define HMI_ANGEL_TO_RADIAN(angel)		(((angel)*HMI_PI)/180.0f)
#define HMI_TRAIL_VERTEX_NUM			16

#define	HMI_BMP_SEGMENT_MAX_CNT		15

/*****OpenGL *******/
/**********Marco function *********/
#define VERTEX_POS_SIZE				3 /* x, y and z */
#define VERTEX_NORMAL_SIZE			3 /* x, y and z */
#define VERTEX_TEXCOORD0_SIZE		2 /* s and t */
#define VERTEX_TEXCOORD1_SIZE		2 /* s and t */
#define VERTEX_COLOR_SIZE			4 /*r, g, b, and a */


#define	HMI_COLOR_ARGB_LEN			4

#define VERTEX_POS_INDX				0
#define VERTEX_NORMAL_INDX			1
#define VERTEX_TEXCOORD0_INDX		2

#define VERTEX_COLOR_INDX			3
#define VERTEX_TEXCOORD1_INDX		4

//#define VERTEX_TEXCOORD1_INDX		VERTEX_COLOR_INDX

#define	HMI_3D_VERTEX_BUF_ENABLE	HMI_YES//HMI_YES
#define	HMI_VERTEX_BUF_INDEX		0
#define	HMI_NORMAL_BUF_INDEX		1
#define	HMI_UV_BUF_INDEX			2
#define	HMI_INDEX_BUF_INDEX			3
#define	HMI_COLOR_BUF_INDEX			HMI_UV_BUF_INDEX
#define	HMI_VERTEX_BUF_CNT			4


#define VERTEX_POS_OFFSET			0
#define VERTEX_NORMAL_OFFSET		3
#define VERTEX_TEXCOORD0_OFFSET		6
#define VERTEX_TEXCOORD1_OFFSET		8
#define HMI_6_VERTEX					6
#define VERTEX_ATTRIB_SIZE		(VERTEX_POS_SIZE + \
								VERTEX_NORMAL_SIZE + \
								VERTEX_TEXCOORD0_SIZE + \
								VERTEX_TEXCOORD1_SIZE)

#define	HMI_IMAGE_VERTEX_CNT			4
#define	HMI_IMAGE_VERTEX_ARRAY_CNT		(HMI_IMAGE_VERTEX_CNT*VERTEX_ATTRIB_SIZE)

#define VERTEX_POS_BUF_INDX				0
#define VERTEX_NORMAL_BUF_INDX			1
#define VERTEX_COLOR_BUF_INDX			VERTEX_NORMAL_BUF_INDX

#define VERTEX_TEX_COOR_BUF_INDX		2
#define VERTEX_INDEX_BUF_INDX			3
#define VERTEX_BUF_MAX_CNT				4

#define HMI_VERTEX_POS_ARRAY_LEN		(HMI_IMAGE_VERTEX_CNT*VERTEX_POS_SIZE)
#define HMI_VERTEX_NOR_ARRAY_LEN		(HMI_IMAGE_VERTEX_CNT*VERTEX_NORMAL_SIZE)
#define HMI_VERTEX_TEX0_ARRAY_LEN		(HMI_IMAGE_VERTEX_CNT*VERTEX_TEXCOORD0_SIZE)
#define HMI_VERTEX_TEX1_ARRAY_LEN		(HMI_IMAGE_VERTEX_CNT*VERTEX_TEXCOORD1_SIZE)
#define HMI_VERTEX_COLOR_ARRAY_LEN		(HMI_IMAGE_VERTEX_CNT*VERTEX_COLOR_SIZE)

#define HMI_TEXT_UV_STRING_LEN			255

#define HMI_TEXT_UV_ARRAY_LEN			(HMI_TEXT_UV_STRING_LEN*HMI_IMAGE_VERTEX_CNT*VERTEX_TEXCOORD1_SIZE)
#define HMI_TEXT_CHAR_ARRAY_LEN			(HMI_TEXT_UV_STRING_LEN*HMI_IMAGE_VERTEX_CNT*VERTEX_TEXCOORD1_SIZE)
#define HMI_TEXT_CHAR_INDEX_ARRAY_LEN	(HMI_TEXT_UV_STRING_LEN*HMI_IMAGE_VERTEX_CNT)

#define HMI_VERTEX_INDICS_ARRAY_LEN		6



#define	HMI_CAM_POS_ELEM		0x01
#define	HMI_LOOKAT_ELEM			0x02
#define	HMI_UP_ELEM				0x04
#define	HMI_L_POS_ELEM			0x08
#define	HMI_L_DIFF_ELEM			0x10
#define	HMI_L_AMB_ELEM			0x20
#define	HMI_L_SPE_ELEM			0x40


#define HMI_MATRIX_ROW					4
#define HMI_MATRIX_COL					4

#define HMI_MATRIX_ROW_3				3
#define HMI_MATRIX_COL_3				3
#define	HMI_PERSPECTIVE_FOVY			60.0f//45.0f//60.0f
#define	HMI_PERSPECTIVE_NEARZ			5//1.0f
#define	HMI_PERSPECTIVE_POS			(HMI_PERSPECTIVE_NEARZ + 1)//1.0f#define	HMI_PERSPECTIVE_			7//1.0f

#define	HMI_PERSPECTIVE_FARZ			20//20.0f

#define PNG_BYTES_TO_CHECK				4
#define HAVE_ALPHA						1
#define NO_ALPHA						0

#define HMI_SUPPORT_PNG				HMI_NO//HMI_YES
#define HMI_SUPPORT_JPG				HMI_NO//HMI_YES


#define HMI_MIN_MULTI_LINE_CNT 							2
#define HMI_MIN_TRIANGLE_POINT_CNT						3

#define	HMI_GPU_GREEDY						HMI_NO//HMI_YES//HMI_NO

#define	HMI_BEFORE_SWAP_FLUSH				HMI_YES//HMI_YES//HMI_NO



#define	HMI_TRANSFORM_COORDINATE(point,new_origin)	point.x -= new_origin.x;\
														point.y -= new_origin.y


#define	HMI_NOT_EXPAND_SPINE	0x80
//#define	HMI_TRANSFORM_COORDINATE_REVERSE_Y(point,new_origin)	point.x -= new_origin.x;\



/**********#define marco *********/
#define HMI_ECT1_ALPHA_OFFSET				(1)

#define QD_DRAW_IMAGE_METHORD				GL_STATIC_DRAW /*GL_STREAM_DRAW,GL_STATIC_DRAW,GL_DYNAMIC_DRAW*/
#define	HMI_INVALIDE_ZONE_NO				4


#if defined (HMI_WINDOWS)	
#define HMI_MALLOC(len)					malloc(len) 
#define HMI_FREE(pdata)					free(pdata)

#define	HMI_PHYSICAL_SCREEN_NO			(HMI_SCREEN0_ENABLE + HMI_SCREEN1_ENABLE)
#define	HMI_RENDER_BUFFER_NO			3

#define HMI_SCREEN_NO					(HMI_PHYSICAL_SCREEN_NO + HMI_RENDER_BUFFER_NO)
#define HMI_SINE_COS_BUFFER_LEN			15

#elif defined (HMI_LINUX)
#define HMI_MALLOC(len)					malloc(len) 
#define HMI_FREE(pdata)					free(pdata)

#define	HMI_PHYSICAL_SCREEN_NO			(HMI_SCREEN0_ENABLE + HMI_SCREEN1_ENABLE)
#define	HMI_RENDER_BUFFER_NO			3

#define HMI_SCREEN_NO					(HMI_PHYSICAL_SCREEN_NO + HMI_RENDER_BUFFER_NO)

#define HMI_SINE_COS_BUFFER_LEN			15

#elif defined (HMI_QNX)
#define HMI_MALLOC(len)					malloc(len) 
#define HMI_FREE(pdata)					free(pdata)

#define	HMI_PHYSICAL_SCREEN_NO			(HMI_SCREEN0_ENABLE + HMI_SCREEN1_ENABLE)
#define	HMI_RENDER_BUFFER_NO			3

#define HMI_SCREEN_NO					(HMI_PHYSICAL_SCREEN_NO + HMI_RENDER_BUFFER_NO)

#define HMI_SINE_COS_BUFFER_LEN			15

#elif defined (HMI_SYLIXOS)
#define HMI_MALLOC(len)					malloc(len) 
#define HMI_FREE(pdata)					free(pdata)

#define	HMI_PHYSICAL_SCREEN_NO			(HMI_SCREEN0_ENABLE + HMI_SCREEN1_ENABLE)
#define	HMI_RENDER_BUFFER_NO			3

#define HMI_SCREEN_NO					(HMI_PHYSICAL_SCREEN_NO + HMI_RENDER_BUFFER_NO)

#define HMI_SINE_COS_BUFFER_LEN			15

#elif defined (HMI_NO_OS)
#define HMI_MALLOC(len)					malloc(len) 
#define HMI_FREE(pdata)					free(pdata)

#define	HMI_PHYSICAL_SCREEN_NO			(HMI_SCREEN0_ENABLE + HMI_SCREEN1_ENABLE)
#define	HMI_RENDER_BUFFER_NO			3

#define HMI_SCREEN_NO					(HMI_PHYSICAL_SCREEN_NO + HMI_RENDER_BUFFER_NO)

#define HMI_SINE_COS_BUFFER_LEN			15

#else
	#error hmi_opengles2_driver.c: Not support th OS.
#endif



#define	HMI_MAX_TREE_CNT					5

#define HMI_CAMERA_MIN_NEAR_PLANES			(0.01f)
#define HMI_CAMERA_MIN_FAR_PLANES			(1.0f)
#define HMI_CAMERA_MIN_VIEW					(1.0f)
#define HMI_CAMERA_MAX_VIEW					(170.0f)//(180.0f)


typedef struct
{
	UINT16	key;	
	SINT16	height;		
}HMI_AVL_TREE_KEY_NODE_STR;

#ifdef HMI_WINDOWS
#define ESUTIL_API  __cdecl
#define ESCALLBACK  __cdecl
#else
#define ESUTIL_API  
#define ESCALLBACK  

#endif

/* esCreateWindow flag - RGB color buffer */
#define ES_WINDOW_RGB           0
/* esCreateWindow flag - ALPHA color buffer */
#define ES_WINDOW_ALPHA         1 
/* esCreateWindow flag - depth buffer */
#define ES_WINDOW_DEPTH         2 
/* esCreateWindow flag - stencil buffer */
#define ES_WINDOW_STENCIL       4
/* esCreateWindow flat - multi-sample buffer */
#define ES_WINDOW_MULTISAMPLE   8


#define		HMI_MATRIE_MAX_ROW		4
#define		HMI_MATRIE_MAX_COL		4
typedef struct
{
    GLfloat   m[HMI_MATRIX_ROW][HMI_MATRIX_COL];
} ES_MATRIX_STR;

//#ifdef HMI_WINDOWS
typedef struct
{
    GLfloat   m[HMI_MATRIE_MAX_ROW][HMI_MATRIE_MAX_COL];
} ESMatrix;
typedef enum 
{
	QD_PATH_END,		/*close*/
	QD_PATH_MOVE_TO,
	QD_PATH_MOVE_TO_REL,
	QD_PATH_LINE_TO,
	QD_PATH_LINE_TO_REL,
	QD_PATH_QUAD,
	QD_PATH_QUAD_REL,
	QD_PATH_CUBLIC,
	QD_PATH_CUBLIC_REL,	
	/*********COUNT*********/
	//QD_PATH_CNT
} HMI_PATH_CMD_ENU;
typedef struct {
	HMI_PATH_CMD_ENU cmd; 
	INT32 			 x;			  
	INT32			 y;			  
} HMI_PATH_STEP;

typedef struct
{
	/// Put your user data here...
	void	*userData;
	UINT32 hmi_vertex_shader_offset;
	UINT32 hmi_vertex_shader_len;
	UINT32 hmi_fragment_shader_offset;
	UINT32 hmi_fragment_shader_len;
	UINT32 hmi_bin_fmt;
	/// Window width
	GLint       width;

	/// Window height
	GLint       height;

#ifdef HMI_WINDOWS
	GLFWwindow	*pwindow;
#else
//	#if(HMI_OPENGL_GLFW == YSE)
//	GLFWwindow	*pwindow;
//	#endif


	/// Window handle
	EGLNativeWindowType  hWnd;

	/// EGL display
	EGLDisplay  eglDisplay;

	/// EGL context
	EGLContext  eglContext;

	/// EGL surface
	EGLSurface  eglSurface;
#endif	

} ESContext;

//#endif



struct	HMI_AVL_TREE_NODE_STR
{	
	UINT16						key;	
	SINT16						height;	
	struct HMI_AVL_TREE_NODE_STR		*pleft;
	struct HMI_AVL_TREE_NODE_STR		*pright;	
	SINT16						next_free_index;
	UINT8						x_index;
	UINT8						y_index;
	UINT16						char_w;
	UINT16						char_h;
};

typedef struct
{	
	UINT16						key;			
	UINT8						x_index;
	UINT8						y_index;
	UINT16						char_w;
	UINT16						char_h;
}HMI_AVL_TREE_USER_DATA_STR;

typedef enum
{
	HMI_TEXTURE_UNIT_BMP,
	HMI_TEXTURE_UNIT_FONT_BUFFER,
	HMI_TEXURE_UINT_FRMBUF,
	HMI_TEXURE_UINT_CUSTOM1,// 2023 03 07
	/*Count*/
	HMI_TEXTURE_UNIT_CNT
}HMI_TEXTURE_UNIT_STR;



typedef struct
{	
	struct HMI_AVL_TREE_NODE_STR		*parray_tree[HMI_MAX_TREE_CNT];	
	UINT16						max_tree_node_cnt;	
	UINT16						user_data_node_len;
	SINT32						free_node_head;
	struct HMI_AVL_TREE_NODE_STR		*phead;
	UINT16						font_w;
	UINT16						font_h;
	BYTE						*pbig_font_buffer;
	GLuint						texture_id;
	UINT16						cur_buffer_index;
}HMI_AVL_TREE_ROOT_NODE_STR;





typedef struct
{
	float_32	center_x;
	float_32	center_y;
	float_32	scale_x;
	float_32	scale_y;
}HMI_SCALE_POS_STR;

typedef struct
{
	GLfloat	angle;
	GLfloat	sin_angle;
	GLfloat	cos_angle;
}SIN_COS_BUFFER_STR;

typedef struct
{
	SIN_COS_BUFFER_STR sin_cos_buffer[HMI_SINE_COS_BUFFER_LEN];
	U16					next;/*new value write next index*/
}SIN_COS_BUFFER_FIFO_STR;


typedef struct		_pic_data pic_data;

struct _pic_data
{
	UINT32	width;
	UINT32	height;
	UINT32	bit_depth;
	BOOLEAN	alpha;
};


typedef enum 
{
	HMI_QD_RGB565,
	HMI_QD_RGB888,
	HMI_QD_ARGB8888,
	HMI_QD_RGBA8888,
	HMI_QD_ARGB4444,
	HMI_QD_RGB444,
	HMI_QD_RGBA4444,
	HMI_QD_ARGB1555,
	HMI_QD_RGBA5551,
	HMI_QD_ALPHA8,
	HMI_QD_RLE24ARGB8888,
	HMI_QD_RLE18ARGB8888,
	HMI_QD_RLE24RGB0888,
	HMI_QD_RLE18RGB0888,
	HMI_QD_RLE8CLUT8,
	HMI_QD_RLE8CLUT4,
	HMI_QD_RLE8CLUT1,
} hmi_qd_fbformat_t;

typedef enum
{
	HMI_IN_LINE_NOT_DIRECT,
	HMI_IN_LINE_SAME_DIRECT,
	HMI_IN_LINE_REVERSE_DIRECT,
	/*Count*/
	HMI_IN_LINE_DIRECT_CNT
}HMI_IN_LINE_STR;

typedef enum
{
	HMI_UNIFORM_IMAGE_TYPE=0,
	HMI_UNIFORM_IMAGE_LIST_TYPE,
	HMI_UNIFORM_TEXT_TYPE,
	HMI_UNIFORM_SCROLL_TYPE,
	HMI_UNIFORM_CUBE_IMAGE_TYPE,
	HMI_UNIFORM_SPLINE_IMAGE_TYPE,
	HMI_UNIFORM_R8_IMAGE_TYPE,/*2020 06 08*/
	
	HMI_UNIFORM_CUBE_COLOR_TYPE=30,
	HMI_UNIFORM_FILL_COLOR_TYPE,
	HMI_UNIFORM_SPLINE_COLOR_TYPE,
	HMI_UNIFORM_CUBE_3D_COLOR_TYPE,
	HMI_UNIFORM_CUBE_3D_TEXTURE_COLOR_TYPE,
	HMI_UNIFORM_ETC1_ALPHA_TYPE,// 35
	HMI_UNIFORM_3D_MODEL_COLOR_TYPE,// 36
	HMI_UNIFORM_3D_MODEL_TEXTURE_TYPE,// 37
	HMI_UNIFORM_3D_MODEL_PICKUP_TYPE,// 38 
	HMI_UNIFORM_3D_MODEL_ONE_COLOR_TYPE,	// 39
	/*draw_3d line*/
	HMI_UNIFORM_3D_LINE_TYPE,//40
	HMI_UNIFORM_3D_POINT_TYPE,//41
	HMI_UNIFORM_DRAW_BORDER,//42
	HMI_UNIFORM_SHADOW,//43 
	HMI_UNIFORM_CENTER_SCALE_CUSTOM_TYPE=60,

	
	/*Count*/
	HMI_UNIFORM_IMAGE_TYPE_CNT
}HMI_UNIFORM_IMAGE_TYPE_STR;

typedef struct
{
    float_32			camera_near;
	float_32			camera_far;
	float_32			view_angle;	
} HMI_DLL_CAMERA_STR;


typedef enum
{
	HMI_ABGR_COLOR_R,
	HMI_ABGR_COLOR_G,
	HMI_ABGR_COLOR_B,
	HMI_ABGR_COLOR_A,
	/*HMI_COLOR_COMPONENT_COUNT*/
	HMI_COLOR_COMPONENT_COUNT
}HMI_ABGR_COMPONENT_STR;

typedef enum 
{	
	TEXTURE_2D_PIXEL_FORMAT_RGBA8888,
	TEXTURE_2D_PIXEL_FORMAT_RGB888,
	TEXTURE_2D_PIXEL_FORMAT_RGB565,
	TEXTURE_2D_PIXEL_FORMAT_A8,
	TEXTURE_2D_PIXEL_FORMAT_A4,
	TEXTURE_2D_PIXEL_FORMAT_A2,
	TEXTURE_2D_PIXEL_FORMAT_A1,
	TEXTURE_2D_PIXEL_FORMAT_ETC1,
	TEXTURE_2D_PIXEL_FORMAT_ETC2RGB888,
	TEXTURE_2D_PIXEL_FORMAT_ETC2RGBA8888,
	/*********COUNT*********/
	TEXTURE_2D_PIXEL_FORMAT_CNT,
} TEXTURE_2D_PIXEL_FORMAT;

typedef struct 
{
	HMI_OBJECT_ID_STR			fathenter_element_id;
	HMI_OBJECT_ID_STR			element_id;
	GLuint						array_buffer_id;
	GLuint						index_buffer_id;
	GLfloat						ratio;/* w/h */
}ELEM_ID_BUFFER_ID_STR;

typedef enum
{
	VERTEX_BUFFER_POSITION_ID,
	VERTEX_BUFFER_NORMAL_ID,
	VERTEX_BUFFER_TEXTURE_ID,
	VERTEX_BUFFER_INDEX_ID,
	/******vertex buffer id cnt***/
	VERTEX_BUFFER_ID_CNT
}vertex_buffer_id_str;



typedef struct
{
    GLfloat   m[HMI_MATRIX_ROW_3][HMI_MATRIX_COL_3];
} ES_MATRIX_3X3_STR;


typedef struct 
{
	GLuint						texture_id;
	UINT32						memory_size;			
}QD_TEXTURE_ID_STR;



typedef enum 
{
	QD_CPU_MEMORY_RELEASE_ONE_OLD,
	QD_GPU_MEMORY_RELEASE_MUL_OLD,
	QD_CPU_GPU_MEMORY_RELEASE_OLD,
	/*********COUNT*********/
	QD_MEMORY_RELEASE_CNT
} QD_MEMORY_RELEASE_STR;

typedef enum 
{
	HMI_DECODE_IMAGE_SUCCESS,
	HMI_DECODE_IMAGE_FAILED,
	HMI_DECODE_IMAGE_MEM_NOT_ENOUGH,	
	/*********COUNT*********/
	HMI_DECODE_STATUS_CNT,
} HMI_DECODE_STATUS_STR;


typedef struct
{
	UINT32					ticks;
	U08						*pBuffer;	
	QD_TEXTURE_ID_STR		texture;
}QD_TEXTURE_INFO_STR;



typedef struct
{
   // Handle to a program object
   GLuint programObject;

} USER_DATA_STR;




//BOOLEAN shader_init ( ESContext *esContext );



typedef struct 
{
	float_32						*pposition;	/*x,y,z*/
	float_32						*protation;	/*x,y,z*/
	float_32						*pscale;	/*x,y,z*/
	float_32						angle_z;	
}HMI_POS_ROT_SCALE_STR;


typedef struct 
{
	float_32						fNearPlanes;
	float_32						fFarPlanes;
	float_32						fieldView;	/* HMI_CAMERA_MIN_VIEW,	HMI_CAMERA_MAX_VIEW*/
	float_32						fRatio;		/*screen w / h*/
}HMI_PROJECT_STR;


typedef struct 
{
	INT32							light_type;			/*0:direction,1:spot,2:point,-1 :no light*/
	//HMI_COLOR_STR					light_color;	
	HMI_COLOR_STR					light_ambiend_color;
	HMI_COLOR_STR					light_diffuse_color;
	HMI_COLOR_STR					light_specular_color;
	INT32							light_shadows_type;	/*0:no shadow,1:soft shadow,2:hard shadow*/
	float_32						light_range;
	float_32						ground_d;
}HMI_LIGHT_SHADOW_PARAMETER_STR;


#define LOC_JCUA_UNIT					(0u)

//#define HMI_PI						3.14159265358979323846		
#define HMI_PI							3.1415926535897932384626433832795f		
	

#define LOC_VRAM_IMAGE_COUNT			19	
#define HMI_CIRCLE_ANGEL				360
#define HMI_HALF_CIRCLE_ANGEL			180

							
#define	HMI_FONT_BUFFER_LEN			100
#define HMI_DISPLAY_ASCII_BEGIN			32
#define	HMI_ALL_FONT_NOT_HIT_LEN		200
#define HMI_NOT_HIT_INVALIDE_INDEX		0xffff
#define HMI_GET_SPLINE_DIRECT(attr)		(attr &0x0f)

#define HMI_COORDINATE_ORIGIN_TRANS(x,y,origin_x,origin_y)	\
									x = x-origin_x;			\
									y = y-origin_y


#define HMI_R_ASYNCHRONOUS_HW_UPDATE	HMI_NO//HMI_NO//HMI_YES
typedef enum 
{
	HMI_FIRST_CON,
	HMI_LAST_CON,
	HMI_MIDDLE_CON,
	HMI_ALONE_CON,
	HMI_FARSI_TYPE_LEN
}HMI_FARSI_CHAR_TYPE;


typedef struct
{
	HMI_FONT_CHAR_STR	font_code;
	U08					*pdata;
}FONT_BUF_ADDR_STR;


typedef struct
{
	U08					font_id;
	FONT_BUF_ADDR_STR	font_buffer;
}MUL_FONT_BUF_ADDR_STR;

typedef struct
{
	GLint			vertexNum;
	GLfloat CONST	*pvertexArray;
	GLfloat	CONST	*pnormalArray;
	GLfloat	CONST	*puvArray;
	GLfloat	CONST	*pcolorArray;
	GLint			numIndices;
	/*GLushort*/GLuint CONST	*pindices;
	GLuint			textureID;
	//GLuint 			vboIds[HMI_VERTEX_BUF_CNT];	// 2023 02 12
	GLuint 			*pvboIds;// 2023 02 12 point to vboIds[HMI_VERTEX_BUF_CNT] 2023 02 12
}HMI_VERTEX_ATTR_STR;

typedef struct
{	
	GLuint 		vboIds[HMI_VERTEX_BUF_CNT];
}HMI_VERTEX_BUF_STR;


typedef struct
{
	UINT32	ticks;
	UINT32	hmi_object_id;
	U08		*pBuffer;	
}rgl_jpg_decode_buffer_str;



typedef enum
{
	HMI_ROTATION_PRIVATE_AXIS,
	HMI_ROTATION_PUBLIC_AXIS1,
	HMI_ROTATION_PUBLIC_AXIS2,
	/*HMI_ROTATION_AXIS COUNT*/
	HMI_ROTATION_AXIS_CNT
}hmi_rotation_axis_str;

typedef enum
{
	HMI_POS_X,
	HMI_POS_Y,
	HMI_POS_Z,
	/*HMI_POS_COMPONENT_COUNT*/
	HMI_POS_COMPONENT_COUNT
}HMI_POS_COMPONENT_STR;


typedef enum
{
	HMI_IMAGE_NO_COMPRESS,
	HMI_IMAGE_JPG,
	HMI_IMAGE_RLE,
	HMI_IMAGE_PNG,
	HMI_IMAGE_ALPHA8,
	HMI_IMAGE_ALPHA4,
	HMI_IMAGE_ALPHA2,
	HMI_IMAGE_ALPHA1,
	HMI_IMAGE_ETC,
	/*HMI_IMAGE COUNT*/
	HMI_IMAGE_CNT
}rgl_image_type_str;

typedef struct   
{	
	GLfloat direction[HMI_POS_COMPONENT_COUNT];
	GLfloat ambient_color[HMI_COLOR_COMPONENT_COUNT];
	GLfloat diffuse_color[HMI_COLOR_COMPONENT_COUNT];
	GLfloat specular_color[HMI_COLOR_COMPONENT_COUNT];
	GLfloat camera_pos[HMI_POS_COMPONENT_COUNT];
	GLfloat	specular_exponent;/*add 2020 11 03*/
}DIRECTIONAL_LIGHT_STR;

typedef struct 
{	
	GLfloat ambient_color[HMI_COLOR_COMPONENT_COUNT];
	GLfloat diffuse_color[HMI_COLOR_COMPONENT_COUNT];
	GLfloat specular_color[HMI_COLOR_COMPONENT_COUNT];
	/*float specular_exponent;*/
}MATERIAL_PROPERTIES_STR;


typedef struct 
{	
	MATERIAL_PROPERTIES_STR materialProp;	
	GLuint					textureID;
	BOOLEAN					normalEn;
	BOOLEAN					colorlEn;
	BOOLEAN					texturelEn;
	BOOLEAN					twoSideEn;// 2023 03 22
#ifndef HMI_SIMULATE_QD
	HMI_BITMAP_STR	CONST	*pimg_data;// 2023 02 12
#endif
}HMI_MATERIAL_PROPERTIES; 

struct	HMI_3D_NODE_INFO_STR
{
	HMI_VERTEX_ATTR_STR				*pvertex_info;
	HMI_MATERIAL_PROPERTIES			*pmaterial_info;	
	INT32							meshCnt;
	struct	HMI_3D_NODE_INFO_STR	*pchild;
	INT32							childCnt;
#ifdef HMI_SIMULATE_QD
	U16	/*_TCHAR*/					strName[HMI_SUB_3D_MODEL_NAME_LEN];// 2021 05 07 ,no used for DLL
	U16	/*_TCHAR*/					strCName[HMI_SUB_3D_MODEL_NAME_LEN];//according C language name,no used for DLL
	C_STRUCT aiMatrix4x4			mTransformation;
#endif
};


typedef struct
{
    double X;
    double Y;
    double Z;    
} vector3d_t;

typedef struct
{
    double X;
    double Y;
    double Z;
    double W;    
} vector4d_t;

typedef enum
{
	HMI_BLEND_TRANSPARENT,
	/*HMI_BLEND_CNT*/
	HMI_BLEND_CNT
}HMI_BLEND_MODE_STR;


typedef enum 
{
	/*add by lq*/
	HMI_IMG_ALPHA1,
	HMI_IMG_ALPHA2,
	HMI_IMG_ALPHA4,
	/**********/
	HMI_IMG_ALPHA8,
	HMI_IMG_RGB565,
	HMI_IMG_ARGB8888,
	HMI_IMG_RGBA8888,
	HMI_IMG_ARGB6666,
	HMI_IMG_RGBA6666,
	HMI_IMG_ARGB1555,
	HMI_IMG_RGB888,
	HMI_IMG_RGBX8888,
	HMI_IMG_RGBA5551,
	HMI_IMG_ARGB4444,
	HMI_IMG_RGBA4444,
	HMI_IMG_CLUT8
} hmi_img_ColorFormat_t;

typedef enum 
{
    HMI_IMG_ATTRIBUTE_RLE_COMPRESSED                = (1 << 0),
    HMI_IMG_ATTRIBUTE_USE_FROM_PERSISTENT_MEMORY	= (1 << 1),
    HMI_IMG_ATTRIBUTE_SWIZZLE                       = (1 << 2),
    HMI_IMG_ATTRIBUTE_CLUT                          = (1 << 3),
    HMI_IMG_ATTRIBUTE_RLE_DECOMPRESS                = (1 << 4),
    HMI_IMG_ATTRIBUTE_UNUSED                        = (1 << 5),
} hmi_img_Attributes_t;



typedef enum
{
	HMI_DRAW_IMAGE_TYPE,
	HMI_DRAW_MUL_TEXTURE_IMAGE_TYPE,
	HMI_DRAW_FILL_TYPE,	
	HMI_DRAW_IMAGE_LIST_TYPE,
	HMI_DRAW_TEXT_TYPE,
	HMI_DRAW_SCROLLBAR_TYPE,
	HMI_DRAW_SWAP_BUFFER_TYPE,
	HMI_DRAW_ENABLE_SPRITE_TYPE,
	HMI_DRAW_DISABLE_SPRITE_TYPE,
	HMI_DRAW_PAGE_ALPHA_TYPE,
	HMI_DRAW_VIDEO_TYPE
}HMI_DRAW_INFO_TYPE_STR;

typedef struct
{
	HMI_FILL_PAGE_STR CONST * phmi_fill_page_prop;
	HMI_RECT_STR 			clip_rect;
	HMI_RECT_STR* 			pdirty_rect;
	UINT8					layer;
	#ifdef	HMI_GRAPHIC_OPENGLES
	UINT8					father_alpha;
	#endif
}HMI_FILL_INFO_STR;


#if 0
typedef struct
{
	UINT32	 vertex_begin;
	UINT32	 vertex_end;

	UINT32	 color_begin;
	UINT32	 color_end;

	UINT32	 normal_begin;
	UINT32	 normal_end;

	UINT32	 uv_begin;
	UINT32	 uv_end;

	UINT32	 index_begin;
	UINT32	 index_end;

	UINT32	 material_begin;
	UINT32	 material_end;

	UINT32	 node_list_index;/*child begin index */
	UINT32	 node_cn;/*child cnt */	 
}HMI_3D_NODE_STR;
#endif
typedef struct
{
	UINT16						mesh_cn;
	HMI_RANGE_MESH_STR	CONST	*pvertex_range; 
	HMI_RANGE_MESH_STR	CONST	*pcolor_range; 
	HMI_RANGE_MESH_STR	CONST	*pnormal_range;
 	HMI_RANGE_MESH_STR	CONST	*puv_range;
	HMI_RANGE_MESH_STR	CONST	*pindex_range;
	HMI_RANGE_MESH_STR  CONST	*pmaterial_range;	
 
	UINT32	 node_list_index;	/*child begin index */
	UINT32	 node_cn;			/*child cnt */	 
}HMI_3D_NODE_STR;

#if 0
typedef struct
{	
	U08				attribute;
	union
	{
		S3POINT_TP			target;/*look at*/
		HMI_OBJECT_ID_STR	target_id;
	}
	
	union
	{
		S3POINT_TP			up;
		HMI_OBJECT_ID_STR	up_id;
	}
	
	union
	{
		S3POINT_TP			position;
		HMI_OBJECT_ID_STR	position_id;
	}	

	/*light*/
	union
	{
		GLfloat direction[HMI_POS_COMPONENT_COUNT]
		HMI_OBJECT_ID_STR	dir_pos_id;
	}

	union
	{
		GLfloat diffuse_color[HMI_COLOR_COMPONENT_COUNT];
		HMI_OBJECT_ID_STR	diffuse_id;
	}
	
	union
	{
		GLfloat ambient_color[HMI_COLOR_COMPONENT_COUNT];
		HMI_OBJECT_ID_STR	ambient_id;
	}	

	union
	{
		GLfloat specular_color[HMI_COLOR_COMPONENT_COUNT];
		HMI_OBJECT_ID_STR	specular_id;
	}

	UINT32					node_index;
}HMI_3DCUBE_STR;

#endif

typedef struct
{	
	HMI_RECT_STR 			screen_target;
	HMI_IMAGE_ATTR_STR		img_compress;
	HMI_RECT_ALPHA_ANGEL_STR *palpha_pos_angel; 										
	HMI_BITMAP_STR CONST	*pimage_prop_info;
	HMI_RECT_STR			clip_rect;
	HMI_RECT_STR*			pdirty_rect;
	UINT8					layer;						
	HMI_ROTATION_STR		rotation;/*rotation point,NULL mean center point*/
	HMI_OBJECT_ID_STR		hmi_object_id;
#ifdef	HMI_GRAPHIC_OPENGLES
	UINT8					father_alpha;
#endif
}HMI_IMAGE_INFO_STR;

typedef struct
{	
	HMI_RECT_STR 			screen_target;
	HMI_IMAGE_ATTR_STR		img_compress;
	HMI_RECT_ALPHA_ANGEL_STR *palpha_pos_angel; 										
	HMI_BITMAP_STR CONST	*pimage_prop_info;
	HMI_RECT_STR			clip_rect;
	HMI_RECT_STR*			pdirty_rect;
	UINT8					layer;						
	HMI_ROTATION_STR		rotation;/*rotation point,NULL mean center point*/
	HMI_OBJECT_ID_STR		hmi_object_id;
#ifdef	HMI_GRAPHIC_OPENGLES
	UINT8					father_alpha;
#endif
	HMI_MUL_TEXTURE_LEN_INDEX_STR CONST	*pmul_texture_prop;
	HMI_TEXTURE_RECT_STR			*pmul_texture_rect_list;
}HMI_MUL_TEXTURE_IMAGE_STR;

typedef struct
{	
	HMI_RECT_STR 			screen_target;
	HMI_IMAGE_ATTR_STR		img_compress;
	HMI_RECT_ALPHA_STR 		*palpha_pos_angel;	
	HMI_IMAGE_LIST_STR CONST *phmi_imagelist_prop;
	HMI_RANGE_STR			index;
	HMI_RECT_STR			clip_rect;
	HMI_RECT_STR*			pdirty_rect;
	UINT8					layer;
	HMI_OBJECT_ID_STR		hmi_object_id;
#ifdef	HMI_GRAPHIC_OPENGLES
	UINT8 					father_alpha;
#endif
}HMI_IMAGE_LIST_INFO_STR;


typedef struct
{	
	HMI_TEXT_RECT_STR 		hmi_text_copy_rect;
	HMI_TEXT_PROP_STR		hmi_text_prop_info;
	HMI_RECT_STR 			clip_rect;
	HMI_RECT_STR* 			pdirty_rect;
	U08 					font_id;
	UINT8					layer;
#ifdef HMI_CLIP_TEXT
	HMI_CHAR_STR CONST *phmi_clip_char;
#endif
#ifdef	HMI_GRAPHIC_OPENGLES	
	UINT8 			father_alpha;
#endif
}HMI_TEXT_INFO_STR;

typedef struct
{
	HMI_RECT_STR 			screen_target; 
	HMI_IMAGE_ATTR_STR		img_compress;													
	HMI_RANGE_STR			cur_range;
	HMI_RECT_STR			clip_rect;
	HMI_RECT_STR* 			pdirty_rect;
	HMI_SCROLL_BAR_STR		*phmi_scrollbar_prop;
	UINT8					layer;
	HMI_OBJECT_ID_STR	hmi_object_id;
	#ifdef	HMI_GRAPHIC_OPENGLES
	UINT8 father_alpha;
	#endif
}HMI_SCROLL_BAR_INFO_STR;

typedef struct
{
	HMI_RECT_STR* 			pdirty_zone; 
	
}HMI_SWAP_BUFFER_INFO_STR;

typedef struct
{
	UINT8 					alpha_value; 
	BOOLEAN					only_set_alpha;
}HMI_PAGE_ALPHA_INFO_STR;

typedef struct
{
	HMI_VIDEO_INFO_STR CONST 	*pfmt;
	HMI_RECT_STR 				*ppos;
	BYTE 						*pvideo_status;
	BYTE 						layer;
}HMI_DISP_VIDEO_INFO_STR;

typedef struct
{
	HMI_DRAW_INFO_TYPE_STR	type;
	union
	{
		HMI_FILL_INFO_STR		fill;
		HMI_IMAGE_INFO_STR		image;
		HMI_MUL_TEXTURE_IMAGE_STR mul_texture_image;
		HMI_IMAGE_LIST_INFO_STR	image_list;
		HMI_TEXT_INFO_STR		text;
		HMI_SCROLL_BAR_INFO_STR scroll_bar;
		HMI_SWAP_BUFFER_INFO_STR swap_buffer;
		HMI_PAGE_ALPHA_INFO_STR  page_alpha;
		HMI_DISP_VIDEO_INFO_STR  video;
	};
}HMI_DRAW_INFO_STR;

#define HMI_DRAW_FIFO_CNT					60

typedef struct
{
	HMI_DRAW_INFO_STR hmi_draw_info_list[HMI_DRAW_FIFO_CNT];
	U08					head;
	U08					tail;
}HMI_DRAW_FIFO_STR;
#define HMI_LINE_BREAK 0xFEFD//0x0a

#if(HMI_FARSI_NEED_REVERSE	==	HMI_YES)
#define HMI_BLANK_UNICODE     			32
#define HMI_ASC_NUMBER_BEGIN    		33/*22--!  48--'0'*/
#define HMI_ASC_NUMBER_END     			63/* 63--? 57--'9'*/

#define HMI_FARSI_UNICODE_BEGIN1   		0x600 
#define HMI_FARSI_UNICODE_END1    		0x6ff
#define HMI_FARSI_UNICODE_BEGIN2   		0x750
#define HMI_FARSI_UNICODE_END2    		0x77f
#define HMI_FARSI_UNICODE_BEGIN3   		0xfb50
#define HMI_FARSI_UNICODE_END3    		0xfdff
#define HMI_FARSI_UNICODE_BEGIN4   		0xfe70
#define HMI_FARSI_UNICODE_END4    		0xfeff
#define HMI_FARSI_NUMBER_BEGIN    		0x660
#define HMI_FARSI_NUMBER_END     			0x669

#define HMI_HEBREW_CHAR_UNICODE_BEGIN5   0x0590	/* basic char*/
#define HMI_HEBREW_CHAR_UNICODE_END5    	0x05ff

//#define HMI_HEBREW_NIQQUD_UNICODE_BEGIN6   	0x05b0	/* 注音符号*/
//#define HMI_HEBREW_NIQQUD_UNICODE_END6    	0x05c7


#define HMI_UNICODE_SYMBOL_BEGIN		0x2000		
#define HMI_UNICODE_SYMBOL_END			0x22FF
#define HMI_LEFT_BRACKET				0x28/*(*/
#define HMI_RIGHT_BRACKET				0x29/*)*/
#define HMI_LEFT_SQUARE_BRACKET			0x5B/*[*/
#define HMI_RIGHT_SQUARE_BRACKET		0x5D/*]*/

#define HMI_LEFT_BRACKETS		0x7B/*{*/
#define HMI_RIGHT_BRACKETS		0x7D/*}*/



typedef struct
{
	U16 	begin;/*include begin index*/
	U16 	end;/*Not include begin index*/
	BOOLEAN reverse;
}HMI_FARSI_ANALYSIS_STR;

typedef enum
{
	HMI_INIT_FIND_NUMBER,
	HMI_BEGIN_FIND_NUMBER,
	HMI_GO_ON_FIND_NUMBER,
	HMI_END_FIND_NUMBER,
	HMI_FIND_NUMBER_CNT
}HMI_FIND_NUMBER_STATUS_STR;

typedef enum
{
	HMI_INIT_CHAR_STATUS,
	HMI_FARSI_CHAR_STATUS,
	HMI_NUMBER_DOT_CHAR_STATUS,
	HMI_OTHER_CHAR_STATUS,
	/*farsi*/
	HMI_FARSI_CHAR_STATUS_CNT
}HMI_SEARCH_FARSI_WORD_STR;

typedef enum
{
	HMI_NULL_SENTENCE,
	HMI_FARSI_SENTENCE_BEGIN,
	HMI_FARSI_SENTENCE_END,
	/*farsi*/
	HMI_FARSI_SENTENCE_CNT
}HMI_SEARCH_FARSI_SENTENCE_STR;

typedef enum
{
	HMI_BLANK_WORD_TYPE,
	HMI_OTHER_WORD_TYPE,
	HMI_FARSI_WORD_TYPE,
	HMI_NUMBER_DOT_WORD_TYPE,
	/*farsi*/
	HMI_WORD_TYPE_CNT
}HMI_WORD_TYPE_STR;

typedef enum
{
	HMI_BLANK_READ_CHAR,
	HMI_FARSI_READ_CHAR,
	HMI_NUMBER_DOT_READ_DOT,
	HMI_OTHER_READ_CHAR,
	/*farsi*/
	HMI_FARSI_CHAR_TYPE_CNT
}HMI_READ_CHAR_TYPE_STR;

typedef enum
{
	HMI_GET_ONE_FARSI_WORD,
	HMI_BEGIN_INDEX_WORD,
	HMI_NOTHING_WORD,
	HMI_GET_ONE_OTHER_WORD,
	/*farsi*/
	HMI_STATUS_ACTION_CNT
}HMI_FARSI_STATUS_ACTION_STR;

typedef enum
{
	HMI_BEGIN_ANALYSIS_SENTENCE,
	HMI_REVERSE_FARSI_SENTENCE,
	HMI_REVERSE_FARSI_GON_ON_SENTENCE,
	HMI_END_ANALYSIS_SENTENCE,
	/*farsi*/
	HMI_ANALYSIS_SENTENCE_CNT
}HMI_ANALYSIS_SENTENCE_STR;

typedef enum
{
	HMI_SENTENCE_REVERSE_STATUS,
	HMI_NUMBER_REVERSE_STATUS,
	HMI_END_REVERSE_STATUS,
	/*Count*/
	HMI_REVERSE_STATUS_CNT
}HMI_REVERSE_STATUS_STR;
#endif

#if 0
typedef struct
{
	BYTE	*ppublic_buffer;
	U32 	buffer_len;
	
}HMI_PUBLIC_BUFFER_STR;
#endif

typedef struct
{
	U16				array_len;
	U16				length;/*3*(n-2)*/
	U16				valide_length;
	POINT32_TP 		*phmi_output_point_triangle;	
	GLfloat			*poutput_point_triangle_gl;
	GLfloat			*poutput_point_triangle_uv_gl;
	GLushort		*pvertex_index;
}HMI_OUTPUT_TRIANGLE_STR;

typedef struct
{
	GLfloat	X;
	GLfloat	Y;
	GLfloat	Z;
	GLfloat	W;
}r_drw2d_Vec4_t;

typedef struct
{
	union
	{
		S3POINT_TP			target;/*look at*/
		HMI_OBJECT_ID_STR	target_id;
	};
	
	union
	{
		S3POINT_TP			up;
		HMI_OBJECT_ID_STR	up_id;
	};
	
	union
	{
		S3POINT_TP			position;
		HMI_OBJECT_ID_STR	position_id;
	};		
}HMI_CAMERA_ATTR;	

#if 0
typedef struct
{	
	U08				attribute;
	union
	{
		S3POINT_TP			private_pos;	/*look at*/
		HMI_OBJECT_ID_STR	private_id;
	};
	
	union
	{
		S3POINT_TP			public1;
		HMI_OBJECT_ID_STR	public1_id;
	};
	
	union
	{
		S3POINT_TP			public2;
		HMI_OBJECT_ID_STR	public2_id;
	};		
}HMI_CUBE_AXIS_ELEM_PROP;
#endif

typedef struct
{	
	U08				attribute;
	
	S3POINT_TP			private_pos;	/*look at*/
	//HMI_OBJECT_ID_STR	private_id;
			
	S3POINT_TP			public1;
	//HMI_OBJECT_ID_STR	public1_id;
			
	S3POINT_TP			public2;
	//HMI_OBJECT_ID_STR	public2_id;
		
}HMI_CUBE_AXIS_ELEM_PROP;



typedef struct
{
	GLfloat color[HMI_COLOR_COMPONENT_COUNT];
}HMI_LIGHT_COLOR_STR;

typedef struct
{
	GLfloat direction[HMI_POS_COMPONENT_COUNT];
}HMI_LIGHT_DIRECTION_STR;

#if 0
typedef struct
{
	HMI_RECT_STR	cube_rect;
	float_32		angel;
	BOOLEAN			bump;/*bit0--bump*/
	HMI_Z_STR		z;
	float_32		private_angel;/*angel location axis*/
	U08				attribute;
	float_32		scale;

	union
	{
		S3POINT_TP			target;/*look at*/
		HMI_OBJECT_ID_STR	target_id;
	};
	
	union
	{
		S3POINT_TP			up;
		HMI_OBJECT_ID_STR	up_id;
	};
	
	union
	{
		S3POINT_TP			position;
		HMI_OBJECT_ID_STR	position_id;
	};	

	/*light*/
	union
	{
		SINT16 direction[HMI_POS_COMPONENT_COUNT];
		HMI_OBJECT_ID_STR	dir_pos_id;
	};

	union
	{
		BYTE diffuse_color[HMI_COLOR_COMPONENT_COUNT];
		HMI_OBJECT_ID_STR	diffuse_id;
	};
	
	union
	{
		BYTE ambient_color[HMI_COLOR_COMPONENT_COUNT];
		HMI_OBJECT_ID_STR	ambient_id;
	};	

	union
	{
		BYTE specular_color[HMI_COLOR_COMPONENT_COUNT];
		HMI_OBJECT_ID_STR	specular_id;
	};

	UINT32					node_index;

}HMI_3DCUBE_STR;
#endif




typedef struct
{
	HMI_RECT_STR	cube_rect;
	float_32		angel;
	BOOLEAN			bump;/*bit0--bump*/
	HMI_Z_STR		z;
	float_32		private_angel;/*angel location axis*/
	U08				attribute;
	float_32		scale;

/*if element id,save at 0 index*/
	S3POINT_TP			target;/*look at*/
	S3POINT_TP			up;
	S3POINT_TP			position;
	/*light*/
	SINT16				direction[HMI_POS_COMPONENT_COUNT];
	SINT16				diffuse_color[HMI_COLOR_COMPONENT_COUNT];
	SINT16				ambient_color[HMI_COLOR_COMPONENT_COUNT];
	SINT16				specular_color[HMI_COLOR_COMPONENT_COUNT];
	/*node index*/
	UINT32				node_index;

}HMI_3DCUBE_STR;


typedef struct
{
	GLfloat	X;
	GLfloat	Y;
	GLfloat	Z;
	GLfloat	W;
}r_drw2d_IntRect_t;


typedef enum 
{
	HMI_POLYGONTYPE_ERRORPOINT,
	HMI_POLYGONTYPE_CONVEXPOINT,
	HMI_POLYGONTYPE_CONCAVEPOINT,
	/*HMI_VERTEXTYPE_MAX_CNT*/
	HMI_VERTEXTYPE_MAX_CNT
}HMI_VERTEXTYPE_STR;

typedef enum 
{
	HMI_STRIDE_VERTEX_POSITION,
	HMI_STRIDE_VERTEX_NORMAL,
	HMI_STRIDE_VERTEX_TEXTURE,
	HMI_STRIDE_VERTEX_COLOR,
	HMI_STRIDE_VERTEX_TEXTURE_ETC1_ALPHA,
	/*HMI_STRIDE_VERTEX_CNT*/
	HMI_STRIDE_VERTEX_CNT
}HMI_STRIDE_VERTEX_STR;




typedef enum 
{
	HMI_POLYGONTYPE_UNKNOWN,
	HMI_POLYGONTYPE_CLOCKWISE,
	HMI_POLYGONTYPE_COUNT_CLOCKWISE,
	/*HMI_POLYGONTYPE_MAX_CNT*/
	HMI_POLYGONDIRECTION_MAX_CNT
}HMI_POLYGONDIRECTION_STR;

#if 0
typedef struct
{
	/* Handle to a program object */
	GLuint	programObject;
	GLuint	vertexShader;
	GLuint	fragmentShader;
	/* Uniform locations */
	GLint	mvp_unform_id;
	GLint	tex_unform_id;	
	GLint	tex_sample_id;	
	GLint	tex_texture_custom0;	/*used as MASK*/
	GLint	screen_w_id;
	GLint	screen_h_id;
	GLint	tex_font_sample_id;
	GLint	draw_type_id;
	GLint	attribute_id;
	GLint	global_alpha_id;
	GLint	one_color_id;
	
	GLint	u_light_direction_id; /* normalized light direction in eye space*/	
	GLint	u_light_ambient_color_id;
	GLint	u_light_diffuse_color_id;
	GLint	u_light_specular_color_id;
	GLint	u_light_specular_exponent_id;

	GLint 	u_material_ambient_color_id;
	GLint 	u_material_diffuse_color_id;
	GLint	u_material_specular_color_id;
	
	GLint	u_camera_id;
} HMI_USER_DATA_STR;
#endif
typedef struct
{
	/* Handle to a program object */
	GLuint	programObject;
	GLuint	vertexShader;
	GLuint	fragmentShader;
	/* Uniform locations */
	GLint	mvp_unform_id;
	GLint	mv_unform_id;
	GLint	p_unform_id;
	GLint	m_to_world_unform_id;
	GLint	normal_to_world_unform_id;
	GLint	tex_unform_id;	
	GLint	tex_sample_id;	
	GLint	tex_texture_custom0;	/*used as MASK*/
	GLint	tex_texture_custom1;
	GLint	screen_w_id;
	GLint	screen_h_id;
	GLint	tex_font_sample_id;
	GLint	draw_type_id;
	GLint	attribute_id;
	GLint	global_alpha_id;
	GLint	one_color_id;

	GLint	u_light_type_id;			/*0:direct,1:spot light,2:point light,-1:no light,-2:pickup*/
	GLint	u_light_direction_id; /* normalized light direction in eye space*/	
	GLint	u_light_ambient_color_id;
	GLint	u_light_diffuse_color_id;
	GLint	u_light_specular_color_id;
	GLint	u_light_specular_exponent_id;// 2020 11 2

	GLint 	u_material_ambient_color_id;
	GLint 	u_material_diffuse_color_id;
	GLint	u_material_specular_color_id;
	
	GLint	u_camera_id;
	GLint	u_resolution;
	GLint	u_blur_size;
	GLint	u_qd_tool;// 2023 04 06   0:at run at API.1:run at qd tool 
#ifndef HMI_SIMULATE_QD
	GLint	u_big_img_w;
	GLint	u_big_img_h;
	GLint	u_small_img_x;
	GLint	u_small_img_y;
	GLint	u_small_img_w;
	GLint	u_small_img_h;
#endif
} HMI_USER_DATA_STR;


#define	HMI_TRANSFORM_COORDINATE(point,new_origin)				point.x -= new_origin.x;\
																point.y -= new_origin.y


#define	HMI_TRANSFORM_COORDINATE_REVERSE_Y(point,new_origin)	point.x -= new_origin.x;\
																point.y = new_origin.y - point.y


typedef enum
{
	GET_CHAR_DATA_CALL_BACK,
	GET_ELEMENT_TYPE_CALL_BACK,
	GET_ELEMENT_PROPERTY_CALL_BACK,
	GET_CONTAINER_ALL_CHILD_CALL_BACK,
	//GET_ELEMENT_PROP_CALL_BACK,
	GET_CUSTOM_PROP_CALL_BACK,
	GET_GET_STATIC_ID_CALL_BACK,
	GET_DRAW_CONTAINER_CALL_BACK,
	CMP_CSTRING_CALL_BACK,
	GET_CUBE_GET_CONTAINER_PROP,
	GET_ELEMENT_PROPERTY2_CALL_BACK,// 2020 06 16
	GET_ELEMENT_BEZIER_CALL_BACK,// 2020 06 16
	//GET_GET_ACTION_CALL_BACK,// 2020 06 16
	GET_GET_ACTION_EXECUTER_CALL_BACK,// 2020 06 16	
	SET_QD_ATTRIBUTE_CALL_BACK,// 2020 08 06
	GET_ROTATION_CHAR_DATA_CALL_BACK,// 2021 09 23
	SET_CUSTOM_PROP_CALL_BACK,// 2021 11 12
	GET_SUB_MODEL_VERTEX_INFO_BCK,// 2022 10 13
	GET_SUB_MODEL_MATERIAL_INFO_BCK,// 2022 10 13
	GET_PICKUP_COLOR_BCK,// 2022 11 25
	GET_CUR_NODE_NAME_CALL_BCK,// 2022 12 28
	PRESET1_DATA_CALL_BACK,
	PRESET2_DATA_CALL_BACK,
	PRESET3_DATA_CALL_BACK,
	PRESET4_DATA_CALL_BACK,
	PRESET5_DATA_CALL_BACK,
	PRESET6_DATA_CALL_BACK,
	PRESET7_DATA_CALL_BACK,
	PRESET8_DATA_CALL_BACK,
	PRESET9_DATA_CALL_BACK,
	PRESET10_DATA_CALL_BACK,
	PRESET11_DATA_CALL_BACK,
	PRESET12_DATA_CALL_BACK,
	PRESET13_DATA_CALL_BACK,
	PRESET14_DATA_CALL_BACK,
	PRESET15_DATA_CALL_BACK,
	PRESET16_DATA_CALL_BACK,
	/*QD CALL BACK COUNT*/
	QD_CALL_BACK_FUNC_CNT
}QD_CALL_BACK_FUNC_STR;

typedef struct
{		
	GLuint	framebuffer;
	GLuint	depthRenderbuffer;
	GLuint	texture;
	GLuint	stencilRenderbuffer;
	UINT32	w;
	UINT32	h;
	GLuint	old_framebuffer;
	UINT32	old_w;
	UINT32	old_h;
	GLenum	internalFormat;
	BOOLEAN	enable;
}HMI_RENDER_BUFFER_STR;

typedef enum
{
	HMI_MASK_HMI_RENDER_BUFFER,	
	HMI_CUBE_RENDER_BUFFER,/*Cube container*/
	/*Count*/
	HMI_RENDER_BUFFER_CNT
}HMI_RENDER_BUFFER_ENUM;



typedef enum
{
	HMI_RGL_QD_TYPE,	
	HMI_CYPRESS_QD_TYPE,	
	HMI_OPENGLES20_QD_TYPE,
	HMI_OPENGLES30_QD_TYPE,
	HMI_OPENVG_QD_TYPE,
	/*Count*/
	HMI_QD_TYPE_CNT
}HMI_QD_TYPE_ENUM;




typedef enum
{
	HMI_QD_NO_ELEMENT_BORDER,
	HMI_QD_NO_BLEND, 	//	2
	HMI_QD_DIS_MASK,	// 4
	HMI_QD_GAUSSIAN,
	HMI_QD_A8_FRMBUF,
	HMI_QD_SHADOW_TYPE,/*0:soft shadow,1:hard shadow*/
	HMI_CAST_ON,
	HMI_QD_MULTISAMPLE4,
	HMI_QD_ALPHA_0_COPY,
	HMI_QD_ATTRIBUTE1,
	HMI_QD_ATTRIBUTE2,
	HMI_QD_ATTRIBUTE3,
	/*Count*/
	HMI_QD_ATTRIBUTE_CNT
}HMI_QD_ATTRIBUTE_ENUM;

typedef struct
{			
	float_32			scene_min[3/*x,y,z*/]; 
	float_32			scene_max[3/*x,y,z*/]; 
	float_32			scene_center[3/*x,y,z*/];
}HMI_BOUNDING_BOX_STR;
#define			HMI_MAX_SUPPORT_LIGHT_CNT	10
typedef struct
{			
	INT32				light_cnt;
	GLfloat				light_position[HMI_MAX_SUPPORT_LIGHT_CNT][HMI_POS_COMPONENT_COUNT];
	INT32				light_type[HMI_MAX_SUPPORT_LIGHT_CNT];
	float_32			ground_d;/*ground distance to origin*/
}HMI_LIGHT_POS_TYPE_STR;


#if 0	// 2023 02 12
typedef struct
{			
	void			*puser_data;
}HMI_CUSTOM_USR_DATA_STR;
#endif
/*
0:3d object,
1:sub 3d object
2:camera
3:light spot
4:light direct
5:light point
6: 3d container custom
*/

typedef struct
{			
	UINT32	 				node_list_index;	/*node index*/
	HMI_CONTAINER_STR		child;
	//HMI_BITMAP_STR			node_texture;2023 03 14 lq
}HMI_3D_SUB_CUSTOM_USR_DATA_STR;

typedef struct
{		
	INT32	 				custom_type;
	void					*pusr_data;
}HMI_CUSTOM_USR_DATA_STR;

typedef float_32		r_drw2d_FixedP_t;
typedef INT32			r_drw2d_Error_t;
#define	R_DRW2D_ERR_OK	0

typedef struct
{
	r_drw2d_FixedP_t	U;
	r_drw2d_FixedP_t	V;	
}r_drw2d_UVCoord_t;





extern	UINT32					m_bDLL2QDAttribute;
extern	HMI_RENDER_BUFFER_STR	hmi_render_buffer_qd[/*HMI_RENDER_BUFFER_CNT*/];



#ifdef __cplusplus
extern "C" {
#endif
void	hmi_init_render_buffer(void);
void	hmi_destory_render_buffer(void);
void	hmi_destory_render_buffer_init(void);
void	restory_previous_frambuffer(HMI_RENDER_BUFFER_ENUM buf_id);
BOOLEAN hmi_switch_render_buffer(HMI_RENDER_BUFFER_ENUM buf_id,UINT32 w,UINT32 h,GLenum	internalFormat,U08	screen_no);
BOOLEAN hmi_switch_render_buffer2(HMI_RENDER_BUFFER_ENUM buf_id,
									UINT32 w,UINT32 h,
									GLenum	internalFormat,U08	screen_no);

void matrix_by_euler_angle_z0x1y2(ES_MATRIX_STR *presult/*col matrix*/,	
									float_32 x_angle,
									float_32 y_angle,
									float_32 z_angle);

void hmi_set_eglDisplay(EGLDisplay disp);
void hmi_set_eglContext(EGLContext context);
void hmi_set_eglSurface(EGLSurface surf);

/**********************Declare Function*************/
#if defined (HMI_WINDOWS)	
void  qd_init_context (ESContext *esContext );
BOOLEAN qd_create_window(ESContext *esContext,U32 flags);
BOOLEAN shader_init ( ESContext *esContext );
void	qd_register_update_func(ESContext *esContext);
void	qd_register_draw_func(ESContext *esContext);
void *hmi_malloc_memory(UINT32 len);

#elif defined (HMI_LINUX)
void  qd_init_context ( ESContext *esContext );
BOOLEAN qd_create_window(ESContext *esContext,U32 flags);
BOOLEAN shader_init (ESContext *esContext );
void	qd_register_update_func(ESContext *esContext);
void	qd_register_draw_func(ESContext *esContext);
void *hmi_malloc_memory(UINT32 len);

#elif defined (HMI_QNX)
void  qd_init_context ( ESContext *esContext );
BOOLEAN qd_create_window(ESContext *esContext,U32 flags);
BOOLEAN shader_init ( ESContext *esContext );
void	qd_register_update_func(ESContext *esContext);
void	qd_register_draw_func(ESContext *esContext);
void *hmi_malloc_memory(UINT32 len);

#elif defined (HMI_SYLIXOS)
void  qd_init_context ( ESContext *esContext );
BOOLEAN qd_create_window(ESContext *esContext,U32 flags);
BOOLEAN shader_init ( ESContext *esContext );
void	qd_register_update_func(ESContext *esContext);
void	qd_register_draw_func(ESContext *esContext);
void *hmi_malloc_memory(UINT32 len);

#elif defined (HMI_NO_OS)

#else
	
#endif


void add_tick_res_manager(void);
void qd_texture_manager_init(void);
#if ((defined( HMI_MCU_IMX6 ))||(defined(HMI_MCU_X86 ))||\
	(defined( HMI_MCU_R_CAR )))
void qd_jpg_init(void);
#endif
rgl_image_type_str get_compress_fmt(HMI_IMAGE_ATTR_STR img_attr);

void call_C_hmi_driver_init(UINT32 hmi_vertex_shader_offset,
							UINT32 hmi_vertex_shader_len,
							UINT32 hmi_fragment_shader_offset,
							UINT32 hmi_fragment_shader_len,
							UINT32 hmi_bin_fmt,
							HMI_SEGMENT_LIST_STR CONST *pseg_info);
void call_C_hmi_driver_deinit(void);
void locCleanUpVRAM_hmi(void);
#if HMI_ALL_FONT_NUMBER > 0
#if ((HMI_DYN_EDIT_TEXTS_NUMBER > 0U) || (HMI_STATIC_TEXTS_NUMBER > 0U))
void call_C_hmi_driver_draw_text(HMI_TEXT_RECT_STR CONST *phmi_text_copy_rect/*screen target and color*/,
										HMI_TEXT_PROP_STR CONST * phmi_text_prop_info,
										HMI_RECT_STR *pclip_rect,
										HMI_RECT_STR *pdirty_rect,
										U08				font_id,
										UINT8 layer
										#ifdef HMI_CLIP_TEXT
										,HMI_CHAR_STR CONST *phmi_clip_char
										#endif																						
										,HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale
										);
#endif
#endif
void call_C_hmi_driver_draw_fill_page( HMI_FILL_PAGE_STR CONST * phmi_fill_page_prop,
											HMI_RECT_STR *pclip_rect,
											HMI_RECT_STR *pdirty_rect,
											UINT8 layer
											#if (defined(HMI_GRAPHIC_OPENGLES)||defined(HMI_GRAPHIC_RGL))
											//,UINT8 father_alpha
											,HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale
											#endif
											);

void call_C_hmi_driver_gradient_fill_page(HMI_RECT_STR CONST * phmi_gfill_page_rect,
											HMI_GRADIENT_FILL_STR CONST * phmi_gfill_page_prop,
											HMI_RECT_STR *pclip_rect,
											HMI_RECT_STR *pdirty_rect,												
											HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale
											);

void call_C_hmi_driver_draw_image(HMI_RECT_STR CONST * pscreen_target,
								HMI_IMAGE_ATTR_STR		img_compress,
								HMI_RECT_ALPHA_ANGEL_STR *palpha_pos_angel,											
								HMI_BITMAP_STR CONST * pimage_prop_info,
								HMI_RECT_STR *pclip_rect,
								HMI_RECT_STR *pdirty_rect,
								UINT8 layer,								
								HMI_ROTATION_STR *protation,/*rotation point,NULL mean center point*/
								HMI_OBJECT_ID_STR	hmi_object_id,								
								HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale								
								);

void call_C_hmi_driver_draw_imagelist(HMI_RECT_STR CONST		*pscreen_target,
												HMI_IMAGE_ATTR_STR		img_compress,
												HMI_RECT_ALPHA_STR *palpha_pos_angel,	
												HMI_IMAGE_LIST_STR CONST *phmi_imagelist_prop,
												HMI_RANGE_STR			index,
												HMI_RECT_STR			*pclip_rect,
												HMI_RECT_STR 			*pdirty_rect,
												UINT8					layer,
												HMI_OBJECT_ID_STR	hmi_object_id,												
												HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale												
												);

void call_C_hmi_driver_draw_scrollbar(HMI_RECT_STR CONST		*pscreen_target, 
												HMI_IMAGE_ATTR_STR		img_compress,	
												/*HMI_IMAGE_LIST_STR CONST *phmi_imagelist_prop,*/
												HMI_RANGE_STR			cur_range,
												HMI_RECT_STR			*pclip_rect,
												HMI_RECT_STR 			*pdirty_rect,
												HMI_SCROLL_BAR_STR		*phmi_scrollbar_prop,
												UINT8					layer,
												HMI_OBJECT_ID_STR	hmi_object_id,												
												HMI_ALPHA_SCALE_PT_STR		*pfather_alpha_scale												
												);


void call_C_hmi_driver_refresh_LCD(HMI_RECT_STR *pdirty_zone,U08 screen);

void hmi_enable_video_layer(BOOLEAN en);
void loc_Error_hmi(U32 err);
void get_texture_res_manager(HMI_OBJECT_ID_STR hmi_object_id,
										QD_TEXTURE_INFO_STR *ptexture,
										HMI_BITMAP_STR	CONST *pdata,																			
										TEXTURE_2D_PIXEL_FORMAT color_fmt,										
										HMI_FONT_CHAR_STR	*pchar_code/*only used in text  char code or image index*/
										);
HMI_FMT_BIG_IMAGE_BUFFER_STR * get_buffer_res_manager(HMI_OBJECT_ID_STR	hmi_object_id,										
										HMI_BITMAP_STR	CONST *pdata/*flash data address*/,										
										HMI_FONT_CHAR_STR	*pchar_code	
										);

U08 * get_font_buffer_res_manager(U08				font_id,
										HMI_FONT_CHAR_STR	char_code,	
										HMI_BITMAP_STR	CONST *pdata /*flash data address*/,										
										UINT8				data_len
										);

void hmi_driver_swap_buffer(HMI_RECT_STR *pdirty_zone);
void set_ortho_matrix2(UINT32	screen_w,UINT32	screen_h);

void free_all_buffer_res_manager(void);
void free_all_jpg_buffer_res_manager(void);
void hmi_clear_dirty_layer(HMI_RECT_STR *pdirty_zone,U08	screen);
void hmi_driver_set_render_buffer(HMI_RECT_STR *pdirty_zone);
/*GLubyte	*readPixelFromBuffer(UINT width,UINT height,GLint		readType,
									GLint	readFormat,UINT32_T	bytesPerPixel);*/

void hmi_display_video(HMI_VIDEO_INFO_STR CONST *pfmt,HMI_RECT_STR *ppos,BYTE *pvideo_status,BYTE layer);

void call_C_hmi_driver_draw_cube(HMI_RECT_STR CONST 	*pscreen_target,
	HMI_3DCUBE_STR/*HMI_CUBE_STR*/ CONST	*pcube_str,
	HMI_CUBE_FACE_STR	*pcube_textrue,
	HMI_CUBE_FACE_STR	*pbump_textrue,
	HMI_RECT_STR 		*pclip_rect,
	HMI_RECT_STR 		*pdirty_rect,
	UINT8 				layer,
	//UINT8 				father_alpha,
	HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale,
	HMI_CUBE_AXIS_ELEM_PROP	CONST *paxis,
	S3POINT_TP			*protation_axis,
	HMI_CAMERA_STR		*pcamera
);

void	hmi_enable_sprite_array(HMI_RECT_STR *pdirty_zone);
BOOLEAN is_buffer_layer(UINT8 layer);
void clear_layer_video_status(void);
void set_layer_video_status(UINT8 layer,HMI_OBJECT_ID_STR object_id,BYTE	*pvideo_status);
void off_layer_video(void);
//void clear_layer_video_status(void);
void hmi_driver_animated_trail(HMI_ROTATION_TRAIL_STR CONST *ptail,	
								BYTE					trail_attr,
								HMI_RECT_STR  			*pscreen_target/*pointer position*/,
								HMI_IMAGE_ATTR_STR		img_compress,
								HMI_RECT_ALPHA_ANGEL_STR *palpha_pos_angel,											
								HMI_BITMAP_STR  		*pimage_prop_info,
								HMI_RECT_STR			*pclip_rect,
								HMI_RECT_STR			*pdirty_rect,
								UINT8					layer,								
								HMI_ROTATION_STR		*protation,/*rotation point,NULL mean center point*/																
								float_32				pointer_angel,
								HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale,
								HMI_BITMAP_STR CONST 	*pimage_pointer_prop_info,
								HMI_RECT_STR			*ptrail_rect
								);

void free_image_res_manager(HMI_OBJECT_ID_STR hmi_object_id);
void hmi_release_font_buffer(void);
void hmi_init_font_buffer(void);
GLuint get_render_buffer(HMI_RENDER_BUFFER_ENUM buf_id);
POINT32_TP *	get_trail_vextex_pos(U08 * pvertex_len,float_32 begin_angel,
										U16 circle_radius_w,U16 circle_radius_h,
										U08	trail_cw,SPOINT_TP *pdisplay_container_pos_left,
										SPOINT_TP *pdisplay_container_pos_right);

float_32 hmi_init_angel_to_y_axis(HMI_ROTATION_STR	*pointer_target_center);
UINT8 get_popup_window_layer(void);
void hmi_driver_set_page_alpha(UINT8 page_alpha,BOOLEAN only_set_alpha);
void call_C_hmi_driver_draw_3d_image(HMI_RECT_STR CONST 	*pscreen_target,
									HMI_CUBE_STR CONST	*pcube_str,
									HMI_CUBE_FACE_STR	*pcube_textrue,
									HMI_CUBE_FACE_STR	*pbump_textrue,
									HMI_RECT_STR 		*pclip_rect,
									HMI_RECT_STR 		*pdirty_rect,
									UINT8 				layer,																	
									//UINT8 				father_alpha,
									HMI_ALPHA_SCALE_PT_STR *pfather_alpha_scale,
									HMI_CUBE_AXIS_ELEM_PROP	CONST *paxis,
									S3POINT_TP			*protation_axis,
									HMI_CAMERA_STR		*pcamera
									);

#if HMI_ALL_FONT_NUMBER > 0
#if ((HMI_DYN_EDIT_TEXTS_NUMBER > 0U) || (HMI_STATIC_TEXTS_NUMBER > 0U))
HMI_WIDTH_STR hmi_driver_get_string_len( HMI_CHAR_STR *phmi_string,UINT8 font_id);
#endif
#endif
void hmi_driver_send_cmdlist(U08 screen);
void hmi_get_union_rect(HMI_RECT_STR CONST *p_r1,HMI_RECT_STR CONST *p_r2,HMI_RECT_STR *phmi_temp_rect);
HMI_FMT_ADDR_STR CONST * hmi_rgl_load_to_vram(HMI_OBJECT_ID_STR	hmi_object_id,
							HMI_BITMAP_STR	CONST *pdata/*flash data address*/,										
							HMI_FONT_CHAR_STR	*pchar_code
							);

#ifdef HMI_R_ASYNCHRONOUS_HW_UPDATE
void hmi_execute_cmd(void);
void hmi_execute_pop_cmd(void);
void hmi_end_mark_execute_cmd(void);

#endif
void hmi_set_a8_color(UINT8	layer,HMI_COLOR_STR	fill_color);

void call_C_hmi_driver_popup_mul_texture_image(HMI_RECT_STR CONST	*pscreen_target,
										HMI_IMAGE_ATTR_STR		img_compress,
										HMI_RECT_ALPHA_ANGEL_STR *palpha_pos_angel, 										
										HMI_BITMAP_STR CONST	*pimage_prop_info,
										HMI_RECT_STR			*pclip_rect,
										HMI_RECT_STR			*pdirty_rect,
										UINT8					layer,								
										HMI_ROTATION_STR		*protation,/*rotation point,NULL mean center point*/
										HMI_OBJECT_ID_STR		hmi_object_id,
										#ifdef	HMI_GRAPHIC_OPENGLES
										UINT8					father_alpha,
										#endif
										HMI_MUL_TEXTURE_LEN_INDEX_STR	CONST *pmul_texture_prop,
										HMI_TEXTURE_RECT_STR			*pmul_texture_rect_list
										);
void call_C_hmi_driver_draw_mul_texture_image(HMI_RECT_STR CONST 	*pscreen_target,
										HMI_IMAGE_ATTR_STR		img_compress,
										HMI_RECT_ALPHA_ANGEL_STR *palpha_pos_angel,											
										HMI_BITMAP_STR CONST 	*pimage_prop_info,
										HMI_RECT_STR			*pclip_rect,
										HMI_RECT_STR			*pdirty_rect,
										UINT8					layer,								
										HMI_ROTATION_STR		*protation,/*rotation point,NULL mean center point*/
										HMI_OBJECT_ID_STR		hmi_object_id,
										#ifdef	HMI_GRAPHIC_OPENGLES
										UINT8					father_alpha,
										#endif
										HMI_MUL_TEXTURE_LEN_INDEX_STR CONST	*pmul_texture_prop,
										HMI_TEXTURE_RECT_STR			*pmul_texture_rect_list
										);											

void hmi_set_font_aligh(HMI_FONT_ALIGN_STR value_align);

#ifdef HMI_INIT_TEXTURE_MEM_FIRST
void call_C_hmi_driver_init_texture_mem(void);
#endif
void*	hmi_malloc_vram(UINT32 len);
void	hmi_dealloc_vram(void *pram);

void call_C_hmi_driver_draw_spline(HMI_RECT_STR CONST 	*pscreen_target,
								HMI_SPLINE_PROP_STR		*pspline_prop,
								HMI_RECT_STR			*pclip_rect,
								HMI_RECT_STR			*pdirty_rect,
								UINT8					layer,								
								HMI_OBJECT_ID_STR		hmi_object_id,								
								HMI_ALPHA_SCALE_PT_STR		*pfather_alpha_scale								
								);
void call_C_hmi_draw3d_mode_shader(HMI_RECT_STR 	*pscreen_target,
								HMI_CUBE_STR *pcube_str,
								S3POINT_TP	*protation_axis,
								HMI_VERTEX_ATTR_STR *pvertexAttr,
						HMI_MATERIAL_PROPERTIES	*pvertexMaterial,
							BOOLEAN normalEn,BOOLEAN colorlEn,
							BOOLEAN texturelEn,
							//float_32 scale_xyz,
							float scale_x,float scale_y,float scale_z,
							//float center_x,float center_y,float center_z,
							//UINT32_T screenW,UINT32_T screenH,
							HMI_RECT_STR	*pclip_rect,
							HMI_RECT_STR	*pdirty_rect,
							UINT32_T		side,
							DIRECTIONAL_LIGHT_STR	*pdirectLight,
							HMI_CAMERA_STR		*pcamera,
							HMI_BITMAP_STR CONST	*pimage_prop_info,
							GLuint 					vboIds[]
							);


void restory_frambuffer(GLuint old_framebuffer,
						UINT32 old_frmbuf_w,UINT32 old_frmbuf_h);
void restory_frambuffer_win_no_texture_sampler(GLuint old_framebuffer,						
						BYTE		win_no,HMI_TEXTURE_UNIT_STR	sampler_id);

void release_frambuffer_res(GLuint *pframebuffer,
						GLuint *pdepthRenderbuffer,GLuint *ptexture,
						GLuint *pstencilRenderbuffer,
						GLuint *pold_framebuffer);
void	release_one_frambuffer_res(HMI_RENDER_BUFFER_ENUM render_buf_id);


void hmi_driver_draw_image(HMI_RECT_STR		*pscreen_target,			
									HMI_RECT_STR	*pdirty_rect,
									HMI_RECT_STR	*pclip_rect,
									GLuint			texture,
									U08	screen,
									UINT8			alpha,
  									float_32		angle,
									float_32		scale);
void	hmi_draw_path_zone(HMI_RECT_STR 				*pdsp_rect,
									HMI_RECT_STR			*pdirty_rect,
									HMI_RECT_STR			*pcliped_farther_rect,																											
									SPOINT32_TP 			path_data[],INT32	path_len,
									INT32/*HMI_PATH_CMD_ENU*/	path_cmd[],INT32	cmd_len,
									UINT32					color/*ARGB*/,
									ES_MATRIX_STR			*pmvp);
void hmi_driver_draw_3d_image(HMI_RECT_STR	 *pscreen_target,
									//VECTOR_FLOAT_TP		container_conner[/*HMI_25D_CONTAINER_CONNER_CNT*/],
									POINT32_TP		*pwh,
									 HMI_RECT_STR	 *pdirty_rect,
									 HMI_RECT_STR	 *pclip_rect,
									 GLuint 		 texture,U08 screen,
									 UINT8			 alpha, 
									 U08			disp_mode,
									 ES_MATRIX_STR	*pmode_matrix,
									 ES_MATRIX_STR	*pview_matrix,
									 ES_MATRIX_STR	*pperspective_matrix);
void hmi_driver_draw_3d_image2(HMI_RECT_STR	 		*pscreen_target,
										POINT32_TP				*pwh,
									 	HMI_RECT_STR	 		*pdirty_rect,
									 	HMI_RECT_STR	 		*pclip_rect,
									 	HMI_IMAGE_ATTR_STR		img_compress,
										HMI_BITMAP_STR CONST 	*pimage_prop_info,
										U08						screen,
									 	UINT8			 		alpha, 
									 	U08						disp_mode,
									 	ES_MATRIX_STR			*pmode_matrix,
									 	ES_MATRIX_STR			*pview_matrix,
									 	ES_MATRIX_STR			*pperspective_matrix);

void hmi_load_bmp_segment(U08	seg_no);
void hmi_load_bmp_all_segment(void);
void hmi_unload_bmp_segment(U08	seg_no);

#if (HMI_LOAD_MULTI_PROCESS == YES)
void	hmi_load_segment_process(void);
#endif

void	hmi_fclose_all_resource(void);
/*Matrix*/
void matrix_multiply_vector(ES_MATRIX_STR *pmatrix, VECTOR_FLOAT_TP * pvector,VECTOR_FLOAT_TP * pvector_result);
void matrix_multiply_vector2(ES_MATRIX_STR *pmatrix, VECTOR_FLOAT_TP * pvector);
void matrix_multiply_vector_vertical(ES_MATRIX_STR *pmatrix, VECTOR_FLOAT_TP * pvector,VECTOR_FLOAT_TP * pvector_result);

void matrix_load_identity(ES_MATRIX_STR *presult);
void matrix_scale(ES_MATRIX_STR *presult, GLfloat sx, GLfloat sy, GLfloat sz);
void matrix_translate(ES_MATRIX_STR *presult, GLfloat tx, GLfloat ty, GLfloat tz);
void matrix_multiply(ES_MATRIX_STR *presult, ES_MATRIX_STR *srcA, ES_MATRIX_STR *srcB);
void matrix_multiply_vector(ES_MATRIX_STR *pmatrix, VECTOR_FLOAT_TP * pvector,VECTOR_FLOAT_TP * pvector_result);
//void matrix_multiply_vector2(ES_MATRIX_STR *pmatrix, VECTOR_FLOAT_TP * pvector);
void matrix_multiply2(ES_MATRIX_STR *srcA, ES_MATRIX_STR *srcB);
void matrix_translate_left(ES_MATRIX_STR *presult, GLfloat tx, GLfloat ty, GLfloat tz);
void matrix_rotate(ES_MATRIX_STR *presult, GLfloat angle,
						GLfloat x, GLfloat y, GLfloat z,
						BOOLEAN	bidentity_matrix);
void matrix_perspective_normal(ES_MATRIX_STR *result, float_32 fovy, 
				float_32 aspect, float_32 nearZ, float_32 farZ);

void matrix_frustum(ES_MATRIX_STR *result, float left, float right, float bottom, float top, float nearZ, float farZ);
void matrix_frustum2(ES_MATRIX_STR *result, float left, float right, float bottom, float top, float nearZ, float farZ);

void matrix_look_at ( ES_MATRIX_STR *result,
                 float_32 posX,    float_32 posY,    float_32 posZ,
                 float_32 lookAtX, float_32 lookAtY, float_32 lookAtZ,
                 float_32 upX,     float_32 upY,     float_32 upZ );
void matrix_ortho(ES_MATRIX_STR *result, float_32 left, float_32 right, 
			float_32 bottom, float_32 top, float_32 nearZ, float_32 farZ);
void matrix_ortho2(ES_MATRIX_STR *result, float_32 left, float_32 right, 
			float_32 bottom, float_32 top, float_32 nearZ, float_32 farZ);

void matrix_perspective(ES_MATRIX_STR *result, float_32 fovy, 
				float_32 aspect, float_32 nearZ, float_32 farZ,
				BOOLEAN perspective);
void matrix_perspective2(ES_MATRIX_STR *result, float_32 fovy, 
				float_32 aspect, float_32 nearZ, float_32 farZ,
				BOOLEAN perspective);

void matrix_near_far_perspective(ES_MATRIX_STR *presult,	
									HMI_WIDTH_STR		w,
									HMI_HEIGHT_STR		h,
									float_32 nearZ, float_32 farZ);

#ifdef HMI_WINDOWS
GLFWwindow	* hmi_get_glfw_window(void);
#endif
void hmi_driver_set_render_screen(UINT8 screen_id);
UINT8 hmi_driver_get_render_screen(void);
#ifdef HMI_MCU_R_CAR
BOOLEAN createEGLDisplay(EGLDisplay *peglDisplay);
BOOLEAN chooseEGLConfig(EGLDisplay eglDisplay, EGLConfig *peglConfig);
BOOLEAN createEGLSurface(EGLDisplay eglDisplay, EGLConfig eglConfig, EGLSurface *peglSurface);
BOOLEAN setupEGLContext(EGLDisplay eglDisplay, EGLConfig eglConfig, EGLSurface eglSurface, EGLContext *peglContext);
void hmi_set_egl_context(EGLDisplay	egl_display,EGLContext	egl_context,
									EGLSurface	egl_surface);


#endif
UINT16 hmi_get_line_counts(HMI_TEXT_RECT_STR CONST *phmi_text_copy_rect/*screen target and color*/,
								HMI_TEXT_PROP_STR CONST * phmi_text_prop_info,
								U08 					font_id
								);

BOOLEAN	hmi_copy_sub_img(HMI_BITMAP_STR CONST 	*pimage_prop_info,
								U16			xoffset,U16	yoffset,U16	w,U16	h,
								void	*ppixels,HMI_BITMAP_STR *pnew_image_prop_info);
INT32 hmi_line_to_zone(POINT32_TP vertex[], 
							INT32	vertex_len, 
							POINT32_TP vertex_zone[], 
							INT32	 vertex_zone_len,
							INT32	line_width);

INT32 hmi_line_to_zone_float(POINT_FLOAT_TP vertex[], 
							INT32	vertex_len, 
							POINT_FLOAT_TP vertex_zone[], 
							INT32	 vertex_zone_len,
							INT32	line_width);

void call_C_hmi_draw3d_sub_mode_shader(
							ES_MATRIX_STR			*pfather_matrix,
							HMI_POS_ROT_SCALE_STR	*p3dSubModel,
							HMI_VERTEX_ATTR_STR		*pvertexAttr,
							HMI_MATERIAL_PROPERTIES	*pvertexMaterial,																					
							HMI_BITMAP_STR CONST	*pimage_prop_info,
							HMI_BOUNDING_BOX_STR	*pboundBox,
							BOOLEAN					bone_color,
							HMI_COLOR_STR			pickup_color,							
							BOOLEAN					bPickup,
							INT32					radius,									
							INT32					xyz	/*x:0 y:1 z:2*/,
							BYTE					global_alpha,
							BOOLEAN					btransparent_texture);


void call_C_hmi_draw3d_sub_mode_shader_shadow(
							ES_MATRIX_STR			*pfather_matrix,
							ES_MATRIX_STR			*pshadow_matrix,
							HMI_POS_ROT_SCALE_STR	*p3dSubModel,
							HMI_VERTEX_ATTR_STR 	*pvertexAttr,							
							HMI_BOUNDING_BOX_STR	*pboundBox,
							BOOLEAN 				bone_color,
							HMI_COLOR_STR			pickup_color);

void matrix_euler_rotate_x(ES_MATRIX_STR *presult,	
									 float_32 degrees);
void matrix_euler_rotate_y(ES_MATRIX_STR *presult,	
									 float_32 degrees);
void matrix_euler_rotate_z(ES_MATRIX_STR *presult,	
									 float_32 degrees);
void	hmi_set_3d_scene_light_camera(HMI_RECT_STR			*pdsp_zone,
												HMI_OBJECT_ID_STR		view_camera_id,
												HMI_OBJECT_ID_STR		light_id[],
												INT32					light_cnt,
												BOOLEAN					bPickup,
												INT32					radius,									
												INT32					xyz	/*x:0 y:1 z:2*/);

void	hmi_pass_camera_parameter(HMI_POS_ROT_SCALE_STR	*ppos,float_32	camera_near,float_32	camera_far,float_32		view_angle);
void  hmi_set_qd_attributte(HMI_QD_ATTRIBUTE_ENUM		attribute_no,
										BOOLEAN 	clear) ;
void hmi_active_texture_device(HMI_TEXTURE_UNIT_STR texture_device_unit_id,
						HMI_RENDER_BUFFER_ENUM render_buf_id);


void	hmi_draw_draw_cublic_bezier_zone(HMI_RECT_STR		*pscreen_target,
													POINT_FLOAT_TP		path_data_f[],
													U08					bezierLength,
													HMI_COLOR_STR		color,
													HMI_RECT_STR		*pclip_rect,
													INT32				line_width);

void	hmi_destory_one_render_buffer(HMI_RENDER_BUFFER_ENUM buf_id);
void	hmi_fopeng_seg_file(U08 seg_no);
void hmi_draw_fbo(HMI_RENDER_BUFFER_ENUM buf_id);
void	hmi_fclose_seg_file(U08 seg_no);

void hmi_set_polygon_offset_fill_on(void);
void hmi_set_polygon_offset_fill_off(void);
BOOLEAN hmi_switch_render_buffer3(HMI_RENDER_BUFFER_ENUM buf_id,UINT32 w,UINT32 h,
									GLenum	internalFormat,U08	screen_no,UINT32 clear_color);

#ifdef __cplusplus
}
#endif

#endif



