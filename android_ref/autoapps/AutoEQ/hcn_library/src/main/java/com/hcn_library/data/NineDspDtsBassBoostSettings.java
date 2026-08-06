package com.hcn_library.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn_library.nativeextdsp.NativeHelper;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;


public class NineDspDtsBassBoostSettings implements NineConstantExtDsp {
    private static int DEF_BASS_BASS_LV = 16384;
    private static int DEF_BASS_FRONT_LV = 16384;
    private static int DEF_BASS_REAR_LV = 16384;
    public static final int DEF_FREQ = 0;
    private static final String KEY_BASS_BASS_FREQ = "nine_dsp_dts_bass_freq";
    private static final String KEY_BASS_BASS_LV = "nine_dsp_dts_bass_lv";
    private static final String KEY_BASS_FRONT_FREQ = "nine_dsp_dts_bass_front_freq";
    private static final String KEY_BASS_FRONT_LV = "nine_dsp_dts_bass_front_lv";
    private static final String KEY_BASS_REAR_FREQ = "nine_dsp_dts_bass_rear_freq";
    private static final String KEY_BASS_REAR_LV = "nine_dsp_dts_bass_rear_lv";
    private static final String KEY_INDEX_CMD = "NINE_dsp_dts_index_id";
    public static final int MAX_LV = 32767;
    private static final String NINE_DSP_DTS_FILE = "nine_dsp_switch";
    private Context context;
    public String[] optionsArray = {"40HZ", "60HZ", "100HZ", "150HZ", "200HZ", "250HZ", "300HZ", "400HZ"};
    private SPUtils spUtils = SPUtils.getInstance(NINE_DSP_DTS_FILE);
    private static final String TAG = "NineDspDtsBassBoostSettings";
    private static NineDspDtsBassBoostSettings nineDspDtsFilterSettings = null;


    public static NineDspDtsBassBoostSettings getInstance(Context context) {
        if (nineDspDtsFilterSettings == null) {
            nineDspDtsFilterSettings = new NineDspDtsBassBoostSettings(context);
            if (EqUtils.isChip7739()) {
                DEF_BASS_BASS_LV = 50;
                DEF_BASS_FRONT_LV = 50;
                DEF_BASS_REAR_LV = 50;
            }
        }
        return nineDspDtsFilterSettings;
    }


    private NineDspDtsBassBoostSettings(Context context) {
        this.context = context;
    }


    public void nativeDTS(int index, int value) {
        int[] iArr = {index, value};
        if (EqUtils.isChip7739()) {
            NativeHelper.getEq().setBassBoost(iArr);
        } else {
            NativeHelper.getEq().setEqDts(iArr);
        }
        Log.d(TAG, String.format("nativeDTS data : [%d, %d]", index, value));
    }


    public void saveDtsBassFrontLv(int value) {
        Log.d(TAG, String.format("saveDtsBassFrontLv  param %d", value));
        spUtils.put(KEY_BASS_FRONT_LV, value);
    }


    public void saveDtsBassRearLv(int value) {
        Log.d(TAG, String.format("saveDtsBassRearLv  param : %d", value));
        spUtils.put(KEY_BASS_REAR_LV, value);
    }


    public void saveDtsBassBassLv(int value) {
        Log.d(TAG, String.format("saveDtsBassBassLv  param : %d", value));
        spUtils.put(KEY_BASS_BASS_LV, value);
    }


    public void saveDtsBassFrontFREQ(int value) {
        Log.d(TAG, String.format("saveDtsBassFrontFREQ  param %d", value));
        spUtils.put(KEY_BASS_FRONT_FREQ, value);
    }


    public void saveDtsBassRearFREQ(int value) {
        Log.d(TAG, String.format("saveDtsBassRearFREQ  param : %d", value));
        spUtils.put(KEY_BASS_REAR_FREQ, value);
    }


    public void saveDtsBassBassFREQ(int value) {
        Log.d(TAG, String.format("saveDtsBassBassFREQ  param : %d", value));
        spUtils.put(KEY_BASS_BASS_FREQ, value);
    }


    public int getDtsBassFrontLv() {
        return spUtils.getInt(KEY_BASS_FRONT_LV, DEF_BASS_FRONT_LV);
    }


    public int getDtsBassRearLv() {
        return spUtils.getInt(KEY_BASS_REAR_LV, DEF_BASS_REAR_LV);
    }


    public int getDtsBassBassLv() {
        return spUtils.getInt(KEY_BASS_BASS_LV, DEF_BASS_BASS_LV);
    }


    public int getDtsBassFrontFREQ() {
        return spUtils.getInt(KEY_BASS_FRONT_FREQ, DEF_FREQ);
    }


    public int getDtsBassRearFREQ() {
        return spUtils.getInt(KEY_BASS_REAR_FREQ, DEF_FREQ);
    }


    public int getDtsBassBassFREQ() {
        return spUtils.getInt(KEY_BASS_BASS_FREQ, DEF_FREQ);
    }


    public void reset() {
        spUtils.remove(KEY_BASS_FRONT_LV);
        spUtils.remove(KEY_BASS_REAR_LV);
        spUtils.remove(KEY_BASS_BASS_LV);
        spUtils.remove(KEY_BASS_FRONT_FREQ);
        spUtils.remove(KEY_BASS_REAR_FREQ);
        spUtils.remove(KEY_BASS_BASS_FREQ);
        nativeAll();
    }


    public void nativeAll() {
        if (EqUtils.isChip7739()) {
            boolean isEnable = NineDspDtsFilterSettings.getInstance(context).getBassBoostSwitchEnable();
            nativeDTS(isEnable ? getDtsBassBassLv() : 0, isEnable ? getDtsBassBassFREQ() : 0);
        } else {
            nativeDTS(NINE_DTS_BASS_FRONT_LV, getDtsBassFrontLv());
            nativeDTS(NINE_DTS_BASS_SUB_LV, getDtsBassBassLv());
            nativeDTS(NINE_DTS_BASS_REAR_LV, getDtsBassRearLv());
            nativeDTS(NINE_DTS_BASS_FRONT_FREQ, getDtsBassFrontFREQ());
            nativeDTS(NINE_DTS_BASS_SUB_FREQ, getDtsBassBassFREQ());
            nativeDTS(NINE_DTS_BASS_REAR_FREQ, getDtsBassRearFREQ());
        }
    }
}