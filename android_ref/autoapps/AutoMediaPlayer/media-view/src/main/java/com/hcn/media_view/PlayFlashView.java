package com.hcn.media_view;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Shader.TileMode;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;

import com.hcn.media_view.uitls.Utils;

/**
 * 频谱视图
 * @author 86158
 */
public class PlayFlashView extends View {
    private final int COUNT = 32;

    private boolean bClear = false;
    private byte[] nowIndex = new byte[COUNT];
    private final byte[] lastIndex = new byte[COUNT];

    private int columnWidth = 0;
    private final Paint p = new Paint();
    private final Paint p2 = new Paint();
    private final Paint p3 = new Paint();
    private final Paint mPaint = new Paint();
    private Canvas canvas = null;
    private Bitmap mSCBitmap = null;
    private int width = 0;
    private int height = 0;
    private int YHeight = 0;
    private int everyWidth = 0;

    private int HEIGHT_LINE = 15;

    private final Context mContext;

    /**
     * 初始化标记
     */
    private boolean mInitialized = false;

    public PlayFlashView(Context context) {
        super(context);
        mContext = context;
    }

    public PlayFlashView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
    }

    private void init() {
        mInitialized = true;

        p.setStyle(Paint.Style.FILL);
        p2.setStyle(Paint.Style.FILL);
        p3.setStyle(Paint.Style.FILL);

        int resId = mContext.getResources().getIdentifier(
                "freq_line_width", "integer", mContext.getPackageName());
        p3.setStrokeWidth(mContext.getResources().getInteger(resId));

        p.setAntiAlias(true);
        p2.setAntiAlias(true);
        p3.setAntiAlias(true);

        p.setDither(true);
        p2.setDither(true);
        p3.setDither(true);

        DisplayMetrics dm = getResources().getDisplayMetrics();
        Activity owner = Utils.getActivityFromView(this);
        if (owner != null) {
            // 不在多窗口模式
            if (!owner.isInMultiWindowMode()) {
                dm = mContext.getApplicationContext().getResources().getDisplayMetrics();
            }
        }

        width = dm.widthPixels;
        height = dm.heightPixels;

        resId = mContext.getResources().getIdentifier(
                "freq_divider_width", "integer", mContext.getPackageName());
        int dividerWidth = mContext.getResources().getInteger(resId);

        resId = mContext.getResources().getIdentifier(
                "freq_width_factor", "integer", mContext.getPackageName());
        int widthFactor = mContext.getResources().getInteger(resId);

        everyWidth = (width - 30) / COUNT;
        columnWidth = everyWidth - dividerWidth - widthFactor;
        if (columnWidth <= 0) {
            columnWidth = 1;
        }

        YHeight = height * 5 / 12;

        resId = mContext.getResources().getIdentifier(
                "freq_bottom", "color", mContext.getPackageName());
        int colorBottom = mContext.getColor(resId);

        resId = mContext.getResources().getIdentifier(
                "freq_top", "color", mContext.getPackageName());
        int colorTop = mContext.getColor(resId);

        resId = mContext.getResources().getIdentifier(
                "freq_top_line", "color", mContext.getPackageName());
        int colorTopLine = mContext.getColor(resId);

        // 频谱渐变
        LinearGradient gradient = new LinearGradient(0, YHeight, 0, YHeight - 127, colorBottom,
                colorTop, TileMode.CLAMP);
        p.setShader(gradient);

        // 倒影渐变
        LinearGradient gradient2 = new LinearGradient(0, YHeight, 0, YHeight + 64,
                (colorBottom & 0x00ffffff) | 0x70000000, (colorTop & 0x00ffffff), TileMode.CLAMP);
        p2.setShader(gradient2);

        p3.setColor(colorTopLine);
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);

        init();
    }

    private void initScreen() {
        if (canvas == null) {
            canvas = new Canvas();
        }

        if (mSCBitmap == null) {
            mSCBitmap = Bitmap.createBitmap(width, height, Config.ARGB_8888);
        } else {
            mSCBitmap.eraseColor(0x00000000);
        }

        canvas.setBitmap(mSCBitmap);
        int viewWidth = getWidth();
        if (!bClear) {
            for (int i = 0; i < COUNT; i++) {
                int rightPos = 15 + everyWidth * i + columnWidth;
                if (rightPos <= viewWidth) {
                    if (nowIndex[i] > 0) {
                        // 画频谱
                        canvas.drawRect(everyWidth * i + 15, YHeight - nowIndex[i],
                                rightPos, YHeight, p);

                        // 画倒影
                        int YHeight2 = nowIndex[i] / 2;
                        canvas.drawRect(everyWidth * i + 15, YHeight, rightPos,
                                YHeight + YHeight2, p2);

                        // 画顶部
                        if (lastIndex[i] < nowIndex[i]) {
                            canvas.drawLine(everyWidth * i + 15,
                                    YHeight - nowIndex[i] - HEIGHT_LINE,
                                    rightPos, YHeight - nowIndex[i] - HEIGHT_LINE, p3);
                            lastIndex[i] = nowIndex[i];
                        } else if (lastIndex[i] - nowIndex[i] > 7) {
                            if (lastIndex[i] >= 3) {
                                lastIndex[i] = (byte) (lastIndex[i] - 2);
                            } else {
                                lastIndex[i] = 0;
                            }

                            canvas.drawLine(everyWidth * i + 15,
                                    YHeight - lastIndex[i] - HEIGHT_LINE, rightPos,
                                    YHeight - lastIndex[i] - HEIGHT_LINE, p3);
                        }
                    } else if (nowIndex[i] <= 0) {
                        if (lastIndex[i] > 3) {
                            lastIndex[i] = (byte) (lastIndex[i] - 2);
                            canvas.drawLine(everyWidth * i + 15, YHeight - lastIndex[i], rightPos,
                                    YHeight - lastIndex[i], p3);
                        } else {
                            lastIndex[i] = 0;
                            canvas.drawLine(everyWidth * i + 15, YHeight, rightPos, YHeight, p3);
                        }
                    }
                }
            }
        } else {
            for (int i = 0; i < COUNT; i++) {
                int rightPos = 15 + everyWidth * i + columnWidth;
                if (rightPos <= viewWidth) {
                    lastIndex[i] = 0;
                    canvas.drawLine(everyWidth * i + 15, YHeight, rightPos, YHeight, p3);
                }
            }
        }
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        if (mInitialized) {
            initScreen();
            canvas.drawBitmap(mSCBitmap, 0, 0, mPaint);
        }
    }

    /**
     * 更新数据
     *
     * @param data 音频数据
     */
    public void updateVisualizer(byte[] data) {
        byte[] model = new byte[data.length / 2 + 1];
        model[0] = (byte) Math.abs(data[0]);
        for (int i = 2, j = 1; j < COUNT; ) {
            model[j] = (byte) Math.hypot(data[i], data[i + 1]);
            if (model[j] < 64) {
                model[j] = (byte) (model[j] * 2);
            }
            i += 2;
            j++;
        }

        bClear = false;
        nowIndex = model;

        invalidate();
    }

    public void clearVisualizer() {
        bClear = true;
        invalidate();
    }
}
