package com.hcn.autoeq.util;

import android.os.Build;
import android.view.View;

import androidx.viewpager.widget.ViewPager;

/**
 * 其实核心代码就是这个动画实现部分，这里设置了一个最大缩放和最小缩放比例，
 * 当处于最中间的 view 往左边滑动时，它的 position 值是小于0的，
 * 并且是越来越小,它右边的 view 的 position 是从1逐渐减小到0的
 */
public class ScalePageTransformer implements ViewPager.PageTransformer {
    //缩放倍率
    public static final float MIN_SCALE = 0.15f;
    //偏移量影响界面范围
    public static final float TRANSLATION_PAGE_NUMBER = 4;


    /**
     * 核心就是实现transformPage(View page, float position)这个方法
     **/
    @Override
    public void transformPage(View page, float position) {
        //偏移的量
        int wight = page.getWidth();
        float offsetDistance = (float) wight * MIN_SCALE / 2;
        //偏移倍率-系数绝对值
        float offsetCoefficient = Math.abs(position);

        //以中心点为瞄点，随着页面偏移，页面会逐渐缩小；
        page.setScaleY(1 - MIN_SCALE * offsetCoefficient);
        page.setScaleX(1 - MIN_SCALE * offsetCoefficient);

        //但是由于缩小，会导致页面距离变大，因此需要做纠正
        float translationX = position * offsetDistance * Math.min(offsetCoefficient, TRANSLATION_PAGE_NUMBER);
        page.setTranslationX(-1 * translationX);

        //透明度，随着倍率的调整，明度化变化(需要优化)
        float alpha = 1.0f;
        //有问题，mcc400mnc109-1920x720会隐藏第三个和第五个图标
        //不同的分辨率和编号下图标个数可能不同，下面方法不适用于所有的情况，暂时隐藏掉
//        if (2.0f < position && position <= 4.0f) {
//            alpha = (3.0f - position) > 0 ? (3.0f - position) : 0f;
//        } else if (-4.0f <= position && position < -2.0f) {
//            alpha = (3.0f + position) > 0 ? (3.0f + position) : 0f;
//        }
        page.setAlpha(alpha);

        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            page.getParent().requestLayout();
        }
    }
}