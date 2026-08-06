package com.hcn.media.video.grid;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Bundle;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.LinearInterpolator;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.viewpager.widget.PagerAdapter;
import androidx.viewpager.widget.ViewPager;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.auto_compat.PlatformUtils;
import com.hcn.media.R3;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_theme.Argument;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media_theme.ThemeX;
import com.hcn.media_base.IMediaEvent;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.IMediaEventListener;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_view.NoScrollViewPager;
import com.hcn.skinx.extend.SkinExAdapterSupport;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Objects;

/**
 * 视频列表界面（mcc201）
 * @author 86158
 */
@SuppressLint("ValidFragment")
public class VideoGridListFragment extends MediaFragment
        implements ViewPager.OnPageChangeListener, OnClickListener {
    private final static String FRAGMENT_NAME = "video-list-mcc201";
    private static final String TAG = VideoGridListFragment.class.getSimpleName();

    private boolean mInitView = false;
    private View mVideoListLayout = null;

    private Animation mRotateAnim = null;

    private RadioButton mBtnStorageFlash = null;
    private RadioButton mBtnStorageSD = null;
    private RadioButton mBtnStorageUSB = null;
    private RadioButton mBtnPlaying = null;

    private LinearLayout mContainer = null;
    private View mSearchLayout = null;
    private View mBtnUpdateList = null;
    private TextView mTipView = null;
    private ImageView mLoadingView = null;
    private NoScrollViewPager mViewPager = null;
    private ViewPagerAdapter mViewPagerAdapter = null;
    private RadioButton mRbIndex1, mRbIndex2, mRbIndex3;
    private TextView mTvTotal;
    private int mItemCountPerPage = 4;

    @SuppressLint("ValidFragment")
    public VideoGridListFragment(IMediaEventListener listener) {
        super(FRAGMENT_NAME);

        mListener = listener;
        if (Argument.isThemeGod(ThemeX.ET_GOD_206)) {
            mItemCountPerPage = 6;
        }
    }

    @Override
    public void initFragment() {
        updateStorageCtrl();
        onChangeStorageEvent();

        if (mViewPager != null) {
            updateIndicator(mViewPager.getCurrentItem());
        }
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_MEDIA_LOADING_START:
                updateStorageCtrl();
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
                if (mAppData.mSelectedDevice != null) {
                    updateTipCtrl();
                    resetData();
                }
                break;

            case IMediaEvent.EVENT_MEDIA_MOUNTED:
                Log.d(TAG, "doCallbackEvent: EVENT_MEDIA_MOUNTED");
                updateStorageCtrl();
                break;

            case IMediaEvent.EVENT_MEDIA_UNMOUNTED:
                Log.d(TAG, "doCallbackEvent: EVENT_MEDIA_UNMOUNTED");
                if (!mAppData.mSelectedDevice.isMounted()) {
                    mAppData.mSelectedDevice = mAppData.mCurrentDevice;
                    onChangeStorageEvent();
                }
                updateStorageCtrl();
                break;

            case IMediaEvent.EVENT_CHANGE_VIDEO_LIST:
            default:
                break;
        }
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fragment_videolist;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container,
            @Nullable Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");

        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);
        assert view != null;

        initView(view);
        initFragment();
        return view;
    }

    private void initView(@NonNull View layout) {
        if (mInitView) {
            LogUtil.d(TAG, "It's already initialized!");
            return;
        }

        mInitView = true;
        mVideoListLayout = layout.findViewById(xId(R.id.llVideoList));

        mBtnUpdateList = layout.findViewById(xId(R.id.btnUpdate));
        if (mBtnUpdateList != null) {
            mBtnUpdateList.setOnClickListener(this);
        }

        mTipView = layout.findViewById(xId(R.id.tv_main_tips));
        mLoadingView = layout.findViewById(xId(R.id.iv_loading));
        mContainer = layout.findViewById(xId(R.id.container));

        mBtnStorageFlash = layout.findViewById(xId(R.id.btnStorageFlash));
        mBtnStorageSD = layout.findViewById(xId(R.id.btnStorageSD1));
        mBtnStorageUSB = layout.findViewById(xId(R.id.btnStorageUSB1));
        mBtnPlaying = layout.findViewById(xId(R.id.btnPlaying));

        if (mBtnStorageFlash != null) {
            mBtnStorageFlash.setOnClickListener(this);
        }

        if (mBtnStorageSD != null) {
            mBtnStorageSD.setOnClickListener(this);
        }

        if (mBtnStorageUSB != null) {
            mBtnStorageUSB.setOnClickListener(this);
        }

        if (mBtnPlaying != null) {
            mBtnPlaying.setOnClickListener(this);
        }

        mSearchLayout = layout.findViewById(xId(R.id.layout_search));
        if (mSearchLayout != null) {
            mSearchLayout.setOnClickListener(this);
        }

        mRbIndex1 = layout.findViewById(xId(R.id.rbIndex1));
        if (mRbIndex1 != null) {
            mRbIndex1.setOnClickListener(this);
        }

        mRbIndex2 = layout.findViewById(xId(R.id.rbIndex2));
        if (mRbIndex2 != null) {
            mRbIndex2.setOnClickListener(this);
        }

        mRbIndex3 = layout.findViewById(xId(R.id.rbIndex3));
        if (mRbIndex3 != null) {
            mRbIndex3.setOnClickListener(this);
        }

        mTvTotal = layout.findViewById(xId(R.id.tvTotal));
        mViewPager = layout.findViewById(xId(R.id.viewpager_center));
        if (mViewPager != null) {
            mViewPagerAdapter = new ViewPagerAdapter(
                    mContext, mAppData.mSelectedDevice.mVideoInfoList);
            mViewPager.setAdapter(mViewPagerAdapter);
            mViewPager.setOnPageChangeListener(this);
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
        // 使用了显示过扫描配置，不需要预留状态栏高度
        if (PlatformUtils.isDisplayOverscanning()) {
            return;
        }

        // 这里资源是默认值，就不用转换了
        int statusBarHeight = MiscUtils.statusBarHeight(mContext, R.dimen.status_bar_height);
        if (mVideoListLayout != null) {
            mVideoListLayout.setPadding(0, statusBarHeight, 0, 0);
        }
    }

    private void initAnimation() {
        mRotateAnim = xAnimation(R.anim.anim_rotate);
        LinearInterpolator lin = new LinearInterpolator();
        mRotateAnim.setInterpolator(lin);
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // TODO: 预留接口
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (getId(v)) {
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
            case R.id.layout_search:
                onSearchEvent();
                break;
            case R.id.btnPlaying:
                onBackEvent();
                break;
            case R.id.btnUpdate:
                onUpdateListEvent();
                break;
            case R.id.rbIndex1:
            case R.id.rbIndex2:
            case R.id.rbIndex3:
                try {
                    int num = Integer.parseInt(((RadioButton) v).getText().toString());
                    mViewPager.setCurrentItem(num - 1);
                } catch (Exception ignored) {
                }
                break;
            default:
                break;
        }
    }

    private void onBackEvent() {
        if (mListener != null) {
            mListener.onMediaEvent(IMediaEvent.EVENT_GOTO_MUSIC_INFO_PAGE, 0, 0);
        }
    }

    private void onChangeStorageEvent() {
        if (mAppData.mSelectedDevice != null) {
            updateTipCtrl();
            resetData();
        }
    }

    private void resetData() {
        if (Objects.isNull(mViewPagerAdapter)) {
            return;
        }

        if (null != mAppData.mSelectedDevice.mVideoInfoList) {
            mViewPagerAdapter.setDataList(mAppData.mSelectedDevice.mVideoInfoList);
            mViewPagerAdapter.notifyDataSetChanged();
        }

        if (null != mViewPager) {
            updateIndicator(mViewPager.getCurrentItem());
        }
    }

    private void updateTipCtrl() {
        if (mAppData.mSelectedDevice.isLoading()) {
            onStartAnimation(true);
            mTipView.setText(getString(R3.string.tip_loading));
            mTipView.setVisibility(View.VISIBLE);
        } else {
            onStartAnimation(false);
            mTipView.setText(getString(R3.string.tip_no_video_file));
            boolean bShowTip = mAppData.mSelectedDevice.mVideoInfoList.isEmpty();
            mTipView.setVisibility(bShowTip ? View.VISIBLE : View.GONE);

            // 避免配置错误，导致空指针异常
            if (mContainer != null) {
                mContainer.setVisibility(bShowTip ? View.GONE : View.VISIBLE);
            }
        }
    }

    public void updateIndicator(int page) {
        int pageCount = 0;
        if (null != mViewPagerAdapter) {
            pageCount = mViewPagerAdapter.getCount();
        }

        mTvTotal.setText(String.valueOf(pageCount));

        if (pageCount == 0) {
            mRbIndex1.setVisibility(View.INVISIBLE);
            mRbIndex2.setVisibility(View.INVISIBLE);
            mRbIndex3.setVisibility(View.INVISIBLE);
            return;
        }

        if (pageCount == 1) {
            mRbIndex1.setVisibility(View.VISIBLE);
            mRbIndex2.setVisibility(View.GONE);
            mRbIndex3.setVisibility(View.GONE);
        } else if (pageCount == 2) {
            mRbIndex1.setVisibility(View.VISIBLE);
            mRbIndex2.setVisibility(View.VISIBLE);
            mRbIndex3.setVisibility(View.GONE);
        } else {
            mRbIndex1.setVisibility(View.VISIBLE);
            mRbIndex2.setVisibility(View.VISIBLE);
            mRbIndex3.setVisibility(View.VISIBLE);
        }

        if (page == 0) {
            mRbIndex1.setChecked(true);
            mRbIndex2.setChecked(false);
            mRbIndex3.setChecked(false);
            mRbIndex1.setText(String.valueOf(page + 1));
            mRbIndex2.setText(String.valueOf(page + 2));
            mRbIndex3.setText(String.valueOf(page + 3));
        } else if ((page == pageCount - 1) && pageCount != 2) {
            mRbIndex1.setChecked(false);
            mRbIndex2.setChecked(false);
            mRbIndex3.setChecked(true);
            mRbIndex1.setText(String.valueOf(page - 1));
            mRbIndex2.setText(String.valueOf(page));
            mRbIndex3.setText(String.valueOf(page + 1));
        } else {
            mRbIndex1.setChecked(false);
            mRbIndex2.setChecked(true);
            mRbIndex3.setChecked(false);
            mRbIndex1.setText(String.valueOf(page));
            mRbIndex2.setText(String.valueOf(page + 1));
            mRbIndex3.setText(String.valueOf(page + 2));
        }
    }

    private void onStartAnimation(boolean bStart) {
        if (bStart) {
            mLoadingView.startAnimation(mRotateAnim);
            mLoadingView.setVisibility(View.VISIBLE);
            mBtnUpdateList.setEnabled(false);
        } else {
            mLoadingView.clearAnimation();
            mLoadingView.setVisibility(View.INVISIBLE);
            mBtnUpdateList.setEnabled(true);
        }
    }

    private void updateStorageCtrl() {
        if (mAppData.mSelectedDevice == mAppData.mSdStorage) {
            mBtnStorageSD.setChecked(true);
        } else if (mAppData.mSelectedDevice == mAppData.mUsbStorage) {
            mBtnStorageUSB.setChecked(true);
        } else {
            mBtnStorageFlash.setChecked(true);
        }
    }

    private void onSearchEvent() {
        if (mListener != null) {
            mListener.onMediaEvent(IMediaEvent.EVENT_GOTO_MUSIC_SEARCH_PAGE, null, null);
        }
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
    public void onPageScrolled(int i, float v, int i1) {
    }

    @Override
    public void onPageSelected(int page) {
        LogUtil.i(TAG, "onPageSelected:index=" + page);
        updateIndicator(page);
    }

    @Override
    public void onPageScrollStateChanged(int state) {
        if (Objects.isNull(mViewPagerAdapter)) {
            return;
        }

        switch (state) {
            case ViewPager.SCROLL_STATE_IDLE:
                // 无动作、初始状态
                mViewPagerAdapter.setScroll(false);
                break;
            case ViewPager.SCROLL_STATE_DRAGGING:
                // 点击、滑屏
                mViewPagerAdapter.setScroll(true);
                break;
            case ViewPager.SCROLL_STATE_SETTLING:
                // 释放
                break;
            default:
                break;
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();

        mInitView = false;
    }

    private class ViewPagerAdapter extends PagerAdapter
            implements SkinExAdapterSupport, OnClickListener {
        private List<MusicInfo> mVideoData = null;

        private int mPageCount = 0;
        private boolean mIsScroll = false;

        public void setScroll(boolean scroll) {
            mIsScroll = scroll;
        }

        /**
         * 用来缓存被 viewpager destroy 掉的 view，以便重复使用;
         */
        private final List<View> cacheViews = new ArrayList<View>();

        public ViewPagerAdapter(Context context, List<MusicInfo> data) {
            setDataList(data);
        }

        public void setDataList(List<MusicInfo> data) {
            if (data == null) {
                data = new ArrayList<>();
            }

            if (data.size() % mItemCountPerPage == 0) {
                mPageCount = data.size() / mItemCountPerPage;
            } else {
                mPageCount = data.size() / mItemCountPerPage + 1;
            }

            mVideoData = data;
        }

        @Override
        public int getCount() {
            return mPageCount;
        }

        @Override
        public int getItemPosition(@NonNull Object object) {
            return POSITION_NONE;
        }

        @Override
        public boolean isViewFromObject(@NonNull View view, @NonNull Object object) {
            return view == object;
        }

        @Override
        public int getLayoutRes(int itemViewType) {
            return R.layout.layout_video_page_item;
        }

        @NonNull
        @Override
        public View instantiateItem(@NonNull ViewGroup container, int position) {
            Log.d(TAG, "instantiateItem: pos = " + position);
            View view;
            ViewHolder viewHolder;

            // 没有缓存的 view 时新建一个用来显示
            if (cacheViews.isEmpty()) {
                view = inflateItemView(-1, container, false);
                viewHolder = new ViewHolder(view);
                view.setTag(viewHolder);
            } else {
                // 有缓存的 view 时取出使用
                view = cacheViews.remove(0);
                viewHolder = (ViewHolder) view.getTag();
            }

            // 设置页面内的各 item
            initItemData(viewHolder, view, position);
            container.addView(view);
            return view;
        }

        private void initItemData(ViewHolder viewHolder, View view, int position) {
            for (int i = 0; i < mItemCountPerPage; i++) {
                // 计算当前页要显示的 item 在 list 中的 position。
                int dataPosition = position * mItemCountPerPage + i;

                if (dataPosition >= mVideoData.size()) {
                    viewHolder.mViewChildHolders[i].mVideoLayout.setVisibility(View.INVISIBLE);
                } else {
                    viewHolder.mViewChildHolders[i].mVideoLayout.setVisibility(View.VISIBLE);
                    viewHolder.mViewChildHolders[i].mVideoLayout.setOnClickListener(this);

                    MusicInfo mMusicInfo = mVideoData.get(dataPosition);
                    int duration = mMusicInfo.mTotalTime;
                    viewHolder.mViewChildHolders[i].mTitle.setText(mMusicInfo.mFileName);

                    String str = String.format(Locale.getDefault(),
                            "%s%d:%d", getString(R3.string.duration_label),
                            duration / 1000 / 60, duration / 1000 % 60);
                    try {
                        viewHolder.mViewChildHolders[i].mDuration.setText(str);
                    } catch (Exception ignored) {
                    }

                    // 不显示格式（客户需求）
                    str = String.format(Locale.getDefault(),
                            "%s%s", getString(R3.string.format_label),
                            MiscUtils.getExtFromFilename(mMusicInfo.mFileName));
                    try {
                        // 删除了？
                    } catch (Exception e) {
                        viewHolder.mViewChildHolders[i].mFormat.setText(str);
                    }

                    try{
                        int defaultImageResId = xId(R.drawable.video_item_bg);
                        BitmapCache.getInstance().loadVideoInfoImage(mMusicInfo.mFilePath,
                                viewHolder.mViewChildHolders[i].mVideoImg, xDrawable("video_gridview_item_n"), !mIsScroll);
                    } catch (Exception ignored) {
                    }
                }
            }
        }

        /**
         * 隐藏各 item
         * @param viewHolder
         */
        private void clearViewContent(ViewHolder viewHolder) {
            try {
                viewHolder.mViewChildHolders[0].mVideoLayout.setVisibility(View.INVISIBLE);
                viewHolder.mViewChildHolders[1].mVideoLayout.setVisibility(View.INVISIBLE);
                viewHolder.mViewChildHolders[2].mVideoLayout.setVisibility(View.INVISIBLE);
                viewHolder.mViewChildHolders[3].mVideoLayout.setVisibility(View.INVISIBLE);
                viewHolder.mViewChildHolders[4].mVideoLayout.setVisibility(View.INVISIBLE);
                viewHolder.mViewChildHolders[5].mVideoLayout.setVisibility(View.INVISIBLE);
            } catch (Exception ignored) {
            }
        }

        @Override
        public void destroyItem(ViewGroup container, int position, @NonNull Object object) {
            View view = (View) object;
            container.removeView(view);

            // 隐藏页面内的item
            clearViewContent((ViewHolder) view.getTag());

            // 添加到缓存
            cacheViews.add(view);
        }

        @SuppressLint("NonConstantResourceId")
        @Override
        public void onClick(View view) {
            int page = mViewPager.getCurrentItem();
            mAppData.mCurrentDevice = mAppData.mSelectedDevice;
            switch (getId(view)) {
                case R.id.video_layout1:
                    mVideoViewModel.playerRelay().accept(
                            t -> t.requestPlayTarget(IPlaylistType.DEVICE_LIST,
                                    mVideoData, page * mItemCountPerPage));
                    break;
                case R.id.video_layout2:
                    mVideoViewModel.playerRelay().accept(
                            t -> t.requestPlayTarget(IPlaylistType.DEVICE_LIST,
                                    mVideoData, page * mItemCountPerPage + 1));
                    break;
                case R.id.video_layout3:
                    mVideoViewModel.playerRelay().accept(
                            t -> t.requestPlayTarget(IPlaylistType.DEVICE_LIST,
                                    mVideoData, page * mItemCountPerPage + 2));
                    break;
                case R.id.video_layout4:
                    mVideoViewModel.playerRelay().accept(
                            t -> t.requestPlayTarget(IPlaylistType.DEVICE_LIST,
                                    mVideoData, page * mItemCountPerPage + 3));
                    break;
                case R.id.video_layout5:
                    mVideoViewModel.playerRelay().accept(
                            t -> t.requestPlayTarget(IPlaylistType.DEVICE_LIST,
                                    mVideoData, page * mItemCountPerPage + 4));
                    break;
                case R.id.video_layout6:
                    mVideoViewModel.playerRelay().accept(
                            t -> t.requestPlayTarget(IPlaylistType.DEVICE_LIST,
                                    mVideoData, page * mItemCountPerPage + 5));
                    break;
                default:
                    break;
            }
        }

        /**
         * viewHolder
         */
        class ViewChildHolder {
            public ViewGroup mVideoLayout;
            public ImageView mVideoImg;
            public TextView mTitle;
            public TextView mDuration;
            public TextView mFormat;

            public ViewChildHolder(ViewGroup layout, ImageView iv, TextView title,
                    TextView duration, TextView format) {
                mVideoLayout = layout;
                mVideoImg = iv;
                mTitle = title;
                mDuration = duration;
                mFormat = format;
            }
        }

        class ViewHolder {
            ViewChildHolder[] mViewChildHolders;

            public ViewHolder(View itemView) {
                mViewChildHolders = new ViewChildHolder[mItemCountPerPage];
                ViewGroup viewGroup = itemView.findViewById(xId(R.id.video_layout1));
                ImageView videoImg = itemView.findViewById(xId(R.id.img_slt1));
                TextView title = itemView.findViewById(xId(R.id.tvTitle1));
                TextView duration = itemView.findViewById(xId(R.id.tvDuration1));
                TextView format = itemView.findViewById(xId(R.id.tvForm1));
                mViewChildHolders[0] = new ViewChildHolder(viewGroup, videoImg, title, duration, format);

                viewGroup = itemView.findViewById(xId(R.id.video_layout2));
                videoImg = itemView.findViewById(xId(R.id.img_slt2));
                title = itemView.findViewById(xId(R.id.tvTitle2));
                duration = itemView.findViewById(xId(R.id.tvDuration2));
                format = itemView.findViewById(xId(R.id.tvForm2));
                mViewChildHolders[1] = new ViewChildHolder(viewGroup, videoImg, title, duration, format);

                viewGroup = itemView.findViewById(xId(R.id.video_layout3));
                videoImg = itemView.findViewById(xId(R.id.img_slt3));
                title = itemView.findViewById(xId(R.id.tvTitle3));
                duration = itemView.findViewById(xId(R.id.tvDuration3));
                format = itemView.findViewById(xId(R.id.tvForm3));
                mViewChildHolders[2] = new ViewChildHolder(viewGroup, videoImg, title, duration, format);

                viewGroup = itemView.findViewById(xId(R.id.video_layout4));
                videoImg = itemView.findViewById(xId(R.id.img_slt4));
                title = itemView.findViewById(xId(R.id.tvTitle4));
                duration = itemView.findViewById(xId(R.id.tvDuration4));
                format = itemView.findViewById(xId(R.id.tvForm4));
                mViewChildHolders[3] = new ViewChildHolder(viewGroup, videoImg, title, duration, format);

                // 兼容 mcc206 的 6 个 item 情况
                try {
                    viewGroup = itemView.findViewById(xId(R.id.video_layout5));
                    videoImg = itemView.findViewById(xId(R.id.img_slt5));
                    title = itemView.findViewById(xId(R.id.tvTitle5));
                    mViewChildHolders[4] = new ViewChildHolder(viewGroup, videoImg, title, duration, format);

                    viewGroup = itemView.findViewById(xId(R.id.video_layout6));
                    videoImg = itemView.findViewById(xId(R.id.img_slt6));
                    title = itemView.findViewById(xId(R.id.tvTitle6));
                    mViewChildHolders[5] = new ViewChildHolder(viewGroup, videoImg, title, duration, format);
                } catch (Exception ignored) {
                }
            }
        }
    }
}
