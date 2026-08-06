package com.hcn.auto.utils;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapShader;
import android.graphics.Canvas;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.LinearGradient;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffColorFilter;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.media.ExifInterface;
import android.os.Build;
import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.renderscript.ScriptIntrinsicBlur;

import android.view.View;

import androidx.annotation.ColorInt;
import androidx.annotation.DrawableRes;
import androidx.annotation.FloatRange;
import androidx.annotation.IntRange;
import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.core.content.ContextCompat;

import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public final class HImageUtils {
    private static final char[] hexDigits = new char[]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    private HImageUtils() {
        throw new UnsupportedOperationException("u can't instantiate me...");
    }

    public static byte[] bitmap2Bytes(Bitmap bitmap, Bitmap.CompressFormat format) {
        if (bitmap == null) {
            return null;
        } else {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            bitmap.compress(format, 100, baos);
            return baos.toByteArray();
        }
    }

    public static Bitmap bytes2Bitmap(byte[] bytes) {
        return bytes != null && bytes.length != 0 ? BitmapFactory.decodeByteArray(bytes, 0, bytes.length) : null;
    }

    public static Bitmap drawable2Bitmap(Drawable drawable) {
        if (drawable instanceof BitmapDrawable) {
            BitmapDrawable bitmapDrawable = (BitmapDrawable)drawable;
            if (bitmapDrawable.getBitmap() != null) {
                return bitmapDrawable.getBitmap();
            }
        }

        Bitmap bitmap;
        if (drawable.getIntrinsicWidth() > 0 && drawable.getIntrinsicHeight() > 0) {
            bitmap = Bitmap.createBitmap(drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight(), drawable.getOpacity() != PixelFormat.OPAQUE ? Bitmap.Config.ARGB_8888 : Bitmap.Config.RGB_565);
        } else {
            bitmap = Bitmap.createBitmap(1, 1, drawable.getOpacity() != PixelFormat.OPAQUE ? Bitmap.Config.ARGB_8888 : Bitmap.Config.RGB_565);
        }

        Canvas canvas = new Canvas(bitmap);
        drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
        drawable.draw(canvas);
        return bitmap;
    }

    public static Drawable bitmap2Drawable(Bitmap bitmap) {
        return bitmap == null ? null : new BitmapDrawable(HUtilsEx.getApp().getResources(), bitmap);
    }

    public static byte[] drawable2Bytes(Drawable drawable, Bitmap.CompressFormat format) {
        return drawable == null ? null : bitmap2Bytes(drawable2Bitmap(drawable), format);
    }

    public static Drawable bytes2Drawable(byte[] bytes) {
        return bitmap2Drawable(bytes2Bitmap(bytes));
    }

    public static Bitmap view2Bitmap(View view) {
        if (view == null) {
            return null;
        } else {
            boolean drawingCacheEnabled = view.isDrawingCacheEnabled();
            boolean willNotCacheDrawing = view.willNotCacheDrawing();
            view.setDrawingCacheEnabled(true);
            view.setWillNotCacheDrawing(false);
            Bitmap drawingCache = view.getDrawingCache();
            Bitmap bitmap;
            if (null == drawingCache) {
                view.measure(View.MeasureSpec.makeMeasureSpec(View.MeasureSpec.UNSPECIFIED, View.MeasureSpec.UNSPECIFIED), View.MeasureSpec.makeMeasureSpec(View.MeasureSpec.UNSPECIFIED, View.MeasureSpec.UNSPECIFIED));
                view.layout(0, 0, view.getMeasuredWidth(), view.getMeasuredHeight());
                view.buildDrawingCache();
                bitmap = Bitmap.createBitmap(view.getDrawingCache());
            } else {
                bitmap = Bitmap.createBitmap(drawingCache);
            }

            view.destroyDrawingCache();
            view.setWillNotCacheDrawing(willNotCacheDrawing);
            view.setDrawingCacheEnabled(drawingCacheEnabled);
            return bitmap;
        }
    }

    public static Bitmap getBitmap(File file) {
        return file == null ? null : BitmapFactory.decodeFile(file.getAbsolutePath());
    }

    public static Bitmap getBitmap(File file, int maxWidth, int maxHeight) {
        if (file == null) {
            return null;
        } else {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeFile(file.getAbsolutePath(), options);
            options.inSampleSize = calculateInSampleSize(options, maxWidth, maxHeight);
            options.inJustDecodeBounds = false;
            return BitmapFactory.decodeFile(file.getAbsolutePath(), options);
        }
    }

    public static Bitmap getBitmap(String filePath) {
        return isSpace(filePath) ? null : BitmapFactory.decodeFile(filePath);
    }

    public static Bitmap getBitmap(String filePath, int maxWidth, int maxHeight) {
        if (isSpace(filePath)) {
            return null;
        } else {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeFile(filePath, options);
            options.inSampleSize = calculateInSampleSize(options, maxWidth, maxHeight);
            options.inJustDecodeBounds = false;
            return BitmapFactory.decodeFile(filePath, options);
        }
    }

    public static Bitmap getBitmap(InputStream is) {
        return is == null ? null : BitmapFactory.decodeStream(is);
    }

    public static Bitmap getBitmap(InputStream is, int maxWidth, int maxHeight) {
        if (is == null) {
            return null;
        } else {
            byte[] bytes = input2Byte(is);
            return getBitmap(bytes, 0, maxWidth, maxHeight);
        }
    }

    public static Bitmap getBitmap(byte[] data, int offset) {
        return data.length == 0 ? null : BitmapFactory.decodeByteArray(data, offset, data.length);
    }

    public static Bitmap getBitmap(byte[] data, int offset, int maxWidth, int maxHeight) {
        if (data.length == 0) {
            return null;
        } else {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeByteArray(data, offset, data.length, options);
            options.inSampleSize = calculateInSampleSize(options, maxWidth, maxHeight);
            options.inJustDecodeBounds = false;
            return BitmapFactory.decodeByteArray(data, offset, data.length, options);
        }
    }

    public static Bitmap getBitmap(@DrawableRes int resId) {
        Drawable drawable = ContextCompat.getDrawable(HUtilsEx.getApp(), resId);
        Canvas canvas = new Canvas();
        Bitmap bitmap = Bitmap.createBitmap(drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight(), Bitmap.Config.ARGB_8888);
        canvas.setBitmap(bitmap);
        drawable.setBounds(0, 0, drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight());
        drawable.draw(canvas);
        return bitmap;
    }

    public static Bitmap getBitmap(@DrawableRes int resId, int maxWidth, int maxHeight) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        Resources resources = HUtilsEx.getApp().getResources();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeResource(resources, resId, options);
        options.inSampleSize = calculateInSampleSize(options, maxWidth, maxHeight);
        options.inJustDecodeBounds = false;
        return BitmapFactory.decodeResource(resources, resId, options);
    }

    public static Bitmap getBitmap(FileDescriptor fd) {
        return fd == null ? null : BitmapFactory.decodeFileDescriptor(fd);
    }

    public static Bitmap getBitmap(FileDescriptor fd, int maxWidth, int maxHeight) {
        if (fd == null) {
            return null;
        } else {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeFileDescriptor(fd, (Rect)null, options);
            options.inSampleSize = calculateInSampleSize(options, maxWidth, maxHeight);
            options.inJustDecodeBounds = false;
            return BitmapFactory.decodeFileDescriptor(fd, (Rect)null, options);
        }
    }

    public static Bitmap drawColor(@NonNull Bitmap src, @ColorInt int color) {
        return drawColor(src, color, false);
    }

    public static Bitmap drawColor(@NonNull Bitmap src, @ColorInt int color, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            Bitmap ret = recycle ? src : src.copy(src.getConfig(), true);
            Canvas canvas = new Canvas(ret);
            canvas.drawColor(color, PorterDuff.Mode.DARKEN);
            return ret;
        }
    }

    public static Bitmap scale(Bitmap src, int newWidth, int newHeight) {
        return scale(src, newWidth, newHeight, false);
    }

    public static Bitmap scale(Bitmap src, int newWidth, int newHeight, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            Bitmap ret = Bitmap.createScaledBitmap(src, newWidth, newHeight, true);
            if (recycle && !src.isRecycled() && ret != src) {
                src.recycle();
            }

            return ret;
        }
    }

    public static Bitmap scale(Bitmap src, float scaleWidth, float scaleHeight) {
        return scale(src, scaleWidth, scaleHeight, false);
    }

    public static Bitmap scale(Bitmap src, float scaleWidth, float scaleHeight, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            Matrix matrix = new Matrix();
            matrix.setScale(scaleWidth, scaleHeight);
            Bitmap ret = Bitmap.createBitmap(src, 0, 0, src.getWidth(), src.getHeight(), matrix, true);
            if (recycle && !src.isRecycled() && ret != src) {
                src.recycle();
            }

            return ret;
        }
    }

    public static Bitmap clip(Bitmap src, int x, int y, int width, int height) {
        return clip(src, x, y, width, height, false);
    }

    public static Bitmap clip(Bitmap src, int x, int y, int width, int height, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            Bitmap ret = Bitmap.createBitmap(src, x, y, width, height);
            if (recycle && !src.isRecycled() && ret != src) {
                src.recycle();
            }

            return ret;
        }
    }

    public static Bitmap skew(Bitmap src, float kx, float ky) {
        return skew(src, kx, ky, 0.0F, 0.0F, false);
    }

    public static Bitmap skew(Bitmap src, float kx, float ky, boolean recycle) {
        return skew(src, kx, ky, 0.0F, 0.0F, recycle);
    }

    public static Bitmap skew(Bitmap src, float kx, float ky, float px, float py) {
        return skew(src, kx, ky, px, py, false);
    }

    public static Bitmap skew(Bitmap src, float kx, float ky, float px, float py, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            Matrix matrix = new Matrix();
            matrix.setSkew(kx, ky, px, py);
            Bitmap ret = Bitmap.createBitmap(src, 0, 0, src.getWidth(), src.getHeight(), matrix, true);
            if (recycle && !src.isRecycled() && ret != src) {
                src.recycle();
            }

            return ret;
        }
    }

    public static Bitmap rotate(Bitmap src, int degrees, float px, float py) {
        return rotate(src, degrees, px, py, false);
    }

    public static Bitmap rotate(Bitmap src, int degrees, float px, float py, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else if (degrees == 0) {
            return src;
        } else {
            Matrix matrix = new Matrix();
            matrix.setRotate((float)degrees, px, py);
            Bitmap ret = Bitmap.createBitmap(src, 0, 0, src.getWidth(), src.getHeight(), matrix, true);
            if (recycle && !src.isRecycled() && ret != src) {
                src.recycle();
            }

            return ret;
        }
    }

    public static int getRotateDegree(String filePath) {
        try {
            ExifInterface exifInterface = new ExifInterface(filePath);
            int orientation = exifInterface.getAttributeInt("Orientation", 1);
            switch (orientation) {
                case 3:
                    return 180;
                case 6:
                    return 90;
                case 8:
                    return 270;
                default:
                    return 0;
            }
        } catch (IOException var3) {
            var3.printStackTrace();
            return -1;
        }
    }

    public static Bitmap toRound(Bitmap src) {
        return toRound(src, 0, 0, false);
    }

    public static Bitmap toRound(Bitmap src, boolean recycle) {
        return toRound(src, 0, 0, recycle);
    }

    public static Bitmap toRound(Bitmap src, @IntRange(from = 0L) int borderSize, @ColorInt int borderColor) {
        return toRound(src, borderSize, borderColor, false);
    }

    public static Bitmap toRound(Bitmap src, @IntRange(from = 0L) int borderSize, @ColorInt int borderColor, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            int width = src.getWidth();
            int height = src.getHeight();
            int size = Math.min(width, height);
            Paint paint = new Paint(1);
            Bitmap ret = Bitmap.createBitmap(width, height, src.getConfig());
            float center = (float)size / 2.0F;
            RectF rectF = new RectF(0.0F, 0.0F, (float)width, (float)height);
            rectF.inset((float)(width - size) / 2.0F, (float)(height - size) / 2.0F);
            Matrix matrix = new Matrix();
            matrix.setTranslate(rectF.left, rectF.top);
            if (width != height) {
                matrix.preScale((float)size / (float)width, (float)size / (float)height);
            }

            BitmapShader shader = new BitmapShader(src, Shader.TileMode.CLAMP, Shader.TileMode.CLAMP);
            shader.setLocalMatrix(matrix);
            paint.setShader(shader);
            Canvas canvas = new Canvas(ret);
            canvas.drawRoundRect(rectF, center, center, paint);
            if (borderSize > 0) {
                paint.setShader((Shader)null);
                paint.setColor(borderColor);
                paint.setStyle(Paint.Style.STROKE);
                paint.setStrokeWidth((float)borderSize);
                float radius = center - (float)borderSize / 2.0F;
                canvas.drawCircle((float)width / 2.0F, (float)height / 2.0F, radius, paint);
            }

            if (recycle && !src.isRecycled() && ret != src) {
                src.recycle();
            }

            return ret;
        }
    }

    public static Bitmap toRoundCorner(Bitmap src, float radius) {
        return toRoundCorner(src, radius, 0, 0, false);
    }

    public static Bitmap toRoundCorner(Bitmap src, float radius, boolean recycle) {
        return toRoundCorner(src, radius, 0, 0, recycle);
    }

    public static Bitmap toRoundCorner(Bitmap src, float radius, @IntRange(from = 0L) int borderSize, @ColorInt int borderColor) {
        return toRoundCorner(src, radius, borderSize, borderColor, false);
    }

    public static Bitmap toRoundCorner(Bitmap src, float radius, @IntRange(from = 0L) int borderSize, @ColorInt int borderColor, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            int width = src.getWidth();
            int height = src.getHeight();
            Paint paint = new Paint(1);
            Bitmap ret = Bitmap.createBitmap(width, height, src.getConfig());
            BitmapShader shader = new BitmapShader(src, Shader.TileMode.CLAMP, Shader.TileMode.CLAMP);
            paint.setShader(shader);
            Canvas canvas = new Canvas(ret);
            RectF rectF = new RectF(0.0F, 0.0F, (float)width, (float)height);
            float halfBorderSize = (float)borderSize / 2.0F;
            rectF.inset(halfBorderSize, halfBorderSize);
            canvas.drawRoundRect(rectF, radius, radius, paint);
            if (borderSize > 0) {
                paint.setShader((Shader)null);
                paint.setColor(borderColor);
                paint.setStyle(Paint.Style.STROKE);
                paint.setStrokeWidth((float)borderSize);
                paint.setStrokeCap(Paint.Cap.ROUND);
                canvas.drawRoundRect(rectF, radius, radius, paint);
            }

            if (recycle && !src.isRecycled() && ret != src) {
                src.recycle();
            }

            return ret;
        }
    }

    public static Bitmap addCornerBorder(Bitmap src, @IntRange(from = 1L) int borderSize, @ColorInt int color, @FloatRange(from = 0.0) float cornerRadius) {
        return addBorder(src, borderSize, color, false, cornerRadius, false);
    }

    public static Bitmap addCornerBorder(Bitmap src, @IntRange(from = 1L) int borderSize, @ColorInt int color, @FloatRange(from = 0.0) float cornerRadius, boolean recycle) {
        return addBorder(src, borderSize, color, false, cornerRadius, recycle);
    }

    public static Bitmap addCircleBorder(Bitmap src, @IntRange(from = 1L) int borderSize, @ColorInt int color) {
        return addBorder(src, borderSize, color, true, 0.0F, false);
    }

    public static Bitmap addCircleBorder(Bitmap src, @IntRange(from = 1L) int borderSize, @ColorInt int color, boolean recycle) {
        return addBorder(src, borderSize, color, true, 0.0F, recycle);
    }

    private static Bitmap addBorder(Bitmap src, @IntRange(from = 1L) int borderSize, @ColorInt int color, boolean isCircle, float cornerRadius, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            Bitmap ret = recycle ? src : src.copy(src.getConfig(), true);
            int width = ret.getWidth();
            int height = ret.getHeight();
            Canvas canvas = new Canvas(ret);
            Paint paint = new Paint(1);
            paint.setColor(color);
            paint.setStyle(Paint.Style.STROKE);
            paint.setStrokeWidth((float)borderSize);
            if (isCircle) {
                float radius = (float)Math.min(width, height) / 2.0F - (float)borderSize / 2.0F;
                canvas.drawCircle((float)width / 2.0F, (float)height / 2.0F, radius, paint);
            } else {
                int halfBorderSize = borderSize >> 1;
                RectF rectF = new RectF((float)halfBorderSize, (float)halfBorderSize, (float)(width - halfBorderSize), (float)(height - halfBorderSize));
                canvas.drawRoundRect(rectF, cornerRadius, cornerRadius, paint);
            }

            return ret;
        }
    }

    public static Bitmap addReflection(Bitmap src, int reflectionHeight) {
        return addReflection(src, reflectionHeight, false);
    }

    public static Bitmap addReflection(Bitmap src, int reflectionHeight, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            boolean REFLECTION_GAP = false;
            int srcWidth = src.getWidth();
            int srcHeight = src.getHeight();
            Matrix matrix = new Matrix();
            matrix.preScale(1.0F, -1.0F);
            Bitmap reflectionBitmap = Bitmap.createBitmap(src, 0, srcHeight - reflectionHeight, srcWidth, reflectionHeight, matrix, false);
            Bitmap ret = Bitmap.createBitmap(srcWidth, srcHeight + reflectionHeight, src.getConfig());
            Canvas canvas = new Canvas(ret);
            canvas.drawBitmap(src, 0.0F, 0.0F, (Paint)null);
            canvas.drawBitmap(reflectionBitmap, 0.0F, (float)srcHeight, (Paint)null);
            Paint paint = new Paint(1);
            LinearGradient shader = new LinearGradient(0.0F, (float)srcHeight, 0.0F, (float)ret.getHeight(), 1895825407, 16777215, Shader.TileMode.MIRROR);
            paint.setShader(shader);
            paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.DST_IN));
            canvas.drawRect(0.0F, (float)srcHeight, (float)srcWidth, (float)ret.getHeight(), paint);
            if (!reflectionBitmap.isRecycled()) {
                reflectionBitmap.recycle();
            }

            if (recycle && !src.isRecycled() && ret != src) {
                src.recycle();
            }

            return ret;
        }
    }

    public static Bitmap addTextWatermark(Bitmap src, String content, int textSize, @ColorInt int color, float x, float y) {
        return addTextWatermark(src, content, (float)textSize, color, x, y, false);
    }

    public static Bitmap addTextWatermark(Bitmap src, String content, float textSize, @ColorInt int color, float x, float y, boolean recycle) {
        if (!isEmptyBitmap(src) && content != null) {
            Bitmap ret = src.copy(src.getConfig(), true);
            Paint paint = new Paint(1);
            Canvas canvas = new Canvas(ret);
            paint.setColor(color);
            paint.setTextSize(textSize);
            Rect bounds = new Rect();
            paint.getTextBounds(content, 0, content.length(), bounds);
            canvas.drawText(content, x, y + textSize, paint);
            if (recycle && !src.isRecycled() && ret != src) {
                src.recycle();
            }

            return ret;
        } else {
            return null;
        }
    }

    public static Bitmap addImageWatermark(Bitmap src, Bitmap watermark, int x, int y, int alpha) {
        return addImageWatermark(src, watermark, x, y, alpha, false);
    }

    public static Bitmap addImageWatermark(Bitmap src, Bitmap watermark, int x, int y, int alpha, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            Bitmap ret = src.copy(src.getConfig(), true);
            if (!isEmptyBitmap(watermark)) {
                Paint paint = new Paint(1);
                Canvas canvas = new Canvas(ret);
                paint.setAlpha(alpha);
                canvas.drawBitmap(watermark, (float)x, (float)y, paint);
            }

            if (recycle && !src.isRecycled() && ret != src) {
                src.recycle();
            }

            return ret;
        }
    }

    public static Bitmap toAlpha(Bitmap src) {
        return toAlpha(src, false);
    }

    public static Bitmap toAlpha(Bitmap src, Boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            Bitmap ret = src.extractAlpha();
            if (recycle && !src.isRecycled() && ret != src) {
                src.recycle();
            }

            return ret;
        }
    }

    public static Bitmap toGray(Bitmap src) {
        return toGray(src, false);
    }

    public static Bitmap toGray(Bitmap src, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            Bitmap ret = Bitmap.createBitmap(src.getWidth(), src.getHeight(), src.getConfig());
            Canvas canvas = new Canvas(ret);
            Paint paint = new Paint();
            ColorMatrix colorMatrix = new ColorMatrix();
            colorMatrix.setSaturation(0.0F);
            ColorMatrixColorFilter colorMatrixColorFilter = new ColorMatrixColorFilter(colorMatrix);
            paint.setColorFilter(colorMatrixColorFilter);
            canvas.drawBitmap(src, 0.0F, 0.0F, paint);
            if (recycle && !src.isRecycled() && ret != src) {
                src.recycle();
            }

            return ret;
        }
    }

    public static Bitmap fastBlur(Bitmap src, @FloatRange(from = 0.0,to = 1.0,fromInclusive = false) float scale, @FloatRange(from = 0.0,to = 25.0,fromInclusive = false) float radius) {
        return fastBlur(src, scale, radius, false, false);
    }

    public static Bitmap fastBlur(Bitmap src, @FloatRange(from = 0.0,to = 1.0,fromInclusive = false) float scale, @FloatRange(from = 0.0,to = 25.0,fromInclusive = false) float radius, boolean recycle) {
        return fastBlur(src, scale, radius, recycle, false);
    }

    public static Bitmap fastBlur(Bitmap src, @FloatRange(from = 0.0,to = 1.0,fromInclusive = false) float scale, @FloatRange(from = 0.0,to = 25.0,fromInclusive = false) float radius, boolean recycle, boolean isReturnScale) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            int width = src.getWidth();
            int height = src.getHeight();
            Matrix matrix = new Matrix();
            matrix.setScale(scale, scale);
            Bitmap scaleBitmap = Bitmap.createBitmap(src, 0, 0, src.getWidth(), src.getHeight(), matrix, true);
            Paint paint = new Paint(3);
            Canvas canvas = new Canvas();
            PorterDuffColorFilter filter = new PorterDuffColorFilter(0, PorterDuff.Mode.SRC_ATOP);
            paint.setColorFilter(filter);
            canvas.scale(scale, scale);
            canvas.drawBitmap(scaleBitmap, 0.0F, 0.0F, paint);
            if (Build.VERSION.SDK_INT >= 17) {
                scaleBitmap = renderScriptBlur(scaleBitmap, radius, recycle);
            } else {
                scaleBitmap = stackBlur(scaleBitmap, (int)radius, recycle);
            }

            if (scale != 1.0F && !isReturnScale) {
                Bitmap ret = Bitmap.createScaledBitmap(scaleBitmap, width, height, true);
                if (!scaleBitmap.isRecycled()) {
                    scaleBitmap.recycle();
                }

                if (recycle && !src.isRecycled() && ret != src) {
                    src.recycle();
                }

                return ret;
            } else {
                if (recycle && !src.isRecycled() && scaleBitmap != src) {
                    src.recycle();
                }

                return scaleBitmap;
            }
        }
    }

    @RequiresApi(17)
    public static Bitmap renderScriptBlur(Bitmap src, @FloatRange(from = 0.0,to = 25.0,fromInclusive = false) float radius) {
        return renderScriptBlur(src, radius, false);
    }

    @RequiresApi(17)
    public static Bitmap renderScriptBlur(Bitmap src, @FloatRange(from = 0.0,to = 25.0,fromInclusive = false) float radius, boolean recycle) {
        RenderScript rs = null;
        Bitmap ret = recycle ? src : src.copy(src.getConfig(), true);

        try {
            rs = RenderScript.create(HUtilsEx.getApp());
            rs.setMessageHandler(new RenderScript.RSMessageHandler());
            Allocation input = Allocation.createFromBitmap(rs, ret, Allocation.MipmapControl.MIPMAP_NONE, 1);
            Allocation output = Allocation.createTyped(rs, input.getType());
            ScriptIntrinsicBlur blurScript = ScriptIntrinsicBlur.create(rs, Element.U8_4(rs));
            blurScript.setInput(input);
            blurScript.setRadius(radius);
            blurScript.forEach(output);
            output.copyTo(ret);
        } finally {
            if (rs != null) {
                rs.destroy();
            }

        }

        return ret;
    }

    public static Bitmap stackBlur(Bitmap src, int radius) {
        return stackBlur(src, radius, false);
    }

    public static Bitmap stackBlur(Bitmap src, int radius, boolean recycle) {
        Bitmap ret = recycle ? src : src.copy(src.getConfig(), true);
        if (radius < 1) {
            radius = 1;
        }

        int w = ret.getWidth();
        int h = ret.getHeight();
        int[] pix = new int[w * h];
        ret.getPixels(pix, 0, w, 0, 0, w, h);
        int wm = w - 1;
        int hm = h - 1;
        int wh = w * h;
        int div = radius + radius + 1;
        int[] r = new int[wh];
        int[] g = new int[wh];
        int[] b = new int[wh];
        int[] vmin = new int[Math.max(w, h)];
        int divsum = div + 1 >> 1;
        divsum *= divsum;
        int[] dv = new int[256 * divsum];

        int i;
        for(i = 0; i < 256 * divsum; ++i) {
            dv[i] = i / divsum;
        }

        int yi = 0;
        int yw = 0;
        int[][] stack = new int[div][3];
        int r1 = radius + 1;

        int rsum;
        int gsum;
        int bsum;
        int x;
        int y;
        int p;
        int stackpointer;
        int stackstart;
        int[] sir;
        int rbs;
        int routsum;
        int goutsum;
        int boutsum;
        int rinsum;
        int ginsum;
        int binsum;
        for(y = 0; y < h; ++y) {
            bsum = 0;
            gsum = 0;
            rsum = 0;
            boutsum = 0;
            goutsum = 0;
            routsum = 0;
            binsum = 0;
            ginsum = 0;
            rinsum = 0;

            for(i = -radius; i <= radius; ++i) {
                p = pix[yi + Math.min(wm, Math.max(i, 0))];
                sir = stack[i + radius];
                sir[0] = (p & 16711680) >> 16;
                sir[1] = (p & '\uff00') >> 8;
                sir[2] = p & 255;
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

            for(x = 0; x < w; ++x) {
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
                sir[0] = (p & 16711680) >> 16;
                sir[1] = (p & '\uff00') >> 8;
                sir[2] = p & 255;
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
                rsum += rinsum;
                gsum += ginsum;
                bsum += binsum;
                stackpointer = (stackpointer + 1) % div;
                sir = stack[stackpointer % div];
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
                rinsum -= sir[0];
                ginsum -= sir[1];
                binsum -= sir[2];
                ++yi;
            }

            yw += w;
        }

        for(x = 0; x < w; ++x) {
            bsum = 0;
            gsum = 0;
            rsum = 0;
            boutsum = 0;
            goutsum = 0;
            routsum = 0;
            binsum = 0;
            ginsum = 0;
            rinsum = 0;
            int yp = -radius * w;

            for(i = -radius; i <= radius; ++i) {
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

            for(y = 0; y < h; ++y) {
                pix[yi] = -16777216 & pix[yi] | dv[rsum] << 16 | dv[gsum] << 8 | dv[bsum];
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

        ret.setPixels(pix, 0, w, 0, 0, w, h);
        return ret;
    }

    public static boolean save(Bitmap src, String filePath, Bitmap.CompressFormat format) {
        return save(src, getFileByPath(filePath), format, false);
    }

    public static boolean save(Bitmap src, File file, Bitmap.CompressFormat format) {
        return save(src, file, format, false);
    }

    public static boolean save(Bitmap src, String filePath, Bitmap.CompressFormat format, boolean recycle) {
        return save(src, getFileByPath(filePath), format, recycle);
    }

    public static boolean save(Bitmap src, File file, Bitmap.CompressFormat format, boolean recycle) {
        if (!isEmptyBitmap(src) && createFileByDeleteOldFile(file)) {
            OutputStream os = null;
            boolean ret = false;

            try {
                os = new BufferedOutputStream(new FileOutputStream(file));
                ret = src.compress(format, 100, os);
                if (recycle && !src.isRecycled()) {
                    src.recycle();
                }
            } catch (IOException var15) {
                var15.printStackTrace();
            } finally {
                try {
                    if (os != null) {
                        os.close();
                    }
                } catch (IOException var14) {
                    var14.printStackTrace();
                }

            }

            return ret;
        } else {
            return false;
        }
    }

    public static boolean isImage(File file) {
        return file != null && file.exists() ? isImage(file.getPath()) : false;
    }

    public static boolean isImage(String filePath) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;

        try {
            BitmapFactory.decodeFile(filePath, options);
            return options.outWidth != -1 && options.outHeight != -1;
        } catch (Exception var3) {
            return false;
        }
    }

    public static ImageType getImageType(String filePath) {
        return getImageType(getFileByPath(filePath));
    }

    public static ImageType getImageType(File file) {
        if (file == null) {
            return null;
        } else {
            FileInputStream is = null;

            ImageType var3;
            try {
                Object var4;
                try {
                    is = new FileInputStream(file);
                    ImageType type = getImageType((InputStream)is);
                    if (type == null) {
                        var4 = null;
                        return (ImageType)var4;
                    }

                    var3 = type;
                } catch (IOException var15) {
                    var15.printStackTrace();
                    var4 = null;
                    return (ImageType)var4;
                }
            } finally {
                try {
                    if (is != null) {
                        is.close();
                    }
                } catch (IOException var14) {
                    var14.printStackTrace();
                }

            }

            return var3;
        }
    }

    private static ImageType getImageType(InputStream is) {
        if (is == null) {
            return null;
        } else {
            try {
                byte[] bytes = new byte[12];
                return is.read(bytes) != -1 ? getImageType(bytes) : null;
            } catch (IOException var2) {
                var2.printStackTrace();
                return null;
            }
        }
    }

    private static ImageType getImageType(byte[] bytes) {
        String type = bytes2HexString(bytes).toUpperCase();
        if (type.contains("FFD8FF")) {
            return ImageType.TYPE_JPG;
        } else if (type.contains("89504E47")) {
            return ImageType.TYPE_PNG;
        } else if (type.contains("47494638")) {
            return ImageType.TYPE_GIF;
        } else if (!type.contains("49492A00") && !type.contains("4D4D002A")) {
            if (type.contains("424D")) {
                return ImageType.TYPE_BMP;
            } else if (type.startsWith("52494646") && type.endsWith("57454250")) {
                return ImageType.TYPE_WEBP;
            } else {
                return !type.contains("00000100") && !type.contains("00000200") ? ImageType.TYPE_UNKNOWN : ImageType.TYPE_ICO;
            }
        } else {
            return ImageType.TYPE_TIFF;
        }
    }

    private static String bytes2HexString(byte[] bytes) {
        if (bytes == null) {
            return "";
        } else {
            int len = bytes.length;
            if (len <= 0) {
                return "";
            } else {
                char[] ret = new char[len << 1];
                int i = 0;

                for(int var4 = 0; i < len; ++i) {
                    ret[var4++] = hexDigits[bytes[i] >> 4 & 15];
                    ret[var4++] = hexDigits[bytes[i] & 15];
                }

                return new String(ret);
            }
        }
    }

    private static boolean isJPEG(byte[] b) {
        return b.length >= 2 && b[0] == -1 && b[1] == -40;
    }

    private static boolean isGIF(byte[] b) {
        return b.length >= 6 && b[0] == 71 && b[1] == 73 && b[2] == 70 && b[3] == 56 && (b[4] == 55 || b[4] == 57) && b[5] == 97;
    }

    private static boolean isPNG(byte[] b) {
        return b.length >= 8 && b[0] == -119 && b[1] == 80 && b[2] == 78 && b[3] == 71 && b[4] == 13 && b[5] == 10 && b[6] == 26 && b[7] == 10;
    }

    private static boolean isBMP(byte[] b) {
        return b.length >= 2 && b[0] == 66 && b[1] == 77;
    }

    private static boolean isEmptyBitmap(Bitmap src) {
        return src == null || src.getWidth() == 0 || src.getHeight() == 0;
    }

    public static Bitmap compressByScale(Bitmap src, int newWidth, int newHeight) {
        return scale(src, newWidth, newHeight, false);
    }

    public static Bitmap compressByScale(Bitmap src, int newWidth, int newHeight, boolean recycle) {
        return scale(src, newWidth, newHeight, recycle);
    }

    public static Bitmap compressByScale(Bitmap src, float scaleWidth, float scaleHeight) {
        return scale(src, scaleWidth, scaleHeight, false);
    }

    public static Bitmap compressByScale(Bitmap src, float scaleWidth, float scaleHeight, boolean recycle) {
        return scale(src, scaleWidth, scaleHeight, recycle);
    }

    public static byte[] compressByQuality(Bitmap src, @IntRange(from = 0L,to = 100L) int quality) {
        return compressByQuality(src, quality, false);
    }

    public static byte[] compressByQuality(Bitmap src, @IntRange(from = 0L,to = 100L) int quality, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            src.compress(Bitmap.CompressFormat.JPEG, quality, baos);
            byte[] bytes = baos.toByteArray();
            if (recycle && !src.isRecycled()) {
                src.recycle();
            }

            return bytes;
        }
    }

    public static byte[] compressByQuality(Bitmap src, long maxByteSize) {
        return compressByQuality(src, maxByteSize, false);
    }

    public static byte[] compressByQuality(Bitmap src, long maxByteSize, boolean recycle) {
        if (!isEmptyBitmap(src) && maxByteSize > 0L) {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            src.compress(Bitmap.CompressFormat.JPEG, 100, baos);
            byte[] bytes;
            if ((long)baos.size() <= maxByteSize) {
                bytes = baos.toByteArray();
            } else {
                baos.reset();
                src.compress(Bitmap.CompressFormat.JPEG, 0, baos);
                if ((long)baos.size() >= maxByteSize) {
                    bytes = baos.toByteArray();
                } else {
                    int st = 0;
                    int end = 100;
                    int mid = 0;

                    while(st < end) {
                        mid = (st + end) / 2;
                        baos.reset();
                        src.compress(Bitmap.CompressFormat.JPEG, mid, baos);
                        int len = baos.size();
                        if ((long)len == maxByteSize) {
                            break;
                        }

                        if ((long)len > maxByteSize) {
                            end = mid - 1;
                        } else {
                            st = mid + 1;
                        }
                    }

                    if (end == mid - 1) {
                        baos.reset();
                        src.compress(Bitmap.CompressFormat.JPEG, st, baos);
                    }

                    bytes = baos.toByteArray();
                }
            }

            if (recycle && !src.isRecycled()) {
                src.recycle();
            }

            return bytes;
        } else {
            return new byte[0];
        }
    }

    public static Bitmap compressBySampleSize(Bitmap src, int sampleSize) {
        return compressBySampleSize(src, sampleSize, false);
    }

    public static Bitmap compressBySampleSize(Bitmap src, int sampleSize, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inSampleSize = sampleSize;
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            src.compress(Bitmap.CompressFormat.JPEG, 100, baos);
            byte[] bytes = baos.toByteArray();
            if (recycle && !src.isRecycled()) {
                src.recycle();
            }

            return BitmapFactory.decodeByteArray(bytes, 0, bytes.length, options);
        }
    }

    public static Bitmap compressBySampleSize(Bitmap src, int maxWidth, int maxHeight) {
        return compressBySampleSize(src, maxWidth, maxHeight, false);
    }

    public static Bitmap compressBySampleSize(Bitmap src, int maxWidth, int maxHeight, boolean recycle) {
        if (isEmptyBitmap(src)) {
            return null;
        } else {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            src.compress(Bitmap.CompressFormat.JPEG, 100, baos);
            byte[] bytes = baos.toByteArray();
            BitmapFactory.decodeByteArray(bytes, 0, bytes.length, options);
            options.inSampleSize = calculateInSampleSize(options, maxWidth, maxHeight);
            options.inJustDecodeBounds = false;
            if (recycle && !src.isRecycled()) {
                src.recycle();
            }

            return BitmapFactory.decodeByteArray(bytes, 0, bytes.length, options);
        }
    }

    public static int[] getSize(String filePath) {
        return getSize(getFileByPath(filePath));
    }

    public static int[] getSize(File file) {
        if (file == null) {
            return new int[]{0, 0};
        } else {
            BitmapFactory.Options opts = new BitmapFactory.Options();
            opts.inJustDecodeBounds = true;
            BitmapFactory.decodeFile(file.getAbsolutePath(), opts);
            return new int[]{opts.outWidth, opts.outHeight};
        }
    }

    public static int calculateInSampleSize(BitmapFactory.Options options, int maxWidth, int maxHeight) {
        int height = options.outHeight;
        int width = options.outWidth;

        int inSampleSize;
        for(inSampleSize = 1; height > maxHeight || width > maxWidth; inSampleSize <<= 1) {
            height >>= 1;
            width >>= 1;
        }

        return inSampleSize;
    }

    private static File getFileByPath(String filePath) {
        return isSpace(filePath) ? null : new File(filePath);
    }

    private static boolean createFileByDeleteOldFile(File file) {
        if (file == null) {
            return false;
        } else if (file.exists() && !file.delete()) {
            return false;
        } else if (!createOrExistsDir(file.getParentFile())) {
            return false;
        } else {
            try {
                return file.createNewFile();
            } catch (IOException var2) {
                var2.printStackTrace();
                return false;
            }
        }
    }

    private static boolean createOrExistsDir(File file) {
        if (file != null) {
            if (file.exists()) {
                if (!file.isDirectory()) {
                    return false;
                }
            } else if (!file.mkdirs()) {
                return false;
            }

            return true;
        } else {
            return false;
        }
    }

    private static boolean isSpace(String s) {
        if (s != null) {
            int i = 0;

            for(int len = s.length(); i < len; ++i) {
                if (!Character.isWhitespace(s.charAt(i))) {
                    return false;
                }
            }
        }

        return true;
    }

    private static byte[] input2Byte(InputStream is) {
        if (is == null) {
            return null;
        } else {
            try {
                ByteArrayOutputStream os = new ByteArrayOutputStream();
                byte[] b = new byte[1024];

                int len;
                while((len = is.read(b, 0, 1024)) != -1) {
                    os.write(b, 0, len);
                }

                byte[] var4 = os.toByteArray();
                return var4;
            } catch (IOException var14) {
                var14.printStackTrace();
            } finally {
                try {
                    is.close();
                } catch (IOException var13) {
                    var13.printStackTrace();
                }

            }

            return (byte[])null;
        }
    }

    public static enum ImageType {
        TYPE_JPG("jpg"),
        TYPE_PNG("png"),
        TYPE_GIF("gif"),
        TYPE_TIFF("tiff"),
        TYPE_BMP("bmp"),
        TYPE_WEBP("webp"),
        TYPE_ICO("ico"),
        TYPE_UNKNOWN("unknown");

        String value;

        private ImageType(String value) {
            this.value = value;
        }

        public String getValue() {
            return this.value;
        }
    }
}
