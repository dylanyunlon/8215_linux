package com.hcn_library.nativeextdsp;

import android.audio.AudioEffect;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.hcn_library.util.Ak7739Utils;

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.Arrays;

public class Ak7739 implements IEq {
    private static final String TAG = Ak7739.class.getSimpleName();

    static final int STATUS_WRITE_CMD_EXE_SUB_ID = 0x88;
    static final int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;

    static final int CMD_SUB_ID_EQ_BAND = 0x01;
    static final int CMD_SUB_ID_BALANCE = 0x02;
    static final int CMD_SUB_ID_ATTENUATE_INVERT_SPEAKER = 0x03;
    static final int CMD_SUB_ID_HPF_LPF = 0x04;
    static final int CMD_SUB_ID_DBB = 0x05;
    static final int CMD_SUB_ID_SURROUND = 0x06;
    static final int CMD_SUB_ID_LOUDNESS_STATUS = 0x07;
    static final int CMD_SUB_ID_LOUDNESS_BASSFILTER = 0x08;
    static final int CMD_SUB_ID_LOUDNESS_TREBLEFILTER = 0x09;

    static final int CMD_SUB_ID_LOUDNESS_BASS_MAXGAIN = 0x0A;

    static final int CMD_SUB_ID_LOUDNESS_TREBLE_MAXGAIN = 0x0B;

    static final int CMD_SUB_ID_DELAY = 0x0C;

    static final int CMD_SUB_ID_CALLERAKM_STATUS = 0x0D;
    static final int CMD_SUB_ID_CALLERAKM_REVERB_LEVEL = 0x0E;
    static final int CMD_SUB_ID_CALLERAKM_BASSBOOST = 0x0F;
    static final int CMD_SUB_ID_CALLERAKM_DYNAMIC_LEVEL = 0x11;
    static final int CMD_SUB_ID_CALLERAKM_SURROUND_LEVEL = 0x12;
    static final int CMD_SUB_ID_CALLERAKM_HIFIL_LEVEL = 0x13;
    static final int CMD_SUB_ID_CALLERAKM_SOUNDBALANCE_LEVEL = 0x16;

    public static final int INDEX_HPF_F = 0x01;
    public static final int INDEX_LPF_F = 0x02;
    public static final int INDEX_HPF_R = 0x03;
    public static final int INDEX_LPF_R = 0x04;
    public static final int INDEX_HPF_SUB = 0x05;
    public static final int INDEX_LPF_SUB = 0x06;
    public static final int INDEX_HPF_CEN = 0x07;
    public static final int INDEX_LPF_CEN = 0x08;


    static double PI = 3.141592653589793238462643383279502884197456789;
    static double FS = 48000;

    private boolean isVirtualCenterEnable;
    private boolean isEqDtsEnable;
    private boolean isSoundFocusEnable;
    private boolean isSoundSurroundEnable;
    private boolean isBassBoostEnable;

/******************************
     input:均衡器
     band :0~35
     freq: 20~20000
     gain: -12~+12
     qvalue:5000
     *********************************/

    public int setEqBand(int[] _data) {
        int band = _data[0];
        int freq = _data[1];
        int gain = _data[2];
        int qvalue = _data[3];

        int[] data = new int[7];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_EQ_BAND;
        data[1] = band;

        if (band >= 0 && band < 36) {
            // band 0~36 use single precision mode
            Ak7739Utils.cal_coef(freq, gain, qvalue/1000d, data, 0);
        }
        log(TAG, "均衡器setEqBand:" + Ak7739Utils.translateTo16(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }



/******************************
     input:声场
     0~157
     default 145
     0:mute
     sub cen 位置？？？
     *********************************/

    public int setEqBalance(int[] _data) {
        int[] data = new int[7];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_BALANCE;
        System.arraycopy(_data, 0, data, 1, 6);
        log(TAG, "声场setEqBalance:" + Arrays.toString(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

/******************************
 * 环绕增益
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

     att :-15~0
     mute :0:unmute 1:mutes
     invert :0：normal 1：相位翻转 保留
     *********************************/

    public int setEqAttSpeaker(int[] _data) {
        int defaultAtt = 145;
        int channel = _data[0];
//        int att = defaultAtt + _data[1];
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
        log(TAG, "setEqAttSpeaker: " + Ak7739Utils.translateTo16(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

/**
     * change check !!!
     * 高低通滤波
     * @param _data
     * @return
     */

    public int setEqHpfLpf(int[] _data) {
        Log.d(TAG, "setEqHpfLpfDouble: calculate start " + Arrays.toString(_data));
        int index = _data[0];
        int[] data = new int[22];
        data[0] = CMD_SUB_ID_HPF_LPF;
        data[1] = index;
        int type = index % 2 == 0 ? Ak7739Utils.LPF : Ak7739Utils.HPF;
        int slope = (type == Ak7739Utils.LPF ? _data[1] : _data[3]) * 6;
        slope = slope == 36 ? 48 : slope;
        slope = slope == 30 ? 36 : slope;
        int freq = type == Ak7739Utils.LPF ? _data[2] : _data[4];
        Log.d(TAG, "setEqHpfLpfDouble: index = " + index
                + "\nfreq = " + freq
                + "\nslope = " + slope);
        int[] result = new int[20];
        Ak7739Utils.lpfHpfCalculate(result, freq, slope, type);
        System.arraycopy(result, 0, data, 2, 20);
//        log(TAG, "setEqHpfLpfDouble: " + Arrays.toString(data));
        log(TAG, "setEqHpfLpfDouble: " + Ak7739Utils.translateTo16(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

/*    public int setEqHpfLpf(int[] _data) {
        Log.d(TAG, "setEqHpfLpfDouble: calculate start " + Arrays.toString(_data));
        int index = _data[0];
        int freq = _data[1];
        int slope = _data[2];

        Log.d(TAG, "setEqHpfLpfDouble: index = " + index
                + "\nfreq = " + freq
                + "\nslope = " + slope);
        int[] fc_table = new int[6];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        int[] data = new int[22];
        data[0] = CMD_SUB_ID_HPF_LPF;
        data[1] = index;
        boolean isHPF = (index == INDEX_HPF_F || index == INDEX_HPF_R || index == INDEX_HPF_SUB || index == INDEX_HPF_CEN);
        int[] subData = new int[5];
        switch (slope) {
            case 0: //斜率开关关闭
                break;
            case 1: // 6db/0
                subData = Ak7739Utils.CalcIIR2Coef(freq, isHPF ? Ak7739Utils.IIR2_1stHPF : Ak7739Utils.IIR2_1stLPF);
                System.arraycopy(subData, 0, data, 2, 5);
                break;
            case 2:
                subData = Ak7739Utils.CalcIIR2Coef(freq, isHPF ? Ak7739Utils.IIR2_HPF : Ak7739Utils.IIR2_LPF);
                System.arraycopy(subData, 0, data, 2, 5);
                break;
            case 3:
                subData = Ak7739Utils.CalcIIR2Coef(freq, isHPF ? Ak7739Utils.IIR2_HPF : Ak7739Utils.IIR2_LPF);
                System.arraycopy(subData, 0, data, 2, 5);
                subData = Ak7739Utils.CalcIIR2Coef(freq, isHPF ? Ak7739Utils.IIR2_1stHPF : Ak7739Utils.IIR2_1stLPF);
                System.arraycopy(subData, 0, data, 7, 5);
                break;
            case 4:
                subData = Ak7739Utils.CalcIIR2Coef(freq, isHPF ? Ak7739Utils.IIR2_HPF : Ak7739Utils.IIR2_LPF);
                System.arraycopy(subData, 0, data, 2, 5);
                subData = Ak7739Utils.CalcIIR2Coef(freq, isHPF ? Ak7739Utils.IIR2_HPF : Ak7739Utils.IIR2_LPF);
                System.arraycopy(subData, 0, data, 7, 5);
                break;
            case 5:
                subData = Ak7739Utils.CalcIIR2Coef(freq, isHPF ? Ak7739Utils.IIR2_HPF : Ak7739Utils.IIR2_LPF);
                System.arraycopy(subData, 0, data, 2, 5);
                subData = Ak7739Utils.CalcIIR2Coef(freq, isHPF ? Ak7739Utils.IIR2_HPF : Ak7739Utils.IIR2_LPF);
                System.arraycopy(subData, 0, data, 7, 5);
                subData = Ak7739Utils.CalcIIR2Coef(freq, isHPF ? Ak7739Utils.IIR2_HPF : Ak7739Utils.IIR2_LPF);
                System.arraycopy(subData, 0, data, 12, 5);
                break;
            case 6:
                subData = Ak7739Utils.CalcIIR2Coef(freq, isHPF ? Ak7739Utils.IIR2_HPF : Ak7739Utils.IIR2_LPF);
                System.arraycopy(subData, 0, data, 2, 5);
                subData = Ak7739Utils.CalcIIR2Coef(freq, isHPF ? Ak7739Utils.IIR2_HPF : Ak7739Utils.IIR2_LPF);
                System.arraycopy(subData, 0, data, 7, 5);
                subData = Ak7739Utils.CalcIIR2Coef(freq, isHPF ? Ak7739Utils.IIR2_HPF : Ak7739Utils.IIR2_LPF);
                System.arraycopy(subData, 0, data, 12, 5);
                subData = Ak7739Utils.CalcIIR2Coef(freq, isHPF ? Ak7739Utils.IIR2_HPF : Ak7739Utils.IIR2_LPF);
                System.arraycopy(subData, 0, data, 17, 5);
                break;
            default:
                break;
        }
        Log.d(TAG, "setEqHpfLpfDouble: " + Ak7739Utils.translateTo16(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }*/


    public int setEqSurround(int[] _data) {
        Log.d(TAG, "setEqSurround");
        int enable = _data[0];

        int[] data = new int[2];
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_SURROUND;
        data[1] = enable;
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    /******************************
     环绕延时
     input:
     fl :0~149   step = 0.1ms
     fr :0~149   step = 0.1ms
     rl :0~149   step = 0.1ms
     rr :0~149   step = 0.1ms
     sub :0~149   step = 0.1ms
     cen :0~149   step = 0.1ms
     *********************************/

    public int setEqSpeakerDelay(int[] _data) {
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
        data[0] = CMD_SUB_ID_DELAY;
        data[1] = fl * 10;
        data[2] = fr * 10;
        data[3] = rl * 10;
        data[4] = rr * 10;
        if (dataLen >= 6) {
            sub = _data[4];
            cen = _data[5];
            data[5] = sub * 10;
            data[6] = cen * 10;
        }
        Log.d(TAG, "setEqSpeakerDelay: fl: " + fl
                + " \nfr: " + fr
                + " \nrl: " + rl
                + " \nrr: " + rr
                + " \nsub: " + sub
                + " \ncen: " + cen);
        log(TAG, "setEqSpeakerDelay: " + Ak7739Utils.translateTo16(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }


/**
     * 均衡器响度
     * @param _data
     * @return
     */

    public int setEqLoudness(int[] _data) {
        Log.d(TAG, "setEqLoudness");
        int[] dataBassGain = new int[2];
        int[] dataTrebleGain = new int[2];
        int[] dataBassFilter = new int[6];
        int[] dataTrebleFilter = new int[6];
        dataBassGain[0] = CMD_SUB_ID_LOUDNESS_BASS_MAXGAIN;
        dataTrebleGain[0] = CMD_SUB_ID_LOUDNESS_TREBLE_MAXGAIN;
        dataBassFilter[0] = CMD_SUB_ID_LOUDNESS_BASSFILTER;
        dataTrebleFilter[0] = CMD_SUB_ID_LOUDNESS_TREBLEFILTER;
        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        int bass_fc = 350;
        int treble_fc = 1600;
        float bass_q = 0.8f;
        float treble_q = 0.9f;
        switch (_data[0]) {
            case 0://关闭
                dataBassGain[1] = 0;
                dataTrebleGain[1] = 0;
                break;
            case 1://低
                dataBassGain[1] = Ak7739Utils.loudnessLevelCal(2);
                dataTrebleGain[1] = Ak7739Utils.loudnessLevelCal(1.2);
                Ak7739Utils.bass_shelving(bass_fc, bass_q, dataBassFilter, 0);
                Ak7739Utils.treble_shelving(treble_fc, treble_q, dataTrebleFilter, 0);
                break;
            case 2://中
                dataBassGain[1] = Ak7739Utils.loudnessLevelCal(3.1);
                dataTrebleGain[1] = Ak7739Utils.loudnessLevelCal(2.2);
                Ak7739Utils.bass_shelving(bass_fc, bass_q, dataBassFilter, 0);
                Ak7739Utils.treble_shelving(treble_fc, treble_q, dataTrebleFilter, 0);
                break;
            case 3://高
                dataBassGain[1] = Ak7739Utils.loudnessLevelCal(4.6);
                dataTrebleGain[1] = Ak7739Utils.loudnessLevelCal(3.0);
                Ak7739Utils.bass_shelving(bass_fc, bass_q, dataBassFilter, 0);
                Ak7739Utils.treble_shelving(treble_fc, treble_q, dataTrebleFilter, 0);
                break;
            default:
                break;
        }
        AudioEffect.getInstance().doExtAudioEffect(cmd, dataBassGain);
        AudioEffect.getInstance().doExtAudioEffect(cmd, dataTrebleGain);
        AudioEffect.getInstance().doExtAudioEffect(cmd, dataBassFilter);
        log(TAG, "setEqLoudness "
                +"\ndataBassGain:" + Ak7739Utils.translateTo16(dataBassGain)
                +"\ndataTrebleGain:" + Ak7739Utils.translateTo16(dataTrebleGain)
                +"\ndataBassFilter:" + Ak7739Utils.translateTo16(dataBassFilter)
                +"\ndataTrebleFilter:" + Ak7739Utils.translateTo16(dataTrebleFilter));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, dataTrebleFilter);
    }

    //场景模式，参数待定
    public int setEqDts (int[] _data) {
        int[] data = new int[2];
//        int cmd = STATUS_WRITE_CMD_EXE_SUB_ID;
        data[0] = CMD_SUB_ID_CALLERAKM_REVERB_LEVEL;
        data[1] = _data[1];
//        data[2] = _data[1] & 0xFFFF;
        for (int i = 0; i < data.length ; i++) {
            Log.d(TAG, String.format("setEqDts: @%d", data[i]));
        }
        isEqDtsEnable = data[1] > 0;
        setEqDtsEnable();
        log(TAG, "场景模式setEqDts: " + Arrays.toString(data));
//        Thread.dumpStack();
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    //虚拟中置，参数待定
    public int setVirtualCenter(int[] _data) {
        int[] data = new int[2];
        data[0] = CMD_SUB_ID_CALLERAKM_SOUNDBALANCE_LEVEL;
        data[1] = _data[0];
        isVirtualCenterEnable = _data[0] > 0;
        setEqDtsEnable();
        log(TAG, "setVirtualCenter: " + Arrays.toString(data));
//        Thread.dumpStack();
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    //声音聚焦，参数待定
    public int setSoundFocus(int[] _data){
        int[] data = new int[2];
        data[0] = CMD_SUB_ID_CALLERAKM_DYNAMIC_LEVEL;
        data[1] = _data[1];
        isSoundFocusEnable = _data[1] > 0;
        setEqDtsEnable();
        log(TAG, "setSoundFocus: " + Arrays.toString(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    //环绕模式，参数待定
    public int setSoundSurround(int[] _data){
        int[] data = new int[2];
        data[0] = CMD_SUB_ID_CALLERAKM_SURROUND_LEVEL;
        data[1] = _data[1];
        isSoundSurroundEnable = _data[1] > 0;
        setEqDtsEnable();
        log(TAG, "setSoundSurround: " + Arrays.toString(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    //低音增强，参数待定
    public int setBassBoost(int[] _data){
        int[] data = new int[3];
        data[0] = CMD_SUB_ID_CALLERAKM_BASSBOOST;
        //todo
        data[1] = _data[0];
        data[2] = _data[1];
        isBassBoostEnable = data[1] > 0;
        setEqDtsEnable();
        log(TAG, "setBassBoost: " + Arrays.toString(data));
        return AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }


    //音效总开关
    private void setEqDtsEnable(){
        int[] data = new int[2];
        data[0] = CMD_SUB_ID_CALLERAKM_STATUS;
        if (isVirtualCenterEnable || isEqDtsEnable || isSoundFocusEnable || isSoundSurroundEnable || isBassBoostEnable) {
            data[1] = 1;
        } else {
            data[1] = 0;
        }
        log(TAG, "音效总开关setEqDtsEnable:" + Arrays.toString(data));
        AudioEffect.getInstance().doExtAudioEffect(cmd, data);
    }

    private TextView resultTv;

    public void setResultTv(TextView resultTv) {
        this.resultTv = resultTv;
    }

    private void log(String TAG,String msg) {
        Log.d(TAG, msg);
        if (resultTv != null) {
            resultTv.setVisibility(View.VISIBLE);
            resultTv.setText(msg);
        }
    }
}
