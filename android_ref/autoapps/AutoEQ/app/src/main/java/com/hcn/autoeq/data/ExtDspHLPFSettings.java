package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantExtDsp;
import com.hcn.autoeq.util.ECDConstantExtDsp;
import com.hcn.autoeq.util.EqUtils;

import java.util.Arrays;

public class ExtDspHLPFSettings implements ConstantExtDsp {

    private static final String TAG = ExtDspHLPFSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(ExtDspHLPFSettings.class.getSimpleName(), Log.DEBUG);

    private static final String EXT_DSP_DBB_FILE = "ext_dsp_hlpf"; // 高低通滤波保存的文件名
    private static final String KEY_CHANNEL = "ext_dsp_hlpf_channel";
    private static final String KEY_FREQ_PROGRESS = "ext_dsp_hlpf_freq_progress_";
    private static final String KEY_FREQ = "ext_dsp_hlpf_freq_";
    private static final String KEY_QVALUE = "ext_dsp_hlpf_qvalue_";

    private Context context;
    private static ExtDspHLPFSettings extDspDbbSettings = null;
    private SPUtils spUtils;

    public static ExtDspHLPFSettings getInstance(Context mContext) {
        if (null == extDspDbbSettings) {
            extDspDbbSettings = new ExtDspHLPFSettings(mContext);
        }
        return extDspDbbSettings;
    }

    private ExtDspHLPFSettings(Context context) {
        this.context = context;
        spUtils = SPUtils.getInstance(EXT_DSP_DBB_FILE);
    }

    public void nativeHLPF(int channel, int freq, int qValue) {
        int[] data = new int[]{channel, freq, qValue, 0};
        NativeHelper.getEq().setEqHpfLpf(data);
        Log.d(TAG, String.format("nativeHLPF data : %s", Arrays.toString(data)));
    }

    public void saveHLPF(int channel, int freqProgress, int freq, int qValue) {
        Log.d(TAG, String.format("saveHLPF channel : %d, freqProgress : %d, freq : %d, qValue : %d"
                , channel, freqProgress, freq, qValue));
        spUtils.put(KEY_CHANNEL, channel);
        spUtils.put(KEY_FREQ_PROGRESS + channel, freqProgress);
        spUtils.put(KEY_FREQ + channel, freq);
        spUtils.put(KEY_QVALUE + channel, qValue, true);
    }

    public int getHLPFChannel() {
        return spUtils.getInt(KEY_CHANNEL, CHANNEL_FRONT_HIGH);
    }

    public int getFreqProgress(int channel) {
        int defaultFreq;
        if (channel == CHANNEL_FRONT_HIGH || channel == CHANNEL_REAR_HIGH) { // 前，后的高通默认值
            defaultFreq = HLPF_FRONT_REAR_FREQ_MIN_DEFAULT;
        } else if (channel == CHANNEL_FRONT_LOW || channel == CHANNEL_REAR_LOW) { // 前，后的低通默认值
            defaultFreq = HLPF_FRONT_REAR_FREQ_MAX_DEFAULT;
        } else if (channel == CHANNEL_SUBWOOFER_HIGH) { // 重低音的高通默认值
            defaultFreq = HLPF_SUBWOOFER_FREQ_MIN_DEFAULT;
        } else if (channel == ECDConstantExtDsp.CHANNEL_CENTER_HIGH) { // 重低音的高通默认值
            if (EqUtils.isYuFeng()) {
                defaultFreq = HLPF_SUBWOOFER_FREQ_MIN_DEFAULT;
            }else {
                defaultFreq = HLPF_SUBWOOFER_FREQ_MAX_DEFAULT;
            }
        } else { // 重低音的低通默认值
            defaultFreq = HLPF_SUBWOOFER_FREQ_MAX_DEFAULT;
        }
        return spUtils.getInt(KEY_FREQ_PROGRESS + channel, defaultFreq);
    }

    public int getFreq(int channel) {
        int defaultFreq;
        if (channel == CHANNEL_FRONT_HIGH || channel == CHANNEL_REAR_HIGH) { // 前，后的高通默认值
            defaultFreq = HLPF_FRONT_REAR_FREQ_MIN_DEFAULT;
        } else if (channel == CHANNEL_FRONT_LOW || channel == CHANNEL_REAR_LOW) { // 前，后的低通默认值
            defaultFreq = HLPF_FRONT_REAR_FREQ_MAX_DEFAULT;
        } else if (channel == CHANNEL_SUBWOOFER_HIGH) { // 重低音的高通默认值
            defaultFreq = HLPF_SUBWOOFER_FREQ_MIN_DEFAULT;
        } else if (channel == CHANNEL_SUBWOOFER_LOW) { // 重低音的低通默认值
            defaultFreq = HLPF_SUBWOOFER_FREQ_MAX_DEFAULT;
        } else if (channel == ECDConstantExtDsp.CHANNEL_CENTER_HIGH) {
            if (EqUtils.isYuFeng()) {// 重低音2使用中置通道，高通默认值
                defaultFreq = HLPF_SUBWOOFER_FREQ_MIN_DEFAULT;
            }else {
                defaultFreq = ECDConstantExtDsp.HLPF_CENTER_FREQ_MIN_DEFAULT;
            }
        } else {
            if (EqUtils.isYuFeng()) {// 重低音2使用中置通道，低通默认值
                defaultFreq = HLPF_SUBWOOFER_FREQ_MAX_DEFAULT;
            }else {
                defaultFreq = ECDConstantExtDsp.HLPF_CENTER_FREQ_MAX_DEFAULT;
            }
        }
        return spUtils.getInt(KEY_FREQ + channel, defaultFreq);
    }

    public int getQValue(int channel) {
        return spUtils.getInt(KEY_QVALUE + channel, HLPF_QVALUE_DEFAULT);
    }

    public void resetByChannel(int channel) {
        spUtils.remove(KEY_FREQ_PROGRESS + channel);
        spUtils.remove(KEY_FREQ + channel);
        spUtils.remove(KEY_QVALUE + channel, true);
        if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType())) {
            nativeHLPF(channel, getFreq(channel), getQValueSelect(getQValue(channel)));
        } else {
            nativeHLPF(channel, getFreq(channel), getQValue(channel));
        }
    }

    public int getQValueSelect(int value) {
        switch (value) {
            case 0:
                return 0;
            case 700:
                return 1;
            case 1000:
                return 2;
            case 1500:
                return 3;
            case 2000:
                return 4;
            case 2500:
                return 5;
            case 3000:
                return 6;
            default:
                return 1;
        }
    }

    public void nativeAll(int... channels) {
        for (int channel : channels) {
            int freq = getFreq(channel);
            int qValue = getQValue(channel);
            nativeHLPF(channel, freq, qValue);
        }
    }

    public void nativeAll7604C(int... channels) {
        for (int channel : channels) {
            int freq;
            int qValue;
            int slope;
            freq = getFreq(channel);
            qValue = getQValue(channel);
            slope = getQValueSelect(qValue);
            Log.d(TAG, "sendMessageWhenStart: chanel = " + channel + " freq = " + freq + " qValue = " + qValue + " slope = " + slope);
           nativeHLPF(channel, freq, slope);
        }
    }


}
