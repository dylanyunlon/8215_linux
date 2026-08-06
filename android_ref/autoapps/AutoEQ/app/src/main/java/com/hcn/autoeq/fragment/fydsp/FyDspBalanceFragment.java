package com.hcn.autoeq.fragment.fydsp;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.EventMessage;
import com.hcn.autoeq.bean.FyDspBalanceMode;
import com.hcn.autoeq.bean.FyDspOutputMode;
import com.hcn.autoeq.data.FyDspBalanceSettings;
import com.hcn.autoeq.data.FyDspHLPFSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.view.BalanceView;
import com.hcn.autoeq.view.BalanceViewCallback;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

public class FyDspBalanceFragment extends BaseFragment implements View.OnClickListener, BalanceViewCallback {

    private static final String TAG = FyDspBalanceFragment.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(FyDspBalanceFragment.class.getSimpleName(), Log.DEBUG);

    private View mainView;
    private BalanceView balanceView;
    private Button btnBalanceModeMain, btnBalanceModeCo, btnBalanceModeRear, btnBalanceModeWhole;
    private Button btnBalanceModeLeft, btnBalanceModeCenter, btnBalanceModeRight;
    private Button btnBalanceFront, btnBalanceRear, btnBalanceLeft, btnBalanceRight;
    private Button btnAdvance;

    private FyDspBalanceSettings fyDspBalanceSettings;
    private FyDspHLPFSettings fyDspHLPFSettings;
    private FyDspOutputMode fyDspOutputMode;

    public FyDspBalanceFragment() {
    }

    public static FyDspBalanceFragment newInstance() {
        FyDspBalanceFragment fragment = new FyDspBalanceFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fydsp_fragment_balance;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        fyDspBalanceSettings = FyDspBalanceSettings.getInstance(mContext);
        fyDspHLPFSettings = FyDspHLPFSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void onStart() {
        super.onStart();
        startBalanceView();
    }

    @Override
    public void onStop() {
        super.onStop();
        stopBalanceView();
    }

    @Override
    public void initView() {
        balanceView = mainView.findViewById(R.id.id_eq_balance);
        balanceView.setEqBalanceListener(this);

        btnBalanceModeMain = mainView.findViewById(R.id.balance_mode_main);
        btnBalanceModeCo = mainView.findViewById(R.id.balance_mode_co);
        btnBalanceModeRear = mainView.findViewById(R.id.balance_mode_rear);
        btnBalanceModeWhole = mainView.findViewById(R.id.balance_mode_whole);
        btnBalanceModeLeft = mainView.findViewById(R.id.balance_mode_left);
        btnBalanceModeCenter = mainView.findViewById(R.id.balance_mode_center);
        btnBalanceModeRight = mainView.findViewById(R.id.balance_mode_right);

        btnBalanceFront = mainView.findViewById(R.id.balance_front);
        btnBalanceRear = mainView.findViewById(R.id.balance_rear);
        btnBalanceLeft = mainView.findViewById(R.id.balance_left);
        btnBalanceRight = mainView.findViewById(R.id.balance_right);

        btnBalanceModeMain.setOnClickListener(this);
        btnBalanceModeCo.setOnClickListener(this);
        btnBalanceModeRear.setOnClickListener(this);
        btnBalanceModeWhole.setOnClickListener(this);
        btnBalanceModeLeft.setOnClickListener(this);
        btnBalanceModeCenter.setOnClickListener(this);
        btnBalanceModeRight.setOnClickListener(this);

        btnBalanceFront.setOnClickListener(this);
        btnBalanceRear.setOnClickListener(this);
        btnBalanceLeft.setOnClickListener(this);
        btnBalanceRight.setOnClickListener(this);

        btnAdvance = mainView.findViewById(R.id.btn_advance);
        btnAdvance.setOnClickListener(this);
    }

    @Override
    public void initData() {
        super.initData();
        fyDspOutputMode = fyDspHLPFSettings.getOutputMode();
        refreshModeButtons();
        refreshBalanceView();
    }

    private void refreshModeButtons() {
        btnBalanceModeMain.setVisibility((fyDspOutputMode == FyDspOutputMode.CHANNEL51 || fyDspOutputMode == FyDspOutputMode.WAY6) ? View.VISIBLE : View.INVISIBLE);
        btnBalanceModeCo.setVisibility((fyDspOutputMode == FyDspOutputMode.CHANNEL51 || fyDspOutputMode == FyDspOutputMode.WAY6) ? View.VISIBLE : View.INVISIBLE);
        btnBalanceModeRear.setVisibility((fyDspOutputMode == FyDspOutputMode.CHANNEL51 || fyDspOutputMode == FyDspOutputMode.WAY6) ? View.VISIBLE : View.INVISIBLE);
        btnBalanceModeWhole.setVisibility((fyDspOutputMode == FyDspOutputMode.CHANNEL51 || fyDspOutputMode == FyDspOutputMode.WAY6) ? View.VISIBLE : View.INVISIBLE);

        btnBalanceModeLeft.setVisibility((fyDspOutputMode == FyDspOutputMode.CHANNEL51 || fyDspOutputMode == FyDspOutputMode.WAY6) ? View.INVISIBLE : View.VISIBLE);
        btnBalanceModeCenter.setVisibility((fyDspOutputMode == FyDspOutputMode.CHANNEL51 || fyDspOutputMode == FyDspOutputMode.WAY6) ? View.INVISIBLE : View.VISIBLE);
        btnBalanceModeRight.setVisibility((fyDspOutputMode == FyDspOutputMode.CHANNEL51 || fyDspOutputMode == FyDspOutputMode.WAY6) ? View.INVISIBLE : View.VISIBLE);

        btnBalanceFront.setVisibility((fyDspOutputMode == FyDspOutputMode.CHANNEL51 || fyDspOutputMode == FyDspOutputMode.WAY6) ? View.VISIBLE : View.INVISIBLE);
        btnBalanceRear.setVisibility((fyDspOutputMode == FyDspOutputMode.CHANNEL51 || fyDspOutputMode == FyDspOutputMode.WAY6) ? View.VISIBLE : View.INVISIBLE);
    }

    private void refreshBalanceView() {
        int[] balance = fyDspBalanceSettings.getBalance();
        balanceView.setBalanceLevel(balance[0], balance[1], false);
        balanceView.setHorizontalOnly(fyDspOutputMode == FyDspOutputMode.WAY2 || fyDspOutputMode == FyDspOutputMode.WAY3);
    }

    private void resetBalanceView() {
        balanceView.setHorizontalOnly(fyDspOutputMode == FyDspOutputMode.WAY2 || fyDspOutputMode == FyDspOutputMode.WAY3);
        if (fyDspOutputMode == FyDspOutputMode.CHANNEL51 || fyDspOutputMode == FyDspOutputMode.WAY6) { // 5.1/6 声道时，回到全车模式
            btnBalanceModeWhole.performClick();
        } else {
            btnBalanceModeCenter.performClick();
        }
    }

    private void startBalanceView() {
        if (balanceView != null) {
            balanceView.monitorThreadStart();
        }
    }

    private void stopBalanceView() {
        if (balanceView != null) {
            balanceView.monitorThreadStop();
        }
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.balance_mode_main:
                // 会自动回调 onMotionChanged 和 onMotionFinished 事件
                balanceView.setBalanceLevel(FyDspBalanceMode.MAIN.getX(), FyDspBalanceMode.MAIN.getY(), true);
                break;
            case R.id.balance_mode_co:
                balanceView.setBalanceLevel(FyDspBalanceMode.CO.getX(), FyDspBalanceMode.CO.getY(), true);
                break;
            case R.id.balance_mode_rear:
                balanceView.setBalanceLevel(FyDspBalanceMode.REAR.getX(), FyDspBalanceMode.REAR.getY(), true);
                break;
            case R.id.balance_mode_whole:
                balanceView.setBalanceLevel(FyDspBalanceMode.WHOLE.getX(), FyDspBalanceMode.WHOLE.getY(), true);
                break;
            case R.id.balance_mode_left:
                balanceView.setBalanceLevel(FyDspBalanceMode.LEFT.getX(), FyDspBalanceMode.LEFT.getY(), true);
                break;
            case R.id.balance_mode_center:
                balanceView.setBalanceLevel(FyDspBalanceMode.CENTER.getX(), FyDspBalanceMode.CENTER.getY(), true);
                break;
            case R.id.balance_mode_right:
                balanceView.setBalanceLevel(FyDspBalanceMode.RIGHT.getX(), FyDspBalanceMode.RIGHT.getY(), true);
                break;
            case R.id.balance_front:
                balanceView.moveFront();
                break;
            case R.id.balance_rear:
                balanceView.moveRear();
                break;
            case R.id.balance_left:
                balanceView.moveLeft();
                break;
            case R.id.balance_right:
                balanceView.moveRight();
                break;
            case R.id.btn_advance:
                fyDspBalanceSettings.saveBalanceOrDelayUIMode(FyDspBalanceSettings.UI_MODE.DELAY);
                EventBus.getDefault().post(new EventMessage(EventMessage.MSG_BALANCE_CHANGE_TO_DELAY));
                return;
            default:
                break;
        }
        onSaveData(balanceView.getBalanceLevel(balanceView.getCurrentX(), balanceView.getCurrentY()).x, balanceView.getBalanceLevel(balanceView.getCurrentX(), balanceView.getCurrentY()).y);
    }

    @Override
    public void onMotionBegin() {

    }

    @Override
    public void onMotionChanged(int x, int y, boolean bUpdate) {
        if (DEBUG) {
            Log.d(TAG, String.format("onMotionChanged x : %d, y : %d, bUpdate : %s", x, y, bUpdate));
        }
        if (bUpdate) {
            fyDspBalanceSettings.nativeBalance(x, y, fyDspOutputMode);
        }
    }

    @Override
    public void onMotionFinished(int x, int y, boolean bUpdate) {
        if (DEBUG) {
            Log.d(TAG, String.format("onMotionFinished x : %d, y : %d", x, y));
        }
        if (bUpdate) {
            EventMessage.anyChanged(mContext, TAG + "_" + "onMotionFinished");
        }
    }

    @Override
    public void onSaveData(int x, int y) {
        if (DEBUG) {
            Log.d(TAG, String.format("onSaveData x : %d, y : %d", x, y));
        }
        fyDspBalanceSettings.saveBalance(FyDspBalanceMode.WHOLE.getName(), x, y);
    }

    @Override
    @Subscribe(sticky = true)
    public void onEvent(EventMessage eventMessage) {
        super.onEvent(eventMessage);
        if (EventMessage.MSG_STICKY_OUTPUT_MODE_CHANGED.equals(eventMessage.getMessage())) {
            fyDspOutputMode = (FyDspOutputMode) eventMessage.getData();
            Log.d(TAG, "MSG_STICKY_OUTPUT_MODE_CHANGED change to " + fyDspOutputMode);
            refreshModeButtons();
            resetBalanceView();
        } else if (EventMessage.MSG_BALANCE_CHANGE_TO_DELAY.equals(eventMessage.getMessage())) {

        } else if (EventMessage.MSG_DELAY_CHANGE_TO_BALANCE.equals(eventMessage.getMessage())) {
            Log.d(TAG, "MSG_DELAY_CHANGE_TO_BALANCE");
            resetBalanceView();
        } else if (EventMessage.MSG_STICKY_USER_MODE_CHANGED.equals(eventMessage.getMessage())) {
            Log.d(TAG, "MSG_STICKY_USER_MODE_CHANGED");
            initData();
        }
    }
}