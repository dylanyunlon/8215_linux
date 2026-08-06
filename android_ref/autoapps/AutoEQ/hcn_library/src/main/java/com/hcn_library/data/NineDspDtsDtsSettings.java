package com.hcn_library.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn_library.nativeextdsp.NativeHelper;
import com.hcn_library.util.ConstantExtDsp;
import com.hcn_library.util.EqUtils;

import java.util.Arrays;

public class NineDspDtsDtsSettings implements ConstantExtDsp {
    private static final String KEY_DTS_MODEL = "NINE_dsp_dts_model";
    private static final String KEY_INDEX_CMD = "NINE_dsp_dts_index_id";
    private static final String NINE_DSP_DTS_FILE = "nine_dsp_switch";
    private Context context;
    private SPUtils spUtils = SPUtils.getInstance(NINE_DSP_DTS_FILE);
    private static final String TAG = "NineDspDtsDtsSettings";

    private static NineDspDtsDtsSettings nineDspDtsDtsSettings = null;

    public static NineDspDtsDtsSettings getInstance(Context context) {
        if (nineDspDtsDtsSettings == null) {
            nineDspDtsDtsSettings = new NineDspDtsDtsSettings(context);
            if (EqUtils.isChip7739()) {
                theater_mode = 35;
                music_mode = 10;
                professional_mode = 50;
            }
        }
        return nineDspDtsDtsSettings;
    }

    private NineDspDtsDtsSettings(Context context) {
        this.context = context;
    }

    public void nativeDTS(int i, int i2) {
        int[] iArr = {i, i2};
        NativeHelper.getEq().setEqDts(iArr);
        Log.d(TAG, String.format("nativeDTS data : %s", Arrays.toString(iArr)));
    }

    // 模式：1-剧院 4-音乐 8-专业
    public void saveDtsModel(int i) {
        Log.d(TAG, String.format("saveDtsModel param : %d", Integer.valueOf(i)));
        spUtils.put(KEY_DTS_MODEL, i);
    }

    public static int theater_mode = 1;
    public static int music_mode = 4;
    public static int professional_mode = 8;
    public int getDtsModel() {
        return spUtils.getInt(KEY_DTS_MODEL, music_mode);
    }
}