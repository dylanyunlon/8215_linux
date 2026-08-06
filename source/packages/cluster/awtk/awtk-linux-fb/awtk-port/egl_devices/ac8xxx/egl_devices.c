/**
 * File:   egl_devices.c
 * Author: AWTK Develop Team
 * Brief:  egl devices for fsl
 *
 * Copyright (c) 2020 - 2020  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2020-11-06 Lou ZhiMing <luozhiming@zlg.com> created
 *
 */

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <stdlib.h>
#include "../egl_devices.h"
//#include "tkc/mem.h"
#include <linux/fb.h>
#include <sys/ioctl.h>

typedef struct _fbdev_window_t {
    unsigned short width;
    unsigned short height;
} fbdev_window_t;

typedef struct _egl_devices_ac8xxx_context_t {
    EGLint               numconfigs;
    EGLDisplay           egldisplay;
    EGLConfig            eglconfig;
    EGLSurface           eglsurface;
    EGLContext           eglcontext;
    EGLNativeWindowType  eglNativeWindow;
    EGLNativeDisplayType eglNativeDisplayType;
} egl_devices_ac8xxx_context_t;

static const EGLint s_configAttribs[] =
{
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RED_SIZE,     8,
    EGL_GREEN_SIZE,   8,
    EGL_BLUE_SIZE,    8,
    EGL_ALPHA_SIZE,   8,
    EGL_STENCIL_SIZE, 8,
    EGL_DEPTH_SIZE,   0,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_NONE,
};

EGLNativeWindowType ac8xxx_createwindow(const char* filename)
{
    struct fb_var_screeninfo vinfo;
    int ret = 0;
    int fd = -1;
    EGLNativeWindowType native_window = (EGLNativeWindowType)0;
    fbdev_window_t* fb_window_ = malloc(sizeof(fbdev_window_t));
    memset(fb_window_, 0x00, sizeof(fbdev_window_t));
    return_value_if_fail(fb_window_ != NULL, NULL);

    fd = open(filename, O_RDWR, 0);
    return_value_if_fail(fd > 0, NULL);

    memset(&vinfo, 0, sizeof(struct fb_var_screeninfo));
    ret = ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);
    return_value_if_fail(ret == 0, NULL);
    close(fd);

    fb_window_->width = vinfo.xres;
    fb_window_->height = vinfo.yres;
    native_window = (EGLNativeWindowType)fb_window_;

    return native_window;
}

void* egl_devices_create(const char* filename) {
  egl_devices_ac8xxx_context_t* ctx = malloc(sizeof(egl_devices_ac8xxx_context_t));
  memset(ctx, 0x00, sizeof(egl_devices_ac8xxx_context_t));
  return_value_if_fail(ctx != NULL, NULL);

  ctx->eglNativeDisplayType = EGL_DEFAULT_DISPLAY;
  ctx->egldisplay = eglGetDisplay(ctx->eglNativeDisplayType);
  eglInitialize(ctx->egldisplay, NULL, NULL);
  assert(eglGetError() == EGL_SUCCESS);
  eglBindAPI(EGL_OPENGL_ES_API);

  eglChooseConfig(ctx->egldisplay, s_configAttribs, &(ctx->eglconfig), 1, &(ctx->numconfigs));
  assert(eglGetError() == EGL_SUCCESS);
  assert(ctx->numconfigs == 1);

  ctx->eglNativeWindow = ac8xxx_createwindow(filename);
  assert(ctx->eglNativeWindow);

  ctx->eglsurface = eglCreateWindowSurface(ctx->egldisplay, ctx->eglconfig, ctx->eglNativeWindow, NULL);
  assert(eglGetError() == EGL_SUCCESS);

  EGLint ContextAttribList[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  ctx->eglcontext = eglCreateContext(ctx->egldisplay, ctx->eglconfig, EGL_NO_CONTEXT, ContextAttribList );
  assert(eglGetError() == EGL_SUCCESS);

  eglMakeCurrent(ctx->egldisplay, ctx->eglsurface, ctx->eglsurface, ctx->eglcontext);
  assert(eglGetError() == EGL_SUCCESS);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_SCISSOR_TEST);

  return (void*)ctx;
}

ret_t egl_devices_dispose(void* ctx) {
  egl_devices_ac8xxx_context_t* context = (egl_devices_ac8xxx_context_t*)ctx;
  return_value_if_fail(context != NULL, RET_BAD_PARAMS);

  eglDestroySurface (context->egldisplay, context->eglsurface);
  assert(eglGetError() == EGL_SUCCESS);

  eglDestroyContext (context->egldisplay, context->eglcontext);
  assert(eglGetError() == EGL_SUCCESS);

  eglTerminate(context->egldisplay);
  assert(eglGetError() == EGL_SUCCESS);

  free(context->eglNativeWindow);
  free(context);

  return RET_OK;
}

float_t egl_devices_get_ratio(void* ctx) {
  (void)ctx;
  return 1.0f;
}

int32_t egl_devices_get_width(void* ctx) {
  EGLint width = 0;
  egl_devices_ac8xxx_context_t* context = (egl_devices_ac8xxx_context_t*)ctx;
  return_value_if_fail(context != NULL, RET_BAD_PARAMS);

  eglQuerySurface(context->egldisplay, context->eglsurface, EGL_WIDTH, &width);
  return (int32_t)width;
}

int32_t egl_devices_get_height(void* ctx) {
  EGLint height = 0;
  egl_devices_ac8xxx_context_t* context = (egl_devices_ac8xxx_context_t*)ctx;
  return_value_if_fail(context != NULL, RET_BAD_PARAMS);

  eglQuerySurface(context->egldisplay, context->eglsurface, EGL_HEIGHT, &height);
  return (int32_t)height;
}

ret_t egl_devices_make_current(void* ctx) {
  egl_devices_ac8xxx_context_t* context = (egl_devices_ac8xxx_context_t*)ctx;
  return_value_if_fail(context != NULL, RET_BAD_PARAMS);

  eglMakeCurrent(context->egldisplay, context->eglsurface, context->eglsurface, context->eglcontext);
  return eglGetError() == EGL_SUCCESS ? RET_OK : RET_FAIL;
}

ret_t egl_devices_swap_buffers(void* ctx) {
  egl_devices_ac8xxx_context_t* context = (egl_devices_ac8xxx_context_t*)ctx;
  return_value_if_fail(context != NULL, RET_BAD_PARAMS);

  eglSwapBuffers(context->egldisplay, context->eglsurface);
  //printf("[%s] swap buffers\n");
  return eglGetError() == EGL_SUCCESS ? RET_OK : RET_FAIL;
}

ret_t egl_devices_resize(void* ctx, uint32_t w, uint32_t h) {
  (void)ctx;
  (void)w;
  (void)h;
  return RET_NOT_IMPL;
}
