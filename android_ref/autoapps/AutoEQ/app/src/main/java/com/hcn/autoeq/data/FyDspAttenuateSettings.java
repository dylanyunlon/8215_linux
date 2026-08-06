package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.hcn.autoeq.nativeextdsp.FY7604;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantFyDsp;

import java.util.Arrays;

public class FyDspAttenuateSettings extends FyDspBaseSettings implements ConstantFyDsp {

    private static final String TAG = FyDspAttenuateSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(FyDspAttenuateSettings.class.getSimpleName(), Log.DEBUG);

    private static final String FY_DSP_ATTENUATE_FILE = "v2_fy_dsp_attenuate"; // 各模式的衰减及反相保存的文件名
    private static final String KEY_ATTENUATE = "fy_dsp_attenuate_";
    private static final String KEY_MUTE = "fy_dsp_mute_";
    private static final String KEY_INVERT = "fy_dsp_invert_";

    private static final String KEY_LINK_LF_RF = "fy_dsp_link_lf_rf";
    private static final String KEY_LINK_LR_RR = "fy_dsp_link_lr_rr";
    private static final String KEY_LINK_CENTER_SUBWOOFER = "fy_dsp_link_center_subwoofer";

    private Context context;
    private static FyDspAttenuateSettings fyDspAttenuateSettings = null;

    public static FyDspAttenuateSettings getInstance(Context context) {
        if (null == fyDspAttenuateSettings) {
            fyDspAttenuateSettings = new FyDspAttenuateSettings(context);
        }
        return fyDspAttenuateSettings;
    }

    private FyDspAttenuateSettings(Context context) {
        super(FY_DSP_ATTENUATE_FILE);
        this.context = context;
    }

    public void nativeAttenuate(String channel, int attenuate, boolean mute, boolean revert) {
        int[] data = new int[]{FY7604.FY_CMD_SUB_ID_ATTENUATE_INVERT_SPEAKER, convertChannel(channel), attenuate, mute ? 1 : 0, revert ? 1 : 0};
        Log.d(TAG, String.format("nativeAttenuate data : %s", Arrays.toString(data)));
        NativeHelper.getEq().setEqAttSpeaker(data);
    }

    public void saveAttenuate(String channel, int attenuate, boolean mute, boolean revert) {
        Log.d(TAG, String.format("saveAttenuate channel : %s, attenuate : %d, mute : %b, revert : %b", channel, attenuate, mute, revert));
        spUtils.put(KEY_ATTENUATE + channel, attenuate);
        spUtils.put(KEY_MUTE + channel, mute);
        spUtils.put(KEY_INVERT + channel, revert, true);
    }

    public void saveLink(boolean lfRf, boolean lrRr, boolean cs) {
        Log.d(TAG, String.format("saveLink lfRf : %b, lrRr : %b, cs : %b", lfRf, lrRr, cs));
        spUtils.put(KEY_LINK_LF_RF, lfRf);
        spUtils.put(KEY_LINK_LR_RR, lrRr, true);
        spUtils.put(KEY_LINK_CENTER_SUBWOOFER, cs, true);
    }

    public boolean getLinkLfRf() {
        return getSpUtils().getBoolean(KEY_LINK_LF_RF, false);
    }

    public boolean getLinkLrRr() {
        return getSpUtils().getBoolean(KEY_LINK_LR_RR, false);
    }

    public boolean getLinkCenterSubwoofer() {
        return getSpUtils().getBoolean(KEY_LINK_CENTER_SUBWOOFER, false);
    }

    public int getAttenuate(String channel) {
        return getSpUtils().getInt(KEY_ATTENUATE + channel, DEF_ATTENUATE);
    }

    public boolean getMute(String channel) {
        return getSpUtils().getBoolean(KEY_MUTE + channel, false);
    }

    public boolean getInvert(String channel) {
        return getSpUtils().getBoolean(KEY_INVERT + channel, false);
    }

    public void reset() {
        spUtils.clear(true);
    }

    private int convertChannel(String channel) {
        if ("LF".equals(channel)) return 1;
        if ("RF".equals(channel)) return 2;
        if ("LR".equals(channel)) return 3;
        if ("RR".equals(channel)) return 4;
        if ("CENTER".equals(channel)) return 5; // right
        if ("SUBWOOFER".equals(channel)) return 6; // left

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
