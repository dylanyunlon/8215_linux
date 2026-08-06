package com.hcn_library.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn_library.nativeextdsp.NativeHelper;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;

import java.util.Arrays;

public class NineDspDtsVirtualCenterSettings implements NineConstantExtDsp {
    private static final int DEF_FOCUS_CENTER_LV = 50;
    private static final int DEF_FOCUS_FRONT_LV = 16384;
    private static final int DEF_FOCUS_REAR_LV = 16384;
    private static final String KEY_VIRTUAL_CENTER_LV = "nine_dsp_virtual_dts_center_lv";
    private static final String KEY_FOCUS_FRONT_LV = "nine_dsp_dts_focus_front_lv";
    private static final String KEY_FOCUS_REAR_LV = "nine_dsp_dts_focus_rear_lv";
    private static final String KEY_INDEX_CMD = "NINE_dsp_dts_index_id";
    public static final int MAX_LV = 32767;
    private static final String NINE_DSP_DTS_FILE = "nine_dsp_switch";
    private Context context;
    private SPUtils spUtils = SPUtils.getInstance(NINE_DSP_DTS_FILE);
    private static final String TAG = "NineDspDtsVirtualCenterSettings";
    private static NineDspDtsVirtualCenterSettings nineDspDtsFilterSettings = null;

    public static NineDspDtsVirtualCenterSettings getInstance(Context context) {
        if (nineDspDtsFilterSettings == null) {
            nineDspDtsFilterSettings = new NineDspDtsVirtualCenterSettings(context);
        }
        return nineDspDtsFilterSettings;
    }

    private NineDspDtsVirtualCenterSettings(Context context) {
        this.context = context;
    }

    public void nativeDTS(int progress) {
        int[] iArr = {progress};
        NativeHelper.getEq().setVirtualCenter(iArr);
        Log.d(TAG, String.format("nativeDTS data : %s", Arrays.toString(iArr)));
    }

    public void saveDtsFocusFrontLv(int i) {
        Log.d(TAG, String.format("aveDtsFocusFrontLv  param %d", Integer.valueOf(i)));
        spUtils.put(KEY_FOCUS_FRONT_LV, i);
    }

    public void saveDtsFocusRearLv(int i) {
        Log.d(TAG, String.format("saveDtsFocusRearLv  param : %d", Integer.valueOf(i)));
        spUtils.put(KEY_FOCUS_REAR_LV, i);
    }

    public void saveDtsFocusCenterLv(int i) {
        Log.d(TAG, String.format("saveDtsFocusCenterLv  param : %d", Integer.valueOf(i)));
        spUtils.put(KEY_VIRTUAL_CENTER_LV, i);
    }

    public int getDtsFocusFrontLv() {
        return spUtils.getInt(KEY_FOCUS_FRONT_LV, DEF_FOCUS_FRONT_LV);
    }

    public int getDtsFocusRearLv() {
        return spUtils.getInt(KEY_FOCUS_REAR_LV, DEF_FOCUS_REAR_LV);
    }

    public int getDtsFocusCenterLv() {
        return spUtils.getInt(KEY_VIRTUAL_CENTER_LV, DEF_FOCUS_CENTER_LV);
    }

    public void reset() {
//        spUtils.remove(KEY_FOCUS_FRONT_LV);
//        spUtils.remove(KEY_FOCUS_REAR_LV);
        spUtils.remove(KEY_VIRTUAL_CENTER_LV);
        nativeAll();
    }

    public void nativeAll() {
        nativeDTS(NineDspDtsFilterSettings.getInstance(context).getCenterSwitchEnable() ? getDtsFocusCenterLv() : 0);
//        nativeDTS(NINE_DTS_FOCUS_FRONT_LV, getDtsFocusFrontLv());
//        nativeDTS(NINE_DTS_FOCUS_REAR_LV, getDtsFocusRearLv());
    }
}