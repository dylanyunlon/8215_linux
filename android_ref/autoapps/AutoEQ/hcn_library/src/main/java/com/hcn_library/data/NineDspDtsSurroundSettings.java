package com.hcn_library.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn_library.nativeextdsp.NativeHelper;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;

import java.util.Arrays;

public class NineDspDtsSurroundSettings implements NineConstantExtDsp {
    private static int DEF_SURROUND_CENTER_REAR_LV = 16384;
    private static int DEF_SURROUND_FRONT_REAR_LV = 16384;
    private static final String KEY_SURROUND_CENTER_REAR_LV = "nine_dsp_dts_surround_center_rear_lv";
    private static final String KEY_SURROUND_FRONT_REAR_LV = "nine_dsp_dts_surround__front_rear_lv";
    public static final int MAX_LV = 32767;
    private static final String NINE_DSP_DTS_FILE = "nine_dsp_switch";
    private Context context;
    private SPUtils spUtils = SPUtils.getInstance(NINE_DSP_DTS_FILE);
    private static final String TAG = "NineDspDtsSurroundSettings";
    private static NineDspDtsSurroundSettings nineDspDtsFilterSettings = null;

    public static NineDspDtsSurroundSettings getInstance(Context context) {
        if (nineDspDtsFilterSettings == null) {
            nineDspDtsFilterSettings = new NineDspDtsSurroundSettings(context);
            if(EqUtils.isChip7739()){
                DEF_SURROUND_CENTER_REAR_LV = 50;
                DEF_SURROUND_FRONT_REAR_LV = 50;
            }
        }
        return nineDspDtsFilterSettings;
    }

    private NineDspDtsSurroundSettings(Context context) {
        this.context = context;
    }

    public void nativeDTS(int i, int i2) {
        int[] iArr = {i, i2};
        if (EqUtils.isChip7739()) {
            NativeHelper.getEq().setSoundSurround(iArr);
        } else {
            NativeHelper.getEq().setEqDts(iArr);
        }
        Log.d(TAG, String.format("nativeDTS data : %s", Arrays.toString(iArr)));
    }

    public void saveDtsSurroundFrontFrontLv(int i) {
        Log.d(TAG, String.format("saveDtsSurroundFrontFrontLv  param %d", Integer.valueOf(i)));
        spUtils.put(KEY_SURROUND_FRONT_REAR_LV, i);
    }

    public void saveDtsSurroundCenterRearLv(int i) {
        Log.d(TAG, String.format("saveDtsSurroundCenterRearLv  param : %d", Integer.valueOf(i)));
        spUtils.put(KEY_SURROUND_CENTER_REAR_LV, i);
    }

    public int getDtsSurroundFrontFrontLv() {
        return spUtils.getInt(KEY_SURROUND_FRONT_REAR_LV, DEF_SURROUND_FRONT_REAR_LV);
    }

    public int getDtsSurroundCenterRearLv() {
        return spUtils.getInt(KEY_SURROUND_CENTER_REAR_LV, DEF_SURROUND_CENTER_REAR_LV);
    }

    public void reset() {
        spUtils.remove(KEY_SURROUND_FRONT_REAR_LV);
        spUtils.remove(KEY_SURROUND_CENTER_REAR_LV);
        nativeAll();
    }

    public void nativeAll() {
        if (EqUtils.isChip7739()) {
            nativeDTS(NINE_DTS_FOCUS_FRONT_REAR_LV, NineDspDtsFilterSettings.getInstance(context).getSurroundSwitchEnable() ? getDtsSurroundCenterRearLv() : 0);
        } else {
            nativeDTS(NINE_DTS_FOCUS_FRONT_REAR_LV, getDtsSurroundCenterRearLv());
            nativeDTS(NINE_DTS_FOCUS_CENTER_REAR_LV, getDtsSurroundCenterRearLv());
        }
    }
}