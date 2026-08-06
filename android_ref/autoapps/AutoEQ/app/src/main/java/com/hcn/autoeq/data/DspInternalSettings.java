package com.hcn.autoeq.data;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.media.audiofx.BassBoost;
import android.media.audiofx.Equalizer;
import android.media.audiofx.Virtualizer;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.animation.LinearInterpolator;

import com.blankj.utilcode.util.SPUtils;
import com.hcn.autoeq.util.ConstantDsp;
import com.hcn.autoeq.util.ConstantEq;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SetupSharedData;

import java.util.Arrays;

public class DspInternalSettings implements ConstantEq, ConstantDsp {

    public static final String HEQ_TAB_DSP_NAME = "heq_tab_dsp_name"; //主页tab栏（dsp）的名称
    public static final String HIDE_EQ_BALANCE = "hide_eq_balance"; //隐藏平衡功能字段;
    public static final String HIDE_EQ_SURROUND = "hide_eq_surround"; //隐藏环绕功能字段;
    public static final String NEED_HIDE_EQ_BALANCE = "1";// 1 隐藏平衡功能;
    public static final String NEED_HIDE_EQ_SURROUND = "1";// 1 隐藏环绕功能;
    static final String TAG = DspInternalSettings.class.getSimpleName();
    static final boolean DEBUG = Log.isLoggable(DspInternalSettings.class.getSimpleName(), Log.DEBUG);
    private final static int MSG_INIT_EFFECTS = 1;
    private final static int MSG_INIT_BASSBOOT = 2;
    private final static int MSG_INIT_VIRTUALIZER = 3;

    protected Context mContext;
    public static DspInternalSettings mInternalSetting = null;
    private Equalizer mEqualizer;
    private BassBoost mBass;
    private Virtualizer mVirtualizer;
    private SPUtils mDspPre;
    //音效较大时power需做淡出，不能直接释放.
    private boolean mDspPower = true;

    public static DspInternalSettings getInstance(Context mContext) {
        if (null == mInternalSetting) {
            mInternalSetting = new DspInternalSettings(mContext);
        }
        return mInternalSetting;
    }

    private DspInternalSettings(Context context) {
        mContext = context;
        mDspPre = SPUtils.getInstance(EQ_SAVE_DSP);
        initEffects();
    }

    private void initEffects() {
        Log.d(TAG, "initEffects: " + EqUtils.disableInternalDsp());
        if (EqUtils.disableInternalDsp()) {
            releaseEffects();
            Log.i(TAG, "initEffects Failed: factory disabled !");
        } else {
            if (mEqualizer == null) {
                mEqualizer = new Equalizer(0, 0);
                mEqualizer.setEnabled(true);
            }
            if (null == mBass) {
                mBass = new BassBoost(0, 0);
                mBass.setEnabled(true);
            }
            if (null == mVirtualizer) {
                mVirtualizer = new Virtualizer(0, 0);
                mVirtualizer.setEnabled(true);
            }
            setDspBassBoost(getDspBassBoost(), false);
            setDspSurround(getDspSurround() ? 10 : 0);
        }
    }

    private void releaseEffects() {
        if (mEqualizer != null) {
            mEqualizer.setEnabled(false);
            mEqualizer.release();
            mEqualizer = null;
        }
        if (mBass != null) {
            mBass.setEnabled(false);
            mBass.release();
            mBass = null;
        }
        if (mVirtualizer != null) {
            mVirtualizer.setEnabled(false);
            mVirtualizer.release();
            mVirtualizer = null;
        }
        Log.i(TAG, "Internal dsp releaseEffects !");
    }

    /**
     * @param {boolean mFadeInOut:是否需要淡入淡出生效音效.}
     * @return ${return_type}
     * @Description:设置内置DSP BAND音效[-1500~1500],因幅度过大会有POPO音,现添加mFadeInOut.
     * @Date: 下午3:11
     * @Author: C.Wong
     **/
    public void setupEqualizer(int[] mData, boolean mFadeInOut) {
        //DSP BandLevelRange [-1500~1500]
        short mBand_0, mBand_1, mBand_2, mBand_3, mBand_4;
        if (mData.length == 32) { // mcx 客户用的32段
            mBand_0 = (short) ((mData[0] + mData[1] + mData[2] + mData[3] + mData[4] + mData[5]) / 4 * 100);
            mBand_1 = (short) ((mData[6] + mData[7] + mData[8] + mData[9] + mData[10] + mData[11]) / 4 * 100);
            mBand_2 = (short) ((mData[12] + mData[13] + mData[14] + mData[15] + mData[16] + mData[17]) / 4 * 100);
            mBand_3 = (short) ((mData[18] + mData[19] + mData[20] + mData[21] + mData[22] + mData[23]) / 4 * 100);
            mBand_4 = (short) ((mData[24] + mData[25] + mData[26] + mData[27] + mData[28] + mData[29] + mData[30] + mData[31]) * 0.1875 * 100);
        } else {
            mBand_0 = (short) ((mData[0] + mData[1] + mData[2]) / 2 * 100);
            mBand_1 = (short) ((mData[3] + mData[4] + mData[5]) / 2 * 100);
            mBand_2 = (short) ((mData[6] + mData[7] + mData[8]) / 2 * 100);
            mBand_3 = (short) ((mData[9] + mData[10] + mData[11]) / 2 * 100);
            mBand_4 = (short) ((mData[12] + mData[13] + mData[14] + mData[15]) * 0.375 * 100);
        }

        if (mEqualizer != null) {
            if (mFadeInOut) {
                effectFadeInOut((short) 0, mEqualizer.getBandLevel((short) 0), mBand_0);
                effectFadeInOut((short) 1, mEqualizer.getBandLevel((short) 1), mBand_1);
                effectFadeInOut((short) 2, mEqualizer.getBandLevel((short) 2), mBand_2);
                effectFadeInOut((short) 3, mEqualizer.getBandLevel((short) 3), mBand_3);
                effectFadeInOut((short) 4, mEqualizer.getBandLevel((short) 4), mBand_4);
            } else {
                mEqualizer.setBandLevel((short) 0, mBand_0);
                mEqualizer.setBandLevel((short) 1, mBand_1);
                mEqualizer.setBandLevel((short) 2, mBand_2);
                mEqualizer.setBandLevel((short) 3, mBand_3);
                mEqualizer.setBandLevel((short) 4, mBand_4);
            }
            if (DEBUG) {
                Log.i(TAG, "Set DSP Equalizer:[" + mBand_0 + "," + mBand_1 + "," +
                        mBand_2 + "," + mBand_3 + "," + mBand_4 + "]");
            }
        }
    }


    public void saveDspBandValue(String value) {
        //只保存DSP Band模式值
        mDspPre.put(STATUS_DSP_BAND, value, true);
    }

    public int[] getDspBandValue() {
        int[] mDspDataVal = new int[EqUtils.DSP_BAND_DEPTH];
        String mAllBandVal = mDspPre.getString(STATUS_DSP_BAND);
        //USER 模式获取保存值
        if (!"".equals(mAllBandVal)) {
            mAllBandVal = mAllBandVal.substring(mAllBandVal.indexOf("[") + 1, mAllBandVal.indexOf("]"));
            String[] mStr = mAllBandVal.split(",");
            for (int i = 0; i < EqUtils.DSP_BAND_DEPTH; i++) {
                //获取各Band值
                mDspDataVal[i] = Integer.valueOf(mStr[i].trim()).intValue();
            }
        } else {
            mDspDataVal = Arrays.copyOf("2".equals(EqUtils.getDspUI()) ? DEF_DSP_BANDS_32[0] : DEF_DSP_BANDS[0], EqUtils.DSP_BAND_DEPTH);
        }
        return mDspDataVal;
    }

    //DSP Power
    public void setDspPower(int checkPower) {
        Log.d(TAG, "setDspPower: checkPower = " + checkPower);
        if (checkPower > 0) {
            mDspPower = true;
            initEffects();
        } else {
            mDspPower = false;
        }
        mDspPre.put(STATUS_DSP_POWER, checkPower, true);
    }

    public boolean getDspPower() {
        return 1 == mDspPre.getInt(STATUS_DSP_POWER, 1);
    }

    //@Commit DSP设置低音.
    public void setDspBassBoost(int mBassBoostVal, boolean mCommit) {
        if (mBass != null) {
            mBass.setStrength((short) (mBassBoostVal * 100));
        }
        if (mCommit) {
            mDspPre.put(STATUS_BASS_BOOST, mBassBoostVal, true);
            //同步保存设置后的低音值到系统属性中，用于配置文件的读取，支持导出导入
            SetupSharedData.getInstance(mContext).setIntValue(EqUtils.HEQ_BASS_BOOST, mBassBoostVal);
        }
        if (DEBUG) {
            Log.i(TAG, "Set Dsp BassBoost:" + mBassBoostVal * 100);
        }
    }

    public int getDspBassBoost() {
        //先获取配置文件配置的值，如果没有，就拿本地的值，本地没有值，就默认为0
        return SetupSharedData.getInstance(mContext).getIntValue(EqUtils.HEQ_BASS_BOOST, mDspPre.getInt(STATUS_BASS_BOOST, 0));
    }

    //DSP Surround
    public void setDspSurround(int mSurround) {
        Log.d(TAG, "setDspSurround: mSurround = " + mSurround + " mVirtualizer is null ? " + (mVirtualizer == null));
        if (mVirtualizer != null) {
            mVirtualizer.setStrength((short) (mSurround * 100));
            mDspPre.put(STATUS_SURROUND, mSurround, true);
            if (DEBUG) {
                Log.i(TAG, "Set Dsp Surround:" + mSurround * 100);
            }
        }
    }

    public boolean getDspSurround() {
        return 0 != mDspPre.getInt(STATUS_SURROUND, 0);
    }

    //@Commit DSP设置用户选择音效.
    public void setDspReverbType(int reverb, int[] mDspVal) {
        Log.i(TAG, "DSP reverb:" + reverb);
        mDspPre.put(STATUS_REVERB_TYPE, reverb, true);
        setupEqualizer(mDspVal, true);
    }

    public int getDspReverbType() {
        return mDspPre.getInt(STATUS_REVERB_TYPE, 0);
    }

    public void startBootDspSetting() {
        Log.i(TAG, "Dsp bootDspSetting start getDspPower = " + getDspPower()
                + " EqUtils.disableInternalDsp() = " + EqUtils.disableInternalDsp()
                + " getDspReverbType = " + getDspReverbType());
        if (getDspPower() && !EqUtils.disableInternalDsp()) {
            if (getDspReverbType() == ConstantEq.EQ_REVERB_USER) {
                setupEqualizer(getDspBandValue(), false);
            } else {
                setupEqualizer(DEF_DSP_BANDS[getDspReverbType()], false);
            }
            setDspBassBoost(getDspBassBoost(), false);
            setDspSurround(getDspSurround() ? 10 : 0);
            Log.i(TAG, "bootDspSetting success getDspBassBoost = " + getDspBassBoost()
                    + " getDspSurround = " + getDspSurround());
        }
    }

    /**
     * @return void
     * @Author C.Wong
     * @Description 处理切换模式时幅度过大POPO音
     * @Date 下午4:05 19-4-2
     * @Param [mSeekBar, from, to]
     **/
    private void effectFadeInOut(short mBand,
                                 final short from, final short to) {
        ValueAnimator animator = ValueAnimator.ofInt(from, to);
        animator.setDuration(1000);
        animator.setInterpolator(new LinearInterpolator());
        animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                int mUpdate = (int) animation.getAnimatedValue();
                if (mEqualizer != null) {
                    mEqualizer.setBandLevel(mBand, (short) mUpdate);
                    if (DEBUG) {
                        Log.i(TAG, "Update band" + mBand + ", To " + to);
                    }

                }
            }
        });

        animator.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animation) {
            }

            @Override
            public void onAnimationEnd(Animator animation) {
                if (mEqualizer != null) {
                    mEqualizer.setBandLevel(mBand, (short) to);
                }
                Log.i(TAG, "AnimationEnd Band" + mBand + ", form " + from + ", To " + to);
                if (!mDspPower && 4 == mBand) {
                    //Dsp Power Off时需动画结束后释放.
                    releaseEffects();
                }
            }

            @Override
            public void onAnimationCancel(Animator animation) {
                Log.i(TAG, "onAnimationCancel" + mBand + ", form " + from + ", To " + to);
                try {
                    if (mEqualizer != null) {
                        mEqualizer.setBandLevel(mBand, (short) to);
                    }
                } catch (Exception ignored) {
                }
            }

            @Override
            public void onAnimationRepeat(Animator animation) {
                Log.i(TAG, "onAnimationRepeat" + mBand + ", form " + from + ", To " + to);
            }
        });
        animator.start();
    }


    /**
     * 设置主页tab栏（dsp）的名称
     *
     * @return
     */
    public String getTabDspName() {
        return SetupSharedData.getInstance(mContext).getStringValue(HEQ_TAB_DSP_NAME);
    }

    /**
     * 是否需要隐藏平衡功能
     *
     * @return
     */
    public boolean hideEqBalance() {
        String hideEqBalance = SetupSharedData.getInstance(mContext).getStringValue(HIDE_EQ_BALANCE);
        return NEED_HIDE_EQ_BALANCE.equals(hideEqBalance);
    }


    /**
     * 是否需要隐藏环绕功能
     *
     * @return
     */
    public boolean hideEqSurround() {
        String hideEqSurround = SetupSharedData.getInstance(mContext).getStringValue(HIDE_EQ_SURROUND);
        return NEED_HIDE_EQ_SURROUND.equals(hideEqSurround);
    }
}
