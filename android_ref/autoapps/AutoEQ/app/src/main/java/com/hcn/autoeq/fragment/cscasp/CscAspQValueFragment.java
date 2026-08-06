package com.hcn.autoeq.fragment.cscasp;


import android.graphics.Point;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.SeekBar;
import android.widget.TextView;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.CscAspEqualizerChartSettings;
import com.hcn.autoeq.data.CscAspQValueSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.CscAspPopupWindow;
import com.hcn.autoeq.view.CscAspQValueView;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.util.Arrays;

/**
 * hcn_asp乐谱
 */
public class CscAspQValueFragment extends BaseFragment
        implements View.OnClickListener {

    private static final String TAG = CscAspQValueFragment.class.getSimpleName();
    private View mainView;

    private Point point = new Point(0, 0);
    private Button btnQLow;
    private Button btnQMiddle;
    private Button btnQHigh;
    private CscAspQValueView qValueView;

    private CscAspPopupWindow mQLowPopWindow;
    private CscAspPopupWindow mQMiddlePopWindow;
    private CscAspPopupWindow mQHighPopWindow;


    /**
     * 音频效果设定类
     */
    private CscAspQValueSettings cscAspQValueSettings;
    /**
     * 音频效果设定类
     */
    private CscAspEqualizerChartSettings cscAspEqualizerChartSettings;

    private int[] QValue = {0, 0, 0};
    private int[] QValuePopWinDowX = {0, 0, 0};
    private int[] QValuePopWinDowY = {0, 0, 0};

    public static CscAspQValueFragment newInstance() {
        CscAspQValueFragment fragment = new CscAspQValueFragment();
        return fragment;
    }

    @Override
    public void initView() {
        qValueView = mainView.findViewById(SkinUtils.getId(R.id.caqv_q));
        btnQLow = mainView.findViewById(SkinUtils.getId(R.id.btn_q_low));
        btnQMiddle = mainView.findViewById(SkinUtils.getId(R.id.btn_q_middle));
        btnQHigh = mainView.findViewById(SkinUtils.getId(R.id.btn_q_high));
        btnQLow.setOnClickListener(this);
        btnQMiddle.setOnClickListener(this);
        btnQHigh.setOnClickListener(this);


        initQLowPopWindow();
        initQMiddlePopWindow();
        initQHighPopWindow();
        dealPopWindow();
        int reverb = cscAspEqualizerChartSettings.getReverb();
        //入场动画
        qValueView.initQValueDate(cscAspQValueSettings.getCscQValue(reverb)[0], cscAspQValueSettings.getCscQValue(reverb)[1], cscAspQValueSettings.getCscQValue(reverb)[2]);
        initViewDate();
    }

    /**
     * 初始化数据
     *
     * @return
     */
    public void initViewDate() {
        int reverb = cscAspEqualizerChartSettings.getReverb();
        if (cscAspQValueSettings != null) {
            QValue[0] = cscAspQValueSettings.getCscQValue(reverb)[0];
            QValue[1] = cscAspQValueSettings.getCscQValue(reverb)[1];
            QValue[2] = cscAspQValueSettings.getCscQValue(reverb)[2];
        }
        //更新数据
        updateViewDate(false, false);
    }


    @Override
    public int getLayoutRes() {
        return R.layout.csc_asp_q_value_fragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        cscAspQValueSettings = CscAspQValueSettings.getInstance(mContext);
        cscAspEqualizerChartSettings = CscAspEqualizerChartSettings.getInstance(mContext);
        return mainView;
    }


    @Override
    public void onClick(View v) {
        if (v.getId() == SkinUtils.getId(R.id.btn_q_low)) {
            showQValuePopWindow(v);
        } else if (v.getId() == SkinUtils.getId(R.id.btn_q_middle)) {
            showQValuePopWindow(v);
        } else if (v.getId() == SkinUtils.getId(R.id.btn_q_high)) {
            showQValuePopWindow(v);
        }
    }

    /**
     * 初始化低音Q的popWindow控件
     */
    private void initQLowPopWindow() {
        View qLowValuePopupView = SkinUtils.inflate(R.layout.csc_asp_q_seekbar);
        mQLowPopWindow = new CscAspPopupWindow(qLowValuePopupView, LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, true);
        TextView tvQValueMin = qLowValuePopupView.findViewById(SkinUtils.getId(R.id.tv_q_value_min));
        TextView tvQValueMax = qLowValuePopupView.findViewById(SkinUtils.getId(R.id.tv_q_value_max));
        tvQValueMin.setText(String.valueOf(0));
        tvQValueMax.setText(String.valueOf(3));

        SeekBar cscAspQLowValueBar = qLowValuePopupView.findViewById(SkinUtils.getId(R.id.csc_asp_q_value_bar));
        cscAspQLowValueBar.setMax(3);
        if ("za10".equals(EqUtils.getSkinName())){
            cscAspQLowValueBar.setThumbOffset(25);
        }else {
            cscAspQLowValueBar.setThumbOffset(6);
        }
        cscAspQLowValueBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int mIndex, boolean forUser) {
                if (mIndex == 0 || mIndex == 1 || mIndex == 2 || mIndex == 3) {
                    QValue[0] = mIndex;
                    updateViewDate(true, forUser);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        mQLowPopWindow.setOnDspPopupListener(new CscAspPopupWindow.OnCscAspopupListener() {
            @Override
            public void UpdatePopupContent() {
                cscAspQLowValueBar.setProgress(QValue[0]);
            }

            @Override
            public void openOrCloseListener(boolean isOpenStatus) {

            }
        });
    }


    /**
     * 初始化中音Q的popWindow弹窗控件
     */
    private void initQMiddlePopWindow() {
        View qMiddleValuePopupView = SkinUtils.inflate(R.layout.csc_asp_q_seekbar);
        mQMiddlePopWindow = new CscAspPopupWindow(qMiddleValuePopupView, LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, true);
        TextView tvQValueMin = qMiddleValuePopupView.findViewById(SkinUtils.getId(R.id.tv_q_value_min));
        TextView tvQValueMax = qMiddleValuePopupView.findViewById(SkinUtils.getId(R.id.tv_q_value_max));
        tvQValueMin.setText(String.valueOf(0));
        tvQValueMax.setText(String.valueOf(3));

        SeekBar cscAspQMiddleValueBar = qMiddleValuePopupView.findViewById(SkinUtils.getId(R.id.csc_asp_q_value_bar));
        cscAspQMiddleValueBar.setMax(3);
        if ("za10".equals(EqUtils.getSkinName())){
            cscAspQMiddleValueBar.setThumbOffset(25);
        }else {
            cscAspQMiddleValueBar.setThumbOffset(6);
        }
        cscAspQMiddleValueBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int mIndex, boolean forUser) {
                if (mIndex == 0 || mIndex == 1 || mIndex == 2 || mIndex == 3) {
                    QValue[1] = mIndex;
                    updateViewDate(true, forUser);
                }

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        mQMiddlePopWindow.setOnDspPopupListener(new CscAspPopupWindow.OnCscAspopupListener() {
            @Override
            public void UpdatePopupContent() {
                cscAspQMiddleValueBar.setProgress(QValue[1]);
            }

            @Override
            public void openOrCloseListener(boolean isOpenStatus) {

            }
        });
    }

    /**
     * 初始化高音Q的popWindow控件
     */
    private void initQHighPopWindow() {
        View qHighValuePopupView = SkinUtils.inflate(R.layout.csc_asp_q_seekbar);
        mQHighPopWindow = new CscAspPopupWindow(qHighValuePopupView, LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, true);
        TextView tvQValueMin = qHighValuePopupView.findViewById(SkinUtils.getId(R.id.tv_q_value_min));
        TextView tvQValueMax = qHighValuePopupView.findViewById(SkinUtils.getId(R.id.tv_q_value_max));
        tvQValueMin.setText(String.valueOf(0));
        tvQValueMax.setText(String.valueOf(1));

        SeekBar cscAspQHighValueBar = qHighValuePopupView.findViewById(SkinUtils.getId(R.id.csc_asp_q_value_bar));
        cscAspQHighValueBar.setMax(1);
        if ("za10".equals(EqUtils.getSkinName())){
            cscAspQHighValueBar.setThumbOffset(25);
        }else {
            cscAspQHighValueBar.setThumbOffset(6);
        }

        cscAspQHighValueBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int mIndex, boolean forUser) {
                if (mIndex == 0 || mIndex == 1 || mIndex == 2 || mIndex == 3) {
                    QValue[2] = mIndex;
                    updateViewDate(true, forUser);
                }

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        mQHighPopWindow.setOnDspPopupListener(new CscAspPopupWindow.OnCscAspopupListener() {
            @Override
            public void UpdatePopupContent() {
                cscAspQHighValueBar.setProgress(QValue[2]);
            }

            @Override
            public void openOrCloseListener(boolean isOpenStatus) {

            }
        });
    }


    /**
     * 显示q值低中高音，进度条PopWindow弹窗
     */
    private void showQValuePopWindow(View view) {
        int id = view.getId();
        if (mQLowPopWindow != null && id == SkinUtils.getId(R.id.btn_q_low)) {

            mQLowPopWindow.showAsDropDown(view.findViewById(SkinUtils.getId(R.id.btn_q_low))
                    , point.x+QValuePopWinDowX[0]
                    , point.y+QValuePopWinDowY[0]
                    , Gravity.TOP);

        }

        if (mQMiddlePopWindow != null && id == SkinUtils.getId(R.id.btn_q_middle)) {

            mQMiddlePopWindow.showAsDropDown(view.findViewById(SkinUtils.getId(R.id.btn_q_middle))
                    , point.x+QValuePopWinDowX[1]
                    , point.y+QValuePopWinDowY[1]
                    , Gravity.TOP);
        }
        if (mQHighPopWindow != null && id == SkinUtils.getId(R.id.btn_q_high)) {

            mQHighPopWindow.showAsDropDown(view.findViewById(SkinUtils.getId(R.id.btn_q_high))
                    , point.x+QValuePopWinDowX[2]
                    , point.y+QValuePopWinDowY[2]
                    , Gravity.TOP);
        }
    }


    /**
     * 获取popWindow弹窗位置，相对点击控件的偏移值
     */
    public void dealPopWindow() {
        ViewTreeObserver vto = btnQLow.getViewTreeObserver();
        vto.addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
                View mContentView =
                        LayoutInflater.from(SkinCompatResources.getInstance().getSkinResId(R.layout.csc_asp_q_seekbar, "layout") != 0
                                        ? SkinUtils.getContext() : mContext)
                                .inflate(SkinUtils.getId(R.layout.csc_asp_q_seekbar), null);
                mContentView.measure(View.MeasureSpec.UNSPECIFIED, View.MeasureSpec.UNSPECIFIED);
                PopupWindow ppw = new PopupWindow();
                ppw.setContentView(mContentView);
                point = new Point(0, 0);
                int x = ppw.getContentView().getMeasuredWidth();
                int y = ppw.getContentView().getMeasuredHeight();
                int x_btn = btnQLow.getMeasuredWidth();
                point.x = (-x + x_btn) / 2;
                point.y = -y * 2 - 15;

                QValuePopWinDowX[0] = SkinCompatResources.getInstance().getInteger(R.integer.csc_asp_q_low_popwindow_x);
                QValuePopWinDowX[1] = SkinCompatResources.getInstance().getInteger(R.integer.csc_asp_q_middle_popwindow_x);
                QValuePopWinDowX[2] = SkinCompatResources.getInstance().getInteger(R.integer.csc_asp_q_high_popwindow_x);

                QValuePopWinDowY[0] = SkinCompatResources.getInstance().getInteger(R.integer.csc_asp_q_low_popwindow_y);
                QValuePopWinDowY[1] = SkinCompatResources.getInstance().getInteger(R.integer.csc_asp_q_middle_popwindow_y);
                QValuePopWinDowY[2] = SkinCompatResources.getInstance().getInteger(R.integer.csc_asp_q_high_popwindow_y);

            }
        });
    }


    /**
     * 更新控件和数据，包括动画
     *
     * @param qValueControl 判断是否是拖动条控制的；
     * @param forUser       判断是否是来自用户主动操作：如主动切换模式，主动重置，主动拖动拖动条
     */
    public void updateViewDate(boolean qValueControl, Boolean forUser) {
        String value;
        if (btnQLow != null) {
            value = QValue[0] + ".0";
            btnQLow.setText(value);
        }
        if (btnQMiddle != null) {
            value = QValue[1] + ".0";
            btnQMiddle.setText(value);
        }
        if (btnQHigh != null) {
            value = QValue[2] + ".0";
            btnQHigh.setText(value);
        }

        if (cscAspQValueSettings == null) {
            return;
        }
        if (qValueView == null) {
            return;
        }

        //如果是来自拖动条控制的话，需要存储数据
        if (qValueControl) {
            cscAspQValueSettings.setCscQValue(QValue[0], QValue[1], QValue[2]);
        }
        //如果是来自主动控制的话，需要动画
        if (forUser) {
            qValueView.qValueControl(QValue[0], QValue[1], QValue[2]);
            qValueView.qValueAnim(qValueView.getAnimSteep(), 3000);
        }

    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if (!hidden) {
            initViewDate();
        }
    }
}
