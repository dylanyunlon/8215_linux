package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.hcn.autoeq.nativeextdsp.FY7604;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantFyDsp;

import java.util.Arrays;

public class FyDspDelaySettings extends FyDspBaseSettings implements ConstantFyDsp {

    private static final String TAG = FyDspDelaySettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(FyDspDelaySettings.class.getSimpleName(), Log.DEBUG);
    public static final int DELAY_PRECISION = 10; // delay值百分比，数值范围是0~1800，显示是0.1~18，进度条范围是0~180，所以再*10

    private static final String FY_DSP_DELAY_FILE = "v2_fy_dsp_delay"; // 各模式的延时保存的文件名
    private static final String KEY_DELAY = "fy_dsp_delay_";

    private Context context;
    private static FyDspDelaySettings extDspDelaySettings = null;

    public static FyDspDelaySettings getInstance(Context context) {
        if (null == extDspDelaySettings) {
            extDspDelaySettings = new FyDspDelaySettings(context);
        }
        return extDspDelaySettings;
    }

    private FyDspDelaySettings(Context context) {
        super(FY_DSP_DELAY_FILE);
        this.context = context;
    }

    private void nativeDelay(int delayLF, int delayRF, int delayLR, int delayRR, int delayCENTER, int delaySUBWOOFER) {
        int[] data = new int[]{FY7604.FY_CMD_SUB_ID_DELAY
                , delayLF * DELAY_PRECISION, delayRF * DELAY_PRECISION, delayLR * DELAY_PRECISION, delayRR * DELAY_PRECISION
                , delayCENTER * DELAY_PRECISION, delaySUBWOOFER * DELAY_PRECISION};
        NativeHelper.getEq().setEqSpeakerDelay(data);
        Log.d(TAG, String.format("nativeDelay data : %s", Arrays.toString(data)));
    }

    public void saveDelay(String channel, int delay) {
        Log.d(TAG, String.format("saveDelay channel : %s, delay : %d", channel, delay));
        spUtils.put(KEY_DELAY + channel, delay, true);
    }

    public int getDelay(String channel) {
        return getSpUtils().getInt(KEY_DELAY + channel, 0);
    }

    public void nativeAll() {
        int delayLF = getDelay("LF");
        int delayRF = getDelay("RF");
        int delayLR = getDelay("LR");
        int delayRR = getDelay("RR");
        int delayCENTER = getDelay("CENTER");
        int delaySUBWOOFER = getDelay("SUBWOOFER");
        nativeDelay(delayLF, delayRF, delayLR, delayRR, delayCENTER, delaySUBWOOFER);
    }

}
