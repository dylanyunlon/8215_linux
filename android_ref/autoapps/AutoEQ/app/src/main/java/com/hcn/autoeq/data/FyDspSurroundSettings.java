package com.hcn.autoeq.data;

import android.content.Context;
import android.util.Log;

import com.hcn.autoeq.nativeextdsp.FY7604;
import com.hcn.autoeq.nativeextdsp.NativeHelper;
import com.hcn.autoeq.util.ConstantFyDsp;

import java.util.Arrays;

public class FyDspSurroundSettings extends FyDspBaseSettings implements ConstantFyDsp {

    private static final String TAG = FyDspSurroundSettings.class.getSimpleName();
    private static final boolean DEBUG = Log.isLoggable(FyDspSurroundSettings.class.getSimpleName(), Log.DEBUG);

    private static final String FY_DSP_SURROUND_FILE = "v3_fy_dsp_surround"; // 环绕保存的文件名
    private static final String KEY_LOUDNESS = "fy_dsp_loudness";
    private static final String KEY_BASS_BOOST_GAIN_CH12 = "fy_dsp_bass_boost_gain_ch12";
    private static final String KEY_BASS_BOOST_FREQ_CH12 = "fy_dsp_bass_boost_freq_ch12";
    private static final String KEY_BASS_BOOST_GAIN_CH34 = "fy_dsp_bass_boost_gain_ch34";
    private static final String KEY_BASS_BOOST_FREQ_CH34 = "fy_dsp_bass_boost_freq_ch34";

    private Context context;
    private static FyDspSurroundSettings extDspSurroundSettings = null;

    public static FyDspSurroundSettings getInstance(Context context) {
        if (null == extDspSurroundSettings) {
            extDspSurroundSettings = new FyDspSurroundSettings(context);
        }
        return extDspSurroundSettings;
    }

    private FyDspSurroundSettings(Context context) {
        super(FY_DSP_SURROUND_FILE);
        this.context = context;
    }

    public void nativeLoudness(int loudness) {
        int[] data = new int[]{FY7604.FY_CMD_SUB_ID_LOUDNESS, loudness};
        Log.d(TAG, String.format("nativeLoudness data : %s", Arrays.toString(data)));
        NativeHelper.getEq().setEqLoudness(data);
    }

    public void saveLoudness(int loudness) {
        Log.d(TAG, String.format("saveLoudness loudness : %d", loudness));
        spUtils.put(KEY_LOUDNESS, loudness, true);
    }

    public int getLoudness() {
        return getSpUtils().getInt(KEY_LOUDNESS, DEF_LOUDNESS_DISABLE);
    }

    public void saveBassBoostGainCh12(int gain) {
        Log.d(TAG, String.format("saveBassBoostGainCh12 gain : %d", gain));
        spUtils.put(KEY_BASS_BOOST_GAIN_CH12, gain, true);
    }

    public int getBassBoostGainCh12() {
        return getSpUtils().getInt(KEY_BASS_BOOST_GAIN_CH12, 0);
    }

    public void saveBassBoostFreqCh12(int freq) {
        Log.d(TAG, String.format("saveBassBoostFreqCh12 freq : %d", freq));
        spUtils.put(KEY_BASS_BOOST_FREQ_CH12, freq, true);
    }

    public int getBassBoostFreqCh12() {
        return getSpUtils().getInt(KEY_BASS_BOOST_FREQ_CH12, DEF_BASS_BOOST_FREQ);
    }

    public void saveBassBoostGainCh34(int gain) {
        Log.d(TAG, String.format("saveBassBoostGainCh34 gain : %d", gain));
        spUtils.put(KEY_BASS_BOOST_GAIN_CH34, gain, true);
    }

    public int getBassBoostGainCh34() {
        return getSpUtils().getInt(KEY_BASS_BOOST_GAIN_CH34, 0);
    }

    public void saveBassBoostFreqCh34(int freq) {
        Log.d(TAG, String.format("saveBassBoostFreqCh34 freq : %d", freq));
        spUtils.put(KEY_BASS_BOOST_FREQ_CH34, freq, true);
    }

    public int getBassBoostFreqCh34() {
        return getSpUtils().getInt(KEY_BASS_BOOST_FREQ_CH34, DEF_BASS_BOOST_FREQ);
    }

    public void reset() {
        saveBassBoostFreqCh12(DEF_BASS_BOOST_FREQ);
        saveBassBoostGainCh12(DEF_LOUDNESS_DISABLE);
        saveBassBoostFreqCh34(DEF_BASS_BOOST_FREQ);
        saveBassBoostGainCh34(DEF_LOUDNESS_DISABLE);
        saveLoudness(DEF_LOUDNESS_DISABLE);
    }

    public void nativeBassBoost(int freqCh12, int gainCh12, int freqCh34, int gainCh34) {
        Log.d(TAG, String.format("nativeBassBoost freq ch12: %d, gain ch12: %d,  freq ch34: %d, gain ch34: %d", freqCh12, gainCh12, freqCh34, gainCh34));
        int[] data = {FY7604.FY_CMD_SUB_ID_BASS, freqCh12, gainCh12 * 10, freqCh34, gainCh34 * 10};
        NativeHelper.getEq().setEqBass(data);
    }

}
