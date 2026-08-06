package com.hcn.autoeq.data;

import static com.hcn.autoeq.nativeextdsp.Ak7604.INDEX_HPF_CEN;
import static com.hcn.autoeq.nativeextdsp.Ak7604.INDEX_HPF_F;
import static com.hcn.autoeq.nativeextdsp.Ak7604.INDEX_HPF_R;
import static com.hcn.autoeq.nativeextdsp.Ak7604.INDEX_HPF_SUB;
import static com.hcn.autoeq.nativeextdsp.Ak7604.INDEX_LPF_F;
import static com.hcn.autoeq.nativeextdsp.Ak7604.INDEX_LPF_R;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantExtDsp;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class ExtDspDelaySettings implements ConstantExtDsp {

    private static final String TAG = ExtDspDelaySettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(ExtDspDelaySettings.class.getSimpleName(), Log.DEBUG);

    private static final String EXT_DSP_DELAY_FILE = "ext_dsp_delay"; // 各模式的延时保存的文件名
    private static final String KEY_DELAY = "ext_dsp_delay_";

    private Context context;

    public static final String CHANEL_FL = "chanel_fl";
    public static final String CHANEL_FR = "chanel_fr";
    public static final String CHANEL_RL = "chanel_rl";
    public static final String CHANEL_RR = "chanel_rr";
    public static final String CHANEL_CEN = "chanel_cen";
    public static final String CHANEL_SUB = "chanel_sub";
    public static Map<String, Integer> map = new HashMap<>();
    private static ExtDspDelaySettings extDspDelaySettings = null;
    private SPUtils spUtils;

    public static ExtDspDelaySettings getInstance(Context mContext) {
        if (null == extDspDelaySettings) {
            extDspDelaySettings = new ExtDspDelaySettings(mContext);
            map.put(CHANEL_FL, INDEX_HPF_F);
            map.put(CHANEL_FR, INDEX_LPF_F);
            map.put(CHANEL_RL, INDEX_HPF_R);
            map.put(CHANEL_RR, INDEX_LPF_R);
            map.put(CHANEL_CEN, INDEX_HPF_SUB);
            map.put(CHANEL_SUB, INDEX_HPF_CEN);
        }
        return extDspDelaySettings;
    }

    private ExtDspDelaySettings(Context context) {
        this.context = context;
        spUtils = SPUtils.getInstance(EXT_DSP_DELAY_FILE);
    }

    public void nativeDelay(int... arrays) {
        NativeHelper.getEq().setEqSpeakerDelay(arrays);
        Log.d(TAG, String.format("nativeDelay data : %s", Arrays.toString(arrays)));
    }

    public void saveDelay(String channel, int delay) {
        Log.d(TAG, String.format("saveDelay channel : %s, delay : %d", channel, delay));
        spUtils.put(KEY_DELAY + channel, delay, true);
    }

    public int getDelay(String channel) {
        return spUtils.getInt(KEY_DELAY + channel, 0);
    }

    public void resetDelay() {
        Log.d(TAG, "resetDelay");
        spUtils.clear(true);
    }

    public void nativeAll() {
        int delayLF = getDelay("LF");
        int delayRF = getDelay("RF");
        int delayLR = getDelay("LR");
        int delayRR = getDelay("RR");
        nativeDelay(delayLF, delayRF, delayLR, delayRR);
    }

    public void nativeAll8581() {
        int delayLF = getDelay("LF");
        int delayRF = getDelay("RF");
        int delayLR = getDelay("LR");
        int delayRR = getDelay("RR");
        int sub = getDelay("SUBWOOFER");
        int cen = getDelay("CENTER");
        nativeDelay(delayLF, delayRF, delayLR, delayRR, sub, cen);
    }

}
