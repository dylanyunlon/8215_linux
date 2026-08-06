package com.hcn.media.music.mcc201;

import static android.carsource.McuConstant.K_EQ;

import android.annotation.SuppressLint;
import android.carsource.McuConstant;
import android.carsource.McuManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Rect;
import android.media.AudioManager;
import android.media.audiofx.AudioEffect;
import android.media.audiofx.Visualizer;
import android.media.audiofx.Visualizer.OnDataCaptureListener;
import android.os.Bundle;
import android.sourceservice.ExtAudioMuxer;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import androidx.annotation.NonNull;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media.R3;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_view.widget.VerticalSeekBar;
import com.hcn.mediaservice.data.MediaTimeInfo;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.IMediaEventListener;
import com.hcn.media.local.utils.HFuncUtils;
import com.hcn.media_view.lyrics.LyricsView.OnSeekToListener;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_view.HTextView;
import com.hcn.media_view.ScrollableSeekBar;

import java.util.Locale;

@SuppressWarnings("deprecation")
public class Mcc201MusicInfoFragment extends MediaFragment
        implements OnClickListener, OnDataCaptureListener,
        OnSeekToListener, OnTouchListener {
    private final static String FRAGMENT_NAME = "music-info-mcc201";
    private final static String TAG = Mcc201MusicInfoFragment.class.getSimpleName();

    private static final int mSeekBarMaxValue = 1000;
    private static final String VOLUME_CHANGED_ACTION = "android.media.VOLUME_CHANGED_ACTION";
    private static final String EXTRA_VOLUME_STREAM_TYPE = "android.media.EXTRA_VOLUME_STREAM_TYPE";

    private boolean mInitView = false;

    private ImageButton mBtnPlay = null;
    private ImageButton mBtnRepeatMode = null;
    private ScrollableSeekBar mSeekbarProgress = null;
    private VerticalSeekBar mVolumeSeekBar = null;
    private TextView m_tvCurrentTime = null;
    private TextView m_tvTotalTime = null;
    private ImageButton btnFavor = null;

    private ImageView m_ivMusicImage = null;
    private HTextView m_tvTitle = null;
    private HTextView m_tvArtist = null;
    private HTextView m_tvMusicTitle = null;
    private HTextView m_tvVolumeValue = null;

    private boolean mSeekbarOperate = false;
    private HTextView mTvResource = null;
    private boolean isHandleFavorComplete = true;
    private ImageButton btnVolume = null;

    private AudioManager mAudioManager;

    /**
     * 广播接收器
     * <p> 处理音量相关状态广播: 静音、音量改变等
     */
    private final BroadcastReceiver receiver = new BroadcastReceiver() {

        @SuppressLint("SetTextI18n")
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if ("android.media.STREAM_MUTE_CHANGED_ACTION".equals(action)) {
                if (mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC) == 0) {
                    btnVolume.setImageResource(R.drawable.mcc201_music_mute);
                } else {
                    btnVolume.setImageResource(R.drawable.mcc201_music_volume);
                }
            } else if (VOLUME_CHANGED_ACTION.equals(action)) {
                int mVolType = intent.getIntExtra(EXTRA_VOLUME_STREAM_TYPE, -1);
                if (mVolType == AudioManager.STREAM_MUSIC) {
                    if (null != mVolumeSeekBar) {
                        mVolumeSeekBar.setProgress(
                                mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC));
                    }
                    if (null != m_tvVolumeValue) {
                        m_tvVolumeValue.setText(
                                mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC) + "");
                    }
                }
            }
        }
    };

    public Mcc201MusicInfoFragment(IMediaEventListener listener) {
        super(FRAGMENT_NAME);

        mListener = listener;
    }

    @Override
    public void onAttach(@NonNull Context context) {
        super.onAttach(context);
        mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.mcc201_fragment_musicinfo, container, false);
        Log.d(TAG, "onCreateView: view = " + view);
        Log.d(TAG, "onCreateView: layout = " + R.layout.mcc201_fragment_musicinfo);

        initView(view);
        mInitView = true;
        return view;
    }

    private void initView(View view) {
        view.findViewById(R.id.btnPlay).setOnClickListener(this);
        view.findViewById(R.id.btnPrev).setOnClickListener(this);
        view.findViewById(R.id.btnNext).setOnClickListener(this);
        view.findViewById(R.id.btnRepeatMode).setOnClickListener(this);
        view.findViewById(R.id.btnMenu).setOnClickListener(this);
        view.findViewById(R.id.btnEQ).setOnClickListener(this);
        view.findViewById(R.id.btnHome).setOnClickListener(this);
        view.findViewById(R.id.btnBack).setOnClickListener(this);

        btnVolume = view.findViewById(R.id.btnVolume);
        btnVolume.setOnClickListener(this);
        mBtnPlay = view.findViewById(R.id.btnPlay);
        mBtnRepeatMode = view.findViewById(R.id.btnRepeatMode);
        mTvResource = view.findViewById(R.id.tvRes);

        m_tvCurrentTime = view.findViewById(R.id.tvCurrentTime);
        m_tvTotalTime = view.findViewById(R.id.tvTotalTime);
        mSeekbarProgress = view.findViewById(R.id.seekbar_progress);
        mVolumeSeekBar = view.findViewById(R.id.seekbar_volume);
        m_tvVolumeValue = view.findViewById(R.id.tvVolumeValue);

        m_ivMusicImage = view.findViewById(R.id.ivMusicImage);
        btnFavor = view.findViewById(R.id.btnFavor);
        btnFavor.setOnClickListener(this);
        m_tvTitle = view.findViewById(R.id.tvTitle);
        m_tvArtist = view.findViewById(R.id.tvArtist);

        m_tvMusicTitle = view.findViewById(R.id.tvMusicTitle);

        m_tvTitle.setFilterTouchesWhenObscured(true);
        m_tvArtist.setFilterTouchesWhenObscured(true);

        initSeekbarProgress();

        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            MusicInfo info = mAppData.musicPlayPositionInfo();
        }

        updateMusicInfo();
        onUpdateSeekbar(mAppData.mMediaPlayState);
        onChangePlayCtrl(mAppData.mMediaPlayState);
        onChangeRepeatModeCtrl(mAppData.musicRepeatMode());
        onChangeSeekbarValue(mAppData.mPlayTimeInfo);
    }

    @SuppressLint("SetTextI18n")
    private void initSeekbarProgress() {
        mVolumeSeekBar.setMax(40);
        mVolumeSeekBar.setProgress(mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC));
        m_tvVolumeValue.setText(mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC) + "");

        mVolumeSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, progress, 0);
                m_tvVolumeValue.setText(progress + "");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @SuppressLint("SetTextI18n")
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                int progress = seekBar.getProgress();
                mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, progress, 0);
                m_tvVolumeValue.setText(progress + "");
            }
        });

        mSeekbarProgress.setMax(mSeekBarMaxValue);
        mSeekbarProgress.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mSeekbarOperate = false;
                int progress = seekBar.getProgress();
                int time = mAppData.mPlayTimeInfo.mTotalTime * progress / mSeekBarMaxValue;
                mMusicViewModel.playerRelay().accept(
                        t -> t.requestExecuteAction(
                                IMediaAction.seekToTime, time, null));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                mSeekbarOperate = true;
                if (mListener != null) {
                    mListener.onMediaEvent(IMediaEvent.EVENT_SCROLL_SEEKBAR, null, null);
                }
                if (mSeekbarProgress.getParent() != null) {
                    mSeekbarProgress.getParent().requestDisallowInterceptTouchEvent(true);
                }
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                // TODO Auto-generated method stub

            }
        });
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(VOLUME_CHANGED_ACTION);
        intentFilter.addAction("android.media.STREAM_MUTE_CHANGED_ACTION");
        mContext.registerReceiver(receiver, intentFilter);
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // TODO: 预留接口
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnPlay:
                onPlayEvent();
                break;
            case R.id.btnPrev:
                onPrevEvent();
                break;
            case R.id.btnNext:
                onNextEvent();
                break;
            case R.id.btnRepeatMode:
                onRepeatModeEvent();
                break;
            case R.id.btnEQ:
                HFuncUtils.instance().gotoEQ(mContext);
                break;
            case R.id.btnHome:
                onHomeEvent();
                break;
            case R.id.btnBack:
                requireActivity().onBackPressed();
                break;
            case R.id.btnMenu:
                onMenuEvent();
                break;
            case R.id.btnVolume:
                McuManager.getsInstance().injectKeyEvent(McuConstant.K_MUTE,
                        KeyEvent.ACTION_DOWN);
                McuManager.getsInstance().injectKeyEvent(McuConstant.K_MUTE,
                        KeyEvent.ACTION_UP);
                break;
            default:
                break;
        }
    }

    private void onMenuEvent() {
        if (mListener != null) {
            mListener.onMediaEvent(
                    IMediaEvent.MCC201_EVENT_GOTO_MUSIC_LIST,
                    null, null);
        }
    }

    private void onHomeEvent() {
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_HOME);
        mContext.startActivity(intent);
    }

    private void onPlayEvent() {
        if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PAUSE)) {
            mMusicViewModel.playerRelay().accept(
                    t -> t.requestPlayControl(IMusicState.PLAY_CMD_PLAY));
        } else {
            mMusicViewModel.playerRelay().accept(
                    t -> t.requestPlayControl(IMusicState.PLAY_CMD_PAUSE));
        }
    }

    private void onPrevEvent() {
        mMusicViewModel.playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.setSeekTimeZero, null, null));
        mMusicViewModel.playerRelay().accept(
                t -> t.requestPlayControl(IMusicState.PLAY_CMD_PREV));
    }

    private void onNextEvent() {
        mMusicViewModel.playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.setSeekTimeZero, null, null));
        mMusicViewModel.playerRelay().accept(
                t -> t.requestPlayControl(IMusicState.PLAY_CMD_NEXT));
    }

    private void onRepeatModeEvent() {
        mMusicViewModel.playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.switchPlayRepeatMode, null, null));
        onChangeRepeatModeCtrl(mAppData.musicRepeatMode());
    }

    private void onEQEvent() {
        if (ExtAudioMuxer.ExtAudioAvailable) {
            McuManager.getsInstance().injectKeyEventTimeout(K_EQ, 50);
        } else {

            try {
                int audioSessionId = mMusicViewModel.getAudioSessionId();
                Intent intent = new Intent(
                        AudioEffect.ACTION_DISPLAY_AUDIO_EFFECT_CONTROL_PANEL);
                intent.putExtra(AudioEffect.EXTRA_AUDIO_SESSION, audioSessionId);
                // / M: must Use startActivityForResult() because That is the
                // requirement of
                // / MusicEffect get the ActivityPackage.
                getActivity().startActivity(intent);
            } catch (Exception e) {
                e.printStackTrace();
            }

        }
    }

    private void onChangePlayCtrl(int playState) {
        int nResId = 0;
        switch (playState) {
            case IMusicState.E_PLAY_STATE_PAUSE:
            case IMusicState.E_PLAY_STATE_STOP:
                nResId = R.drawable.mcc201_btn_play_bg;
                break;
            case IMusicState.E_PLAY_STATE_PLAY:
                nResId = R.drawable.mcc201_btn_pause_bg;
                break;

            default:
                break;
        }

        if (mBtnPlay != null) {
            mBtnPlay.setImageResource(nResId);
        }
    }

    private void onChangeRepeatModeCtrl(int playState) {
        int nResId = 0;
        switch (playState) {
            case IMusicState.REPEAT_MODE_QUEUE:
                nResId = R.drawable.mcc201_btn_rpt_bg;
                break;
            case IMusicState.REPEAT_MODE_ALL:
                nResId = R.drawable.mcc201_btn_rpt_bg;
                break;
            case IMusicState.REPEAT_MODE_ONE:
                nResId = R.drawable.mcc201_btn_rpt1_bg;
                break;
            case IMusicState.REPEAT_MODE_RANDOM:
                nResId = R.drawable.mcc201_btn_rdm_bg;
                break;

            default:
                break;
        }

        if (mBtnRepeatMode != null) {
            mBtnRepeatMode.setImageResource(nResId);
        }
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE:
                if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_MUSIC) {
                    onChangePlayCtrl(IMusicState.E_PLAY_STATE_STOP);
                    return;
                }
                if (!mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                    onUpdateSeekbar(mAppData.mMediaPlayState);
                }

                onChangePlayCtrl(mAppData.mMediaPlayState);
                if (!mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                    uninitVisualizer();
                }
                break;
            case IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME:
                if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_MUSIC) {
                    return;
                }
                if (!mSeekbarProgress.isEnabled()) {
                    // avoid when on play moment seek SeekBar
                    onUpdateSeekbar(mAppData.mMediaPlayState);
                }
                onChangeSeekbarValue(mAppData.mPlayTimeInfo);
                break;
            case IMediaEvent.EVENT_CHANGE_REPEAT_MODE:
                onChangeRepeatModeCtrl(mAppData.musicRepeatMode());
                break;
            case IMediaEvent.EVENT_UPDATE_MUSIC_LIST:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
                updateMusicInfo();
                break;
            default:
                break;
        }
    }

    private void changeMusicInfo(MusicInfo info) {
        if (info != null) {
            String text_unknown = getString(R3.string.text_unknown);
            if (TextUtils.isEmpty(info.mTitle) || "<Unknown>".equals(info.mTitle)) {
                if (TextUtils.isEmpty(info.mFileName)) {
                    m_tvTitle.setText(text_unknown);
                    m_tvMusicTitle.setText(text_unknown);
                } else {
                    int pos = info.mFileName.lastIndexOf(".");
                    if (pos != -1) {
                        String Name = info.mFileName.substring(0, pos);
                        if (TextUtils.isEmpty(Name)) {
                            m_tvTitle.setText(text_unknown);
                            m_tvMusicTitle.setText(text_unknown);
                        } else {
                            m_tvTitle.setText(Name);
                            m_tvMusicTitle.setText(Name);
                        }
                    } else {
                        m_tvTitle.setText(info.mFileName);
                        m_tvMusicTitle.setText(info.mFileName);
                    }
                }
            } else {
                int pos = info.mTitle.lastIndexOf(".");
                if (pos != -1) {
                    String title = info.mTitle.substring(0, pos);
                    if (TextUtils.isEmpty(title)) {
                        m_tvTitle.setText(text_unknown);
                        m_tvMusicTitle.setText(text_unknown);
                    } else {
                        m_tvTitle.setText(title);
                        m_tvMusicTitle.setText(title);
                    }
                } else {
                    m_tvTitle.setText(info.mTitle);
                    m_tvMusicTitle.setText(info.mTitle);
                }
            }

            if (TextUtils.isEmpty(info.mArtist) || "<Unknown>".equals(info.mArtist)) {
                m_tvArtist.setText(text_unknown);
            } else {
                m_tvArtist.setText(info.mArtist);
            }

            if (TextUtils.isEmpty(info.mFilePath) || "<Unknown>".equals(info.mFilePath)) {
                mTvResource.setText(text_unknown);
            } else {
                mTvResource.setText(info.mFilePath);
            }

            updateMusicImage(info);
        }
    }

    private void updateMusicImage(MusicInfo info) {
        btnFavor.setImageResource(
                info.mFavorite ? R.drawable.mcc201_icon_favor_p : R.drawable.mcc201_icon_favor_n);
        m_ivMusicImage.setTag(info.mFilePath);
        if (info.mID3Type == MusicInfo.ID3_TYPE_ERROR) {
            m_ivMusicImage.setImageResource(R.drawable.default_thumbnails_bg_zxdz);
        } else {
            BitmapCache.getInstance().loadNativeImage(
                    info.mFilePath, m_ivMusicImage, R.drawable.mcc201_music_cover, true);
        }
    }

    private void updateMusicInfo() {
        MusicInfo info = mAppData.mCurrentMediaInfo;
        if (null != info) {
            changeMusicInfo(info);
        }
    }

    private void onUpdateSeekbar(int playState) {
        if (playState == IMusicState.E_PLAY_STATE_PLAY) {
            mSeekbarProgress.setEnabled(true);
        } else {
            mSeekbarProgress.setEnabled(false);
        }
    }

    private void uninitVisualizer() {
        Visualizer visualizer = mMusicViewModel.getVisualizer();
        if (visualizer != null) {
            if (visualizer.getEnabled()) {
                visualizer.setEnabled(false);
            }
        }
    }

    @SuppressLint("SetTextI18n")
    private void onChangeSeekbarValue(MediaTimeInfo state) {
        int nTotalTime = state.mTotalTime / 1000;
        int nCurrentTime = state.mCurrentTime / 1000;

        if (nTotalTime > 0) {
            int value = nCurrentTime * mSeekBarMaxValue / nTotalTime;
            String totalTime;
            String currentTime;
            if (nTotalTime >= 6000) {
                totalTime = String.format(Locale.getDefault(),
                        "%03d:%02d", nTotalTime / 60, nTotalTime % 60);
                currentTime = String.format(Locale.getDefault(),
                        "%03d:%02d", nCurrentTime / 60, nCurrentTime % 60);
            } else {
                totalTime = String.format(Locale.getDefault(),
                        "%02d:%02d", nTotalTime / 60 % 100, nTotalTime % 60);
                currentTime = String.format(Locale.getDefault(),
                        "%02d:%02d", nCurrentTime / 60 % 100, nCurrentTime % 60);
            }

            m_tvCurrentTime.setText(currentTime);
            m_tvTotalTime.setText(totalTime);

            if (!mSeekbarOperate) {
                mSeekbarProgress.setProgress(value);
            }
        } else {
            m_tvCurrentTime.setText("00:00");
            m_tvTotalTime.setText("00:00");
            if (!mSeekbarOperate) {
                mSeekbarProgress.setProgress(0);
            }
        }
    }

    @Override
    public void onFftDataCapture(Visualizer arg0, byte[] fft, int arg2) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onWaveFormDataCapture(Visualizer arg0, byte[] fft, int arg2) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onSeekTo(int progress) {
        mMusicViewModel.playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.seekToTime, progress, null));
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouch(View v, MotionEvent event) {
        if (v.getId() == R.id.layout_progress) {
            Rect seekRect = new Rect();
            mSeekbarProgress.getHitRect(seekRect);
            if ((event.getY() >= (seekRect.top - 10)) && (event.getY() <= (seekRect.bottom + 10))) {
                float y = seekRect.top + seekRect.height() / 2.0f;
                // seekBar only accept relative x
                float x = event.getX() - seekRect.left;
                if (x < 0) {
                    x = 0;
                } else if (x > seekRect.width()) {
                    x = seekRect.width();
                }
                MotionEvent me = MotionEvent.obtain(event.getDownTime(), event.getEventTime(),
                        event.getAction(), x, y, event.getMetaState());
                return mSeekbarProgress.onTouchEvent(me);
            }
        }
        return false;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mContext.unregisterReceiver(receiver);
    }
}
