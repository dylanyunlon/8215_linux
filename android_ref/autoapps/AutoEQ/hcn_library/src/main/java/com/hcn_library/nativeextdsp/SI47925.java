package com.hcn_library.nativeextdsp;


import android.audio.AudioEffect;
import android.util.Log;

import com.blankj.utilcode.util.ThreadUtils;
import com.hcn_library.bean.SIDspBiQuadParams;
import com.hcn_library.bean.SIDspBiQuadParamsExt;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.SI47925EqDealDataUtil;
import com.hcn_library.util.SystemUtils;

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.Arrays;
import java.util.List;

public class SI47925 implements IEq {
    private static final String TAG = SI47925.class.getSimpleName();

    static final int SI47925_ID = 0X8B;
    static final int SI47925_CMD_SUB_ID_EQ_BAND = 0x01;
    static final int SI47925_CMD_SUB_ID_BALANCE = 0x02;
    static final int SI47925_CMD_SUB_ID_ATTENUATE_INVERT_SPEAKER = 0x03;
    static final int SI47925_CMD_SUB_ID_HPF_LPF = 0x04;
    static final int SI47925_CMD_SUB_ID_DBB = 0x05;
    static final int SI47925_CMD_SUB_ID_SURROUND = 0x06;
    static final int SI47925_CMD_SUB_ID_LOUDNESS = 0x07;
    static final int SI47925_CMD_SUB_ID_DELAY = 0x08;
    static final  int SI47925_CMD_SUB_ID_HPF_LPF_V2 = 0x09;
    static final  int SI47925_CMD_SUB_ID_DTS = 0x0A;


    public static final int SI47925_INDEX_CHANEL_FL = 0x00;    //前左、前右
    public static final int SI47925_INDEX_CHANEL_FR = 0x01;        //后左、后右
    public static final int SI47925_INDEX_CHANEL_RL = 0x02;        //中置
    public static final int SI47925_INDEX_CHANEL_RR = 0x03;        //中置

    public static final int SI47925_INDEX_CHANEL_CEN = 0x04;
    public static final int SI47925_INDEX_CHANEL_SUB = 0x05;

    static double FS = 48000;

    public static final String TASK_EXECUTION_FINISH = "task_execution_finish";

    public int setEqBand(int[] _data) {
        int band = _data[0];
        int freq = _data[1];
        int gain = _data[2];
        int qvalue = _data[3];
        Log.d(TAG, String.format("0peakingCoef setEqBand data, band:[%s], freq:[%s] ,gain:[%s], qvalue:[%s]", band, freq, gain, qvalue));
        SIDspBiQuadParams eqBiQuad = SI47925EqDealDataUtil.peakingCoef(
                SI47925EqDealDataUtil.FILTER_TYPE.FILTER_PEAK_EQ
                , gain, SI47925EqDealDataUtil.FILTER_PARAM_TYPE.FILTER_PARAM_Q,
                new BigDecimal(qvalue / 1000d).setScale(1, RoundingMode.HALF_UP).doubleValue()
                , freq);

        int[] data = new int[8];
        data[0] = SI47925_CMD_SUB_ID_EQ_BAND;
        data[1] = band;
        if (gain == 0) {
            data[2] = 0x000000;
            data[3] = 0x000000;
            data[4] = 0x400000;
            data[5] = 0x000000;
            data[6] = 0x000000;
            data[7] = 0x00;
        } else {
            data[2] = eqBiQuad.getA1();
            data[3] = eqBiQuad.getA2();
            data[4] = eqBiQuad.getB0();
            data[5] = eqBiQuad.getB1();
            data[6] = eqBiQuad.getB2();
            data[7] = eqBiQuad.getbShift();
        }
        Log.d(TAG, "setEqBand: " + SystemUtils.translateTo16(data));
        ThreadUtils.executeBySingle(new ThreadUtils.Task<Object>() {
            @Override
            public Object doInBackground() throws Throwable {
                AudioEffect.getInstance().doExtAudioEffect(SI47925_ID, data);
                return null;
            }

            @Override
            public void onSuccess(Object result) {

            }

            @Override
            public void onCancel() {

            }

            @Override
            public void onFail(Throwable t) {

            }
        });
        return 1;
    }


    public int setEqBalance(int[] _data) {
        int fl = _data[0];
        int fr = _data[1];
        int rl = _data[2];
        int rr = _data[3];
        int cen = _data[4];
        int sub = _data[5];

        int[] data = new int[7];
        data[0] = SI47925_CMD_SUB_ID_BALANCE;
        data[1] = fl;
        data[2] = fr;
        data[3] = rl;
        data[4] = rr;
        data[5] = cen;
        data[6] = sub;
        Log.d(TAG, String.format("setEqBalance data : %s", Arrays.toString(data)));

        ThreadUtils.executeBySingle(new ThreadUtils.Task<Object>() {
            @Override
            public Object doInBackground() throws Throwable {
                AudioEffect.getInstance().doExtAudioEffect(SI47925_ID, data);
                return null;
            }

            @Override
            public void onSuccess(Object result) {

            }

            @Override
            public void onCancel() {

            }

            @Override
            public void onFail(Throwable t) {

            }
        });
        return 1;
    }


    public int setEqAttSpeaker(int[] _data) {
        if (EqUtils.DSP_CHIP_SI47925_DTS.equals(EqUtils.getEqChipType())) {
            return setEqAttSpeakerSIDts(_data);
        }
        if (EqUtils.DSP_CHIP_SI47925.equals(EqUtils.getEqChipType())) {
            return setEqAttSpeakerSI(_data);
        }
        int[] data = new int[5];
        int channel = _data[0];
        int att = _data[1];
        int mute = _data[2];

        data[0] = SI47925_CMD_SUB_ID_ATTENUATE_INVERT_SPEAKER;
        data[1] = channel;
        data[2] = att;
        data[3] = mute;
        data[4] = 0;
        Log.d(TAG, String.format("setEqAttSpeaker data : %s", Arrays.toString(data)));
        ThreadUtils.executeBySingle(new ThreadUtils.Task<Object>() {
            @Override
            public Object doInBackground() throws Throwable {
                AudioEffect.getInstance().doExtAudioEffect(SI47925_ID, data);
                return null;
            }

            @Override
            public void onSuccess(Object result) {

            }

            @Override
            public void onCancel() {

            }

            @Override
            public void onFail(Throwable t) {

            }
        });
        return 1;
    }

    public int setEqAttSpeakerSIDts(int[] _data) {
        int[] data = new int[5];
        int channel = _data[0];
        int att = _data[1];
        int mute = _data[2];
        int invert = 0;
        data[0] = SI47925_CMD_SUB_ID_ATTENUATE_INVERT_SPEAKER;
        data[1] = channel;
        data[2] = att;
        data[3] = mute;
        data[4] = invert;
        Log.d(TAG, String.format("setEqAttSpeakerSIDts : %s", Arrays.toString(data)));
        ThreadUtils.executeBySingle(new ThreadUtils.Task<Object>() {
            @Override
            public Object doInBackground() throws Throwable {
                AudioEffect.getInstance().doExtAudioEffect(SI47925_ID, data);
                return null;
            }

            @Override
            public void onSuccess(Object result) {

            }

            @Override
            public void onCancel() {

            }

            @Override
            public void onFail(Throwable t) {

            }
        });
        return 1;
    }
    public int setEqAttSpeakerSI(int[] _data) {
        int[] data = new int[5];
        int channel = _data[0];
        int att = _data[1];
        int mute = _data[2];
        int invert = data[3];
        data[0] = SI47925_CMD_SUB_ID_ATTENUATE_INVERT_SPEAKER;
        data[1] = channel;
        data[2] = att;
        data[3] = mute;
        data[4] = invert;
        Log.d(TAG, String.format("setEqAttSpeakerSI data : %s", Arrays.toString(data)));
        ThreadUtils.executeBySingle(new ThreadUtils.Task<Object>() {
            @Override
            public Object doInBackground() throws Throwable {
                AudioEffect.getInstance().doExtAudioEffect(SI47925_ID, data);
                return null;
            }

            @Override
            public void onSuccess(Object result) {

            }

            @Override
            public void onCancel() {

            }

            @Override
            public void onFail(Throwable t) {

            }
        });
        return 1;
    }

    private void translateArray(int[] _data) {
        int lPfSlope;
        int lPfFreq;
        int hPfSlope;
        int hPfFreq;
        lPfSlope = _data[1];
        lPfFreq = _data[2];
        hPfSlope = _data[3];
        hPfFreq = _data[4];

        switch (lPfSlope) {
            case 0:
                lPfSlope = 0;
                break;
            case 1:
                lPfSlope = 1;
                break;
            case 2:
                lPfSlope = 3;
                break;
            case 3:
                lPfSlope = 5;
                break;
            case 4:
                lPfSlope = 7;
                break;
            case 5:
                lPfSlope = 9;
                break;
            case 6:
                lPfSlope = 11;
                break;
        }

        switch (hPfSlope) {
            case 0:
                hPfSlope = 0;
                break;
            case 1:
                hPfSlope = 2;
                break;
            case 2:
                hPfSlope = 4;
                break;
            case 3:
                hPfSlope = 6;
                break;
            case 4:
                hPfSlope = 8;
                break;
            case 5:
                hPfSlope = 10;
                break;
            case 6:
                hPfSlope = 12;
                break;
        }

        int[] data = new int[3];
        if (_data[0] == 0) {
            data[0] = 0;
            data[1] = lPfSlope;
            data[2] = lPfFreq;
            setEqHpfLpfSI(data);

            data[0] = 6;
            data[1] = hPfSlope;
            data[2] = hPfFreq;
            setEqHpfLpfSI(data);

            data[0] = 1;
            data[1] = lPfSlope;
            data[2] = lPfFreq;
            setEqHpfLpfSI(data);

            data[0] = 7;
            data[1] = hPfSlope;
            data[2] = hPfFreq;
            setEqHpfLpfSI(data);
        } else if (_data[0] == 1) {
            data[0] = 2;
            data[1] = lPfSlope;
            data[2] = lPfFreq;
            setEqHpfLpfSI(data);

            data[0] = 8;
            data[1] = hPfSlope;
            data[2] = hPfFreq;
            setEqHpfLpfSI(data);

            data[0] = 3;
            data[1] = lPfSlope;
            data[2] = lPfFreq;
            setEqHpfLpfSI(data);

            data[0] = 9;
            data[1] = hPfSlope;
            data[2] = hPfFreq;
            setEqHpfLpfSI(data);
        } else if (_data[0] == 3) {
            data[0] = 5;
            data[1] = lPfSlope;
            data[2] = lPfFreq;
            setEqHpfLpfSI(data);

            data[0] = 11;
            data[1] = hPfSlope;
            data[2] = hPfFreq;
            setEqHpfLpfSI(data);
        } else if (_data[0] == 2) {
            data[0] = 4;
            data[1] = lPfSlope;
            data[2] = lPfFreq;
            setEqHpfLpfSI(data);

            data[0] = 10;
            data[1] = hPfSlope;
            data[2] = hPfFreq;
            setEqHpfLpfSI(data);
        }
    }

    public int setEqHpfLpf(int[] _data) {
        if (EqUtils.DSP_CHIP_SI47925.equals(EqUtils.getEqChipType()) || EqUtils.DSP_CHIP_SI47925_DTS.equals(EqUtils.getEqChipType())) {
            translateArray(_data);
            return 1;
        }

        int channel = _data[0];
        int lpfSlope = _data[1];
        int lpfFreq = _data[2];
        int hpfSlope = _data[3];
        int hpfFreq = _data[4];

        Log.d(TAG, String.format("0peakingCoef setEqHpfLpf data, channel:[%s], lpfSlope:[%s] ,lpfFreq:[%s], hpfSlope:[%s],hpfFreq:[%s]",
                channel, lpfSlope, lpfFreq, hpfSlope, hpfFreq));
        SIDspBiQuadParams lpfBiquad = SI47925EqDealDataUtil.lpfCalcBinary(lpfFreq);
        SIDspBiQuadParams hpfBiquad = SI47925EqDealDataUtil.hpfCalcBinary(hpfFreq);
        SIDspBiQuadParams binary = new SIDspBiQuadParams();
        binary.setB0(0x400000);

        int[] data = new int[74];
        data[0] = SI47925_CMD_SUB_ID_HPF_LPF;
        data[1] = channel;

        int i = 0;
        int index = 2;
        for (i = 0; i < lpfSlope; i++) {
            data[index++] = lpfBiquad.getA1();
            data[index++] = lpfBiquad.getA2();
            data[index++] = lpfBiquad.getB0();
            data[index++] = lpfBiquad.getB1();
            data[index++] = lpfBiquad.getB2();
            data[index++] = lpfBiquad.getbShift();
        }

        //HPF SET
        for (i = 0; i < hpfSlope; i++) {
            data[index++] = hpfBiquad.getA1();
            data[index++] = hpfBiquad.getA2();
            data[index++] = hpfBiquad.getB0();
            data[index++] = hpfBiquad.getB1();
            data[index++] = hpfBiquad.getB2();
            data[index++] = hpfBiquad.getbShift();
        }

        for (i = 0; i < 12 - (lpfSlope + hpfSlope); i++) {
            data[index++] = binary.getA1();
            data[index++] = binary.getA2();
            data[index++] = binary.getB0();
            data[index++] = binary.getB1();
            data[index++] = binary.getB2();
            data[index++] = binary.getbShift();
        }

        Log.d(TAG, String.format("setEqHpfLpf data : %s", Arrays.toString(data)));
        ThreadUtils.executeBySingle(new ThreadUtils.Task<Object>() {
            @Override
            public Object doInBackground() throws Throwable {
                AudioEffect.getInstance().doExtAudioEffect(SI47925_ID, data);
                return null;
            }

            @Override
            public void onSuccess(Object result) {

            }

            @Override
            public void onCancel() {

            }

            @Override
            public void onFail(Throwable t) {

            }
        });
        return 1;
    }

    public int
    setEqHpfLpfSI(int[] _data) {

        int channel = _data[0];
        int xover_Slope = _data[1];
        int xover_Freq = _data[2];

        Log.d(TAG, String.format("0peakingCoef setEqHpfLpfSI data, channel:[%s], xover_Slope:[%s] ,xover_Freq:[%s] : ",
                channel, xover_Slope, xover_Freq));
        List<SIDspBiQuadParamsExt> siDspBiQuadParamsList= SI47925EqDealDataUtil.SIDspHpfLpfFilter(xover_Slope, xover_Freq, FS);

        int[] data = new int[26];
        data[0] = SI47925_CMD_SUB_ID_HPF_LPF_V2;
        data[1] = channel;

        int index = 2;
        for (int i = 0; i < 4; i++) {
            data[index++] = SI47925EqDealDataUtil.convert_to_binary_point(
                    siDspBiQuadParamsList.get(i).getA1(), true, 24, 22);
            data[index++] = SI47925EqDealDataUtil.convert_to_binary_point(
                    siDspBiQuadParamsList.get(i).getA2(), true, 24, 22);
            data[index++] = SI47925EqDealDataUtil.convert_to_binary_point(
                    siDspBiQuadParamsList.get(i).getB0(), true, 24, 22);
            data[index++] = SI47925EqDealDataUtil.convert_to_binary_point(
                    siDspBiQuadParamsList.get(i).getB1(), true, 24, 22);
            data[index++] = SI47925EqDealDataUtil.convert_to_binary_point(
                    siDspBiQuadParamsList.get(i).getB2(), true, 24, 22);
            data[index++] = siDspBiQuadParamsList.get(i).getbShift();

            Log.d(TAG, "setEqHpfLpfSI el " + siDspBiQuadParamsList.get(i).toString());
        }

        Log.d(TAG, "setEqHpfLpfSI data : " + SystemUtils.translateTo16(data));
        ThreadUtils.executeBySingle(new ThreadUtils.Task<Object>() {
            @Override
            public Object doInBackground() throws Throwable {
                AudioEffect.getInstance().doExtAudioEffect(SI47925_ID, data);
                return null;
            }

            @Override
            public void onSuccess(Object result) {

            }

            @Override
            public void onCancel() {

            }

            @Override
            public void onFail(Throwable t) {

            }
        });
        return 1;
    }

    public int setEqSpeakerDelay(int[] _data) {
        Log.d(TAG, "setEqSpeakerDelay: start data : " + Arrays.toString(_data));
        int channel = _data[0];
        int bypass = _data[1];
        int delay = _data[2];
        int polarity = _data[3];
        byte[] buf = new byte[2];
        addParamToBuf(buf, delay, false, 16, 8, 0);

        int delayInt = (int) Math.ceil((((buf[1] & 0xFF) << 8) + (buf[0] & 0xFF)) / 10.0);
        int[] data = new int[5];
        data[0] = SI47925_CMD_SUB_ID_DELAY;
        data[1] = channel;
        data[2] = bypass;
        data[3] = delayInt;
        data[4] = polarity;

        for (int i = 0; i < 5 ; i++) {
            Log.d(TAG, String.format("setEqSpeakerDelay: @%d", data[i]));
        }
        ThreadUtils.executeBySingle(new ThreadUtils.Task<Object>() {
            @Override
            public Object doInBackground() throws Throwable {
                AudioEffect.getInstance().doExtAudioEffect(SI47925_ID, data);
                return null;
            }

            @Override
            public void onSuccess(Object result) {

            }

            @Override
            public void onCancel() {

            }

            @Override
            public void onFail(Throwable t) {

            }
        });
        return 1;
    }

    public int setEqDbbSI(int[] _data) {
        int band = _data[0];
        int freq = _data[1];
        int gain = _data[2];
        double qvalue = 3000;
        Log.d(TAG, String.format("0peakingCoef setEqDbb data, band:[%s], freq:[%s] ,gain:[%s], qvalue:[%s]", band, freq, gain, qvalue));
        SIDspBiQuadParams eqBiQuad = SI47925EqDealDataUtil.peakingCoef(
                SI47925EqDealDataUtil.FILTER_TYPE.FILTER_PEAK_EQ, gain
                , SI47925EqDealDataUtil.FILTER_PARAM_TYPE.FILTER_PARAM_Q,
                new BigDecimal(qvalue / 1000d).setScale(1, RoundingMode.HALF_UP).doubleValue()
                , freq);

        int[] data = new int[8];
        data[0] = SI47925_CMD_SUB_ID_DBB;
        data[1] = band;
        if (gain == 0) {
            data[2] = 0x000000;
            data[3] = 0x000000;
            data[4] = 0x400000;
            data[5] = 0x000000;
            data[6] = 0x000000;
            data[7] = 0x00;
        } else {
            data[2] = eqBiQuad.getA1();
            data[3] = eqBiQuad.getA2();
            data[4] = eqBiQuad.getB0();
            data[5] = eqBiQuad.getB1();
            data[6] = eqBiQuad.getB2();
            data[7] = eqBiQuad.getbShift();
        }
        Log.d(TAG, "setEqDbb: " + SystemUtils.translateTo16(data));
        ThreadUtils.executeBySingle(new ThreadUtils.Task<Object>() {
            @Override
            public Object doInBackground() throws Throwable {
                AudioEffect.getInstance().doExtAudioEffect(SI47925_ID, data);
                return null;
            }

            @Override
            public void onSuccess(Object result) {

            }

            @Override
            public void onCancel() {

            }

            @Override
            public void onFail(Throwable t) {

            }
        });
        return 1;
    }

    public int setEqDbb(int[] _data) {
        if (EqUtils.DSP_CHIP_SI47925.equals(EqUtils.getEqChipType())) {
            if (_data[0] == 1) {
                _data[0] = 0;
                setEqDbbSI(_data);
                _data[0] = 1;
                setEqDbbSI(_data);
            } else if (_data[0] == 2) {
                _data[0] = 2;
                setEqDbbSI(_data);
                _data[0] = 3;
                setEqDbbSI(_data);
            } else if (_data[0] == 3) {
                _data[0] = 5;
                setEqDbbSI(_data);
            }
        }
        if (EqUtils.DSP_CHIP_SI47925_DTS.equals(EqUtils.getEqChipType())) {
            if (_data[0] == 1) {
                _data[0] = 0;
                setEqDbbSI(_data);
                _data[0] = 1;
                setEqDbbSI(_data);
            } else if (_data[0] == 2) {
                _data[0] = 2;
                setEqDbbSI(_data);
                _data[0] = 3;
                setEqDbbSI(_data);
            } else if (_data[0] == 3) {
                _data[0] = 5;
                setEqDbbSI(_data);
            } else if (_data[0] == 4) {
                _data[0] = 4;
                setEqDbbSI(_data);
            }
        }

         return 1;
    }

    public int setEqSurround(int[] _data) {
        int enable = _data[0];
        double main_gain;
        double SRD_gain;
        if (enable == 0) {
            main_gain = 0;
            SRD_gain = -128;
        } else {
            main_gain = -3;
            SRD_gain = 0;
        }


        int[] data = new int[5];

        byte[] paramBuf = new byte[4];
        addParamToBuf(paramBuf, main_gain, true, 16, 8, 0);
        addParamToBuf(paramBuf, SRD_gain, true, 16, 8, 2);
        int cmd = SI47925_ID;
        data[0] = SI47925_CMD_SUB_ID_SURROUND;
        data[1] = paramBuf[0] & 0xFF;
        data[2] = paramBuf[1] & 0xFF;
        data[3] = paramBuf[2] & 0xFF;
        data[4] = paramBuf[3] & 0xFF;
        for (int i = 0; i < 5 ; i++) {
            Log.d(TAG, String.format("setEqSurround: @%d", data[i]));
        }

        Log.d(TAG, "setEqSurround: " + Arrays.toString(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    public void addParamToBuf(byte[] buf,double value, boolean is_signed, int total_bits, int fractional_bits, int offset) {
        int tmp = SI47925EqDealDataUtil.convert_to_binary_point(value, is_signed, total_bits, fractional_bits);
        int sizeBytes = total_bits / fractional_bits;
        for (int i = 0; i < sizeBytes; i++) {
            buf[offset + i] = (byte) ((tmp >> (8 * (i))) & 0xFF);
        }
    }

    public int setEqLoudness(int[] _data) {
        double low_shelf_cf = 200;
        double low_shelf_gain = 6.0;
        double high_shelf_cf = 2000;
        double high_shelf_gain = 6.0;
        if (_data[0] == 0) {
            low_shelf_cf = 0;
            low_shelf_gain = 0;
            high_shelf_cf = 0;
            high_shelf_gain = 0;
        } else if (_data[0] == 2) {
            low_shelf_cf = 200;
            low_shelf_gain = 4.0;
            high_shelf_cf = 2000;
            high_shelf_gain = 4.0;
        } else if (_data[0] == 3) {
            low_shelf_cf = 200;
            low_shelf_gain = 8.0;
            high_shelf_cf = 2000;
            high_shelf_gain = 8.0;
        }
        double qvalue = 1.0;
        SIDspBiQuadParams low_shelf_biquad =
                SI47925EqDealDataUtil.peakingCoef(SI47925EqDealDataUtil.FILTER_TYPE.FILTER_LOW_SHELF
                        , low_shelf_gain, SI47925EqDealDataUtil.FILTER_PARAM_TYPE.FILTER_PARAM_S,
                        qvalue, low_shelf_cf);
        SIDspBiQuadParams high_shelf_biquad =
                SI47925EqDealDataUtil.peakingCoef(SI47925EqDealDataUtil.FILTER_TYPE.FILTER_HIGH_SHELF
                        , high_shelf_gain, SI47925EqDealDataUtil.FILTER_PARAM_TYPE.FILTER_PARAM_S,
                        qvalue, high_shelf_cf);


        int[] data = new int[13];

        if((low_shelf_gain == 0.0) && (high_shelf_gain == 0.0)) {
            // loudness off   bypass
            data[1] = 0x000000;
            data[2] = 0x000000;
            data[3] = 0x400000;
            data[4] = 0x000000;
            data[5] = 0x000000;
            data[6] = 0x00;

            data[7] = 0x000000;
            data[8] = 0x000000;
            data[9] = 0x400000;
            data[10] = 0x000000;
            data[11] = 0x000000;
            data[12] = 0x00;

        } else  {
            data[1] = low_shelf_biquad.getA1();
            data[2] = low_shelf_biquad.getA2();
            data[3] = low_shelf_biquad.getB0();
            data[4] = low_shelf_biquad.getB1();
            data[5] = low_shelf_biquad.getB2();
            data[6] = low_shelf_biquad.getbShift();

            data[7] = high_shelf_biquad.getA1();
            data[8] = high_shelf_biquad.getA2();
            data[9] = high_shelf_biquad.getB0();
            data[10] = high_shelf_biquad.getB1();
            data[11] = high_shelf_biquad.getB2();
            data[12] = high_shelf_biquad.getbShift();
        }

        int cmd = SI47925_ID;
        data[0] = SI47925_CMD_SUB_ID_LOUDNESS;
        Log.d(TAG, "setEqLoudness: " + SystemUtils.translateTo16(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    // 新增DTS功能

    public int setEqDts (int[] _data) {
        int[] data = new int[3];
        int cmd = SI47925_ID;
        data[0] = SI47925_CMD_SUB_ID_DTS;
        data[1] = _data[0] & 0xFF;
        data[2] = _data[1] & 0xFFFF;
        for (int i = 0; i < 3 ; i++) {
            Log.d(TAG, String.format("setEqDts: @%d", data[i]));
        }

        Log.d(TAG, "setEqDts: " + Arrays.toString(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }
}
