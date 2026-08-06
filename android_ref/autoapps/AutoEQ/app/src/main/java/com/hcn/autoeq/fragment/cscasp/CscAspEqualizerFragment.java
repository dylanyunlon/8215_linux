package com.hcn.autoeq.fragment.cscasp;


import static com.hcn.autoeq.util.ConstantCscAsp.BAND_TOTAL;
import static com.hcn.autoeq.util.ConstantCscAsp.EXT_CSC_ASP_REVERB_SIZE;
import static com.hcn.autoeq.util.ConstantCscAsp.EXT_CSC_ASP_REVERB_USER0;

import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.SeekBar;

import com.hcn.autoeq.R;
import com.hcn.autoeq.data.CscAspEqualizerChartSettings;
import com.hcn.autoeq.data.CscAspQValueSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.autoeq.view.CscAspSeekBar;
import com.hcn.autoeq.view.CustomAspTopTabView;
import com.hcn.common.misc.LogUtils;

import java.util.Locale;


/**
 * hcn_asp乐谱
 */
public class CscAspEqualizerFragment extends BaseFragment
        implements View.OnClickListener, SeekBar.OnSeekBarChangeListener {

    private static final String TAG = CscAspEqualizerFragment.class.getSimpleName();
    private View mainView;

    private LinearLayout llSeekbarGroup;
    private CscAspSeekBar cscAspSeekBar;

    private HorizontalScrollView scrollView;

    /**
     * 两屏幕专用
     */
    private ImageView moveScrollBtn;

    /**
     * 超两屏幕专用
     */
    private ImageView moveScrollBtnNext;
    private ImageView moveScrollBtnPre;

    private CustomAspTopTabView customAspTopTabView;

    private CscAspEqualizerChartSettings cscAspEqualizerChartSettings;

    /**
     * 音频Q值设定类
     */
    private CscAspQValueSettings cscAspQValueSettings;

    //段数数量
    private int bandNumber = BAND_TOTAL;

    /**
     * 保存当前界面各 gain value
     */
    private float[][] bandValue;


    /**
     * 低中高音Q值
     */
    private int[] qValue = {1, 1, 1};

    /**
     * 弹窗
     */
    CscAspUserModeResetDialog cscAspUserModeResetDialog;
    boolean isScrolledPastHalf = false;

    @Override
    public int getLayoutRes() {
        return R.layout.csc_asp_equalizer_fragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        cscAspEqualizerChartSettings = CscAspEqualizerChartSettings.getInstance(mContext);
        cscAspQValueSettings = CscAspQValueSettings.getInstance(mContext);
        return mainView;
    }

    public static CscAspEqualizerFragment newInstance() {
        CscAspEqualizerFragment fragment = new CscAspEqualizerFragment();
        return fragment;
    }

    @Override
    public void initView() {
        customAspTopTabView = mainView.findViewById(SkinUtils.getId(R.id.second_csc_asp_top_tab));
        customAspTopTabView.setOnDspPopupListener(new CustomAspTopTabView.OnCscAspTopTabViewListener() {
            @Override
            public void updateModeContent() {
                refreshView();
            }

            @Override
            public void showUserResetDialog(int mode) {
                cscAspUserModeResetDialog = CscAspUserModeResetDialog.newInstance(mode);
                cscAspUserModeResetDialog.show(getParentFragmentManager(), "");
                cscAspUserModeResetDialog.setOnDialogListener(new CscAspUserModeResetDialog.OnDialogListener() {
                    @Override
                    public void isOk(int mode) {
                        //重置数据
                        int reverb = cscAspEqualizerChartSettings.getReverb();
                        if (reverb < EXT_CSC_ASP_REVERB_SIZE) {
                            return;
                        }
                        //拖动结束后，要遍历获取所有增益值，存储起来，作为自定义模式的值，并且更新状态栏；
                        for (int mUiBand = 0; mUiBand < bandNumber; mUiBand++) {
                            bandValue[0][mUiBand] = 0;
                        }
                        cscAspQValueSettings.setCscQValue(qValue[0], qValue[1], qValue[2]);
                        cscAspEqualizerChartSettings.saveBandValue(bandValue);
                        //更新view
                        refreshBandSeekBarStatus();
                    }
                });
            }
        });

        llSeekbarGroup = mainView.findViewById(SkinUtils.getId(R.id.ll_csc_asp_seekbar_group));

        scrollView = mainView.findViewById(SkinUtils.getId(R.id.hsv_csc_asp_band));

        moveScrollBtn = mainView.findViewById(SkinUtils.getId(R.id.btn_move_scroll));
        moveScrollBtnNext = mainView.findViewById(SkinUtils.getId(R.id.btn_move_scroll_next));
        moveScrollBtnPre = mainView.findViewById(SkinUtils.getId(R.id.btn_move_scroll_pre));
        scrollView.getViewTreeObserver().addOnScrollChangedListener(new ViewTreeObserver.OnScrollChangedListener() {
            @Override
            public void onScrollChanged() {
                // 获取ScrollView的宽度
                int scrollViewWidth = scrollView.getWidth();
                // 获取内容宽度
                int scrollContentWidth = scrollView.getChildAt(0).getWidth();
                // 获取当前可见区域的左侧部位置
                int scrollX = scrollView.getScrollX();

                // 判断是否滑动过一半
                isScrolledPastHalf = (scrollContentWidth - scrollViewWidth) / 2 < scrollX;

                if (isScrolledPastHalf) {
                    Log.d(TAG, "zzz");
                    // 已经滑动过一半
                    if (moveScrollBtn != null) {
                        Log.d(TAG, "jjj");
                        moveScrollBtn.setImageDrawable(SkinUtils.getDrawable(R.drawable.csc_asp_btn_pre_n));
                    }
                } else {
                    // 尚未滑动过一半
                    if (moveScrollBtn != null) {
                        moveScrollBtn.setImageDrawable(SkinUtils.getDrawable(R.drawable.csc_asp_btn_next_n));
                    }

                }
            }
        });

        if (moveScrollBtn != null) {
            moveScrollBtn.setOnClickListener(this);
        }
        if (moveScrollBtnNext != null) {
            moveScrollBtnNext.setOnClickListener(this);
        }
        if (moveScrollBtnPre != null) {
            moveScrollBtnPre.setOnClickListener(this);
        }
    }

    @Override
    public void initData() {
        super.initData();
        //获取总段数
        String [] values = SkinUtils.getStringArray(R.array.center_csc_asp_freq_36_segment);
        if(values != null){
            bandNumber = values.length;
        }else{
            LogUtils.vTag(TAG,"get center_csc_asp_freq_36_segment fail!");
        }

        //获取音谱模式
        int reverb = cscAspEqualizerChartSettings.getReverb();
        //初始化，获取具体音谱模式的 gain centerFre；
        bandValue = cscAspEqualizerChartSettings.getUserBandValue(reverb);
        initLlSeekbarGroup();
        refreshView();
    }

    public void initLlSeekbarGroup() {
        String[] fc = SkinUtils.getStringArray(R.array.center_csc_asp_freq_36_segment);
        if(fc == null){
            LogUtils.vTag(TAG,"get center_csc_asp_freq_36_segment fail!");
            return;
        }
        for (int i = 0; i < bandNumber; i++) {
            cscAspSeekBar = new CscAspSeekBar(mContext);
            LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
            lp.gravity = Gravity.CENTER;
            cscAspSeekBar.setPadding(SkinUtils.getInteger(R.integer.csc_asp_seekBar_parent_layout_padding_left), 0, SkinUtils.getInteger(R.integer.csc_asp_seekBar_parent_layout_padding_right), 0);
            cscAspSeekBar.setSeekBarTag(i);
            cscAspSeekBar.setOnSeekBarChangeListener(this);
            String freq = "";
            int freqValue = Integer.parseInt(fc[i]);
            if (freqValue >= 1000) {
                freq = String.format(Locale.getDefault(), "%.1f", freqValue / 1000f) + "K";
            } else {
                freq = String.format(Locale.getDefault(), "%d", freqValue);
            }
            cscAspSeekBar.setFreq(freq);
            llSeekbarGroup.addView(cscAspSeekBar, lp);
        }
    }

    private void refreshView() {
        //获取音谱模式
        int reverb = cscAspEqualizerChartSettings.getReverb();
        //初始化，获取具体音谱模式的 gain centerFre；
        bandValue = cscAspEqualizerChartSettings.getUserBandValue(reverb);
        refreshTabStatus();
        refreshBandSeekBarStatus();
    }


    @Override
    public void onClick(View v) {
        int id = v.getId();
        if (id == SkinUtils.getId(R.id.btn_move_scroll)) {
            if (!isScrolledPastHalf) {
                scrollView.fullScroll(ScrollView.FOCUS_RIGHT);
            } else {
                scrollView.fullScroll(ScrollView.FOCUS_LEFT);
            }
        } else if (id == SkinUtils.getId(R.id.btn_move_scroll_next)) {
            int scrollContentWidth = scrollView.getChildAt(0).getWidth();
            Log.d(TAG, "scrollContentWidth:" + scrollContentWidth + ",scrollView:" + scrollView.getScrollX());
            scrollView.smoothScrollBy(scrollContentWidth / 4, 0);
        } else if (id == SkinUtils.getId(R.id.btn_move_scroll_pre)) {
            int scrollContentWidth = scrollView.getChildAt(0).getWidth();
            Log.d(TAG, "scrollContentWidth:" + scrollContentWidth + ",scrollView:" + scrollView.getScrollX());
            scrollView.smoothScrollBy(-scrollContentWidth / 4, 0);
        }
    }

    /**
     * 刷新音效模式状态
     */
    private void refreshBandSeekBarStatus() {
        if (cscAspEqualizerChartSettings == null) {
            return;
        }
        float[] _gainValue = bandValue[0];
        for (int i = 0; i < bandNumber; i++) {
            View child = llSeekbarGroup.getChildAt(i);
            if (child instanceof CscAspSeekBar) {
                CscAspSeekBar seekBar = (CscAspSeekBar) child;
                seekBar.setProgress(_gainValue[i]);
            }
        }
    }

    /**
     * 刷新状态栏状态
     * 响度，场景模式，自定义模式相关
     */
    private void refreshTabStatus() {
        if (customAspTopTabView == null) {
            return;
        }
        customAspTopTabView.initViewStatus();
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            // 进度条拖动的时候，只修改底层音效和临时变量，拖动完毕后，再把临时变量值保存到 sp 文件中
            int mTag = (int) seekBar.getTag();
            bandValue[0][mTag] = progress;
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        int reverb = cscAspEqualizerChartSettings.getReverb();
        boolean enable = reverb <= EXT_CSC_ASP_REVERB_SIZE;
        if (enable) {
            cscAspEqualizerChartSettings.saveReverb(EXT_CSC_ASP_REVERB_USER0);
            cscAspEqualizerChartSettings.saveBandValue(bandValue);
            refreshView();
        }
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        cscAspEqualizerChartSettings.saveBandValue(bandValue);
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if (hidden && customAspTopTabView != null) {
            customAspTopTabView.highGallery();
        }
        refreshView();
    }

}
