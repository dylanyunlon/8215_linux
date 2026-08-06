package com.hcn.media.music.gallery;

import static android.carsource.McuConstant.K_EQ;

import android.annotation.SuppressLint;
import android.carsource.McuManager;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.media.audiofx.AudioEffect;
import android.media.audiofx.Visualizer;
import android.media.audiofx.Visualizer.OnDataCaptureListener;
import android.os.Bundle;
import android.sourceservice.ExtAudioMuxer;

import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.viewpager.widget.PagerAdapter;
import androidx.viewpager.widget.ViewPager;

import com.auto.transformer.AccordionTransformer;
import com.hcn.AutoMediaPlayer.R;
import com.hcn.common.lang.RunnableEx;
import com.hcn.common.misc.LogUtils;
import com.hcn.media.R3;
import com.hcn.media.extend.base.IExtend;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.fragment.IMusicPage;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_data.storage.IStorageDevice;
import com.hcn.media.music.base.GalleryInfoLayout;
import com.hcn.media.music.base.LyricSpectraLayout;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.mediaservice.data.MediaTimeInfo;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.IMediaEventListener;
import com.hcn.media.local.utils.HFuncUtils;
import com.hcn.common.Utility;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_view.HTextView;
import com.hcn.plugin.ApkClassLoaderEx;

import java.util.Locale;
import java.util.Objects;

/**
 * mcc154 音乐信息页面
 * <p> 带 Gallery 画廊效果的播放页面；
 *
 * @author 65821
 */
@SuppressWarnings("deprecation")
public class MusicInfoFragmentEx extends MediaFragment
        implements OnClickListener, ViewPager.OnPageChangeListener,
        OnDataCaptureListener {
    private final static String FRAGMENT_NAME = "music-info-mcc154";
    private static final String TAG = MusicInfoFragmentEx.class.getSimpleName();

    private static final int mSeekBarMaxValue = 1000;

    private boolean mCreateView = false;
    private boolean mInitView = false;

    private ViewPager mViewPager = null;
    private ViewPaperAdapter mViewPaperAdapter;

    private Button mBtnPlay = null;
    private Button mBtnRepeatMode = null;
    private SeekBar mSeekbarProgress = null;
    private TextView mTvCurrentTime = null;
    private TextView mTvTotalTime = null;

    private ImageView mIvPageBg = null;
    private ImageView mIvMusicImage = null;
    private TextView mTvSongStorage = null;
    private HTextView mTvTitle = null;
    private HTextView mTvArtist = null;
    private HTextView mTvAlbum = null;
    private TextView mTvTotalValue = null;

    private boolean mSeekbarOperate = false;
    private GalleryInfoLayout mMusicInfoLayout = null;
    private LyricSpectraLayout mMcc154LrcOrFreqLayout = null;

    public MusicInfoFragmentEx(IMediaEventListener listener) {
        super(FRAGMENT_NAME);

        mListener = listener;
        mViewPaperAdapter = null;

        // 根据皮肤包 music_info_page_extend 配置，加载 MusicInfoPageExtend，调用皮肤包里面的方法
        String pageExtendResConfigName = "music_info_page_extend";
        if (xBoolean(pageExtendResConfigName)) {
            ApkClassLoaderEx classLoader = xClassLoader();
            if (!Objects.isNull(classLoader)) {
                String pageExtendClassName =
                        IExtend.MUSIC_PACKAGE_NAME + ".MusicInfoPageExtend";
                mPageExtend = classLoader.newPageExtendInterface(pageExtendClassName, this);
            }

            LogUtils.iTag(TAG, mPageExtend != null?
                    "Has MusicInfoPageExtend class.": "No MusicInfoPageExtend class.");
        }
    }

    @Override
    public void initFragment() {
        if (!mCreateView) {
            return;
        }

        if (mAppData.mCurrentDevice != null) {
            changeStorageInfo(mAppData.mCurrentDevice.storageType());
        }

        updateMusicInfo();
        onUpdateSeekbar(mAppData.mMediaPlayState);
        onChangePlayCtrl(mAppData.mMediaPlayState);
        onChangeRepeatModeCtrl(mAppData.musicRepeatMode());
        onChangeSeekbarValue(mAppData.mPlayTimeInfo);


        if (mMusicInfoLayout != null) {
            mMusicInfoLayout.initLayout();
        }

        if (mMcc154LrcOrFreqLayout != null) {
            mMcc154LrcOrFreqLayout.initLayout();
        }

        int position = mViewPager.getCurrentItem();
        updatePageCtrl(position);
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

    @Override
    public void uninitFragment() {
        mAppData.mIsControlPage = false;
        uninitVisualizer();
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fragment_musicinfo;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);

        assert view != null;
        initView(view);

        mCreateView = true;
        initFragment();
        mInitView = true;
        return view;
    }

    private void initView(View layout) {
        Log.d(TAG, "initView: " + layout.getContentDescription());

        // 菜单按钮
        View list = layout.findViewById(xId(R.id.btnList));
        if (list != null) {
            list.setOnClickListener(this);
        }

        View prev = layout.findViewById(xId(R.id.btnPrev));
        if (prev != null) {
            prev.setOnClickListener(this);
        }

        View next = layout.findViewById(xId(R.id.btnNext));
        if (next != null) {
            next.setOnClickListener(this);
        }

        View eq = layout.findViewById(xId(R.id.btnEQ));
        if (eq != null) {
            eq.setOnClickListener(this);
        }

        View page0 = layout.findViewById(xId(R.id.btnPage0));
        if (page0 != null) {
            page0.setOnClickListener(this);
        }

        View page1 = layout.findViewById(xId(R.id.btnPage1));
        if (page1 != null) {
            page1.setOnClickListener(this);
        }

        mBtnPlay = layout.findViewById(xId(R.id.btnPlay));
        mBtnPlay.setOnClickListener(this);

        mBtnRepeatMode = layout.findViewById(xId(R.id.btnRepeatMode));
        mBtnRepeatMode.setOnClickListener(this);

        // 进度信息
        mTvCurrentTime = layout.findViewById(xId(R.id.tvCurrentTime));
        mTvTotalTime = layout.findViewById(xId(R.id.tvTotalTime));
        mSeekbarProgress = layout.findViewById(xId(R.id.seekbar_progress));

        // ID3 信息
        mIvPageBg = layout.findViewById(xId(R.id.ivPageBg));
        mIvMusicImage = layout.findViewById(xId(R.id.ivMusicImage));
        mTvSongStorage = layout.findViewById(xId(R.id.tvSongStorage));
        mTvTitle = layout.findViewById(xId(R.id.tvTitle));
        mTvArtist = layout.findViewById(xId(R.id.tvArtist));
        mTvAlbum = layout.findViewById(xId(R.id.tvAlbum));
        mTvTotalValue = layout.findViewById(xId(R.id.tvTotalValue));

        mTvTitle.setFilterTouchesWhenObscured(true);
        mTvArtist.setFilterTouchesWhenObscured(true);
        mTvAlbum.setFilterTouchesWhenObscured(true);

        // 播放列表画廊
        mViewPager = layout.findViewById(xId(R.id.viewpager_center));
        if (mViewPager != null) {
            mViewPaperAdapter = new ViewPaperAdapter(mContext);
            mViewPager.setAdapter(mViewPaperAdapter);
            mViewPager.setOnPageChangeListener(this);
            mViewPager.setPageTransformer(true, new AccordionTransformer());
        }

        initSeekbarProgress();
    }

    private void initSeekbarProgress() {
        mSeekbarProgress.setMax(mSeekBarMaxValue);

        // 进度条改变监听
        mSeekbarProgress.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mSeekbarOperate = false;
                int progress = seekBar.getProgress();
                if (progress < 100) {
                    progress = progress + 1;
                }

                int time = mAppData.mPlayTimeInfo.mTotalTime * progress / mSeekBarMaxValue;
                mMusicViewModel.playerRelay().accept(
                        t -> t.requestExecuteAction(
                                IMediaAction.seekToTime, time, null));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                mSeekbarOperate = true;
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                // TODO Auto-generated method stub
            }
        });
    }

    @Override
    public void onResume() {
        super.onResume();

        updateMusicBottomMenu(isOrientation(Configuration.ORIENTATION_LANDSCAPE));
        int position = mViewPager.getCurrentItem();
        updatePageCtrl(position);
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
                mMusicViewModel.fragment2MainUi().execute(
                        t -> t.onEvent(eventId, wParam, lParam));
                break;
            case IMediaEvent.EVENT_NONE:
            default:
                break;
        }
    }

    @Override
    protected void onOrientationChangedEvent(@NonNull Configuration newConfig) {
        super.onOrientationChangedEvent(newConfig);

        // 横竖屏变化
        boolean landscape = newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE;
        updateMusicBottomMenu(landscape);
    }

    /**
     * 更新音乐播放界面底部菜单元素
     * <p> 横竖屏切换时候使用，隐藏部分菜单按钮；

     * @param landscape 是横屏显示与否
     */
    private void updateMusicBottomMenu(boolean landscape) {
        View contentView = getView();
        if (contentView != null) {
            // 隐藏显示 EQ 按钮。
            View eqButt = contentView.findViewById(xId(R.id.btnEQ));
            eqButt.setVisibility(landscape? View.VISIBLE: View.GONE);

            // 有些旧的布局，EQ 按钮有外壳布局，也需要脱壳。
            View enclosureView = getView().findViewById(xId(R.id.btnEQ_Enclosure));
            if (enclosureView != null) {
                enclosureView.setVisibility(landscape ? View.VISIBLE : View.GONE);
            }
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        mAppData.mIsControlPage = false;
    }

    @Override
    protected void onHandlePageEvent(int event, Object obj1, Object obj2) {
        if (!mInitView) {
            return;
        }

        switch (event) {
            case IMediaEvent.EVENT_UPDATE_MUSIC_ID3:
                updateMusicTextInfo();
                break;
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
            default:
                break;
        }
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
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
                if (!mSeekbarProgress.isEnabled()) {
                    // avoid when on play moment seek SeekBar
                    onUpdateSeekbar(mAppData.mMediaPlayState);
                }
                onChangeSeekbarValue(mAppData.mPlayTimeInfo);
                Visualizer visualizer = mMusicViewModel.getVisualizer();
                if (visualizer != null && !visualizer.getEnabled()) {
                    initVisualizer();
                }
                break;
            }
            case IMediaEvent.EVENT_CHANGE_REPEAT_MODE: {
                onChangeRepeatModeCtrl(mAppData.musicRepeatMode());
                break;
            }
            case IMediaEvent.EVENT_UPDATE_MUSIC_LIST:
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM: {
                updateMusicInfo();
                break;
            }
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST: {
                if (mAppData.mCurrentDevice != null) {
                    changeStorageInfo(mAppData.mCurrentDevice.storageType());
                }
                break;
            }
            default:
                break;
        }

        if (mMusicInfoLayout != null) {
            mMusicInfoLayout.doCallbackEvent(eventId);
        }

        if (mMcc154LrcOrFreqLayout != null) {
            mMcc154LrcOrFreqLayout.doCallbackEvent(eventId);
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

    private void onUpdateSeekbar(int playState) {
        mSeekbarProgress.setEnabled(playState == IMusicState.E_PLAY_STATE_PLAY);
    }

    private void updatePageCtrl(int position) {
        if (position == 0) {
            if (null != mMusicInfoLayout) {
                mMusicInfoLayout.setAnimationState(true);
            }

            if (null != mMcc154LrcOrFreqLayout) {
                mMcc154LrcOrFreqLayout.setAnimationState(false);
            }

            mIvPageBg.setBackgroundResource(xDrawableId2(R.drawable.page_1));
            mAppData.mIsControlPage = true;
            uninitVisualizer();
        } else {
            if (null != mMusicInfoLayout) {
                mMusicInfoLayout.setAnimationState(false);
            }

            if (null != mMcc154LrcOrFreqLayout) {
                mMcc154LrcOrFreqLayout.setAnimationState(true);
            }

            mIvPageBg.setBackgroundResource(xDrawableId2(R.drawable.page_2));
            mAppData.mIsControlPage = false;
            mAppData.updateMusicSelectPosition(-1);
        }
    }

    private void initVisualizer() {
        if (mViewPager.getCurrentItem() == 0) {
            return;
        }

        if (!mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
            return;
        }

        Visualizer visualizer = mMusicViewModel.getVisualizer();
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

    private void uninitVisualizer() {
        Visualizer visualizer = mMusicViewModel.getVisualizer();
        if (visualizer != null) {
            visualizer.setDataCaptureListener(null,
                    Visualizer.getMaxCaptureRate() / 2, false, false);
            if (visualizer.getEnabled()) {
                visualizer.setEnabled(false);
            }
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LogUtil.d(TAG, "onDestroyView.");

        mInitView = false;
        mViewPager.removeAllViews();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        uninitVisualizer();
    }

    @Override
    public void onPageScrolled(int arg0, float arg1, int arg2) {
        uninitVisualizer();
    }

    @Override
    public void onPageScrollStateChanged(int arg0) {
    }

    @Override
    public void onPageSelected(int position) {
        // TODO Auto-generated method stub
        updatePageCtrl(position);
    }

    @Override
    public void onFftDataCapture(Visualizer visualizer, byte[] fft, int samplingRate) {
        if (mMcc154LrcOrFreqLayout != null) {
            mMcc154LrcOrFreqLayout.updateVisualizer(fft);
        }
    }

    @Override
    public void onWaveFormDataCapture(Visualizer visualizer, byte[] waveform, int samplingRate) {
        if (mMcc154LrcOrFreqLayout != null) {
            mMcc154LrcOrFreqLayout.updateVisualizer(waveform);
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (getId(v)) {
            case R.id.btnList:
                onListEvent();
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
            case R.id.btnPage0:
                onChangePageEvent(0);
                break;
            case R.id.btnPage1:
                onChangePageEvent(1);
                break;
            default:
                break;
        }
    }

    private void onListEvent() {
        if (null != mContext) {
            mAppData.mSelectedDevice = mAppData.mCurrentDevice;
            mMusicViewModel.fragment2MainUi().execute(
                    t -> t.onEvent(IMediaEvent.EVENT_SHOW_MUSIC_FRAGMENT,
                            IMusicPage.E_GROUP_SHOW_MUSIC_LIST_EX, null));
        }
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
        requestMediaAction(IMediaAction.setSeekTimeZero);
        requestMediaAction(IMediaAction.playControl, IMusicState.PLAY_CMD_PREV);
    }

    private void onNextEvent() {
        requestMediaAction(IMediaAction.setSeekTimeZero);
        requestMediaAction(IMediaAction.playControl, IMusicState.PLAY_CMD_NEXT);
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
                requireActivity().startActivity(intent);
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
                nResId = R.drawable.btn_play_bg;
                break;
            case IMusicState.E_PLAY_STATE_PLAY:
            default:
                nResId = R.drawable.btn_pause_bg;
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
                nResId = R.drawable.btn_repeat_queue_bg;
                break;
            case IMusicState.REPEAT_MODE_ALL:
                nResId = R.drawable.btn_repeat_all_bg;
                break;
            case IMusicState.REPEAT_MODE_ONE:
                nResId = R.drawable.btn_repeat_one_bg;
                break;
            case IMusicState.REPEAT_MODE_RANDOM:
                nResId = R.drawable.btn_video_repeat_random_bg;
                break;
            default:
                break;
        }

        if (mBtnRepeatMode != null) {
            mBtnRepeatMode.setBackgroundResource(xDrawableId2(nResId));
        }
    }

    private void updateMusicInfo() {
        // 检查索引的有效性
        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            MusicInfo info = mAppData.musicPlayPositionInfo();
            changeMusicInfo(info);
            changeTotalValue(mAppData.musicPlayPosition(), mAppData.musicPlaylist().size());
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
            MusicInfo info = mAppData.musicPlayPositionInfo();
            updateMusicId3Text(info);
            changeTotalValue(mAppData.musicPlayPosition(), mAppData.musicPlaylist().size());

            // 可以去更新下 Gallery 的缩略图
            if (mMusicInfoLayout != null
                    && info.mID3Type == MusicInfo.ID3_TYPE_EXTRACTED) {
                // MusicGalleryAdapter 更新有条件判断
                mMusicInfoLayout.updateAlbumCover((String) info.mFilePath);
            }
        }
    }

    private void changeStorageInfo(int nStorageType) {
        switch (nStorageType) {
            case IStorageDevice.STORAGE_TYPE_FLASH:
                mTvSongStorage.setText(getText(R3.string.storage_flash_label));
                break;
            case IStorageDevice.STORAGE_TYPE_SDCARD:
                mTvSongStorage.setText(getText(R3.string.storage_sdcard_label));
                break;
            case IStorageDevice.STORAGE_TYPE_USB:
                mTvSongStorage.setText(getText(R3.string.storage_usb1_label));
                break;
            default:
                break;
        }
    }

    private void changeTotalValue(int index, int total) {
        if (total > 0) {
            String text = String.format(Locale.getDefault(), "%d/%d", index + 1, total);
            mTvTotalValue.setText(text);
        }
    }

    private void changeMusicInfo(MusicInfo info) {
        if (info != null) {
            updateMusicImage(info);
            updateMusicId3Text(info);
        }
    }

    /**
     * 更新 ID3 文字信息
     *
     * @param info 需要更新的歌曲信息
     */
    private void updateMusicId3Text(MusicInfo info) {
        if (Objects.isNull(info)) {
            return;
        }

        String text_unknown = getString(R3.string.text_unknown);
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

        if (info.mID3Type == MusicInfo.ID3_TYPE_ERROR) {
            mIvMusicImage.setImageResource(xDrawableId2(R.drawable.default_thumbnails_bg));
        } else {
            BitmapCache.getInstance().loadNativeImage(
                    info.mFilePath, mIvMusicImage, xDrawableId2(R.drawable.default_thumbnails_bg),
                    new RunnableEx() {
                        @Override
                        public void callback(Object obj) {
                            super.callback(obj);

                            // 解析到了缩略图（从无要有）
                            if (mMusicInfoLayout != null
                                    && info.mID3Type == MusicInfo.ID3_TYPE_EXTRACTED) {
                                mMusicInfoLayout.updateAlbumCover((String) obj);
                            }
                        }
                    }, true);
        }
    }

    private void onChangePageEvent(int index) {
        if (mViewPager != null) {
            mViewPager.setCurrentItem(index);
        }
    }

    /**
     * 画廊适配器
     * <p> 显示当前播放列表信息；
     */
    private class ViewPaperAdapter extends PagerAdapter {
        static final int VIEW_TAG_KEY = 0xEE00F001;

        public ViewPaperAdapter(Context context) {
        }

        @Override
        public int getCount() {
            return 2;
        }

        @Override
        public boolean isViewFromObject(@NonNull View view, @NonNull Object object) {
            return view == object;
        }

        @NonNull
        @Override
        public View instantiateItem(@NonNull ViewGroup container, int position) {
            View view;
            if (position == 0) {
                if (mMusicInfoLayout == null) {
                    mMusicInfoLayout = new GalleryInfoLayout(mContext, mMusicViewModel);
                    mMusicInfoLayout.setTag(VIEW_TAG_KEY, "info");
                    mMusicInfoLayout.setMediaEventListener(mPostbox);
                } else {
                    mMusicInfoLayout.initLayout();
                }

                view = mMusicInfoLayout;
            } else {
                if (mMcc154LrcOrFreqLayout == null) {
                    mMcc154LrcOrFreqLayout = new LyricSpectraLayout(mContext, mMusicViewModel);
                    mMcc154LrcOrFreqLayout.setTag(VIEW_TAG_KEY, "lrc-freq");
                    mMcc154LrcOrFreqLayout.setMediaEventListener(mPostbox);
                } else {
                    mMcc154LrcOrFreqLayout.initLayout();
                }

                view = mMcc154LrcOrFreqLayout;
            }

            assert view != null;
            String viewTag = (String) view.getTag(VIEW_TAG_KEY);
            int childCount = container.getChildCount();
            LogUtil.v(TAG, "instantiateItem: "
                    + "position = " + position + "/" + childCount +  ", tag = " + viewTag);

            // 重复添加 view 到父对象将导致 IllegalStateException
            if (view.getParent() == null) {
                container.addView(view, LinearLayout.LayoutParams.MATCH_PARENT,
                        LinearLayout.LayoutParams.MATCH_PARENT);
            } else {
                LogUtil.w(TAG, "instantiateItem: The specified child already has a parent.");
            }

            return view;
        }

        @Override
        public void destroyItem(ViewGroup container, int position, @NonNull Object object) {
            View view = (View) object;
            int childCount = container.getChildCount();
            LogUtil.v(TAG, "destroyItem: "
                    + "position = " + position + "/" + childCount
                    +  ", tag = " + view.getTag(VIEW_TAG_KEY));

            container.removeView(view);
        }
    }

    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        // 分屏的时候大多数情况好像收不到回调
        LogUtil.d(TAG, "onConfigurationChanged: newConfig = " + newConfig.toString());

        // mWallpaperSelectorLayout 需要隐藏掉
        hideWallpaperLayout();
    }

    public void hideWallpaperLayout() {
        // 具体业务逻辑由皮肤包去实现
        if (mPageExtend != null) {
            String result = mPageExtend.tryCallMethod("hideWallpaperLayout");
            LogUtils.vTag(TAG, "tryCallMethod/hideWallpaperLayout: " + result);
        }
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
