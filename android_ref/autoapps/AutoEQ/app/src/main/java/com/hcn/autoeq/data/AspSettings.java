package com.hcn.autoeq.data;

import android.audio.AudioEffect;
import android.content.Context;
import android.util.Log;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.util.ConstantAsp;
import com.hcn.autoeq.util.ConstantEq;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SetupSharedData;

import java.util.Arrays;

public class AspSettings implements ConstantEq, ConstantAsp {

    static final String TAG = AspSettings.class.getSimpleName();
    static final boolean DEBUG = Log.isLoggable(AspSettings.class.getSimpleName(), Log.DEBUG);
    protected Context mContext;
    public static AspSettings mEqDataSetting = null;
    private AudioEffect mAudioEffect = null;
    private SPUtils mDspPre, mAspPre, mBalancePre;
    //user 模式保存数值
    private boolean mAspUserReverb = true;

    public static AspSettings getInstance(Context mContext) {
        if (null == mEqDataSetting) {
            mEqDataSetting = new AspSettings(mContext);
        }
        return mEqDataSetting;
    }

    private AspSettings(Context context) {
        mContext = context;
        if (null == mAudioEffect) {
            mAudioEffect = AudioEffect.getInstance();
            mDspPre = SPUtils.getInstance(EQ_SAVE_DSP);
            mAspPre = SPUtils.getInstance(EQ_SAVE_ASP);
            mBalancePre = SPUtils.getInstance(EQ_SAVE_BALANCE);
            //mAspUserReverb = getEQReverbType(0) > IEqConstant.EQ_REVERB_SIZE ? true : false;
        }
    }

    //@Commit ASP设置用户选择音效.
    public void setAspReverbType(int reverb, int[] mAspVal) {
        setAspBassBoost(mAspVal[0], true);
        setAspTreble(mAspVal[1], true);
        Log.i(TAG, "ASP reverb:" + reverb + ",Treble:" + getAspTreble() + ",BassBoost:" + getAspBassBoost());
        mAspPre.put(STATUS_REVERB_TYPE, reverb, true);
    }

    public int getAspReverbType() {
        return mAspPre.getInt(STATUS_REVERB_TYPE, 0);
    }

    public void setAspBandValue(String value) {
        //只保存User模式下值
        mAspPre.put(STATUS_ASP_BAND, value, true);
    }

    public int[] getUserBandValue() {
        int[] mUserVal = new int[EqUtils.ASP_BAND_DEPTH];
        String mAllBandVal = mAspPre.getString(STATUS_ASP_BAND);
        //USER 模式获取保存值
        if (!"".equals(mAllBandVal)) {
            mAllBandVal = mAllBandVal.substring(mAllBandVal.indexOf("[") + 1, mAllBandVal.indexOf("]"));
            String[] mStr = mAllBandVal.split(",");
            for (int i = 0; i < EqUtils.ASP_BAND_DEPTH; i++) {
                //获取各Band值
                mUserVal[i] = Integer.valueOf(mStr[i].trim()).intValue();
            }
        } else {
            mUserVal = Arrays.copyOf(DEF_ASP_BANDS[ConstantEq.EQ_REVERB_USER], DEF_ASP_BANDS[ConstantEq.EQ_REVERB_USER].length);
        }
        return mUserVal;
    }

    //@Commit ASP设置高音.
    public void setAspTreble(int mTrebleVal, boolean mCommit) {
        mAudioEffect.setTreble(mTrebleVal, AudioEffect.DONOTCARE);
        if (mCommit) {
            mAspPre.put(STATUS_TREBLE, mTrebleVal, true);
        }
        Log.i(TAG, "setAspTreble" + mTrebleVal);
    }

    public int getAspTreble() {
        return mAspPre.getInt(STATUS_TREBLE, 7);
    }

    //@Commit ASP设置低音.
    public void setAspBassBoost(int mBassBoostVal, boolean mCommit) {
        mAudioEffect.setBassBoost(mBassBoostVal, 200);
        if (mCommit) {
            mAspPre.put(STATUS_BASS_BOOST, mBassBoostVal, true);
        }
        Log.i(TAG, "setAspBassBoost" + mBassBoostVal);
    }

    public int getAspBassBoost() {
        return mAspPre.getInt(STATUS_BASS_BOOST, 7);
    }

    //ASP Surround
    public void setAspSurround(int mSurround) {
        Log.d(TAG, "setAspSurround: mSurround = " + mSurround);
        mAudioEffect.setSurround(mSurround);
        mAspPre.put(STATUS_SURROUND, mSurround, true);
    }

    public boolean getAspSurround() {
        return 1 == mAspPre.getInt(STATUS_SURROUND, 0);
    }

    //ASP Loudness
    public void setAspLoudness(int mLoud) {
        Log.d(TAG, "setAspLoudness: mLoud = " + mLoud);
        mAudioEffect.setLoud(mLoud);
        mAspPre.put(STATUS_LOUDNESS, mLoud, true);
    }

    public boolean getAspLoudness() {
        return 1 == mAspPre.getInt(STATUS_LOUDNESS, 0);
    }

    public void setAspSubWoofer(int mSubWoofer, boolean mCommit) {
        Log.d(TAG, "setAspSubWoofer: mSubWoofer = " + mSubWoofer + " mCommit = " + mCommit);
        mAudioEffect.setSubWoofer(mSubWoofer, 20);
        if (mCommit) {
            mAspPre.put(STATUS_SUBWOOFER, mSubWoofer, true);
        }
    }

    public int getAspSubWoofer() {
        //先拿配置文件的值，如果为空，则拿本地dsp保存的值来判断是否开关，若还未空，则拿asp本地保存的值！
        return SetupSharedData.getInstance(mContext).getIntValue(EqUtils.HEQ_BASS_BOOST, mDspPre.getInt(STATUS_BASS_BOOST, mAspPre.getInt(STATUS_SUBWOOFER, 0))) > 0 ? 1 : 0;
    }

    public void setAspBalance(int x, int y, Boolean saveVal) {
        if (saveVal) {
            mBalancePre.put(STATUS_BALANCE_X, x, true);
            mBalancePre.put(STATUS_BALANCE_Y, y, true);
        }

        final int BALANCE_VALUE_MAX = 7;
        final int BALANCE_DISTAND = 7;
        final int BALANCE_FURTHEST = 49;

        int FL, FR, RL, RR;
        int temp = 15;

        if ((x >= 0) && (y >= 0)) {
            FL = (temp * (BALANCE_VALUE_MAX - x)) / BALANCE_DISTAND;
            FR = temp;
            RL = (temp * (BALANCE_VALUE_MAX - y) * (BALANCE_VALUE_MAX - x)) / BALANCE_FURTHEST;
            RR = (temp * (BALANCE_VALUE_MAX - y)) / BALANCE_DISTAND;
        } else if ((x >= 0) && (y < 0)) {
            FL = (temp * (BALANCE_VALUE_MAX - x) * (y + BALANCE_VALUE_MAX)) / BALANCE_FURTHEST;
            FR = (temp * (y + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            RL = (temp * (BALANCE_VALUE_MAX - x)) / BALANCE_DISTAND;
            RR = temp;
        } else if ((x < 0) && (y >= 0)) {
            FL = temp;
            FR = (temp * (x + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            RL = (temp * (BALANCE_VALUE_MAX - y)) / BALANCE_DISTAND;
            RR = (temp * (x + BALANCE_VALUE_MAX) * (BALANCE_VALUE_MAX - y)) / BALANCE_FURTHEST;
        } else {
            FL = (temp * (y + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
            FR = (temp * (x + BALANCE_VALUE_MAX) * (y + BALANCE_VALUE_MAX)) / BALANCE_FURTHEST;
            RL = temp;
            RR = (temp * (x + BALANCE_VALUE_MAX)) / BALANCE_DISTAND;
        }
        Log.i(TAG, "setAspBalance FL=" + FL + ",FR=" + FR + ",RL=" + RL + ",RR=" + RR);
        mAudioEffect.setBalance(FL, FR, RL, RR);
        if (!EqUtils.hasAsp() || EqUtils.isRk3326()) {
            float mLeft = FL > RL ? FL : RL;
            float mRight = FR > RR ? FR : RR;
            Log.d(TAG, "setAspBalance: mLeft = " + mLeft + " mRight = " + mRight);
            AudioEffect.getInstance().setMasterVolume(mLeft / 15, mRight / 15);
        }
    }

    public int[] getAspBalance() {
        int[] xy = new int[2];
        //从文件中获取的都是真实值，范围 -7 ~ +7
        xy[0] = mBalancePre.getInt(STATUS_BALANCE_X, 0);
        xy[1] = mBalancePre.getInt(STATUS_BALANCE_Y, 0);
        return xy;
    }

    /**
     * Asp应用程序启动时从保存的文件获取属性值设置给系统 Asp默认低音频率10KHz ,低音100Hz,ASP只能设置高低音.
     */
    public void startBootAspSetting() {
        Log.i(TAG, "Asp bootAspSetting start");
        //高低音设置设置
        mAudioEffect.setTreble(getAspTreble(), AudioEffect.DONOTCARE);
        mAudioEffect.setBassBoost(getAspBassBoost(), 100);
        //平衡
        int[] mBlance = getAspBalance();
        setAspBalance(mBlance[0], mBlance[1], false);
        //响度
        mAudioEffect.setLoud(getAspLoudness() ? 1 : 0);
        //线路输出
        mAudioEffect.setSubWoofer(getAspSubWoofer() > 0 ? 1 : 0, 200);
        Log.i(TAG, "Boot Set ASP Treble:" + getAspTreble() + ",BassBoost:" + getAspBassBoost() +
                ",Balance:" + Arrays.toString(mBlance));
    }
}
