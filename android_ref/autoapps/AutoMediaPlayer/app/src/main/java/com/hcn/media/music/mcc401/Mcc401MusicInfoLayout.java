package com.hcn.media.music.mcc401;

import android.annotation.SuppressLint;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.media.audiofx.Visualizer;
import android.media.audiofx.Visualizer.OnDataCaptureListener;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
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
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.viewpager.widget.PagerAdapter;
import androidx.viewpager.widget.ViewPager;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.config.Customer;
import com.hcn.media.R3;
import com.hcn.auto.AutoStatus;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media.base.layout.FrameLayoutEx;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_data.storage.IStorageDevice;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media.vm.action.IPlayerEx;
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
import com.hcn.media_view.ScrollableSeekBar;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Objects;

/**
 * Mcc401 音乐信息布局
 * @author 86158
 */
@SuppressLint("ViewConstructor")
@SuppressWarnings("deprecation")
public class Mcc401MusicInfoLayout extends FrameLayoutEx
        implements OnClickListener, OnDataCaptureListener,
        OnSeekToListener, OnTouchListener {

    private final static String TAG = Mcc401MusicInfoLayout.class.getSimpleName();

    private static final int mSeekBarMaxValue = 1000;

    private final Handler mUserDelayHandler;
    private final String mCustomerName;

    private Button mBtnPlay = null;
    private Button mBtnRepeatMode = null;
    private ScrollableSeekBar mSeekbarProgress = null;
    private TextView mTvCurrentTime = null;
    private TextView mTvTotalTime = null;

    private ImageView mIvPageBg = null;
    private ImageView mIvMusicImage = null;
    private TextView mTvSongStorage = null;
    private HTextView mTvTitle = null;
    private HTextView mTvArtist = null;
    private HTextView mTvAlbum = null;
    private TextView mTvTotalValue = null;
    private LinearLayout mFreqLayout, mLyricLayout;
    private Button mBtnChangeLyric;

    private boolean mSeekbarOperate = false;

    private TextView mTvNoLyrics = null;
    private PlayFlashView mFrequencyView = null;
    private LyricsView mLyricsView = null;
    private LyricsManager mLyricsManager = null;
    private List<LyricsRow> mLyricsList = null;
    private boolean mFirstChanged = true;
    private RelativeLayout mRlId3Info;
    private LyricsView mLyricView = null;

    private View mFreqLyricView = null;
    private View mId3View = null;
    private ViewPager mViewPagerInfo = null;
    private List<View> mViews = null;
    private ViewPagerAdapter mViewPagerAdapter = null;
    private RadioButton mRbPage1, mRbPage2, mRbShowLyric;

    public Mcc401MusicInfoLayout(Context context,
                                 @NonNull IPlayerEx player) {
        this(context, null, player);
    }

    public Mcc401MusicInfoLayout(Context context,
                                 AttributeSet attrs,
                                 @NonNull IPlayerEx player) {
        this(context, attrs, 0, player);
    }

    public Mcc401MusicInfoLayout(Context context,
                                 AttributeSet attrs,
                                 int defStyle,
                                 @NonNull IPlayerEx player) {
        super(context, attrs, defStyle, player);

        mCustomerName = Customer.name();
        mUserDelayHandler = new UserDelayHandler(mContext.getMainLooper());

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

        mId3View = initId3View();
        mFreqLyricView = initFreqView();

        mViews = new ArrayList<>();
        mViews.add(mId3View);
        mViews.add(mFreqLyricView);

        mViewPagerInfo = findViewById(xId(R.id.viewpager_info));
        if (mViewPagerInfo != null) {
            mViewPagerAdapter = new ViewPagerAdapter(mContext, mViews);
            mViewPagerInfo.setAdapter(mViewPagerAdapter);
            mViewPagerInfo.setOnPageChangeListener(new ViewPageChangeListener());
        }

        mFreqLayout = mFreqLyricView.findViewById(xId(R.id.lin_freq));
        if (mFreqLayout != null) {
            mFreqLayout.setOnClickListener(this);
        }

        mLyricLayout = mFreqLyricView.findViewById(xId(R.id.lin_lyric));
        if (mLyricLayout != null) {
            mLyricLayout.setOnClickListener(this);
        }

        mLyricsView = mFreqLyricView.findViewById(xId(R.id.lyrics_view));
        mTvTitle = mId3View.findViewById(xId(R.id.tvTitle));
        mTvArtist = mId3View.findViewById(xId(R.id.tvArtist));
        mTvAlbum = mId3View.findViewById(xId(R.id.tvAlbum));
        mTvTotalValue = mId3View.findViewById(xId(R.id.tvTotalValue));
        mTvNoLyrics = mFreqLyricView.findViewById(xId(R.id.tvNoLyrics));
        mFrequencyView = mFreqLyricView.findViewById(xId(R.id.FrequencyView));

        mRbPage1 = findViewById(xId(R.id.rb_page1));
        if (mRbPage1 != null) {
            mRbPage1.setOnClickListener(this);
        }

        mRbPage2 = findViewById(xId(R.id.rb_page2));
        if (mRbPage2 != null) {
            mRbPage2.setOnClickListener(this);
        }

        mRbShowLyric = findViewById(xId(R.id.rbShowLyric));
        if (mRbShowLyric != null) {
            mRbShowLyric.setOnClickListener(this);
        }

        mBtnPlay = findViewById(xId(R.id.btnPlay));
        if (mBtnPlay != null) {
            mBtnPlay.setOnClickListener(this);
        }

        View btnPrev = findViewById(xId(R.id.btnPrev));
        if (btnPrev != null){
            btnPrev.setOnClickListener(this);
        }

        View btnNext = findViewById(xId(R.id.btnNext));
        if (btnNext != null) {
            btnNext.setOnClickListener(this);
        }

        mBtnRepeatMode = findViewById(xId(R.id.btnRepeatMode));
        if (mBtnRepeatMode != null) {
            mBtnRepeatMode.setOnClickListener(this);
        }

        mBtnChangeLyric = findViewById(xId(R.id.btn_change_lyric));
        if (null != mBtnChangeLyric) {
            mBtnChangeLyric.setOnClickListener(this);
        }

        View progressLayout = findViewById(xId(R.id.layout_progress));
        if (progressLayout != null) {
            progressLayout.setOnTouchListener(this);
        }

        mTvCurrentTime = findViewById(xId(R.id.tvCurrentTime));
        mTvTotalTime = findViewById(xId(R.id.tvTotalTime));
        mSeekbarProgress = findViewById(xId(R.id.seekbar_progress));

        mIvPageBg = findViewById(xId(R.id.ivPageBg));
        mIvMusicImage = findViewById(xId(R.id.ivMusicImage));
        mTvSongStorage = findViewById(xId(R.id.tvSongStorage));

        if (mTvTitle != null) {
            mTvTitle.setFilterTouchesWhenObscured(true);
        }

        if (mTvArtist != null) {
            mTvArtist.setFilterTouchesWhenObscured(true);
        }

        if (mTvAlbum != null) {
            mTvAlbum.setFilterTouchesWhenObscured(true);
        }

        if (mLyricsView != null) {
            mLyricsView.setOnLrcClickListener(this::onChangeLyricView);
            mLyricsView.setOnSeekToListener(this);
            mLyricsView.SetPainTypeface(Typeface.SERIF);

            mLyricsView.SetCurPaintColor(
                    xColor(R.color.lyric_view_cur_paint_color));
            mLyricsView.SetNotCurPaintColor(
                    xColor(R.color.lyric_view_not_cur_paint_color));
        }

        initSeekbarCtrl();

        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            MusicInfo info = mAppData.musicPlayPositionInfo();
            if (mFirstChanged) {
                updateLrcRowList(info);
            }
        }

        mRlId3Info = findViewById(xId(R.id.rel_id3_infos));
        if (Customer.ZE_XUN_ELECTRONICS.equals(mCustomerName)) {
            mRlId3Info.setBackgroundResource(xDrawableId2(R.drawable.playing_bg_zxdz));
        } else {
            mRlId3Info.setBackgroundResource(xDrawableId2(R.drawable.playing_bg));
        }

        if (1 == Settings.System.getInt(
                mContext.getContentResolver(), "car_home_style", 0)) {
            mRlId3Info.setBackgroundResource(xDrawableId2(R.drawable.playing_bg_style_1));
        }

        updateMusicInfo();
        onUpdateSeekbar(mAppData.mMediaPlayState);
        onChangePlayCtrl(mAppData.mMediaPlayState);
        onChangeRepeatModeCtrl(mAppData.musicRepeatMode());
        onChangeSeekbarValue(mAppData.mPlayTimeInfo);

        return view;
    }

    private void initSeekbarCtrl() {
        mSeekbarProgress.setMax(mSeekBarMaxValue);
        mSeekbarProgress.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mSeekbarOperate = false;

                int progress = seekBar.getProgress();
                int time = mAppData.mPlayTimeInfo.mTotalTime * progress / mSeekBarMaxValue;
                playerRelay().accept(
                        t -> t.requestExecuteAction(
                                IMediaAction.seekToTime, time, null));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                LogUtil.low_i("seekBar", "onStartTrackingTouch");

                mSeekbarOperate = true;
                if (mMediaEventPostbox != null) {
                    mMediaEventPostbox.onMediaEvent(
                            IMediaEvent.EVENT_SCROLL_SEEKBAR,
                            null,
                            null);
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

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (getId(v)) {
            case R.id.rb_page1:
                mViewPagerInfo.setCurrentItem(0, true);
                break;

            case R.id.rb_page2:
                mViewPagerInfo.setCurrentItem(1, true);
                break;

            case R.id.rbShowLyric:
                if (mViewPagerInfo.getCurrentItem() == 0) {
                    mViewPagerInfo.setCurrentItem(1, true);
                } else {
                    mViewPagerInfo.setCurrentItem(0, true);
                }
                break;

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

    void onChangeLyricView() {
        if (mFreqLayout.getVisibility() == View.VISIBLE) {
            mFreqLayout.setVisibility(View.GONE);
            mLyricLayout.setVisibility(View.VISIBLE);
            if (null != mBtnChangeLyric) {
                mBtnChangeLyric.setText(xString(R3.string.frequency_label));
            }
        } else {
            mFreqLayout.setVisibility(View.VISIBLE);
            mLyricLayout.setVisibility(View.GONE);
            if (null != mBtnChangeLyric) {
                mBtnChangeLyric.setText(xString(R3.string.lyric_label));
            }
        }
    }

    private void onPlayEvent() {
        if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PAUSE)) {
            playerRelay().accept(
                    t -> t.requestPlayControl(IMusicState.PLAY_CMD_PLAY));
        } else {
            playerRelay().accept(
                    t -> t.requestPlayControl(IMusicState.PLAY_CMD_PAUSE));
        }
    }

    private void onPrevEvent() {
        if (mUserDelayHandler.hasMessages(
                UserDelayHandler.EVENT_BUTT_PREV_FILTER)
                || mUserDelayHandler.hasMessages(
                        UserDelayHandler.EVENT_BUTT_NEXT_FILTER)) {
            return;
        } else {
            mUserDelayHandler.sendEmptyMessageDelayed(
                    UserDelayHandler.EVENT_BUTT_PREV_FILTER, 200);
        }

        playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.setSeekTimeZero, null, null));
        playerRelay().accept(
                t -> t.requestPlayControl(IMusicState.PLAY_CMD_PREV));
    }

    private void onNextEvent() {
        if (mUserDelayHandler.hasMessages(UserDelayHandler.EVENT_BUTT_PREV_FILTER)
                || mUserDelayHandler.hasMessages(UserDelayHandler.EVENT_BUTT_NEXT_FILTER)) {
            return;
        } else {
            mUserDelayHandler.sendEmptyMessageDelayed(UserDelayHandler.EVENT_BUTT_NEXT_FILTER, 200);
        }

        playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.setSeekTimeZero, null, null));
        playerRelay().accept(
                t -> t.requestPlayControl(IMusicState.PLAY_CMD_NEXT));
    }

    private void onRepeatModeEvent() {
        playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.switchPlayRepeatMode, null, null));
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
            default:
                break;
        }

        if (mBtnPlay != null) {
            mBtnPlay.setBackgroundResource(xDrawableId2(nResId));
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
                nResId = xDrawableId2(R.drawable.btn_video_repeat_random_bg);
                break;
            default:
                break;
        }

        if (mBtnRepeatMode != null) {
            mBtnRepeatMode.setBackgroundResource(nResId);
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
        onUpdateSeekbar(mAppData.mMediaPlayState);
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
                    onUpdateSeekbar(mAppData.mMediaPlayState);
                }

                onChangePlayCtrl(mAppData.mMediaPlayState);
                if (!mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                    uninitVisualizer();
                }
                break;
            }

            case IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME: {
                if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_MUSIC) {
                    return;
                }

                // avoid when on play moment seek SeekBar
                if (!mSeekbarProgress.isEnabled()) {
                    onUpdateSeekbar(mAppData.mMediaPlayState);
                }

                onChangeSeekbarValue(mAppData.mPlayTimeInfo);
                Visualizer visualizer = player().getVisualizer();

                if (visualizer != null && !visualizer.getEnabled()) {
                    initVisualizer();
                }

                onChangeLyricsView(mAppData.mPlayTimeInfo);
                break;
            }

            case IMediaEvent.EVENT_CHANGE_REPEAT_MODE:
                onChangeRepeatModeCtrl(mAppData.musicRepeatMode());
                break;

            case IMediaEvent.EVENT_UPDATE_MUSIC_LIST:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
                updateMusicInfo();
                break;

            case IMediaEvent.EVENT_UPDATE_MUSIC_ID3:
                updateMusicID3Info(mAppData.mCurrentMediaInfo);
                break;

            default:
                break;
        }
    }

    private void onChangeLyricsView(MediaTimeInfo info) {
        if (mLyricsList != null && mLyricsList.size() > 0) {
            mLyricsView.seekTo(info.mCurrentTime, true, false);
        }
    }

    public void updateVisualizer(byte[] data) {
        if (mFrequencyView != null) {
            if (AutoStatus.isReversing()) {
                return;
            }

            mFrequencyView.updateVisualizer(data);
        }
    }

    private void changeStorageInfo(int nStorageType) {
        switch (nStorageType) {
            case IStorageDevice.STORAGE_TYPE_FLASH:
                mTvSongStorage.setText(xString(R3.string.storage_flash_label));
                break;

            case IStorageDevice.STORAGE_TYPE_SDCARD:
                mTvSongStorage.setText(xString(R3.string.storage_sdcard_label));
                break;

            case IStorageDevice.STORAGE_TYPE_USB:
                mTvSongStorage.setText(xString(R3.string.storage_usb1_label));
                break;

            default:
                break;
        }
    }

    private void updateMusicID3Info(MusicInfo info) {
        if (null == info) {
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

        if (TextUtils.isEmpty(info.mArtist) || "<Unknown>".equals(info.mArtist)) {
            mTvArtist.setText(text_unknown);
        } else {
            mTvArtist.setText(info.mArtist);
        }

        if (TextUtils.isEmpty(info.mAlbum) || "<Unknown>".equals(info.mAlbum)) {
            mTvAlbum.setText(text_unknown);
        } else {
            mTvAlbum.setText(info.mAlbum);
        }
    }

    private void updateMusicImage(MusicInfo info) {
        mIvMusicImage.setTag(info.mFilePath);

        // [客户定制默认专辑资源]
        if (Customer.ZE_XUN_ELECTRONICS.equals(mCustomerName)) {
            if (MusicInfo.ID3_TYPE_ERROR == info.mID3Type) {
                mIvMusicImage.setImageResource(xId(R.drawable.default_thumbnails_bg_zxdz));
            } else {
                BitmapCache.getInstance().loadNativeImage(info.mFilePath,
                        mIvMusicImage, xId(R.drawable.default_thumbnails_bg_zxdz), true);
            }
        } else {
            if (MusicInfo.ID3_TYPE_ERROR == info.mID3Type) {
                mIvMusicImage.setImageResource(xId(R.drawable.default_thumbnails_bg));
            } else {
                BitmapCache.getInstance().loadNativeImage(info.mFilePath,
                        mIvMusicImage, xId(R.drawable.default_thumbnails_bg), true);
            }
        }
    }

    private void changeMusicInfo(MusicInfo info) {
        if (null == info) {
            return;
        }

        updateMusicID3Info(info);
        updateMusicImage(info);
    }

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
            mLyricsView.setVisibility(View.VISIBLE);
            mTvNoLyrics.setVisibility(View.GONE);
            mLyricsView.setLrcRows(mLyricsList);
        } else {
            mLyricsView.setVisibility(View.GONE);
            mTvNoLyrics.setVisibility(View.VISIBLE);
        }
    }

    private void changeTotalValue(int index, int total) {
        if (total > 0) {
            String text = String.format(Locale.getDefault(), "%d/%d", index + 1, total);
            mTvTotalValue.setText(text);
        }
    }

    private void onUpdateSeekbar(int playState) {
        mSeekbarProgress.setEnabled(playState == IMusicState.E_PLAY_STATE_PLAY);
    }

    private void uninitVisualizer() {
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

            if (!mSeekbarOperate) {
                mSeekbarProgress.setProgress(value);
            }
        } else {
            mTvCurrentTime.setText("00:00");
            mTvTotalTime.setText("00:00");
            if (!mSeekbarOperate) {
                mSeekbarProgress.setProgress(0);
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

            // 128---1024
            visualizer.setCaptureSize(128);
            visualizer.setDataCaptureListener(this,
                    Visualizer.getMaxCaptureRate() / 2, false, true);

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
        playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.seekToTime, progress, null));
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        // TODO Auto-generated method stub
        LogUtil.low_i("test", "v:" + (getId(v) == R.id.layout_progress));

        if (getId(v) == R.id.layout_progress) {
            Rect seekRect = new Rect();
            mSeekbarProgress.getHitRect(seekRect);
            LogUtil.low_i("test", "bottom:" + seekRect.bottom + "  top:" + seekRect.top);

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

    @SuppressLint("InflateParams")
    private View initFreqView() {
        return xInflate(R.layout.layout_freq_lyric, null);
    }

    @SuppressLint("InflateParams")
    private View initId3View() {
        return xInflate(R.layout.layout_id3_info, null);
    }

    private class ViewPageChangeListener implements ViewPager.OnPageChangeListener {

        @Override
        public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
        }

        @Override
        public void onPageSelected(int position) {
            switch (position) {
                case 0: {
                    if (null != mRbPage1) {
                        mRbPage1.setChecked(true);
                    }
                    if (null != mRbPage2) {
                        mRbPage2.setChecked(false);
                    }
                    if (null != mRbShowLyric) {
                        mRbShowLyric.setChecked(false);
                    }
                    break;
                }
                case 1: {
                    if (null != mRbPage1) {
                        mRbPage1.setChecked(false);
                    }
                    if (null != mRbPage2) {
                        mRbPage2.setChecked(true);
                    }
                    if (null != mRbShowLyric) {
                        mRbShowLyric.setChecked(true);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        @Override
        public void onPageScrollStateChanged(int state) {
        }
    }

    private static class ViewPagerAdapter extends PagerAdapter {
        private final List<View> mViews;

        public ViewPagerAdapter(Context context, List<View> views) {
            this.mViews = views;
        }

        @Override
        public int getCount() {
            return mViews.size();
        }

        @Override
        public boolean isViewFromObject(@NonNull View view, @NonNull Object object) {
            return view == object;
        }

        @NonNull
        @Override
        public View instantiateItem(ViewGroup container, int position) {
            container.addView(mViews.get(position), LinearLayout.LayoutParams.MATCH_PARENT,
                    LinearLayout.LayoutParams.MATCH_PARENT);
            return mViews.get(position);
        }

        @Override
        public void destroyItem(ViewGroup container, int position, @NonNull Object object) {
            container.removeView((View) object);
        }
    }

    private static class UserDelayHandler extends Handler {
        private static final int EVENT_BUTT_NEXT_FILTER = 4;
        private static final int EVENT_BUTT_PREV_FILTER = 5;

        public UserDelayHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_BUTT_NEXT_FILTER:
                case EVENT_BUTT_PREV_FILTER:
                    // 过滤 Button 点击
                    break;

                default:
                    break;
            }
        }
    }
}
