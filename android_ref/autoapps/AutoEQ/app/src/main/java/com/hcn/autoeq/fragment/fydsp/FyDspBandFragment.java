package com.hcn.autoeq.fragment.fydsp;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.TextView;

import com.auto.hequalizer.EqualizerSurface;
import com.auto.hequalizer.UIMode;
import com.blankj.utilcode.util.ToastUtils;
import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.Band;
import com.hcn.autoeq.bean.EventMessage;
import com.hcn.autoeq.bean.FyDspBandMode;
import com.hcn.autoeq.data.FyDspAttenuateSettings;
import com.hcn.autoeq.data.FyDspBalanceSettings;
import com.hcn.autoeq.data.FyDspBandSettings;
import com.hcn.autoeq.data.FyDspDelaySettings;
import com.hcn.autoeq.data.FyDspHLPFSettings;
import com.hcn.autoeq.data.FyDspHelper;
import com.hcn.autoeq.data.FyDspSurroundSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.ConstantFyDsp;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.FyDspBandSeekBar;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.util.Comparator;
import java.util.List;

public class FyDspBandFragment extends BaseFragment implements ConstantFyDsp, View.OnClickListener
        , SeekBar.OnSeekBarChangeListener, RadioGroup.OnCheckedChangeListener
        , View.OnLongClickListener, FyDspAlertDialog.Callback {

    private static final String TAG = FyDspBandFragment.class.getSimpleName();
    private View mainView;
    private Button btnBandMode, btnResetBand;
    private EqualizerSurface mEqualizerSurface;
    private LinearLayout llSeekBar;
    private TextView tvGainMax, tvGainMin;
    private RadioGroup rgUserMode;

    private FyDspBandSettings fyDspBandSettings;
    private FyDspBandMode fyDspBandMode;
    private List<Band> bands;
    private View longClickView;

    private HorizontalScrollView scrollView;

    private ImageView moveScrollBtn;

    public FyDspBandFragment() {
    }

    public static FyDspBandFragment newInstance() {
        FyDspBandFragment fragment = new FyDspBandFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fydsp_fragment_band;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        fyDspBandSettings = FyDspBandSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        LinearLayout llBand = mainView.findViewById(R.id.ll_band);
        llSeekBar = (LinearLayout) LayoutInflater.from(mContext).inflate(R.layout.fydsp_fragment_band_seekbar_32, null);
        llBand.addView(llSeekBar);

        btnBandMode = mainView.findViewById(R.id.btn_band_mode);
        btnBandMode.setOnClickListener(this);
        btnResetBand = mainView.findViewById(R.id.btn_reset_band);
        btnResetBand.setOnClickListener(this);

        tvGainMax = mainView.findViewById(R.id.tv_gain_max);
        tvGainMin = mainView.findViewById(R.id.tv_gain_min);
        tvGainMax.setText(String.valueOf(DEF_GAIN_PROGRESS_MAX / 2));
        tvGainMin.setText(String.valueOf(DEF_GAIN_PROGRESS_MAX / -2));

        rgUserMode = mainView.findViewById(R.id.rg_user_mode);
        rgUserMode.setOnCheckedChangeListener(this);
        for (int i = 0; i < rgUserMode.getChildCount(); i++) {
            rgUserMode.getChildAt(i).setOnLongClickListener(this);
        }

        scrollView = mainView.findViewById(R.id.hsv_band);
        scrollView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View arg0, MotionEvent arg1) {
                return true;
            }
        });
        moveScrollBtn = mainView.findViewById(R.id.btn_move_scroll);
        if (scrollView.getScrollX() == 0) {
            moveScrollBtn.setImageResource(R.drawable.fydsp_icon_arrow_right);
        } else {
            moveScrollBtn.setImageResource(R.drawable.fydsp_icon_arrow_left);
        }
        moveScrollBtn.setOnClickListener(this);
    }

    @Override
    public void initData() {
        super.initData();
        fyDspBandMode = fyDspBandSettings.getBandMode();
        bands = fyDspBandSettings.getBands(fyDspBandMode);
        initEqualizerSurfaceView();
        refreshEqualizerSurfaceView();
        refreshBandSeekBarStatus();
        refreshBandMode();
        refreshUserMode();
    }

    public void refreshUserMode() {
        String userMode = fyDspBandSettings.getUserMode();
        for (int i = 0; i < rgUserMode.getChildCount(); i++) {
            RadioButton radioButton = (RadioButton) rgUserMode.getChildAt(i);
            if (userMode.equals(radioButton.getTag())) {
                rgUserMode.check(radioButton.getId());
                break;
            }
        }
    }

    private void initEqualizerSurfaceView() {
        mEqualizerSurface = mainView.findViewById(R.id.es_freq);
        // 配置不能拖动
        mEqualizerSurface.setEnabled(false);
        mEqualizerSurface.initConfig("freq_curve_color_2", SkinUtils.getColor(R.color.fydsp_fragment_band_freq_curve_color_1));
        // 配置增益区间
        mEqualizerSurface.setGainRange(DEF_GAIN_PROGRESS_MAX / 2 * -1, DEF_GAIN_PROGRESS_MAX / 2);
        // 配置中心频段个数
        mEqualizerSurface.setCenterFreqBands(bands.size());
        // [测试数据]
        for (int i = 0; i < bands.size(); i++) {
            mEqualizerSurface.setCenterFreqBandsValue(i, bands.get(i).getFreq());
        }
        // 限制图表的增益值为整数
        mEqualizerSurface.setGainRound(true);
        // 配置曲线的光滑度, 也就是曲率, 取整范围: 0.0 ~ 0.3
        mEqualizerSurface.setCurveSmoothness(0.18f);
        // 配置均衡器曲线的显示模式
        mEqualizerSurface.setEqualizerUIMode(UIMode.BEZIER_EQUIDISTANT);
        // 初始化配置
        mEqualizerSurface.initialize();
    }

    private void refreshEqualizerSurfaceView() {
        for (int i = 0; i < bands.size(); i++) {
            mEqualizerSurface.setBand(i, bands.get(i).getGain(), false);
        }
    }

    private void refreshBandSeekBarStatus() {
        for (int i = 0; i < bands.size(); i++) {
            View child = llSeekBar.getChildAt(i);
            if (child instanceof FyDspBandSeekBar) {
                Band band = bands.get(i);
                FyDspBandSeekBar bandSeekBarView = (FyDspBandSeekBar) child;
                bandSeekBarView.setProgress(band.getGain(), false);
                bandSeekBarView.setFreq(band.getFreq());
                bandSeekBarView.setQValue(band.getQ());
                bandSeekBarView.setFragmentManager(getParentFragmentManager());
                bandSeekBarView.setOnSeekBarChangeListener(this);
                bandSeekBarView.setSeekBarStatus(true);
            }
        }
    }

    private void refreshBandMode() {
        btnBandMode.setText(FyDspBandMode.format(mContext, fyDspBandMode));
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_band_mode:
                // 弹出 模式选择 dialog
                FyDspBandModeDialog fyDspBandModeDialog = FyDspBandModeDialog.newInstance();
                fyDspBandModeDialog.show(getParentFragmentManager(), "");
                break;
            case R.id.btn_reset_band:
                // 1.修改临时变量
                fyDspBandMode = DEF_BAND_MODE; // 还原为默认模式
                bands = fyDspBandSettings.getBands(fyDspBandMode); // 还原为默认模式的数据
                // 2.刷新界面
                refreshEqualizerSurfaceView();
                refreshBandSeekBarStatus();
                refreshBandMode();
                EventMessage.anyChanged(mContext, TAG + "_" + "btn_reset_band");
                // 3.保存数据
                fyDspBandSettings.resetCustomBands(); // 还原 custom 模式的数据
                fyDspBandSettings.saveBandMode(fyDspBandMode); // 保存当前模式
                // 4.设置底层音效
                fyDspBandSettings.nativeBands(bands); // 设置音效
                break;
            case R.id.btn_move_scroll:
                if (scrollView.getScrollX() == 0) {
                    scrollView.fullScroll(ScrollView.FOCUS_RIGHT);
                    moveScrollBtn.setImageResource(R.drawable.fydsp_icon_arrow_left);
                } else {
                    scrollView.fullScroll(ScrollView.FOCUS_LEFT);
                    moveScrollBtn.setImageResource(R.drawable.fydsp_icon_arrow_right);
                }
                break;
        }
    }

    @Override
    public void onCheckedChanged(RadioGroup radioGroup, int checkedId) {
        boolean hasChildPressed = false;
        for (int i = 0; i < radioGroup.getChildCount(); i++) {
            if (radioGroup.getChildAt(i).isPressed()) {
                hasChildPressed = true;
            }
        }
        // 界面初始化时也会调用 onCheckedChanged 事件，这个时候不需要改变数据和实际音效
        // 手动点击时才往下执行
        if (!hasChildPressed) {
            return;
        }

        String userMode = (String) radioGroup.findViewById(checkedId).getTag();
        fyDspBandSettings.saveUserMode(userMode);
        fyDspBandSettings.saveBandMode(FyDspBandMode.CUSTOM); // 还原为“自定义“模式

        // 其他 fragment 没用初始化时，收不到 event，所以在点击切换的时候，直接设置所有音效
        FyDspHelper.nativeAllData(mContext); // 设置所有音效

        EventBus.getDefault().postSticky(new EventMessage(EventMessage.MSG_STICKY_USER_MODE_CHANGED, userMode));
    }

    @Override
    @Subscribe(sticky = true)
    public void onEvent(EventMessage eventMessage) {
        super.onEvent(eventMessage);
        if (EventMessage.MSG_BAND_MODE_CHANGED.equals(eventMessage.getMessage())) {
            Log.d(TAG, "MSG_BAND_MODE_CHANGED");
            clearUserModeAnyChanged();
            fyDspBandMode = (FyDspBandMode) eventMessage.getData();
            bands = fyDspBandSettings.getBands(fyDspBandMode);
            // 2.刷新界面
            refreshEqualizerSurfaceView();
            refreshBandSeekBarStatus();
            refreshBandMode();
            // 3.保存数据
            fyDspBandSettings.saveBandMode(fyDspBandMode); // 保存当前模式
            // 4.设置底层音效
            fyDspBandSettings.nativeBands(bands); // 设置音效

        } else if (EventMessage.MSG_BAND_FREQ_CHANGED.equals(eventMessage.getMessage())) {
            Log.d(TAG, "MSG_BAND_FREQ_CHANGED");
            Band band = (Band) eventMessage.getData();

            // 判断频点是否已经存在，需要忽略
            boolean exist = bands.stream().anyMatch(b -> b.getFreq() == band.getFreq());
            if (exist) {
                ToastUtils.showShort(R.string.fydsp_band_freq_exist);
                return;
            }

            change2CustomMode();
            clearUserModeAnyChanged();

            bands.remove(band.getIndex()); // 删除修改前的原始条目
            bands.add(band); // 把修改后的数据添加到集合中
            bands.sort(Comparator.comparingInt(Band::getFreq)); // 按频点排序
            for (int i = 0; i < bands.size(); i++) {
                bands.get(i).setIndex(i); // 更新索引
            }

            refreshEqualizerSurfaceView();
            refreshBandSeekBarStatus();

            fyDspBandSettings.saveBands(bands);
            fyDspBandSettings.nativeBands(bands); // 设置音效
        } else if (EventMessage.MSG_BAND_Q_VALUE_CHANGED.equals(eventMessage.getMessage())) {
            Log.d(TAG, "MSG_BAND_Q_VALUE_CHANGED");
            // 根据回调对象，更新 bands 数据
            Band _band = (Band) eventMessage.getData();
            int _bandIndex = _band.getIndex();

            Band band = bands.get(_bandIndex);
            band.setQ(_band.getQ());

            change2CustomMode();
            clearUserModeAnyChanged();
        } else if (EventMessage.MSG_STICKY_ANY_CHANGED.equals(eventMessage.getMessage())) {
            Log.d(TAG, "MSG_ANY_CHANGED");
            clearUserModeAnyChanged();

            // 比如从用户模式，操作任何数据，都要把当前的数据保存到自定义模式里
            // 保存数据到 sp 中
            fyDspBandSettings.saveBands(bands);
        } else if (EventMessage.MSG_STICKY_USER_MODE_CHANGED.equals(eventMessage.getMessage())) {
            Log.d(TAG, "MSG_STICKY_USER_MODE_CHANGED");
            initData();
        }
    }

    // 增益进度条的回调
    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
//        Log.d(TAG, "onProgressChanged");
        // 进度条拖动的时候，只修改底层音效和临时变量，拖动完毕后，再把临时变量值保存到 sp 文件中
        int _bandIndex = Integer.parseInt((String) seekBar.getTag());

        Band band = bands.get(_bandIndex);
        band.setGain(progress);

        // 拖动进度条时，需要同步更新曲线图
        refreshEqualizerSurfaceView();

        // 设置底层音效
        fyDspBandSettings.nativeBand(band);
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
//        Log.d(TAG, "onStartTrackingTouch");

        change2CustomMode();
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
//        Log.d(TAG, "onStopTrackingTouch");
        EventMessage.anyChanged(mContext, TAG + "_" + "onStopTrackingTouch");

        fyDspBandSettings.saveBands(bands);
    }

    // 有任何操作变化，则清除用户模式
    private void clearUserModeAnyChanged() {
        // 有任何变动，都清除选中状态
        rgUserMode.clearCheck();
        fyDspBandSettings.saveUserMode("");
    }

    // 在预设模式里，进行任何操作，都进入到 custom 模式
    private void change2CustomMode() {
        Log.d(TAG, "change2CustomMode current mode : " + fyDspBandMode.name());
        if (fyDspBandMode != FyDspBandMode.CUSTOM) {
            // 1.修改临时数据，把预设模式的值，复制到 custom 模式
            bands = fyDspBandSettings.getBands(fyDspBandMode);
            fyDspBandMode = FyDspBandMode.CUSTOM;

            // 保存数据到 sp 中
            fyDspBandSettings.saveBands(bands);
            fyDspBandSettings.saveBandMode(fyDspBandMode);

            // 2.更新界面
            // 更新模式按钮为 custom
            refreshBandMode();
        }
    }

    @Override
    public boolean onLongClick(View v) {
        Log.d(TAG, "onLongClick v : " + v.getId());
        this.longClickView = v;
        FyDspAlertDialog fyDspAlertDialog = FyDspAlertDialog.newInstance();
        fyDspAlertDialog.setTitle(getString(R.string.fydsp_band_user_mode_title));
        fyDspAlertDialog.setCallback(this);
        fyDspAlertDialog.show(getParentFragmentManager(), "");
        return true; // return true，不往下执行 onclick 事件
    }

    // 确定保存数据到用户模式
    @Override
    public void onOkClicked() {
        fyDspBandSettings.saveBands(bands); // 如果在非自定义模式（比如 JAZZ），先保存所有的 bands 数据到 sp 里，再复制到 CUSTOM 模式

        String userModeFrom = fyDspBandSettings.getUserMode();
        String userModeTo = (String) rgUserMode.findViewById(longClickView.getId()).getTag();
        // 把所有的数据都复制一份
        FyDspBandSettings.getInstance(mContext).reload(userModeFrom, userModeTo);
        FyDspBalanceSettings.getInstance(mContext).reload(userModeFrom, userModeTo);
        FyDspDelaySettings.getInstance(mContext).reload(userModeFrom, userModeTo);
        FyDspSurroundSettings.getInstance(mContext).reload(userModeFrom, userModeTo);
        FyDspAttenuateSettings.getInstance(mContext).reload(userModeFrom, userModeTo);
        FyDspHLPFSettings.getInstance(mContext).reload(userModeFrom, userModeTo);

        ToastUtils.showShort(R.string.fydsp_band_user_mode_save_ok);
    }

    @Override
    public void onCancelClicked() {

    }
}
