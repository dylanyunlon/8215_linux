package com.hcn_library.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn_library.nativeextdsp.NativeHelper;
import com.hcn_library.util.NineConstantExtDsp;

import java.util.Arrays;

public class NineDspDtsFilterSettings implements NineConstantExtDsp {
    private static final String KEY_CENTER_SWITCH = "nine_dsp_dts_center_switch";
    private static final String KEY_DTS_SWITCH = "nine_dsp_dts_switch";
    private static final String KEY_INDEX_CMD = "NINE_dsp_dts_index_id";
    private static final String NINE_DSP_DTS_FILE = "nine_dsp_switch";
    private Context context;
    private SPUtils spUtils = SPUtils.getInstance(NINE_DSP_DTS_FILE);
    private static final String TAG = "NineDspDtsFilterSettings";
    private static NineDspDtsFilterSettings nineDspDtsFilterSettings = null;

    private static final String KEY_SOUND_FOCUS = "nine_dsp_dts_sound_focus";
    private static final String KEY_SURROUND = "nine_dsp_dts_surround";
    private static final String KEY_BASS_BOOST = "nine_dsp_dts_bass_boost";


    public static NineDspDtsFilterSettings getInstance(Context context) {
        if (nineDspDtsFilterSettings == null) {
            nineDspDtsFilterSettings = new NineDspDtsFilterSettings(context);
        }
        return nineDspDtsFilterSettings;
    }

    private NineDspDtsFilterSettings(Context context) {
        this.context = context;
    }

    public void nativeDTS(int i, int i2) {
        int[] iArr = {i, i2};
        NativeHelper.getEq().setEqDts(iArr);
        Log.d(TAG, String.format("nativeDTS data : %s", Arrays.toString(iArr)));
    }

    public void saveDtsSwitch(int i) {
        Log.d(TAG, String.format("saveDtsSwitch param : %d", Integer.valueOf(i)));
        spUtils.put(KEY_DTS_SWITCH, i);
    }

    public void saveCenterSwitch(int i) {
        Log.d(TAG, String.format("saveCenterSwitch  param : %d", Integer.valueOf(i)));
        spUtils.put(KEY_CENTER_SWITCH, i);
    }

    public int getDtsSwitch() {
        Log.d(TAG, "getDtsSwitch: " + spUtils.getInt(KEY_DTS_SWITCH, 0)); //默认开，0开，1关
        return spUtils.getInt(KEY_DTS_SWITCH, 0);
    }

    public boolean getDtsSwitchEnable() {
        return getDtsSwitch() == 0;
    }

    public int getCenterSwitch() {
        return spUtils.getInt(KEY_CENTER_SWITCH, 0);
    } //默认关，0关，1开

    public boolean getCenterSwitchEnable() {
        return getCenterSwitch() == 1;
    }

    public void saveSoundFocusSwitch(int i) {//0:enable 1:disable
        Log.d(TAG, String.format("saveSoundFocusSwitch  param : %d", Integer.valueOf(i)));
        spUtils.put(KEY_SOUND_FOCUS, i);
    }

    public boolean getSoundFocusSwitchEnable() {
        return spUtils.getInt(KEY_SOUND_FOCUS, 0) == 0;
    }

    public void saveSurroundSwitch(int i) {//0:enable 1:disable
        Log.d(TAG, String.format("saveSurroundSwitch  param : %d", Integer.valueOf(i)));
        spUtils.put(KEY_SURROUND, i);
    }

    public boolean getSurroundSwitchEnable() {
        return spUtils.getInt(KEY_SURROUND, 0) == 0;
    }

    public void saveBassBoostSwitch(int i) {//0:enable 1:disable
        Log.d(TAG, String.format("saveBassBoostSwitch  param : %d", Integer.valueOf(i)));
        spUtils.put(KEY_BASS_BOOST, i);
    }

    public boolean getBassBoostSwitchEnable() {
        return spUtils.getInt(KEY_BASS_BOOST, 0) == 0;
    }

    public void nativeAll() {
        nativeDTS(NINE_DTS_BYPASS, getDtsSwitch());
        nativeDTS(NINE_DTS_PHANTOM_CENTER_ENABLE, getCenterSwitch());
    }
}