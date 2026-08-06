package com.hcn.media.music.gallery;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Bundle;
import android.os.Message;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.RadioButton;

import androidx.annotation.NonNull;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.fragment.IMusicPage;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_data.storage.IStorageDevice;
import com.hcn.media.folder.MediaFilePathScan;
import com.hcn.media.music.base.FolderListLayout;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media.music.base.AlbumListLayout;
import com.hcn.media.music.base.ArtistListLayout;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media_view.widget.PagerAdapterEx;
import com.hcn.media_view.widget.ViewPagerEx;

import java.util.Objects;

/**
 * mcc154 音乐列表页面
 * <p> 带 ID3 分类列表的页面，特定客户定制实现，不再过多维护；
 *
 * @author 65821
 */
public class MusicListFragmentEx extends MediaFragment
        implements OnClickListener, ViewPagerEx.OnPageChangeListener {
    private final static String FRAGMENT_NAME = "music-list-mcc154";
    private static final String TAG = MusicListFragmentEx.class.getSimpleName();

    private boolean mInitView = false;

    private ViewPagerEx mViewPager = null;
    private ViewPagerAdapter mViewPagerAdapter = null;

    private RadioButton mBtnListSong = null;
    private RadioButton mBtnListAlbum = null;
    private RadioButton mBtnListArtist = null;
    private RadioButton mBtnListFolder = null;

    private RadioButton mBtnStorageFlash = null;
    private RadioButton mBtnStorageSD = null;
    private RadioButton mBtnStorageUSB = null;

    private Button mBtnUpdateList = null;
    private View mSearchLayout = null;
    private RadioButton mBtnPlaying = null;

    /** 分类列表布局视图 **/
    private MusicListLayoutEx mMusicListLayout = null;
    private FolderListLayout mFolderListLayout = null;
    private AlbumListLayout mAlbumListLayout = null;
    private ArtistListLayout mArtistListLayout = null;

    /** 默认无参构造函数 **/
    public MusicListFragmentEx() {
        super(FRAGMENT_NAME);
    }

    @Override
    public void initFragment() {
        updateFragment();
    }

    @Override
    public void uninitFragment() {
        super.uninitFragment();
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
    public int getLayoutRes() {
        return R.layout.fragment_musiclist;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        LogUtil.v(TAG, "onCreateView.");
        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);

        assert view != null;
        initView(view);
        initFragment();
        mInitView = true;

        return view;
    }

    private void initView(View layout) {
        // 信息分类列表
        mBtnListSong = layout.findViewById(xId(R.id.btnListSong));
        if (mBtnListSong != null) {
            mBtnListSong.setOnClickListener(this);
        }

        mBtnListAlbum = layout.findViewById(xId(R.id.btnListAlbum));
        if (mBtnListAlbum != null) {
            mBtnListAlbum.setOnClickListener(this);
        }

        mBtnListArtist = layout.findViewById(xId(R.id.btnListArtist));
        if (mBtnListArtist != null) {
            mBtnListArtist.setOnClickListener(this);
        }

        mBtnListFolder = layout.findViewById(xId(R.id.btnListFolder));
        if (mBtnListFolder != null) {
            mBtnListFolder.setOnClickListener(this);
        }

        // 存储设备列表
        mBtnStorageFlash = layout.findViewById(xId(R.id.btnStorageFlash));
        mBtnStorageUSB = layout.findViewById(xId(R.id.btnStorageUSB1));
        mBtnStorageSD = layout.findViewById(xId(R.id.btnStorageSD1));

        mBtnStorageFlash.setOnClickListener(this);
        mBtnStorageUSB.setOnClickListener(this);
        mBtnStorageSD.setOnClickListener(this);

        // 操作功能按钮
        mSearchLayout = layout.findViewById(xId(R.id.layout_search));
        mBtnUpdateList = layout.findViewById(xId(R.id.btnUpdate));
        mBtnPlaying = layout.findViewById(xId(R.id.btnPlaying));

        mSearchLayout.setOnClickListener(this);
        mBtnUpdateList.setOnClickListener(this);
        mBtnPlaying.setOnClickListener(this);

        // 视图页面增强控件
        // Debug: ViewPagerEx.setDebug(Utils.isDebugVersion());
        mViewPager = layout.findViewById(xId(R.id.viewpager_center));
        if (mViewPager != null) {
            mViewPagerAdapter = new ViewPagerAdapter(mContext);
            mViewPager.setAdapter(mViewPagerAdapter);
            mViewPager.setOffscreenPageLimit(4);
            mViewPager.setOnPageChangeListener(this);
        }
    }

    /** 更新页面显示内容 **/
    private void updateFragment() {
        if (Objects.isNull(mViewPager)) {
            return;
        }

        LogUtil.v(TAG, "updateFragment.");

        switch (mAppData.mMusicListPageType) {
            case IMusicState.PAGE_INDEX_ALBUM:
                mViewPager.setCurrentItem(0x01);
                break;
            case IMusicState.PAGE_INDEX_ARTIST:
                mViewPager.setCurrentItem(0x02);
                break;
            case IMusicState.PAGE_INDEX_FOLDER:
                mViewPager.setCurrentItem(0x03);
                break;
            case IMusicState.PAGE_INDEX_MUSIC:
            default:
                mViewPager.setCurrentItem(0x00);
                break;
        }

        updateStorageButton();
        updateId3ListTypeMenu();
        enableListUpdateButton();
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
    public void onResume() {
        super.onResume();
        LogUtil.v(TAG, "onResume.");
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // 只处理确认授权的事件
        if (eventId == IMediaEvent.EVENT_GOTO_MUSIC_SEARCH_PAGE) {
            mMusicViewModel.fragment2MainUi().execute(
                    t -> t.onEvent(eventId, wParam, lParam));
        }
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_MEDIA_LOADING_START:
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
                enableListUpdateButton();
                break;
            case IMediaEvent.EVENT_MEDIA_MOUNTED:
                updateStorageButton();
                H0.sendEmptyUniqueMessageDelayed(
                        MsgEx.MSG_UPDATE_STORAGE_DEVICE_STATE, 1500);
                break;
            case IMediaEvent.EVENT_MEDIA_UNMOUNTED:
                H0.sendEmptyUniqueMessageDelayed(
                        MsgEx.MSG_UPDATE_STORAGE_DEVICE_LIST_INFO, 1000);
                break;
            default:
                break;
        }

        dispatchCallbackEvent(eventId);
    }

    /** 分发事件到子元素视图 **/
    private void dispatchCallbackEvent(int eventId) {
        if (mMusicListLayout != null) {
            mMusicListLayout.doCallbackEvent(eventId);
        }

        if (mAlbumListLayout != null) {
            mAlbumListLayout.doCallbackEvent(eventId);
        }

        if (mArtistListLayout != null) {
            mArtistListLayout.doCallbackEvent(eventId);
        }

        if (mFolderListLayout != null) {
            mFolderListLayout.doCallbackEvent(eventId);
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        LogUtil.v(TAG, "onPause.");
    }

    @Override
    public void onPageScrolled(int arg0, float arg1, int arg2) {
        LogUtil.v(TAG, "onPageScrolled: " + arg0);
    }

    @Override
    public void onPageScrollStateChanged(int arg0) {
    }

    @Override
    public void onPageSelected(int position) {
        switch (position) {
            case 0x00:
                mAppData.mMusicListPageType = IMusicState.PAGE_INDEX_MUSIC;
                break;
            case 0x01:
                mAppData.mMusicListPageType = IMusicState.PAGE_INDEX_ALBUM;
                break;
            case 0x02:
                mAppData.mMusicListPageType = IMusicState.PAGE_INDEX_ARTIST;
                break;
            case 0x03:
                mAppData.mMusicListPageType = IMusicState.PAGE_INDEX_FOLDER;
                break;
            default:
                break;
        }

        updateId3ListTypeMenu();
    }

    /** 更新 ID3 列表类型菜单 **/
    private void updateId3ListTypeMenu() {
        int listType = mAppData.mMusicListPageType;
        LogUtil.v(TAG, "updateId3ListTypeMenu: " + listType);

        switch (listType) {
            case IMusicState.PAGE_INDEX_ALBUM:
                mBtnListAlbum.setChecked(true);
                break;
            case IMusicState.PAGE_INDEX_ARTIST:
                mBtnListArtist.setChecked(true);
                break;
            case IMusicState.PAGE_INDEX_FOLDER:
                mBtnListFolder.setChecked(true);
                break;
            case IMusicState.PAGE_INDEX_MUSIC:
            default:
                mBtnListSong.setChecked(true);
                break;
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (getId(v)) {
            case R.id.btnListSong:
                mViewPager.setCurrentItem(0x00, true);
                break;
            case R.id.btnListAlbum:
                mViewPager.setCurrentItem(0x01, true);
                break;
            case R.id.btnListArtist:
                mViewPager.setCurrentItem(0x02, true);
                break;
            case R.id.btnListFolder:
                mViewPager.setCurrentItem(0x03, true);
                break;
            case R.id.btnUpdate:
                onUpdateListEvent();
                break;
            case R.id.btnStorageFlash:
                mAppData.mSelectedDevice = mAppData.mFlashStorage;
                onChangeStorageEvent();
                break;
            case R.id.btnStorageSD1:
                mAppData.mSelectedDevice = mAppData.mSdStorage;
                onChangeStorageEvent();
                break;
            case R.id.btnStorageUSB1:
                mAppData.mSelectedDevice = mAppData.mUsbStorage;
                onChangeStorageEvent();
                break;
            case R.id.btnPlaying:
                onBackEvent();
                break;
            case R.id.layout_search:
                onSearchEvent();
                break;
            default:
                break;
        }
    }

    /** 更新当前选择的存储设备列表 **/
    private void onUpdateListEvent() {
        if (Objects.isNull(mAppData.mSelectedDevice)) {
            return;
        }

        mMusicViewModel.playerRelay().accept(
                t -> t.requestExecuteAction(
                        IMediaAction.scanStorageDeviceInfo,
                        mAppData.mSelectedDevice.mFilePath,
                        null));
    }

    /** 使能列表更新按钮状态 **/
    private void enableListUpdateButton() {
        if (Objects.isNull(mAppData.mSelectedDevice)) {
            return;
        }

        if (mBtnUpdateList != null) {
            mBtnUpdateList.setEnabled(!mAppData.mSelectedDevice.isLoading());
        }
    }

    private void onBackEvent() {
        mMusicViewModel.fragment2MainUi().execute(
                t -> t.onEvent(IMediaEvent.EVENT_SHOW_MUSIC_FRAGMENT,
                        IMusicPage.E_GROUP_SHOW_MUSIC_INFO_EX, null));
    }

    private void onSearchEvent() {
        mMusicViewModel.fragment2MainUi().execute(
                t -> t.onEvent(IMediaEvent.EVENT_SHOW_MUSIC_FRAGMENT,
                        IMusicPage.E_GROUP_SHOW_MUSIC_SEARCH_EX, null));
    }

    private void onChangeStorageEvent() {
        enableListUpdateButton();
        dispatchCallbackEvent(IMediaEvent.EVENT_CHANGE_MUSIC_STORAGE);
    }

    /**
     * 更新存储设备列表信息
     * <p> 当前正在播放的 USB/SD 移除后需要更新列表信息；
     */
    private void updateStorageDeviceList() {
        if (!mAppData.mSelectedDevice.isMounted()) {
            mAppData.mSelectedDevice = mAppData.mCurrentDevice;
        }

        updateFragment();
    }

    /**
     * 更新存储按键状态
     * <p> 设备不存在需要灰调按钮；
     */
    private void updateStorageButton() {
        mBtnStorageFlash.setEnabled(true);
        mBtnStorageSD.setEnabled(isSdcardMounted());
        mBtnStorageUSB.setEnabled(isUsbMounted());

        // 当前选择的存储设备
        switch (mAppData.mSelectedDevice.storageType()) {
            case IStorageDevice.STORAGE_TYPE_USB:
                mBtnStorageUSB.setChecked(true);
                break;
            case IStorageDevice.STORAGE_TYPE_SDCARD:
                mBtnStorageSD.setChecked(true);
                break;
            case IStorageDevice.STORAGE_TYPE_FLASH:
            default:
                mBtnStorageFlash.setChecked(true);
                break;
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LogUtil.d(TAG, "onDestroyView.");

        // 清理 ViewPager 中的 View;
        mViewPager.removeAllViews();
        mViewPager = null;
    }

    /**
     * ViewPager 适配器
     * <p> 列表（歌曲/艺术家/专辑/文件夹）页面创建显隐管理
     */
    private class ViewPagerAdapter extends PagerAdapterEx {
        static final int VIEW_TAG_KEY = 0xEE00F001;

        public ViewPagerAdapter(Context context) {
        }

        @Override
        public int getCount() {
            return 4;
        }

        @Override
        public boolean isViewFromObject(@NonNull View view, @NonNull Object object) {
            return view == object;
        }

        /** 初始化歌曲列表布局 **/
        private View initSongListLayout() {
            if (mMusicListLayout == null) {
                mMusicListLayout = new MusicListLayoutEx(mContext, mMusicViewModel);
                mMusicListLayout.setTag(VIEW_TAG_KEY, "list");
                mMusicListLayout.initDataObject();
                mMusicListLayout.setMediaEventListener(mPostbox);
            } else {
                mMusicListLayout.initLayout();
            }

            return mMusicListLayout;
        }

        /** 初始化专辑列表布局 **/
        private View initAlbumListLayout() {
            if (mAlbumListLayout == null) {
                mAlbumListLayout = new AlbumListLayout(mContext, mMusicViewModel);
                mAlbumListLayout.setTag(VIEW_TAG_KEY, "album");
                mAlbumListLayout.initDataObject();
                mAlbumListLayout.setMediaEventListener(mPostbox);
            } else {
                mAlbumListLayout.initLayout();
            }

            return mAlbumListLayout;
        }

        /** 初始化艺术家列表布局 **/
        private View initArtistListLayout() {
            if (mArtistListLayout == null) {
                mArtistListLayout = new ArtistListLayout(mContext, mMusicViewModel);
                mArtistListLayout.setTag(VIEW_TAG_KEY, "artist");
                mArtistListLayout.initDataObject();
                mArtistListLayout.setMediaEventListener(mPostbox);
            } else {
                mArtistListLayout.initLayout();
            }

            return mArtistListLayout;
        }

        /** 初始化文件夹列表布局 **/
        private View initFolderListLayout() {
            if (mFolderListLayout == null) {
                mFolderListLayout = new FolderListLayout(
                        mContext, null, 0, mMusicViewModel,
                        mAppData.mCurrentDevice.mFilePath, MediaFilePathScan.SINGLE_STORAGE_MODE);
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
            if (position == 0x00) {
                view = initSongListLayout();
            } else if (position == 0x01) {
                view = initAlbumListLayout();
            } else if (position == 0x02) {
                view = initArtistListLayout();
            } else if (position == 0x03) {
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

            container.addView(view, LinearLayout.LayoutParams.MATCH_PARENT,
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

        /** 重置所有选项 **/
        public void resetAllItem() {
            mMusicListLayout = null;
            mAlbumListLayout = null;
            mArtistListLayout = null;
            mFolderListLayout = null;
        }
    }
}
