package com.hcn.media.music.common;

import static com.hcn.config.Feature.BIT.REMOTE_CONTROL_FOCUS;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Bundle;

import android.os.Message;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.RadioButton;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.viewpager.widget.PagerAdapter;
import androidx.viewpager.widget.ViewPager;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.common.misc.LogUtils;
import com.hcn.config.Feature;
import com.hcn.media.extend.base.IExtend;
import com.hcn.media_base.constant.IConstant;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.fragment.IMusicPage;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media.music.base.FolderListLayout;
import com.hcn.media.music.base.MusicListLayout;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_theme.Argument;
import com.hcn.media_theme.ThemeEx;
import com.hcn.plugin.ApkClassLoaderEx;
import com.hcn.skinx.SkinX;

import java.util.Objects;

/**
 * 音乐列表页面
 * @author 65821
 */
@SuppressLint("ValidFragment")
public class MusicListFragment extends MediaFragment
        implements OnClickListener, ViewPager.OnPageChangeListener {
    private static final String FRAGMENT_NAME = "music-list";
    private static final String TAG = MusicListFragment.class.getSimpleName();

    private boolean mInitView = false;
    private ViewPager mViewPager = null;

    private RadioButton mBtnStorageFlash = null;
    private RadioButton mBtnStorageSD = null;
    private RadioButton mBtnStorageUSB = null;
    private RadioButton mBtnListFolder = null;

    private MusicListLayout mFlashMusicListLayout = null;
    private MusicListLayout mUSBMusicListLayout = null;
    private MusicListLayout mSDMusicListLayout = null;
    private FolderListLayout mFolderListLayout = null;

    /** 音乐列表页面构造函数 **/
    @SuppressLint("ValidFragment")
    public MusicListFragment() {
        super(FRAGMENT_NAME);

        // 支持检查扩展皮肤包（逻辑扩展）
        String pageExtendResConfigName = "music_list_page_extend";
        if (xBoolean(pageExtendResConfigName)) {
            ApkClassLoaderEx classLoader = xClassLoader();
            if (!Objects.isNull(classLoader)) {
                String pageExtendClassName =
                        IExtend.MUSIC_PACKAGE_NAME + ".MusicListPageExtend";
                mPageExtend = classLoader.newPageExtendInterface(pageExtendClassName, this);
            }

            LogUtils.iTag(TAG, mPageExtend != null?
                    "Has MusicListPageExtend class.": "No MusicListPageExtend class.");
        }
    }

    @Override
    public void initFragment() {
    }

    @Override
    public void uninitFragment() {
        super.uninitFragment();
    }

    @Override
    public void onAttach(@NonNull Context context) {
        super.onAttach(context);
        Log.d(TAG, "onAttach");

        // 首次进入列表，如果不是文件列表，则显示当前播放的
        tryUpdateMusicListType();
    }

    /**
     * 更新音乐列表类型
     * <p> 根据之前选择的存储设备对象，恢复列表显示页；
     */
    private void tryUpdateMusicListType() {
        // 文件夹列表不用调整当前列表显示状态
        if (mAppData.mMusicListPageType  == IMusicState.PAGE_INDEX_FOLDER) {
            return;
        }

        if (!SkinX.getBoolean("folder_interface_only", false)) {
            if (mAppData.mSelectedDevice == mAppData.mFlashStorage) {
                mAppData.mMusicListPageType = IMusicState.PAGE_INDEX_FLASH;
            } else if (mAppData.mSelectedDevice == mAppData.mUsbStorage) {
                mAppData.mMusicListPageType = IMusicState.PAGE_INDEX_USB;
            } else if (mAppData.mSelectedDevice == mAppData.mSdStorage) {
                mAppData.mMusicListPageType = IMusicState.PAGE_INDEX_SD;
            }
        } else {
            // 前装N91要求只加载文件夹界面
            mAppData.mMusicListPageType = IMusicState.PAGE_INDEX_FOLDER;
        }
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fragment_musiclist;
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container,
                             Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");

        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);

        assert view != null;
        initView(view);
        initFragment();
        mInitView = true;

        return view;
    }

    private void initView(View layout) {
        // Flash
        mBtnStorageFlash = layout.findViewById(xId(R.id.btnStorageFlash));
        if (mBtnStorageFlash != null) {
            mBtnStorageFlash.setOnClickListener(this);
        }

        // USB
        mBtnStorageUSB = layout.findViewById(xId(R.id.btnStorageUSB1));
        if (mBtnStorageUSB != null) {
            mBtnStorageUSB.setOnClickListener(this);
        }

        // SD
        mBtnStorageSD = layout.findViewById(xId(R.id.btnStorageSD));
        if (mBtnStorageSD != null) {
            mBtnStorageSD.setOnClickListener(this);
        }
        // 8163 广告机不需要 SD 卡列表
        Feature mFeature = Feature.instance();
        if (mFeature.hasFeature(REMOTE_CONTROL_FOCUS)){
            mBtnStorageSD.setVisibility(View.GONE);
        }

        // Folder
        mBtnListFolder = layout.findViewById(xId(R.id.btnListFolder));
        if (mBtnListFolder != null) {
            mBtnListFolder.setOnClickListener(this);
        }

        // Update
        View btnUpdateList = layout.findViewById(xId(R.id.btnUpdate));
        if (btnUpdateList != null) {
            btnUpdateList.setOnClickListener(this);

            // [横屏分配状态/是多窗口状态，且是竖屏窗口状态]
            if (requireActivity().isInMultiWindowMode()
                    && MiscUtils.isPortraitWindow(requireContext())) {
                btnUpdateList.setVisibility(View.GONE);
            } else {
                btnUpdateList.setVisibility(View.VISIBLE);
            }
        }

        // Search
        View btnSearchLayout = layout.findViewById(xId(R.id.btnSearch));
        if (btnSearchLayout != null) {
            btnSearchLayout.setOnClickListener(this);
        }

        // Back PlayInfo Page
        View btnPlaying = layout.findViewById(xId(R.id.btnPlaying));
        if (btnPlaying != null) {
            btnPlaying.setOnClickListener(this);
        }

        // List Pager(Flash/USB/SDCard/Folder)
        mViewPager = layout.findViewById(xId(R.id.viewpager_center));
        if (mViewPager != null) {
            ViewPagerAdapter viewPagerAdapter = new ViewPagerAdapter(mContext);
            mViewPager.setAdapter(viewPagerAdapter);
            mViewPager.setOnPageChangeListener(this);
        }
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        Log.d(TAG, "onHiddenChanged, hidden = " + hidden);
        if (!hidden) {
            updateFragment();
        }
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        Log.d(TAG, "onViewCreated");
    }

    @Override
    public void onStart() {
        super.onStart();
        Log.d(TAG, "onStart");
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
        updateFragment();
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // 有效性检查
        if (Objects.isNull(mContext)
                || Objects.isNull(mMusicViewModel)) {
            return;
        }

        // 点击 Search 按钮触发的回调
        if (eventId == IMediaEvent.EVENT_GOTO_MUSIC_SEARCH_PAGE) {
            mMusicViewModel.fragment2MainUi().execute(
                    t -> t.onEvent(IMediaEvent.EVENT_SHOW_MUSIC_FRAGMENT,
                            IMusicPage.E_GROUP_SHOW_MUSIC_SEARCH, null));
        }

        mMusicViewModel.fragment2MainUi().execute(
                t -> t.onEvent(eventId, wParam, lParam));
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_MEDIA_UNMOUNTED: {
                H0.sendEmptyUniqueMessageDelayed(
                        MsgEx.MSG_UPDATE_STORAGE_DEVICE_LIST_INFO, 1000);
                break;
            }
            case IMediaEvent.EVENT_MEDIA_MOUNTED: {
                updateStorageButton();
                H0.sendEmptyUniqueMessageDelayed(
                        MsgEx.MSG_UPDATE_STORAGE_DEVICE_STATE, 1500);
                break;
            }
            case IMediaEvent.EVENT_MEDIA_LOADING_START:
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
            case IMediaEvent.EVENT_UPDATE_MUSIC_LIST:
            default:
                break;
        }

        dispatchCallbackEvent(eventId);
    }

    /** 分发事件到子元素视图 **/
    private void dispatchCallbackEvent(int eventId) {
        if (mFlashMusicListLayout != null) {
            mFlashMusicListLayout.doCallbackEvent(eventId);
        }

        if (mUSBMusicListLayout != null) {
            mUSBMusicListLayout.doCallbackEvent(eventId);
        }

        if (mSDMusicListLayout != null) {
            mSDMusicListLayout.doCallbackEvent(eventId);
        }

        if (mFolderListLayout != null) {
            mFolderListLayout.doCallbackEvent(eventId);
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.d(TAG, "onPause");
    }

    public void updateFragment() {
        Log.d(TAG, "updateFragment: " + mAppData.mMusicListPageType);

        switch (mAppData.mMusicListPageType) {
            case IMusicState.PAGE_INDEX_USB:
                mViewPager.setCurrentItem(Page.USB);
                break;
            case IMusicState.PAGE_INDEX_SD:
                mViewPager.setCurrentItem(Page.SDCARD);
                break;
            case IMusicState.PAGE_INDEX_FOLDER:
                mViewPager.setCurrentItem(Page.FOLDER);
                break;
            case IMusicState.PAGE_INDEX_FLASH:
            default:
                mViewPager.setCurrentItem(Page.FLASH);
                break;
        }

        updateStorageButton();
        onChangeListType(mAppData.mMusicListPageType);
    }

    @Override
    public void onPageScrolled(int arg0, float arg1, int arg2) {
        // TODO Auto-generated method stub
        LogUtil.v(TAG, "onPageScrolled");
    }

    @Override
    public void onPageScrollStateChanged(int arg0) {
        Log.v(TAG, "onPageScrollStateChanged");
    }

    @Override
    public void onPageSelected(int position) {
        // TODO Auto-generated method stub
        Log.v(TAG, "onPageSelected pos-" + position);

        if (position == Page.FLASH) {
            mAppData.mMusicListPageType = IMusicState.PAGE_INDEX_FLASH;
            mAppData.mSelectedDevice = mAppData.mFlashStorage;
        } else if (position == Page.USB) {
            mAppData.mMusicListPageType = IMusicState.PAGE_INDEX_USB;
            mAppData.mSelectedDevice = mAppData.mUsbStorage;
        } else if (position == Page.SDCARD) {
            mAppData.mMusicListPageType = IMusicState.PAGE_INDEX_SD;
            mAppData.mSelectedDevice = mAppData.mSdStorage;
        } else if (position == Page.FOLDER) {
            mAppData.mMusicListPageType = IMusicState.PAGE_INDEX_FOLDER;
        }

        onChangeListType(mAppData.mMusicListPageType);
    }

    private void onChangeListType(int nListType) {
        switch (nListType) {
            case IMusicState.PAGE_INDEX_FLASH:
                mBtnStorageFlash.setChecked(true);
                break;
            case IMusicState.PAGE_INDEX_USB:
                mBtnStorageUSB.setChecked(true);
                break;
            case IMusicState.PAGE_INDEX_SD:
                mBtnStorageSD.setChecked(true);
                break;
            case IMusicState.PAGE_INDEX_FOLDER:
                mBtnListFolder.setChecked(true);
                break;
            default:
                break;
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (getId(v)) {
            case R.id.btnStorageFlash:
                mViewPager.setCurrentItem(Page.FLASH, true);
                break;
            case R.id.btnStorageUSB1:
                mViewPager.setCurrentItem(Page.USB, true);
                break;
            case R.id.btnStorageSD:
                mViewPager.setCurrentItem(Page.SDCARD, true);
                break;
            case R.id.btnListFolder:
                mViewPager.setCurrentItem(Page.FOLDER, true);
                break;
            case R.id.btnUpdate:
                onUpdateListEvent();
                break;
            case R.id.btnPlaying:
                onBackEvent();
                break;
            case R.id.btnSearch:
                onSearchEvent();
                break;
            default:
                break;
        }
    }

    /**
     * 点击刷新按钮
     * <p> 将触发对应的设备强制扫描；
     */
    private void onUpdateListEvent() {
        switch (mAppData.mMusicListPageType) {
            case IMusicState.PAGE_INDEX_FLASH:
                mFlashMusicListLayout.refresh();
                break;
            case IMusicState.PAGE_INDEX_USB:
                mUSBMusicListLayout.refresh();
                break;
            case IMusicState.PAGE_INDEX_SD:
                mSDMusicListLayout.refresh();
                break;
            case IMusicState.PAGE_INDEX_FOLDER:
                mFolderListLayout.refresh();
                break;
            default:
                break;
        }
    }

    /**
     * 返回上一级页面
     * <p> 列表的上一级页面是播放信息界面；
     */
    private void onBackEvent() {
        if (Objects.isNull(mContext)) {
            return;
        }

        if (Objects.isNull(mMusicViewModel)) {
            return;
        }

        if (!SkinX.getBoolean("folder_unify_back_button", false)) {
            mMusicViewModel.fragment2MainUi().execute(
                    t -> t.onEvent(IMediaEvent.EVENT_SHOW_MUSIC_FRAGMENT,
                            IMusicPage.MCC204_E_GROUP_SHOW_MUSIC_INFO, null));
        } else {
            // 前装 N91 要求文件夹界面回退键与返回音乐信息界面键合为同一返回按键
            if (IConstant.PATH_FLASH.equals(mFolderListLayout.getCurrentFilePath())) {
                mFolderListLayout.setCurrentFilePath(mFolderListLayout.getRootFilePath());
                mFolderListLayout.refresh();
            } else if (!mFolderListLayout.getRootFilePath().equals(mFolderListLayout.getCurrentFilePath())) {
                int index =mFolderListLayout. getCurrentFilePath().lastIndexOf('/');
                if (index > 0) {
                    mFolderListLayout.setCurrentFilePath(mFolderListLayout.getCurrentFilePath().substring(0, index));
                    mFolderListLayout.refresh();
                }
            } else {
                mMusicViewModel.fragment2MainUi().execute(
                        t -> t.onEvent(IMediaEvent.EVENT_SHOW_MUSIC_FRAGMENT,
                                IMusicPage.MCC204_E_GROUP_SHOW_MUSIC_INFO, null));
            }
        }
    }

    /**
     * 点击查找按钮
     * <p> 将切换到 SearchFragment 页面；
     */
    private void onSearchEvent() {
        switch (mAppData.mMusicListPageType) {
            case IMusicState.PAGE_INDEX_FLASH:
                mFlashMusicListLayout.search();
                break;
            case IMusicState.PAGE_INDEX_USB:
                mUSBMusicListLayout.search();
                break;
            case IMusicState.PAGE_INDEX_SD:
                mSDMusicListLayout.search();
                break;
            case IMusicState.PAGE_INDEX_FOLDER:
                mFolderListLayout.search();
                break;
            default:
                break;
        }
    }

    private void onChangeStorageEvent() {
        dispatchCallbackEvent(IMediaEvent.EVENT_CHANGE_MUSIC_STORAGE);
    }

    private void updateStorageButton() {
        mBtnStorageFlash.setEnabled(true);
        mBtnStorageSD.setEnabled(isSdcardMounted());
        mBtnStorageUSB.setEnabled(isUsbMounted());

        if (mAppData.mSelectedDevice == mAppData.mSdStorage) {
            mBtnStorageSD.setChecked(true);
        } else if (mAppData.mSelectedDevice == mAppData.mUsbStorage) {
            mBtnStorageUSB.setChecked(true);
        } else {
            mBtnStorageFlash.setChecked(true);
        }
    }

    private void updateStorageDeviceList() {
        if (!mAppData.mSelectedDevice.isMounted()) {
            mAppData.mSelectedDevice = mAppData.mCurrentDevice;
        }

        updateStorageButton();
        tryUpdateMusicListType();
        updateFragment();
        onChangeStorageEvent();
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
    public void onStop() {
        super.onStop();
        Log.d(TAG, "onStop");
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LogUtil.d(TAG, "onDestroyView.");

        // 清理 ViewPager 中的 View;
        mViewPager.removeAllViews();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
    }

    @Override
    public void onDetach() {
        super.onDetach();
        Log.d(TAG, "onDetach");
    }

    /**
     * 页面类型定义
     * <p> 结合 ViewPagerAdapter 使用；
     */
    private interface Page {
        int FLASH = 0;
        int USB = 1;
        int SDCARD = 2;
        int FOLDER = 3;
        int COUNT = 4;
    }

    /**
     * ViewPager 适配器
     * <p> 管理 Flash/USB/SDCard/Folder 视图的切换；
     */
    private class ViewPagerAdapter extends PagerAdapter implements Page {
        static final int VIEW_TAG_KEY = 0xEE00F001;

        public ViewPagerAdapter(Context context) {
        }

        @Override
        public int getCount() {
            return COUNT;
        }

        @Override
        public boolean isViewFromObject(@NonNull View view, @NonNull Object object) {
            return view == object;
        }

        /** 初始化内置存储列表布局 **/
        private View initFlashListLayout() {
            if (mFlashMusicListLayout == null) {
                mFlashMusicListLayout = new MusicListLayout(mContext, mMusicViewModel);
                mFlashMusicListLayout.setTag(VIEW_TAG_KEY, "flash");
                mFlashMusicListLayout.initDataObject();
                mFlashMusicListLayout.setMediaEventListener(mPostbox);
                mFlashMusicListLayout.setStorageDevice(mAppData.mFlashStorage);
            } else {
                mFlashMusicListLayout.initLayout();
            }

            return mFlashMusicListLayout;
        }

        /** 初始化 USB 储列表布局 **/
        private View initUsbListLayout() {
            if (mUSBMusicListLayout == null) {
                mUSBMusicListLayout = new MusicListLayout(mContext, mMusicViewModel);
                mUSBMusicListLayout.setTag(VIEW_TAG_KEY, "usb");
                mUSBMusicListLayout.initDataObject();
                mUSBMusicListLayout.setMediaEventListener(mPostbox);
                mUSBMusicListLayout.setStorageDevice(mAppData.mUsbStorage);
            } else {
                mUSBMusicListLayout.initLayout();
            }

            return mUSBMusicListLayout;
        }

        /** 初始化 SDCard 存储列表布局 **/
        private View initSdCardListLayout() {
            if (mSDMusicListLayout == null) {
                mSDMusicListLayout = new MusicListLayout(mContext, mMusicViewModel);
                mSDMusicListLayout.setTag(VIEW_TAG_KEY, "sd");
                mSDMusicListLayout.initDataObject();
                mSDMusicListLayout.setMediaEventListener(mPostbox);
                mSDMusicListLayout.setStorageDevice(mAppData.mSdStorage);
            } else {
                mSDMusicListLayout.initLayout();
            }

            return mSDMusicListLayout;
        }

        /** 初始化文件夹存储列表布局 **/
        private View initFolderListLayout() {
            if (mFolderListLayout == null) {
                mFolderListLayout = new FolderListLayout(mContext, mMusicViewModel);
                mFolderListLayout.setTag(VIEW_TAG_KEY, "folder");
                mFolderListLayout.initDataObject();
                mFolderListLayout.setMediaEventListener(mPostbox);
            }

            mFolderListLayout.initLayout();
            return mFolderListLayout;
        }

        @NonNull
        @Override
        public View instantiateItem(@NonNull ViewGroup container, int position) {
            View view = null;

            // 根据 Item 位置初始化显示布局
            if (position == FLASH) {
                view = initFlashListLayout();
            } else if (position == USB) {
                view = initUsbListLayout();
            } else if (position == SDCARD) {
                view = initSdCardListLayout();
            } else if (position == FOLDER) {
                view = initFolderListLayout();
            }

            assert view != null;
            String viewTag = (String) view.getTag(VIEW_TAG_KEY);
            int childCount = container.getChildCount();
            LogUtil.v(TAG, "instantiateItem: "
                    + "position = " + position + "/" + childCount +  ", tag = " + viewTag);

            // 重复添加 view 到父对象将导致 IllegalStateException
            if (view.getParent() != null) {
                LogUtil.v(TAG, "instantiateItem: The specified child already has a parent.");
                container.removeView(view);
            }

            container.addView(view,
                    LinearLayout.LayoutParams.MATCH_PARENT,
                    LinearLayout.LayoutParams.MATCH_PARENT);
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
}
