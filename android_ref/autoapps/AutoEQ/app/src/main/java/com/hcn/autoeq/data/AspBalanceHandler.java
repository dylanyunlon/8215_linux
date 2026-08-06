package com.hcn.autoeq.data;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;

import com.hcn.autoeq.R;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.BalanceView;
import com.hcn.autoeq.view.BalanceViewCallback;

import java.util.Timer;
import java.util.TimerTask;

/**
 * 本类并没有继承Handler,它只是一个帮助 EqUIMain 分担业务的处理类
 */
public class AspBalanceHandler {

    private static final String TAG = AspBalanceHandler.class.getSimpleName();

    private Context mContext;
    private View mAspUIMain;
    private BalanceView mEqBalance;
    private Timer mLongClickTimer = null;
    private TimerTask mLcTimerTask = null;
    private Handler mHandler = new EqBalanceButtsHandler();
    private LCTask mCurrTask = LCTask.ACTION_CURR_TASK;
    private int mMotionX = 0, mMotionY = 0;

    // move action
    private final static int move_balance_none = 0;
    private final static int move_balance_left = 1;
    private final static int move_balance_righ = 2;
    private final static int move_balance_fron = 4;
    private final static int move_balance_rear = 8;
    private final static int move_balance_mxfl = 5;
    private final static int move_balance_mxfr = 6;
    private final static int move_balance_mxrl = 9;
    private final static int move_balance_mxrr = 10;

    private final static int WM_LONG_CLICK_TASK = 1;

    public AspBalanceHandler(Context context, View mUIMain) {
        mContext = context;
        this.mAspUIMain = mUIMain;
        init();
    }

    private void init() {

        mEqBalance = (BalanceView) mAspUIMain.findViewById(SkinUtils.getId(R.id.id_eq_balance));

        mEqBalance.setEqBalanceListener(new AutoEqBalanceListener());
    }


    /**
     * 只刷新UI,不执行监听事件，没有执行其他任何操作。
     */
    public void update() {

        int[] balance = AspSettings.getInstance(mContext).getAspBalance();
        //刷新EqBalanceView 参数false表示不执行监听事件
        mEqBalance.setBalanceLevel(balance[0] + 7, 7 - balance[1], false);
    }

    public void reset() {
        mEqBalance.setBalanceLevel(7, 7, true);
    }

    public void setBalance(int x, int y) {
        mEqBalance.setBalanceLevel(x, y, true);
    }

    // Eq Balance Changed
    private final class AutoEqBalanceListener implements BalanceViewCallback {

        @Override
        public void onMotionBegin() {
        }

        @Override
        public void onMotionChanged(int x, int y, boolean bUpdate) {
            Log.d(TAG, "onMotionChanged: x==" + x + "  y==" + y + "  bUpdate==" + bUpdate);
            if (bUpdate) {
                mMotionX = x - 7;
                mMotionY = 7 - y;
                AspSettings.getInstance(mContext).setAspBalance(mMotionX, mMotionY, false);
            }
        }

        @Override
        public void onMotionFinished(int x, int y, boolean bUpdate) {
            AspSettings.getInstance(mContext).setAspBalance(mMotionX, mMotionY, true);
        }

        @Override
        public void onSaveData(int x, int y) {
            mMotionX = x - 7;
            mMotionY = 7 - y;
            AspSettings.getInstance(mContext).setAspBalance(mMotionX, mMotionY, true);
        }

    }

    private void checkLongClickTask() {

        if (move_balance_none == mCurrTask.value()) {
            stopLongClickTask();
        } else {
            startLongClickTask();
        }
    }

    private void startLongClickTask() {

        if (null == mLongClickTimer) {
            mLongClickTimer = new Timer();
            if (null == mLcTimerTask) {
                mLcTimerTask = new EqBalanceButtsTimerTask();
            }
            mLongClickTimer.schedule(mLcTimerTask, 100, 300);
        }
    }

    private void stopLongClickTask() {

        if (null != mLongClickTimer) {
            mLongClickTimer.cancel();
            mLongClickTimer = null;
        }
        if (null != mLcTimerTask) {
            mLcTimerTask.cancel();
            mLcTimerTask = null;
        }
    }


    private final class EqBalanceButtsTimerTask extends TimerTask {

        @Override
        public void run() {

            if (mHandler != null) {
                Message message = mHandler.obtainMessage(WM_LONG_CLICK_TASK);
                mHandler.sendMessage(message);
            }
        }
    }

    @SuppressLint("HandlerLeak")
    private final class EqBalanceButtsHandler extends Handler {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case WM_LONG_CLICK_TASK:
                    handleLongClickTask();
                    break;
                default:
                    break;
            }
        }
    }

    private void handleLongClickTask() {

        if (mLongClickTimer == null || mLcTimerTask == null) return;

        switch (mCurrTask.value()) {
            case move_balance_none:
                stopLongClickTask();
                break;
            case move_balance_left:
                onEqBalanceLeft();
                break;
            case move_balance_righ:
                onEqBalanceRight();
                break;
            case move_balance_fron:
                onEqBalanceFront();
                break;
            case move_balance_rear:
                onEqBalanceRear();
                break;
            case move_balance_mxfl:
                onEqBalanceFLTask();
                break;
            case move_balance_mxfr:
                onEqBalanceFRTask();
                break;
            case move_balance_mxrl:
                onEqBalanceRLTask();
                break;
            case move_balance_mxrr:
                onEqBalanceRRTask();
                break;
            default:
                break;
        }
    }

    // onClick
    public void onEqBalanceLeft() {
        if (null != mEqBalance) {
            mEqBalance.moveLeft();
        }
    }

    public void onEqBalanceRight() {
        if (null != mEqBalance) {
            mEqBalance.moveRight();
        }
    }

    public void onEqBalanceFront() {
        if (null != mEqBalance) {
            mEqBalance.moveFront();
        }
    }

    public void onEqBalanceRear() {
        if (null != mEqBalance) {
            mEqBalance.moveRear();
        }
    }

    // hide method
    private void onEqBalanceFLTask() {
        if (null != mEqBalance) {
            mEqBalance.moveFrontLeft();
        }
    }

    private void onEqBalanceFRTask() {
        if (null != mEqBalance) {
            mEqBalance.moveFrontRight();
        }
    }

    public void saveData(int x, int y) {
        mMotionX = x - 7;
        mMotionY = 7 - y;
        AspSettings.getInstance(mContext).setAspBalance(mMotionX, mMotionY, true);
    }

    private void onEqBalanceRLTask() {
        if (null != mEqBalance) {
            mEqBalance.moveRearLeft();
        }
    }

    private void onEqBalanceRRTask() {
        if (null != mEqBalance) {
            mEqBalance.moveRearRight();
        }
    }

    // onLongClick
    public void onLongEqBalanceLeft() {
        mCurrTask.mixTask(LCTask.ACTION_LEFT_TASK);
        startLongClickTask();
    }

    public void onLongEqBalanceRight() {
        mCurrTask.mixTask(LCTask.ACTION_RIGHT_TASK);
        startLongClickTask();
    }

    public void onLongEqBalanceFront() {
        mCurrTask.mixTask(LCTask.ACTION_FRONT_TASK);
        startLongClickTask();
    }

    public void onLongEqBalanceRear() {
        mCurrTask.mixTask(LCTask.ACTION_REAR_TASK);
        startLongClickTask();
    }

    // ActionUp
    public void onActionUpEqBalanceLeft() {
        mCurrTask.revokeTask(LCTask.ACTION_LEFT_TASK);
        checkLongClickTask();
    }

    public void onActionUpEqBalanceRight() {
        mCurrTask.revokeTask(LCTask.ACTION_RIGHT_TASK);
        checkLongClickTask();
    }

    public void onActionUpEqBalanceFront() {
        mCurrTask.revokeTask(LCTask.ACTION_FRONT_TASK);
        checkLongClickTask();
    }

    public void onActionUpEqBalanceRear() {
        mCurrTask.revokeTask(LCTask.ACTION_REAR_TASK);
        checkLongClickTask();
    }

    public enum LCTask {
        ACTION_NONE_TASK(0), ACTION_LEFT_TASK(1), ACTION_RIGHT_TASK(2), ACTION_FRONT_TASK(
                4), ACTION_REAR_TASK(8), ACTION_FL_TASK(5), ACTION_FR_TASK(6), ACTION_RL_TASK(
                9), ACTION_RR_TASK(10), ACTION_CURR_TASK(0);

        private int mTask;
        private int mWeight;

        private LCTask(int nTask) {
            mTask = nTask;
            mWeight = nTask;
        }

        private int value() {
            return mTask;
        }

        private int weight() {
            return mWeight;
        }


        public LCTask mixTask(LCTask task) {

            switch (mTask) {
                case 0: // none
                    switch (mWeight) {
                        case 0:
                            mTask = task.value();
                            mWeight = task.weight();
                            break;
                        case 3:
                            switch (task.value()) {
                                case 4:
                                    mTask = 4;
                                    mWeight = 7;
                                    break;
                                case 8:
                                    mTask = 8;
                                    mWeight = 11;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case 12:
                            switch (task.value()) {
                                case 1:
                                    mTask = 1;
                                    mWeight = 13;
                                    break;
                                case 2:
                                    mTask = 2;
                                    mWeight = 14;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case 1: // left
                    switch (mWeight) {
                        case 1:
                            switch (task.value()) {
                                case 2: // right
                                    mTask = 0;
                                    mWeight = 3;
                                    break;
                                case 4: // front
                                    mTask = 5;
                                    mWeight = 5;
                                    break;
                                case 8: // rear
                                    mTask = 9;
                                    mWeight = 9;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case 13:
                            switch (task.value()) {
                                case 2:
                                    mTask = 0;
                                    mWeight = 15;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case 2: // right
                    switch (mWeight) {
                        case 2:
                            switch (task.value()) {
                                case 1:
                                    mTask = 0;
                                    mWeight = 3;
                                    break;
                                case 4:
                                    mTask = 6;
                                    mWeight = 6;
                                    break;
                                case 8:
                                    mTask = 10;
                                    mWeight = 10;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case 14:
                            switch (task.value()) {
                                case 1:
                                    mTask = 0;
                                    mWeight = 15;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case 4: // front
                    switch (mWeight) {
                        case 4:
                            switch (task.value()) {
                                case 1: // left
                                    mTask = 5;
                                    mWeight = 5;
                                    break;
                                case 2: // right
                                    mTask = 6;
                                    mWeight = 6;
                                    break;
                                case 8: // rear
                                    mTask = 0;
                                    mWeight = 12;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case 7:
                            switch (task.value()) {
                                case 8:
                                    mTask = 0;
                                    mWeight = 15;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case 8: // rear
                    switch (mWeight) {
                        case 8:
                            switch (task.value()) {
                                case 1: // left
                                    mTask = 9;
                                    mWeight = 9;
                                    break;
                                case 2: // right
                                    mTask = 10;
                                    mWeight = 10;
                                    break;
                                case 4: // front
                                    mTask = 0;
                                    mWeight = 12;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        case 11:
                            switch (task.value()) {
                                case 4:
                                    mTask = 0;
                                    mWeight = 15;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case 5: // fl
                    switch (task.value()) {
                        case 2: // right
                            mTask = 4;
                            mWeight = 7;
                            break;
                        case 8: // rear
                            mTask = 1;
                            mWeight = 13;
                            break;
                        default:
                            break;
                    }
                    break;
                case 6: // fr
                    switch (task.value()) {
                        case 1: // left
                            mTask = 4;
                            mWeight = 7;
                            break;
                        case 8: // rear
                            mTask = 2;
                            mWeight = 14;
                            break;
                        default:
                            break;
                    }
                    break;
                case 9: // rl
                    switch (task.value()) {
                        case 2: // right
                            mTask = 8;
                            mWeight = 11;
                            break;
                        case 4: // front
                            mTask = 1;
                            mWeight = 13;
                            break;
                        default:
                            break;
                    }
                    break;
                case 10: // rr
                    switch (task.value()) {
                        case 1: // left
                            mTask = 8;
                            mWeight = 11;
                            break;
                        case 4: // front
                            mTask = 2;
                            mWeight = 14;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }

            return this;
        }


        public LCTask revokeTask(LCTask task) {

            switch (mWeight) {
                case 1:
                    switch (task.value()) {
                        case 1:
                            mTask = 0;
                            mWeight = 0;
                            break;
                        default:
                            break;
                    }
                    break;
                case 2:
                    switch (task.value()) {
                        case 2:
                            mTask = 0;
                            mWeight = 0;
                            break;
                        default:
                            break;
                    }
                    break;
                case 4:
                    switch (task.value()) {
                        case 4:
                            mTask = 0;
                            mWeight = 0;
                            break;
                        default:
                            break;
                    }
                    break;
                case 8:
                    switch (task.value()) {
                        case 8:
                            mTask = 0;
                            mWeight = 0;
                            break;
                        default:
                            break;
                    }
                    break;
                case 5:
                    switch (task.value()) {
                        case 1:
                            mTask = 4;
                            mWeight = 4;
                            break;
                        case 4:
                            mTask = 1;
                            mWeight = 1;
                            break;
                        default:
                            break;
                    }
                    break;
                case 6:
                    switch (task.value()) {
                        case 2:
                            mTask = 4;
                            mWeight = 4;
                            break;
                        case 4:
                            mTask = 2;
                            mWeight = 2;
                            break;
                        default:
                            break;
                    }
                    break;
                case 9:
                    switch (task.value()) {
                        case 1:
                            mTask = 8;
                            mWeight = 8;
                            break;
                        case 8:
                            mTask = 1;
                            mWeight = 1;
                            break;
                        default:
                            break;
                    }
                    break;
                case 10:
                    switch (task.value()) {
                        case 2:
                            mTask = 8;
                            mWeight = 8;
                            break;
                        case 8:
                            mTask = 2;
                            mWeight = 2;
                            break;
                        default:
                            break;
                    }
                    break;
                case 3:
                    switch (task.value()) {
                        case 1:
                            mTask = 2;
                            mWeight = 2;
                            break;
                        case 2:
                            mTask = 1;
                            mWeight = 1;
                            break;
                        default:
                            break;
                    }
                    break;
                case 12:
                    switch (task.value()) {
                        case 4:
                            mTask = 8;
                            mWeight = 8;
                            break;
                        case 8:
                            mTask = 4;
                            mWeight = 4;
                            break;
                        default:
                            break;
                    }
                    break;
                case 7:
                    switch (task.value()) {
                        case 1:
                            mTask = 6;
                            mWeight = 6;
                            break;
                        case 2:
                            mTask = 5;
                            mWeight = 5;
                            break;
                        case 4:
                            mTask = 0;
                            mWeight = 3;
                            break;
                        default:
                            break;
                    }
                    break;
                case 14:
                    switch (task.value()) {
                        case 8:
                            mTask = 6;
                            mWeight = 6;
                            break;
                        case 2:
                            mTask = 0;
                            mWeight = 12;
                            break;
                        case 4:
                            mTask = 10;
                            mWeight = 10;
                            break;
                        default:
                            break;
                    }
                    break;
                case 11:
                    switch (task.value()) {
                        case 1:
                            mTask = 10;
                            mWeight = 10;
                            break;
                        case 2:
                            mTask = 9;
                            mWeight = 9;
                            break;
                        case 8:
                            mTask = 0;
                            mWeight = 3;
                            break;
                        default:
                            break;
                    }
                    break;
                case 13:
                    switch (task.value()) {
                        case 1:
                            mTask = 0;
                            mWeight = 12;
                            break;
                        case 4:
                            mTask = 9;
                            mWeight = 9;
                            break;
                        case 8:
                            mTask = 5;
                            mWeight = 5;
                            break;
                        default:
                            break;
                    }
                    break;
                case 15:
                    switch (task.value()) {
                        case 1:
                            mTask = 2;
                            mWeight = 14;
                            break;
                        case 2:
                            mTask = 1;
                            mWeight = 13;
                            break;
                        case 4:
                            mTask = 8;
                            mWeight = 11;
                            break;
                        case 8:
                            mTask = 4;
                            mWeight = 7;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }

            return this;
        }
    }
}
