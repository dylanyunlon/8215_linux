package com.hcn.media.music.base;

import android.content.Context;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.AnimationDrawable;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.Nullable;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media.base.layout.FrameLayoutEx;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media.vm.action.IPlayerEx;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.mediaservice.data.MediaTimeInfo;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_view.lyrics.LyricsManager;
import com.hcn.media_view.lyrics.LyricsRow;
import com.hcn.media_view.lyrics.LyricsView;
import com.hcn.media_view.lyrics.LyricsView.OnSeekToListener;
import com.hcn.media_view.PlayFlashView;

import java.util.List;

/**
 * mcc154 音乐歌词和频谱布局视图
 * @author 65821
 */
public class LyricSpectraLayout extends FrameLayoutEx
        implements OnItemClickListener, OnItemLongClickListener,
        OnSeekToListener {
    private static final String TAG = LyricSpectraLayout.class.getSimpleName();

    private ImageView ivShowLeft = null;
    private TextView m_tvNoLyrics = null;
    private PlayFlashView mFrequencyView = null;
    private LyricsView mLyricsView = null;
    private LyricsManager mLyricsManager = null;
    private List<LyricsRow> mLyricsList = null;
    private boolean mFirstChanged = true;

    public LyricSpectraLayout(Context context,
                              IPlayerEx player) {
        this(context, null, player);
    }

    public LyricSpectraLayout(Context context,
                              AttributeSet attrs,
                              IPlayerEx player) {
        this(context, attrs, 0, player);
    }

    public LyricSpectraLayout(Context context,
                              AttributeSet attrs,
                              int defStyle,
                              IPlayerEx player) {
        super(context, attrs, defStyle, player);

        // 歌词管理器对象（唯一实例）
        mLyricsManager = LyricsManager.instance();

        // [注意：不可以提取放到父类中使用/否则 inflate 的视图会被系统回收]
        initContentView(getLayoutRes(), this);
    }

    @Override
    protected int getLayoutRes() {
        return R.layout.layout_lrc_freq;
    }

    @Nullable
    @Override
    protected View initContentView(int layoutRes, ViewGroup root) {
        View view = super.initContentView(layoutRes, root);

        // 显示提示
        assert view != null;
        ivShowLeft = findViewById(xId(R.id.ivLeftShow));
        m_tvNoLyrics = findViewById(xId(R.id.tvNoLyrics));
        mFrequencyView = findViewById(xId(R.id.FrequencyView));
        mLyricsView = findViewById(xId(R.id.lyrics_view));
        mLyricsView.setOnSeekToListener(this);
        mLyricsView.SetPainTypeface(Typeface.SERIF);
        mLyricsView.SetCurPaintColor(Color.YELLOW);
        mLyricsView.SetNotCurPaintColor(Color.GREEN);

        initLayout();
        return view;
    }

    public void setAnimationState(boolean start) {
        if (null != ivShowLeft) {
            AnimationDrawable left = (AnimationDrawable) ivShowLeft.getBackground();
            if (null != left) {
                if (start && !left.isRunning()) {
                    left.start();
                } else if (!start && left.isRunning()) {
                    left.stop();
                }
            }
        }
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();

    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        setAnimationState(false);
    }

    @Override
    public void onSeekTo(int progress) {
        playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.seekToTime, progress, null));
    }

    public void updateVisualizer(byte[] data) {
        if (mFrequencyView != null) {
            mFrequencyView.updateVisualizer(data);
        }
    }

    /**
     * 触发指定位置的媒体对象
     * <pre>
     *    在这个类中，这个函数的设计绝对是狗屎；
     *    就不删除了，以后当设计典型讲（全局变量的使用大忌）；
     * </pre>
     *
     * @param position
     */
    private void onChangeMusicEvent(int position) {
        // 非常不连贯的设计，这里就赋值全局变量导致体验非常差
        if (position < mAppData.musicPlaylist().size()) {
            mAppData.updateMusicPlayPosition(position);
            mAppData.mMusicPlayIndex = mAppData.musicPlaylist().get(position).mIndex;
        }
        
        if (mMediaEventPostbox != null) {
            mMediaEventPostbox.onMediaEvent(
                    IMediaEvent.EVENT_CHANGE_MUSIC_ITEM, position, null);
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
    }

    @Override
    public void doCallbackEvent(int eventId) {
        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
                updateMusicInfo();
                break;
            case IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME:
                onChangeLyricsView(mAppData.mPlayTimeInfo);
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

    private void updateMusicInfo() {
        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            MusicInfo info = mAppData.musicPlayPositionInfo();
            updateLrcRowList(info);
        }
    }

    private void updateLrcRowList(MusicInfo info) {
        mFirstChanged = false;
        mLyricsList = mLyricsManager.getLrcFromSong(info.mFilePath);
        if (mLyricsList != null && mLyricsList.size() > 0) {
            mLyricsView.setVisibility(View.VISIBLE);
            m_tvNoLyrics.setVisibility(View.GONE);
            mLyricsView.setLrcRows(mLyricsList);
        } else {
            mLyricsView.setVisibility(View.GONE);
            m_tvNoLyrics.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        onChangeMusicEvent(position);
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
        if (mAppData.mCurrentDevice != null) {
            MusicInfo info = mAppData.musicPlaylist().get(position);
            if (!info.mFavorite) {
                info.mFavorite = true;
                if (!mAppData.mCurrentDevice.mMusicFavoriteList.contains(info)) {
                    mAppData.mCurrentDevice.mMusicFavoriteList.add(info);
                }
            } else {
                info.mFavorite = false;
                mAppData.mCurrentDevice.mMusicFavoriteList.remove(info);
            }
        }

        return true;
    }
}
