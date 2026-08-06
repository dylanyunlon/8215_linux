package com.autochips.bluetooth.skin;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.renderscript.ScriptIntrinsicBlur;
import android.util.Log;
import android.view.View;

/**
 * @author simon
 * @describe 图片毛玻璃处理
 * @date 2022/11/22 16:31
 */
public class FastBlurUtils {
    /**
     * 图片放大倍数
     */
    private static final float BITMAP_ZOOM_IN_SCALE = 4.0f;
    /**
     * 图片缩放倍数
     */
    private static final float BITMAP_ZOOM_OUT_SCALE = 0.25f;
    /**
     * 模糊程度参数值，模糊半径(radius)越大，性能要求越高，模糊半径不能超过25
     */
    private static final int BLUR_RADIUS = 7;

    public static Bitmap getBlurBackgroundDrawer(Activity activity) {
        long startMs = System.currentTimeMillis();
        Bitmap srcBitmap = takeScreenShot(activity);
        Bitmap desBitmap = startBlurBackground(activity, srcBitmap);
        Log.i("FastBlurUtility", "=====blur time===:" + (System.currentTimeMillis() - startMs));
        return desBitmap;
    }

    /**
     * 截图
     *
     * @param activity
     * @return
     */
    public static Bitmap takeScreenShot(Activity activity) {
        View view = activity.getWindow().getDecorView();
        view.setDrawingCacheEnabled(true);
        view.buildDrawingCache();
        Bitmap bitmap = view.getDrawingCache();
        // 获取屏幕长和高
        int width = activity.getResources().getDisplayMetrics().widthPixels;
        int height = activity.getResources().getDisplayMetrics().heightPixels;
        Bitmap bmp = Bitmap.createBitmap(bitmap, 0, 0, width, height);
        view.destroyDrawingCache();
        return bmp;
    }

    /**
     * 原始图片大小虚化，采用算法的方式
     *
     * @param bitmap
     * @return
     */
    public static Bitmap startBlurBitmap(Bitmap bitmap) {
        return fastBlurBitmap(bitmap, BLUR_RADIUS);
    }

    /**
     * 原始图片大小虚化，采用RenderScript方式
     *
     * @param context
     * @param bitmap
     * @return
     */

    public static Bitmap startBlurBitmap(Context context, Bitmap bitmap) {
        return gaussianBlur(context, bitmap, BLUR_RADIUS);
    }

    /**
     * 缩放图片大小虚化，采用算法的方式
     *
     * @param background
     * @return
     */
    public static Bitmap startBlurBackground(Bitmap background) {
        Bitmap smallBlurBitmap = fastBlurBitmap(zoomOut(background), BLUR_RADIUS);
        return zoomIn(smallBlurBitmap);
    }

    /**
     * 缩放图片大小虚化，采用RenderScript方式
     *
     * @param context
     * @param background
     * @return
     */
    public static Bitmap startBlurBackground(Context context, Bitmap background) {
        Bitmap smallBlurBitmap = gaussianBlur(context, zoomOut(background), BLUR_RADIUS);
        return zoomIn(smallBlurBitmap);
    }

    /**
     * 放大图片
     *
     * @param bitmap
     * @return
     */
    public static Bitmap zoomIn(Bitmap bitmap) {
        Matrix matrix = new Matrix();
        matrix.postScale(BITMAP_ZOOM_IN_SCALE, BITMAP_ZOOM_IN_SCALE);
        return Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
    }

    /**
     * 缩放图片
     *
     * @param bitmap
     * @return
     */
    public static Bitmap zoomOut(Bitmap bitmap) {
        Matrix matrix = new Matrix();
        matrix.postScale(BITMAP_ZOOM_OUT_SCALE, BITMAP_ZOOM_OUT_SCALE);
        return Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
    }

    /**
     * 高斯模糊图片，
     *
     * @param context
     * @param original
     * @param radius   模糊程度参数值，模糊半径(radius)越大，性能要求越高，模糊半径不能超过25
     * @return
     */
    public static Bitmap gaussianBlur(Context context, Bitmap original, int radius) {
        //api大于17才能用
        if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.JELLY_BEAN_MR1) {
            return null;
        }
        RenderScript renderScript = RenderScript.create(context);
        Allocation input = Allocation.createFromBitmap(renderScript, original);
        Allocation output = Allocation.createTyped(renderScript, input.getType());
        ScriptIntrinsicBlur scriptIntrinsicBlur = ScriptIntrinsicBlur.create(renderScript, Element.U8_4(renderScript));
        scriptIntrinsicBlur.setRadius(radius);
        scriptIntrinsicBlur.setInput(input);
        scriptIntrinsicBlur.forEach(output);
        output.copyTo(original);
        return original;
    }

    /**
     * 对图片进行高斯模糊算法
     *
     * @param sentBitmap
     * @param radius     //模糊程度参数值，模糊半径(radius)越大，性能要求越高，模糊半径不能超过25
     * @return
     */
    public static Bitmap fastBlurBitmap(Bitmap sentBitmap, int radius) {
        Bitmap bitmap = sentBitmap.copy(sentBitmap.getConfig(), true);

        if (radius < 1) {
            return (null);
        }

        int w = bitmap.getWidth();
        int h = bitmap.getHeight();

        int[] pix = new int[w * h];
        bitmap.getPixels(pix, 0, w, 0, 0, w, h);

        int wm = w - 1;
        int hm = h - 1;
        int wh = w * h;
        int div = radius + radius + 1;

        int r[] = new int[wh];
        int g[] = new int[wh];
        int b[] = new int[wh];
        int rsum, gsum, bsum, x, y, i, p, yp, yi, yw;
        int vmin[] = new int[Math.max(w, h)];

        int divsum = (div + 1) >> 1;
        divsum *= divsum;
        int dv[] = new int[256 * divsum];
        for (i = 0; i < 256 * divsum; i++) {
            dv[i] = (i / divsum);
        }

        yw = yi = 0;

        int[][] stack = new int[div][3];
        int stackpointer;
        int stackstart;
        int[] sir;
        int rbs;
        int r1 = radius + 1;
        int routsum, goutsum, boutsum;
        int rinsum, ginsum, binsum;

        for (y = 0; y < h; y++) {
            rinsum = ginsum = binsum = routsum = goutsum = boutsum = rsum = gsum = bsum = 0;
            for (i = -radius; i <= radius; i++) {
                p = pix[yi + Math.min(wm, Math.max(i, 0))];
                sir = stack[i + radius];
                sir[0] = (p & 0xff0000) >> 16;
                sir[1] = (p & 0x00ff00) >> 8;
                sir[2] = (p & 0x0000ff);
                rbs = r1 - Math.abs(i);
                rsum += sir[0] * rbs;
                gsum += sir[1] * rbs;
                bsum += sir[2] * rbs;
                if (i > 0) {
                    rinsum += sir[0];
                    ginsum += sir[1];
                    binsum += sir[2];
                } else {
                    routsum += sir[0];
                    goutsum += sir[1];
                    boutsum += sir[2];
                }
            }
            stackpointer = radius;

            for (x = 0; x < w; x++) {

                r[yi] = dv[rsum];
                g[yi] = dv[gsum];
                b[yi] = dv[bsum];

                rsum -= routsum;
                gsum -= goutsum;
                bsum -= boutsum;

                stackstart = stackpointer - radius + div;
                sir = stack[stackstart % div];

                routsum -= sir[0];
                goutsum -= sir[1];
                boutsum -= sir[2];

                if (y == 0) {
                    vmin[x] = Math.min(x + radius + 1, wm);
                }
                p = pix[yw + vmin[x]];

                sir[0] = (p & 0xff0000) >> 16;
                sir[1] = (p & 0x00ff00) >> 8;
                sir[2] = (p & 0x0000ff);

                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];

                rsum += rinsum;
                gsum += ginsum;
                bsum += binsum;

                stackpointer = (stackpointer + 1) % div;
                sir = stack[(stackpointer) % div];

                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];

                rinsum -= sir[0];
                ginsum -= sir[1];
                binsum -= sir[2];

                yi++;
            }
            yw += w;
        }
        for (x = 0; x < w; x++) {
            rinsum = ginsum = binsum = routsum = goutsum = boutsum = rsum = gsum = bsum = 0;
            yp = -radius * w;
            for (i = -radius; i <= radius; i++) {
                yi = Math.max(0, yp) + x;

                sir = stack[i + radius];

                sir[0] = r[yi];
                sir[1] = g[yi];
                sir[2] = b[yi];

                rbs = r1 - Math.abs(i);

                rsum += r[yi] * rbs;
                gsum += g[yi] * rbs;
                bsum += b[yi] * rbs;

                if (i > 0) {
                    rinsum += sir[0];
                    ginsum += sir[1];
                    binsum += sir[2];
                } else {
                    routsum += sir[0];
                    goutsum += sir[1];
                    boutsum += sir[2];
                }

                if (i < hm) {
                    yp += w;
                }
            }
            yi = x;
            stackpointer = radius;
            for (y = 0; y < h; y++) {
                pix[yi] = (0xff000000 & pix[yi]) | (dv[rsum] << 16) | (dv[gsum] << 8) | dv[bsum];

                rsum -= routsum;
                gsum -= goutsum;
                bsum -= boutsum;

                stackstart = stackpointer - radius + div;
                sir = stack[stackstart % div];

                routsum -= sir[0];
                goutsum -= sir[1];
                boutsum -= sir[2];

                if (x == 0) {
                    vmin[y] = Math.min(y + r1, hm) * w;
                }
                p = x + vmin[y];

                sir[0] = r[p];
                sir[1] = g[p];
                sir[2] = b[p];

                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];

                rsum += rinsum;
                gsum += ginsum;
                bsum += binsum;

                stackpointer = (stackpointer + 1) % div;
                sir = stack[stackpointer];

                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];

                rinsum -= sir[0];
                ginsum -= sir[1];
                binsum -= sir[2];

                yi += w;
            }
        }

        bitmap.setPixels(pix, 0, w, 0, 0, w, h);

        return (bitmap);
    }
}