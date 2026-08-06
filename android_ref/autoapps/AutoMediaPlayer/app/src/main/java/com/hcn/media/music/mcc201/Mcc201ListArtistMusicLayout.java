package com.hcn.media.music.mcc201;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Handler;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.ListView;
import android.widget.TextView;

import androidx.annotation.NonNull;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media.adapter.mcc201.Mcc201FolderListAdapter;
import com.hcn.media.adapter.mcc201.Mcc201MusicListAdapter;
import com.hcn.media.R3;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_base.impl.MediaEventPostbox;
import com.hcn.media.base.layout.FrameLayoutEx;
import com.hcn.media_data.folder.MusicFilesInfo;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media.vm.action.IPlayerEx;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_data.storage.StorageDeviceEx;
import com.hcn.media.music.ILayoutCallback;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map.Entry;

import io.vov.vitamio.utils.Log;

/**
 * mcc201 音乐艺术家列表布局视图
 * <pre>
 *    为特定客户需求创建，后续不再维护；
 *    这类布局不可以在布局资源文件（[@layout].xml）中直接使用；
 * </pre>
 *
 * @author 65821
 */
@SuppressLint("ViewConstructor")
public class Mcc201ListArtistMusicLayout extends FrameLayoutEx
        implements OnItemClickListener, OnItemLongClickListener,
        OnScrollListener, OnClickListener, OnTouchListener {

    private static final String TAG = Mcc201ListArtistMusicLayout.class.getSimpleName();

    private ListView mFolderGridView = null;
    private ListView mMusicGridView = null;
    private Mcc201FolderListAdapter mFolderListAdapter = null;
    private Mcc201MusicListAdapter mMusicListAdapter = null;
    private TextView mShowTipCtrl = null;
    private StorageDeviceEx mStorageDeviceEx = null;
    private Handler mUserHandler = null;
    private final List<MusicFilesInfo> mListMusicFile = new ArrayList<>();
    private ILayoutCallback mLayoutCallback = null;
    private List<MusicInfo> mListMusicInfo = new ArrayList<>();

    private Runnable mTimeRunnable = () -> {
        mUserHandler.removeCallbacksAndMessages(null);
        mMusicGridView.setSelection(mAppData.musicPlayPosition());
    };

    public Mcc201ListArtistMusicLayout(Context context,
                                       @NonNull IPlayerEx player) {
        this(context, null, player);
    }

    public Mcc201ListArtistMusicLayout(Context context,
                                       AttributeSet attrs,
                                       @NonNull IPlayerEx player) {
        this(context, attrs, 0, player);
    }

    public Mcc201ListArtistMusicLayout(Context context,
                                       AttributeSet attrs,
                                       int defStyle,
                                       @NonNull IPlayerEx player) {
        super(context, attrs, defStyle, player);

        initView();
    }

    private void initView() {
        LayoutInflater.from(mContext).inflate(R.layout.mcc201_layout_folder, this, true);

        mUserHandler = new Handler(mContext.getMainLooper());

        mShowTipCtrl = findViewById(R.id.tvShowTip);
        mMusicGridView = findViewById(R.id.gridview_musics);
        mFolderGridView = findViewById(R.id.gridview_folder);
        mFolderListAdapter = new Mcc201FolderListAdapter(mContext, mFolderGridView);
        mMusicListAdapter = new Mcc201MusicListAdapter(mContext, mMusicGridView);
        mFolderGridView.setAdapter(mFolderListAdapter);
        mFolderGridView.setOnItemClickListener(this);
        mFolderGridView.setOnItemLongClickListener(this);
        mFolderGridView.setOnScrollListener(this);
        mMusicGridView.setAdapter(mMusicListAdapter);
        mMusicGridView.setOnItemClickListener(this);
        mMusicGridView.setOnItemLongClickListener(this);
        mMusicGridView.setOnScrollListener(this);
    }

    @Override
    public void initLayout() {
        mUserHandler.removeCallbacksAndMessages(null);
        initData();
    }

    private void initData() {
        HashMap<String, List<MusicInfo>> mapPath = new HashMap<>();
        for (int i = 0; i < mAppData.mStorageDeviceList.size(); i++) {
            mapPath.putAll(mAppData.mStorageDeviceList.get(i).mArtistListMap);
        }

        setData(mapPath);
        updatePlayIndex();
        updateTipCtrl();
    }

    private void setData(HashMap<String, List<MusicInfo>> pathMap) {
        mListMusicFile.clear();
        for (Entry<String, List<MusicInfo>> entry : pathMap.entrySet()) {
            MusicFilesInfo mFileInfo = new MusicFilesInfo();
            mFileInfo.mPathName = entry.getKey();
            mFileInfo.mListMusicInfo = entry.getValue();
            mListMusicFile.add(mFileInfo);
        }
        mFolderListAdapter.updateInfoList(mListMusicFile);
    }

    private void updateTipCtrl() {
        mShowTipCtrl.setText(xString(R3.string.tip_no_music_file));
        boolean bShowTip = mAppData.musicPlaylist().isEmpty();
        mShowTipCtrl.setVisibility(bShowTip ? View.VISIBLE : View.GONE);
    }

    @Override
    public void doCallbackEvent(int eventId) {
        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE:
                mFolderListAdapter.notifyDataSetChanged();
                break;
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
                updatePlayIndex();
                break;
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
            case IMediaEvent.EVENT_MEDIA_UNMOUNTED:
            case IMediaEvent.EVENT_MEDIA_MOUNTED:
            case IMediaEvent.EVENT_ID3_SCAN_FINISHED:
            case IMediaEvent.EVENT_CHANGE_MUSIC_STORAGE:
                initData();
                break;
            case IMediaEvent.EVENT_MEDIA_LOADING_START:
            default:
                break;
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        mUserHandler.removeCallbacksAndMessages(null);
        mUserHandler.postDelayed(mTimeRunnable, 10000);
        return false;
    }

    private void updatePlayIndex() {
        if (null != mAppData.mCurrentMediaInfo) {
            String parentRoute = new File(
                    mAppData.mCurrentMediaInfo.mFilePath).getParentFile().getAbsolutePath();
            mFolderListAdapter.updatePlayIndex(parentRoute);
        }
    }

    public int getListOfPosition() {
        int size = (mAppData.mSelectedDevice.mMusicInfoList.size());
        for (int i = 0; i < size; i++) {
            if (mAppData.mCurrentMediaInfo != null && mAppData.mCurrentMediaInfo.mIndex
                    == mAppData.mSelectedDevice.mMusicInfoList.get(
                    i).mIndex) {
                return i;
            }
        }
        return 0;
    }

    @Override
    public void setMediaEventListener(MediaEventPostbox listener) {
        super.setMediaEventListener(listener);

        if (null != mFolderListAdapter) {
            mFolderListAdapter.setMediaEventListener(mMediaEventPostbox);
        }
    }

    @Override
    public void initDataObject() {
        initData();
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        Log.i(TAG, "isFolder:" + (view.getId() == R.id.gridview_folder));
        if (parent.getId() == R.id.gridview_folder) {
            mFolderListAdapter.updateSelectIndex(position);
            mFolderGridView.setSelection(position);
            mListMusicInfo = mListMusicFile.get(position).mListMusicInfo;
            mMusicListAdapter.updateInfoList(mListMusicInfo);
        } else if (parent.getId() == R.id.gridview_musics) {
            playerRelay().accept(t -> t.requestPlayTarget(
                    IPlaylistType.DEVICE_LIST, mListMusicInfo, position));
        }
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
        return true;
    }

    @Override
    public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,
            int totalItemCount) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onScrollStateChanged(AbsListView view, int scrollState) {
        if (scrollState == OnScrollListener.SCROLL_STATE_IDLE) {
            mFolderListAdapter.notifyDataSetChanged();
        }
    }

    private void onUpdateListEvent() {
        if (mStorageDeviceEx != null) {
            playerRelay().accept(
                    t -> t.requestExecuteAction(
                            IMediaAction.scanStorageDeviceInfo,
                            mStorageDeviceEx.mFilePath,
                            null));
        }
    }

    private void onChangeAllListView() {
        if (null != mLayoutCallback) {
            mLayoutCallback.onCallback(mStorageDeviceEx, IMediaEvent.EVENT_GOTO_ALL_LIST_PAGE);
        }
    }

    private void onSearchEvent() {
        if (mMediaEventPostbox != null) {
            mMediaEventPostbox.onMediaEvent(
                    IMediaEvent.EVENT_GOTO_MUSIC_SEARCH_PAGE,
                    null,
                    null);

            mAppData.mSearchClickDevice = mStorageDeviceEx;
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.btn_refresh:
                onUpdateListEvent();
                break;
            case R.id.btn_search:
                onSearchEvent();
                break;
            default:
                break;
        }
    }
}
