package com.hcn.media.video.common;

import static com.hcn.config.Feature.BIT.REMOTE_CONTROL_FOCUS;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Message;
import android.os.SystemClock;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.LinearInterpolator;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.auto_compat.PlatformUtils;
import com.hcn.common.misc.LogUtils;
import com.hcn.config.Feature;
import com.hcn.media.R3;
import com.hcn.media.adapter.VideoFolderListAdapter;
import com.hcn.media.extend.base.IExtend;
import com.hcn.media.folder.MediaFilePathScan;
import com.hcn.media_base.constant.IConstant;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.thread.HTaskRunnable;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_common.utils.ViewUtilsEx;
import com.hcn.media.adapter.VideoListAdapter;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.fragment.IVideoPage;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media_data.ListSceneManager;
import com.hcn.media_data.folder.AbcFolderUtils;
import com.hcn.media_data.folder.FilePathScanManager;
import com.hcn.media_data.ui.base.PageDataKV;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_theme.ThemeEx;
import com.hcn.plugin.ApkClassLoaderEx;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * 视频列表页面
 * @author 65821
 */
@SuppressLint("ValidFragment")
public class VideoListFragment extends MediaFragment
        implements OnClickListener, OnItemClickListener, OnScrollListener,
        MediaFilePathScan.IMediaFilePathScanCallBack {

    private static final String FRAGMENT_NAME = "video-list";
    private static final String TAG = VideoListFragment.class.getSimpleName();

    private final boolean mHasFolderFeature;

    private boolean mInitView = false;
    private View mVideoListLayout = null;
    private Animation mRotateAnim = null;

    private RadioButton mBtnStorageFlash = null;
    private RadioButton mBtnListFolder = null;
    private RadioButton mBtnStorageSD = null;
    private RadioButton mBtnStorageUSB = null;
    private View mBtnPlaying = null;
    private View mSearchLayout = null;

    private View mBtnUpdateList = null;
    private AbsListView mVideoGridView = null;
    private AbsListView mVideoFolderView = null;
    private VideoListAdapter mVideoListAdapter = null;
    private TextView mTipView = null;
    private ImageView mLoadingView = null;

    private String mCurrentFilePath = MediaFilePathScan.DEFAULT_ROOT;
    private String mRootPath = MediaFilePathScan.DEFAULT_ROOT;
    private int mMode = MediaFilePathScan.ALL_STORAGE_MODE;

    /**
     * 是否支持存储按键 Disable 显示状态
     * <pre>
     *    配置文件：src/main/res-compat/config/values[-mcc?-mnc?]/config.xml
     *            <bool name="support_storage_button_disable">true</bool>
     *    如果支持，简单的说就是当存储设备不存在的时候把对应的按钮显示成灰色（Disable，不可点击）；
     * </pre>
     * 
     * @see View#setEnabled(boolean); 
     */
    private final boolean mSupportDisableStorageButton;
    private VideoFolderListAdapter mVideoFolderListAdapter = null;
    private TextView mBackPromptText = null;
    private View mFolderBack = null;

    /** 视频列表页面构造函数 **/
    public VideoListFragment() {
        super(FRAGMENT_NAME);

        // 读取 VideoList 列表选项风格类型
        mSupportDisableStorageButton = xBoolean(
                "support_storage_button_disable", true);

        // 支持检查扩展皮肤包（逻辑扩展）
        String pageExtendResConfigName = "video_list_page_extend";
        if (xBoolean(pageExtendResConfigName)) {
            ApkClassLoaderEx classLoader = xClassLoader();
            if (!Objects.isNull(classLoader)) {
                LogUtils.iTag(TAG, "classLoader"+Objects.isNull(classLoader));
                String pageExtendClassName =
                        IExtend.VIDEO_PACKAGE_NAME + ".VideoListPageExtend";
                LogUtils.iTag(TAG, "pageExtendClassName"+pageExtendClassName);
                mPageExtend = classLoader.newPageExtendInterface(pageExtendClassName, this);
            }

            LogUtils.iTag(TAG, mPageExtend != null?
                    "Has VideoListPageExtend class.": "No VideoListPageExtend class.");
        }

        // 是否支持视频文件夹列表功能
        mHasFolderFeature = xBoolean("video_list_folder_feature");
    }

    /**
     * 存储按钮是否支持 disable 状态；
     * <p> disable 状态下不可以点击，且需要显示成灰色状态；
     *
     * @return 支持/不支持
     */
    private boolean isSupportDisableStorageButton() {
        return mSupportDisableStorageButton;
    }

    @Override
    public void onAttach(@NonNull Context context) {
        super.onAttach(context);
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        LogUtil.v(TAG, "onCreate.");
    }

    @Override
    public int getLayoutRes() {
        if (ThemeEx.useVideoListExpandLayout()) {
            return requestUiModel().videoShowInBottomHalfScreen()?
                    R.layout.fragment_videolist_expand: R.layout.fragment_videolist;
        }

        return R.layout.fragment_videolist;
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
            @Nullable Bundle savedInstanceState) {
        LogUtil.v(TAG, "onCreateView.");

        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);

        assert view != null;
        initView(view);
        return view;
    }

    private void initView(View layout) {
        if (mInitView) {
            LogUtil.d(TAG, "It's already initialized!");
            return;
        }

        mInitView = true;
        mVideoListLayout = layout.findViewById(xId(R.id.llVideoList));

        // 刷新数据按钮
        mBtnUpdateList = layout.findViewById(xId(R.id.btnUpdate));
        if (mBtnUpdateList != null) {
            mBtnUpdateList.setOnClickListener(this);

            // [横屏分配状态/是多窗口状态，且是竖屏窗口状态]
            if (requireActivity().isInMultiWindowMode()
                    && MiscUtils.isPortraitWindow(requireContext())) {
                mBtnUpdateList.setVisibility(View.GONE);
            } else {
                mBtnUpdateList.setVisibility(View.VISIBLE);
            }
        }

        // 提示信息元素
        mTipView = layout.findViewById(xId(R.id.tv_main_tips));
        mLoadingView = layout.findViewById(xId(R.id.iv_loading));

        // 内置存储按钮
        mBtnStorageFlash = layout.findViewById(xId(R.id.btnStorageFlash));
        if (mBtnStorageFlash != null) {
            mBtnStorageFlash.setOnClickListener(this);
        }

        // 文件存储按钮
        if (mHasFolderFeature) {
            mBtnListFolder = layout.findViewById(xId(R.id.btnListFolder));
            if (mBtnListFolder != null) {
                mBtnListFolder.setOnClickListener(this);
            }
        }

        // SD 卡存储按钮
        mBtnStorageSD = layout.findViewById(xId(R.id.btnStorageSD1));
        if (mBtnStorageSD != null) {
            mBtnStorageSD.setOnClickListener(this);
        }
		
        // 8163 广告机不需要 SD 卡列表
        Feature mFeature = Feature.instance();
        if (mFeature.hasFeature(REMOTE_CONTROL_FOCUS)){
            mBtnStorageSD.setVisibility(View.GONE);
        }

        // USB 存储按钮
        mBtnStorageUSB = layout.findViewById(xId(R.id.btnStorageUSB1));
        if (mBtnStorageUSB != null) {
            mBtnStorageUSB.setOnClickListener(this);
        }

        // 返回播放页面按钮
        mBtnPlaying = layout.findViewById(xId(R.id.btnPlaying));
        if (mBtnPlaying != null) {
            mBtnPlaying.setOnClickListener(this);
        }

        // 查找歌曲按钮
        mSearchLayout = layout.findViewById(xId(R.id.layout_search));
        if (mSearchLayout != null) {
            mSearchLayout.setOnClickListener(this);
        }

        // 视频文件夹返回视图
        if (mHasFolderFeature) {
            mFolderBack = layout.findViewById(xId(R.id.item_folder_back));
            mBackPromptText = (TextView) findViewByName("folder_back_prompt");
            mVideoFolderView = layout.findViewById(xId(R.id.folder_list));
            if (mFolderBack != null) {
                mFolderBack.setVisibility(View.GONE);
                mFolderBack.setOnClickListener(this);
            }

            if (mVideoFolderView != null) {
                mVideoFolderView.setOnItemClickListener(this);
            }
        }

        // 数据列表视图
        mVideoGridView = layout.findViewById(xId(R.id.main_grid));
        if (mVideoGridView != null) {
            mVideoListAdapter = new VideoListAdapter(mContext, mVideoGridView);
            mVideoGridView.setAdapter(mVideoListAdapter);
            mVideoGridView.setOnItemClickListener(this);
            mVideoGridView.setOnScrollListener(this);
        }

        adjustLayoutByStatusBar();
        initAnimation();
    }

    /**
     * 依据状态栏调整局部参数
     * <pre>
     *    不同的主题需要调整的位置不一样；
     *    这里主要调整需要配置 android:paddingTop="@*android:dimen/status_bar_height" 的元素；
     *    原因，AndroidStudio 编译的 apk 运行时系统不认识这个常量；
     * </pre>
     */
    private void adjustLayoutByStatusBar() {
        if (Objects.isNull(mVideoListLayout)) {
            return;
        }

        // 如果视频显示在分屏的下半部分
        if (requestUiModel().videoShowInBottomHalfScreen()) {
            // 如果没有使用扩展布局，不需要预留状态栏高度
            if (!ThemeEx.useVideoListExpandLayout()) {
                mVideoListLayout.setPadding(0, 0, 0, 0);
            }

            return;
        }

        // 使用了显示过扫描配置，不需要预留状态栏高度
        if (PlatformUtils.isDisplayOverscanning()) {
            return;
        }

        // 调整状态栏高度，否则列表页面显示会和状态栏重叠（视频是全屏窗口模式）；
        int statusBarHeight = MiscUtils.statusBarHeight(mContext, R.dimen.status_bar_height);
        mVideoListLayout.setPadding(0, statusBarHeight, 0, 0);
    }

    private void initAnimation() {
        // 兼容扩展主题包的加载动画方式
        mRotateAnim = xAnimation(R.anim.anim_rotate);
        if (mRotateAnim != null) {
            mRotateAnim.setInterpolator(new LinearInterpolator());
        }
    }

    @Override
    public void initFragment() {
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_MEDIA_LOADING_START:
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE: {
                mediaLoadingCompleteUpdateData();
                break;
            }
            case IMediaEvent.EVENT_MEDIA_MOUNTED: {
                updateStorageButton();
                H0.sendEmptyUniqueMessageDelayed(
                        MsgEx.MSG_UPDATE_STORAGE_DEVICE_STATE, 1500);
                break;
            }
            case IMediaEvent.EVENT_MEDIA_UNMOUNTED: {
                H0.sendEmptyUniqueMessageDelayed(
                        MsgEx.MSG_UPDATE_STORAGE_DEVICE_LIST_INFO, 1000);
                break;
            }
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE:
            case IMediaEvent.EVENT_CHANGE_VIDEO_ITEM:
            case IMediaEvent.EVENT_CHANGE_VIDEO_LIST: {
                updateStorageButton();
                setPlayingFilePath();
                updateList();
                break;
            }
            default:
                break;
        }
    }

    private void mediaLoadingCompleteUpdateData() {
        if (mBtnListFolder != null && mBtnListFolder.isChecked()) {
            onRefreshScan();
        } else {
            updateTipCtrl();
            updateList();
        }
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if (hidden) {
        } else {
            updateFragment();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        updateFragment();
    }

    /**
     * 插件请求任务执行入口
     * <p> 后续根据实际情况扩展实现，确保满足常用需求；
     *
     * @param method  方法类型
     * @param objects 参数集
     * @return 执行结果（具体约定）
     */
    @Override
    protected Object requestExecuteMethod_Impl(String method, Object... objects) {
        switch (method) {
            case "onItemClick":
                if (objects.length == 1) {
                    if (objects[0] instanceof Integer) {
                        onItemClick((Integer)objects[0]);
                    }
                }
                return null;
            default:
                break;
        }
        return super.requestExecuteMethod_Impl(method, objects);
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // TODO: 预留接口
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        // [横屏分配状态/是多窗口状态，且是竖屏窗口状态]
        if (requireActivity().isInMultiWindowMode()
                && MiscUtils.isPortraitWindow(requireContext())) {
            mBtnUpdateList.setVisibility(View.GONE);
        } else {
            mBtnUpdateList.setVisibility(View.VISIBLE);
        }
    }

    /**
     * 更新存储设备列表信息
     * <p> 当前正在播放的 USB/SD 移除后需要更新列表信息；
     */
    public void updateStorageDeviceList() {
        if (!mAppData.mSelectedDevice.isMounted()) {
            mAppData.mSelectedDevice = mAppData.mCurrentDevice;
        }

        updateFragment();
        updateStorageButton();
    }

    /**
     * 当前页面消息定义
     * <p> 子类的消息定义必须从 {@link  H#MSG_BASE_THRESHOLD} 后开始；
     */
    private interface MsgEx extends H {
        int MSG_IDLE = MSG_BASE_THRESHOLD;

        // 更新存储设备状态
        int MSG_UPDATE_STORAGE_DEVICE_STATE = MSG_IDLE + 1;

        // 更新存储设备列表信息
        int MSG_UPDATE_STORAGE_DEVICE_LIST_INFO = MSG_IDLE + 2;
    }

    @Override
    protected void onHandleMessage(@NonNull Message msg) {
        super.onHandleMessage(msg);

        switch (msg.what) {
            case MsgEx.MSG_UPDATE_STORAGE_DEVICE_STATE:
                updateStorageButton();
                break;
            case MsgEx.MSG_UPDATE_STORAGE_DEVICE_LIST_INFO:
                updateStorageDeviceList();
                break;
            case MsgEx.MSG_NONE:
            default:
                break;
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        if (parent.getAdapter() instanceof VideoListAdapter) {
            if (mAppData.mSelectedDevice != null) {
                List<MusicInfo> infoList = mAppData.mSelectedDevice.mVideoInfoList;
                mAppData.mCurrentDevice = mAppData.mSelectedDevice;
                mVideoViewModel.playerRelay().accept(
                        t -> t.requestPlayTarget(
                                IPlaylistType.DEVICE_LIST, infoList, position));
            }
        } else {
            if (!mHasFolderFeature) {
                return;
            }

            if (Objects.isNull(mAppData.mFilePathScanManager)) {
                return;
            }

            FilePathScanManager fileManager = mAppData.mFilePathScanManager;
            if (position >= fileManager.mVideoInfoList.size()) {
                return;
            }

            MusicInfo info = fileManager.mVideoInfoList.get(position);
            if (info != null) {
                // 媒体对象索引 -1 表示文件夹；
                if (info.mIndex == -1) {
                    // 直接触发扫描文件夹
                    mCurrentFilePath = info.mFilePath;
                    onRefreshScan();
                } else if (info.mIndex >= 0) {
                    int newPos = position - fileManager.mMediaFolderList.size();

                    // 当前存储设备（选择设备）信息
                    mAppData.mCurrentDevice =
                            mAppData.mSelectedDevice =
                                    mAppData.getStorageDeviceFromPath(info.mFilePath);
                    // 请求播放指定列表中的媒体对象
                    mVideoViewModel.playerRelay().accept(t -> t.requestPlayTarget(
                            IPlaylistType.DEVICE_LIST, fileManager.mVideoOnlyList, newPos));
                    ListSceneManager.getInstance().saveUserListScene(PageDataKV.ActionSceneValue.FOLDER, fileManager.mFilePath);
                }
            }
        }
    }

    public void onItemClick(int position) {
        if (mAppData.mSelectedDevice != null) {
            List<MusicInfo> infoList = mAppData.mSelectedDevice.mVideoInfoList;
            mAppData.mCurrentDevice = mAppData.mSelectedDevice;
            mVideoViewModel.playerRelay().accept(
                    t -> t.requestPlayTarget(
                            IPlaylistType.DEVICE_LIST, infoList, position));
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (getId(v)) {
            case R.id.btnStorageFlash:
                mAppData.mSelectedDevice = mAppData.mFlashStorage;
                updateFragment();
                break;
            case R.id.btnStorageSD1:
                mAppData.mSelectedDevice = mAppData.mSdStorage;
                updateFragment();
                break;
            case R.id.btnStorageUSB1:
                mAppData.mSelectedDevice = mAppData.mUsbStorage;
                updateFragment();
                break;
            case R.id.btnListFolder:
                if (mHasFolderFeature) {
                    mBtnListFolder.setChecked(true);
                    onRefreshScan();
                }
                break;
            case R.id.layout_search:
                onSearchEvent();
                break;
            case R.id.btnPlaying:
                onBackEvent();
                break;
            case R.id.btnUpdate:
                onUpdateEvent();
                break;
            case R.id.item_folder_back:
                onFolderBack();
                break;
            default:
                break;
        }
    }

    private void onUpdateEvent() {
        if (mBtnListFolder != null && mBtnListFolder.isChecked()) {
            onRefreshScan();
        } else {
            onUpdateListEvent();
        }
    }

    private void onFolderBack() {
        if (!mHasFolderFeature) {
            return;
        }

        if (IConstant.PATH_FLASH.equals(mCurrentFilePath)) {
            mCurrentFilePath = mRootPath;
            onRefreshScan();
        } else if (!mRootPath.equals(mCurrentFilePath)) {
            int index = mCurrentFilePath.lastIndexOf('/');
            if (index > 0) {
                mCurrentFilePath = mCurrentFilePath.substring(0, index);
                onRefreshScan();
            }
        } else {
            // 如果是根目录显示省略号
            if (!Objects.isNull(mBackPromptText)) {
                mBackPromptText.setText(xString(R3.string.ellipsis_symbol));
            }
        }
    }

    private void updateFolderStatus() {
        if (mHasFolderFeature) {
            if (!ViewUtilsEx.isVisible(mFolderBack)) {
                mFolderBack.setVisibility(View.VISIBLE);
            }
        }

        // 显示层级提示信息
        if (!Objects.isNull(mBackPromptText)) {
            @StringRes int backPromptId = R3.string.ellipsis_symbol;
            if (!mRootPath.equals(mCurrentFilePath)) {
                backPromptId = R3.string.go_back_folder;
            }

            mBackPromptText.setText(xString(backPromptId));
        }

        if (mAppData.mFilePathScanManager.isLoading()) {
            onStartAnimation(true);
            mTipView.setText(mContext.getString(R3.string.tip_loading));
            mVideoFolderView.setVisibility(View.GONE);
            mTipView.setVisibility(View.VISIBLE);
        } else {
            onStartAnimation(false);
            mTipView.setText(mContext.getString(R3.string.tip_no_music_file));
            mVideoFolderView.setVisibility(View.VISIBLE);
            mTipView.setVisibility(View.GONE);
        }
    }
    /**
     * 执行扫描操作
     */
    private void onRefreshScan() {
        if (!mHasFolderFeature) {
            return;
        }

        FilePathScanManager fileManager = mAppData.mFilePathScanManager;
        if (new File(mCurrentFilePath).exists()) {
            fileManager.mFilePath = mCurrentFilePath;
        } else {
            fileManager.mFilePath = mRootPath;
        }
        fileManager.mIsLoading = true;
        fileManager.mMediaPathState.mLoadingIndex.incrementAndGet();
        fileManager.mMediaPathState.mIsLoadFinished = false;
        // 執行掃描動作
        MediaFilePathScan.getInstance(mMode)
                .loadMediaPathList(fileManager.mFilePath,
                        fileManager.mMediaPathState,
                        fileManager.mMediaPathState.mLoadingIndex.get(),
                        fileManager.SCAN_VIDEO_FILE_TYPE,
                        this);
        updateFolderStatus();
    }

    @Override
    public void onPathScanFinishedEx(FilePathScanManager fileManager, String path) {
        if (!mHasFolderFeature) {
            return;
        }

        if (Objects.isNull(mAppData.mFilePathScanManager)) {
            return;
        }
        if (fileManager != null) {
            // 同步更新 ID3 信息
            long nowMillis = SystemClock.elapsedRealtime();
            AbcFolderUtils.updateFolderListId3Info(
                    fileManager.getObjectTag(),
                    fileManager.mVideoOnlyList,
                    false,
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
                                    && list.size() == manager.mVideoOnlyList.size()) {
                                manager.mVideoOnlyList.clear();
                                manager.mVideoOnlyList.addAll(list);
                            }

                            long deltaTime = SystemClock.elapsedRealtime() - nowMillis;
                            LogUtils.vTag(TAG, "updateFolderListId3Info:" +
                                    " size = " + list.size() + ", execution time = " +  deltaTime);
                        }
                    });

            // 更新列表信息
            setPlayingFilePath();
            updateDataList(fileManager);
        }
    }

    private void updateDataList(FilePathScanManager fileManager) {
        FilePathScanManager manager = mAppData.mFilePathScanManager;

        manager.mIsLoading = false;
        manager.mObjectTag = fileManager.mObjectTag;
        manager.mMediaPathState.mIsLoadFinished = true;

        manager.mVideoInfoList.clear();
        manager.mVideoOnlyList.clear();
        manager.mMediaFolderList.clear();

        manager.mVideoInfoList.addAll(fileManager.mMediaFolderList);
        manager.mVideoInfoList.addAll(fileManager.mVideoOnlyList);

        manager.mVideoOnlyList.addAll(fileManager.mVideoOnlyList);
        manager.mMediaFolderList.addAll(fileManager.mMediaFolderList);

        if (!Objects.isNull(mVideoFolderView)
                && !Objects.isNull(mVideoGridView) && !ViewUtilsEx.isVisible(mVideoFolderView)) {
            mVideoFolderView.setVisibility(View.VISIBLE);
            mVideoGridView.setVisibility(View.GONE);
        }

        if (mVideoFolderListAdapter == null) {
            mVideoFolderListAdapter = new VideoFolderListAdapter(mContext,mVideoFolderView);
            mVideoFolderListAdapter.setDataList(manager.mVideoInfoList);
            mVideoFolderView.setAdapter(mVideoFolderListAdapter);
        } else if (mVideoFolderView.getAdapter() == null) {
            mVideoFolderListAdapter.setDataList(manager.mVideoInfoList);
            mVideoFolderView.setAdapter(mVideoFolderListAdapter);
        } else {
            mVideoFolderListAdapter.setDataList(manager.mVideoInfoList);
            mVideoFolderListAdapter.notifyDataSetChanged();
        }
        updateFolderStatus();
    }

    private void onBackEvent() {
        if (Objects.isNull(mContext)) {
            return;
        }

        if (Objects.isNull(mVideoViewModel)) {
            return;
        }

        // 跳转到视频播放页面
        mVideoViewModel.fragment2MainUi().execute(
                t -> t.onEvent(IMediaEvent.EVENT_SHOW_VIDEO_FRAGMENT,
                        IVideoPage.E_GROUP_SHOW_VIDEO_INFO, null));
    }

    private void updateFragment() {
        updateStorageButton();
        updateTipCtrl();
        setPlayingFilePath();
        updateList();
    }

    private void setPlayingFilePath() {
        if (Objects.isNull(mVideoListAdapter)) {
            return;
        }

        try {
            mVideoListAdapter.setPlayingFilePath(mAppData.mCurrentMediaInfo.mFilePath);
            // 具体业务逻辑由皮肤包去实现
            if (mPageExtend != null) {
                String result = mPageExtend.tryCallMethod("setPlayingFilePath");
                LogUtil.v(TAG, "tryCallMethod/setPlayingFilePath: " + result);
            }
        } catch (Exception ignored) {
        }
    }

    public void updateList() {
        if (mHasFolderFeature) {
            if (!ViewUtilsEx.isVisible(mVideoGridView)) {
                mVideoGridView.setVisibility(View.VISIBLE);
                mVideoFolderView.setVisibility(View.GONE);
            }

            if (ViewUtilsEx.isVisible(mFolderBack)) {
                mFolderBack.setVisibility(View.GONE);
            }
        }

        if (null != mVideoListAdapter && null != mAppData.mSelectedDevice) {
            mVideoListAdapter.setDataList(mAppData.mSelectedDevice.mVideoInfoList);
            mVideoListAdapter.notifyDataSetChanged();
        }
        if (mPageExtend != null) {
            String result = mPageExtend.tryCallMethod("updateList");
            LogUtil.v(TAG, "tryCallMethod/updateList: " + result);
        }
    }

    /**
     * 更新提示信息
     * <p> 注意：所有牵扯到 Text 的显示都需要初始化显示设置；
     */
    private void updateTipCtrl() {
        if (mAppData.mSelectedDevice == null) {
            return;
        }

        if (mAppData.mSelectedDevice.isLoading()) {
            onStartAnimation(true);
            mTipView.setText(getString(R3.string.tip_loading));
            mTipView.setVisibility(View.VISIBLE);
        } else {
            onStartAnimation(false);
            mTipView.setText(getString(R3.string.tip_no_video_file));
            boolean bShowTip = mAppData.mSelectedDevice.mVideoInfoList.isEmpty();
            mTipView.setVisibility(bShowTip ? View.VISIBLE : View.GONE);
        }
    }

    /**
     * 开始旋转动画
     * @param bStart 开始/停止
     */
    private void onStartAnimation(boolean bStart) {
        if (Objects.isNull(mLoadingView)) {
            return;
        }

        if (bStart) {
            // 特定皮肤不需要这个 Loading 动画
            if (!ViewUtilsEx.isVisible(mLoadingView, View.GONE)) {
                mLoadingView.startAnimation(mRotateAnim);
                mLoadingView.setVisibility(View.VISIBLE);
            }

            mBtnUpdateList.setEnabled(false);
        } else {
            if (!ViewUtilsEx.isVisible(mLoadingView, View.GONE)) {
                mLoadingView.clearAnimation();
                mLoadingView.setVisibility(View.INVISIBLE);
            }

            mBtnUpdateList.setEnabled(true);
        }
    }

    /**
     * 更新承储按钮昨天
     * <p> 主要是按钮的使能与高亮；
     */
    private void updateStorageButton() {
        // 检查是否有存储设备按钮。
        if (Objects.isNull(mBtnStorageFlash)) {
            return;
        }

        mBtnStorageFlash.setEnabled(true);

        // 是否支持显示 disable 状态（皮肤包自己配置）
        if (isSupportDisableStorageButton()) {
            mBtnStorageSD.setEnabled(isSdcardMounted());
            mBtnStorageUSB.setEnabled(isUsbMounted());
        }

        if (mAppData.mSelectedDevice == mAppData.mSdStorage) {
            mBtnStorageSD.setChecked(true);
        } else if (mAppData.mSelectedDevice == mAppData.mUsbStorage) {
            mBtnStorageUSB.setChecked(true);
        } else {
            mBtnStorageFlash.setChecked(true);
        }

        if (mBtnListFolder != null) {
            mBtnListFolder.setChecked(false);
        }
    }

    private void onSearchEvent() {
        if (Objects.isNull(mContext)) {
            return;
        }

        if (Objects.isNull(mVideoViewModel)) {
            return;
        }

        mVideoViewModel.fragment2MainUi().execute(
                t -> t.onEvent(IMediaEvent.EVENT_SHOW_VIDEO_FRAGMENT,
                        IVideoPage.E_GROUP_SHOW_VIDEO_SEARCH, null));
    }

    private void onUpdateListEvent() {
        if (mAppData.mSelectedDevice != null) {
            mVideoViewModel.playerRelay().accept(
                    t -> t.requestExecuteAction(
                            IMediaAction.scanStorageDeviceInfo,
                            mAppData.mSelectedDevice.mFilePath,
                            null));
        }
    }

    @Override
    public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,
            int totalItemCount) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onScrollStateChanged(AbsListView view, int scrollState) {
        if (Objects.isNull(mVideoListAdapter)) {
            return;
        }

        if (scrollState == OnScrollListener.SCROLL_STATE_IDLE) {
            mVideoListAdapter.setScrollState(false);
            mVideoListAdapter.notifyDataSetChanged();
        } else {
            mVideoListAdapter.setScrollState(true);
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LogUtil.v(TAG, "onDestroyView.");

        mInitView = false;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        LogUtil.v(TAG, "onDestroy.");
    }

    @Override
    public void onUpdateUiModeView(boolean isNightMode) {
        super.onUpdateUiModeView(isNightMode);

        // 刷新资源
        mVideoListAdapter.notifyDataSetChanged();
    }
}
