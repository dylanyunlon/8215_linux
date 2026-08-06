package com.hcn.autoeq.view;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;
import android.util.AttributeSet;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.autoeq.R;
import com.hcn_library.util.SkinUtils;

/**
 * 自定义seekbar，绘制渐变背景，支持enable和不可用时背景色渐变透明
 * 当前Skin框架下会有问题显示需要AppCompat主题问题，暂时不使用
 */

public class NineCustomSeekbar extends androidx.appcompat.widget.AppCompatSeekBar {
    private static final String TAG = NineCustomSeekbar.class.getSimpleName();
    private GradientDrawable gradientDrawable;
    private GradientDrawable backgroundDrawable; // 用于绘制底色的 GradientDrawable
    private GradientDrawable unableDrawable; // 用于绘制不可用时纯色底色的 GradientDrawable
    private float cornerRadius; // 圆角半径
    private float sliderHeight;
    private int unableStyle; // 0 代表不可用时设置纯色背景，1 代表不可用时设置渐变透明背景
    private int[] gradientColors;
    private int[] unableColors;

    public NineCustomSeekbar(@NonNull Context context) {
        super(context);
        init();
    }

    public NineCustomSeekbar(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        initAttrs(attrs);
        init();
    }

    public NineCustomSeekbar(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initAttrs(attrs);
        init();
    }

    private void initAttrs(AttributeSet attrs) {
        // 读取自定义属性
        TypedArray typedArray = getContext().obtainStyledAttributes(attrs, R.styleable.nine_seek_bar_attr);
        int colorsResId = typedArray.getResourceId(R.styleable.nine_seek_bar_attr_gradient_colors, R.array.nine_gradient_colors);
        int unableColorsResId = typedArray.getResourceId(R.styleable.nine_seek_bar_attr_gradient_unable_colors, R.array.gradient_unable_colors);
        typedArray.recycle();
        // 获取颜色数组
        gradientColors = getColorsFromResource(getContext(), colorsResId);
        unableColors = getColorsFromResource(getContext(), unableColorsResId);
    }

    private void init() {
        cornerRadius = SkinUtils.getDimension(R.dimen.x8);
        sliderHeight = SkinUtils.getDimension(R.dimen.y28);

        // 初始化渐变轨道的 GradientDrawable
        gradientDrawable = new GradientDrawable();
        gradientDrawable.setColors(gradientColors);
        gradientDrawable.setGradientType(GradientDrawable.LINEAR_GRADIENT);
        gradientDrawable.setOrientation(GradientDrawable.Orientation.BL_TR);
        gradientDrawable.setCornerRadius(cornerRadius);

        // 初始化底色的 GradientDrawable
        backgroundDrawable = new GradientDrawable();
        backgroundDrawable.setColor(SkinUtils.getColor(R.color.nine_seek_bar_bg_color));
        backgroundDrawable.setCornerRadius(cornerRadius);

        // 初始化不可用时底色的 GradientDrawable
        unableDrawable = new GradientDrawable();
        unableDrawable.setColors(unableColors);
        unableDrawable.setGradientType(GradientDrawable.LINEAR_GRADIENT);
        unableDrawable.setOrientation(GradientDrawable.Orientation.BL_TR);
        unableDrawable.setCornerRadius(cornerRadius);
    }

    private void setDrawableBounds(GradientDrawable drawable, int startX, int progressWidth) {
        drawable.setBounds(
                startX,
                getHeight() / 2 - (int) (sliderHeight / 2),
                startX + progressWidth,
                getHeight() / 2 + (int) (sliderHeight / 2)
        );
    }

    private int[] getColorsFromResource(Context context, int resId) {
        TypedArray array = context.getResources().obtainTypedArray(resId);
        int[] colors = new int[array.length()];
        for (int i = 0; i < array.length(); i++) {
            colors[i] = array.getColor(i, Color.TRANSPARENT);
        }
        array.recycle();
        return colors;
    }

    @Override
    protected synchronized void onDraw(Canvas canvas) {
        int startX = getPaddingLeft();
        int endX = getWidth() - getPaddingRight();
        int totalWidth = endX - startX;
        int progressWidth = (int) (totalWidth * (getProgress() / (float) getMax()));
        // 设置底色的范围
        setDrawableBounds(backgroundDrawable, startX, totalWidth);
        // 绘制底色
        backgroundDrawable.draw(canvas);
        if (this.isEnabled()) {
            gradientDrawable.setAlpha(this.isEnabled() ? 255 : 76);
            // 设置渐变Drawable的范围
            setDrawableBounds(gradientDrawable, startX, progressWidth);
            // 绘制渐变Drawable
            gradientDrawable.draw(canvas);
        } else {
            // 设置不可用时底色的范围
            setDrawableBounds(unableDrawable, startX, progressWidth);
            // 绘制不可用时底色
            unableDrawable.draw(canvas);
        }

        super.onDraw(canvas);
        //Log.d(TAG, "getPaddingLeft(): " + startX + " getPaddingRight(): " + getPaddingRight() + " getWidth: " + getWidth() + " getHeight(): " + getHeight() + " unableStyle: " + unableStyle);
    }

    public void setCornerRadius(float cornerRadius) {
        this.cornerRadius = cornerRadius;
    }

    public void setSliderHeight(float sliderHeight) {
        this.sliderHeight = sliderHeight;
    }
}