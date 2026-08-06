package com.hcn.autoeq.fragment.siextdsp;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.ExtDspBalanceSettings;
import com.hcn.autoeq.data.SIExtDspBalanceSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.BalanceView;
import com.hcn.autoeq.view.BalanceViewCallback;
/**
 * 和普通extdsp的UI相似，但功能的实现上是不同的，存在一部分区别，
 * 所以应该另外创建setting以及Fragment，方便以后的扩展和修改；
 */
public class SIExtDspBalanceFragment extends BaseFragment implements View.OnClickListener, BalanceViewCallback {

    private static final String TAG = SIExtDspBalanceFragment.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(SIExtDspBalanceFragment.class.getSimpleName(), Log.DEBUG);

    private View mainView;
    private BalanceView balanceView;

    private SIExtDspBalanceSettings siExtDspBalanceSettings;

    public SIExtDspBalanceFragment() {
    }

    public static SIExtDspBalanceFragment newInstance() {
        SIExtDspBalanceFragment fragment = new SIExtDspBalanceFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.extdsp_fragment_balance;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        siExtDspBalanceSettings = SIExtDspBalanceSettings.getInstance(mContext);
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
        balanceView = mainView.findViewById(SkinUtils.getId(R.id.id_eq_balance));
        balanceView.setEqBalanceListener(this);
        startBalanceView();
        int[] balance = siExtDspBalanceSettings.getBalance();
        balanceView.setBalanceLevel(balance[0], balance[1], false);

        mainView.findViewById(SkinUtils.getId(R.id.balance_mode_main)).setOnClickListener(this);
        mainView.findViewById(SkinUtils.getId(R.id.balance_mode_co)).setOnClickListener(this);
        mainView.findViewById(SkinUtils.getId(R.id.balance_mode_rear)).setOnClickListener(this);
        mainView.findViewById(SkinUtils.getId(R.id.balance_mode_whole)).setOnClickListener(this);
        mainView.findViewById(SkinUtils.getId(R.id.balance_front)).setOnClickListener(this);
        mainView.findViewById(SkinUtils.getId(R.id.balance_rear)).setOnClickListener(this);
        mainView.findViewById(SkinUtils.getId(R.id.balance_left)).setOnClickListener(this);
        mainView.findViewById(SkinUtils.getId(R.id.balance_right)).setOnClickListener(this);
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
    public void onClick(View mButtonView) {
        int id = mButtonView.getId();
        if (id == SkinUtils.getId(R.id.balance_mode_main)) {// 会自动回调 onMotionChanged 和 onMotionFinished 事件
            balanceView.setBalanceLevel(ExtDspBalanceSettings.BALANCE_MODE.MAIN.getX(), ExtDspBalanceSettings.BALANCE_MODE.MAIN.getY(), true);
        } else if (id == SkinUtils.getId(R.id.balance_mode_co)) {
            balanceView.setBalanceLevel(ExtDspBalanceSettings.BALANCE_MODE.CO.getX(), ExtDspBalanceSettings.BALANCE_MODE.CO.getY(), true);
        } else if (id == SkinUtils.getId(R.id.balance_mode_rear)) {
            balanceView.setBalanceLevel(ExtDspBalanceSettings.BALANCE_MODE.REAR.getX(), ExtDspBalanceSettings.BALANCE_MODE.REAR.getY(), true);
        } else if (id == SkinUtils.getId(R.id.balance_mode_whole)) {
            balanceView.setBalanceLevel(ExtDspBalanceSettings.BALANCE_MODE.WHOLE.getX(), ExtDspBalanceSettings.BALANCE_MODE.WHOLE.getY(), true);
        } else if (id == SkinUtils.getId(R.id.balance_front)) {
            balanceView.moveFront();
        } else if (id == SkinUtils.getId(R.id.balance_rear)) {
            balanceView.moveRear();
        } else if (id == SkinUtils.getId(R.id.balance_left)) {
            balanceView.moveLeft();
        } else if (id == SkinUtils.getId(R.id.balance_right)) {
            balanceView.moveRight();
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
            siExtDspBalanceSettings.nativeBalance(x, y);
        }
    }

    @Override
    public void onMotionFinished(int x, int y, boolean bUpdate) {
        if (DEBUG) {
            Log.d(TAG, String.format("onMotionFinished x : %d, y : %d, bUpdate : %s", x, y, bUpdate));
        }
    }

    @Override
    public void onSaveData(int x, int y) {
        if (DEBUG) {
            Log.d(TAG, String.format("onSaveData x : %d, y : %d", x, y));
        }
        siExtDspBalanceSettings.saveBalance(SIExtDspBalanceSettings.BALANCE_MODE.WHOLE.getName(), x, y);
    }

}