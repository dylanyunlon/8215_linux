package com.hcn.autoeq.fragment.cscasp;


import static com.hcn.autoeq.data.CscAspBalanceSettings.CSC_ASP_BALANCE_DISTANCE;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.CscAspBalanceSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.BalanceView;
import com.hcn.autoeq.view.BalanceViewCallback;

/**
 * csc_asp平衡
 */
public class CscAspBalanceFragment extends BaseFragment
        implements View.OnClickListener, BalanceViewCallback {
    private static final String TAG = CscAspBalanceFragment.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(CscAspBalanceFragment.class.getSimpleName(), Log.DEBUG);
    private View mainView;

    //重置按钮
    private Button btnCscAspBalanceReset;
    private Button btnCscAspBalanceFront;
    private Button btnCscAspBalanceRear;
    private Button btnCscAspBalanceLeft;
    private Button btnCscAspBalanceRight;

    private TextView tvCscAspBalanceFrontValue;
    private TextView tvCscAspBalanceRearValue;
    private TextView tvCscAspBalanceLeftValue;
    private TextView tvCscAspBalanceRightValue;


    private BalanceView idCscAspEqBalanceView;

    private CscAspBalanceSettings cscAspBalanceSettings;

    public static CscAspBalanceFragment newInstance() {
        CscAspBalanceFragment fragment = new CscAspBalanceFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.csc_asp_balance_fragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        cscAspBalanceSettings = CscAspBalanceSettings.getInstance(mContext);
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
        idCscAspEqBalanceView = mainView.findViewById(SkinUtils.getId(R.id.id_csc_asp_eq_balance));
        idCscAspEqBalanceView.setEqBalanceListener(this);
        //按钮
        btnCscAspBalanceFront = mainView.findViewById(SkinUtils.getId(R.id.btn_csc_asp_balance_front));
        btnCscAspBalanceRear = mainView.findViewById(SkinUtils.getId(R.id.btn_csc_asp_balance_rear));
        btnCscAspBalanceLeft = mainView.findViewById(SkinUtils.getId(R.id.btn_csc_asp_balance_left));
        btnCscAspBalanceRight = mainView.findViewById(SkinUtils.getId(R.id.btn_csc_asp_balance_right));
        btnCscAspBalanceReset = mainView.findViewById(SkinUtils.getId(R.id.btn_csc_asp_balance_reset));
        btnCscAspBalanceFront.setOnClickListener(this);
        btnCscAspBalanceRear.setOnClickListener(this);
        btnCscAspBalanceLeft.setOnClickListener(this);
        btnCscAspBalanceRight.setOnClickListener(this);
        btnCscAspBalanceReset.setOnClickListener(this);

        //文本
        tvCscAspBalanceFrontValue = mainView.findViewById(SkinUtils.getId(R.id.tv_csc_asp_balance_front_value));
        tvCscAspBalanceRearValue = mainView.findViewById(SkinUtils.getId(R.id.tv_csc_asp_balance_rear_value));
        tvCscAspBalanceLeftValue = mainView.findViewById(SkinUtils.getId(R.id.tv_csc_asp_balance_left_value));
        tvCscAspBalanceRightValue = mainView.findViewById(SkinUtils.getId(R.id.tv_csc_asp_balance_right_value));

        startBalanceView();
        int[] balance = cscAspBalanceSettings.getCscAspBalance();
        idCscAspEqBalanceView.setBalanceLevel(balance[0], balance[1], false);
        updateBalanceValue(balance[0], balance[1]);

    }

    /**
     * 更新平衡的数值
     */
    public void updateBalanceValue(int x, int y) {
        int FrontValue = CSC_ASP_BALANCE_DISTANCE - y;
        int RearValue = y - CSC_ASP_BALANCE_DISTANCE;
        int LeftValue = CSC_ASP_BALANCE_DISTANCE - x;
        int RightValue = x - CSC_ASP_BALANCE_DISTANCE;
        if (tvCscAspBalanceFrontValue != null) {
            tvCscAspBalanceFrontValue.setText(String.valueOf(FrontValue));
        }
        if (tvCscAspBalanceRearValue != null) {
            tvCscAspBalanceRearValue.setText(String.valueOf(RearValue));
        }
        if (tvCscAspBalanceLeftValue != null) {
            tvCscAspBalanceLeftValue.setText(String.valueOf(LeftValue));
        }
        if (tvCscAspBalanceRightValue != null) {
            tvCscAspBalanceRightValue.setText(String.valueOf(RightValue));
        }
    }

    private void startBalanceView() {
        if (idCscAspEqBalanceView != null) {
            idCscAspEqBalanceView.monitorThreadStart();
        }
    }

    private void stopBalanceView() {
        if (idCscAspEqBalanceView != null) {
            idCscAspEqBalanceView.monitorThreadStop();
        }
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        if (id == SkinUtils.getId(R.id.btn_csc_asp_balance_front)) {
            idCscAspEqBalanceView.moveFront();
        } else if (id == SkinUtils.getId(R.id.btn_csc_asp_balance_rear)) {
            idCscAspEqBalanceView.moveRear();
        } else if (id == SkinUtils.getId(R.id.btn_csc_asp_balance_left)) {
            idCscAspEqBalanceView.moveLeft();
        } else if (id == SkinUtils.getId(R.id.btn_csc_asp_balance_right)) {
            idCscAspEqBalanceView.moveRight();
        } else if (id == SkinUtils.getId(R.id.btn_csc_asp_balance_reset)) {
            idCscAspEqBalanceView.setBalanceLevel(CSC_ASP_BALANCE_DISTANCE, CSC_ASP_BALANCE_DISTANCE, true);
        }
        onSaveData(idCscAspEqBalanceView.getBalanceLevel(idCscAspEqBalanceView.getCurrentX(), idCscAspEqBalanceView.getCurrentY()).x, idCscAspEqBalanceView.getBalanceLevel(idCscAspEqBalanceView.getCurrentX(), idCscAspEqBalanceView.getCurrentY()).y);
    }

    @Override
    public void onMotionBegin() {

    }

    @Override
    public void onMotionChanged(int x, int y, boolean bUpdate) {
        Log.d(TAG, String.format("onMotionChanged x : %d, y : %d, bUpdate : %s", x, y, bUpdate));
        if (bUpdate) {
            cscAspBalanceSettings.setCscAspBalance(x, y, bUpdate);
        }
        updateBalanceValue(x, y);
    }

    @Override
    public void onMotionFinished(int x, int y, boolean bUpdate) {
        updateBalanceValue(x, y);
    }

    @Override
    public void onSaveData(int x, int y) {
        cscAspBalanceSettings.saveCscAspBalance(x, y);
    }
}
