package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantExtDsp;

import java.util.Arrays;

public class ExtDspAttenuateSettings implements ConstantExtDsp {

    private static final String TAG = ExtDspAttenuateSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(ExtDspAttenuateSettings.class.getSimpleName(), Log.DEBUG);

    private static final String EXT_DSP_ATTENUATE_FILE = "ext_dsp_attenuate"; // 各模式的衰减及反相保存的文件名
    private static final String KEY_ATTENUATE = "ext_dsp_attenuate_";
    private static final String KEY_MUTE = "ext_dsp_mute_";
    private static final String KEY_INVERT = "ext_dsp_invert_";

    private static final String KEY_LINK_LF_RF = "ext_dsp_link_lf_rf";
    private static final String KEY_LINK_LR_RR = "ext_dsp_link_lr_rr";

    private Context context;
    private static ExtDspAttenuateSettings extDspAttenuateSettings = null;
    private SPUtils spUtils;

    public static ExtDspAttenuateSettings getInstance(Context mContext) {
        if (null == extDspAttenuateSettings) {
            extDspAttenuateSettings = new ExtDspAttenuateSettings(mContext);
        }
        return extDspAttenuateSettings;
    }

    private ExtDspAttenuateSettings(Context context) {
        this.context = context;
        spUtils = SPUtils.getInstance(EXT_DSP_ATTENUATE_FILE);
    }

    public void nativeAttenuate(String channel, int attenuate, boolean mute, boolean revert) {
        int[] data = new int[]{convertChannel(channel), attenuate, mute ? 1 : 0, revert ? 1 : 0};
        NativeHelper.getEq().setEqAttSpeaker(data);
        Log.d(TAG, String.format("nativeAttenuate data : %s", Arrays.toString(data)));
    }

    public void saveAttenuate(String channel, int attenuate, boolean mute, boolean revert) {
        Log.d(TAG, String.format("saveAttenuate channel : %s, attenuate : %d, mute : %b, revert : %b", channel, attenuate, mute, revert));
        spUtils.put(KEY_ATTENUATE + channel, attenuate);
        spUtils.put(KEY_MUTE + channel, mute);
        spUtils.put(KEY_INVERT + channel, revert, true);
    }

    public void saveLink(boolean lfRf, boolean lrRr) {
        Log.d(TAG, String.format("saveLink lfRf : %b, lrRr : %b", lfRf, lrRr));
        spUtils.put(KEY_LINK_LF_RF, lfRf);
        spUtils.put(KEY_LINK_LR_RR, lrRr, true);
    }

    public boolean getLinkLfRf() {
        return spUtils.getBoolean(KEY_LINK_LF_RF, false);
    }

    public boolean getLinkLrRr() {
        return spUtils.getBoolean(KEY_LINK_LR_RR, false);
    }

    public int getAttenuate(String channel) {
        return spUtils.getInt(KEY_ATTENUATE + channel, ConstantExtDsp.DEF_ATTENUATE);
    }

    public boolean getMute(String channel) {
        return spUtils.getBoolean(KEY_MUTE + channel, false);
    }

    public boolean getInvert(String channel) {
        return spUtils.getBoolean(KEY_INVERT + channel, false);
    }

    private int convertChannel(String channel) {
        if ("LF".equals(channel)) return 1;
        if ("RF".equals(channel)) return 2;
        if ("LR".equals(channel)) return 3;
        if ("RR".equals(channel)) return 4;
        if ("SUBWOOFER".equals(channel)) return 5;
        if ("CENTER".equals(channel)) return 6;
        return -1;
    }

    public void nativeAll(String... channels) {
        for (String channel : channels) {
            int attenuate = getAttenuate(channel);
            boolean mute = getMute(channel);
            boolean invert = getInvert(channel);
            nativeAttenuate(channel, attenuate, mute, invert);
        }
    }

}
