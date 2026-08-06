package com.hcn_library.nativeextdsp;


import static com.hcn_library.util.Ak7604Utils.hpfCalculate;
import static com.hcn_library.util.Ak7604Utils.hpfSubCalculate;
import static com.hcn_library.util.Ak7604Utils.lpfCalculate;
import static com.hcn_library.util.Ak7604Utils.lpfSubCalculate;

import android.audio.AudioEffect;
import android.util.Log;

import com.hcn_library.util.Ak7604Utils;
import com.hcn_library.util.EqUtils;

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.Arrays;

public class Ak7604 implements IEq {
    private static final String TAG = Ak7604.class.getSimpleName();

    static final int STATUS_WRITE_CMD_EXE_SUB_ID = 0x88;

    static final int CMD_SUB_ID_EQ_BAND = 0x01;
    static final int CMD_SUB_ID_BALANCE = 0x02;
    static final int CMD_SUB_ID_ATTENUATE_INVERT_SPEAKER = 0x03;
    static final int CMD_SUB_ID_HPF_LPF = 0x04;
    static final int CMD_SUB_ID_DBB = 0x05;
    static final int CMD_SUB_ID_SURROUND = 0x06;
    static final int CMD_SUB_ID_LOUDNESS = 0x07;
    static final int CMD_SUB_ID_DELAY = 0x08;
    static final int CMD_SUB_ID_EQ_BAND_DOUBLE = 0x09;

    static final int CMD_SUB_ID_HPF_LPF_DOUBLE = 0x0A;

    static final int CMD_SUB_ID_DBB_DOUBLE = 0x0B;

    static final int CMD_SUB_ID_BALANCE_DOUBLE = 0x0C;

    static final int CMD_SUB_ID_DELAY_DOUBLE = 0x0D;

    public static final int INDEX_HPF_F = 0x01;
    public static final int INDEX_LPF_F = 0x02;
    public static final int INDEX_HPF_R = 0x03;
    public static final int INDEX_LPF_R = 0x04;
    public static final int INDEX_HPF_SUB = 0x05;
    public static final int INDEX_LPF_SUB = 0x06;
    public static final int INDEX_HPF_CEN = 0x07;
    public static final int INDEX_LPF_CEN = 0x08;
    static final int INDEX_HPF_LPF_MAX = 0x09;

    static final int DBB_F_BAND1 = 0x01;
    static final int DBB_R_BAND1 = 0x02;
    static final int DBB_SUB_BAND1 = 0x03;
    static final int DBB_CHANNEL_MAX = 0x04;

    static double PI = 3.141592653589793238462643383279502884197456789;
    static double FS = 48000;

    /******************************
     input:
     band :0~15
     freq: 20~20000
     gain: -12~+12
     qvalue:0.5~6
     *********************************/
    public int setEqBand(int[] _data) {
        if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType()) || "gb05".equals(EqUtils.getSkinName())) {
            return setEqBandDouble(_data);
        }
        int band = _data[0];
        int freq = _data[1];
        int gain = _data[2];
        int qvalue = _data[3];

        int[] data = new int[7];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_EQ_BAND;
        data[1] = band;
        peakingCoef(freq, gain / 2.0, new BigDecimal(qvalue / 1000d).setScale(1, RoundingMode.HALF_UP).doubleValue(), data);
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    public int setEqBandDouble(int[] _data) {
        Log.d(TAG, "setEqBandDouble: calculate start " + Arrays.toString(_data));
        int[] data;
        int[] result;
        int band = _data[0];
        int freq = _data[1];
        int gain = _data[2];
        int qvalue = _data[3];
        double qValueDouble  = new BigDecimal(qvalue / 1000d).setScale(1, RoundingMode.HALF_UP).doubleValue();
        Log.d(TAG, "setEqBandDouble: band = " + band
                + "\nfreq = " + freq
                + "\ngain = " + gain
                + "\nqvalue = " + qvalue);
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        if (band < 6) {
            data = new int[10];
            Ak7604Utils.calculate(freq, gain / 1.0, qValueDouble , 1, data);
        } else {
            data = new int[5];
            Ak7604Utils.calculate(freq, gain / 1.0, qValueDouble, 0, data);
        }
        result = new int[data.length + 2];
        result[0] = CMD_SUB_ID_EQ_BAND_DOUBLE;
        result[1] = band;
        if (data.length > 0) {
            for (int i = 0; i < data.length; i++) {
                result[i + 2] = data[i];
            }
        }
        Log.d(TAG, "setEqBandDouble: " + Ak7604Utils.translateTo16(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, result);
    }

    /******************************
     input:
     fl :0~15  default 15
     fr :0~15  default 15
     rl :0~15  default 15
     rr :0~15  default 15
     *********************************/
    public int setEqBalance(int[] _data) {
        if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType()) || "gb05".equals(EqUtils.getSkinName())) {
            return setEqBalanceDouble(_data);
        }
        int fl = _data[0];
        int fr = _data[1];
        int rl = _data[2];
        int rr = _data[3];

        int[] data = new int[5];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_BALANCE;
        data[1] = fl;
        data[2] = fr;
        data[3] = rl;
        data[4] = rr;
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    public int setEqBalanceDouble(int[] _data) {
        int fl = _data[0];
        int fr = _data[1];
        int rl = _data[2];
        int rr = _data[3];
        int sub = _data[4];
        int cen = _data[5];

        Log.d(TAG, "setEqBalanceDouble: fl = " + fl
                + "\nfr = " + fr
                + "\nrl = " + rl
                + "\nrr = " + rr
                + "\nsub = " + sub
                + "\ncen = " + cen);

        int[] data = new int[7];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_BALANCE_DOUBLE;
        data[1] = fl;
        data[2] = fr;
        data[3] = rl;
        data[4] = rr;
        data[5] = sub;
        data[6] = cen;
        Log.d(TAG, "setEqBalanceDouble: " + Ak7604Utils.translateTo16(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    /******************************
     input:
     channel :
     enum {
     INDEX_CHANEL_FL = 0x01,
     INDEX_CHANEL_FR,
     INDEX_CHANEL_RL,
     INDEX_CHANEL_RR,
     INDEX_CHANEL_SUB,
     INDEX_CHANEL_MAX
     };

     att :-60~+10
     mute :0:unmute 1:mutes
     invert :0：normal 1：相位翻转
     *********************************/
    public int setEqAttSpeaker(int[] _data) {
        int channel = _data[0];
        int att = _data[1];
        int mute = _data[2];
        int invert = _data[3];

        Log.d(TAG, "setEqAttSpeaker: channel = " + channel
                + "\natt = " + att
                + "\nmute = " + mute
                + "\ninvert = " + invert);

        int[] data = new int[5];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_ATTENUATE_INVERT_SPEAKER;
        data[1] = channel;
        data[2] = att;
        data[3] = mute;
        data[4] = invert;
        Log.d(TAG, "setEqAttSpeaker: " + Ak7604Utils.translateTo16(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    /******************************
     input:
     index :
     enum {
     INDEX_HPF_F = 0x01,
     INDEX_LPF_F,
     INDEX_HPF_R,
     INDEX_LPF_R,
     INDEX_HPF_SUB,
     INDEX_LPF_SUB,
     INDEX_HPF_LPF_MAX
     };
     freq:
     前后左右声道 20~20000HZ；
     重低音声道20~400；
     qvalue:
     为对应斜率，需要单位DB/O时，需要换算,默认6DB/O
     建议查表方式：
     6DB/O=700 12DB/O=750 18DB/O=800 24DB/O=850
     36DB/O=900 48DB/O=950
     OFF = 0
     *********************************/
    public int setEqHpfLpf(int[] _data) {
        if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType()) || "gb05".equals(EqUtils.getSkinName())) {
            return setEqHpfLpfDouble(_data);
        }
        int index = _data[0];
        int freq = _data[1];
        int qvalue = _data[2];

        int[] data = new int[7];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_HPF_LPF;
        data[1] = index;
        data[2] = 0x000000;
        data[3] = 0x000000;
        data[4] = 0x000000;
        data[5] = 0x000000;
        data[6] = 0x400000;

        switch (index) {
            case INDEX_HPF_F:
            case INDEX_HPF_R:
                if (qvalue != 0)
                    hpfCoef(freq, new BigDecimal(qvalue / 1000d).setScale(2, RoundingMode.HALF_UP).doubleValue(), data);
                break;
            case INDEX_LPF_F:
            case INDEX_LPF_R:
                if (qvalue != 0)
                    lpfCoef(freq, new BigDecimal(qvalue / 1000d).setScale(2, RoundingMode.HALF_UP).doubleValue(), data);
                break;
            case INDEX_HPF_SUB:
                if (qvalue != 0)
                    hpfCoef(freq, new BigDecimal(qvalue / 1000d).setScale(2, RoundingMode.HALF_UP).doubleValue(), data);
                break;
            case INDEX_LPF_SUB:
                if (qvalue == 0) {
                    qvalue = 1000;
                    freq = 200;
                }
                lpfCoef(freq, new BigDecimal(qvalue / 1000d).setScale(2, RoundingMode.HALF_UP).doubleValue(), data);
                break;
            default:
                break;
        }
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    public int setEqHpfLpfDouble(int[] _data) {
        Log.d(TAG, "setEqHpfLpfDouble: calculate start " + Arrays.toString(_data));
        int index = _data[0];
        int freq = _data[1];
        int slope = _data[2];

        Log.d(TAG, "setEqHpfLpfDouble: index = " + index
                + "\nfreq = " + freq
                + "\nslope = " + slope);
        int[] fc_table = new int[6];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        int[] data = new int[0];

        switch (index) {
            case INDEX_HPF_F:
            case INDEX_HPF_R:
            case INDEX_HPF_CEN:
                data = new int[57];
                data[0] = CMD_SUB_ID_HPF_LPF_DOUBLE;
                data[1] = index;
                hpfCalculate(slope, fc_table, freq, data);
                break;
            case INDEX_LPF_F:
            case INDEX_LPF_R:
            case INDEX_LPF_CEN:
                data = new int[32];
                data[0] = CMD_SUB_ID_HPF_LPF_DOUBLE;
                data[1] = index;
                lpfCalculate(slope, fc_table, freq, data);
                break;
            case INDEX_HPF_SUB:
                data = new int[52];
                data[0] = CMD_SUB_ID_HPF_LPF_DOUBLE;
                data[1] = index;
                hpfSubCalculate(slope, fc_table, freq, data);
                break;
            case INDEX_LPF_SUB:
                data = new int[52];
                data[0] = CMD_SUB_ID_HPF_LPF_DOUBLE;
                data[1] = index;
                lpfSubCalculate(slope, fc_table, freq, data);
                break;
            default:
                break;
        }
        Log.d(TAG, "setEqHpfLpfDouble: " + Ak7604Utils.translateTo16(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    /******************************
     input:
     index :
     enum {
     DBB_F_BAND1 = 0x01, //前置喇叭
     DBB_R_BAND1,//后置
     DBB_SUB_BAND1,//重低音喇叭
     DBB_CHANNEL_MAX
     };
     freq:
     20~400HZ
     gain: 0~15
     *********************************/
    public int setEqDbb(int[] _data) {
        if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType()) || "gb05".equals(EqUtils.getSkinName())) {
            return setEqDbbDouble(_data);
        }
        int index = _data[0];
        int freq = _data[1];
        int gain = _data[2];

        int[] data = new int[7];
        double qvalue = 5;
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_DBB;
        data[1] = index;
        peakingCoef(freq, gain, qvalue, data);
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    public int setEqDbbDouble(int[] _data) {
        Log.d(TAG, "setEqDbbDouble: start " + Arrays.toString(_data));
        int index = _data[0];
        int freq = _data[1];
        int gain = _data[2];
        double qValue = 5.0D;
        Log.d(TAG, "setEqDbbDouble: index = " + index
                + "\nfreq = " + freq
                + "\ngain = " + gain
                + "\nqValue = " + qValue);
        int[] result;
        int[] data;
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data = new int[10];
        Ak7604Utils.calculate(freq, gain / 1.0, qValue, 1, data);
        result = new int[data.length + 2];
        result[0] = CMD_SUB_ID_DBB_DOUBLE;
        result[1] = index;
        for (int i = 0; i < data.length; i++) {
            result[i + 2] = data[i];
        }
        Log.d(TAG, "setEqDbbDouble: " + Ak7604Utils.translateTo16(result));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, result);
    }

    public int setEqSurround(int[] _data) {
        Log.d(TAG, "setEqSurround");
        int enable = _data[0];

        int[] data = new int[2];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_SURROUND;
        data[1] = enable;
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    /**
     * 关闭，中，低，高
     * 0， 1, 4, 8
     * @param _data
     * @return
     */
    public int setEqLoudness(int[] _data) {
        Log.d(TAG, "setEqLoudness");
        int index = _data[0];
        if ("gb05".equals(EqUtils.getSkinName())) {
            if (index == 2) {
                index = 4;
            }
            if (index == 3) {
                index = 8;
            }
        }
        int[] data = new int[2];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_LOUDNESS;
        data[1] = index;
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    /******************************
     input:
     fl :0~200   step = 0.1ms
     fr :0~200   step = 0.1ms
     rl :0~200   step = 0.1ms
     rr :0~200   step = 0.1ms
     *********************************/
    public int setEqSpeakerDelay(int[] _data) {
        if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType()) || "gb05".equals(EqUtils.getSkinName())) {
            return setEqSpeakerDelayDouble(_data);
        }
        int fl = _data[0];
        int fr = _data[1];
        int rl = _data[2];
        int rr = _data[3];

        int[] data = new int[5];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_DELAY;
        data[1] = fl;
        data[2] = fr;
        data[3] = rl;
        data[4] = rr;
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    public int setEqSpeakerDelayDouble(int[] _data) {
        int dataLen = _data.length;
        Log.d(TAG, "setEqSpeakerDelayDouble: start " + Arrays.toString(_data));
        int fl = _data[0];
        int fr = _data[1];
        int rl = _data[2];
        int rr = _data[3];
        int sub = -1;
        int cen = -1;
        int[] data = new int[7];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_DELAY_DOUBLE;
        data[1] = fl;
        data[2] = fr;
        data[3] = rl;
        data[4] = rr;
        if (dataLen >= 6) {
            sub = _data[4];
            cen = _data[5];
            data[5] = sub;
            data[6] = cen;
        }
        Log.d(TAG, "setEqSpeakerDelayDouble: fl: " + fl
                + " \nfr: " + fr
                + " \nrl: " + rl
                + " \nrr: " + rr
                + " \nsub: " + sub
                + " \ncen: " + cen);
        Log.d(TAG, "setEqSpeakerDelayDouble: " + Ak7604Utils.translateTo16(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    private int fix24(double x, int shift) {
        //x = x * (1 << (23 - shift));  // pow(2, 23 - shift);
        x = x * (Math.pow(2, 23 - shift));
        x = Math.min(8388607.0, x);
        x = Math.max(-8388608.0, x);
        if (x < 0)
            x += 16777216;
        return (int) Math.max(0., Math.min(16777215.0, x + 0.5));
    }

    private void peakingCoef(double fc, double gain, double Q, int[] result) {
        double A = Math.pow(10., gain / 40.0);
        double omega = 2.0 * PI * fc / FS;
        double sn = Math.sin(omega);
        double cs = Math.cos(omega);
        double alpha = sn / (2.0 * Q);

        double p0 = 1. + (alpha / A);
        double a2 = (1. - alpha * A) / p0;
        double a1 = -2. * cs / p0;
        double b2 = -(1. - alpha / A) / p0;
        double b1 = 2. * cs / p0;
        double a0 = (1. + alpha * A) / p0;

        //printf("%f :: b = %f, %f, %f, a = %f, %f\n", gain, a0, a1, a2, b1, b2);
        result[2] = fix24(a2, 1);
        result[3] = fix24(a1, 1);
        result[4] = fix24(b2, 1);
        result[5] = fix24(b1, 1);
        result[6] = fix24(a0, 1);

        Log.d(TAG, String.format("peaking_coef:%x, %x, %x, %x, %x\n", result[2], result[3], result[4], result[5], result[6]));
    }

    private void lowShelf(double fc, double gain, double Q, int[] result) {
        double omega = 2. * PI * fc / FS;
        double sn = Math.sin(omega);
        double cs = Math.cos(omega);
        double A = Math.pow(10., gain / 40.0);
        double beta = Math.sqrt(A / Q);
        double p0 = (A + 1.) + (A - 1.) * cs + beta * sn;

        double a2 = (A * ((A + 1.) - (A - 1.) * cs - beta * sn)) / p0;
        double a1 = (2 * A * ((A - 1) - (A + 1) * cs)) / p0;
        double b2 = -((A + 1) + (A - 1) * cs - beta * sn) / p0;
        double b1 = -(-2 * ((A - 1) + (A + 1) * cs)) / p0;
        double a0 = (A * ((A + 1) - (A - 1) * cs + beta * sn)) / p0;

        //printf("lowShelf :: freq = %f. %f, %f, %f, a = %f, %f\n", fc, a0, a1, a2, b1, b2);

        result[2] = fix24(a2, 1); // float64_t_fix4p27(b0);
        result[3] = fix24(a1, 1); // float64_t_fix4p27(b1);
        result[4] = fix24(b2, 1); // float64_t_fix4p27(b2);
        result[5] = fix24(b1, 1); // float64_t_fix4p27(-b1);  // a1
        result[6] = fix24(a0, 1); // float64_t_fix4p27(a2);

        Log.d(TAG, String.format("lowShelf:%x, %x, %x, %x, %x\n", result[2], result[3], result[4], result[5], result[6]));
    }

    private void hpfCoef(double fc, double Q, int[] result) {
        double omega = 2. * PI * fc / FS;
        double sn = Math.sin(omega);
        double cs = Math.cos(omega);
        double alpha = sn / (2. * Q);

        double p0 = 1. + alpha;
        double a2 = (1. + cs) / 2. / p0;
        double a1 = -(1. + cs) / p0;
        double b2 = -(1. - alpha) / p0;
        double b1 = 2. * cs / p0;
        double a0 = (1. + cs) / 2. / p0;

        //printf("hpf :: freq = %f. %f, %f, %f, a = %f, %f\n", fc, a0, a1, a2, b1, b2);

        result[2] = fix24(a2, 1); // float64_t_fix4p27(b0);
        result[3] = fix24(a1, 1); // float64_t_fix4p27(b1);
        result[4] = fix24(b2, 1); // float64_t_fix4p27(b2);
        result[5] = fix24(b1, 1); // float64_t_fix4p27(-b1);  // a1
        result[6] = fix24(a0, 1); // float64_t_fix4p27(a2);

        Log.d(TAG, String.format("hpf_coef:%x, %x, %x, %x, %x\n", result[2], result[3], result[4], result[5], result[6]));
    }

    private void lpfCoef(double fc, double Q, int[] result) {
        double omega = 2. * PI * fc / FS;
        double sn = Math.sin(omega);
        double cs = Math.cos(omega);
        double alpha = sn / (2. * Q);

        double p0 = 1. + alpha;
        double a2 = (1. - cs) / 2. / p0;
        double a1 = (1. - cs) / p0;
        double b2 = -(1. - alpha) / p0;
        double b1 = 2. * cs / p0;
        double a0 = (1. - cs) / 2. / p0;

        //printf("lpf :: freq = %f. %f, %f, %f, a = %f, %f\n", fc, a0, a1, a2, b1, b2);

        result[2] = fix24(a2, 1); // float64_t_fix4p27(b0);
        result[3] = fix24(a1, 1); // float64_t_fix4p27(b1);
        result[4] = fix24(b2, 1); // float64_t_fix4p27(b2);
        result[5] = fix24(b1, 1); // float64_t_fix4p27(-b1);  // a1
        result[6] = fix24(a0, 1); // float64_t_fix4p27(a2);

        Log.d(TAG, String.format("lpf_coef:%x, %x, %x, %x, %x\n", result[2], result[3], result[4], result[5], result[6]));
    }

}
