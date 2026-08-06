package com.hcn.bluetooth.view;

import android.bluetooth.BluetoothHeadsetClientCall;
import android.content.Context;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.media.AudioManager;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.LinearLayout;

import com.hcn.bluetooth.bean.CacheData;
import com.hcn.bluetooth.bean.CallInfo;
import com.hcn.bluetooth.skin.SkinUtils;

import java.util.List;
import java.util.Map;

public abstract class PhoneCallViewBase extends LinearLayout implements View.OnTouchListener {
    protected Context mContext;
    protected AudioManager mAudioManager;
    protected WindowManager mWindowManager;
    protected WindowManager.LayoutParams mLayoutParams;
    protected View mViewRoot;
    protected boolean isAddView = false;
    /**
     * 是否允许显示当前通话框,适用于HiCar连接时,HiCar会显示通话状态,不需要蓝牙弹框
     */
    protected boolean mEnable = true;

    //用于处理拖动
    protected boolean mMoveFlag = false;
    protected Rect rect = new Rect();
    protected int lastX, lastY;
    protected int paramX, paramY;

    public PhoneCallViewBase(Context context, WindowManager wm, AudioManager am) {
        super(context);
        mContext = context;
        mWindowManager = wm;
        mAudioManager = am;

        mLayoutParams = new WindowManager.LayoutParams();
        //[注意：这个窗口的 type 不要修改，SurfaceFlinger 会用到来做分层显示]
        mLayoutParams.type = WindowManager.LayoutParams.TYPE_DRAG;
        mLayoutParams.format = PixelFormat.RGBA_8888;
        mLayoutParams.gravity = Gravity.CENTER;
        mLayoutParams.width = WindowManager.LayoutParams.WRAP_CONTENT;
        mLayoutParams.height = WindowManager.LayoutParams.WRAP_CONTENT;
        mLayoutParams.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
        //compatible with 8227 skin apk
        int layoutId = getLayoutId();
        if (layoutId > 0) {
            mViewRoot = SkinUtils.getLayout(getLayoutId(),this);
            mViewRoot.setOnTouchListener(this);
            initView();
        }
    }

    protected abstract int getLayoutId();

    protected abstract void initView();

    public abstract void onActionCallStateChanged(List<BluetoothHeadsetClientCall> callList);

    public abstract void updateAudioState(int state);

    public abstract void updateCallTime(Map<String, CallInfo> callTimeMap);

    public void showView() {
        if (!mEnable) {
            return;
        }


        boolean isAndroidAutoMode= CacheData.getInstance().isAndroidAutoMode();
        Log.d("PhoneCallViewBase", "showView isAndroidAutoMode="+isAndroidAutoMode);
        if(isAndroidAutoMode){
            Log.d("PhoneCallViewBase", "showView return isAndroidAutoMode="+isAndroidAutoMode);
            return;
        }


        if (!isAddView) {
            isAddView = true;
            try {
                mWindowManager.addView(this, mLayoutParams);
            } catch (Exception e) {

            }
        }
    }

    public void hideView() {
        if (isAddView) {
            try {
                mWindowManager.removeView(this);
                isAddView = false;
            } catch (Exception e) {

            }
        }
    }

    public void updateView() {
        if (isAddView) {
            try {
                mWindowManager.updateViewLayout(this, mLayoutParams);
            } catch (Exception e) {

            }
        }
    }

    public boolean isAddView() {
        return isAddView;
    }

    public void setEnable(boolean enable) {
        mEnable = enable;
    }

    @Override
    public boolean onTouch(View view, MotionEvent event) {
        final int action = event.getAction();
        float x = event.getRawX();
        float y = event.getRawY();

        switch (action) {
            case MotionEvent.ACTION_DOWN:
                lastX = (int) x;
                lastY = (int) y;
                paramX = mLayoutParams.x;
                paramY = mLayoutParams.y;
                mMoveFlag = false;
                break;
            case MotionEvent.ACTION_MOVE:
                int dx = (int) (x - lastX);
                int dy = (int) (y - lastY);
                if ((mLayoutParams.gravity & Gravity.BOTTOM) == Gravity.BOTTOM) {
                    dy = -dy;
                }
                calculateLayoutParamXY(paramX + dx, paramY + dy);
                if (Math.abs(dx) > 10 || Math.abs(dy) > 10) {
                    mMoveFlag = true;
                    updateView();
                }
                break;
            case MotionEvent.ACTION_UP:
                if (mMoveFlag) {
                    calculateLayoutParamXY(mLayoutParams.x, mLayoutParams.y);
                    updateView();
                }
                break;
            default:
                break;
        }
        return true;
    }

    public void calculateLayoutParamXY(int x, int y) {
        getWindowVisibleDisplayFrame(rect);
        //通话框居中显示,x_move_range表示向左和向右可移动的距离
        int x_move_range = (int) ((rect.right - rect.left - getWidth()) * 0.5f);
        if (Math.abs(x) <= x_move_range) {
            mLayoutParams.x = x;
        } else {
            mLayoutParams.x = Integer.signum(x) * x_move_range;
        }
        if ((mLayoutParams.gravity & Gravity.BOTTOM) == Gravity.BOTTOM
                || (mLayoutParams.gravity & Gravity.TOP) == Gravity.TOP) {
            mLayoutParams.y = Math.min(Math.max(0, y), rect.bottom - rect.top - getHeight());
        } else if ((mLayoutParams.gravity & Gravity.CENTER_VERTICAL) == Gravity.CENTER_VERTICAL) {
            int y_move_range = (int) ((rect.bottom - rect.top - getHeight()) * 0.5f);
            if (Math.abs(y) <= y_move_range) {
                mLayoutParams.y = y;
            } else {
                mLayoutParams.y = Integer.signum(y) * y_move_range;
            }
        }
    }
}
