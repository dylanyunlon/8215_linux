package com.hcn_library.data;

import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn_library.nativeextdsp.NativeHelper;
import com.hcn_library.util.NineConstantExtDsp;


public class NineDspDbbSettings implements NineConstantExtDsp {
    private static final String KEY_CHANNEL = "nine_dsp_dbb_channel";
    private static final String KEY_FREQ = "nine_dsp_dbb_freq_";
    private static final String KEY_GAIN = "nine_dsp_dbb_gain_";
    private static final String KEY_TOUCH_X = "nine_dsp_dbb_touch_x_";
    private static final String KEY_TOUCH_Y = "nine_dsp_dbb_touch_y_";
    private static final String NINE_DSP_DBB_FILE = "nine_dsp_dbb";
    private Context context;
    private SPUtils spUtils = SPUtils.getInstance(NINE_DSP_DBB_FILE);
    private static final String TAG = "NineDspDbbSettings";
    private static NineDspDbbSettings nineDspDbbSettings = null;


    public static NineDspDbbSettings getInstance(Context context) {
        if (nineDspDbbSettings == null) {
            nineDspDbbSettings = new NineDspDbbSettings(context);
        }
        return nineDspDbbSettings;
    }


    private NineDspDbbSettings(Context context) {
        this.context = context;
    }


    public void nativeDbb(int channel, int freq, int gain) {
        NativeHelper.getEq().setEqDbb(new int[]{channel, freq, gain, 0});
        Log.d(TAG, String.format("nativeDbb data : [%d, %d, %d, 0]", channel, freq, gain));
    }


    public void saveDbb(int channel, float touchX, float touchY, int freq, int gain) {
        Log.d(TAG, String.format("saveDbb channel : %d, touchX : %f, touchY : %f, freq : %d, gain : %d", channel, touchX, touchY, freq, gain));
        spUtils.put(KEY_CHANNEL, channel);
        spUtils.put(KEY_TOUCH_X + channel, touchX);
        spUtils.put(KEY_TOUCH_Y + channel, touchY);
        spUtils.put(KEY_FREQ + channel, freq);
        spUtils.put(KEY_GAIN + channel, gain);
    }


    public int getDbbChannel() {
        int channel = spUtils.getInt(KEY_CHANNEL, NINE_DEF_DBB_CHANNEL_FLFR);
        Log.d(TAG, "KEY_CHANEL = " + channel);
        return channel;
    }


    public float getDbbTouchX(int channel, float defaultTouchX) {
        return spUtils.getFloat(KEY_TOUCH_X + channel, defaultTouchX);
    }


    public float getDbbTouchY(int channel, float defaultTouchY) {
        return spUtils.getFloat(KEY_TOUCH_Y + channel, defaultTouchY);
    }


    public int getDbbFreq(int channel) {
        return spUtils.getInt(KEY_FREQ + channel, 210);
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