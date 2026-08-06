package com.hcn.autoeq.nativeextdsp;

import android.audio.AudioEffect;
import android.util.Log;

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.Arrays;

public class CSC37534 implements IEq {

    private static final String TAG = CSC37534.class.getSimpleName();

    static final int CSC_37534_ID = 0x8A;
    static final int CSC_CMD_SUB_ID_SET_PEQ = 0x01;
    static final int CSC_CMD_SUB_ID_LOUDNESS = 0x02;
    static final int CSC_CMD_SUB_ID_BALANCE = 0x03;
    static final int CSC_CMD_SUB_ID_LPF_SUBWOOFER = 0x04;


    /**
     * @param _data
     * @return
     */
    public int setEqBand(int[] _data) {
        int[] data = new int[10];
        int cmd = CSC_37534_ID;
        data[0] = CSC_CMD_SUB_ID_SET_PEQ;
        data[1] = _data[0];
        data[2] = _data[1];
        data[3] = _data[2];
        data[4] = _data[3];
        data[5] = _data[4];
        data[6] = _data[5];
        data[7] = _data[6];
        data[8] = _data[7];
        data[9] = _data[8];
        Log.d(TAG, String.format("setEqBand data : %s", Arrays.toString(data)));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    public int setEqLoudness(int[] _data) {
        int[] data = new int[2];
        int cmd = CSC_37534_ID;
        data[0] = CSC_CMD_SUB_ID_LOUDNESS;
        data[1] = _data[0];
        Log.d(TAG, String.format("setEqLoudness data : %s", Arrays.toString(data)));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }


    public int setEqBalance(int[] _data) {
        int fl = _data[0];
        int fr = _data[1];
        int rl = _data[2];
        int rr = _data[3];

        int[] data = new int[5];
        int cmd = CSC_37534_ID;
        data[0] = CSC_CMD_SUB_ID_BALANCE;
        data[1] = fl;
        data[2] = fr;
        data[3] = rl;
        data[4] = rr;
        Log.d(TAG, String.format("setEqBalance data : %s", Arrays.toString(data)));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }


    public int setEqSurround(int[] _data) {
        int[] data = new int[3];
        int cmd = CSC_37534_ID;
        data[0] = CSC_CMD_SUB_ID_LPF_SUBWOOFER;
        data[1] = _data[0];
        data[2] = _data[1];
        Log.d(TAG, String.format("setEqSurround data : %s", Arrays.toString(data)));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

}
