package com.hcn.autoeq.fragment.fydsp;

import android.graphics.drawable.AnimationDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.SeekBar;

import com.hcn.autoeq.R;
import com.hcn.autoeq.adapter.FyDspLoudnessAdapter;
import com.hcn.autoeq.bean.EventMessage;
import com.hcn.autoeq.bean.FyDspLoudness;
import com.hcn.autoeq.bean.FyDspOutputMode;
import com.hcn.autoeq.data.FyDspHLPFSettings;
import com.hcn.autoeq.data.FyDspSurroundSettings;
import com.hcn.autoeq.fragment.BaseFragment;
import com.hcn.autoeq.util.ConstantFyDsp;
import com.hcn.autoeq.view.CustomSpinner;
import com.hcn.autoeq.view.FyDspSeekBar;

import org.greenrobot.eventbus.Subscribe;

import java.util.Arrays;
import java.util.List;
import java.util.Locale;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

public class FyDspSurroundFragment extends BaseFragment implements ConstantFyDsp, View.OnClickListener, AdapterView.OnItemSelectedListener {

    private static final String TAG = FyDspSurroundFragment.class.getSimpleName();

    private static final int SEEK_BAR_MIN_GAIN = 0; // 增益调节最小为 0.0
    private static final int SEEK_BAR_MAX_GAIN = 12; // 增益调节最大为 12.0

    //显示45-300，实际上传（90-600）
    private static final int SEEK_BAR_MIN_FREQ = 45; // 频率调节最小为 45
    private static final int SEEK_BAR_MAX_FREQ = 300; // 频率调节最大为 300

    private View mainView;
    private CustomSpinner spLoudness;
    private ImageView ivSpeakerAnim;
    private FyDspSeekBar fdsbGainCh12, fdsbFreqCh12, fdsbGainCh34, fdsbFreqCh34;
    private Button btnResetSurround;

    private FyDspSurroundSettings fyDspSurroundSettings;
    private FyDspHLPFSettings fyDspHLPFSettings;
    private FyDspOutputMode fyDspOutputMode;
    // 记录是不是手动选择列表项
    private boolean spLoudnessFromUser;

    public FyDspSurroundFragment() {
    }

    public static FyDspSurroundFragment newInstance() {
        FyDspSurroundFragment fragment = new FyDspSurroundFragment();
        return fragment;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fydsp_fragment_surround;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mainView = super.onCreateView(inflater, container, savedInstanceState);
        fyDspSurroundSettings = FyDspSurroundSettings.getInstance(mContext);
        fyDspHLPFSettings = FyDspHLPFSettings.getInstance(mContext);
        return mainView;
    }

    @Override
    public void initView() {
        spLoudness = mainView.findViewById(R.id.sp_loudness);
        ivSpeakerAnim = mainView.findViewById(R.id.iv_speaker_anim);

        fdsbGainCh12 = mainView.findViewById(R.id.fdsb_gain_ch12);
        fdsbFreqCh12 = mainView.findViewById(R.id.fdsb_freq_ch12);
        fdsbGainCh34 = mainView.findViewById(R.id.fdsb_gain_ch34);
        fdsbFreqCh34 = mainView.findViewById(R.id.fdsb_freq_ch34);

        spLoudness.setOnItemSelectedListener(this);
        spLoudness.setOnTouchListener((v, event) -> {
            spLoudnessFromUser = true;
            return false;
        });

        btnResetSurround = mainView.findViewById(R.id.btn_reset_surround);
        btnResetSurround.setOnClickListener(this);
    }

    @Override
    public void initData() {
        super.initData();
        fyDspOutputMode = fyDspHLPFSettings.getOutputMode();
        initGainCh12();
        initFreqCh12();
        initGainCh34();
        initFreqCh34();
        refreshGainStatus();
        refreshFreqStatus();
        refreshSpeaker();
        refreshLoudnessStatus();
    }

    private void initGainCh12() {
        fdsbGainCh12.setTitle(getString(R.string.fydsp_bassboost_gain));
        fdsbGainCh12.getSeekBar().setMax(SEEK_BAR_MAX_GAIN);
        fdsbGainCh12.setMinText(String.format(Locale.getDefault(), "%d", SEEK_BAR_MIN_GAIN));
        fdsbGainCh12.setMaxText(String.format(Locale.getDefault(), "%ddB", SEEK_BAR_MAX_GAIN));
        fdsbGainCh12.getSeekBar().setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                fdsbGainCh12.setValue(String.format(Locale.getDefault(), "%d", progress));
                int freq = fdsbFreqCh12.getSeekBar().getProgress();
                // 不能小于最低的范围值
                if (freq < SEEK_BAR_MIN_FREQ) {
                    freq = SEEK_BAR_MIN_FREQ;
                }
                fyDspSurroundSettings.nativeBassBoost(freq, seekBar.getProgress(), fyDspSurroundSettings.getBassBoostFreqCh34(), fyDspSurroundSettings.getBassBoostGainCh34());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                EventMessage.anyChanged(mContext, TAG + "_" + "onStopTrackingTouch");
                fyDspSurroundSettings.saveBassBoostGainCh12(seekBar.getProgress());
            }
        });
    }

    private void initFreqCh12() {
        fdsbFreqCh12.setTitle(getString(R.string.fydsp_bassboost_freq));
        fdsbFreqCh12.getSeekBar().setMin(SEEK_BAR_MIN_FREQ);
        fdsbFreqCh12.getSeekBar().setMax(SEEK_BAR_MAX_FREQ);
        fdsbFreqCh12.setMinText(String.format(Locale.getDefault(), "%d", SEEK_BAR_MIN_FREQ));
        fdsbFreqCh12.setMaxText(String.format(Locale.getDefault(), "%dHz", SEEK_BAR_MAX_FREQ));
        fdsbFreqCh12.getSeekBar().setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                // 不能小于最低的范围值
                if (progress < SEEK_BAR_MIN_FREQ) {
                    progress = SEEK_BAR_MIN_FREQ;
                }
                fdsbFreqCh12.setValue(String.valueOf(progress));
                fyDspSurroundSettings.nativeBassBoost(progress, fdsbGainCh12.getSeekBar().getProgress(), fyDspSurroundSettings.getBassBoostFreqCh34(), fyDspSurroundSettings.getBassBoostGainCh34());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                EventMessage.anyChanged(mContext, TAG + "_" + "onStopTrackingTouch");
                int freq = seekBar.getProgress();
                // 不能小于最低的范围值
                if (freq < SEEK_BAR_MIN_FREQ) {
                    freq = SEEK_BAR_MIN_FREQ;
                }
                fyDspSurroundSettings.saveBassBoostFreqCh12(freq);
            }
        });
    }

    private void initGainCh34() {
        fdsbGainCh34.setTitle(getString(R.string.fydsp_bassboost_gain));
        fdsbGainCh34.getSeekBar().setMax(SEEK_BAR_MAX_GAIN);
        fdsbGainCh34.setMinText(String.format(Locale.getDefault(), "%d", SEEK_BAR_MIN_GAIN));
        fdsbGainCh34.setMaxText(String.format(Locale.getDefault(), "%ddB", SEEK_BAR_MAX_GAIN));
        fdsbGainCh34.getSeekBar().setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                fdsbGainCh34.setValue(String.format(Locale.getDefault(), "%d", progress));
                int freq = fdsbFreqCh34.getSeekBar().getProgress();
                // 不能小于最低的范围值
                if (freq < SEEK_BAR_MIN_FREQ) {
                    freq = SEEK_BAR_MIN_FREQ;
                }
                fyDspSurroundSettings.nativeBassBoost(fyDspSurroundSettings.getBassBoostFreqCh12(), fyDspSurroundSettings.getBassBoostGainCh12(), freq, seekBar.getProgress());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                EventMessage.anyChanged(mContext, TAG + "_" + "onStopTrackingTouch");
                fyDspSurroundSettings.saveBassBoostGainCh34(seekBar.getProgress());
            }
        });
    }

    private void initFreqCh34() {
        fdsbFreqCh34.setTitle(getString(R.string.fydsp_bassboost_freq));
        fdsbFreqCh34.getSeekBar().setMin(SEEK_BAR_MIN_FREQ);
        fdsbFreqCh34.getSeekBar().setMax(SEEK_BAR_MAX_FREQ);
        fdsbFreqCh34.setMinText(String.format(Locale.getDefault(), "%d", SEEK_BAR_MIN_FREQ));
        fdsbFreqCh34.setMaxText(String.format(Locale.getDefault(), "%dHz", SEEK_BAR_MAX_FREQ));
        fdsbFreqCh34.getSeekBar().setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                // 不能小于最低的范围值
                if (progress < SEEK_BAR_MIN_FREQ) {
                    progress = SEEK_BAR_MIN_FREQ;
                }
                fdsbFreqCh34.setValue(String.valueOf(progress));
                fyDspSurroundSettings.nativeBassBoost(fyDspSurroundSettings.getBassBoostFreqCh12(), fyDspSurroundSettings.getBassBoostGainCh12(), progress, fdsbGainCh34.getSeekBar().getProgress());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                EventMessage.anyChanged(mContext, TAG + "_" + "onStopTrackingTouch");
                int freq = seekBar.getProgress();
                // 不能小于最低的范围值
                if (freq < SEEK_BAR_MIN_FREQ) {
                    freq = SEEK_BAR_MIN_FREQ;
                }
                fyDspSurroundSettings.saveBassBoostFreqCh34(freq);
            }
        });
    }

    private void refreshGainStatus() {
        fdsbGainCh12.setProgress(fyDspSurroundSettings.getBassBoostGainCh12());
        fdsbGainCh12.setValue(String.valueOf(fyDspSurroundSettings.getBassBoostGainCh12()));
        fdsbGainCh34.setProgress(fyDspSurroundSettings.getBassBoostGainCh34());
        fdsbGainCh34.setValue(String.valueOf(fyDspSurroundSettings.getBassBoostGainCh34()));
    }

    private void refreshFreqStatus() {
        fdsbFreqCh12.setProgress(fyDspSurroundSettings.getBassBoostFreqCh12());
        fdsbFreqCh12.setValue(String.valueOf(fyDspSurroundSettings.getBassBoostFreqCh12()));
        fdsbFreqCh34.setProgress(fyDspSurroundSettings.getBassBoostFreqCh34());
        fdsbFreqCh34.setValue(String.valueOf(fyDspSurroundSettings.getBassBoostFreqCh34()));
    }

    private void refreshLoudnessStatus() {
        final List<FyDspLoudness> fyDspLoudnessList = Arrays.stream(FyDspLoudness.values()).collect(Collectors.toList());
        spLoudness.setAdapter(new FyDspLoudnessAdapter(mContext, fyDspLoudnessList, spLoudness));

        int loudness = fyDspSurroundSettings.getLoudness();
        int loudnessIndex = IntStream.range(0, fyDspLoudnessList.size()).filter(i -> loudness == fyDspLoudnessList.get(i).getLoudness()).findFirst().orElse(0);
        spLoudness.setSelection(loudnessIndex);
    }

    private void refreshSpeaker() {
        switch (fyDspOutputMode) {
            case WAY2:
                ivSpeakerAnim.setBackgroundResource(R.drawable.fydsp_surround_speaker_anim_way2);
                break;
            case WAY3:
                ivSpeakerAnim.setBackgroundResource(R.drawable.fydsp_surround_speaker_anim_way3);
                break;
            case CHANNEL51:
                ivSpeakerAnim.setBackgroundResource(R.drawable.fydsp_surround_speaker_anim_channel51);
                break;
        }
    }

    private void refreshSpeakerAnim(boolean start) {
        AnimationDrawable animationDrawable = (AnimationDrawable) ivSpeakerAnim.getBackground();
        if (start) {
            animationDrawable.start();
        } else {
            animationDrawable.stop();
            animationDrawable.selectDrawable(0); // 关闭后，回到第一帧的关闭状态图
        }
    }

    @Override
    @Subscribe(sticky = true)
    public void onEvent(EventMessage eventMessage) {
        super.onEvent(eventMessage);
        if (EventMessage.MSG_STICKY_OUTPUT_MODE_CHANGED.equals(eventMessage.getMessage())) {
            fyDspOutputMode = (FyDspOutputMode) eventMessage.getData();
            Log.d(TAG, "MSG_STICKY_OUTPUT_MODE_CHANGED change to " + fyDspOutputMode);
            refreshSpeaker();
            refreshSpeakerAnim(fyDspSurroundSettings.getLoudness() > 0);
        } else if (EventMessage.MSG_STICKY_USER_MODE_CHANGED.equals(eventMessage.getMessage())) {
            Log.d(TAG, "MSG_STICKY_USER_MODE_CHANGED");
            initData();
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_reset_surround:
                EventMessage.anyChanged(mContext, TAG + "_" + "btn_reset_surround");

                fyDspSurroundSettings.reset();

                refreshGainStatus();
                refreshFreqStatus();
                refreshSpeaker();
                refreshLoudnessStatus();

                fyDspSurroundSettings.nativeBassBoost(fyDspSurroundSettings.getBassBoostFreqCh12(), fyDspSurroundSettings.getBassBoostGainCh12(),
                        fyDspSurroundSettings.getBassBoostFreqCh34(), fyDspSurroundSettings.getBassBoostGainCh34());
                fyDspSurroundSettings.nativeLoudness(fyDspSurroundSettings.getLoudness());
                break;
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
        FyDspLoudness fyDspLoudness = (FyDspLoudness) spLoudness.getSelectedItem();
        refreshSpeakerAnim(fyDspLoudness.getLoudness() > 0);

        if (spLoudnessFromUser) {
            spLoudnessFromUser = false;

            EventMessage.anyChanged(mContext, TAG + "_" + "onItemSelected");

            fyDspSurroundSettings.nativeLoudness(fyDspLoudness.getLoudness());
            fyDspSurroundSettings.saveLoudness(fyDspLoudness.getLoudness());
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> adapterView) {

    }
}
