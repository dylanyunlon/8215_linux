package com.autochips.bluetooth.util;

import android.graphics.Paint;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.widget.TextView;

import androidx.annotation.NonNull;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

/**
* pzhou
* 实现逻辑：将 需要跑马灯的Textview放入容器，从前往后开始取
* 在取到Textview后判断文本长度是否大于控件宽度 
* 如果大于，就开始执行跑马灯
* ######循环实现
* 通过反射查看是否执行完成，执行完成后找下一个同样处理
**/
public class HMarqueeUtil {
    private final String TAG = "HMarqueeUtil";

    private int mId = 0;
    private int mFailCount = 0;
    private boolean bRun = true;
    private Handler mHandler = null;
    private TextView mMarqueeTv;
    private List<TextView> mData = new ArrayList<>();

    public HMarqueeUtil(Handler handler) {
        this.mHandler = handler;
    }


    public void add(TextView tv) {
        if (!mData.contains(tv)) {
            mData.add(tv);
        }
    }

    private final int MSG_INIT_VIEW = 1;
    private final int MSG_CHECK_STATE = 2;

    private Handler handler = new Handler() {
        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_INIT_VIEW:
                    if (mId >= mData.size()) {
                        mId = 0;
                    }
                    if (!nextMarquee()) {
                        mId++;
                        mFailCount ++;
                        if(mFailCount >= mData.size()){
                            return;
                        }
                        sendEmptyMessageDelayed(MSG_INIT_VIEW, 1000);
                    } else {
                        mFailCount = 0;
                        sendEmptyMessageDelayed(MSG_CHECK_STATE, 1000);
                    }

                    break;
                case MSG_CHECK_STATE:
                    if (isFinish(mMarqueeTv)) {
                        mMarqueeTv.setSelected(false);
                        mId++;
                        sendEmptyMessageDelayed(MSG_INIT_VIEW, 1000);
                    } else {
                        sendEmptyMessageDelayed(MSG_CHECK_STATE, 3000);
                    }
                    break;
                default:
                    break;
            }
        }
    };

    public void stopMarquee(){
        if(handler.hasMessages(MSG_CHECK_STATE)) {
            handler.removeMessages(MSG_CHECK_STATE);
        }
        if(handler.hasMessages(MSG_INIT_VIEW)) {
            handler.removeMessages(MSG_INIT_VIEW);
        }
    }
    public void startMarquee() {
        if (mData.size() == 0) {
            return;
        }
        mFailCount = 0;
        mId = 0;
        handler.sendEmptyMessageDelayed(MSG_INIT_VIEW, 500);
    }

    public void reStart() {
        Log.d(TAG, "reStart:");
        if (mMarqueeTv != null) {
            mMarqueeTv.setSelected(false);
        }
        if(handler.hasMessages(MSG_CHECK_STATE)) {
            handler.removeMessages(MSG_CHECK_STATE);
        }
        if(handler.hasMessages(MSG_INIT_VIEW)) {
            handler.removeMessages(MSG_INIT_VIEW);
        }
        startMarquee();
    }


	/**
	*判断当前控件是否符合跑马灯要求
	**/
    private boolean nextMarquee() {
        mMarqueeTv = mData.get(mId);
        if (mMarqueeTv == null) {
            Log.d(TAG, "nextMarquee: null");
            return false;
        }
        int viewWidth = mMarqueeTv.getWidth();
        Log.w(TAG, "nextMarquee: viewWidth: " + viewWidth );
        String txt = mMarqueeTv.getText().toString();
        Log.w(TAG, " , txt :" + txt);
        if (viewWidth <= 0 || TextUtils.isEmpty(txt)) {
            return false;
        }
        float txtWidth = mMarqueeTv.getPaint().measureText(txt);
        Log.w(TAG, " , txtWidth :" + txtWidth);
        if ((txtWidth - viewWidth) < 10) {
            Log.d(TAG, "nextMarquee: < 10");
            return false;
        }
        mMarqueeTv.setSelected(true);
        return true;
    }

	/**
	*判断是否执行完
	**/
    private boolean isFinish(TextView tv) {
        if (mMarqueeTv == null) {
            return true;
        }
        try {
            Class cls = tv.getClass();
            Field marquee = cls.getDeclaredField("mMarquee");
            marquee.setAccessible(true);
            Object mMarquee = marquee.get(tv);
            if (mMarquee == null) {
                Log.d(TAG, "isFinish: mMarquee is null");
                return true;
            }
            //
            Class cls2 = mMarquee.getClass();
            Field state = cls2.getDeclaredField("mStatus");
            state.setAccessible(true);
            Object mState = state.get(mMarquee);
            Log.d(TAG, "isFinish: " + mState);
            byte curState = (byte) mState;
            if (curState == 2 || curState == 1) {
                return false;
            } else if (curState == 0) {
                return true;
            }
        } catch (NoSuchFieldException e) {
            Log.d(TAG, "isFinish: " + e.getMessage());
        } catch (IllegalAccessException e) {
            Log.d(TAG, "isFinish: " + e.getMessage());
        } catch (IllegalArgumentException e) {
            Log.d(TAG, "isFinish: " + e.getMessage());
        } catch (NullPointerException e) {
            Log.d(TAG, "isFinish: " + e.getMessage());
        }

        return true;
    }

}
