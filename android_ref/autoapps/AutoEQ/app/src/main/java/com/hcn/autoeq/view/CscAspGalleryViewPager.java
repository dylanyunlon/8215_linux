package com.hcn.autoeq.view;

import android.content.Context;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.viewpager.widget.ViewPager;

import com.hcn.autoeq.R;
import com.hcn.autoeq.util.SkinUtils;

/**
 * 重写 viewpager 的 touch 事件
 * 实现点击左右两边的 view，将 view 移动到中间
 * 点击中间区域，触发 click 事件
 */
public class CscAspGalleryViewPager extends ViewPager {

    private static final String TAG = CscAspGalleryViewPager.class.getSimpleName();

    // 判断是滑动还是点击的距离差值
    private final static float DISTANCE = 10;
    private float downX;
    private float downY;
    private View viewOnActionDown = null;
    private long lastTouchTime;

    public CscAspGalleryViewPager(Context context) {
        super(context);
    }

    public CscAspGalleryViewPager(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    private OnItemClickListener onItemClickListener;

    public interface OnItemClickListener {
        void onItemClick(View view, int position);

        //新增点击无效情况
        void onItemInvalidClick();

        //展示画廊
        void showViewPager();

        //关闭画廊
        void closeViewPager();
    }

    public void setOnItemClickListener(OnItemClickListener onItemClickListener) {
        this.onItemClickListener = onItemClickListener;
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {
        return true;
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        // 请求父容器不要拦截本控件的事件
        getParent().requestDisallowInterceptTouchEvent(true);

        if (ev.getAction() == MotionEvent.ACTION_DOWN) {
            Log.d(TAG, "ACTION_DOWN");
            if ((SystemClock.elapsedRealtime() - lastTouchTime) < 500) {
                Log.d(TAG, "touch fast, do nothing!");
                return true;
            }
            lastTouchTime = SystemClock.elapsedRealtime();

            downX = ev.getX();
            downY = ev.getY();

            // 找寻按下去时对应的view，并设置按钮为按下状态效果
            viewOnActionDown = viewOfClickOnScreen(ev);
            if (viewOnActionDown != null) {
                TextView btn = (TextView) viewOnActionDown.findViewById(SkinUtils.getId(R.id.btn_image));
                btn.setPressed(true);
            }

        } else if (ev.getAction() == MotionEvent.ACTION_UP) {
            Log.d(TAG, "ACTION_UP");
            // 松开时恢复按钮状态
            if (viewOnActionDown != null) {
                TextView btn = (TextView) viewOnActionDown.findViewById(SkinUtils.getId(R.id.btn_image));
                btn.setPressed(false);
            }

            float upX = ev.getX();
            float upY = ev.getY();
            //如果 up的位置和down 的位置 距离 > 设置的距离,则事件继续传递,不执行下面的点击切换事件
            if (Math.abs(upX - downX) > DISTANCE || Math.abs(upY - downY) > DISTANCE) {
                return super.dispatchTouchEvent(ev);
            }

            View view = viewOfClickOnScreen(ev);
            if (view != null) {
                int position = (Integer) view.getTag();
                if (getCurrentItem() != position) {
                    setCurrentItem(position);
//                    return true; // 此处不能拦截，否则会导致某些子控件依然保持pressed状态
                } else {
                    TextView btn = (TextView) view.findViewById(SkinUtils.getId(R.id.btn_image));
                    btn.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            Log.d(TAG, "onItemClick tag : " + v.getTag() + ", " + getCurrentItem());
                            // 只有点击的是中间的项目才生效点击事件，修复点一个按钮导致多个按钮同时触发点击事件，触发了其他按钮的点击事件
                            if ((Integer) v.getTag() == getCurrentItem() && onItemClickListener != null) {
                                onItemClickListener.onItemClick(v, (int) v.getTag());
                            }
                        }
                    });
                    btn.performClick();
                    return true; // 拦截子控件的事件，避免重复点击事件
                }
            } else {
                //点击无效
                onItemClickListener.onItemInvalidClick();
            }
        } else if (ev.getAction() == MotionEvent.ACTION_CANCEL) {
            // 取消事件时恢复按钮状态
            if (viewOnActionDown != null) {
                TextView btn = (TextView) viewOnActionDown.findViewById(SkinUtils.getId(R.id.btn_image));
                btn.setPressed(false);
            }
        } else if (ev.getAction() == MotionEvent.ACTION_MOVE) {
            Log.d(TAG, "ACTION_MOVE");
        }
        return super.dispatchTouchEvent(ev);
    }

    /**
     * 根据点击的x，y坐标，确定是点击的是哪一页
     *
     * @param ev
     * @return
     */
    private View viewOfClickOnScreen(MotionEvent ev) {
        int childCount = getChildCount();
        int[] location = new int[2];
        for (int i = 0; i < childCount; i++) {
            View view = getChildAt(i);
            int position = (Integer) view.getTag();
            view.getLocationOnScreen(location);
            int minX = location[0];
            int minY = location[1];

            int maxX = location[0] + view.getWidth();
            int maxY = location[1] + view.getHeight();

            float x = ev.getRawX();
            float y = ev.getRawY();


            if ((x > minX && x < maxX) && (y > minY && y < maxY)) {
                return view;
            }
        }
        return null;
    }


    @Override
    protected void onVisibilityChanged(@NonNull View changedView, int visibility) {
        super.onVisibilityChanged(changedView, visibility);
        if (onItemClickListener == null) {
            return;
        }
        if (visibility == VISIBLE) {
            onItemClickListener.showViewPager();
        } else if (visibility == GONE) {
            onItemClickListener.closeViewPager();
        }
    }
}