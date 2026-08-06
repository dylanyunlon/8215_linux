package com.hcn.autoeq.nine;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import com.hcn.autoeq.R;
import com.hcn_library.BaseFragment;
import com.hcn_library.data.NineDspBalanceSettings;
import com.hcn_library.util.SkinUtils;
import com.hcn_library.view.BalanceView;
import com.hcn_library.view.BalanceViewCallback;

public class NineDspBalanceFragment extends BaseFragment implements View.OnTouchListener, BalanceViewCallback {
    private BalanceView balanceView;
    private Button btnCo;
    private Button btnMain;
    private Button btnRear;
    private Button btnWhole;
    private View mainView;
    private NineDspBalanceSettings nineDspBalanceSettings;
    private static final String TAG = "NineDspBalanceFragment";


    @Override
    public int getLayoutRes() {
        return R.layout.nine_dsp_fragment_balance;
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    public static NineDspBalanceFragment newInstance() {
        return new NineDspBalanceFragment();
    }

    @Override
    public View onCreateView(LayoutInflater layoutInflater, ViewGroup viewGroup, Bundle bundle) {
        mainView = super.onCreateView(layoutInflater, viewGroup, bundle);
        nineDspBalanceSettings = NineDspBalanceSettings.getInstance(mContext);
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
        balanceView = (BalanceView) mainView.findViewById(SkinUtils.getId(R.id.id_eq_balance));
        Log.d(TAG, "balanceView is null:" + (balanceView == null) + " skin id: " + SkinUtils.getId(R.id.id_eq_balance) + " id: " + R.id.id_eq_balance);
        balanceView.setEqBalanceListener(this);
        startBalanceView();
        int[] balance = nineDspBalanceSettings.getBalance();
        balanceView.setBalanceLevel(balance[0], balance[1], false);
        btnMain = (Button) mainView.findViewById(SkinUtils.getId(R.id.balance_mode_main));
        btnCo = (Button) mainView.findViewById(SkinUtils.getId(R.id.balance_mode_co));
        btnRear = (Button) mainView.findViewById(SkinUtils.getId(R.id.balance_mode_rear));
        btnWhole = (Button) mainView.findViewById(SkinUtils.getId(R.id.balance_mode_whole));
        btnMain.setOnTouchListener(this);
        btnCo.setOnTouchListener(this);
        btnRear.setOnTouchListener(this);
        btnWhole.setOnTouchListener(this);
        mainView.findViewById(SkinUtils.getId(R.id.balance_front)).setOnTouchListener(this);
        mainView.findViewById(SkinUtils.getId(R.id.balance_rear)).setOnTouchListener(this);
        mainView.findViewById(SkinUtils.getId(R.id.balance_left)).setOnTouchListener(this);
        mainView.findViewById(SkinUtils.getId(R.id.balance_right)).setOnTouchListener(this);
        refreshButtonStatus();
        Log.d(TAG, "initView x: " + balance[0] + " y: " + balance[1]);
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
    public void onMotionBegin() {
        btnMain.setSelected(false);
        btnCo.setSelected(false);
        btnRear.setSelected(false);
        btnWhole.setSelected(false);
    }

    @Override
    public void onMotionChanged(int i, int i2, boolean z) {
        Log.d(TAG, String.format("onMotionChanged x : %d, y : %d, bUpdate : %s", Integer.valueOf(i), Integer.valueOf(i2), Boolean.valueOf(z)));
        if (z) {
            nineDspBalanceSettings.nativeBalance(i, i2);
        }
    }

    @Override
    public void onMotionFinished(int i, int i2, boolean z) {
        Log.d(TAG, String.format("onMotionFinished x : %d, y : %d, bUpdate : %s", Integer.valueOf(i), Integer.valueOf(i2), Boolean.valueOf(z)));
    }

    @Override
    public void onSaveData(int i, int i2) {
        Log.d(TAG, String.format("onSaveData x : %d, y : %d", Integer.valueOf(i), Integer.valueOf(i2)));
        nineDspBalanceSettings.saveBalance(NineDspBalanceSettings.BALANCE_MODE.WHOLE.getName(), i, i2);
        refreshButtonStatus();
    }

    private void refreshButtonStatus() {
        btnMain.setSelected(false);
        btnCo.setSelected(false);
        btnRear.setSelected(false);
        btnWhole.setSelected(false);
        int i = nineDspBalanceSettings.getBalance()[0];
        int i2 = nineDspBalanceSettings.getBalance()[1];
        if (i == NineDspBalanceSettings.BALANCE_MODE.MAIN.getX() && i2 == NineDspBalanceSettings.BALANCE_MODE.MAIN.getY()) {
            btnMain.setSelected(true);
        } else if (i == NineDspBalanceSettings.BALANCE_MODE.CO.getX() && i2 == NineDspBalanceSettings.BALANCE_MODE.CO.getY()) {
            btnCo.setSelected(true);
        } else if (i == NineDspBalanceSettings.BALANCE_MODE.REAR.getX() && i2 == NineDspBalanceSettings.BALANCE_MODE.REAR.getY()) {
            btnRear.setSelected(true);
        } else if (i == NineDspBalanceSettings.BALANCE_MODE.WHOLE.getX() && i2 == NineDspBalanceSettings.BALANCE_MODE.WHOLE.getY()) {
            btnWhole.setSelected(true);
        }
        if (i == NineDspBalanceSettings.BALANCE_MODE.WHOLE.getX() && i2 == NineDspBalanceSettings.BALANCE_MODE.WHOLE.getY()) {
            balanceView.setBackground(SkinUtils.getDrawable(R.drawable.nine_balance_car_bg_with_coord_whole));
        } else {
            balanceView.setBackground(SkinUtils.getDrawable(R.drawable.nine_balance_car_bg_with_coord));
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            int id = v.getId();
            if (id == SkinUtils.getId(R.id.balance_mode_main)) {
                balanceView.setBalanceLevel(NineDspBalanceSettings.BALANCE_MODE.MAIN.getX(), NineDspBalanceSettings.BALANCE_MODE.MAIN.getY(), true);
            } else if (id == SkinUtils.getId(R.id.balance_mode_co)) {
                balanceView.setBalanceLevel(NineDspBalanceSettings.BALANCE_MODE.CO.getX(), NineDspBalanceSettings.BALANCE_MODE.CO.getY(), true);
            } else if (id == SkinUtils.getId(R.id.balance_mode_rear)) {
                balanceView.setBalanceLevel(NineDspBalanceSettings.BALANCE_MODE.REAR.getX(), NineDspBalanceSettings.BALANCE_MODE.REAR.getY(), true);
            } else if (id == SkinUtils.getId(R.id.balance_mode_whole)) {
                balanceView.setBalanceLevel(NineDspBalanceSettings.BALANCE_MODE.WHOLE.getX(), NineDspBalanceSettings.BALANCE_MODE.WHOLE.getY(), true);
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
        return false;
    }
}