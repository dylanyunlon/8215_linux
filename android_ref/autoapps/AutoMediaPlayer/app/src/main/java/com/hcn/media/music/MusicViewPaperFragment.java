package com.hcn.media.music;

import static android.carsource.McuConstant.K_EQ;

import android.annotation.SuppressLint;
import android.carsource.McuManager;
import android.content.ContentResolver;
import android.content.Context;
import android.os.Bundle;
import android.os.Message;
import android.os.SystemClock;
import android.provider.Settings;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.RadioButton;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.viewpager.widget.PagerAdapter;
import androidx.viewpager.widget.ViewPager;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.common.app.HActivityUtils;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_theme.Argument;
import com.hcn.media_theme.ThemeX;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.fragment.IMusicPage;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media.local.utils.HFuncUtils;
import com.hcn.media.music.base.FolderListLayout;
import com.hcn.media.music.base.MusicInfoLayout;
import com.hcn.media.music.base.MusicListLayout;
import com.hcn.media.music.mcc401.Mcc401MusicInfoLayout;

import java.util.Objects;

/**
 * 播放界面
 * <p> 当不配置 Theme 的时候默认主题；
 * @author 65821
 */
@SuppressLint("ValidFragment")
public class MusicViewPaperFragment extends MediaFragment
        implements OnClickListener, ViewPager.OnPageChangeListener, ITouchEventListener {
    private static final String FRAGMENT_NAME = "music-ViewPaper";
    private static final String TAG = MusicViewPaperFragment.class.getSimpleName();

    /**
     * 当前 Fragment 在可见状态
     * <p> 该变量意思是当前 Fragment 是否是对用户可见（眼睛是否能看到页面）。
     */
    private boolean mInVisibleState = false;

    private ViewPager mViewPager = null;
    private boolean mInitView = false;

    private Button mBtnEq = null;
    private RadioButton mBtnStorageFlash = null;
    private RadioButton mBtnStorageUSB = null;
    private RadioButton mBtnStorageSd = null;
    private RadioButton mBtnPlaying = null;
    private RadioButton mBtnFolderList = null;

    private FolderListLayout mFolderListLayout = null;
    private MusicInfoLayout mMusicInfoLayout = null;
    private Mcc401MusicInfoLayout mMcc401MusicInfoLayout = null;
    private MusicListLayout mMultiUSBMusicLayout = null;
    private MusicListLayout mMultiFlashMusicLayout = null;
    private MusicListLayout mMultiSDMusicLayout = null;

    /**
     * 最近一次触摸列表布局的时间
     * <p> 过滤处理 ITouchEventListener 的事件触发动作, 提高效率。
     */
    private long mLastTouchListTime = -1;

    public MusicViewPaperFragment() {
        super(FRAGMENT_NAME);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        LogUtil.e(TAG, "onCreate");
    }

    @Override
    public void initFragment() {
        // 恢复上次页面类型
        if (mViewPager != null) {
            gotoMusicListPageItem(mAppData.mMusicListPageType);
        } else {
            throw new NullPointerException("initFragment() needs to be called after initView()!");
        }
    }

    @Override
    public void uninitFragment() {
        super.uninitFragment();
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fragment_music_viewpaper;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);

        assert view != null;
        initView(view);
        initFragment();

        mInitView = true;
        return view;
    }

    @SuppressLint("UseCompatLoadingForDrawables")
    private void initView(View layout) {
        mBtnEq = layout.findViewById(xId(R.id.btnEQ));
        mBtnStorageFlash = layout.findViewById(xId(R.id.btnStorageFlash));
        mBtnStorageUSB = layout.findViewById(xId(R.id.btnStorageUSB1));
        mBtnFolderList = layout.findViewById(xId(R.id.btnListFolder));
        mBtnPlaying = layout.findViewById(xId(R.id.btnPlaying));
        mBtnStorageSd = layout.findViewById(xId(R.id.btnStorageSD));

        // 指定的主界面風格
        ContentResolver resolver = mContext.getContentResolver();
        if (1 == Settings.System.getInt(resolver, "car_home_style", 0)) {
            mBtnStorageFlash.setCompoundDrawablesWithIntrinsicBounds(
                    null,
                    xDrawable(R.drawable.rb_flash_bg_style_1),
                    null,
                    null);
            mBtnStorageUSB.setCompoundDrawablesWithIntrinsicBounds(
                    null,
                    xDrawable(R.drawable.rb_usb_bg_style_1),
                    null,
                    null);
            mBtnFolderList.setCompoundDrawablesWithIntrinsicBounds(
                    null,
                    xDrawable(R.drawable.rb_queue_bg_style_1),
                    null,
                    null);
            mBtnPlaying.setCompoundDrawablesWithIntrinsicBounds(
                    null,
                    xDrawable(R.drawable.rb_playing_bg_style_1),
                    null,
                    null);
            mBtnEq.setCompoundDrawablesWithIntrinsicBounds(
                    null,
                    xDrawable(R.drawable.rb_eq_bg_style_1),
                    null,
                    null);
        }

        mBtnEq.setOnClickListener(this);
        mBtnStorageFlash.setOnClickListener(this);
        mBtnStorageUSB.setOnClickListener(this);
        mBtnStorageSd.setOnClickListener(this);
        mBtnFolderList.setOnClickListener(this);
        mBtnPlaying.setOnClickListener(this);

        mViewPager = layout.findViewById(xId(R.id.viewpager_center));
        if (mViewPager != null) {
            ViewPagerAdapter viewPagerAdapter = new ViewPagerAdapter(mContext);
            mViewPager.setAdapter(viewPagerAdapter);
            mViewPager.setOnPageChangeListener(this);
        }
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        adjustLayoutElements(view);
    }

    /**
     * 跳转布局元素
     * <pre>
     *    对于特定 UI 在横竖屏显示的时候做显示调整；
     *    主要是为了减少在 res/layout-port/ 中添加相识度高的类重复布局文件；
     * </pre>
     *
     * @param view 当前页面关联视图
     */
    private void adjustLayoutElements(@NonNull View view) {
        // 针对分配竖屏，我们统一不显示 EQ 按钮
        if (requireActivity().isInMultiWindowMode()
                && MiscUtils.isPortraitWindow(requireContext())) {
            if (mBtnEq != null) {
                mBtnEq.setVisibility(View.GONE);
            }
        } else {
            if (mBtnEq != null) {
                mBtnEq.setVisibility(View.VISIBLE);
            }
        }
    }

    @Override
    public void onPageScrolled(int arg0, float arg1, int arg2) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onPageScrollStateChanged(int arg0) {
    }

    @Override
    public void onPageSelected(int position) {
        switch (position) {
            case IMusicState.PAGE_INDEX_FLASH:
                mAppData.mSelectedDevice = mAppData.mFlashStorage;
                break;
            case IMusicState.PAGE_INDEX_USB:
                mAppData.mSelectedDevice = mAppData.mUsbStorage;
                break;
            case IMusicState.PAGE_INDEX_SD:
                mAppData.mSelectedDevice = mAppData.mSdStorage;
                break;
            case IMusicState.PAGE_INDEX_FOLDER:
            case IMusicState.PAGE_INDEX_PLAY:
            default:
                break;
        }

        mAppData.mMusicListPageType = position;
        onChangeListType(mAppData.mMusicListPageType);
    }

    private void onChangeListType(int nListType) {
        switch (nListType) {
            case IMusicState.PAGE_INDEX_PLAY:
                mBtnPlaying.setChecked(true);
                break;
            case IMusicState.PAGE_INDEX_FLASH:
                mBtnStorageFlash.setChecked(true);
                break;
            case IMusicState.PAGE_INDEX_USB:
                mBtnStorageUSB.setChecked(true);
                break;
            case IMusicState.PAGE_INDEX_SD:
                mBtnStorageSd.setChecked(true);
                break;
            case IMusicState.PAGE_INDEX_FOLDER:
                mBtnFolderList.setChecked(true);
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
                gotoMusicListPageItem(IMusicState.PAGE_INDEX_FLASH, true);
                break;
            case R.id.btnStorageUSB1:
                gotoMusicListPageItem(IMusicState.PAGE_INDEX_USB, true);
                break;
            case R.id.btnStorageSD:
                gotoMusicListPageItem(IMusicState.PAGE_INDEX_SD, true);
                break;
            case R.id.btnListFolder:
                gotoMusicListPageItem(IMusicState.PAGE_INDEX_FOLDER, true);
                break;
            case R.id.btnPlaying:
                showMusicInfo();
                break;
            case R.id.btnEQ:
                // 跳转到音效界面
                HFuncUtils.instance().gotoEQ(mContext);
                break;
            default:
                break;
        }
    }

    /**
     * 显示音乐信息页面
     */
    public boolean showMusicInfo() {
        if (mViewPager != null
                && mViewPager.getCurrentItem() != IMusicState.PAGE_INDEX_PLAY) {
            mViewPager.setCurrentItem(IMusicState.PAGE_INDEX_PLAY, true);
            return true;
        }
        return false;
    }

    /**
     * 跳转到指定的列表页
     * <p> FLASH、USB、SD、FOLDER、PLAYING...
     *
     * @param item page 索引
     */
    private void gotoMusicListPageItem(int item) {
        switch (item) {
            case IMusicState.PAGE_INDEX_FLASH:
            case IMusicState.PAGE_INDEX_USB:
            case IMusicState.PAGE_INDEX_SD:
                mViewPager.setCurrentItem(item);
                break;

            case IMusicState.PAGE_INDEX_FOLDER:
                mViewPager.setCurrentItem(item);
                if (mFolderListLayout != null) {
                    mFolderListLayout.initLayout();
                }
                break;

            case IMusicState.PAGE_INDEX_PLAY:
                mViewPager.setCurrentItem(item);
                return;

            default:
                break;
        }

        // 开始监听列表活动状态
        H0.sendEmptyUniqueMessageDelayed(
                MsgEx.MSG_LIST_PAGE_OPERATION_TIMEOUT, 8000);
    }

    /**
     * 跳转到指定的列表页
     *
     * @param item page 索引
     * @param smoothScroll 是否平滑切换
     */
    private void gotoMusicListPageItem(int item, boolean smoothScroll) {
        switch (item) {
            case IMusicState.PAGE_INDEX_FLASH:
            case IMusicState.PAGE_INDEX_USB:
            case IMusicState.PAGE_INDEX_SD:
                mViewPager.setCurrentItem(item, smoothScroll);
                break;

            case IMusicState.PAGE_INDEX_FOLDER:
                mViewPager.setCurrentItem(item, smoothScroll);
                if (mFolderListLayout != null) {
                    mFolderListLayout.initLayout();
                }
                break;

            case IMusicState.PAGE_INDEX_PLAY:
                mViewPager.setCurrentItem(item);
                return;

            default:
                break;
        }

        // 开始监听列表活动状态
        H0.sendEmptyUniqueMessageDelayed(
                MsgEx.MSG_LIST_PAGE_OPERATION_TIMEOUT, 8000);
    }

    private void onSearchEvent() {
        switch (mAppData.mMusicListPageType) {
            case IMusicState.PAGE_INDEX_FLASH:
                mMultiFlashMusicLayout.search();
                break;
            case IMusicState.PAGE_INDEX_USB:
                mMultiUSBMusicLayout.search();
                break;
            case IMusicState.PAGE_INDEX_SD:
                mMultiSDMusicLayout.search();
                break;
            case IMusicState.PAGE_INDEX_FOLDER:
                mFolderListLayout.search();
                break;
            default:
                break;
        }
    }

    private void onEQEvent() {
        McuManager.getsInstance()
                .injectKeyEventTimeout(K_EQ, 50);
    }

    /**
     * ViewPagerAdapter
     * ViewPagerAdapt* <pre>
     *    本意是动态管理各个页面，方便资源管控，为系统性能提供动态调整支持；
     *    但实际上最后 addView 的 Item 都是常驻内存的（Fuck the design）;
     *    所以尽可能不要这么干，该释放的就释放；
     * </pre>
     */
    private class ViewPagerAdapter extends PagerAdapter {
        static final int VIEW_TAG_KEY = 0xEE00F001;

        public ViewPagerAdapter(Context context) {
        }

        @Override
        public int getCount() {
            return 5;
        }

        @Override
        public boolean isViewFromObject(@NonNull View view, @NonNull Object object) {
            return view == object;
        }

        /** 初始化音乐信息布局 **/
        private View initMusicInfoLayout() {
            // mcc401 客户定制布局
            if (Argument.isThemeGod(ThemeX.ET_GOD_401)) {
                if (mMcc401MusicInfoLayout == null) {
                    mMcc401MusicInfoLayout = new Mcc401MusicInfoLayout(mContext, mMusicViewModel);
                    mMcc401MusicInfoLayout.setTag(VIEW_TAG_KEY, "info");
                    mMcc401MusicInfoLayout.initDataObject();
                    mMcc401MusicInfoLayout.setMediaEventListener(mPostbox);
                } else {
                    mMcc401MusicInfoLayout.initLayout();
                }

                return mMcc401MusicInfoLayout;
            }

            // 默认播放信息布局
            if (mMusicInfoLayout == null) {
                mMusicInfoLayout = new MusicInfoLayout(mContext, mMusicViewModel);
                mMusicInfoLayout.setTag(VIEW_TAG_KEY, "info");
                mMusicInfoLayout.initDataObject();
                mMusicInfoLayout.setMediaEventListener(mPostbox);
            } else {
                mMusicInfoLayout.initLayout();
            }

            return mMusicInfoLayout;
        }

        /** 初始化内置存储列表布局 **/
        private View initFlashListLayout() {
            if (null == mMultiFlashMusicLayout) {
                mMultiFlashMusicLayout = new MusicListLayout(mContext, mMusicViewModel);
                mMultiFlashMusicLayout.setTag(VIEW_TAG_KEY, "flash");
                mMultiFlashMusicLayout.initDataObject();
                mMultiFlashMusicLayout.setMediaEventListener(mPostbox);
                mMultiFlashMusicLayout.setStorageDevice(mAppData.mFlashStorage);
                mMultiFlashMusicLayout.setTouchEventListener(MusicViewPaperFragment.this);
            } else {
                mMultiFlashMusicLayout.initLayout();
            }

            return mMultiFlashMusicLayout;
        }

        /** 初始化 USB 存储列表布局 **/
        private View initUsbListLayout() {
            if (mMultiUSBMusicLayout == null) {
                mMultiUSBMusicLayout = new MusicListLayout(mContext, mMusicViewModel);
                mMultiUSBMusicLayout.setTag(VIEW_TAG_KEY, "usb");
                mMultiUSBMusicLayout.initDataObject();
                mMultiUSBMusicLayout.setMediaEventListener(mPostbox);
                mMultiUSBMusicLayout.setStorageDevice(mAppData.mUsbStorage);
                mMultiUSBMusicLayout.setTouchEventListener(MusicViewPaperFragment.this);
            } else {
                mMultiUSBMusicLayout.initLayout();
            }

            return mMultiUSBMusicLayout;
        }

        /** 初始化 SDCard 存储列表布局 **/
        private View initSdCardListLayout() {
            if (mMultiSDMusicLayout == null) {
                mMultiSDMusicLayout = new MusicListLayout(mContext, mMusicViewModel);
                mMultiSDMusicLayout.setTag(VIEW_TAG_KEY, "sd");
                mMultiSDMusicLayout.initDataObject();
                mMultiSDMusicLayout.setMediaEventListener(mPostbox);
                mMultiSDMusicLayout.setStorageDevice(mAppData.mSdStorage);
                mMultiSDMusicLayout.setTouchEventListener(MusicViewPaperFragment.this);
            } else {
                mMultiSDMusicLayout.initLayout();
            }

            return mMultiSDMusicLayout;
        }

        /** 初始化文件夹存储显示列表布局 **/
        private View initFolderListLayout() {
            if (mFolderListLayout == null) {
                mFolderListLayout = new FolderListLayout(mContext, mMusicViewModel);
                mFolderListLayout.setTag(VIEW_TAG_KEY, "folder");
                mFolderListLayout.initDataObject();
                mFolderListLayout.setMediaEventListener(mPostbox);
                mFolderListLayout.setTouchEventListener(MusicViewPaperFragment.this);
            }

            mFolderListLayout.initLayout();
            return mFolderListLayout;
        }

        @NonNull
        @Override
        public View instantiateItem(@NonNull ViewGroup container, int position) {
            View view = null;

            // 根据 Item 位置初始化显示布局
            if (position == IMusicState.PAGE_INDEX_PLAY) {
               view = initMusicInfoLayout();
            } else if (position == IMusicState.PAGE_INDEX_FLASH) {
                view = initFlashListLayout();
            } else if (position == IMusicState.PAGE_INDEX_USB) {
                view = initUsbListLayout();
            } else if (position == IMusicState.PAGE_INDEX_SD) {
                view = initSdCardListLayout();
            } else if (position == IMusicState.PAGE_INDEX_FOLDER) {
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
    }

    @Override
    public void onResume() {
        super.onResume();
        LogUtil.e(TAG, "onResume.");

        onVisibleStateChanged(true);
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
            return;
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
            case IMediaEvent.EVENT_CONTROL_SMART_CW:
            case IMediaEvent.EVENT_CONTROL_SMART_CCW:
            case IMediaEvent.EVENT_CANCEL_SMART_CONTROL:
                dispatchSmartControlEvent(eventId);
                break;

            default:
                dispatchCallbackEvent(eventId);
                break;
        }
    }

    /**
     * 处理 SMART 回调事件
     * <p> 这里 smart 事件只分发给当前显示列表视图；
     *
     * @param event 事件 ID
     */
    private void dispatchSmartControlEvent(int event) {
        switch (mAppData.mMusicListPageType) {
            case IMusicState.PAGE_INDEX_PLAY:
                if (Argument.isThemeGod(ThemeX.ET_GOD_401)) {
                    if (mMcc401MusicInfoLayout != null) {
                        mMcc401MusicInfoLayout.doCallbackEvent(event);
                    }
                } else {
                    if (mMusicInfoLayout != null) {
                        mMusicInfoLayout.doCallbackEvent(event);
                    }
                }
                break;
            case IMusicState.PAGE_INDEX_FLASH:
                if (mMultiFlashMusicLayout != null) {
                    mMultiFlashMusicLayout.doCallbackEvent(event);
                }
                break;
            case IMusicState.PAGE_INDEX_USB:
                if (mMultiUSBMusicLayout != null) {
                    mMultiUSBMusicLayout.doCallbackEvent(event);
                }
                break;
            case IMusicState.PAGE_INDEX_SD:
                if (mMultiSDMusicLayout != null) {
                    mMultiSDMusicLayout.doCallbackEvent(event);
                }
                break;
            case IMusicState.PAGE_INDEX_FOLDER:
                if (mFolderListLayout != null) {
                    mFolderListLayout.doCallbackEvent(event);
                }
                break;
            default:
                break;
        }
    }

    /** 公共媒体事件分发处理 **/
    private void dispatchCallbackEvent(int eventId) {
        if (mFolderListLayout != null) {
            mFolderListLayout.doCallbackEvent(eventId);
        }

        if (Argument.isThemeGod(ThemeX.ET_GOD_401)) {
            if (mMcc401MusicInfoLayout != null) {
                mMcc401MusicInfoLayout.doCallbackEvent(eventId);
            }
        } else {
            if (mMusicInfoLayout != null) {
                mMusicInfoLayout.doCallbackEvent(eventId);
            }
        }

        if (mMultiUSBMusicLayout != null) {
            mMultiUSBMusicLayout.doCallbackEvent(eventId);
        }

        if (mMultiFlashMusicLayout != null) {
            mMultiFlashMusicLayout.doCallbackEvent(eventId);
        }

        if (mMultiSDMusicLayout != null) {
            mMultiSDMusicLayout.doCallbackEvent(eventId);
        }
    }

    /**
     * 列表触摸监听
     * <p> 如果这个函数触发，说明当前列表页面是活跃的。
     */
    @Override
    public void onTouchTrigger() {
        // 非可显状态不处理
        if (mViewPager == null || !mInVisibleState) {
            Log.v(TAG, "onTouchTrigger, Non-visual state!");
            return;
        }

        // 当前在播放页面，也不处理；
        int currentPage = mViewPager.getCurrentItem();
        if (currentPage == IMusicState.PAGE_INDEX_PLAY) {
            return; // 播放页面无需处理超时消息。
        }

        // 避免频繁触发动作
        long uptimeMillis = SystemClock.uptimeMillis();
        if (uptimeMillis - mLastTouchListTime > 250) {
            mLastTouchListTime = uptimeMillis;

            // 6S 不操作就进入超时检查
            H0.sendEmptyUniqueMessageDelayed(
                    MsgEx.MSG_LIST_PAGE_OPERATION_TIMEOUT, 8000);
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        LogUtil.e(TAG, "onPause.");

        onVisibleStateChanged(false);
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        LogUtil.e(TAG, "onHiddenChanged: " + hidden);

        onVisibleStateChanged(!hidden);
    }

    @Override
    public void onStop() {
        super.onStop();
    }

    @Override
    public void onSaveInstanceState(@NonNull Bundle outState) {
        Log.e(TAG, ">>> onSaveInstanceState().");

        // [如果 Activity 屏蔽了 onSaveInstanceState(Bundle outState), 就不会调用到这里]
        super.onSaveInstanceState(outState);
    }

    /**
     * 更新当前 Fragment 可视状态
     * <p> 如果进入可视状态，那么需要检查当前是否在列表页面。
     *
     * @param visible 当前可视状态
     */
    private void onVisibleStateChanged(boolean visible) {
        if (!visible) {
            H0.removeMessages(MsgEx.MSG_LIST_PAGE_OPERATION_TIMEOUT);
        } else {
            // 过滤重复动作
            if (mInVisibleState) {
                return;
            }

            if (mViewPager != null) {
                int currentPage = mViewPager.getCurrentItem();
                switch (currentPage) {
                    case IMusicState.PAGE_INDEX_FLASH:
                    case IMusicState.PAGE_INDEX_USB:
                    case IMusicState.PAGE_INDEX_SD:
                    case IMusicState.PAGE_INDEX_FOLDER:
                        H0.sendEmptyUniqueMessageDelayed(
                                MsgEx.MSG_LIST_PAGE_OPERATION_TIMEOUT, 8000);
                        break;
                    default:
                        break;
                }
            }
        }

        mInVisibleState = visible;
    }

    /**
     * 列表界面操作超时
     * <p> 如列表界面长时间未操作，可以返回播放界面。
     */
    private void onMsgListPageOperationTimeout() {
        // 新添加的逻辑功能，暂时只处理 mcc402 这个配置；
        if (!Argument.isThemeGod(ThemeX.ET_GOD_402)) {
            return;
        }

        // 如果未初始化或非显示状态，则不处理；
        if (mViewPager == null || !mInVisibleState) {
            Log.v(TAG, "onMsgListPageOperationTimeout, Non-visual state!");
            return;
        }

        // 当前在播放页面，也不处理；
        int currentPage = mViewPager.getCurrentItem();
        if (currentPage == IMusicState.PAGE_INDEX_PLAY) {
            return; // 播放页面无需处理超时消息。
        }

        // 超时消息是否已消费
        boolean consumed = false;
        switch (currentPage) {
            case IMusicState.PAGE_INDEX_FLASH:
                if (mMultiFlashMusicLayout != null
                        && mMultiFlashMusicLayout.isIdleState()) {
                    showMusicInfo();
                    consumed = true;
                }
                break;
            case IMusicState.PAGE_INDEX_USB:
                if (mMultiUSBMusicLayout != null
                        && mMultiUSBMusicLayout.isIdleState()) {
                    showMusicInfo();
                    consumed = true;
                }
                break;
            case IMusicState.PAGE_INDEX_SD:
                if (mMultiSDMusicLayout != null
                        && mMultiSDMusicLayout.isIdleState()) {
                    showMusicInfo();
                    consumed = true;
                }
                break;
            case IMusicState.PAGE_INDEX_FOLDER:
                if (mFolderListLayout != null
                        && mFolderListLayout.isIdleState()) {
                    showMusicInfo();
                    consumed = true;
                }
                break;
            default:
                break;
        }

        // 未消费，需要继续计时；
        if (!consumed) {
            H0.sendEmptyUniqueMessageDelayed(
                    MsgEx.MSG_LIST_PAGE_OPERATION_TIMEOUT, 8000);
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

    @Override
    public void onDestroy() {
        super.onDestroy();
        LogUtil.e(TAG, "onDestroy.");

        onVisibleStateChanged(false);
    }

    /**
     * 当前页面消息定义
     * <p> 子类的消息定义必须从 {@link  H#MSG_BASE_THRESHOLD} 后开始；
     */
    private interface MsgEx extends H {
        int MSG_IDLE = MSG_BASE_THRESHOLD;

        // 列页面操作超时
        int MSG_LIST_PAGE_OPERATION_TIMEOUT = MSG_IDLE + 1;
    }

    @Override
    protected void onHandleMessage(@NonNull Message msg) {
        switch (msg.what) {
            case MsgEx.MSG_LIST_PAGE_OPERATION_TIMEOUT:
                onMsgListPageOperationTimeout();
                break;
            case MsgEx.MSG_NONE:
            default:
                break;
        }
    }
}
