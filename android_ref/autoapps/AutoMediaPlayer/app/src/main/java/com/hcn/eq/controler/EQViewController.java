package com.hcn.eq.controler;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.util.Log;
import android.view.View;
import android.widget.ImageButton;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.SeekBar;
import android.widget.TextView;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_model.eq.IEqConstant;
import com.hcn.media_model.eq.SharePreferencesTools;
import com.hcn.eq.listener.IEQModeListener;
import com.hcn.media_model.eq.EQMediaController;

/**
 * 音效视图控制器
 * @author 65821
 */
public class EQViewController implements IEqConstant {
    public static final String TAG = "EQViewController";

    public SeekBar mEqSeekBarViewOn60HZ, mEqSeekBarViewOn230HZ,
            mEqSeekBarViewOn910HZ, mEqSeekBarViewOn3600HZ,
            mEqSeekBarViewOn14KHZ, mEqSeekBarViewTreble, mEqSeekBarViewSurround;
    public RadioButton mUser, mNews, mJazz, mCity, mPop, mEle, mClassic,
            mMovie, mRock, mTechno;
    public ImageButton mResetButton;
    public TextView mTextView60HZ, mTextView230HZ, mTextView910HZ, mTextView3600HZ,
            mTextView14kHZ, mTextViewTreble, mTextViewSurround;
    public RadioGroup mRadioGroup;
    public EqReverbGroupListener mEqReverbGroupListener;
    public IEQModeListener mIEQModeListener;
    public EQController mEQController;
    public SharePreferencesTools mSharePreferencesTools;

    // 重置按钮点击监听
    public View.OnClickListener mResetOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            mUser.setChecked(true);

            if (mIEQModeListener != null) {
                mIEQModeListener.onDefault();
            }

            mEqSeekBarViewTreble.setProgress(0);
            mEqSeekBarViewSurround.setProgress(0);
        }
    };

    // seekbar 进度改变监听
    SeekBar.OnSeekBarChangeListener mBandChangeListen = new SeekBar.OnSeekBarChangeListener() {
        @Override
        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
            setSeekbarValue(Integer.parseInt((String) seekBar.getTag()), progress, false);
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
            setSeekbarValue(Integer.parseInt((String) seekBar.getTag()),
                    seekBar.getProgress(), true);
        }
    };

    private Activity mContext;
    private boolean hasInit = false;

    public EQViewController(Activity context) {
        this.mContext = context;

        initView();
        initMode();
        hasInit = true;
    }

    public boolean getInitStatus() {
        return hasInit;
    }

    private void initView() {
        Log.d(TAG, "initView.");

        mRadioGroup = (RadioGroup) mContext.findViewById(R.id.id_reverb_group);
        mUser = (RadioButton) mContext.findViewById(R.id.id_reverb_user);
        mNews = (RadioButton) mContext.findViewById(R.id.id_reverb_news);
        mJazz = (RadioButton) mContext.findViewById(R.id.id_reverb_jazz);
        mCity = (RadioButton) mContext.findViewById(R.id.id_reverb_city);
        mEle = (RadioButton) mContext.findViewById(R.id.id_reverb_electronic);
        mClassic = (RadioButton) mContext.findViewById(R.id.id_reverb_classiz);
        mMovie = (RadioButton) mContext.findViewById(R.id.id_reverb_movie);
        mRock = (RadioButton) mContext.findViewById(R.id.id_reverb_rock);
        mTechno = (RadioButton) mContext.findViewById(R.id.id_reverb_techno);
        mPop = (RadioButton) mContext.findViewById(R.id.id_reverb_pop);

        mResetButton = (ImageButton) mContext.findViewById(R.id.eq_reset);
        mResetButton.setOnClickListener(mResetOnClickListener);

        mEqSeekBarViewOn60HZ = (SeekBar) mContext.findViewById(R.id.one_seek);
        mEqSeekBarViewOn230HZ = (SeekBar) mContext.findViewById(R.id.two_seek);
        mEqSeekBarViewOn910HZ = (SeekBar) mContext.findViewById(R.id.three_seek);
        mEqSeekBarViewOn3600HZ = (SeekBar) mContext.findViewById(R.id.four_seek);
        mEqSeekBarViewOn14KHZ = (SeekBar) mContext.findViewById(R.id.five_seek);
        mEqSeekBarViewTreble = (SeekBar) mContext.findViewById(R.id.six_seek);
        mEqSeekBarViewSurround = (SeekBar) mContext.findViewById(R.id.seven_seek);

        mTextView60HZ = (TextView) mContext.findViewById(R.id.one_value);
        mTextView230HZ = (TextView) mContext.findViewById(R.id.two_value);
        mTextView910HZ = (TextView) mContext.findViewById(R.id.three_value);
        mTextView3600HZ = (TextView) mContext.findViewById(R.id.four_value);
        mTextView14kHZ = (TextView) mContext.findViewById(R.id.five_value);
        mTextViewTreble = (TextView) mContext.findViewById(R.id.six_value);
        mTextViewSurround = (TextView) mContext.findViewById(R.id.seven_value);

        mEqSeekBarViewOn60HZ.setOnSeekBarChangeListener(mBandChangeListen);
        mEqSeekBarViewOn230HZ.setOnSeekBarChangeListener(mBandChangeListen);
        mEqSeekBarViewOn910HZ.setOnSeekBarChangeListener(mBandChangeListen);
        mEqSeekBarViewOn3600HZ.setOnSeekBarChangeListener(mBandChangeListen);
        mEqSeekBarViewOn14KHZ.setOnSeekBarChangeListener(mBandChangeListen);
        mEqSeekBarViewTreble.setOnSeekBarChangeListener(mBandChangeListen);
        mEqSeekBarViewSurround.setOnSeekBarChangeListener(mBandChangeListen);

        mEqSeekBarViewOn60HZ.setMax(20);
        mEqSeekBarViewOn230HZ.setMax(20);
        mEqSeekBarViewOn910HZ.setMax(20);
        mEqSeekBarViewOn3600HZ.setMax(20);
        mEqSeekBarViewOn14KHZ.setMax(20);
        mEqSeekBarViewTreble.setMax(10);
        mEqSeekBarViewSurround.setMax(10);
    }

    public void initMode() {
        Log.d(TAG, "initMode.");

        mEqReverbGroupListener = new EqReverbGroupListener(this);
        mRadioGroup.setOnCheckedChangeListener(mEqReverbGroupListener);

        mEQController = new EQController(this);
        setEQController(mEQController);

        mSharePreferencesTools = SharePreferencesTools.getSharePreferencesTools();
        mSharePreferencesTools.init(mContext.getApplicationContext());

        radioInit();
    }

    @SuppressLint("SetTextI18n")
    public void radioInit() {
        int id = mSharePreferencesTools.getModePreference();

        switch (id) {
            case USER:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onUser(mSharePreferencesTools.getEQFirstPreference(),
                            mSharePreferencesTools.getEQTwoPreference(),
                            mSharePreferencesTools.getEQThreePreference(),
                            mSharePreferencesTools.getEQFourPreference(),
                            mSharePreferencesTools.getEQFivePreference());
                }
                mUser.setChecked(true);
                break;
            case DANCE:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onCity();
                }
                mCity.setChecked(true);
                break;
            case JAZZ:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onJazz();
                }
                mJazz.setChecked(true);
                break;
            case LIFE:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onNews();
                }
                mNews.setChecked(true);
                break;
            case POP:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onPop();
                }
                mPop.setChecked(true);
                break;
            case ELE:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onElectronic();
                }
                mEle.setChecked(true);
                break;
            case CLASSIC:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onClassic();
                }
                mClassic.setChecked(true);
                break;
            case RB:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onMovie();
                }
                mMovie.setChecked(true);
                break;
            case ROCK:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onRock();
                }
                mRock.setChecked(true);
                break;
            case PURE_MUSIC:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onTechno();
                }
                mTechno.setChecked(true);
                break;

            default:
                break;
        }

        int progress = mSharePreferencesTools.getEQSixPreference();
        mEqSeekBarViewTreble.setProgress(progress);
        if (progress > 0) {
            mTextViewTreble.setText("+" + progress);
        } else {
            mTextViewTreble.setText(String.valueOf(progress));
        }

        progress = mSharePreferencesTools.getEQSevenPreference();
        mEqSeekBarViewSurround.setProgress(progress);
        if (progress > 0) {
            mTextViewSurround.setText("+" + progress);
        } else {
            mTextViewSurround.setText(String.valueOf(progress));
        }
    }

    public void start() {
        Log.d(TAG, "start");
    }

    public void stop() {
    }

    public void release() {
    }

    //@Commit HCN2019 applay to SWC EQ.
    public void onClickPreSet() {
        int mCurIndex = mRadioGroup.indexOfChild(
                mRadioGroup.findViewById(mRadioGroup.getCheckedRadioButtonId()));
        if (mCurIndex >= mRadioGroup.getChildCount() - 1) {
            mCurIndex = 0;
        } else {
            mCurIndex++;
        }

        RadioButton mButton = (RadioButton) (mRadioGroup.getChildAt(mCurIndex));
        mButton.setChecked(true);
    }

    /**
     * 接口封装
     * <pre>
     *    用来做条件过滤;
     *    未初始化好，不设置调用实际接口；
     * </pre>
     *
     * @param uuid
     * @param num
     * @param sessionId
     */
    private void setupEqualizer(short uuid, short num, int sessionId) {
        final EQMediaController eqc = EQMediaController.instance();
        if (eqc != null && getInitStatus()) {
            eqc.setupEqualizer(uuid, num, sessionId);
        }
    }

    /**
     * 接口封装
     * <pre>
     *    用来做条件过滤;
     *    未初始化好，不设置调用实际接口；
     * </pre>
     *
     * @param num
     * @param sessionId
     */
    private void setupBassBoost(short num, int sessionId) {
        final EQMediaController eqc = EQMediaController.instance();
        if (eqc != null && getInitStatus()) {
            eqc.setupBassBoost(num, sessionId);
        }
    }

    /**
     * 接口封装
     * <pre>
     *    用来做条件过滤;
     *    未初始化好，不设置调用实际接口；
     * </pre>
     *
     * @param num
     * @param sessionId
     */
    private void setupVirtBoost(short num, int sessionId) {
        final EQMediaController eqc = EQMediaController.instance();
        if (eqc != null && getInitStatus()) {
            eqc.setupVirtBoost(num, sessionId);
        }
    }

    private void setSeekbarValue(int mIndexBand, int mvalue, boolean mSave) {
        EQMediaController controller = EQMediaController.instance();

        short mSureBand = (short) (mvalue - 10);
        if (mIndexBand < UID_BAND_BASS) {
            setTextValue(mIndexBand, mSureBand);
            setupEqualizer((short) mIndexBand,
                    (short) (mSureBand),
                    controller.getAudioSessionId());
        } else if (mIndexBand == UID_BAND_BASS) {
            setTextValue(UID_BAND_BASS, mvalue);
            if (mEQController != null) {
                setupBassBoost((short) (mvalue),
                        controller.getAudioSessionId());
            }
            if (mSave) {
                mSharePreferencesTools.setSixEQPreference(mvalue);
                setupBassBoost((short) (mvalue),
                        controller.getAudioSessionId());
            }
        } else if (mIndexBand == uid_band_virt) {
            setTextValue(uid_band_virt, mvalue);
            if (mEQController != null) {
                setupVirtBoost((short) (mvalue),
                        controller.getAudioSessionId());
            }
            if (mSave) {
                mSharePreferencesTools.setSevenEQPreference(mvalue);
                setupVirtBoost((short) (mvalue),
                        controller.getAudioSessionId());
            }
        }

        // Band Stop 时保存数值
        if (mSave && mUser.isChecked()) {
            switch (mIndexBand) {
                case UID_BAND_60_HZ:
                    setupEqualizer(UID_BAND_60_HZ,
                            (short) (mSureBand),
                            controller.getAudioSessionId());
                    mSharePreferencesTools.setFirstEQPreference(mvalue);
                    break;
                case UID_BAND_230_HZ:
                    setupEqualizer(UID_BAND_230_HZ,
                            (short) (mSureBand),
                            controller.getAudioSessionId());
                    mSharePreferencesTools.setTwoEQPreference(mvalue);
                    break;
                case UID_BAND_910_HZ:
                    setupEqualizer(UID_BAND_910_HZ,
                            (short) (mSureBand),
                            controller.getAudioSessionId());
                    mSharePreferencesTools.setThreeEQPreference(mvalue);
                    break;
                case UID_BAND_3600_HZ:
                    setupEqualizer(UID_BAND_3600_HZ,
                            (short) (mSureBand),
                            controller.getAudioSessionId());
                    mSharePreferencesTools.setFourEQPreference(mvalue);
                    break;
                case UID_BAND_14K_HZ:
                    setupEqualizer(UID_BAND_14K_HZ,
                            (short) (mSureBand),
                            controller.getAudioSessionId());
                    mSharePreferencesTools.setFiveEQPreference(mvalue);
                    break;
                default:
                    break;
            }
        }
    }

    public void setEQController(IEQModeListener modeListener) {
        this.mIEQModeListener = modeListener;
    }

    @SuppressLint("NonConstantResourceId")
    public void onEqReverbChecked(int id) {
        if (!getInitStatus()) {
            return; // 没初始化完成，不要去调用 EQ 的接口，避免不必要的声音卡顿问题；
        }

        LogUtil.e(TAG, "onEqReverbChecked:" + id + "   mIEQModeListener: " + mIEQModeListener);
        switch (id) {
            case R.id.id_reverb_user:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onUser(mSharePreferencesTools.getEQFirstPreference(),
                            mSharePreferencesTools.getEQTwoPreference(),
                            mSharePreferencesTools.getEQThreePreference(),
                            mSharePreferencesTools.getEQFourPreference(),
                            mSharePreferencesTools.getEQFivePreference());
                }
                break;
            case R.id.id_reverb_jazz:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onJazz();
                }
                break;
            case R.id.id_reverb_pop:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onPop();
                }
                break;
            case R.id.id_reverb_classiz:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onClassic();
                }
                break;
            case R.id.id_reverb_rock:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onRock();
                }
                break;
            case R.id.id_reverb_news:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onNews();
                }
                break;
            case R.id.id_reverb_city:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onCity();
                }
                break;
            case R.id.id_reverb_movie:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onMovie();
                }
                break;
            case R.id.id_reverb_electronic:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onElectronic();
                }
                break;
            case R.id.id_reverb_techno:
                if (mIEQModeListener != null) {
                    mIEQModeListener.onTechno();
                }
                break;
            default:
                break;
        }
    }

    private void setTextValue(int sourceId, int values) {
        String flag = "";
        //Band -10~10
        switch (sourceId) {
            case UID_BAND_60_HZ:
                mTextView60HZ.setText(String.valueOf(values));
                break;
            case UID_BAND_230_HZ:
                mTextView230HZ.setText(String.valueOf(values));
                break;
            case UID_BAND_910_HZ:
                mTextView910HZ.setText(String.valueOf(values));
                break;
            case UID_BAND_3600_HZ:
                mTextView3600HZ.setText(String.valueOf(values));
                break;
            case UID_BAND_14K_HZ:
                mTextView14kHZ.setText(String.valueOf(values));
                break;
            case UID_BAND_BASS:
                mTextViewTreble.setText(String.valueOf(values));
                break;
            case uid_band_virt:
                mTextViewSurround.setText(String.valueOf(values));
            default:
                break;
        }
    }

    public static final class EqReverbGroupListener implements
            OnCheckedChangeListener {

        private EQViewController mEQViewController = null;

        public EqReverbGroupListener(EQViewController eqViewController) {
            if (null != eqViewController) {
                mEQViewController = eqViewController;
            }
        }

        @Override
        public void onCheckedChanged(RadioGroup group, int checkedId) {
            LogUtil.e(EQViewController.TAG,
                    "onCheckedChanged: checkedId = " + Integer.toHexString(checkedId));
            mEQViewController.onEqReverbChecked(checkedId);
        }
    }
}
