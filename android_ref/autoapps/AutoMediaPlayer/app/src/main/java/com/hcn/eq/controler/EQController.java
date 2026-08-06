package com.hcn.eq.controler;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.annotation.SuppressLint;
import android.view.animation.LinearInterpolator;
import android.widget.SeekBar;
import android.widget.TextView;

import com.hcn.media_model.eq.IEqConstant;
import com.hcn.eq.listener.IEQModeListener;

import io.vov.vitamio.utils.Log;

@SuppressLint("NewApi")
public class EQController implements IEQModeListener, IEqConstant {
    private static final String TAG = EQController.class.getSimpleName();

    private static final float aphla = 0.9f;
    private static final float Noaphla = 1.0f;
    private static final int DEFAULT_BANDVAL = 10;

    private final EQViewController mEQViewController;
    private final boolean enable = true;
    private final boolean disable = false;

    public EQController(EQViewController eqViewController) {
        mEQViewController = eqViewController;
    }

    @Override
    public void onDefault() {
        Log.d(TAG, "onDefault.");

        setData(mEQViewController.mEqSeekBarViewOn60HZ, Noaphla, enable, DEFAULT_BANDVAL,
                mEQViewController.mTextView60HZ);
        setData(mEQViewController.mEqSeekBarViewOn230HZ, Noaphla, enable, DEFAULT_BANDVAL,
                mEQViewController.mTextView230HZ);
        setData(mEQViewController.mEqSeekBarViewOn910HZ, Noaphla, enable, DEFAULT_BANDVAL,
                mEQViewController.mTextView910HZ);
        setData(mEQViewController.mEqSeekBarViewOn3600HZ, Noaphla, enable, DEFAULT_BANDVAL,
                mEQViewController.mTextView3600HZ);
        setData(mEQViewController.mEqSeekBarViewOn14KHZ, Noaphla, enable, DEFAULT_BANDVAL,
                mEQViewController.mTextView14kHZ);

        mEQViewController.mSharePreferencesTools.setFirstEQPreference(DEFAULT_BANDVAL);
        mEQViewController.mSharePreferencesTools.setTwoEQPreference(DEFAULT_BANDVAL);
        mEQViewController.mSharePreferencesTools.setThreeEQPreference(DEFAULT_BANDVAL);
        mEQViewController.mSharePreferencesTools.setFourEQPreference(DEFAULT_BANDVAL);
        mEQViewController.mSharePreferencesTools.setFiveEQPreference(DEFAULT_BANDVAL);
        mEQViewController.mSharePreferencesTools.setSixEQPreference(0);
        mEQViewController.mSharePreferencesTools.setSevenEQPreference(0);
        mEQViewController.mSharePreferencesTools.setModePreference(USER);
    }

    @Override
    public void onUser(int m60hz, int m230hz, int m910hz, int m3600hz, int m14khz) {
        Log.d(TAG, "onUser.");

        setData(mEQViewController.mEqSeekBarViewOn60HZ, Noaphla, enable, m60hz,
                mEQViewController.mTextView60HZ);
        setData(mEQViewController.mEqSeekBarViewOn230HZ, Noaphla, enable, m230hz,
                mEQViewController.mTextView230HZ);
        setData(mEQViewController.mEqSeekBarViewOn910HZ, Noaphla, enable, m910hz,
                mEQViewController.mTextView910HZ);
        setData(mEQViewController.mEqSeekBarViewOn3600HZ, Noaphla, enable, m3600hz,
                mEQViewController.mTextView3600HZ);
        setData(mEQViewController.mEqSeekBarViewOn14KHZ, Noaphla, enable, m14khz,
                mEQViewController.mTextView14kHZ);
        mEQViewController.mSharePreferencesTools.setModePreference(USER);
    }

    @Override
    public void onNews() {//life
        Log.d(TAG, "onNews.");

        setData(mEQViewController.mEqSeekBarViewOn60HZ, aphla, disable, 20,
                mEQViewController.mTextView60HZ);
        setData(mEQViewController.mEqSeekBarViewOn230HZ, aphla, disable, 12,
                mEQViewController.mTextView230HZ);
        setData(mEQViewController.mEqSeekBarViewOn910HZ, aphla, disable, 10,
                mEQViewController.mTextView910HZ);
        setData(mEQViewController.mEqSeekBarViewOn3600HZ, aphla, disable, 20,
                mEQViewController.mTextView3600HZ);
        setData(mEQViewController.mEqSeekBarViewOn14KHZ, aphla, disable, 14,
                mEQViewController.mTextView14kHZ);
        mEQViewController.mSharePreferencesTools.setModePreference(LIFE);
    }

    @Override
    public void onJazz() {
        Log.d(TAG, "onJazz.");

        setData(mEQViewController.mEqSeekBarViewOn60HZ, aphla, disable, 14,
                mEQViewController.mTextView60HZ);
        setData(mEQViewController.mEqSeekBarViewOn230HZ, aphla, disable, 15,
                mEQViewController.mTextView230HZ);
        setData(mEQViewController.mEqSeekBarViewOn910HZ, aphla, disable, 9,
                mEQViewController.mTextView910HZ);
        setData(mEQViewController.mEqSeekBarViewOn3600HZ, aphla, disable, 6,
                mEQViewController.mTextView3600HZ);
        setData(mEQViewController.mEqSeekBarViewOn14KHZ, aphla, disable, 2,
                mEQViewController.mTextView14kHZ);
        mEQViewController.mSharePreferencesTools.setModePreference(JAZZ);
    }

    @Override
    public void onCity() {//dance
        Log.d(TAG, "onCity.");

        setData(mEQViewController.mEqSeekBarViewOn60HZ, aphla, disable, 19,
                mEQViewController.mTextView60HZ);
        setData(mEQViewController.mEqSeekBarViewOn230HZ, aphla, disable, 14,
                mEQViewController.mTextView230HZ);
        setData(mEQViewController.mEqSeekBarViewOn910HZ, aphla, disable, 7,
                mEQViewController.mTextView910HZ);
        setData(mEQViewController.mEqSeekBarViewOn3600HZ, aphla, disable, 7,
                mEQViewController.mTextView3600HZ);
        setData(mEQViewController.mEqSeekBarViewOn14KHZ, aphla, disable, 10,
                mEQViewController.mTextView14kHZ);
        mEQViewController.mSharePreferencesTools.setModePreference(DANCE);
    }

    @Override
    public void onPop() {
        Log.d(TAG, "onPop.");

        setData(mEQViewController.mEqSeekBarViewOn60HZ, aphla, disable, 10,
                mEQViewController.mTextView60HZ);
        setData(mEQViewController.mEqSeekBarViewOn230HZ, aphla, disable, 18,
                mEQViewController.mTextView230HZ);
        setData(mEQViewController.mEqSeekBarViewOn910HZ, aphla, disable, 18,
                mEQViewController.mTextView910HZ);
        setData(mEQViewController.mEqSeekBarViewOn3600HZ, aphla, disable, 12,
                mEQViewController.mTextView3600HZ);
        setData(mEQViewController.mEqSeekBarViewOn14KHZ, aphla, disable, 6,
                mEQViewController.mTextView14kHZ);
        mEQViewController.mSharePreferencesTools.setModePreference(POP);
    }

    @Override
    public void onElectronic() {
        Log.d(TAG, "onElectronic.");

        setData(mEQViewController.mEqSeekBarViewOn60HZ, aphla, disable, 16,
                mEQViewController.mTextView60HZ);
        setData(mEQViewController.mEqSeekBarViewOn230HZ, aphla, disable, 12,
                mEQViewController.mTextView230HZ);
        setData(mEQViewController.mEqSeekBarViewOn910HZ, aphla, disable, 14,
                mEQViewController.mTextView910HZ);
        setData(mEQViewController.mEqSeekBarViewOn3600HZ, aphla, disable, 10,
                mEQViewController.mTextView3600HZ);
        setData(mEQViewController.mEqSeekBarViewOn14KHZ, aphla, disable, 6,
                mEQViewController.mTextView14kHZ);
        mEQViewController.mSharePreferencesTools.setModePreference(ELE);
    }

    @Override
    public void onClassic() {
        Log.d(TAG, "onClassic.");

        setData(mEQViewController.mEqSeekBarViewOn60HZ, aphla, disable, 10,
                mEQViewController.mTextView60HZ);
        setData(mEQViewController.mEqSeekBarViewOn230HZ, aphla, disable, 10,
                mEQViewController.mTextView230HZ);
        setData(mEQViewController.mEqSeekBarViewOn910HZ, aphla, disable, 10,
                mEQViewController.mTextView910HZ);
        setData(mEQViewController.mEqSeekBarViewOn3600HZ, aphla, disable, 8,
                mEQViewController.mTextView3600HZ);
        setData(mEQViewController.mEqSeekBarViewOn14KHZ, aphla, disable, 5,
                mEQViewController.mTextView14kHZ);
        mEQViewController.mSharePreferencesTools.setModePreference(CLASSIC);
    }

    @Override
    public void onMovie() {//R&B
        Log.d(TAG, "onMovie.");

        setData(mEQViewController.mEqSeekBarViewOn60HZ, aphla, disable, 14,
                mEQViewController.mTextView60HZ);
        setData(mEQViewController.mEqSeekBarViewOn230HZ, aphla, disable, 13,
                mEQViewController.mTextView230HZ);
        setData(mEQViewController.mEqSeekBarViewOn910HZ, aphla, disable, 10,
                mEQViewController.mTextView910HZ);
        setData(mEQViewController.mEqSeekBarViewOn3600HZ, aphla, disable, 15,
                mEQViewController.mTextView3600HZ);
        setData(mEQViewController.mEqSeekBarViewOn14KHZ, aphla, disable, 15,
                mEQViewController.mTextView14kHZ);
        mEQViewController.mSharePreferencesTools.setModePreference(RB);
    }

    @Override
    public void onRock() {
        Log.d(TAG, "onRock.");

        setData(mEQViewController.mEqSeekBarViewOn60HZ, aphla, disable, 18,
                mEQViewController.mTextView60HZ);
        setData(mEQViewController.mEqSeekBarViewOn230HZ, aphla, disable, 11,
                mEQViewController.mTextView230HZ);
        setData(mEQViewController.mEqSeekBarViewOn910HZ, aphla, disable, 10,
                mEQViewController.mTextView910HZ);
        setData(mEQViewController.mEqSeekBarViewOn3600HZ, aphla, disable, 14,
                mEQViewController.mTextView3600HZ);
        setData(mEQViewController.mEqSeekBarViewOn14KHZ, aphla, disable, 20,
                mEQViewController.mTextView14kHZ);
        mEQViewController.mSharePreferencesTools.setModePreference(ROCK);
    }

    @Override
    public void onTechno() {
        Log.d(TAG, "onTechno.");

        setData(mEQViewController.mEqSeekBarViewOn60HZ, aphla, disable, 16,
                mEQViewController.mTextView60HZ);
        setData(mEQViewController.mEqSeekBarViewOn230HZ, aphla, disable, 14,
                mEQViewController.mTextView230HZ);
        setData(mEQViewController.mEqSeekBarViewOn910HZ, aphla, disable, 13,
                mEQViewController.mTextView910HZ);
        setData(mEQViewController.mEqSeekBarViewOn3600HZ, aphla, disable, 15,
                mEQViewController.mTextView3600HZ);
        setData(mEQViewController.mEqSeekBarViewOn14KHZ, aphla, disable, 15,
                mEQViewController.mTextView14kHZ);
        mEQViewController.mSharePreferencesTools.setModePreference(PURE_MUSIC);
    }

    public void setData(SeekBar eqSeekBarView, float alpha, boolean enable, int num, TextView view) {
        eqSeekBarView.setAlpha(alpha);
        eqSeekBarView.setPressed(enable);
        eqSeekBarView.setEnabled(enable);

        //第一次进入不用处理缓升缓降
        if (!mEQViewController.getInitStatus()) {
            eqSeekBarView.setProgress(num);
        } else {
            volumeGradient(eqSeekBarView, eqSeekBarView.getProgress(), num);
        }
    }

    /**
     * @return void
     * @Author C.Wong
     * @Description 处理切换模式时幅度过大POPO音
     * @Date 下午4:05 19-4-2
     * @Param [mSeekBar, from, to]
     **/
    private void volumeGradient(final SeekBar mSeekBar,
            final int from, final int to) {
        ValueAnimator animator = ValueAnimator.ofInt(from, to);
        animator.setDuration(500);
        animator.setInterpolator(new LinearInterpolator());
        animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                mSeekBar.setProgress((int) animation.getAnimatedValue());
            }
        });

        animator.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationStart(Animator animation) {
            }

            @Override
            public void onAnimationEnd(Animator animation) {
                mSeekBar.setProgress(to);
            }

            @Override
            public void onAnimationCancel(Animator animation) {
                try {
                    mSeekBar.setProgress(to);
                } catch (Exception ignored) {
                }
            }

            @Override
            public void onAnimationRepeat(Animator animation) {
            }
        });

        animator.start();
    }
}
