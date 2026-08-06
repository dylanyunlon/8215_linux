package com.hcn.autoeq.nativeextdsp;

import android.audio.AudioEffect;
import android.util.Log;

import java.util.Arrays;

public class FY7604 implements IEq {
    private static final String TAG = FY7604.class.getSimpleName();

    static final int STATUS_WRITE_CMD_EXE_SUB_ID = 0x89;

    public static final int FY_CMD_SUB_ID_SET_PEQ_SINGLE = 0x01;
    public static final int FY_CMD_SUB_ID_SET_PEQ_32ALL = 0x02;
    public static final int FY_CMD_SUB_ID_SET_EQ_EFECT = 0x03;
    public static final int FY_CMD_SUB_ID_ATTENUATE_INVERT_SPEAKER = 0x04;
    public static final int FY_CMD_SUB_ID_BASS = 0x05;
    public static final int FY_CMD_SUB_ID_LPF_HPF = 0x06;
    public static final int FY_CMD_SUB_ID_BALANCE = 0x07;
    public static final int FY_CMD_SUB_ID_DELAY = 0x08;
    public static final int FY_CMD_SUB_ID_LOUDNESS = 0x09;

    public int setEqBand(int[] data) {
        Log.d(TAG, String.format("setEqBand data : %s", Arrays.toString(data)));
        return AudioEffect.getInstance().doExtAudioEffect(STATUS_WRITE_CMD_EXE_SUB_ID, data);
    }

    public int setEqBands(int[] data) {
        Log.d(TAG, String.format("setEqBands data : %s", Arrays.toString(data)));
        return AudioEffect.getInstance().doExtAudioEffect(STATUS_WRITE_CMD_EXE_SUB_ID, data);
    }

    public int setEqBalance(int[] data) {
        Log.d(TAG, String.format("setEqBalance data : %s", Arrays.toString(data)));
        return AudioEffect.getInstance().doExtAudioEffect(STATUS_WRITE_CMD_EXE_SUB_ID, data);
    }

    public int setEqAttSpeaker(int[] data) {
        Log.d(TAG, String.format("setEqAttSpeaker data : %s", Arrays.toString(data)));
        return AudioEffect.getInstance().doExtAudioEffect(STATUS_WRITE_CMD_EXE_SUB_ID, data);
    }

    public int setEqHpfLpf(int[] data) {
        Log.d(TAG, String.format("setEqHpfLpf data : %s", Arrays.toString(data)));
        return AudioEffect.getInstance().doExtAudioEffect(STATUS_WRITE_CMD_EXE_SUB_ID, data);
    }

    public int setEqDbb(int[] _data) {
        return -1;
    }

    public int setEqSurround(int[] _data) {
        return -1;
    }

    public int setEqLoudness(int[] data) {
        Log.d(TAG, String.format("setEqLoudness data : %s", Arrays.toString(data)));
        return AudioEffect.getInstance().doExtAudioEffect(STATUS_WRITE_CMD_EXE_SUB_ID, data);
    }

    public int setEqSpeakerDelay(int[] data) {
        Log.d(TAG, String.format("setEqSpeakerDelay data : %s", Arrays.toString(data)));
        return AudioEffect.getInstance().doExtAudioEffect(STATUS_WRITE_CMD_EXE_SUB_ID, data);
    }

    public int setEqBass(int[] data) {
        Log.d(TAG, String.format("setEqBass data : %s", Arrays.toString(data)));
        return AudioEffect.getInstance().doExtAudioEffect(STATUS_WRITE_CMD_EXE_SUB_ID, data);
    }

}
