package com.hcn.media.music.base;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.media.audiofx.Visualizer;
import android.media.audiofx.Visualizer.OnDataCaptureListener;
import android.os.Build;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.common.app.HActivityUtils;
import com.hcn.common.misc.LogUtils;
import com.hcn.config.Customer;
import com.hcn.media.R3;
import com.hcn.auto.AutoStatus;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media.base.layout.FrameLayoutEx;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media.vm.action.IPlayerEx;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_common.utils.ViewUtilsEx;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_data.debug.DebugUiData;
import com.hcn.media_data.ui.MediaPageState;
import com.hcn.media_data.ui.base.PageDataKV;
import com.hcn.media_theme.Argument;
import com.hcn.media_theme.ThemeX;
import com.hcn.mediaservice.data.MediaTimeInfo;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media.local.utils.HFuncUtils;
import com.hcn.common.Utility;
import com.hcn.media_view.lyrics.LyricsManager;
import com.hcn.media_view.lyrics.LyricsRow;
import com.hcn.media_view.lyrics.LyricsView;
import com.hcn.media_view.lyrics.LyricsView.OnSeekToListener;
import com.hcn.media_view.PlayFlashView;
import com.hcn.media_view.HTextView;

import java.util.List;
import java.util.Locale;
import java.util.Objects;

/**
 * 承储设备播放界面布局
 * <p> 子项元素基本都是使用的帧布局，扩展性强；
 *
 * @author 86158
 */
@SuppressLint("ViewConstructor")
@SuppressWarnings("deprecation")
public class MusicInfoLayout extends FrameLayoutEx
        implements OnClickListener, OnDataCaptureListener,
        OnSeekToListener, OnTouchListener {
    private final static String TAG = MusicInfoLayout.class.getSimpleName();

    private static final int mSeekBarMaxValue = 1000;

    private View mBtnPlay = null;
    private View mBtnRepeatMode = null;
    private SeekBar mSeekBarProgress = null;
    private TextView mTvCurrentTime = null;
    private TextView mTvTotalTime = null;

    private ImageView mIvMusicImage = null;
    private HTextView mTvTitle = null;
    private HTextView mTvArtist = null;
    private HTextView mTvAlbum = null;
    private TextView mTvTotalValue = null;
    private View mFreqLayout, mLyricLayout;
    private View mBtnChangeLyric;

    private boolean mSeekBarOperate = false;

    private TextView mTvNoLyrics = null;
    private PlayFlashView mFrequencyView = null;
    private LyricsView mLyricsView = null;
    private LyricsManager mLyricsManager = null;
    private List<LyricsRow> mLyricsList = null;
    private boolean mFirstChanged = true;
    private final String mCustomerName;

    public MusicInfoLayout(Context context,
                           @NonNull IPlayerEx player) {
        this(context, null, player);
    }

    public MusicInfoLayout(Context context,
                           AttributeSet attrs,
                           @NonNull IPlayerEx player) {
        this(context, attrs, 0, player);
    }

    public MusicInfoLayout(Context context,
                           AttributeSet attrs,
                           int defStyle,
                           @NonNull IPlayerEx player) {
        super(context, attrs, defStyle, player);
        mCustomerName = Customer.name();

        // 歌词管理器对象（唯一实例）
        mLyricsManager = LyricsManager.instance();

        // [注意：不可以提取放到父类中使用/否则 inflate 的视图会被系统回收]
        initContentView(getLayoutRes(), this);
    }

    @Override
    protected int getLayoutRes() {
        return R.layout.fragment_musicinfo;
    }

    @Nullable
    @Override
    protected View initContentView(int layoutRes, ViewGroup root) {
        LogUtil.d(TAG, "initContentView.");
        View view = super.initContentView(layoutRes, root);
        assert view != null;

        mBtnPlay = findViewById(xId(R.id.btnPlay));
        if (mBtnPlay != null) {
            mBtnPlay.setOnClickListener(this);
        }

        View btnPrev = findViewById(xId(R.id.btnPrev));
        if (null != btnPrev) {
            btnPrev.setOnClickListener(this);
        }

        View btnNext = findViewById(xId(R.id.btnNext));
        if (null != btnNext) {
            btnNext.setOnClickListener(this);
        }

        View btnRepeatMode = findViewById(xId(R.id.btnRepeatMode));
        if (null != btnRepeatMode) {
            btnRepeatMode.setOnClickListener(this);
        }

        mBtnChangeLyric = findViewById(xId(R.id.btn_change_lyric));
        if (null != mBtnChangeLyric) {
            mBtnChangeLyric.setOnClickListener(this);
        }

        mFreqLayout = findViewById(xId(R.id.lin_freq));
        if (null != mFreqLayout) {
            mFreqLayout.setOnClickListener(this);
        }

        mLyricLayout = findViewById(xId(R.id.lin_lyric));
        if (null != mLyricLayout) {
            mLyricLayout.setOnClickListener(this);
        }

        View progressLayout = findViewById(xId(R.id.layout_progress));
        if (progressLayout != null) {
            progressLayout.setOnTouchListener(this);
        }

        mBtnRepeatMode = findViewById(xId(R.id.btnRepeatMode));

        mTvCurrentTime = findViewById(xId(R.id.tvCurrentTime));
        mTvTotalTime = findViewById(xId(R.id.tvTotalTime));
        mSeekBarProgress = findViewById(xId(R.id.seekbar_progress));

        mIvMusicImage = findViewById(xId(R.id.ivMusicImage));
        mTvTitle = findViewById(xId(R.id.tvTitle));
        mTvArtist = findViewById(xId(R.id.tvArtist));
        mTvAlbum = findViewById(xId(R.id.tvAlbum));
        mTvTotalValue = findViewById(xId(R.id.tvTotalValue));

        // 指定当该 View 的窗口被其他可见的窗口遮挡时是否过滤触摸事件
        if (mTvTitle != null) {
            mTvTitle.setFilterTouchesWhenObscured(true);
        }

        if (mTvArtist != null) {
            mTvArtist.setFilterTouchesWhenObscured(true);
        }

        if (mTvAlbum != null) {
            mTvAlbum.setFilterTouchesWhenObscured(true);
        }

        mTvNoLyrics = findViewById(xId(R.id.tvNoLyrics));
        mFrequencyView = findViewById(xId(R.id.FrequencyView));

        mLyricsView = findViewById(xId(R.id.lyrics_view));
        if (mLyricsView != null) {
            mLyricsView.setOnSeekToListener(this);
            mLyricsView.SetPainTypeface(Typeface.SERIF);
            mLyricsView.setOnLrcClickListener(this::onChangeLyricView);

            mLyricsView.SetCurPaintColor(
                    xColor(R.color.lyric_view_cur_paint_color));
            mLyricsView.SetNotCurPaintColor(
                    xColor(R.color.lyric_view_not_cur_paint_color));
        }

        initSeekBarCtrl();

        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            MusicInfo info = mAppData.musicPlayPositionInfo();
            if (mFirstChanged) {
                updateLrcRowList(info);
            }
        }

        ViewGroup relId3Info = findViewById(xId(R.id.rel_id3_infos));
        if (null != relId3Info) {
            if (Customer.ZE_XUN_ELECTRONICS.equals(mCustomerName)) {
                relId3Info.setBackgroundResource(xDrawableId2(R.drawable.playing_bg_zxdz));
            } else {
                relId3Info.setBackgroundResource(xDrawableId2(R.drawable.playing_bg));
            }

            if (1 == Settings.System.getInt(
                    mContext.getContentResolver(), "car_home_style", 0)) {
                relId3Info.setBackgroundResource(xDrawableId2(R.drawable.playing_bg_style_1));
            }
        }

        adjustLyricFreqLayoutBySplitScreen(this);
        updateMusicInfo();

        onUpdateSeekBar(mAppData.mMediaPlayState);
        onChangePlayCtrl(mAppData.mMediaPlayState);
        onChangeRepeatModeCtrl(mAppData.musicRepeatMode());
        onChangeSeekbarValue(mAppData.mPlayTimeInfo);

        // 初始化显示选择（从记忆）
        initLyricAndFreqVisible();

        return view;
    }

    /**
     * 调整歌词频谱类布局元素
     * <pre>
     *    对于特定 UI 在横竖屏显示的时候做显示调整；
     *    主要是为了减少在 res/layout-port/ 中添加相识度高的类重复布局文件；
     * </pre>
     *
     * @param view 当前页面关联视图
     */
    private void adjustLyricFreqLayoutBySplitScreen(@NonNull View view) {
        View layout = view.findViewById(xId(R.id.lyric_freq_layout));
        if (Objects.isNull(layout)) {
            return;
        }

        // 我们统一不显示歌词与频谱元素；
        Activity owner = HActivityUtils.getActivityByContext(mContext);
        if (Objects.isNull(owner)) {
            return;
        }

        // 判断是分屏状态（和调用时机有关）
        boolean isInSplitScreenMode = false;
        Configuration configuration = getResources().getConfiguration();
        if (configuration != null) {
            String configText = configuration.toString();
            if (!TextUtils.isEmpty(configText)) {
                assert owner != null;
                if (owner.isInMultiWindowMode()
                        && (Build.VERSION.SDK_INT < Build.VERSION_CODES.P
                        || configText.contains("mWindowingMode=split-screen"))) {
                    isInSplitScreenMode = true;
                }
            }
        }

        // [别随意修改，主要是兼容历史效果]
        if (isInSplitScreenMode
                && MiscUtils.isPortraitWindow(mContext)) {
            layout.setVisibility(View.GONE);
        } else {
            // 历史客户皮肤处理（显示体验）
            switch (Argument.E_THEME_GOD) {
                case ThemeX.ET_GOD_206:
                    layout.setVisibility(
                            isInSplitScreenMode? View.GONE: View.VISIBLE);
                    break;
                case ThemeX.ET_GOD_NONE:
                default:
                    layout.setVisibility(View.VISIBLE);
                    break;
            }
        }
    }

    private void initSeekBarCtrl() {
        mSeekBarProgress.setMax(mSeekBarMaxValue);
        mSeekBarProgress.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mSeekBarOperate = false;

                int progress = seekBar.getProgress();
                int time = mAppData.mPlayTimeInfo.mTotalTime * progress / mSeekBarMaxValue;
                if (mMediaEventPostbox != null) {
                    mMediaEventPostbox.onMediaAction(
                            IMediaAction.seekToTime, time, null);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                LogUtil.low_i("seekBar", "onStartTrackingTouch");
                mSeekBarOperate = true;

                if (mMediaEventPostbox != null) {
                    mMediaEventPostbox.onMediaEvent(
                            IMediaEvent.EVENT_SCROLL_SEEKBAR,
                            null,
                            null);
                }

                if (mSeekBarProgress.getParent() != null) {
                    mSeekBarProgress.getParent().requestDisallowInterceptTouchEvent(true);
                }
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                // TODO Auto-generated method stub
            }
        });
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (getId(v)) {
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

            case R.id.btn_change_lyric:
            case R.id.lin_freq:
            case R.id.lin_lyric:
                onChangeLyricView();
                break;

            default:
                break;
        }
    }

    /**
     * 初始化歌词频谱显示状态
     * <p> 根据 {@link MediaPageState} 记录的状态显示当前 UI 状态；
     */
    private void initLyricAndFreqVisible() {
        // 上一次的歌词频谱显示标记
        final int flag = MediaPageState.instance().readInt(
                PageDataKV.Key.LYRICS_SPECTRUM_FLAG, PageDataKV.Value.LYRICS);

        switch (flag) {
            case PageDataKV.Value.LYRICS:
                setLyricLayoutVisible();
                break;
            case PageDataKV.Value.SPECTRUM:
                setFreqLayoutVisible();
                break;
            default:
                break;
        }
    }

    /**
     * 显示歌词布局视图
     * <p> 对应的隐藏频谱布局视图；
     */
    private void setLyricLayoutVisible() {
        if (null == mFreqLayout || null == mLyricLayout) {
            return;
        }

        mFreqLayout.setVisibility(View.GONE);
        mLyricLayout.setVisibility(View.VISIBLE);

        if (null != mBtnChangeLyric
                && mBtnChangeLyric instanceof Button) {
            ((Button)mBtnChangeLyric).setText(xString(R3.string.frequency_label));
        }

        MediaPageState.instance().write(
                PageDataKV.Key.LYRICS_SPECTRUM_FLAG,
                PageDataKV.Value.LYRICS);
    }

    /**
     * 显示频谱布局视图
     * <p> 对应的隐藏歌词布局视图；
     */
    private void setFreqLayoutVisible() {
        if (null == mFreqLayout || null == mLyricLayout) {
            return;
        }

        mFreqLayout.setVisibility(View.VISIBLE);
        mLyricLayout.setVisibility(View.GONE);

        if (null != mBtnChangeLyric
                && mBtnChangeLyric instanceof Button) {
            ((Button)mBtnChangeLyric).setText(xString(R3.string.lyric_label));
        }

        MediaPageState.instance().write(
                PageDataKV.Key.LYRICS_SPECTRUM_FLAG,
                PageDataKV.Value.SPECTRUM);
    }

    private void onChangeLyricView() {
        if (null == mFreqLayout || null == mLyricLayout) {
            return;
        }

        if (mFreqLayout.getVisibility() == View.VISIBLE) {
            setLyricLayoutVisible();
        } else {
            setFreqLayoutVisible();
        }
    }

    private void onPlayEvent() {
        if (Objects.isNull(mMediaEventPostbox)) {
            return;
        }

        if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PAUSE)) {
            mMediaEventPostbox.onMediaAction(
                    IMediaAction.playControl, IMusicState.PLAY_CMD_PLAY, null);
        } else {
            mMediaEventPostbox.onMediaAction(
                    IMediaAction.playControl, IMusicState.PLAY_CMD_PAUSE, null);
        }
    }

    private void onPrevEvent() {
        if (Objects.isNull(mMediaEventPostbox)) {
            return;
        }

        mMediaEventPostbox.onMediaAction(
                IMediaAction.setSeekTimeZero, null, null);
        mMediaEventPostbox.onMediaAction(
                IMediaAction.playControl, IMusicState.PLAY_CMD_PREV, null);
    }

    private void onNextEvent() {
        if (Objects.isNull(mMediaEventPostbox)) {
            return;
        }

        mMediaEventPostbox.onMediaAction(
                IMediaAction.setSeekTimeZero, null, null);
        mMediaEventPostbox.onMediaAction(
                IMediaAction.playControl, IMusicState.PLAY_CMD_NEXT, null);
    }

    private void onRepeatModeEvent() {
        if (Objects.isNull(mMediaEventPostbox)) {
            return;
        }

        mMediaEventPostbox.onMediaAction(
                IMediaAction.switchPlayRepeatMode, null, null);
        onChangeRepeatModeCtrl(mAppData.musicRepeatMode());
    }

    private void onEQEvent() {
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_LAUNCHER);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        ComponentName cn = new ComponentName("com.auto.apieq", "com.auto.apieq.EqUIMain");
        intent.setComponent(cn);

        try {
            mContext.startActivity(intent);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void onChangePlayCtrl(int playState) {
        int nResId = 0;
        switch (playState) {
            case IMusicState.E_PLAY_STATE_PAUSE:
            case IMusicState.E_PLAY_STATE_STOP:
                nResId = R.drawable.btn_play_bg;
                break;
            case IMusicState.E_PLAY_STATE_PLAY:
                nResId = R.drawable.btn_pause_bg;
                break;
        }

        if (mBtnPlay != null) {
            if (mBtnPlay instanceof Button) {
                mBtnPlay.setBackgroundResource(xDrawableId2(nResId));
            } else if (mBtnPlay instanceof ImageView) {
                ((ImageView) mBtnPlay).setImageResource(xDrawableId2(nResId));
            }
        }
    }

    private void onChangeRepeatModeCtrl(int playState) {
        int nResId = 0;
        switch (playState) {
            case IMusicState.REPEAT_MODE_QUEUE:
                nResId = xDrawableId2(R.drawable.btn_repeat_queue_bg);
                break;
            case IMusicState.REPEAT_MODE_ALL:
                nResId = xDrawableId2(R.drawable.btn_repeat_all_bg);
                break;
            case IMusicState.REPEAT_MODE_ONE:
                nResId = xDrawableId2(R.drawable.btn_repeat_one_bg);
                break;
            case IMusicState.REPEAT_MODE_RANDOM:
                nResId = xDrawableId2(R.drawable.btn_repeat_random_bg);
                break;
        }

        if (mBtnRepeatMode != null) {
            if (mBtnRepeatMode instanceof Button) {
                mBtnRepeatMode.setBackgroundResource(nResId);
            } else if (mBtnRepeatMode instanceof ImageView) {
                ((ImageView) mBtnRepeatMode).setImageResource(nResId);
            }
        }
    }

    @Override
    public void initLayout() {
        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            MusicInfo info = mAppData.musicPlayPositionInfo();

            if (mFirstChanged) {
                updateLrcRowList(info);
            }
        }

        updateMusicInfo();
        onUpdateSeekBar(mAppData.mMediaPlayState);
        onChangePlayCtrl(mAppData.mMediaPlayState);
        onChangeRepeatModeCtrl(mAppData.musicRepeatMode());
        onChangeSeekbarValue(mAppData.mPlayTimeInfo);

        initVisualizer();
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (Objects.isNull(getContentView())) {
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE: {
                if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_MUSIC) {
                    onChangePlayCtrl(IMusicState.E_PLAY_STATE_STOP);
                    return;
                }

                if (!mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                    onUpdateSeekBar(mAppData.mMediaPlayState);
                }

                onChangePlayCtrl(mAppData.mMediaPlayState);
                if (!mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                    unInitVisualizer();
                }
                break;
            }
            case IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME: {
                if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_MUSIC) {
                    return;
                }

                // avoid when on play moment seek SeekBar
                if (!mSeekBarProgress.isEnabled()) {
                    onUpdateSeekBar(mAppData.mMediaPlayState);
                }

                onChangeSeekbarValue(mAppData.mPlayTimeInfo);
                Visualizer visualizer = player().getVisualizer();
                if (visualizer != null && !visualizer.getEnabled()) {
                    initVisualizer();
                }

                onChangeLyricsView(mAppData.mPlayTimeInfo);
                break;
            }
            case IMediaEvent.EVENT_CHANGE_REPEAT_MODE: {
                onChangeRepeatModeCtrl(mAppData.musicRepeatMode());
                break;
            }
            case IMediaEvent.EVENT_UPDATE_MUSIC_LIST:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM: {
                updateMusicInfo();
                break;
            }
            case IMediaEvent.EVENT_UPDATE_MUSIC_ID3: {
                updateMusicTextInfo();
                break;
            }
            default:
                break;
        }
    }

    private void onChangeLyricsView(MediaTimeInfo info) {
        if (mLyricsList != null && mLyricsList.size() > 0) {
            mLyricsView.seekTo(
                    info.mCurrentTime,
                    true,
                    false);
        }
    }

    public void updateVisualizer(byte[] data) {
        if (mFrequencyView != null) {
            if (DebugUiData.MUSIC_DEBUG_I) {
                LogUtils.iTag(TAG,
                        "updateVisualizer: data.length = " + data.length);
            }

            if (AutoStatus.isReversing()
                    || !ViewUtilsEx.isVisible(mFrequencyView, View.VISIBLE)) {
                return;
            }

            mFrequencyView.updateVisualizer(data);
        }
    }

    private void updateMusicImage(MusicInfo info) {
        mIvMusicImage.setTag(info.mFilePath);
        if (Customer.ZE_XUN_ELECTRONICS.equals(mCustomerName)) {
            if (MusicInfo.ID3_TYPE_ERROR == info.mID3Type) {
                mIvMusicImage.setImageResource(xDrawableId2(R.drawable.default_thumbnails_bg_zxdz));
            } else {
                BitmapCache.getInstance().loadNativeImage(info.mFilePath,
                        mIvMusicImage, xDrawableId2(R.drawable.default_thumbnails_bg_zxdz), true);
            }
        } else {
            if (MusicInfo.ID3_TYPE_ERROR == info.mID3Type) {
                mIvMusicImage.setImageResource(xDrawableId2(R.drawable.default_thumbnails_bg));
            } else {
                BitmapCache.getInstance().loadNativeImage(info.mFilePath,
                        mIvMusicImage, xDrawableId2(R.drawable.default_thumbnails_bg), true);
            }
        }
    }

    /**
     * 刷新视图，用于自定义 View 的刷新
     *
     * @param isNightMode 是否为夜间模式
     */
    @Override
    public void onUpdateUiModeView(boolean isNightMode) {
        super.onUpdateUiModeView(isNightMode);

        // 刷新资源
        updatePlayerResource();
    }

    /**
     * 更新 ID3 文字信息
     * @param info 音乐对象
     */
    private void updateId3TextInfo(MusicInfo info) {
        if (Objects.isNull(info)) {
            return;
        }

        String text_unknown = xString(R3.string.text_unknown);
        if (!Utility.supportMediaId3Title(mContext) ||
                TextUtils.isEmpty(info.mTitle) ||
                "<Unknown>".equals(info.mTitle)) {
            if (TextUtils.isEmpty(info.mFileName)) {
                mTvTitle.setText(text_unknown);
            } else {
                int pos = info.mFileName.lastIndexOf(".");
                if (pos != -1) {
                    String Name = info.mFileName.substring(0, pos);
                    if (TextUtils.isEmpty(Name)) {
                        mTvTitle.setText(text_unknown);
                    } else {
                        mTvTitle.setText(Name);
                    }
                } else {
                    mTvTitle.setText(info.mFileName);
                }
            }
        } else {
            int pos = info.mTitle.lastIndexOf(".");
            if (pos != -1) {
                String title = info.mTitle.substring(0, pos);
                if (TextUtils.isEmpty(title)) {
                    mTvTitle.setText(text_unknown);
                } else {
                    mTvTitle.setText(title);
                }
            } else {
                mTvTitle.setText(info.mTitle);
            }
        }

        if (TextUtils.isEmpty(info.mArtist)
                || "<Unknown>".equals(info.mArtist)) {
            mTvArtist.setText(text_unknown);
        } else {
            mTvArtist.setText(info.mArtist);
        }

        if (TextUtils.isEmpty(info.mAlbum)
                || "<Unknown>".equals(info.mAlbum)) {
            mTvAlbum.setText(text_unknown);
        } else {
            mTvAlbum.setText(info.mAlbum);
        }
    }

    private void changeMusicInfo(MusicInfo info) {
        if (Objects.isNull(info)) {
            return;
        }

        updateId3TextInfo(info);
        updateMusicImage(info);
    }

    /**
     * 更新音乐播放信息
     * <p> index、ID3、Lrc...
     */
    private void updateMusicInfo() {
        changeTotalValue(
                mAppData.musicPlayPosition(),
                mAppData.musicPlaylist().size());

        MusicInfo info = mAppData.mCurrentMediaInfo;
        if (null != info) {
            changeMusicInfo(info);
            updateLrcRowList(info);
        }
    }

    private void updateLrcRowList(MusicInfo info) {
        mFirstChanged = false;
        mLyricsList = mLyricsManager.getLrcFromSong(info.mFilePath);

        if (mLyricsList != null && mLyricsList.size() > 0) {
            if (mLyricsView != null) {
                mLyricsView.setVisibility(View.VISIBLE);
            }

            if (mTvNoLyrics != null) {
                mTvNoLyrics.setVisibility(View.GONE);
            }

            mLyricsView.setLrcRows(mLyricsList);
        } else {
            if (mLyricsView != null) {
                mLyricsView.setVisibility(View.GONE);
            }

            if (mTvNoLyrics != null) {
                mTvNoLyrics.setVisibility(View.VISIBLE);
            }
        }
    }

    private void changeTotalValue(int index, int total) {
        if (mTvTotalValue != null && total > 0) {
            String text = String.format(Locale.getDefault(), "%d/%d", index + 1, total);
            mTvTotalValue.setText(text);
        }
    }

    /**
     * 只更新文字相关信息
     * @see IMediaEvent#EVENT_UPDATE_MUSIC_ID3
     */
    private void updateMusicTextInfo() {
        // 检查索引的有效性
        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            // 只更新文字信息（专辑封面单独处理）
            MusicInfo info = mAppData.musicPlayPositionInfo();
            if (!Objects.isNull(info)) {
                updateId3TextInfo(info);
            }

            changeTotalValue(mAppData.musicPlayPosition(), mAppData.musicPlaylist().size());
        }
    }

    private void onUpdateSeekBar(int playState) {
        mSeekBarProgress.setEnabled(
                playState == IMusicState.E_PLAY_STATE_PLAY);
    }

    private void unInitVisualizer() {
        Visualizer visualizer = player().getVisualizer();
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

            mTvCurrentTime.setText(currentTime);
            mTvTotalTime.setText(totalTime);

            if (!mSeekBarOperate) {
                mSeekBarProgress.setProgress(value);
            }
        } else {
            mTvCurrentTime.setText("00:00");
            mTvTotalTime.setText("00:00");
            if (!mSeekBarOperate) {
                mSeekBarProgress.setProgress(0);
            }
        }
    }

    private void initVisualizer() {
        if (!mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
            return;
        }

        Visualizer visualizer = player().getVisualizer();
        if (visualizer != null) {
            if (visualizer.getEnabled()) {
                visualizer.setEnabled(false);
            }

            visualizer.setCaptureSize(128);
            visualizer.setDataCaptureListener(
                    this, Visualizer.getMaxCaptureRate() / 2, false, true);

            if (!visualizer.getEnabled()) {
                visualizer.setEnabled(true);
            }
        }
    }

    @Override
    public void initDataObject() {
        initVisualizer();
    }

    @Override
    public void onFftDataCapture(Visualizer arg0, byte[] fft, int arg2) {
        updateVisualizer(fft);
    }

    @Override
    public void onWaveFormDataCapture(Visualizer arg0, byte[] fft, int arg2) {
        updateVisualizer(fft);
    }

    @Override
    public void onSeekTo(int progress) {
        if (mMediaEventPostbox != null) {
            mMediaEventPostbox.onMediaAction(
                    IMediaAction.seekToTime, progress, null);
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        // TODO Auto-generated method stub
        LogUtil.low_i("test", "v:" + (getId(v) == R.id.layout_progress));

        if (getId(v) == R.id.layout_progress) {
            Rect seekRect = new Rect();
            mSeekBarProgress.getHitRect(seekRect);
            LogUtil.low_i("test", "bottom:" + seekRect.bottom + "  top:" + seekRect.top);

            if ((event.getY() >= (seekRect.top - 10))
                    && (event.getY() <= (seekRect.bottom + 10))) {
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
                return mSeekBarProgress.onTouchEvent(me);
            }
        }

        return false;
    }

    /**
     * 更新播放资源，重新触发设置图标资源
     * <p> 用于白天黑夜切换 </>
     *
     * @see #onUpdateUiModeView(boolean)
     */
    protected void updatePlayerResource() {
        // 检查索引的有效性
        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            MusicInfo info = mAppData.musicPlayPositionInfo();
            if (info != null) {
                updateMusicImage(info);
            }
        }

        onChangePlayCtrl(mAppData.mMediaPlayState);
        onChangeRepeatModeCtrl(mAppData.musicRepeatMode());
    }
}
