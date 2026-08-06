package com.hcn.media.music.gallery;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media.R3;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media.adapter.MusicSongListAdapter;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media.base.layout.FrameLayoutEx;
import com.hcn.media.vm.action.IPlayerEx;
import com.hcn.media_view.widget.ListViewEx;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.List;
import java.util.Objects;

/**
 * mcc154 音乐列表布局视图
 * @author 65821
 */
@SuppressLint("ViewConstructor")
public class MusicListLayoutEx extends FrameLayoutEx
        implements  OnItemClickListener, OnItemLongClickListener, OnScrollListener {
    private static final String TAG = MusicListLayoutEx.class.getSimpleName();

    private ListViewEx mMusicGridView = null;
    private MusicSongListAdapter mMusicListAdapter = null;
    private TextView mTvShowPrompt = null;

    public MusicListLayoutEx(Context context,
                             @NonNull IPlayerEx player) {
        this(context, null, player);
    }

    public MusicListLayoutEx(Context context,
                             AttributeSet attrs,
                             @NonNull IPlayerEx player) {
        this(context, attrs, 0, player);
    }

    public MusicListLayoutEx(Context context,
                             AttributeSet attrs,
                             int defStyle,
                             @NonNull IPlayerEx player) {
        super(context, attrs, defStyle, player);

        // [注意：不可以提取放到父类中使用/否则 inflate 的视图会被系统回收]
        initContentView(getLayoutRes(), this);
    }

    @Override
    protected int getLayoutRes() {
        return R.layout.layout_listmusic;
    }

    @Nullable
    @Override
    protected View initContentView(int layoutRes, ViewGroup root) {
        LogUtil.d(TAG, "initContentView.");
        View view = super.initContentView(layoutRes, root);

        // 显示提示
        assert view != null;
        mTvShowPrompt = findViewById(xId(R.id.tvShowTip));

        // Debug: ListViewEx.setDebug(Utils.isDebugVersion());
        mMusicGridView = findViewById(xId(R.id.gridview_music));
        if (mMusicGridView != null) {
            mMusicListAdapter = new MusicSongListAdapter(mContext, mMusicGridView);
            mMusicGridView.setAdapter(mMusicListAdapter);
            mMusicGridView.setOnItemClickListener(this);
            mMusicGridView.setOnScrollListener(this);
        }

        return view;
    }

    @Override
    public void initDataObject() {
        super.initDataObject();

        // 更新布局信息元素
        updateLayoutInfo("init");
    }

    @Override
    public void initLayout() {
        updateLayoutInfo("init-layout");
    }

    /** 更新布局视图的显示信息 **/
    private void updateLayoutInfo(String reason) {
        if (Objects.isNull(mAppData.mSelectedDevice)) {
            return;
        }

        updatePromptInfo(reason);
        setPlayingFilePath();

        // 延时列表的刷新动作
        removeCallbacks(this::updateDataList);
        postDelayed(this::updateDataList, 10);
    }

    /** 更新当前列表视图显示信息 **/
    public void updateDataList() {
        if (Objects.isNull(mAppData.mSelectedDevice)) {
            return;
        }

        if (null != mMusicListAdapter) {
            mMusicListAdapter.setDataList(mAppData.mSelectedDevice.mMusicInfoList);
            mMusicListAdapter.notifyDataSetChanged();
        }
    }

    /** 更新提示信息（文字信息） **/
    private void updatePromptInfo(String reason) {
        if (Objects.isNull(mAppData.mSelectedDevice)) {
            return;
        }

        LogUtil.d(TAG, "updateListStatus, reason: " + reason);

        if (mAppData.mSelectedDevice.isLoading()) {
            mTvShowPrompt.setText(getString(R3.string.tip_loading));
            mTvShowPrompt.setVisibility(View.VISIBLE);
        } else {
            mTvShowPrompt.setText(getString(R3.string.tip_no_music_file));
            boolean bShowTip = mAppData.mSelectedDevice.mMusicInfoList.isEmpty();
            mTvShowPrompt.setVisibility(bShowTip ? View.VISIBLE : View.GONE);
        }
    }

    @Override
    public void doCallbackEvent(int eventId) {
        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE:
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
                setPlayingFilePath();
                updateDataList();
                break;
            case IMediaEvent.EVENT_MEDIA_LOADING_START:
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
            case IMediaEvent.EVENT_CHANGE_MUSIC_STORAGE:
            case IMediaEvent.EVENT_ID3_SCAN_FINISHED:
                updateDataList();
                updatePromptInfo("event: " + eventId);
                break;
            default:
                break;
        }
    }

    /**
     * 设置当前播放曲目路径并更新列表
     */
    private void setPlayingFilePath() {
        try {
            mMusicListAdapter.setPlayingFilePath(
                    mAppData.mCurrentMediaInfo.mFilePath);
        } catch (Exception ignored) {
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        LogUtil.v(TAG, "onItemClick: " + position);

        if (mAppData.mSelectedDevice != null) {
            List<MusicInfo> infoList = mAppData.mSelectedDevice.mMusicInfoList;
            mAppData.mCurrentDevice = mAppData.mSelectedDevice;
            playerRelay().accept(t -> t.requestPlayTarget(
                    IPlaylistType.DEVICE_LIST, infoList, position));
        }
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
        LogUtil.v(TAG, "onItemLongClick: " + position);

        if (mAppData.mSelectedDevice != null) {
            List<MusicInfo> infoList = mAppData.mSelectedDevice.mMusicInfoList;
            MusicInfo info = infoList.get(position);
            if (info.mFavorite) {
                info.mFavorite = false;
                mAppData.mSelectedDevice.mMusicFavoriteList.remove(info);
            } else {
                info.mFavorite = true;
                if (!mAppData.mSelectedDevice.mMusicFavoriteList.contains(info)) {
                    mAppData.mSelectedDevice.mMusicFavoriteList.add(info);
                }
            }
        }
        return true;
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {
        boolean intercepted = super.onInterceptTouchEvent(ev);
        LogUtil.v(TAG, "onInterceptTouchEvent: " +
                "Action = " + ev.getAction() + ", intercepted = " + intercepted);
        return intercepted;
    }

    @Override
    public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,
            int totalItemCount) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onScrollStateChanged(AbsListView view, int scrollState) {
        LogUtil.v(TAG, "onScrollStateChanged: scrollState = " + scrollState);

        if (scrollState == OnScrollListener.SCROLL_STATE_IDLE) {
            mMusicListAdapter.setScrollState(false);
            mMusicListAdapter.notifyDataSetChanged();
        } else {
            mMusicListAdapter.setScrollState(true);
        }
    }

    @Override
    public void onUpdateUiModeView(boolean isNightMode) {
        super.onUpdateUiModeView(isNightMode);

        // 通知适配器更新
        mMusicListAdapter.notifyDataSetChanged();
    }
}
