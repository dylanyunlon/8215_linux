package com.hcn.media_common.utils;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.PorterDuff.Mode;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Shader.TileMode;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.view.View;
import android.view.View.MeasureSpec;

/**
 * 位图工具
 *
 * @author 65821
 */
public class BitmapUtil {

    public static Bitmap createReflectedBitmap(Bitmap srcBitmap) {
        if (null == srcBitmap) {
            return null;
        }

        // The gap between the reflection bitmap and original bitmap.
        final int REFLECTION_GAP = 4;

        int srcWidth = srcBitmap.getWidth();
        int srcHeight = srcBitmap.getHeight();
        int reflectionWidth = srcBitmap.getWidth();
        int reflectionHeight = srcBitmap.getHeight() / 3;

        if (0 == srcWidth || srcHeight == 0) {
            return null;
        }

        // The matrix
        Matrix matrix = new Matrix();
        matrix.preScale(1, -1);

        try {
            // The reflection bitmap, width is same with original's, height
            // is half of original's.
            Bitmap reflectionBitmap = Bitmap.createBitmap(srcBitmap, 0, srcHeight / 2, srcWidth,
                    srcHeight / 2, matrix,
                    false);

            if (null == reflectionBitmap) {
                return null;
            }

            // Create the bitmap which contains original and reflection
            // bitmap.
            Bitmap bitmapWithReflection = Bitmap.createBitmap(reflectionWidth,
                    srcHeight + reflectionHeight
                            + REFLECTION_GAP, Config.ARGB_8888);

            if (null == bitmapWithReflection) {
                return null;
            }

            // Prepare the canvas to draw stuff.
            Canvas canvas = new Canvas(bitmapWithReflection);

            // Draw the original bitmap.
            canvas.drawBitmap(srcBitmap, 0, 0, null);

            // Draw the reflection bitmap.
            canvas.drawBitmap(reflectionBitmap, 0, srcHeight + REFLECTION_GAP, null);

            Paint paint = new Paint();
            paint.setAntiAlias(true);
            LinearGradient shader = new LinearGradient(0, srcHeight, 0,
                    bitmapWithReflection.getHeight()
                            + REFLECTION_GAP, 0x70FFFFFF, 0x00FFFFFF, TileMode.MIRROR);
            paint.setShader(shader);
            paint.setXfermode(new PorterDuffXfermode(android.graphics.PorterDuff.Mode.DST_IN));

            // Draw the linear shader.
            canvas.drawRect(0, srcHeight, srcWidth,
                    bitmapWithReflection.getHeight() + REFLECTION_GAP, paint);

            return bitmapWithReflection;
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }

    private static Bitmap createReflectedBitmapById(Context mContext, int resId) {
        @SuppressLint("UseCompatLoadingForDrawables")
        Drawable drawable = mContext.getResources().getDrawable(resId);
        if (drawable instanceof BitmapDrawable) {
            Bitmap bitmap = ((BitmapDrawable) drawable).getBitmap();
            return BitmapUtil.createReflectedBitmap(bitmap);
        }
        return null;
    }

    public static Bitmap drawableToBitmap(Drawable drawable) {
        if (null == drawable) {
            return null;
        }

        int width = drawable.getIntrinsicWidth();
        int height = drawable.getIntrinsicHeight();

        return drawableToBitmap(drawable, width, height);
    }

    public static Bitmap drawableToBitmap(Drawable drawable, int width, int height) {
        if (null == drawable || width <= 0 || height <= 0) {
            return null;
        }

        Config config =
                (drawable.getOpacity() != PixelFormat.OPAQUE) ? Config.ARGB_8888 : Config.RGB_565;

        Bitmap bitmap = null;

        try {
            bitmap = Bitmap.createBitmap(width, height, config);
            if (null != bitmap) {
                Canvas canvas = new Canvas(bitmap);
                drawable.setBounds(0, 0, width, height);
                drawable.draw(canvas);
            }
        } catch (OutOfMemoryError | Exception e) {
            e.printStackTrace();
        }

        return bitmap;
    }

    private static Bitmap alphaLayer(Bitmap bmp, Context mContext, int resId) {
        int width = bmp.getWidth();
        int height = bmp.getHeight();
        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565);

        Bitmap overlay = BitmapFactory.decodeResource(mContext.getResources(), resId);

        int w = overlay.getWidth();
        int h = overlay.getHeight();
        float scaleX = width * 1F / w;
        float scaleY = height * 1F / h;
        Matrix matrix = new Matrix();
        matrix.postScale(scaleX, scaleY);

        Bitmap overlayCopy = Bitmap.createBitmap(overlay, 0, 0, w, h, matrix, true);

        int pixColor = 0;
        int layColor = 0;
        int newColor = 0;

        int pixR = 0;
        int pixG = 0;
        int pixB = 0;
        int pixA = 0;

        int newR = 0;
        int newG = 0;
        int newB = 0;
        int newA = 0;

        int layR = 0;
        int layG = 0;
        int layB = 0;
        int layA = 0;

        float alpha = 0.3F;
        float alphaR = 0F;
        float alphaG = 0F;
        float alphaB = 0F;

        for (int i = 0; i < width; i++) {
            for (int k = 0; k < height; k++) {
                pixColor = bmp.getPixel(i, k);
                layColor = overlayCopy.getPixel(i, k);

                pixR = Color.red(pixColor);
                pixG = Color.green(pixColor);
                pixB = Color.blue(pixColor);
                pixA = Color.alpha(pixColor);

                layR = Color.red(layColor);
                layG = Color.green(layColor);
                layB = Color.blue(layColor);
                layA = Color.alpha(layColor);

                if (layR < 20 && layG < 20 && layB < 20) {
                    alpha = 1F;
                } else {
                    alpha = 0.3F;
                }

                alphaR = alpha;
                alphaG = alpha;
                alphaB = alpha;

                newR = (int) (pixR * alphaR + layR * (1 - alphaR));
                newG = (int) (pixG * alphaG + layG * (1 - alphaG));
                newB = (int) (pixB * alphaB + layB * (1 - alphaB));
                layA = (int) (pixA * alpha + layA * (1 - alpha));

                newR = Math.min(255, Math.max(0, newR));
                newG = Math.min(255, Math.max(0, newG));
                newB = Math.min(255, Math.max(0, newB));
                newA = Math.min(255, Math.max(0, layA));

                newColor = Color.argb(newA, newR, newG, newB);
                bitmap.setPixel(i, k, newColor);
            }
        }

        return bitmap;
    }

    public static Bitmap drawTextToBitmap(Context gContext, Bitmap bitmap, String gText) {
        Resources resources = gContext.getResources();
        float scale = resources.getDisplayMetrics().density;

        android.graphics.Bitmap.Config bitmapConfig = bitmap.getConfig();
        // set default bitmap config if none
        if (bitmapConfig == null) {
            bitmapConfig = android.graphics.Bitmap.Config.ARGB_8888;
        }
        // resource bitmaps are imutable,
        // so we need to convert it to mutable one
        bitmap = bitmap.copy(bitmapConfig, true);

        Canvas canvas = new Canvas(bitmap);
        // new antialised Paint
        Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        // text color - #3D3D3D
        paint.setColor(Color.rgb(255, 0, 0));
        // text size in pixels
        paint.setTextSize((int) (14 * scale));
        // text shadow
        paint.setShadowLayer(1f, 0f, 1f, Color.WHITE);

        // draw text to the Canvas center
        Rect bounds = new Rect();
        paint.getTextBounds(gText, 0, gText.length(), bounds);
        int x = (bitmap.getWidth() - bounds.width()) / 2;
        int y = (bitmap.getHeight() + bounds.height()) / 2;

        canvas.drawText(gText, x * scale, y * scale, paint);
        return bitmap;
    }

    public static Bitmap getViewBitmap(View convertView, int width, int height) {
        convertView.setDrawingCacheEnabled(true);
        convertView.measure(MeasureSpec.makeMeasureSpec(width, MeasureSpec.EXACTLY),
                MeasureSpec.makeMeasureSpec(height, MeasureSpec.EXACTLY));
        convertView.layout(0, 0, convertView.getMeasuredWidth(), convertView.getMeasuredHeight());
        convertView.buildDrawingCache();
        return convertView.getDrawingCache();
    }

    public static Bitmap getRoundedCornerBitmap(Bitmap bitmap) {
        Bitmap output = Bitmap.createBitmap(bitmap.getWidth(), bitmap.getHeight(),
                Config.ARGB_8888);
        Canvas canvas = new Canvas(output);

        final int color = Color.RED;
        final Paint paint = new Paint();

        final Rect rect = new Rect(0, 0, bitmap.getWidth(), bitmap.getHeight());
        final RectF rectF = new RectF(rect);
        final float roundPx = 50;

        paint.setAntiAlias(true);
        canvas.drawARGB(0, 0, 0, 0);
        paint.setColor(color);
        canvas.drawRoundRect(rectF, roundPx, roundPx, paint);
        paint.setXfermode(new PorterDuffXfermode(Mode.SRC_IN));
        canvas.drawBitmap(bitmap, rect, rect, paint);
        return output;
    }

    public static Bitmap getCroppedRoundBitmap(Bitmap bmp, int radius) {
        Bitmap scaledSrcBmp;
        int diameter = radius * 2;

        int bmpWidth = bmp.getWidth();
        int bmpHeight = bmp.getHeight();
        int squareWidth = 0, squareHeight = 0;
        int x = 0, y = 0;
        Bitmap squareBitmap;

        if (bmpHeight > bmpWidth) {
            squareWidth = bmpWidth;
            squareHeight = bmpWidth;
            y = (bmpHeight - bmpWidth) / 2;
            squareBitmap = Bitmap.createBitmap(bmp, x, y, squareWidth, squareHeight);
        } else if (bmpHeight < bmpWidth) {
            squareWidth = squareHeight = bmpHeight;
            x = (bmpWidth - bmpHeight) / 2;
            squareBitmap = Bitmap.createBitmap(bmp, x, y, squareWidth, squareHeight);
        } else {
            squareBitmap = bmp;
        }

        if (squareBitmap.getWidth() != diameter || squareBitmap.getHeight() != diameter) {
            scaledSrcBmp = Bitmap.createScaledBitmap(squareBitmap, diameter, diameter, true);
        } else {
            scaledSrcBmp = squareBitmap;
        }

        Bitmap output = Bitmap.createBitmap(scaledSrcBmp.getWidth(), scaledSrcBmp.getHeight(),
                Config.ARGB_8888);
        Canvas canvas = new Canvas(output);

        Paint paint = new Paint();
        Rect rect = new Rect(0, 0, scaledSrcBmp.getWidth(), scaledSrcBmp.getHeight());

        paint.setAntiAlias(true);
        paint.setFilterBitmap(true);
        paint.setDither(true);
        canvas.drawARGB(0, 0, 0, 0);
        canvas.drawCircle(scaledSrcBmp.getWidth()/2.0f,
                scaledSrcBmp.getHeight()/2.0f,
                scaledSrcBmp.getWidth()/2.0f,
                paint);
        paint.setXfermode(new PorterDuffXfermode(Mode.SRC_IN));
        canvas.drawBitmap(scaledSrcBmp, rect, rect, paint);

        // bmp.recycle();
        // squareBitmap.recycle();
        // scaledSrcBmp.recycle();

        bmp = null;
        squareBitmap = null;
        scaledSrcBmp = null;
        return output;
    }
}
