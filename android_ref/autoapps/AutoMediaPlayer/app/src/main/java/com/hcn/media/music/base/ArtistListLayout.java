package com.hcn.media.music.base;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ExpandableListView;
import android.widget.ExpandableListView.OnChildClickListener;
import android.widget.ExpandableListView.OnGroupCollapseListener;
import android.widget.ExpandableListView.OnGroupExpandListener;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media.R3;
import com.hcn.media.adapter.ExpandableListViewAdapter;
import com.hcn.media.base.layout.FrameLayoutEx;
import com.hcn.media.vm.action.IPlayerEx;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_data.base.MusicKeyInfo;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.List;

/**
 * 艺术家列表布局视图
 * @author 65821
 */
public class ArtistListLayout extends FrameLayoutEx
        implements OnGroupCollapseListener, OnGroupExpandListener,
        OnChildClickListener, OnItemClickListener, OnScrollListener {
    private static final String TAG = AlbumListLayout.class.getSimpleName();

    private ExpandableListView mExpandableListView = null;
    private ExpandableListViewAdapter mListViewAdapter = null;
    private TextView mShowTipCtrl = null;

    public ArtistListLayout(Context context,
                            @NonNull IPlayerEx player) {
        this(context, null, player);
    }

    public ArtistListLayout(Context context,
                            AttributeSet attrs,
                            @NonNull IPlayerEx player) {
        this(context, attrs, 0, player);
    }

    public ArtistListLayout(Context context,
                            AttributeSet attrs,
                            int defStyle,
                            @NonNull IPlayerEx player) {
        super(context, attrs, defStyle, player);

        // [注意：不可以提取放到父类中使用/否则 inflate 的视图会被系统回收]
        initContentView(getLayoutRes(), this);
    }

    @Override
    protected int getLayoutRes() {
        return R.layout.layout_listalbum_multi;
    }

    @Nullable
    @Override
    protected View initContentView(int layoutRes, ViewGroup root) {
        View view = super.initContentView(layoutRes, root);

        // 显示提示
        assert view != null;
        mShowTipCtrl = findViewById(xId(R.id.tvShowTip));

        mExpandableListView = findViewById(xId(R.id.gridview_album));
        if (mExpandableListView != null) {
            mListViewAdapter = new ExpandableListViewAdapter(
                    mContext, R.drawable.icon_list_artist);
            mExpandableListView.setAdapter(mListViewAdapter);
            mExpandableListView.setOnGroupCollapseListener(this);
            mExpandableListView.setOnGroupExpandListener(this);
            mExpandableListView.setOnChildClickListener(this);
            mExpandableListView.setVerticalScrollBarEnabled(false);
            mExpandableListView.setOnScrollListener(this);
        }

        initLayout();
        return view;
    }

    @Override
    public void initLayout() {
        setPlayingFilePath();
        updatePromptInfo();

        // 延时列表的刷新动作
        removeCallbacks(this::updateDataList);
        postDelayed(this::updateDataList, 10);
    }

    public void updateDataList() {
        if (null != mListViewAdapter) {
            if (mAppData.mSelectedDevice != null) {
                mListViewAdapter.setDataList(
                        mAppData.mSelectedDevice.mArtistListMap);
            }

            mListViewAdapter.notifyDataSetChanged();
        }
    }

    private void updatePromptInfo() {
        if (mAppData.mSelectedDevice != null) {
            if (mAppData.mSelectedDevice.isLoading()) {
                mShowTipCtrl.setText(xString(R3.string.tip_loading));
                mShowTipCtrl.setVisibility(View.VISIBLE);
            } else if (!mAppData.mSelectedDevice.mID3ParseState.mIsLoadFinished) {
                mShowTipCtrl.setText(xString(R3.string.tip_loading));
                mShowTipCtrl.setVisibility(View.VISIBLE);
            } else {
                mShowTipCtrl.setText(xString(R3.string.tip_no_music_file));
                boolean bShowTip = mAppData.mSelectedDevice.mArtistListMap.isEmpty();
                mShowTipCtrl.setVisibility(bShowTip ? View.VISIBLE : View.GONE);
            }
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
                updatePromptInfo();
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
            mListViewAdapter.setPlayingFilePath(
                    mAppData.mCurrentMediaInfo.mFilePath);
        } catch (Exception ignored) {
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        if (mAppData.mSelectedDevice != null) {
            List<MusicInfo> infoList = mAppData.mSelectedDevice.mMusicInfoList;
            mAppData.mCurrentDevice = mAppData.mSelectedDevice;
            playerRelay().accept(t -> t.requestPlayTarget(
                    IPlaylistType.DEVICE_LIST, infoList, position));
        }
    }

    @Override
    public boolean onChildClick(ExpandableListView parent, View v, int groupPosition,
            int childPosition, long id) {
        if (mAppData.mSelectedDevice != null) {
            MusicKeyInfo keyInfo = mListViewAdapter.getItemInfo(groupPosition);
            if (keyInfo == null) {
                return false;
            }

            List<MusicInfo> infoList = keyInfo.mInfoList;
            if (infoList != null) {
                mAppData.mCurrentDevice = mAppData.mSelectedDevice;
                playerRelay().accept(t -> t.requestPlayTarget(
                        IPlaylistType.DEVICE_LIST, infoList, childPosition));
            }
        }
        return false;
    }

    @Override
    public void onGroupCollapse(int groupPosition) {
    }

    @Override
    public void onGroupExpand(int groupPosition) {
        for (int i = 0; i < mListViewAdapter.getGroupCount(); i++) {
            // ensure only one expanded Group exists at every time
            if (groupPosition != i && mExpandableListView.isGroupExpanded(groupPosition)) {
                mExpandableListView.collapseGroup(i);
            }
        }
    }

    @Override
    public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,
            int totalItemCount) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onScrollStateChanged(AbsListView view, int scrollState) {
        if (scrollState == OnScrollListener.SCROLL_STATE_IDLE) {
            mListViewAdapter.setScrollState(false);
            mListViewAdapter.notifyDataSetChanged();
        } else {
            mListViewAdapter.setScrollState(true);
        }
    }
    @Override
    public void onUpdateUiModeView(boolean isNightMode) {
        super.onUpdateUiModeView(isNightMode);

        //通知适配器更新刷新，重新设置资源
        mListViewAdapter.notifyDataSetChanged();
    }

}
