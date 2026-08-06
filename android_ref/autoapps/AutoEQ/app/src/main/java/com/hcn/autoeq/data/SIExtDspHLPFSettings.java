package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.R;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.SIConstantExtDsp;
import com.hcn.autoeq.util.SkinUtils;

import java.util.Arrays;

public class SIExtDspHLPFSettings implements SIConstantExtDsp {

    private static final String TAG = SIExtDspHLPFSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(SIExtDspHLPFSettings.class.getSimpleName(), Log.DEBUG);

    private static final String SI_EXT_DSP_DBB_FILE = "si_ext_dsp_hlpf"; // 高低通滤波保存的文件名
    private static final String SI_KEY_CHANNEL = "si_ext_dsp_hlpf_channel";
    private static final String SI_KEY_FREQ_PROGRESS = "si_ext_dsp_hlpf_freq_progress_";
    private static final String SI_KEY_FREQ = "si_ext_dsp_hlpf_freq_";
    private static final String SI_KEY_QVALUE = "si_ext_dsp_hlpf_qvalue_";

    private Context context;
    private static SIExtDspHLPFSettings extDspDbbSettings = null;
    private SPUtils spUtils;

    public static SIExtDspHLPFSettings getInstance(Context mContext) {
        if (null == extDspDbbSettings) {
            extDspDbbSettings = new SIExtDspHLPFSettings(mContext);
        }
        return extDspDbbSettings;
    }

    private SIExtDspHLPFSettings(Context context) {
        this.context = context;
        spUtils = SPUtils.getInstance(SI_EXT_DSP_DBB_FILE);
    }

    public void nativeHLPF(int channel, int lpfSlope, int lpfFreq, int hpfSlope, int hpfFreq) {
        int realChannel = channel;
        if (channel == SI_CHANNEL_FRONT_HIGH || channel == SI_CHANNEL_FRONT_LOW) {
            realChannel = INDEX_LPF_HPF_F;
        } else if (channel == SI_CHANNEL_REAR_HIGH || channel == SI_CHANNEL_REAR_LOW) {
            realChannel = INDEX_LPF_HPF_R;
        } else if (channel == SI_CHANNEL_SUBWOOFER_HIGH || channel == SI_CHANNEL_SUBWOOFER_LOW) {
            realChannel = INDEX_LPF_HPF_SUB;
        } else if (channel == SI_CHANNEL_CENTER_HIGH || channel == SI_CHANNEL_CENTER_LOW) {
            realChannel = INDEX_LPF_HPF_CEN;
        }

        //需要高低滤音
        int[] data = new int[]{realChannel, lpfSlope, lpfFreq, hpfSlope, hpfFreq};
        NativeHelper.getEq().setEqHpfLpf(data);
        Log.d(TAG, String.format("nativeHLPF data : %s", Arrays.toString(data)));
    }

    public static int mapValue(int progressValue, int actualMin, int actualMax, int displayMin, int displayMax) {
        return (progressValue - displayMin) * (actualMax - actualMin) / (displayMax - displayMin) + actualMin;
    }


    public void saveHLPF(int channel, int freqProgress, int freq, int qValue) {
        Log.d(TAG, String.format("saveHLPF channel : %d, freqProgress : %d, freq : %d, qValue : %d"
                , channel, freqProgress, freq, qValue));
        spUtils.put(SI_KEY_CHANNEL, channel);
        spUtils.put(SI_KEY_FREQ_PROGRESS + channel, freqProgress);
        spUtils.put(SI_KEY_FREQ + channel, freq);
        spUtils.put(SI_KEY_QVALUE + channel, qValue, true);
    }

    public int getHLPFChannel() {
        return spUtils.getInt(SI_KEY_CHANNEL, CHANNEL_FRONT_HIGH);
    }

    public int getFreqProgress(int channel) {
        int defaultFreq;
        if (channel == SI_CHANNEL_FRONT_HIGH) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_F_MIN_DEFAULT;
        } else if (channel == SI_CHANNEL_REAR_HIGH) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_R_MIN_DEFAULT;
        } else if (channel == SI_CHANNEL_FRONT_LOW) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_F_MAX_DEFAULT;
        } else if (channel == SI_CHANNEL_REAR_LOW) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_R_MAX_DEFAULT;
        } else if (channel == SI_CHANNEL_SUBWOOFER_HIGH) { // 重低音的高通默认值
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_SUB_MIN_DEFAULT;
        } else if (channel == SI_CHANNEL_SUBWOOFER_LOW) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_SUB_MAX_DEFAULT;
        } else if (channel == SI_CHANNEL_CENTER_HIGH) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_CEN_MIN_DEFAULT;
        } else if (channel == SI_CHANNEL_CENTER_LOW) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_CEN_MAX_DEFAULT;
        } else { // 重低音的低通默认值
            defaultFreq = HLPF_SUBWOOFER_FREQ_MAX_DEFAULT;
        }
        return spUtils.getInt(SI_KEY_FREQ_PROGRESS + channel, defaultFreq);
    }

    public int getFreq(int channel) {
        int defaultFreq;
        if (channel == SI_CHANNEL_FRONT_HIGH) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_F_MIN_DEFAULT;
        } else if (channel == SI_CHANNEL_REAR_HIGH) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_R_MIN_DEFAULT;
        } else if (channel == SI_CHANNEL_FRONT_LOW) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_F_MAX_DEFAULT;
        } else if (channel == SI_CHANNEL_REAR_LOW) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_R_MAX_DEFAULT;
        } else if (channel == SI_CHANNEL_SUBWOOFER_HIGH) { // 重低音的高通默认值
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_SUB_MIN_DEFAULT;
        } else if (channel == SI_CHANNEL_SUBWOOFER_LOW) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_SUB_MAX_DEFAULT;
        } else if (channel == SI_CHANNEL_CENTER_HIGH) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_CEN_MIN_DEFAULT;
        } else if (channel == SI_CHANNEL_CENTER_LOW) {
            defaultFreq = SI_HLPF_INDEX_LPF_HPF_CEN_MAX_DEFAULT;
        } else { // 重低音的低通默认值
            defaultFreq = HLPF_SUBWOOFER_FREQ_MAX_DEFAULT;
        }
        return spUtils.getInt(SI_KEY_FREQ + channel, defaultFreq);
    }

    public int getQValue(int channel) {
        return spUtils.getInt(SI_KEY_QVALUE + channel, 2);
    }

    public void resetByChannel(int channelHigh, int channelLow) {
        spUtils.remove(SI_KEY_FREQ_PROGRESS + channelHigh);
        spUtils.remove(SI_KEY_FREQ + channelHigh);
        spUtils.remove(SI_KEY_QVALUE + channelHigh, true);

        spUtils.remove(SI_KEY_FREQ_PROGRESS + channelLow);
        spUtils.remove(SI_KEY_FREQ + channelLow);
        spUtils.remove(SI_KEY_QVALUE + channelLow, true);
    }

    public void resetCurrentChannel(int channelHigh, int channelLow) {
        nativeHLPF(channelHigh, getQValue(channelLow), getFreq(channelLow), getQValue(channelHigh), getFreq(channelHigh));
    }

    public void nativeAll(int... channels) {
        for (int channel : channels) {
            int channelHigh, channelLOW;
            int freqHigh, freqLow;
            int qValueHigh, qValueLow;
            if (channel == INDEX_LPF_HPF_F) {
                channelHigh = SI_CHANNEL_FRONT_HIGH;
                channelLOW = SI_CHANNEL_FRONT_LOW;
            } else if (channel == INDEX_LPF_HPF_R) {
                channelHigh = SI_CHANNEL_REAR_HIGH;
                channelLOW = SI_CHANNEL_REAR_LOW;
            } else if (channel == INDEX_LPF_HPF_CEN) {
                channelHigh = SI_CHANNEL_CENTER_HIGH;
                channelLOW = SI_CHANNEL_CENTER_LOW;
            } else if (channel == INDEX_LPF_HPF_SUB) {
                channelHigh = SI_CHANNEL_SUBWOOFER_HIGH;
                channelLOW = SI_CHANNEL_SUBWOOFER_LOW;
            } else {
                channelHigh = SI_CHANNEL_FRONT_HIGH;
                channelLOW = SI_CHANNEL_FRONT_LOW;
            }
            freqHigh = getFreq(channelHigh);
            freqLow = getFreq(channelLOW);
            qValueHigh = getQValue(channelHigh);
            qValueLow = getQValue(channelLOW);
            nativeHLPF(channelHigh, qValueLow, freqLow, qValueHigh, freqHigh);
        }
    }

}
