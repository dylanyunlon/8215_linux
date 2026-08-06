package com.hcn.media_model.eq;

import android.annotation.SuppressLint;
import android.content.Context;
import android.media.audiofx.BassBoost;
import android.media.audiofx.Equalizer;
import android.media.audiofx.Virtualizer;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import androidx.annotation.NonNull;

import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_model.base.ILocalzModel;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.Objects;

public class EQMediaController implements EqChangeListener {
    private final static String TAG = "EQMediaController";

    private final static int MSG_INIT_EFFECTS = 1;
    private final static int MSG_INIT_BASSBOOT = 2;

    private static EQMediaController mInstance;

    private final Reference<Context> mContextRef;
    private final ILocalzModel mLocalzModel;

    private Equalizer mEqualizer;
    private BassBoost mBassBoost;
    private Virtualizer mVirtualizer;
    private int mTempBass = 0;
    private SharePreferencesTools mTools;
    private int mCurrentSession = -1;

    @SuppressLint("HandlerLeak")
    private final Handler H = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_INIT_EFFECTS:
                    setupEffects();
                    removeMessages(MSG_INIT_BASSBOOT);
                    mTempBass = 0;
                    sendEmptyMessage(MSG_INIT_BASSBOOT);
                    break;

                case MSG_INIT_BASSBOOT:
                    if (mTempBass > mTools.getBassPreference()) {
                        return;
                    }
                    if (mBassBoost != null) {
                        mBassBoost.setStrength((short) (mTempBass * 100));
                    }
                    removeMessages(MSG_INIT_BASSBOOT);
                    mTempBass++;
                    sendEmptyMessageDelayed(MSG_INIT_BASSBOOT, 100);
                    break;

                default:
                    break;
            }
        }
    };

    /** 唯一实例对象/需要先初始化 **/
    public static EQMediaController instance() {
        if (Objects.isNull(mInstance)) {
            throw new RuntimeException(
                    "Please initialize [EQMediaController] Object!");
        }

        return mInstance;
    }

    /**
     * 初始化当前实例
     *
     * @param context 上下文环境
     * @param localzModel 主业务模型
     */
    public static void init(@NonNull Context context, @NonNull ILocalzModel localzModel) {
        if (Objects.isNull(mInstance)) {
            mInstance = new EQMediaController(context, localzModel);
        } else {
            throw new RuntimeException(
                    "[EQMediaController] already initialized!");
        }
    }

    /** @see EQMediaController#init(Context, ILocalzModel) **/
    private EQMediaController(@NonNull Context context, @NonNull ILocalzModel localzModel) {
        super();

        mContextRef = new WeakReference<>(context);
        mLocalzModel = localzModel;
    }

    public void setupEqualizer(short uuid, short num, int sessionId) {
        LogUtil.i(TAG, "setupEqualizer: sessionId = " + sessionId + ", index = " + uuid);

        if (mCurrentSession != sessionId) {
            reInitEffects(sessionId);
        }

        mEqualizer.setBandLevel(uuid, (short) (num * (short) 100));
    }

    public void setupBassBoost(short num, int sessionId) {
        LogUtil.i(TAG, "setupBassBoost: sessionId = " + sessionId + ", num = " + num);

        if (mCurrentSession != sessionId) {
            reInitEffects(sessionId);
        }

        mBassBoost.setStrength((short) (num * (short) 100));
        if (mTools != null) {
            mTools.setBassPreference(num);
        }
    }

    public void setupVirtBoost(short num, int sessionId) {
        final int sesid = mLocalzModel.getAudioSessionId();
        LogUtil.i(TAG, "setupVirtBoost: sessionId = " + sessionId + ", num = " + num);

        if (mCurrentSession != sessionId) {
            reInitEffects(sessionId);
        }
        mVirtualizer.setStrength((short) (num * (short) 100));
        if (mTools != null) {
            mTools.setVirtPreference(num);
        }
    }

    public void onPauses() {
        releaseEffects();
    }

    public void releaseEffects() {
        if (mEqualizer != null) {
            mEqualizer.release();
        }

        if (mBassBoost != null) {
            mBassBoost.release();
        }

        if (mVirtualizer != null) {
            mVirtualizer.release();
        }
    }

    private void reInitEffects(int sessionId) {
        mEqualizer = new Equalizer(0, sessionId);
        mVirtualizer = new Virtualizer(0, sessionId);
        mBassBoost = new BassBoost(0, sessionId);

        mCurrentSession = sessionId;
        mVirtualizer.setEnabled(true);
        mBassBoost.setEnabled(true);
        mEqualizer.setEnabled(true);
    }

    private void setupEffects() {
        if (mTools == null) {
            return;
        }

        Log.d(TAG, "setupEffects");

        int mode = mTools.getModePreference();
        int id = mLocalzModel.getAudioSessionId();
        if (id != mCurrentSession) {
            reInitEffects(id);
        }

        //Set user basss
        //setupBassBoost(((short) (mTools.getBassPreference())), id);

        //Set user surround
        setupVirtBoost(((short) (mTools.getVirtPreference())), id);

        switch (mode) {
            case IEqConstant.USER:
                setupEqualizer((short) 0, (short) ((mTools.getEQFirstPreference() - 10)), id);
                setupEqualizer((short) 1, (short) ((mTools.getEQTwoPreference() - 10)), id);
                setupEqualizer((short) 2, (short) ((mTools.getEQThreePreference() - 10)), id);
                setupEqualizer((short) 3, (short) ((mTools.getEQFourPreference() - 10)), id);
                setupEqualizer((short) 4, (short) ((mTools.getEQFivePreference() - 10)), id);
                break;
            case IEqConstant.LIFE:
                setupEqualizer((short) 0, (short) (10), id);
                setupEqualizer((short) 1, (short) (2), id);
                setupEqualizer((short) 2, (short) 0, id);
                setupEqualizer((short) 3, (short) (10), id);
                setupEqualizer((short) 4, (short) (4), id);
                break;
            case IEqConstant.JAZZ:
                setupEqualizer((short) 0, (short) (4), id);
                setupEqualizer((short) 1, (short) (5), id);
                setupEqualizer((short) 2, (short) (-1), id);
                setupEqualizer((short) 3, (short) (-4), id);
                setupEqualizer((short) 4, (short) (-8), id);
                break;
            case IEqConstant.DANCE:
                setupEqualizer((short) 0, (short) (9), id);
                setupEqualizer((short) 1, (short) (4), id);
                setupEqualizer((short) 2, (short) (-3), id);
                setupEqualizer((short) 3, (short) (-3), id);
                setupEqualizer((short) 4, (short) (0), id);
                break;
            case IEqConstant.POP:
                setupEqualizer((short) 0, (short) (0), id);
                setupEqualizer((short) 1, (short) (8), id);
                setupEqualizer((short) 2, (short) (8), id);
                setupEqualizer((short) 3, (short) (2), id);
                setupEqualizer((short) 4, (short) (-4), id);
                break;
            case IEqConstant.ELE:
                setupEqualizer((short) 0, (short) (6), id);
                setupEqualizer((short) 1, (short) (2), id);
                setupEqualizer((short) 2, (short) (4), id);
                setupEqualizer((short) 3, (short) (0), id);
                setupEqualizer((short) 4, (short) (-4), id);
                break;
            case IEqConstant.CLASSIC:
                setupEqualizer((short) 0, (short) (0), id);
                setupEqualizer((short) 1, (short) (0), id);
                setupEqualizer((short) 2, (short) (0), id);
                setupEqualizer((short) 3, (short) (-2), id);
                setupEqualizer((short) 4, (short) (-5), id);
                break;
            case IEqConstant.RB:
                setupEqualizer((short) 0, (short) (4), id);
                setupEqualizer((short) 1, (short) (3), id);
                setupEqualizer((short) 2, (short) (0), id);
                setupEqualizer((short) 3, (short) (5), id);
                setupEqualizer((short) 4, (short) (5), id);
                break;
            case IEqConstant.ROCK:
                setupEqualizer((short) 0, (short) (8), id);
                setupEqualizer((short) 1, (short) (1), id);
                setupEqualizer((short) 2, (short) (0), id);
                setupEqualizer((short) 3, (short) (4), id);
                setupEqualizer((short) 4, (short) (10), id);
                break;
            case IEqConstant.PURE_MUSIC:
                setupEqualizer((short) 0, (short) (6), id);
                setupEqualizer((short) 1, (short) (4), id);
                setupEqualizer((short) 2, (short) (3), id);
                setupEqualizer((short) 3, (short) (5), id);
                setupEqualizer((short) 4, (short) (5), id);
                break;
            default:
                break;
        }
    }

    @Override
    public void onEqChange(SharePreferencesTools s) {
        mTools = s;
        int mode = s.getModePreference();
        int id = mLocalzModel.getAudioSessionId();
        LogUtil.i(TAG, "onEQChange: sessionId = " + id + " " + mode);

        // [释放资源]
        releaseEffects();

        H.removeMessages(MSG_INIT_BASSBOOT);
        H.removeMessages(MSG_INIT_EFFECTS);
        H.sendEmptyMessageDelayed(MSG_INIT_EFFECTS, 500);
    }

    public int getAudioSessionId() {
        return mLocalzModel.getAudioSessionId();
    }
}
