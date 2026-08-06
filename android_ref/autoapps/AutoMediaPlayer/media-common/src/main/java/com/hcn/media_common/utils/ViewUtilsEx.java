package com.hcn.media_common.utils;

import android.view.View;

import java.util.Objects;

/**
 * 视图工具
 * <pre>
 *    想积累一个积累一个 View 工具类，让部分常用代码更加简介；
 *    区别于类 {@link androidx.appcompat.widget.ViewUtils};
 * </pre>
 *
 * @author 65821
 */
public class ViewUtilsEx {

    /**
     * 私有构造函数
     * <p> 禁止工具类被实例化；
     */
    private ViewUtilsEx() {}

    /**
     * 判定目标视图是否是可视状态；
     *
     * @param view 需要判定的视图
     * @return 是/否
     */
    public static boolean isVisible(View view) {
        if (Objects.isNull(view)) {
            return false;
        } else {
            return view.getVisibility() == View.VISIBLE;
        }
    }

    /**
     * 判定目标视图是否是输入的可视标记；
     *
     * @param view 需要判定的视图
     * @param visibility 可视标记 {@link View#GONE,View#INVISIBLE,View#VISIBLE}
     * @return 是/否
     */
    public static boolean isVisible(View view, int visibility) {
        if (Objects.isNull(view)) {
            return false;
        }

        return view.getVisibility() == visibility;
    }
}
