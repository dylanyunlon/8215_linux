package com.hcn_library.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn_library.nativeextdsp.NativeHelper;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;

import java.util.Arrays;

public class NineDspHLPFSettings implements NineConstantExtDsp {
    private static final String DSP_DBB_FILE = "nine_dsp_hlpf";
    private static final String KEY_CHANNEL = "nine_dsp_hlpf_channel";
    private static final String KEY_FREQ = "nine_dsp_hlpf_freq_";
    private static final String KEY_QVALUE = "nine_dsp_hlpf_qvalue_";
    private static final String KEY_QVALUE_SWITCH = "nine_dsp_hlpf_qvalue_switch";
    private Context context;
    private SPUtils spUtils = SPUtils.getInstance(DSP_DBB_FILE);
    private static final String TAG = "NineDspHLPFSettings";
    private static NineDspHLPFSettings nineDspDbbSettings = null;

    public static NineDspHLPFSettings getInstance(Context context) {
        if (nineDspDbbSettings == null) {
            nineDspDbbSettings = new NineDspHLPFSettings(context);
        }
        return nineDspDbbSettings;
    }

    private NineDspHLPFSettings(Context context) {
        this.context = context;
    }

    public void nativeHLPF(int channel, int lpfSlope, int lpfFreq, int hpfSlope, int hpfFreq) {
        if ("gb05".equals(EqUtils.getSkinName())) {
            // 这里对7604c和si47925dts芯片定义的高低通channel编号进行区分
            if (channel == NINE_CHANNEL_CENTER_HIGH) {
                channel = 7;
            } else if (channel == NINE_CHANNEL_SUBWOOFER_HIGH) {
                channel = 5;
            } else if (channel == NINE_CHANNEL_CENTER_LOW) {
                channel = 8;
            } else if (channel == NINE_CHANNEL_SUBWOOFER_LOW) {
                channel = 6;
            }
            int[] data = new int[]{channel, lpfFreq, lpfSlope, 0}; // 默认
            if (channel == NINE_CHANNEL_FRONT_HIGH || channel == NINE_CHANNEL_REAR_HIGH || channel == NINE_CHANNEL_CENTER_HIGH || channel == NINE_CHANNEL_SUBWOOFER_HIGH) {
                data = new int[]{channel, hpfFreq, hpfSlope, 0};
            }
            NativeHelper.getEq().setEqHpfLpf(data);
            Log.d(TAG, String.format("nativeHLPF data : %s", Arrays.toString(data)));
            return;
        }
        if (EqUtils.isChip7739()){
            int[] data = new int[]{channel, lpfSlope, lpfFreq, hpfSlope, hpfFreq};
            NativeHelper.getEq().setEqHpfLpf(data);
           return;
        }
        int realChannel = channel;
        if (channel == NINE_CHANNEL_FRONT_HIGH || channel == NINE_CHANNEL_FRONT_LOW) {
            realChannel = INDEX_LPF_HPF_F;
        } else if (channel == NINE_CHANNEL_REAR_HIGH || channel == NINE_CHANNEL_REAR_LOW) {
            realChannel = INDEX_LPF_HPF_R;
        } else if (channel == NINE_CHANNEL_CENTER_HIGH || channel == NINE_CHANNEL_CENTER_LOW) {
            realChannel = INDEX_LPF_HPF_CEN;
        } else if (channel == NINE_CHANNEL_SUBWOOFER_HIGH || channel == NINE_CHANNEL_SUBWOOFER_LOW) {
            realChannel = INDEX_LPF_HPF_SUB;
        }

        //需要高低滤音
        int[] data = new int[]{realChannel, lpfSlope, lpfFreq, hpfSlope, hpfFreq};
        NativeHelper.getEq().setEqHpfLpf(data);
        Log.d(TAG, String.format("nativeHLPF data : %s", Arrays.toString(data)));
    }

    public boolean getSlopeSwitch(int i) {
        Log.d(TAG, "getSlopeSwitch channel: " + i + " isOpen:" + spUtils.getBoolean(KEY_QVALUE_SWITCH + i, true) + " value:" + spUtils.getInt(KEY_QVALUE + i, 1));
        return spUtils.getBoolean(KEY_QVALUE_SWITCH + i, true);
    }

    public void saveSlopeSwitch(int i, boolean z) {
        Log.d(TAG, "saveSlopeSwitch channel: " + i + " enable: " + z);
        spUtils.put(KEY_QVALUE_SWITCH + i, z);
    }

    public void saveHLPF(int i, int i2, int i3) {
        Log.d(TAG, String.format("saveHLPF channel : %d, freq : %d, qValue : %d", Integer.valueOf(i), Integer.valueOf(i2), Integer.valueOf(i3)));
        spUtils.put(KEY_CHANNEL, i);
        spUtils.put(KEY_FREQ + i, i2);
        spUtils.put(KEY_QVALUE + i, i3, true);
    }

    public int getHLPFChannel() {
        return spUtils.getInt(KEY_CHANNEL, 1);
    }

    public int getFreq(int channel) {
        int defaultFreq;
        if (channel == NINE_CHANNEL_FRONT_HIGH) {
            defaultFreq = NINE_HLPF_INDEX_LPF_HPF_F_MIN_DEFAULT;
        } else if (channel == NINE_CHANNEL_REAR_HIGH) {
            defaultFreq = NINE_HLPF_INDEX_LPF_HPF_R_MIN_DEFAULT;
        } else if (channel == NINE_CHANNEL_FRONT_LOW) {
            defaultFreq = NINE_HLPF_INDEX_LPF_HPF_F_MAX_DEFAULT;
        } else if (channel == NINE_CHANNEL_REAR_LOW) {
            defaultFreq = NINE_HLPF_INDEX_LPF_HPF_R_MAX_DEFAULT;
        } else if (channel == NINE_CHANNEL_CENTER_HIGH) {
            defaultFreq = NINE_HLPF_INDEX_LPF_HPF_CEN_MIN_DEFAULT;
        } else if (channel == NINE_CHANNEL_CENTER_LOW) {
            defaultFreq = NINE_HLPF_INDEX_LPF_HPF_CEN_MAX_DEFAULT;
        } else if (channel == NINE_CHANNEL_SUBWOOFER_HIGH) { // 重低音的高通默认值
            defaultFreq = NINE_HLPF_INDEX_LPF_HPF_SUB_MIN_DEFAULT;
        } else { // 重低音的低通默认值
            defaultFreq = NINE_HLPF_INDEX_LPF_HPF_SUB_MAX_DEFAULT;
        }
        return spUtils.getInt(KEY_FREQ + channel, defaultFreq);
    }

    // 斜率选项，0关闭，1为第一项
    public int getQValue(int i) {
        Log.d(TAG, "getQValue channel: " + i + "value: " + spUtils.getInt(KEY_QVALUE + i, 1));
        return spUtils.getInt(KEY_QVALUE + i, 1);
    }

    public void resetByChannel(int channelHigh, int channelLow) {
        spUtils.remove(KEY_FREQ + channelHigh);
        spUtils.remove(KEY_QVALUE + channelHigh, true);
        spUtils.remove(KEY_FREQ + channelLow);
        spUtils.remove(KEY_QVALUE + channelLow, true);
        spUtils.remove(KEY_QVALUE_SWITCH + channelHigh);
        spUtils.remove(KEY_QVALUE_SWITCH + channelLow);
    }

    public void resetCurrentChannel(int channelHigh, int channelLow) {
        if ("gb05".equals(EqUtils.getSkinName()) || EqUtils.isChip7739()) {
            nativeHLPF(channelLow, getQValue(channelLow), getFreq(channelLow), getQValue(channelHigh), getFreq(channelHigh));
            nativeHLPF(channelHigh, getQValue(channelLow), getFreq(channelLow), getQValue(channelHigh), getFreq(channelHigh));
        } else {
            nativeHLPF(channelHigh, getQValue(channelLow), getFreq(channelLow), getQValue(channelHigh), getFreq(channelHigh));
        }
    }


    public void nativeAll(int... channels) {
        for (int channel : channels) {
            int channelHigh, channelLOW;
            int freqHigh, freqLow;
            int qValueHigh, qValueLow;
            if (channel == INDEX_LPF_HPF_F) {
                channelHigh = NINE_CHANNEL_FRONT_HIGH;
                channelLOW = NINE_CHANNEL_FRONT_LOW;
            } else if (channel == INDEX_LPF_HPF_R) {
                channelHigh = NINE_CHANNEL_REAR_HIGH;
                channelLOW = NINE_CHANNEL_REAR_LOW;
            } else if (channel == INDEX_LPF_HPF_CEN) {
                channelHigh = NINE_CHANNEL_CENTER_HIGH;
                channelLOW = NINE_CHANNEL_CENTER_LOW;
            } else {
                channelHigh = NINE_CHANNEL_SUBWOOFER_HIGH;
                channelLOW = NINE_CHANNEL_SUBWOOFER_LOW;
            }
            freqHigh = getFreq(channelHigh);
            freqLow = getFreq(channelLOW);
            qValueHigh = getSlopeSwitch(channelHigh) ? getQValue(channelHigh) : 0;
            qValueLow = getSlopeSwitch(channelLOW) ? getQValue(channelLOW) : 0;
            if ("gb05".equals(EqUtils.getSkinName()) || EqUtils.isChip7739()) {
                nativeHLPF(channelLOW, qValueLow, freqLow, qValueHigh, freqHigh);
                nativeHLPF(channelHigh, qValueLow, freqLow, qValueHigh, freqHigh);
            } else {
                nativeHLPF(channelHigh, qValueLow, freqLow, qValueHigh, freqHigh);
            }
        }
    }
}