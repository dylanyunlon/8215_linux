package com.hcn.media_view.widget;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Px;
import androidx.core.view.GravityCompat;
import androidx.core.view.ViewCompat;
import androidx.customview.widget.ViewDragHelper;

import com.hcn.media_view.compat.DrawerLayout;

import java.lang.reflect.Field;

/**
 * DrawerLayout 扩展
 * <pre>
 *    抽屉布局的扩展，使抽屉拉开的时候，抽屉外的按钮也能点击；
 *
 *    更多炫酷侧滑效果请参考如下网址：
 *        https://zhuanlan.zhihu.com/p/75946497;
 * </pre>
 *
 * @author 65821
 */
public class DrawerLayoutEx extends DrawerLayout {

    /**
     * 当前视图构建状态监听
     * <p> 测量、布局、绘制状态等；
     */
    private IStateListener mStateListener = null;

    public DrawerLayoutEx(@NonNull Context context) {
        super(context);
    }

    public DrawerLayoutEx(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public DrawerLayoutEx(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {
        // 如果是点击到了 Top 视图，则不再传给父类处理。
        if (ev.getAction() == MotionEvent.ACTION_DOWN) {
            final float x = ev.getX();
            final float y = ev.getY();
            final View touchedView = findTopChildUnder((int) x, (int) y);

            if (touchedView != null
                    && isContentView(touchedView)
                    && isDrawerOpen(GravityCompat.END)) {
                return false;
            }
        }

        return super.onInterceptTouchEvent(ev);
    }

    /**
     * 判断点击位置是否位于相应的 View 内
     *
     * @param x 横坐标
     * @param y 纵坐标
     * @return 返回当前坐标命中的 view 对象
     */
    public View findTopChildUnder(int x, int y) {
        final int childCount = getChildCount();
        for (int i = childCount - 1; i >= 0; i--){
            final View child = getChildAt(i);
            if (x >= child.getLeft() && x < child.getRight() &&
                    y >= child.getTop() && y < child.getBottom()) {
                return child;
            }
        }
        return null;
    }

    /**
     * 判断点击触摸点的 View 是否是 ContentView 视图；
     * <p> ContentView 视图表示主页面的 View 元素（不是 ViewGroup）；
     *
     * @param child 子 view
     * @return boolean 是/否
     */
    boolean isContentView(View child) {
        return ((LayoutParams) child.getLayoutParams()).gravity == Gravity.NO_GRAVITY;
    }

    /**
     * 判断点击触摸点的 View 是否是 DrawerView
     * <p> DrawerView 就是侧边栏的 View
     *
     * @param child 子 view
     * @return boolean 是/否
     */
    boolean isDrawerView(View child) {
        int gravity = ((LayoutParams) child.getLayoutParams()).gravity;
        int absGravity = GravityCompat.getAbsoluteGravity(
                gravity, ViewCompat.getLayoutDirection(child));

        if ((absGravity & Gravity.START) != 0) {
            // This child is a left-edge drawer
            return true;
        }

        // This child is a right-edge drawer
        return (absGravity & Gravity.END) != 0;
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        if (mStateListener != null) {
            mStateListener.onMeasured();
        }
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        super.onLayout(changed, l, t, r, b);

        if (mStateListener != null) {
            mStateListener.onLayouted();
        }
    }

    /**
     * 设置左侧拖动器边缘感应大小
     * <pre>
     *     注意 DrawerLayout 的 onLayout 会调整边缘大小到默认值；
     *     所以，最好视图在初始化的时候就调整好触摸边界的感应默认值大小；
     * </pre>
     *
     * @param newEdgeSize 边缘感应大小
     */
    public void setLeftDragEdgeSize(@Px @IntRange(from = 0) int newEdgeSize) {
        try {
            // 左侧拖拽器
            Field draggerField = DrawerLayout.class.getDeclaredField("mLeftDragger");
            draggerField.setAccessible(true);
            ViewDragHelper vdh = (ViewDragHelper) draggerField.get(this);

            // 设置边缘值
            Field edgeSizeField = ViewDragHelper.class.getDeclaredField("mEdgeSize");
            edgeSizeField.setAccessible(true);
            edgeSizeField.setInt(vdh, newEdgeSize);

            // 设置默认值
            Field defaultEdgeSizeField = ViewDragHelper.class.getDeclaredField("mDefaultEdgeSize");
            defaultEdgeSizeField.setAccessible(true);
            defaultEdgeSizeField.setInt(vdh, newEdgeSize);
        } catch (Exception ignored) {
        }
    }

    /**
     * 设置右侧拖动器边缘感应大小
     * <pre>
     *     注意 DrawerLayout 的 onLayout 会调整边缘大小到默认值；
     *     所以，最好视图在初始化的时候就调整好触摸边界的感应默认值大小；
     * </pre>
     *
     * @param newEdgeSize 边缘感应大小
     */
    public void setRightDragEdgeSize(@Px @IntRange(from = 0) int newEdgeSize) {
        try {
            // 左侧拖拽器
            Field draggerField = DrawerLayout.class.getDeclaredField("mRightDragger");
            draggerField.setAccessible(true);
            ViewDragHelper vdh = (ViewDragHelper) draggerField.get(this);

            // 设置边缘值
            Field edgeSizeField = ViewDragHelper.class.getDeclaredField("mEdgeSize");
            edgeSizeField.setAccessible(true);
            edgeSizeField.setInt(vdh, newEdgeSize);

            // 设置默认值
            Field defaultEdgeSizeField = ViewDragHelper.class.getDeclaredField("mDefaultEdgeSize");
            defaultEdgeSizeField.setAccessible(true);
            defaultEdgeSizeField.setInt(vdh, newEdgeSize);
        } catch (Exception ignored) {
        }
    }

    @Override
    public void onDraw(Canvas c) {
        super.onDraw(c);

        if (mStateListener != null) {
            mStateListener.onDrawed();
        }
    }

    /**
     * 抽屉布局事件状态回调
     * <p> 用来监听当前布局的测量、布局、绘制状态；
     */
    public interface IStateListener {
        /**
         * 已测量
         * <p> 在 {@link DrawerLayoutEx#onMeasure(int, int)} 最后调用；
         */
        void onMeasured();

        /**
         * 已布局
         * <p> 在 {@link DrawerLayoutEx#onLayout(boolean, int, int, int, int)} 最后调用；
         */
        void onLayouted();

        /**
         * 已绘制
         * <p> 在 {@link DrawerLayoutEx#onDraw(Canvas)} 最后调用；
         */
        void onDrawed();
    }

    /**
     * 设置当前视图构建状态监听
     * <p> 用来监听视图的各个构建状态完成时机；
     *
     * @param listener 监听对象
     */
    public void setBuildStateListener(IStateListener listener) {
        mStateListener = listener;
    }
}
