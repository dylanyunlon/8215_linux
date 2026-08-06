package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantExtDsp;

import java.util.Arrays;

public class ExtDspSurroundSettings implements ConstantExtDsp {

    private static final String TAG = ExtDspSurroundSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(ExtDspSurroundSettings.class.getSimpleName(), Log.DEBUG);

    private static final String EXT_DSP_SURROUND_FILE = "ext_dsp_surround"; // 环绕保存的文件名
    private static final String KEY_SURROUND_ENABLE = "ext_dsp_surround_enable";
    private static final String KEY_LOUDNESS_ENABLE = "ext_dsp_loudness_enable";

    private Context context;
    private static ExtDspSurroundSettings extDspSurroundSettings = null;
    private SPUtils spUtils;

    public static ExtDspSurroundSettings getInstance(Context mContext) {
        if (null == extDspSurroundSettings) {
            extDspSurroundSettings = new ExtDspSurroundSettings(mContext);
        }
        return extDspSurroundSettings;
    }

    private ExtDspSurroundSettings(Context context) {
        this.context = context;
        spUtils = SPUtils.getInstance(EXT_DSP_SURROUND_FILE);
    }

    public void nativeSurround(int enable) {
        int[] data = new int[]{enable};
        NativeHelper.getEq().setEqSurround(data);
        Log.d(TAG, String.format("nativeSurround data : %s", Arrays.toString(data)));
    }

    public void saveSurround(int enable) {
        Log.d(TAG, String.format("saveSurround enable : %d", enable));
        spUtils.put(KEY_SURROUND_ENABLE, enable, true);
    }

    public int getSurround() {
        return spUtils.getInt(KEY_SURROUND_ENABLE, DEF_SURROUND_ENABLE_STATUS);
    }

    public void nativeLoudness(int enable) {
        int[] data = new int[]{enable};
        NativeHelper.getEq().setEqLoudness(data);
        Log.d(TAG, String.format("nativeLoudness data : %s", Arrays.toString(data)));
    }

    public void saveLoudness(int enable) {
        Log.d(TAG, String.format("saveLoudness enable : %d", enable));
        spUtils.put(KEY_LOUDNESS_ENABLE, enable, true);
    }

    public int getLoudness() {
        return spUtils.getInt(KEY_LOUDNESS_ENABLE, DEF_LOUDNESS_ENABLE_STATUS);
    }

}
