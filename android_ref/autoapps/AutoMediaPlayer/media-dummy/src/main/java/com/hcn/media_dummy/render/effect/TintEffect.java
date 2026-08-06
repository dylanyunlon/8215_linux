package com.hcn.media_dummy.render.effect;

import android.graphics.Color;
import android.opengl.GLSurfaceView;

import com.hcn.media_dummy.render.view.FunVideoGLView;

/**
 * Tints the video with specified color..
 *
 * @author sheraz.khilji
 */
public class TintEffect implements FunVideoGLView.ShaderInterface {
    private int mTint = 0xFF0000FF;

    /**
     * Initialize Effect
     *
     * @param color Integer, representing an ARGB color with 8 bits per channel.
     *              May be created using Color class.
     */
    public TintEffect(int color) {
        this.mTint = color;

    }

    @Override
    public String getShader(GLSurfaceView mGlSurfaceView) {
        float[] colorRatio = {0.21f, 0.71f, 0.07f};
        String[] colorRatioString = new String[3];
        colorRatioString[0] = "color_ratio[0] = " + colorRatio[0] + ";\n";
        colorRatioString[1] = "color_ratio[1] = " + colorRatio[1] + ";\n";
        colorRatioString[2] = "color_ratio[2] = " + colorRatio[2] + ";\n";

        float[] tintColor = {Color.red(mTint) / 255f,
                Color.green(mTint) / 255f, Color.blue(mTint) / 255f};

        String[] tintString = new String[3];
        tintString[0] = "tint[0] = " + tintColor[0] + ";\n";
        tintString[1] = "tint[1] = " + tintColor[1] + ";\n";
        tintString[2] = "tint[2] = " + tintColor[2] + ";\n";

        return "#extension GL_OES_EGL_image_external : require\n"
                + "precision mediump float;\n"
                + "uniform samplerExternalOES sTexture;\n"
                + " vec3 tint;\n"
                + " vec3 color_ratio;\n"
                + "varying vec2 vTextureCoord;\n"
                + "void main() {\n"
                // Parameters that were created above
                + colorRatioString[0]
                + colorRatioString[1]
                + colorRatioString[2]
                + tintString[0]
                + tintString[1]
                + tintString[2]
                + "  vec4 color = texture2D(sTexture, vTextureCoord);\n"
                + "  float avg_color = dot(color_ratio, color.rgb);\n"
                + "  vec3 new_color = min(0.8 * avg_color + 0.2 * tint, 1.0);\n"
                + "  gl_FragColor = vec4(new_color.rgb, color.a);\n" + "}\n";
    }
}
