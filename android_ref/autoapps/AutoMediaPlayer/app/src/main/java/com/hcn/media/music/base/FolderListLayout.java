package com.hcn.media.music.base;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.SystemClock;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.common.misc.LogUtils;
import com.hcn.common.utils.HFileUtils;
import com.hcn.media.R3;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media.adapter.MusicFolderListAdapter;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IConstant;
import com.hcn.media.base.layout.FrameLayoutEx;
import com.hcn.media_common.thread.HTaskRunnable;
import com.hcn.media_data.ListSceneManager;
import com.hcn.media_data.folder.AbcFolderUtils;
import com.hcn.media_data.folder.FilePathScanManager;
import com.hcn.media.music.ITouchEventListener;
import com.hcn.media.vm.action.IPlayerEx;
import com.hcn.media_data.ui.base.PageDataKV;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media.folder.MediaFilePathScan;
import com.hcn.media.folder.MediaFilePathScan.IMediaFilePathScanCallBack;

import java.io.File;
import java.util.ArrayList;
import java.util.Objects;

/**
 * 承储设备文件列表布局
 * <p> 子项元素基本都是使用的帧布局，扩展性强；
 *
 * @author 86158
 */
@SuppressLint("ViewConstructor")
public class FolderListLayout extends FrameLayoutEx
        implements OnItemClickListener, View.OnClickListener,
        OnScrollListener, IMediaFilePathScanCallBack {

    private static final String TAG = FolderListLayout.class.getSimpleName();

    private final int mMode;
    private String mRootFilePath = null;
    private String mCurrentFilePath = null;

    private TextView mBackPromptText;
    private MusicFolderListAdapter mListViewAdapter = null;
    private AbsListView mListView = null;
    private TextView mShowTipCtrl = null;

    /**
     * 反馈触摸事件触发状态用
     * <p> 给外部 Fragment 或者其它父容器反馈当前列表活跃状态；
     */
    private ITouchEventListener mTouchEventListener = null;

    public FolderListLayout(Context context,
                            @NonNull IPlayerEx player) {
        this(context, null, 0, player,
                MediaFilePathScan.DEFAULT_ROOT, MediaFilePathScan.ALL_STORAGE_MODE);
    }

    public FolderListLayout(Context context,
                            AttributeSet attrs,
                            @NonNull IPlayerEx player) {
        this(context, attrs, 0, player,
                MediaFilePathScan.DEFAULT_ROOT, MediaFilePathScan.ALL_STORAGE_MODE);
    }

    public FolderListLayout(Context context,
                            AttributeSet attrs,
                            int defStyle,
                            @NonNull IPlayerEx player) {
        this(context, attrs, defStyle, player,
                MediaFilePathScan.DEFAULT_ROOT, MediaFilePathScan.ALL_STORAGE_MODE);
    }

    public FolderListLayout(Context context,
                            AttributeSet attrs,
                            int defStyle,
                            @NonNull IPlayerEx player,
                            String rootPath,
                            int mode) {
        super(context, attrs, defStyle, player);

        mMode = mode;

        // [记忆上一次的列表路径]
        String currentPath = rootPath;
        final String tempFilePath = mAppData.mFilePathScanManager.mFilePath;
        if (!TextUtils.isEmpty(tempFilePath)
                && HFileUtils.isFileExists(tempFilePath)) {
            currentPath = tempFilePath;
        }

        setFilePath(rootPath, currentPath);

        // [注意：不可以提取放到父类中使用/否则 inflate 的视图会被系统回收]
        initContentView(getLayoutRes(), this);
    }

    @Override
    protected int getLayoutRes() { return R.layout.layout_folderlist; }

    @Nullable
    @Override
    protected View initContentView(int layoutRes, ViewGroup root) {
        LogUtil.d(TAG, "initContentView.");
        View view = super.initContentView(layoutRes, root);

        // 显示提示
        assert view != null;
        mShowTipCtrl = view.findViewById(xId(R.id.tvShowTip));

        View folderBack = view.findViewById(xId(R.id.item_folder_back));
        if (null != folderBack) {
            folderBack.setOnClickListener(this);
        }

        View btnRefresh = view.findViewById(xId(R.id.btn_refresh));
        if (null != btnRefresh) {
            btnRefresh.setOnClickListener(this);
        }

        View btnSearch = view.findViewById(xId(R.id.btn_search));
        if (null != btnSearch) {
            btnSearch.setOnClickListener(this);
        }

        // 返回键的文本提示信息（返回上一级/...）
        mBackPromptText = (TextView) findViewByName("folder_back_prompt");
        mListView = view.findViewById(xId(R.id.gridview_music));
        if (mListView != null) {
            mListViewAdapter = new MusicFolderListAdapter(mContext, mListView);
            mListView.setAdapter(mListViewAdapter);
            mListView.setOnItemClickListener(this);
            mListView.setOnScrollListener(this);
        }

        return view;
    }

    @Override
    public void initLayout() {
        setPlayingFilePath();
        refresh();
    }

    @Override
    public void initDataObject() {
        // TODO: 预留扩展
    }

    public void setFilePath(String rootFilePath, String currentFilePath) {
        mRootFilePath = rootFilePath;
        mCurrentFilePath = currentFilePath;
    }

    public String getRootFilePath() {
        return mRootFilePath;
    }

    public String getCurrentFilePath() {
        return mCurrentFilePath;
    }

    public void setCurrentFilePath(String mCurrentFilePath) {
        this.mCurrentFilePath = mCurrentFilePath;
    }

    @Override
    public void onPathScanFinishedEx(FilePathScanManager fileManager, String path) {
        if (Objects.isNull(mAppData.mFilePathScanManager)) {
            return;
        }

        if (fileManager != null) {
            // 同步更新 ID3 信息
            long nowMillis = SystemClock.elapsedRealtime();
            AbcFolderUtils.updateFolderListId3Info(
                    fileManager.getObjectTag(),
                    fileManager.mMusicOnlyList,
                    true,
                    new HTaskRunnable.OnCompletionListener() {
                        @Override
                        public void onCompletion(Object result) {
                            // TODO: reserved
                        }

                        @Override
                        public void onCompletion(long taskTag, Object result) {
                            // 有效数据才需要处理更新
                            if (!(result instanceof ArrayList)) {
                                return;
                            }

                            @SuppressWarnings("unchecked")
                            ArrayList<MusicInfo> list = (ArrayList<MusicInfo>) result;

                            // 如果列表信息个数都不一样，肯定不是一个任务了。
                            FilePathScanManager manager = mAppData.mFilePathScanManager;
                            if (taskTag == manager.getObjectTag()
                                    && list.size() == manager.mMusicOnlyList.size()) {
                                manager.mMusicOnlyList.clear();
                                manager.mMusicOnlyList.addAll(list);
                            }

                            long deltaTime = SystemClock.elapsedRealtime() - nowMillis;
                            LogUtils.vTag(TAG, "updateFolderListId3Info:" +
                                    " size = " + list.size() + ", execution time = " +  deltaTime);
                        }
                    });

            // 更新列表信息
            updateDataList(fileManager);
        }
    }

    /**
     * 更新文件夹数据列表
     * <p> 文件夹扫描完成、数据更新完成的时候调用；
     *
     * @param fileManager {@link FilePathScanManager}
     */
    private void updateDataList(FilePathScanManager fileManager) {
        // 更新给 UI 显示对象
        FilePathScanManager manager = mAppData.mFilePathScanManager;

        manager.mIsLoading = false;
        manager.mObjectTag = fileManager.mObjectTag;
        manager.mMediaPathState.mIsLoadFinished = true;

        manager.mMusicInfoList.clear();
        manager.mMusicOnlyList.clear();
        manager.mMediaFolderList.clear();

        manager.mMusicInfoList.addAll(fileManager.mMediaFolderList);
        manager.mMusicInfoList.addAll(fileManager.mMusicOnlyList);

        manager.mMusicOnlyList.addAll(fileManager.mMusicOnlyList);
        manager.mMediaFolderList.addAll(fileManager.mMediaFolderList);

        updateFolderStatus();
        mListViewAdapter.setDataList(manager.mMusicInfoList);
        notifyDataSetChanged();
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        if (mTouchEventListener != null) {
            mTouchEventListener.onTouchTrigger();
        }

        return super.dispatchTouchEvent(ev);
    }

    /**
     * 设置触摸事件监听
     * <p> 如果父容器想知道当前列表布局是否在触摸活跃状态，可使用此接口监听触摸动作。
     *
     * @param listener 监听者
     */
    public void setTouchEventListener(ITouchEventListener listener) {
        mTouchEventListener = listener;
    }

    /**
     * 页面是否是空闲的
     * <p> 如果当前列表页面没有任何加载刷新任务，说明其在空闲状态;
     * <p> 可扩展
     *
     * @return <code>true</code> 页面空闲；反之，页面忙；
     */
    public boolean isIdleState() {
        return !isShowLoading();
    }

    /**
     * 是显示 Loading 提示中
     */
    public boolean isShowLoading() {
        if (mShowTipCtrl == null) {
            return false;
        }

        Object tag = mShowTipCtrl.getTag(0xDB00F001);
        if (tag instanceof String) {
            String strTag = (String) tag;
            return strTag.equals("loading")
                    && mShowTipCtrl.getVisibility() == View.VISIBLE;
        }

        return false;
    }

    private void onBtnRefresh() {
        refresh();
    }

    /**
     * 扫描当前路径并更新列表
     * <p> 这里回头可以优化下，改成多线程扫描，可以提高扫描效率；
     */
    public void refresh() {
        LogUtil.v(TAG, "refresh.");

        FilePathScanManager fileManager = mAppData.mFilePathScanManager;
        if (new File(mCurrentFilePath).exists()) {
            fileManager.mFilePath = mCurrentFilePath;
        } else {
            fileManager.mFilePath = mRootFilePath;
        }

        fileManager.mIsLoading = true;
        fileManager.mMediaPathState.mLoadingIndex.incrementAndGet();
        fileManager.mMediaPathState.mIsLoadFinished = false;

        // 執行掃描動作
        MediaFilePathScan.getInstance(mMode)
                .loadMediaPathList(fileManager.mFilePath,
                        fileManager.mMediaPathState,
                        fileManager.mMediaPathState.mLoadingIndex.get(),
                        fileManager.SCAN_MUSIC_FILE_TYPE,
                        this);

        // 更新 UI 顯示狀態
        updateFolderStatus();
    }

    private void onBtnSearch() {
        search();
    }

    public void search() {
        if (mAppData.mFilePathScanManager != null) {
            mAppData.mSearchList = mAppData.mFilePathScanManager.mMusicOnlyList;
            ListSceneManager.getInstance().updateListScenePath(mAppData.mFilePathScanManager.mFilePath);
        }

        // 投递事件给父级组件
        if (mMediaEventPostbox != null) {
            mMediaEventPostbox.onMediaEvent(
                    IMediaEvent.EVENT_GOTO_MUSIC_SEARCH_PAGE, null, null);
        }
    }

    public void notifyDataSetChanged() {
        if (null != mListViewAdapter) {
            mListViewAdapter.notifyDataSetChanged();
        }
    }

    private void updateFolderStatus() {
        // 显示层级提示信息
        if (!Objects.isNull(mBackPromptText)) {
            @StringRes int backPromptId = R3.string.ellipsis_symbol;
            if (!mRootFilePath.equals(mCurrentFilePath)) {
                backPromptId = R3.string.go_back_folder;
            }

            mBackPromptText.setText(xString(backPromptId));
        }

        // 指定文件夹在加载状态
        if (mAppData.mFilePathScanManager.isLoading()) {
            mShowTipCtrl.setText(getResources().getString(R3.string.tip_loading));
            mShowTipCtrl.setVisibility(View.VISIBLE);
            mShowTipCtrl.setTag(0xDB00F001, "loading");
        } else {
            mShowTipCtrl.setText(getResources().getString(R3.string.tip_no_music_file));
            boolean bShowTip = mAppData.mFilePathScanManager.mMusicInfoList.isEmpty();
            mShowTipCtrl.setVisibility(bShowTip ? View.VISIBLE : View.GONE);

            // 如果是 loading 中，说明正好 Loading 结束。
            if (isShowLoading()) {
                // 模拟一个触摸事件，让外部监听重新计时超时时间。
                if (mTouchEventListener != null) {
                    mTouchEventListener.onTouchTrigger();
                }

                // 取消 Loading 标记
                mShowTipCtrl.setTag(0xDB00F001, "other");
            }
        }
    }

    @Override
    public void doCallbackEvent(int nEventID) {
        switch (nEventID) {
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE:
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
                setPlayingFilePath();
                notifyDataSetChanged();
                break;
            case IMediaEvent.EVENT_CHANGE_MUSIC_STORAGE:
            case IMediaEvent.EVENT_MEDIA_UNMOUNTED:
                if (mMode == MediaFilePathScan.SINGLE_STORAGE_MODE) {
                    try {
                        setFilePath(
                                mAppData.mSelectedDevice.mFilePath,
                                mAppData.mSelectedDevice.mFilePath);
                    } catch (Exception ignored) {
                    }
                }
                refresh();
                break;
            case IMediaEvent.EVENT_MEDIA_LOADING_START:
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
                refresh();
                break;

            default:
                break;
        }
    }

    private void setPlayingFilePath() {
        try {
            mListViewAdapter.setPlayingFilePath(
                    mAppData.mCurrentMediaInfo.mFilePath);
        } catch (Exception ignored) {
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        if (Objects.isNull(mAppData.mFilePathScanManager)) {
            return;
        }

        FilePathScanManager fileManager = mAppData.mFilePathScanManager;
        if (position >= fileManager.mMusicInfoList.size()) {
            return;
        }

        MusicInfo info = fileManager.mMusicInfoList.get(position);
        if (info != null) {
            // 媒体对象索引 -1 表示文件夹；
            if (info.mIndex == -1) {
                // 直接触发扫描文件夹
                mCurrentFilePath = info.mFilePath;
                refresh();
            } else if (info.mIndex >= 0) {
                int newPos = position - fileManager.mMediaFolderList.size();

                // 当前存储设备（选择设备）信息
                mAppData.mCurrentDevice =
                        mAppData.mSelectedDevice =
                                mAppData.getStorageDeviceFromPath(info.mFilePath);

                // 请求播放指定列表中的媒体对象
                playerRelay().accept(t -> t.requestPlayTarget(
                        IPlaylistType.FOLDER_LIST, fileManager.mMusicOnlyList, newPos));
                ListSceneManager.getInstance().saveUserListScene(PageDataKV.ActionSceneValue.FOLDER, fileManager.mFilePath);
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
            mListViewAdapter.notifyDataSetChanged();
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View view) {
        switch (getId(view)) {
            case R.id.item_folder_back:
                onFolderBack();
                break;
            case R.id.btn_refresh:
                onBtnRefresh();
                break;
            case R.id.btn_search:
                onBtnSearch();
                break;
            default:
                break;
        }
    }

    public void onFolderBack() {
        if (IConstant.PATH_FLASH.equals(mCurrentFilePath)) {
            mCurrentFilePath = mRootFilePath;
            refresh();
        } else if (!mRootFilePath.equals(mCurrentFilePath)) {
            int index = mCurrentFilePath.lastIndexOf('/');
            if (index > 0) {
                mCurrentFilePath = mCurrentFilePath.substring(0, index);
                refresh();
            }
        } else {
            // 如果是根目录显示省略号
            if (!Objects.isNull(mBackPromptText)) {
                mBackPromptText.setText(xString(R3.string.ellipsis_symbol));
            }
        }
    }

    @Override
    public void onUpdateUiModeView(boolean isNightMode) {
        super.onUpdateUiModeView(isNightMode);

        // 通知适配器刷新，重新设置资源
        mListViewAdapter.notifyDataSetChanged();
    }
}
