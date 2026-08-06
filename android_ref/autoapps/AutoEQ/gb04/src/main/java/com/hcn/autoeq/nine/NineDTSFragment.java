package com.hcn.autoeq.nine;

import static com.hcn_library.data.NineDspDtsDtsSettings.theater_mode;
import static com.hcn_library.data.NineDspDtsDtsSettings.music_mode;
import static com.hcn_library.data.NineDspDtsDtsSettings.professional_mode;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.hcn.autoeq.R;
import com.hcn_library.BaseFragment;
import com.hcn_library.data.NineDspDtsDtsSettings;
import com.hcn_library.data.NineDspDtsFilterSettings;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;
import com.hcn_library.util.SkinUtils;

public class NineDTSFragment extends BaseFragment implements NineConstantExtDsp, View.OnClickListener, NineDtsFilterFragment.EnableViewInterface {
    private static final String TAG = "NineDTSFragment";
    private int checkedModel = 0;
    private NineDspDtsFilterSettings filterSettings;
    private LinearLayout llMusic;
    private LinearLayout llProfessional;
    private LinearLayout llTheater;
    private View mainView;
    private TextView tvTheater, tvMusic, tvProfessional;
    private NineDspDtsDtsSettings nineDspDtsDtsSettings;

    @Override 
    public int getLayoutRes() {
        return R.layout.nine_dts_fragment_model;
    }

    public static NineDTSFragment newInstance() {
        return new NineDTSFragment();
    }

    @Override 
    public View onCreateView(LayoutInflater layoutInflater, ViewGroup viewGroup, Bundle bundle) {
        mainView = super.onCreateView(layoutInflater, viewGroup, bundle);
        nineDspDtsDtsSettings = NineDspDtsDtsSettings.getInstance(mContext);
        filterSettings = NineDspDtsFilterSettings.getInstance(mContext);
        return mainView;
    }

    @Override 
    public void initView() {
        tvTheater = mainView.findViewById(SkinUtils.getId(R.id.tv_theater));
        tvMusic = mainView.findViewById(SkinUtils.getId(R.id.tv_music));
        tvProfessional = mainView.findViewById(SkinUtils.getId(R.id.tv_professional));
        llTheater = (LinearLayout) mainView.findViewById(SkinUtils.getId(R.id.ll_theater));
        llMusic = (LinearLayout) mainView.findViewById(SkinUtils.getId(R.id.ll_music));
        llProfessional = (LinearLayout) mainView.findViewById(SkinUtils.getId(R.id.ll_professional));
        llTheater.setOnClickListener(this);
        llMusic.setOnClickListener(this);
        llProfessional.setOnClickListener(this);
    }

    @Override 
    public void initData() {
        super.initData();
        setViewsEnableStatus();
        int dtsModel = nineDspDtsDtsSettings.getDtsModel();
        updateCheckedModel(dataToModelIndex(dtsModel));
    }

    @Override 
    public void onClick(View view) {
        if (view.getId() == SkinUtils.getId(R.id.ll_theater)) {
            updateCheckedModel(0);
            nineDspDtsDtsSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_PROCESS_MODEL, theater_mode);
            nineDspDtsDtsSettings.saveDtsModel(theater_mode);
        } else if (view.getId() == SkinUtils.getId(R.id.ll_music)) {
            nineDspDtsDtsSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_PROCESS_MODEL, music_mode);
            nineDspDtsDtsSettings.saveDtsModel(music_mode);
            updateCheckedModel(1);
        } else if (view.getId() == SkinUtils.getId(R.id.ll_professional)) {
            nineDspDtsDtsSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_PROCESS_MODEL, professional_mode);
            nineDspDtsDtsSettings.saveDtsModel(professional_mode);
            updateCheckedModel(2);
        }
    }

    private void updateCheckedModel(int i) {
        tvTheater.setAlpha(0.6f);
        tvMusic.setAlpha(0.6f);
        tvProfessional.setAlpha(0.6f);
        llTheater.setSelected(false);
        llMusic.setSelected(false);
        llProfessional.setSelected(false);
        if (i == 0) {
            llTheater.setSelected(true);
            tvTheater.setAlpha(1f);
        } else if (i == 1) {
            llMusic.setSelected(true);
            tvMusic.setAlpha(1f);
        } else if (i == 2) {
            llProfessional.setSelected(true);
            tvProfessional.setAlpha(1f);
        }
        Log.d(TAG, "updateCheckedModel： " + i);
    }

    private int dataToModelIndex(int value) {
        int index = 1; // 默认音乐模式
        if (value == theater_mode) {
            index = 0;
        } else if (value == music_mode) {
            index = 1;
        } else if (value == professional_mode) {
            index = 2;
        }
        Log.d(TAG, "dataToModelIndex-----value: " + value + " index: " + index);
        return index;
    }

    @Override 
    public void updateView(boolean enable) {
        if(filterSettings == null) return;
        setViewsEnableStatus();
    }

    private void setViewsEnableStatus() {
        boolean enable = filterSettings.getDtsSwitch() == 0;
        if (EqUtils.isChip7739()) {
            enable = filterSettings.getDtsSwitchEnable();
            nineDspDtsDtsSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_PROCESS_MODEL, enable ? nineDspDtsDtsSettings.getDtsModel() : 0);
        }
        mainView.setAlpha(enable ? 1.0f : 0.4f);
        llTheater.setEnabled(enable);
        llMusic.setEnabled(enable);
        llProfessional.setEnabled(enable);
        Log.d(TAG, "setViewsEnableStatus enable： " + enable);
    }

    @Override 
    public void onHiddenChanged(boolean z) {
        super.onHiddenChanged(z);
        if (!z) {
            NineDspDtsFilterSettings.getInstance(mContext);
            setViewsEnableStatus();
        }
        Log.d(TAG, "onHiddenChanged  hidden: " + z);
    }

    @Override 
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
    }
}