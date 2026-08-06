package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.SIConstantExtDsp;

import java.util.Arrays;

public class SIExtDspAttenuateSettings implements SIConstantExtDsp {

    private static final String TAG = SIExtDspAttenuateSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(SIExtDspAttenuateSettings.class.getSimpleName(), Log.DEBUG);

    private static final String SI_EXT_DSP_ATTENUATE_FILE = "si_ext_dsp_attenuate"; // 各模式的衰减及反相保存的文件名
    private static final String SI_KEY_ATTENUATE = "si_ext_dsp_attenuate_";
    private static final String SI_KEY_MUTE = "si_ext_dsp_mute_";
    private static final String SI_KEY_INVERT = "si_ext_dsp_invert_";

    private static final String SI_KEY_LINK_LF_RF = "si_ext_dsp_link_lf_rf";
    private static final String SI_KEY_LINK_LR_RR = "si_ext_dsp_link_lr_rr";

    private Context context;
    private static SIExtDspAttenuateSettings extDspAttenuateSettings = null;
    private SPUtils spUtils;

    public static SIExtDspAttenuateSettings getInstance(Context mContext) {
        if (null == extDspAttenuateSettings) {
            extDspAttenuateSettings = new SIExtDspAttenuateSettings(mContext);
        }
        return extDspAttenuateSettings;
    }

    private SIExtDspAttenuateSettings(Context context) {
        this.context = context;
        spUtils = SPUtils.getInstance(SI_EXT_DSP_ATTENUATE_FILE);
    }

    public void nativeAttenuate(String channel, int attenuate, boolean mute) {
        int[] data = new int[]{convertChannel(channel), attenuate, mute ? 1 : 0};
        NativeHelper.getEq().setEqAttSpeaker(data);
        Log.d(TAG, String.format("nativeAttenuate data : %s", Arrays.toString(data)));
    }

    public void nativeAttenuateSI(String channel, int attenuate, boolean mute, boolean isInvert) {
        Log.d(TAG, "nativeAttenuateSI: channel = " + channel + " attenuate = " + attenuate + "mute = " + attenuate + " isInvert = " + isInvert);
        int[] data = new int[]{convertChannel(channel), attenuate, mute ? 1 : 0, isInvert ? 1 : 0};
        NativeHelper.getEq().setEqAttSpeaker(data);
        Log.d(TAG, String.format("nativeAttenuateSI data : %s", Arrays.toString(data)));
    }

    public void nativeAttenuateSI(String channel, int attenuate, boolean mute, boolean isInvert, boolean change) {
        Log.d(TAG, "nativeAttenuateSI: channel = " + channel + " attenuate = " + attenuate + "mute = " + attenuate + " isInvert = " + isInvert);
        int[] data = new int[]{convertChannel(channel), attenuate, mute ? 1 : 0, isInvert ? 1 : 0};
        if (change) {
            SIExtDspDelaySettings settings = SIExtDspDelaySettings.getInstance(context);
            int delay = settings.getDelay(channel);
            int polarity = isInvert ? 1 : 0;
            data = new int[]{convertChannel(channel), delay == 0 && polarity == 0 ? 1 : 0, delay, polarity};
            NativeHelper.getEq().setEqSpeakerDelay(data);
        } else {
            NativeHelper.getEq().setEqAttSpeaker(data);
        }
        Log.d(TAG, String.format("nativeAttenuateSI data : %s", Arrays.toString(data)));
    }

    public void saveAttenuate(String channel, int attenuate, boolean mute) {
        Log.d(TAG, String.format("saveAttenuate channel : %s, attenuate : %d, mute : %b", channel, attenuate, mute));
        spUtils.put(SI_KEY_ATTENUATE + channel, attenuate);
        spUtils.put(SI_KEY_MUTE + channel, mute);
    }

    public void saveAttenuateSI(String channel, int attenuate, boolean mute, boolean isInvert) {
        Log.d(TAG, String.format("saveAttenuateSI channel : %s, attenuate : %d, mute : %b, isInvert %b", channel, attenuate, mute, isInvert));
        spUtils.put(SI_KEY_ATTENUATE + channel, attenuate);
        spUtils.put(SI_KEY_MUTE + channel, mute);
        spUtils.put(SI_KEY_INVERT + channel, isInvert);
    }

    public void saveLink(boolean lfRf, boolean lrRr) {
        Log.d(TAG, String.format("saveLink lfRf : %b, lrRr : %b", lfRf, lrRr));
        spUtils.put(SI_KEY_LINK_LF_RF, lfRf);
        spUtils.put(SI_KEY_LINK_LR_RR, lrRr, true);
    }

    public boolean getLinkLfRf() {
        return spUtils.getBoolean(SI_KEY_LINK_LF_RF, false);
    }

    public boolean getLinkLrRr() {
        return spUtils.getBoolean(SI_KEY_LINK_LR_RR, false);
    }

    public int getAttenuate(String channel) {
        return spUtils.getInt(SI_KEY_ATTENUATE + channel, SIConstantExtDsp.DEF_ATTENUATE);
    }

    public boolean getMute(String channel) {
        return spUtils.getBoolean(SI_KEY_MUTE + channel, false);
    }

    public boolean getInvert(String channel) {
        return spUtils.getBoolean(SI_KEY_INVERT + channel, false);

    }

    private int convertChannel(String channel) {
        if ("LF".equals(channel)) return 0;
        if ("RF".equals(channel)) return 1;
        if ("LR".equals(channel)) return 2;
        if ("RR".equals(channel)) return 3;
        if ("CENTER".equals(channel)) return 4;
        if ("SUBWOOFER".equals(channel)) return 5;
        return -1;
    }

    public void nativeAll(String... channels) {
        for (String channel : channels) {
            int attenuate = getAttenuate(channel);
            boolean mute = getMute(channel);
            boolean isInvert = getInvert(channel);
            nativeAttenuateSI(channel, attenuate, mute, isInvert, true);
            nativeAttenuateSI(channel, attenuate, mute, isInvert);
        }
    }

}
