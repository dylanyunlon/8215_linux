package com.hcn.media_dummy.render.effect;

import android.opengl.GLSurfaceView;

import com.hcn.media_dummy.render.view.FunVideoGLView;

/**
 * 显示没有任何效果的普通视频。
 * <p> 默认显示，不带任何效果；
 *
 * @author sheraz.khilji
 */
public class NoEffect implements FunVideoGLView.ShaderInterface {

    /**
     * Initialize
     */
    public NoEffect() {
    }

    @Override
    public String getShader(GLSurfaceView glSurfaceView) {
        String shader = "#extension GL_OES_EGL_image_external : require\n"
                + "precision mediump float;\n"
                + "varying vec2 vTextureCoord;\n"
                + "uniform samplerExternalOES sTexture;\n" + "void main() {\n"
                + "  gl_FragColor = texture2D(sTexture, vTextureCoord);\n"
                + "}\n";

        return shader;
    }
}
