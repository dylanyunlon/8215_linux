package com.hcn.autoeq.data;

import static com.hcn.autoeq.nativeextdsp.SI47925.SI47925_INDEX_CHANEL_CEN;
import static com.hcn.autoeq.nativeextdsp.SI47925.SI47925_INDEX_CHANEL_FL;
import static com.hcn.autoeq.nativeextdsp.SI47925.SI47925_INDEX_CHANEL_FR;
import static com.hcn.autoeq.nativeextdsp.SI47925.SI47925_INDEX_CHANEL_RL;
import static com.hcn.autoeq.nativeextdsp.SI47925.SI47925_INDEX_CHANEL_RR;
import static com.hcn.autoeq.nativeextdsp.SI47925.SI47925_INDEX_CHANEL_SUB;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.blankj.utilcode.util.StringUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.SIConstantExtDsp;
import com.hcn.autoeq.util.SI47925EqDealDataUtil;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * si47925协议
 */
public class SIExtDspDelaySettings implements SIConstantExtDsp {

    private static final String TAG = SIExtDspDelaySettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(SIExtDspDelaySettings.class.getSimpleName(), Log.DEBUG);

    private static final String SI_EXT_DSP_DELAY_FILE = "si_ext_dsp_delay"; // 各模式的延时保存的文件名
    private static final String SI_KEY_DELAY = "si_ext_dsp_delay_";

    public static final String CHANEL_FL = "LF";
    public static final String CHANEL_FR = "RF";
    public static final String CHANEL_RL = "LR";
    public static final String CHANEL_RR = "RR";
    public static final String CHANEL_CEN = "CENTER";
    public static final String CHANEL_SUB = "SUBWOOFER";

    //通道
    public static Map<String, Integer> map = new HashMap<>();


    private Context context;
    private static SIExtDspDelaySettings extDspDelaySettings = null;
    private SPUtils spUtils;

    public static SIExtDspDelaySettings getInstance(Context mContext) {
        if (null == extDspDelaySettings) {
            extDspDelaySettings = new SIExtDspDelaySettings(mContext);
            map.put(CHANEL_FL, SI47925_INDEX_CHANEL_FL);
            map.put(CHANEL_FR, SI47925_INDEX_CHANEL_FR);
            map.put(CHANEL_RL, SI47925_INDEX_CHANEL_RL);
            map.put(CHANEL_RR, SI47925_INDEX_CHANEL_RR);
            map.put(CHANEL_CEN, SI47925_INDEX_CHANEL_SUB);
            map.put(CHANEL_SUB, SI47925_INDEX_CHANEL_CEN);
        }
        return extDspDelaySettings;
    }

    private SIExtDspDelaySettings(Context context) {
        this.context = context;
        spUtils = SPUtils.getInstance(SI_EXT_DSP_DELAY_FILE);
    }

    public void nativeDelay(int channelId, int delay, int polarity) {
        Log.d(TAG, "nativeDelay: channelId = " + channelId + " delay = " + delay + " polarity = " + polarity);
        int[] data = new int[]{channelId, delay == 0 ? 1 : 0, delay, polarity};
        NativeHelper.getEq().setEqSpeakerDelay(data);
        Log.d(TAG, String.format("si nativeDelay data : %s", Arrays.toString(data)));
    }

    public void saveDelay(String channel, int delay) {
        Log.d(TAG, String.format("si saveDelay channel : %s, delay : %d", channel, delay));
        spUtils.put(SI_KEY_DELAY + channel, delay, true);
    }

    public int getDelay(String channel) {
        if (StringUtils.isEmpty(channel)) {
            return 0;
        }
        Log.d(TAG, "getDelay: " + spUtils.getInt(SI_KEY_DELAY + channel, 0));
        return spUtils.getInt(SI_KEY_DELAY + channel, 0);
    }

    public void resetDelay() {
        Log.d(TAG, "si resetDelay");
        spUtils.clear(true);
    }

    public void nativeAll(String... channels) {
        SIExtDspAttenuateSettings extDspAttenuateSettings = SIExtDspAttenuateSettings.getInstance(context);
        for (String channel : channels) {
            int channelId = getChannelId(channel);
            int delay = getDelay(channel);
            int polarity = extDspAttenuateSettings.getInvert(channel) ? 1 : 0;
            nativeDelay(channelId, delay, polarity);
        }
    }

    public static int getChannelId(String channels) {
        if (map == null || StringUtils.isEmpty(channels)) {
            return 0;
        }
        Integer value = map.get(channels);
        return value != null ? value : 0;
    }


}
