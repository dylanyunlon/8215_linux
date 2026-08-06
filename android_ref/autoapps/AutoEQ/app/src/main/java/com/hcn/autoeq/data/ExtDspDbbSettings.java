package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantExtDsp;

import java.util.Arrays;

public class ExtDspDbbSettings implements ConstantExtDsp {

    private static final String TAG = ExtDspDbbSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(ExtDspDbbSettings.class.getSimpleName(), Log.DEBUG);

    private static final String EXT_DSP_DBB_FILE = "ext_dsp_dbb"; // dbb 保存的文件名
    private static final String KEY_CHANNEL = "ext_dsp_dbb_channel";
    private static final String KEY_TOUCH_X = "ext_dsp_dbb_touch_x_";
    private static final String KEY_TOUCH_Y = "ext_dsp_dbb_touch_y_";
    private static final String KEY_FREQ = "ext_dsp_dbb_freq_";
    private static final String KEY_GAIN = "ext_dsp_dbb_gain_";

    private Context context;
    private static ExtDspDbbSettings extDspDbbSettings = null;
    private SPUtils spUtils;

    public static ExtDspDbbSettings getInstance(Context mContext) {
        if (null == extDspDbbSettings) {
            extDspDbbSettings = new ExtDspDbbSettings(mContext);
        }
        return extDspDbbSettings;
    }

    private ExtDspDbbSettings(Context context) {
        this.context = context;
        spUtils = SPUtils.getInstance(EXT_DSP_DBB_FILE);
    }

    public void nativeDbb(int channel, int freq, int gain) {
        int[] data = new int[]{channel, freq, gain, 0};
        NativeHelper.getEq().setEqDbb(data);
        Log.d(TAG, String.format("nativeDbb data : %s", Arrays.toString(data)));
    }

    public void saveDbb(int channel, float touchX, float touchY, int freq, int gain) {
        Log.d(TAG, String.format("saveDbb channel : %d, touchX : %f, touchY : %f, freq : %d, gain : %d", channel, touchX, touchY, freq, gain));
        spUtils.put(KEY_CHANNEL, channel);
        spUtils.put(KEY_TOUCH_X + channel, touchX);
        spUtils.put(KEY_TOUCH_Y + channel, touchY);
        spUtils.put(KEY_FREQ + channel, freq);
        spUtils.put(KEY_GAIN + channel, gain, true);
    }

    public int getDbbChannel() {
        return spUtils.getInt(KEY_CHANNEL, DEF_DBB_CHANNEL_FLFR);
    }

    public float getDbbTouchX(int channel, float defaultValue) {
        return spUtils.getFloat(KEY_TOUCH_X + channel, defaultValue);
    }

    public float getDbbTouchY(int channel, float defaultValue) {
        return spUtils.getFloat(KEY_TOUCH_Y + channel, defaultValue);
    }

    public int getDbbFreq(int channel) {
        return spUtils.getInt(KEY_FREQ + channel, 190);
    }

    public int getDbbGain(int channel) {
        return spUtils.getInt(KEY_GAIN + channel, 0);
    }

    public void nativeAll(int... channels) {
        for (int channel : channels) {
            nativeDbb(channel, getDbbFreq(channel), getDbbGain(channel));
        }
    }

}
