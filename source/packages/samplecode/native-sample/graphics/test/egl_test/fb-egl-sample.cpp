/*
 * libraries needed:   libEGL libGLESv2
 *
 * compile with:   CROSS_COMPLE-g++  --sysroot=$DA_SYSROOT -lEGL -lGLESv2  fb-egl-sample.cpp -o fb-egl.bin
 */
#include  <iostream>
#include  <cstdlib>
#include  <cstring>
using namespace std;
#include <stdlib.h>
#include  <math.h>
#include  <sys/time.h>
#include  <GLES2/gl2.h>
#include  <EGL/egl.h>

typedef struct {
unsigned short width;
unsigned short height;
} fb_window;

const char vertex_src [] =
"                                        \
   attribute vec4        position;       \
   varying mediump vec2  pos;            \
   uniform vec4          offset;         \
                                         \
   void main()                           \
   {                                     \
      gl_Position = position + offset;   \
      pos = position.xy;                 \
   }                                     \
";

const char fragment_src [] =
"                                                      \
   varying mediump vec2    pos;                        \
   uniform mediump float   phase;                      \
                                                       \
   void  main()                                        \
   {                                                   \
      gl_FragColor  =  vec4( 1., 0.9, 0.7, 1.0 ) *     \
        cos( 30.*sqrt(pos.x*pos.x + 1.5*pos.y*pos.y)   \
             + atan(pos.y,pos.x) - phase );            \
   }                                                   \
";
//  some more formulas to play with...
//      cos( 20.*(pos.x*pos.x + pos.y*pos.y) - phase );
//      cos( 20.*sqrt(pos.x*pos.x + pos.y*pos.y) + atan(pos.y,pos.x) - phase );
//      cos( 30.*sqrt(pos.x*pos.x + 1.5*pos.y*pos.y - 1.8*pos.x*pos.y*pos.y)
//            + atan(pos.y,pos.x) - phase );

void
print_shader_info_log (
	GLuint  shader      // handle to the shader
)
{
	GLint  length;

	glGetShaderiv ( shader , GL_INFO_LOG_LENGTH , &length );

	if ( length ) {
		char* buffer  =  new char [ length ];
		glGetShaderInfoLog ( shader , length , NULL , buffer );
		cout << "shader info: " <<  buffer << flush;
		delete [] buffer;

		GLint success;
		glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
		if ( success != GL_TRUE )   exit ( 1 );
	}
}


GLuint
load_shader (
	const char  *shader_source,
	GLenum       type
)
{
	GLuint  shader = glCreateShader( type );

	glShaderSource  ( shader , 1 , &shader_source , NULL );
	glCompileShader ( shader );

	print_shader_info_log ( shader );

	return shader;
}

fb_window *win;
EGLDisplay  egl_display;
EGLContext  egl_context;
EGLSurface  egl_surface;

GLfloat
	norm_x    =  0.0,
	norm_y    =  0.0,
	offset_x  =  0.0,
	offset_y  =  0.0,
	p1_pos_x  =  0.0,
	p1_pos_y  =  0.0;

GLint
	phase_loc,
	offset_loc,
	position_loc;

bool update_pos = false;

const float vertexArray[] = {
	0.0,  0.5,  0.0,
	-0.5,  0.0,  0.0,
	0.0, -0.5,  0.0,
	0.5,  0.0,  0.0,
	0.0,  0.5,  0.0 
};

#define W 1024
#define H 600

void  render()
{
	static float  phase = 0;
	static int    donesetup = 0;

	//static XWindowAttributes gwa;

	//// draw

	if ( !donesetup ) {
		glViewport ( 0 , 0 , W , H );
		glClearColor ( 0.08 , 0.06 , 0.07 , 1.);    // background color
		donesetup = 1;
	}
	glClear ( GL_COLOR_BUFFER_BIT );

	glUniform1f ( phase_loc , phase );  // write the value of phase to the shaders phase
	phase  =  ::fmodf ( phase + 0.5f , 2.f * 3.141f );    // and update the local variable

	if ( update_pos ) {  // if the position of the texture has changed due to user action
		GLfloat old_offset_x  =  offset_x;
		GLfloat old_offset_y  =  offset_y;

		offset_x  =  norm_x - p1_pos_x;
		offset_y  =  norm_y - p1_pos_y;

		p1_pos_x  =  norm_x;
		p1_pos_y  =  norm_y;

		offset_x  +=  old_offset_x;
		offset_y  +=  old_offset_y;

		update_pos = false;
	}

	glUniform4f ( offset_loc  ,  offset_x , offset_y , 0.0 , 0.0 );

	glVertexAttribPointer ( position_loc, 3, GL_FLOAT, false, 0, vertexArray );
	glEnableVertexAttribArray ( position_loc );
	glDrawArrays ( GL_TRIANGLE_STRIP, 0, 5 );

	eglSwapBuffers ( egl_display, egl_surface );  // get the rendered buffer to the screen
}

int  main()
{
	///////  the X11 part  //////////////////////////////////////////////////////////////////
	// in the first part the program opens a connection to the X11 window manager
	//

	win = (fb_window *)calloc(1, sizeof (fb_window));
	win->width = W;
	win->height = H; 

	///////  the egl part  //////////////////////////////////////////////////////////////////
	//  egl provides an interface to connect the graphics related functionality of openGL ES
	//  with the windowing interface and functionality of the native operation system (X11
	//  in our case.

	egl_display  =  eglGetDisplay( (EGLNativeDisplayType) 0 );
	if ( egl_display == EGL_NO_DISPLAY ) {
		cerr << "Got no EGL display." << endl;
		return 1;
	}

	if ( !eglInitialize( egl_display, NULL, NULL ) ) {
		cerr << "Unable to initialize EGL" << endl;
		return 1;
	}

	EGLint attr[] = {       // some attributes to set up our egl-interface
		EGL_BUFFER_SIZE, 16,
		EGL_RENDERABLE_TYPE,
		EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	EGLConfig  ecfg;
	EGLint     num_config;
	if ( !eglChooseConfig( egl_display, attr, &ecfg, 1, &num_config ) ) {
		cerr << "Failed to choose config (eglError: " << eglGetError() << ")" << endl;
		return 1;
	}

	if ( num_config != 1 ) {
		cerr << "Didn't get exactly one config, but " << num_config << endl;
		return 1;
	}

	egl_surface = eglCreateWindowSurface ( egl_display, ecfg, (NativeWindowType)win, NULL );
	if ( egl_surface == EGL_NO_SURFACE ) {
		cerr << "Unable to create EGL surface (eglError: " << eglGetError() << ")" << endl;
		return 1;
	}

	//// egl-contexts collect all state descriptions needed required for operation
	EGLint ctxattr[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	egl_context = eglCreateContext ( egl_display, ecfg, EGL_NO_CONTEXT, ctxattr );
	if ( egl_context == EGL_NO_CONTEXT ) {
		cerr << "Unable to create EGL context (eglError: " << eglGetError() << ")" << endl;
		return 1;
	}

	//// associate the egl-context with the egl-surface
	eglMakeCurrent( egl_display, egl_surface, egl_surface, egl_context );

	///////  the openGL part  ///////////////////////////////////////////////////////////////

	GLuint vertexShader   = load_shader ( vertex_src , GL_VERTEX_SHADER  );     // load vertex shader
	GLuint fragmentShader = load_shader ( fragment_src , GL_FRAGMENT_SHADER );  // load fragment shader

	GLuint shaderProgram  = glCreateProgram ();                 // create program object
	glAttachShader ( shaderProgram, vertexShader );             // and attach both...
	glAttachShader ( shaderProgram, fragmentShader );           // ... shaders to it

	glLinkProgram ( shaderProgram );    // link the program
	glUseProgram  ( shaderProgram );    // and select it for usage

	//// now get the locations (kind of handle) of the shaders variables
	position_loc  = glGetAttribLocation  ( shaderProgram , "position" );
	phase_loc     = glGetUniformLocation ( shaderProgram , "phase"    );
	offset_loc    = glGetUniformLocation ( shaderProgram , "offset"   );
	if ( position_loc < 0  ||  phase_loc < 0  ||  offset_loc < 0 ) {
		cerr << "Unable to get uniform location" << endl;
		return 1;
	}

	const float window_width  = 800.0, window_height = 480.0;

	//// this is needed for time measuring  -->  frames per second
	struct  timezone  tz;
	timeval  t1, t2;
	gettimeofday ( &t1 , &tz );
	int  num_frames = 0;

	bool quit = false;
	while ( !quit ) {    // the main loop
	  render();   // now we finally put something on the screen

	  if ( ++num_frames % 100 == 0 ) {
		 gettimeofday( &t2, &tz );
		 float dt  =  t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6;
		 cout << "fps: " << num_frames / dt << endl;
		 num_frames = 0;
		 t1 = t2;
		}
	// usleep( 1000*10 );
	}

	////  cleaning up...
	eglDestroyContext ( egl_display, egl_context );
	eglDestroySurface ( egl_display, egl_surface );
	eglTerminate      ( egl_display );
	free(win); 
	return 0;
}
