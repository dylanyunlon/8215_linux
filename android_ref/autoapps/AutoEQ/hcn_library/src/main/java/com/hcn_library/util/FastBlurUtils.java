package com.hcn_library.util;

import static com.blankj.utilcode.util.ThreadUtils.runOnUiThread;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.renderscript.ScriptIntrinsicBlur;
import android.util.Log;
import android.view.View;

import androidx.annotation.DrawableRes;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Random;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

/**
 * @author simon
 * @describe 图片毛玻璃处理
 * @date 2022/11/22 16:31
 */
public class FastBlurUtils {
    private static final String TAG = FastBlurUtils.class.getSimpleName();
    // 图片放大倍数
    private static float BITMAP_ZOOM_IN_SCALE = 4.0f;
    // 图片缩放倍数
    private static float BITMAP_ZOOM_OUT_SCALE = 0.25f;
    // 模糊程度参数值，模糊半径(radius)越大，性能要求越高，模糊半径不能超过25
    private static int BLUR_RADIUS = 15;
    private static RenderScript rs;
    // 复用线程池
    private static ExecutorService executor = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());
    // 用于存储之前任务的 Future 对象
    private static List<Future<?>> previousFutures = new ArrayList<>();


    public static void setBlurQuality(float enlargeScale, int blurRadius) {
        BITMAP_ZOOM_IN_SCALE = enlargeScale;
        BITMAP_ZOOM_OUT_SCALE = 1f / enlargeScale;
        BLUR_RADIUS = blurRadius;
    }

    public static void resetBlurQuality() {
        BITMAP_ZOOM_IN_SCALE = 4;
        BITMAP_ZOOM_OUT_SCALE = 1f / 4;
        BLUR_RADIUS = 25;
    }

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
        int screenWidth = activity.getResources().getDisplayMetrics().widthPixels;
        int screenHeight = activity.getResources().getDisplayMetrics().heightPixels;
        Bitmap bmp = Bitmap.createBitmap(bitmap, 0, 0, screenWidth, screenHeight);
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
     * 资源图片转bitmap
     *
     * @param resources
     * @param drawableResId
     * @return
     */
    public static Bitmap drawableToBitmap(Resources resources, @DrawableRes int drawableResId) {
        Drawable drawable = resources.getDrawable(drawableResId);
        if (drawable instanceof BitmapDrawable) {
            return ((BitmapDrawable) drawable).getBitmap();
        }
        // 如果drawable不是BitmapDrawable，手动创建Bitmap
        Bitmap bitmap = Bitmap.createBitmap(drawable.getIntrinsicWidth(), drawable.getIntrinsicHeight(), Bitmap.Config.ARGB_8888);
        drawable.setBounds(0, 0, bitmap.getWidth(), bitmap.getWidth());
        drawable.draw(new Canvas(bitmap));
        return bitmap;
    }

    public static Bitmap createSolidColorBitmap(int color, int bitmapWidth, int bitmapHeight) {
        Bitmap bitmap = Bitmap.createBitmap(bitmapWidth, bitmapHeight, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        Paint paint = new Paint();
        paint.setColor(color);
        canvas.drawRect(0, 0, bitmapWidth, bitmapHeight, paint);
        return addNoiseToBitmap(bitmap);
    }

    /**
     * 添加噪点
     *
     * @param bitmap
     * @return
     */
    public static Bitmap addNoiseToBitmap(Bitmap bitmap) {
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        Bitmap result = Bitmap.createBitmap(width, height, bitmap.getConfig());
        Canvas canvas = new Canvas(result);
        canvas.drawBitmap(bitmap, 0, 0, null);
        Paint paint = new Paint();
        Random random = new Random();
        for (int i = 0; i < 1000; i++) { // 可以调整噪点数量
            int x = random.nextInt(width);
            int y = random.nextInt(height);
            int color = Color.rgb(random.nextInt(256), random.nextInt(256), random.nextInt(256));
            paint.setColor(color);
            canvas.drawPoint(x, y, paint);
        }
        return result;
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
     * @param backgroundBitmap
     * @return
     */
    public static Bitmap startBlurBackground(Bitmap backgroundBitmap) {
        Bitmap smallBlurBitmap = fastBlurBitmap(zoomOut(backgroundBitmap), BLUR_RADIUS);
        return zoomIn(smallBlurBitmap);
    }

    /**
     * 缩放图片大小虚化，采用RenderScript方式
     *
     * @param context
     * @param backgroundBitmap
     * @return
     */
    public static Bitmap startBlurBackground(Context context, Bitmap backgroundBitmap) {
        Bitmap smallBlurBitmap = gaussianBlur(context, zoomOut(backgroundBitmap), BLUR_RADIUS);
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
     * @param originalBitmap
     * @param blurRadius     模糊程度参数值，模糊半径(radius)越大，性能要求越高，模糊半径不能超过25
     * @return
     */
    public static Bitmap gaussianBlur(Context context, Bitmap originalBitmap, int blurRadius) {
        //api大于17才能用
        if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.JELLY_BEAN_MR1) {
            return null;
        }
        RenderScript renderScript = RenderScript.create(context);
        Allocation input = Allocation.createFromBitmap(renderScript, originalBitmap);
        Allocation output = Allocation.createTyped(renderScript, input.getType());
        ScriptIntrinsicBlur scriptIntrinsicBlur = ScriptIntrinsicBlur.create(renderScript, Element.U8_4(renderScript));
        scriptIntrinsicBlur.setRadius(blurRadius);
        scriptIntrinsicBlur.setInput(input);
        scriptIntrinsicBlur.forEach(output);
        output.copyTo(originalBitmap);
        return originalBitmap;
    }

    /**
     * 对图片进行高斯模糊算法
     *
     * @param sentBitmap
     * @param blurRadius //模糊程度参数值，模糊半径(radius)越大，性能要求越高，模糊半径不能超过25
     * @return
     */
    public static Bitmap fastBlurBitmap(Bitmap sentBitmap, int blurRadius) {
        Bitmap bitmap = sentBitmap.copy(sentBitmap.getConfig(), true);

        if (blurRadius < 1) {
            return (null);
        }

        int bitmapWidth = bitmap.getWidth();
        int bitmapHeight = bitmap.getHeight();

        int[] pixels = new int[bitmapWidth * bitmapHeight];
        bitmap.getPixels(pixels, 0, bitmapWidth, 0, 0, bitmapWidth, bitmapHeight);

        int widthMinusOne = bitmapWidth - 1;
        int heightMinusOne = bitmapHeight - 1;
        int totalPixels = bitmapWidth * bitmapHeight;
        int divisor = blurRadius + blurRadius + 1;

        int[] red = new int[totalPixels];
        int[] green = new int[totalPixels];
        int[] blue = new int[totalPixels];
        int redSum, greenSum, blueSum, x, y, i, pixel, yPosition, index, yStride;
        int[] minValue = new int[Math.max(bitmapWidth, bitmapHeight)];

        int divisorSum = (divisor + 1) >> 1;
        divisorSum *= divisorSum;
        int[] divisorValue = new int[256 * divisorSum];
        for (i = 0; i < 256 * divisorSum; i++) {
            divisorValue[i] = (i / divisorSum);
        }

        yStride = index = 0;

        int[][] stack = new int[divisor][3];
        int stackPointer;
        int stackStart;
        int[] stackItem;
        int radiusPlusOne = blurRadius + 1;
        int redOutSum, greenOutSum, blueOutSum;
        int redInSum, greenInSum, blueInSum;

        for (y = 0; y < bitmapHeight; y++) {
            redInSum = greenInSum = blueInSum = redOutSum = greenOutSum = blueOutSum = redSum = greenSum = blueSum = 0;
            for (i = -blurRadius; i <= blurRadius; i++) {
                pixel = pixels[index + Math.min(widthMinusOne, Math.max(i, 0))];
                stackItem = stack[i + blurRadius];
                stackItem[0] = (pixel & 0xff0000) >> 16;
                stackItem[1] = (pixel & 0x00ff00) >> 8;
                stackItem[2] = (pixel & 0x0000ff);
                int radiusBoundary = radiusPlusOne - Math.abs(i);
                redSum += stackItem[0] * radiusBoundary;
                greenSum += stackItem[1] * radiusBoundary;
                blueSum += stackItem[2] * radiusBoundary;
                if (i > 0) {
                    redInSum += stackItem[0];
                    greenInSum += stackItem[1];
                    blueInSum += stackItem[2];
                } else {
                    redOutSum += stackItem[0];
                    greenOutSum += stackItem[1];
                    blueOutSum += stackItem[2];
                }
            }
            stackPointer = blurRadius;

            for (x = 0; x < bitmapWidth; x++) {

                red[index] = divisorValue[redSum];
                green[index] = divisorValue[greenSum];
                blue[index] = divisorValue[blueSum];

                redSum -= redOutSum;
                greenSum -= greenOutSum;
                blueSum -= blueOutSum;

                stackStart = stackPointer - blurRadius + divisor;
                stackItem = stack[stackStart % divisor];

                redOutSum -= stackItem[0];
                greenOutSum -= stackItem[1];
                blueOutSum -= stackItem[2];

                if (y == 0) {
                    minValue[x] = Math.min(x + blurRadius + 1, widthMinusOne);
                }
                pixel = pixels[yStride + minValue[x]];

                stackItem[0] = (pixel & 0xff0000) >> 16;
                stackItem[1] = (pixel & 0x00ff00) >> 8;
                stackItem[2] = (pixel & 0x0000ff);

                redInSum += stackItem[0];
                greenInSum += stackItem[1];
                blueInSum += stackItem[2];

                redSum += redInSum;
                greenSum += greenInSum;
                blueSum += blueInSum;

                stackPointer = (stackPointer + 1) % divisor;
                stackItem = stack[(stackPointer) % divisor];

                redOutSum += stackItem[0];
                greenOutSum += stackItem[1];
                blueOutSum += stackItem[2];

                redInSum -= stackItem[0];
                greenInSum -= stackItem[1];
                blueInSum -= stackItem[2];

                index++;
            }
            yStride += bitmapWidth;
        }
        for (x = 0; x < bitmapWidth; x++) {
            redInSum = greenInSum = blueInSum = redOutSum = greenOutSum = blueOutSum = redSum = greenSum = blueSum = 0;
            yPosition = -blurRadius * bitmapWidth;
            for (i = -blurRadius; i <= blurRadius; i++) {
                index = Math.max(0, yPosition) + x;

                stackItem = stack[i + blurRadius];

                stackItem[0] = red[index];
                stackItem[1] = green[index];
                stackItem[2] = blue[index];

                int radiusBoundary = radiusPlusOne - Math.abs(i);

                redSum += red[index] * radiusBoundary;
                greenSum += green[index] * radiusBoundary;
                blueSum += blue[index] * radiusBoundary;

                if (i > 0) {
                    redInSum += stackItem[0];
                    greenInSum += stackItem[1];
                    blueInSum += stackItem[2];
                } else {
                    redOutSum += stackItem[0];
                    greenOutSum += stackItem[1];
                    blueOutSum += stackItem[2];
                }

                if (i < heightMinusOne) {
                    yPosition += bitmapWidth;
                }
            }
            index = x;
            stackPointer = blurRadius;
            for (y = 0; y < bitmapHeight; y++) {
                pixels[index] = (0xff000000 & pixels[index]) | (divisorValue[redSum] << 16) | (divisorValue[greenSum] << 8) | divisorValue[blueSum];

                redSum -= redOutSum;
                greenSum -= greenOutSum;
                blueSum -= blueOutSum;

                stackStart = stackPointer - blurRadius + divisor;
                stackItem = stack[stackStart % divisor];

                redOutSum -= stackItem[0];
                greenOutSum -= stackItem[1];
                blueOutSum -= stackItem[2];

                if (x == 0) {
                    minValue[y] = Math.min(y + radiusPlusOne, heightMinusOne) * bitmapWidth;
                }
                pixel = x + minValue[y];

                stackItem[0] = red[pixel];
                stackItem[1] = green[pixel];
                stackItem[2] = blue[pixel];

                redInSum += stackItem[0];
                greenInSum += stackItem[1];
                blueInSum += stackItem[2];

                redSum += redInSum;
                greenSum += greenInSum;
                blueSum += blueInSum;

                stackPointer = (stackPointer + 1) % divisor;
                stackItem = stack[stackPointer];

                redOutSum += stackItem[0];
                greenOutSum += stackItem[1];
                blueOutSum += stackItem[2];

                redInSum -= stackItem[0];
                greenInSum -= stackItem[1];
                blueInSum -= stackItem[2];

                index += bitmapWidth;
            }
        }

        bitmap.setPixels(pixels, 0, bitmapWidth, 0, 0, bitmapWidth, bitmapHeight);

        return (bitmap);
    }

    public static BitmapDrawable getBlurWindowBg(int x, int y, int width, int height, int resourceId, int blurResourceId, Context context, float radiusSize) {
        if (width <= 0 || height <= 0 || x <= 0 || y <= 0) {
            return null;
        }
        // 获取模糊资源
        Drawable blurDrawable = SkinUtils.getDrawable(blurResourceId);
        // 获取可见部分的位图并进行模糊处理
        Bitmap visiblePartBitmap = getVisibleBitmap(x, y, width, height, BitmapUtils.drawableToBitmap(SkinUtils.getDrawable(resourceId)));
        // 创建最终的位图并进行绘制操作
        Bitmap finalBitmap = createAndDrawBlurBitmap(visiblePartBitmap, width, height, radiusSize);
        // 将模糊资源绘制在最终位图上
        blurDrawable.setBounds(0, 0, width, height);
        blurDrawable.draw(new Canvas(finalBitmap));
        return new BitmapDrawable(context.getResources(), finalBitmap);
    }

    public static Bitmap getVisibleBitmap(int x, int y, int width, int height, Bitmap bitmap) {
        Log.d("FastBlurUtils", "window viewX = " + x + "; viewY = " + y + "; widthView = " + width + "; heightView = " + height + "; bitmap.width" + bitmap.getWidth() + "; bitmap.height" + bitmap.getHeight());
        return zoomIn(fastBlurBitmap(zoomOut(Bitmap.createBitmap(bitmap, x, y, width, height)), 20));
    }

    public static BitmapDrawable getBlurBackground(int width, int height, int resourceId, int blurResourceId, View view, Context context, float radiusSize) {
        if (width <= 0 || height <= 0) {
            return null;
        }
        // 获取模糊资源
        Drawable blurDrawable = SkinUtils.getDrawable(blurResourceId);
        Log.d("FastBlurUtils", "view height: " + height + " view width: " + width + " resourceDrawable bitmap height = " + BitmapUtils.drawableToBitmap(SkinUtils.getDrawable(resourceId)).getHeight() + " resourceDrawable bitmap width = " + BitmapUtils.drawableToBitmap(SkinUtils.getDrawable(resourceId)).getWidth());
        // 获取可见部分的位图并进行模糊处理
        Bitmap visiblePartBitmap = getVisiblePartBitmap(view, BitmapUtils.drawableToBitmap(SkinUtils.getDrawable(resourceId)));
        // 创建最终的位图并进行绘制操作
        Bitmap finalBitmap = createAndDrawBlurBitmap(visiblePartBitmap, width, height, radiusSize);
        // 将模糊资源绘制在最终位图上
        blurDrawable.setBounds(0, 0, width, height);
        blurDrawable.draw(new Canvas(finalBitmap));
        return new BitmapDrawable(context.getResources(), finalBitmap);
    }


    private static Bitmap createAndDrawBlurBitmap(Bitmap visiblePartBitmap, int width, int height, float radiusSize) {
        // 创建最终尺寸的位图
        Bitmap finalBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(finalBitmap);
        Paint paint = new Paint();
        paint.setAntiAlias(true);

        // 创建 Matrix 对象用于缩放操作
        Matrix matrix = new Matrix();
        float scaleWidth = (float) width / visiblePartBitmap.getWidth();
        float scaleHeight = (float) height / visiblePartBitmap.getHeight();
        // 设置缩放比例
        matrix.postScale(scaleWidth, scaleHeight);

        // 绘制圆角矩形
        if (radiusSize > 0) {
            canvas.drawRoundRect(new RectF(0.0f, 0.0f, width, height), radiusSize, radiusSize, paint);
            paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC_IN));
        }

        // 使用 Matrix 缩放 Bitmap 并绘制
        canvas.drawBitmap(visiblePartBitmap, matrix, paint);

        return finalBitmap;
    }


    public static Bitmap getVisiblePartBitmap(View view, Bitmap bitmap) {
        // 获取视图在屏幕上的位置
        int[] location = new int[2];
        view.getLocationOnScreen(location);
        int viewX = location[0];
        int viewY = location[1];
        int viewWidth = view.getWidth();
        int viewHeight = view.getHeight();
        Log.d("FastBlurUtils", "background viewX = " + viewX + "; viewY = " + viewY + "; viewWidth = " + viewWidth + "; viewHeight = " + viewHeight + "; bitmap.width = " + bitmap.getWidth() + "; bitmap.height = " + bitmap.getHeight());// 裁剪出视图可见部分的位图
        Bitmap croppedBitmap = Bitmap.createBitmap(bitmap, viewX, viewY, viewWidth, viewHeight);
        // 先缩小位图，进行模糊处理，再放大
        return zoomIn(fastBlurBitmap(zoomOut(croppedBitmap), BLUR_RADIUS));
    }

    // 精确截取View对应的背景区域
    public static Bitmap cropBitmap(Bitmap source, int left, int top, int width, int height) {
        if (width <= 0 || height <= 0 || source == null) {
            Log.d(TAG, "cropBitmap + width <= 0 || height <= 0 || source == null");
            return null;
        }
        // 考虑父容器偏移量
        left = Math.max(0, left);
        top = Math.max(0, top);

        // 创建目标Bitmap
        Bitmap output = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(output);

        // 绘制裁剪区域
        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG);
        canvas.drawBitmap(source,
                new Rect(left, top, left + width, top + height),
                new Rect(0, 0, width, height),
                paint
        );
        return output;
    }

    public static Bitmap getBackgroundBitmap(int currentDrawableId, Context context) {
        Drawable drawable = SkinUtils.getDrawable(currentDrawableId);
        if (drawable == null) return null;
        // 计算屏幕尺寸，如果和图片的尺寸不一致，则进行缩放
        int screenWidth = context.getResources().getDisplayMetrics().widthPixels;
        int screenHeight = context.getResources().getDisplayMetrics().heightPixels;
        // 计算缩放比例（保持宽高比）
        float widthRatio = (float) screenWidth * 1f / drawable.getIntrinsicWidth();
        float heightRatio = (float) screenHeight * 1f / drawable.getIntrinsicHeight();

        // 计算缩放后尺寸
        int scaledWidth = Math.round(drawable.getIntrinsicWidth() * widthRatio);
        int scaledHeight = Math.round(drawable.getIntrinsicHeight() * heightRatio);

        try {
            // 创建缩放后的 Bitmap
            Bitmap scaledBitmap = Bitmap.createBitmap(scaledWidth, scaledHeight, Bitmap.Config.ARGB_8888);
            Canvas canvas = new Canvas(scaledBitmap);

            // 设置缩放矩阵
            Matrix matrix = new Matrix();
            matrix.postScale(widthRatio, heightRatio);

            // 绘制缩放后的图形
            Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG);
            canvas.drawBitmap(
                    drawableToBitmap(drawable, scaledWidth, scaledHeight),  // 原始 Bitmap
                    matrix,
                    paint
            );

            return scaledBitmap;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    public static Bitmap drawableToBitmap(Drawable drawable, int defaultWidth, int defaultHeight) {
        int width = drawable.getIntrinsicWidth();
        int height = drawable.getIntrinsicHeight();
        // xml 中未设置宽高时，使用默认值
        if (width <= 0 || height <= 0) {
            width = defaultWidth;
            height = defaultHeight;
            Log.d(TAG, "drawableToBitmap + drawable.getIntrinsicWidth() <= 0 || drawable.getIntrinsicHeight() <= 0");
        }
        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
        drawable.draw(canvas);
        return bitmap;
    }

    // 单独线程处理模糊
    public static void applyGlassEffectOptimized(boolean[] ifBlur, View[] views, int[] blurRadii, int[] sampleSizes, boolean[] ifWindow, float[] cornerRadii, int[] drawableIds, int currentDrawableId, View triggerButton, Context mContext) {
        //views = new View[]{llSurround, gridViewInPopup, btnLoudness, btnEdit, triggerButton, llLoudnessPop, clEdit, ivSurfaceBg};
        Log.d(TAG, "applyGlassEffectOptimized + views.length = " + views.length + " ifBlur " + Arrays.toString(ifBlur));
        // 取消之前的任务
        cancelPreviousTasks();
        List<Future<?>> futures = new ArrayList<>();
        for (int i = 0; i < views.length; i++) {
            final int index = i;
            if (!ifBlur[i]) {
                continue;
            }
            // 判断线程池状态
            Log.d(TAG, "applyGlassEffectOptimized + executor.isShutdown() = " + executor.isShutdown());
            // 提交任务并获取 Future 对象
            Future<?> future = executor.submit(() -> {
                try {
                    processView(index, views, blurRadii, sampleSizes, ifWindow, cornerRadii, drawableIds, currentDrawableId, triggerButton, mContext);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            });
            futures.add(future);
        }
        // 更新 previousFutures 列表
        previousFutures = futures;

        // 等待所有任务完成
        for (Future<?> future : futures) {
            try {
                future.get();
            } catch (InterruptedException | ExecutionException e) {
                e.printStackTrace();
            }
        }
    }

    private static void processView(int index, View[] views, int[] blurRadii, int[] sampleSizes, boolean[] ifWindow, float[] cornerRadii, int[] drawableIds, int currentDrawableId, View triggerButton, Context mContext) {
        try {
            View targetView = views[index];
            // ...获取位置等逻辑同原代码...
            int radiusPx = Math.max((int) cornerRadii[index], 0);
            int blurRadius = Math.max(blurRadii[index], 0);
            int sampleSize = Math.max(sampleSizes[index], 0);
            boolean windowFlag = ifWindow[index];
            int[] locationArr = new int[2];
            int viewLeft, viewTop;

            // 获取背景区域
            if (windowFlag &&  triggerButton != null) {
                triggerButton.getLocationOnScreen(locationArr);
                viewLeft = locationArr[0];
                viewTop = locationArr[1] + triggerButton.getMeasuredHeight();
            } else {
                targetView.getLocationOnScreen(locationArr);
                viewLeft = locationArr[0];
                viewTop = locationArr[1];
            }
            Log.d(TAG, "applyGlassEffectWithGlide: " + index + " " + locationArr[0] + " " + locationArr[1] + " " + targetView.getWidth() + " " + targetView.getHeight());

            // 1. 截取View对应的背景区域, 直接处理原始Bitmap
            Bitmap original = cropBitmap(
                    getBackgroundBitmap(currentDrawableId, mContext),
                    viewLeft,
                    viewTop,
                    targetView.getWidth(),
                    targetView.getHeight()
            );
            if (original == null) {
                Log.d(TAG, "applyGlassEffectOptimized: original == null");
                return;
            }

            // 2. 先缩放再模糊
            int scaledWidth = targetView.getWidth() / sampleSize;
            int scaledHeight = targetView.getHeight() / sampleSize;
            original = Bitmap.createScaledBitmap(original, scaledWidth, scaledHeight, true);


            // 3. 使用RenderScript模糊，再恢复到原来的尺寸
            Bitmap blurred = renderScriptBlur(original, blurRadius, scaledWidth, scaledHeight, mContext);
            scaledWidth = targetView.getWidth();
            scaledHeight = targetView.getHeight();
            blurred = Bitmap.createScaledBitmap(blurred, scaledWidth, scaledHeight, true);

            // 4. 添加圆角（在UI线程外操作）
            Bitmap rounded = addRoundedCorners(blurred, radiusPx);

            // 5. 合成最终图片
            final Bitmap output = createFinalBitmap(rounded, index, targetView, drawableIds[index]);

            // 6. UI更新
            runOnUiThread(() -> setViewBackground(targetView, output, mContext, drawableIds[index]));

            // 7. 回收中间Bitmap
            original.recycle();
            blurred.recycle();
            rounded.recycle();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static void setViewBackground(View targetView, Bitmap output, Context mContext, int defaultDrawableId) {
        runOnUiThread(() -> {
            Drawable drawable = SkinUtils.getDrawable(defaultDrawableId);
            if (output != null) {
                drawable = new BitmapDrawable(mContext.getResources(), output);
            }
            targetView.setBackground(drawable);
        });
    }

    // RenderScript模糊实现
    public static Bitmap renderScriptBlur(Bitmap bitmap, int radius, int maxWidth, int maxHeight, Context context) {
        Bitmap output = Bitmap.createScaledBitmap(bitmap, maxWidth, maxHeight, true);
        if (rs == null) {
            rs = RenderScript.create(context);
        }
        Allocation input = Allocation.createFromBitmap(rs, output);
        Allocation outputAlloc = Allocation.createTyped(rs, input.getType());

        ScriptIntrinsicBlur blur = ScriptIntrinsicBlur.create(rs, Element.U8_4(rs));
        blur.setRadius(Math.min(radius, 25)); // RenderScript最大支持25
        blur.setInput(input);
        blur.forEach(outputAlloc);
        outputAlloc.copyTo(output);
        outputAlloc.destroy();
        input.destroy();
        blur.destroy();
        return output;
    }

    // 优化圆角处理
    public static Bitmap addRoundedCorners(Bitmap bitmap, int radius) {
        Bitmap output = Bitmap.createBitmap(bitmap.getWidth(), bitmap.getHeight(), Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(output);

        Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);

        Path path = new Path();
        path.addRoundRect(new RectF(0, 0, bitmap.getWidth(), bitmap.getHeight()),
                radius, radius, Path.Direction.CW);

        canvas.clipPath(path);
        canvas.drawBitmap(bitmap, 0, 0, paint);
        return output;
    }


    public static Bitmap createFinalBitmap(Bitmap resource, int index, View targetView, int defaultDrawableId) {
        Bitmap output = Bitmap.createBitmap(
                resource.getWidth(),
                resource.getHeight(),
                Bitmap.Config.ARGB_8888
        );

        Canvas canvas = new Canvas(output);
        Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);

        Drawable defaultDrawable = SkinUtils.getDrawable(defaultDrawableId);
        Bitmap defaultBitmap = drawableToBitmap(defaultDrawable, resource.getWidth(), resource.getHeight());

        if (defaultBitmap != null) {
            defaultBitmap = Bitmap.createScaledBitmap(defaultBitmap, resource.getWidth(), resource.getHeight(), true);
        }

        Log.d(TAG, "applyGlass resource " + " index: " + index + " " + resource.getWidth() + " " + resource.getHeight() + " targetView: " + targetView.getWidth() + " " + targetView.getHeight());

        canvas.drawBitmap(resource, 0, 0, paint);

        if (defaultBitmap != null) {
            canvas.drawBitmap(defaultBitmap, 0, 0, paint);
        }

        return output;
    }

    private static void cancelPreviousTasks() {
        for (Future<?> future : previousFutures) {
            if (future != null && !future.isDone()) {
                future.cancel(true);
            }
        }
        previousFutures.clear();
    }

    // 关闭线程池的方法，可在应用退出时调用
    public static void shutdownExecutor() {
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            executor.shutdown();
            try {
                if (!executor.awaitTermination(60, TimeUnit.SECONDS)) {
                    executor.shutdownNow();
                    executor = null;
                }
            } catch (InterruptedException e) {
                executor.shutdownNow();
                executor = null;
            }
        }));
    }

    public static void releaseRenderScript() {
        cancelPreviousTasks();
        shutdownExecutor();
        if (rs != null) {
            rs.destroy();
            rs = null;
        }
    }
}