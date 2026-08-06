package com.hcn_library.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.blankj.utilcode.util.StringUtils;
import com.hcn_library.nativeextdsp.Ak7604;
import com.hcn_library.nativeextdsp.NativeHelper;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;


public class NineDspDelaySettings implements NineConstantExtDsp {
    public static final String CHANEL_CEN = "CENTER";
    public static final String CHANEL_FL = "LF";
    public static final String CHANEL_FR = "RF";
    public static final String CHANEL_RL = "LR";
    public static final String CHANEL_RR = "RR";
    public static final String CHANEL_SUB = "SUBWOOFER";
    private static final String NINE_DSP_DELAY_FILE = "nine_dsp_delay";
    private static final String NINE_KEY_DELAY = "NINE_dsp_delay_";
    private Context context;
    private SPUtils spUtils = SPUtils.getInstance(NINE_DSP_DELAY_FILE);
    private static final String TAG = "NineDspDelaySettings";

    private static final Map<String, Integer> channelMap = new HashMap<>();
    private static NineDspDelaySettings nineDspDelaySettings = null;


    public static NineDspDelaySettings getInstance(Context context) {
        if (nineDspDelaySettings == null) {
            nineDspDelaySettings = new NineDspDelaySettings(context);
            // 初始化通道映射
            channelMap.put(CHANEL_FL, 0);
            channelMap.put(CHANEL_FR, 1);
            channelMap.put(CHANEL_RL, 2);
            channelMap.put(CHANEL_RR, 3);
            channelMap.put(CHANEL_CEN, 4);
            channelMap.put(CHANEL_SUB, 5);
        }
        return nineDspDelaySettings;
    }


    private NineDspDelaySettings(Context context) {
        this.context = context;
    }


    public void nativeDelay(int channelId, int delay, int polarity) {
        Log.d(TAG, "nativeDelay: channelId = " + channelId + " delay = " + delay + " polarity = " + polarity);
        NativeHelper.getEq().setEqSpeakerDelay(new int[]{channelId, (delay == 0 && polarity == 0) ? 1 : 0, delay, polarity});
        Log.d(TAG, String.format("nativeDelay data : [%d, %d, %d, %d]", channelId, (delay == 0 && polarity == 0) ? 1 : 0, delay, polarity));
    }

    // 7604_c芯片同时传递所有通道数据
    public void nativeDelay7604C() {
        int delayLF = getDelay("LF");
        int delayRF = getDelay("RF");
        int delayLR = getDelay("LR");
        int delayRR = getDelay("RR");
        int sub = getDelay("SUBWOOFER");
        int cen = getDelay("CENTER");
        int[] arrays = {delayLF, delayRF, delayLR, delayRR, sub, cen};
        NativeHelper.getEq().setEqSpeakerDelay(arrays);
        Log.d(TAG, String.format("nativeDelay data : %s", Arrays.toString(arrays)));
    }

    public void saveDelay(String channel, int delay) {
        Log.d(TAG, String.format("saveDelay channel : %s, delay : %d", channel, delay));
        spUtils.put(NINE_KEY_DELAY + channel, delay, true);
    }


    public int getDelay(String channel) {
        if (StringUtils.isEmpty(channel)) {
            return 0;
        }
        int delay = spUtils.getInt(NINE_KEY_DELAY + channel, 0);
        Log.d(TAG, " channel: " + channel + "    getDelay: " + delay);
        return delay;
    }


    public void resetDelay() {
        Log.d(TAG, "resetDelay");
        spUtils.clear(true);
        nativeAll(CHANEL_FL, CHANEL_FR, CHANEL_RL, CHANEL_RR, CHANEL_SUB, CHANEL_CEN);
    }


    public void nativeAll(String... channels) {
        NineDspAttenuateSettings attenuateSettings = NineDspAttenuateSettings.getInstance(context);
        if ("gb05".equals(EqUtils.getSkinName())) {
            nativeDelay7604C();
        } else {
            for (String channel : channels) {
                nativeDelay(getChannelId(channel), getDelay(channel), attenuateSettings.getInvert(channel) ? 1 : 0);
            }
        }
    }


    public static int getChannelId(String channel) {
        if (StringUtils.isEmpty(channel) || channelMap.get(channel) == null) {
            return 0;
        }
        return channelMap.get(channel);
    }
}