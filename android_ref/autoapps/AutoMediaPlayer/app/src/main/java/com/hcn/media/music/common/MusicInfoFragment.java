package com.hcn.media.music.common;

import static android.carsource.McuConstant.K_EQ;

import static com.hcn.config.Feature.BIT.REMOTE_CONTROL_FOCUS;

import android.animation.ObjectAnimator;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.carsource.McuManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Color;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.media.audiofx.Visualizer;
import android.media.audiofx.Visualizer.OnDataCaptureListener;
import android.os.Build;
import android.os.Bundle;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.util.DisplayMetrics;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.animation.LinearInterpolator;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.coordinatorlayout.widget.CoordinatorLayout;
import androidx.core.content.ContextCompat;
import androidx.core.view.GravityCompat;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.RecyclerView;
import androidx.viewpager2.adapter.FragmentStateAdapter;
import androidx.viewpager2.adapter.FragmentViewHolder;
import androidx.viewpager2.widget.ViewPager2;

import com.google.android.material.bottomsheet.BottomSheetBehavior;
import com.hcn.AutoMediaPlayer.R;
import com.hcn.common.app.HToastUtils;
import com.hcn.common.misc.LogUtils;
import com.hcn.common.utils.HColorUtils;
import com.hcn.common.widget.HSnackbarUtils;
import com.hcn.config.Feature;
import com.hcn.media.R3;
import com.hcn.media.base.config.MediaFeatureKeys;
import com.hcn.media.extend.base.IExtend;
import com.hcn.media_common.observer.ObserverEx;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_common.utils.ViewUtilsEx;
import com.hcn.auto.AutoStatus;
import com.hcn.media_base.HMediaConfig;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.fragment.IMusicPage;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_base.fragment.PageEvent;
import com.hcn.media_data.FavoriteManager;
import com.hcn.media.music.common.simple.ISimpleList;
import com.hcn.media.music.common.simple.SimpleListFragment;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_theme.Argument;
import com.hcn.media_theme.ThemeX;
import com.hcn.media_view.CircleImageView;
import com.hcn.media_view.compat.DrawerLayout;
import com.hcn.media_view.widget.DrawerLayoutEx;
import com.hcn.mediaservice.data.MediaTimeInfo;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media.local.utils.HFuncUtils;
import com.hcn.common.Utility;
import com.hcn.media_view.lyrics.LyricsManager;
import com.hcn.media_view.lyrics.LyricsRow;
import com.hcn.media_view.lyrics.LyricsView;
import com.hcn.media_view.lyrics.LyricsView.OnSeekToListener;
import com.hcn.media_view.PlayFlashView;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_view.HTextView;
import com.hcn.plugin.ApkClassLoaderEx;
import com.hcn.skinx.SkinX;
import com.hcn.media_theme.ThemeEx;
import com.orhanobut.logger.Logger;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Objects;

import io.reactivex.rxjava3.android.schedulers.AndroidSchedulers;
import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.core.ObservableOnSubscribe;
import io.reactivex.rxjava3.schedulers.Schedulers;

/**
 * 音乐播放 Fragment
 * <p> MT8163 平台默认主题；
 *
 * @author 86158
 */
@SuppressWarnings("deprecation")
public class MusicInfoFragment extends MediaFragment
        implements OnClickListener, OnDataCaptureListener,
        OnSeekToListener, OnTouchListener {

    private final static String FRAGMENT_NAME = "music-info";
    private final static String TAG = MusicInfoFragment.class.getSimpleName();

    /**
     * 视图初始状态
     * <p> 标记视图初始化状态，避免重复操作;
     */
    private boolean mInitView = false;

    /**
     * 菜单按钮对象
     * <p> 上下曲、播放暂停、播放模式...
     */
    private View mBtnPlay = null;
    private View mBtnRepeatMode = null;
    private View mBtnEQ = null;

    /**
     * ID3 相关对象
     * <p> 专辑图片、歌曲名、艺术家、专辑名称...
     */
    private ImageView mMusicImage = null;
    private HTextView mTvTitle = null;
    private HTextView mTvArtist = null;
    private HTextView mTvAlbum = null;
    private TextView mTvTotalValue = null;

    /**
     * 播放进度信息
     * <p> 进度条、播放时间、歌曲总时长...
     */
    private SeekBar mSeekbarProgress = null;
    private TextView mTvCurrentTime = null;
    private TextView mTvTotalTime = null;
    private TextView m_tvNewTime = null;
    private TextView m_tvDelayTime = null;
    private View mSeekTimeLayout = null;
    private static final int SEEKBAR_MAX_VALUE = 1000;

    /**
     * Seekbar 拖动中
     * <p> 拖动中，不处理外部进度更新事件；
     */
    private boolean mSeekbarTracking = false;

    /**
     * 频谱、歌词相关对象
     * <p> 频谱效果视图、歌词视图、歌词解析、无歌词提示信息；
     */
    private View mBtnChangeLyric;
    private ViewGroup mLrcLayout;
    private PlayFlashView mFrequencyView = null;
    private LyricsView mLyricsView = null;
    private LyricsManager mLyricsManager = null;
    private final List<LyricsRow> mLyricsList = new ArrayList<>();
    private TextView mTvNoLyrics = null;
    private boolean mFirstChanged = true;

    /**
     * 播放动画对象
     * <p> 专辑封面旋转动画，比较迟 CPU 资源，正常情况下不打开；
     */
    private ObjectAnimator mRotateAnim = null;
    private ImageView mIvPlayingAnim = null;

    /** 拾音器（用来做唱片机的动画元素） */
    private ImageView mIvPlayingPickup = null;

    /**
     * 背景视图对象
     * <p> 当前页面背景与专辑高斯模糊效果视图，叠加效果；
     */
    private View mBgView = null;
    private View mBgMaskView = null;

    /**
     * 抽屉布局视图
     * <p> 特定的客户需求，用来实现侧滑菜单效果；
     */
    private DrawerLayoutEx mDrawerLayoutEx = null;
    private View mDrawerHandleView = null;
    private DrawerListenerImpl mDrawerListener = null;

    /**
     * 专辑封面布局信息与歌词布局信息
     * <pre>
     *     当且仅当专辑封面布局和歌词信息布局叠加设计的时候使用；
     *     为支持新扩展皮肤包中的需求，参考设计如下：
     *     <FrameLayout>
     *         <FrameLayout android:id="@+id/music_album_layout"></FrameLayout>
     *         <FrameLayout android:id="@+id/music_lyrics_layout"></FrameLayout>
     *     </FrameLayout>
     * </pre>
     */
    private View mMusicAlbumLayout = null;
    private View mMusicLyricsLayout = null;

    /**
     * 控制按钮（统一命名）
     * <pre>
     *    ImageView/ivFavorite：收藏歌曲；
     *    ImageView/ivPlayMode：播放模式；
     * </pre>
     */
    private View mIvFavorite = null;
    private View mIvPlayMode = null;
    private View mDrawerHandlePort;

    /**
     * 必须是无参构造函数
     * <p> 带参构造
     */
    public MusicInfoFragment() {
        super(FRAGMENT_NAME);

        // 支持检查扩展皮肤包（逻辑扩展）
        String pageExtendResConfigName = "music_info_page_extend";
        if (xBoolean(pageExtendResConfigName)) {
            ApkClassLoaderEx classLoader = xClassLoader();
            if (!Objects.isNull(classLoader)) {
                String pageExtendClassName =
                        IExtend.MUSIC_PACKAGE_NAME + ".MusicInfoPageExtend";
                mPageExtend = classLoader.newPageExtendInterface(pageExtendClassName, this);
            }

            LogUtils.iTag(TAG, mPageExtend != null?
                    "Has MusicInfoPageExtend class.": "No MusicInfoPageExtend class.");
        }
    }

    @Override
    public void onAttach(@NonNull Activity context) {
        super.onAttach(context);
        Log.d(TAG, "onAttach");
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");

        // 歌词管理器对象（唯一实例）
        mLyricsManager = LyricsManager.instance();
    }

    /**
     * 当 Fragment 结合 ViewPager 使用的时候这个方法会调用;
     * <p> 这个方法是在 onCreateView 之前使用, 不要使用控件;
     * @param isVisibleToUser
     */
    @Override
    public void setUserVisibleHint(boolean isVisibleToUser) {
        super.setUserVisibleHint(isVisibleToUser);
        Log.d(TAG, "setUserVisibleHint: " + isVisibleToUser);
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fragment_musicinfo;
    }

    @SuppressLint("InflateParams")
    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater,
                             @Nullable ViewGroup container, Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");

        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);

        assert view != null;
        initView(view);

        showLyricsLayout();
        initAnimation();
        return view;
    }

    private void initView(View layout) {
        if (mInitView) {
            return;
        }

        // 专辑封面背景
        mBgView = layout.findViewById(xId(R.id.bg));
        mBgMaskView = layout.findViewById(xId(R.id.bg_mask));

        // 播放显示相关信息
        mIvPlayingAnim = layout.findViewById(xId(R.id.ivPlayingAnim));
        mMusicImage = layout.findViewById(xId(R.id.ivMusicImage));
        mIvPlayingPickup = layout.findViewById(xId(R.id.ivPlayingPickup));
        mTvTitle = layout.findViewById(xId(R.id.tvTitle));
        mTvArtist = layout.findViewById(xId(R.id.tvArtist));
        mTvAlbum = layout.findViewById(xId(R.id.tvAlbum));
        mTvTotalValue = layout.findViewById(xId(R.id.tvTotalValue));
        mDrawerHandlePort = layout.findViewById(xId(R.id.ib_drawer_handle_port));
        mSeekTimeLayout = layout.findViewById(xId(R.id.layoutSeekTime));
        m_tvNewTime = layout.findViewById(xId(R.id.tvNewTime));
        m_tvDelayTime = layout.findViewById(xId(R.id.tvDelayTime));

        // 设置开启触摸过滤
        if (mTvTitle != null) {
            mTvTitle.setFilterTouchesWhenObscured(true);
            mTvArtist.setFilterTouchesWhenObscured(true);
            mTvAlbum.setFilterTouchesWhenObscured(true);
        }

        // 歌词和频谱相关控件
        mTvNoLyrics = layout.findViewById(xId(R.id.tvNoLyrics));
        mLrcLayout = layout.findViewById(xId(R.id.lin_lyric));
        mFrequencyView = layout.findViewById(xId(R.id.FrequencyView));

        mLyricsView = layout.findViewById(xId(R.id.lyrics_view));
        if (mLyricsView != null) {
            mLyricsView.setOnSeekToListener(this);
            mLyricsView.SetPainTypeface(Typeface.SERIF);
            mLyricsView.SetCurPaintColor(SkinX.getColor(R.color.lyric_view_cur_paint_color));
            mLyricsView.SetNotCurPaintColor(SkinX.getColor(R.color.lyric_view_not_cur_paint_color));
            if (SkinX.getBoolean("lyrics_view_align_left", false)) {
                mLyricsView.setAlignLeft(true);
            }
        }

        // 扩展皮肤元素初始化
        initViewSkinEx(layout);

        // 点击滑动事件初始化
        initClickView(layout);
        initSeekbarProgress(layout);

        mInitView = true;

        // 底部上拉音乐列表
        if (mDrawerHandlePort != null) {
            BottomSheetBehavior<View> bottomSheetBehavior;
            View bottomSheet = layout.findViewById(xId(R.id.bottom_sheet));
            View cardTopSpace = layout.findViewById(xId(R.id.layout_music_card_top_space));
            View mCardBottomSpace = layout.findViewById(xId(R.id.layout_music_card_bottom_space));
            CoordinatorLayout.LayoutParams params =
                    (CoordinatorLayout.LayoutParams) bottomSheet.getLayoutParams();
            BottomSheetBehavior<View> behavior = new BottomSheetBehavior<>();
            behavior.setHideable(false);
            behavior.setPeekHeight(48);
            params.setBehavior(behavior);
            bottomSheet.setLayoutParams(params);
            bottomSheetBehavior = BottomSheetBehavior.from(bottomSheet);
            mDrawerHandlePort.setOnClickListener(v -> {
                if (bottomSheetBehavior.getState() == BottomSheetBehavior.STATE_EXPANDED) {
                    bottomSheetBehavior.setState(BottomSheetBehavior.STATE_COLLAPSED);
                } else {
                    bottomSheetBehavior.setState(BottomSheetBehavior.STATE_EXPANDED);
                }
            });
            layout.postDelayed(() -> {
                bottomSheetBehavior.setState(BottomSheetBehavior.STATE_COLLAPSED);
            }, 500);

            if (cardTopSpace != null && mCardBottomSpace != null) {
                LinearLayout.LayoutParams topParams = (LinearLayout.LayoutParams) cardTopSpace.getLayoutParams();
                LinearLayout.LayoutParams bottomParams = (LinearLayout.LayoutParams) mCardBottomSpace.getLayoutParams();
                final float topWeight = topParams.weight;
                final float bottomWeight = bottomParams.weight;
                bottomSheetBehavior.addBottomSheetCallback(new BottomSheetBehavior.BottomSheetCallback() {
                    @Override
                    public void onStateChanged(@NonNull View bottomSheet, int newState) {
                    }

                    @Override
                    public void onSlide(@NonNull View bottomSheet, float slideOffset) {
                        topParams.weight = topWeight * (1 - slideOffset);
                        bottomParams.weight = bottomWeight * (1 + slideOffset);
                        cardTopSpace.setLayoutParams(topParams);
                        mCardBottomSpace.setLayoutParams(bottomParams);
                    }
                });
            }
        }
    }

    /**
     * 扩展兼容视图初始化
     * <p> 多主题支持，特定需求有特定的控件；
     *
     * @param fragmentView 页面视图
     */
    private void initViewSkinEx(@NonNull View fragmentView) {
        if (mInitView) {
            return;
        }

        // 例如：uis8581 平台的默认主题设计
        if (ThemeEx.useMusicDrawerLayoutStyle()) {
            initDrawerLayoutSKinDesign(fragmentView);
        }

        // 扩展的按钮图标/收藏歌曲
        mIvFavorite = findViewByName("ivFavorite");
        if (mIvFavorite != null) {
            // 检查收藏列表初始化状态
            FavoriteManager fm = FavoriteManager.getInstance();
            boolean musicFavoriteInited = fm.isInitCompleted(FavoriteManager.Type.MUSIC);
            mIvFavorite.setEnabled(musicFavoriteInited);

            // 监听收藏图标的点击事件
            mIvFavorite.setOnClickListener(v -> {
                // 是否允许执行该次点击事件
                int delayMillis = 200;
                if (!allowExecuteClickEvent(delayMillis)) {
                    return;
                }

                onClickFavoriteEvent();
            });

            // 如果是在分屏状态
            if (requireActivity().isInMultiWindowMode()) {
                mIvFavorite.setVisibility(View.INVISIBLE);
            }
        }

        // 扩展的按钮图标/播放模式
        mIvPlayMode = findViewByName("ivPlayMode");
        if (mIvPlayMode != null) {
            mIvPlayMode.setOnClickListener(v -> onRepeatModeEvent());
        }
    }

    /**
     * 抽屉布局设计风格主题视图初始化
     * <pre>
     *     多主题支持，特定需求有特定的控件；
     *     参考皮肤：blue01/mcc400-mnc100
     * </pre>
     *
     * @param fragmentView 页面视图
     */
    private void initDrawerLayoutSKinDesign(@NonNull View fragmentView) {
        if (mInitView) {
            return;
        }

        mDrawerLayoutEx = (DrawerLayoutEx) findViewByName("music_drawer_layout");
        if (Objects.isNull(mDrawerLayoutEx)) {
            return;
        }

        // 调整触摸边界感应大小
        mDrawerLayoutEx.setLeftDragEdgeSize(56);
        mDrawerLayoutEx.setRightDragEdgeSize(56);

        // 如果有 DrawerLayout 部分设计
        mDrawerLayoutEx.setScrimColor(Color.TRANSPARENT);
        mDrawerLayoutEx.setStatusBarBackgroundColor(Color.TRANSPARENT);

        // 监听抽屉打开关闭状态
        mDrawerListener = new DrawerListenerImpl(mDrawerLayoutEx);
        mDrawerLayoutEx.addDrawerListener(mDrawerListener);

        // 抽屉的把手视图
        mDrawerHandleView = findViewByName("iv_drawer_handle");
        if (mDrawerHandleView != null) {
            mDrawerHandleView.setOnClickListener(v -> {
                // 如果抽屉是打开的，则关闭
                if (mDrawerLayoutEx.isDrawerOpen(GravityCompat.END)) {
                    mDrawerLayoutEx.closeDrawers();
                } else {
                    // 如果抽屉没有打开，则打开右侧的抽屉（默认都是右侧设计）
                    mDrawerLayoutEx.openDrawer(GravityCompat.END);
                }
            });

            // 如果是分屏显示状态
            if (requireActivity().isInMultiWindowMode()) {
                mDrawerHandleView.setVisibility(View.GONE);
                mDrawerLayoutEx.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_CLOSED);
            }
        }

        // 专辑布局视图/歌词布局视图
        mMusicAlbumLayout = findViewByName("music_album_layout");
        mMusicLyricsLayout = findViewByName("music_lyrics_layout");
        if (mMusicAlbumLayout != null && mMusicLyricsLayout != null) {
            // 专辑封面遮罩设计/遮罩部分点击切换歌词，遮罩右边点击触发动画
            View albumShadeView = findViewByName("music_album_shade");
            if (albumShadeView != null) {
                albumShadeView.setOnClickListener(v -> {
                    mMusicAlbumLayout.setVisibility(View.INVISIBLE);
                    mMusicLyricsLayout.setVisibility(View.VISIBLE);

                    // 停止专辑封面动画
                    if (mRotateAnim != null) {
                        mRotateAnim.pause();
                    }
                });
            }

            // 专辑封面动画触发监听
            mMusicAlbumLayout.setOnClickListener(v -> {
                if (Objects.isNull(mRotateAnim)) {
                    mMusicAlbumLayout.setVisibility(View.INVISIBLE);
                    mMusicLyricsLayout.setVisibility(View.VISIBLE);
                    return;
                }

                // 播放中才可以操作动画
                if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                    if (mRotateAnim.isPaused()) {
                        mRotateAnim.resume();
                    } else {
                        mRotateAnim.pause();
                    }
                }
            });

            // 歌词遮罩设计/遮罩部分点击切换专辑封面，遮罩右边可滑动歌词；
            View lyricsShadeView = findViewByName("music_lyrics_shade");
            if (lyricsShadeView != null) {
                lyricsShadeView.setOnClickListener(v -> {
                    mMusicAlbumLayout.setVisibility(View.VISIBLE);
                    mMusicLyricsLayout.setVisibility(View.INVISIBLE);

                    // 检查并恢复专辑封面动画；
                    if (mRotateAnim != null) {
                        // 非播放状态，不处理；
                        if (!mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                            return;
                        }

                        mRotateAnim.resume();
                    }
                });
            }
        }
    }

    @Override
    public void onUpdateUiModeView(boolean isNightMode) {
        super.onUpdateUiModeView(isNightMode);

        // 刷新资源
        updatePlayerResource();
    }

    /**
     * 当前页面消息定义
     * <p> 子类的消息定义必须从 {@link  H#MSG_BASE_THRESHOLD} 后开始；
     */
    private interface MsgEx extends H {
        int MSG_IDLE = MSG_BASE_THRESHOLD;

        /** 更新抽屉布局 **/
        int MSG_UPDATE_DRAWER_LAYOUT_CONTENT = MSG_IDLE + 1;
    }

    @Override
    protected void onHandleMessage(@NonNull Message msg) {
        super.onHandleMessage(msg);

        switch (msg.what) {
            case MsgEx.MSG_UPDATE_DRAWER_LAYOUT_CONTENT:
                onMsgUpdateDrawerLayoutContent(msg.obj);
                break;
            case MsgEx.MSG_IDLE:
            default:
                break;
        }
    }

    /**
     * 更新抽屉布局内容
     * <p> 滑动后需要调整布局内容
     *
     * @param obj {@link Float} 侧滑页面滑动比例
     */
    private void onMsgUpdateDrawerLayoutContent(Object obj) {
        if (Objects.isNull(mDrawerListener)
                || Objects.isNull(mDrawerListener.mSideslipPage)) {
            return;
        }

        assert obj instanceof Float;
        float slideOffset = (float) obj;
        mDrawerListener.updateDrawerSlideOffset(
                mDrawerListener.mSideslipPage, slideOffset);
    }

    /**
     * 抽屉布局状态监听
     * <pre>
     *    提供抽屉滑动、打开、关闭、状态改变函数回调;
     *    抽屉状态：{@link DrawerLayout#STATE_IDLE|STATE_DRAGGING|STATE_SETTLING}
     * </pre>
     */
    private final class DrawerListenerImpl implements DrawerLayout.DrawerListener {
        /**
         * 被监听的抽屉布局
         * <p> 必须是一个弱引用，避免相互强引用；
         */
        private final Reference<DrawerLayoutEx> mDrawerLayoutRef;

        /**
         * 抽屉布局的内容页
         * <p> 正常控件内容页是在侧滑页的下方，我们这里在滑动的过程中动态调整内容页的宽度；
         */
        private final View mContentPage;

        /**
         * 抽屉布局的侧滑页
         * <p> 侧滑页配置 android:layout_gravity="end" 表示抽屉在右边，“start” 表示抽屉在左边；
         */
        private final View mSideslipPage;

        /**
         * Simple List 抬头
         * <pre>
         *    1、LeftTab 对应的是播放列表；
         *    2、RightTab 对应的是收藏夹列表；
         * </pre>
         */
        private RadioButton mSimpleListLeftTab;
        private RadioButton mSimpleListRightTab;

        /**
         * SimpleList 视图
         * <pre>
         *    列表实现部分采用 ViewPager2 实现；
         *    每个 Tab-List 也是一个 Fragment 碎片;
         * </pre>
         */
        private ViewPager2 mSimpleListViewPager2;
        private FragmentStateAdapterEx mFragmentStateAdapter;

        /**
         * 内容页面卡片视图以及其原始宽度
         * <pre>
         *    用来动态调整设置内容页面的宽度时使用；
         *    公式：抽屉布局内容页卡片的宽度 - 抽屉页已经滑出来的宽度；
         * </pre>
         */
        private View mContentPageCard;
        private int mContentPageCardWidth;

        /**
         * 进度信息左右填充视图与填充空间宽度
         * <pre>
         *     用来动态调整设置内容页面的宽度时使用；
         *     公式：原始全屏填充宽度 * (1 - 抽屉划出比率 * K)；
         * </pre>
         */
        private View mProgressLeftSpace;
        private View mProgressRightSpace;
        private int mProgressLRSpaceWidth;

        /**
         * ID3 信息左右填充视图与填充空间宽度
         * <pre>
         *     用来动态调整设置内容页面的宽度时使用；
         *     公式：原始全屏填充宽度 * (1 - 抽屉划出比率 * K)；
         * </pre>
         */
        private View mId3LeftSpace;
        private View mId3RightSpace;
        private int mId3LRSpaceWidth;

        /**
         * ID3 信息卡片布局与占位宽度
         * <pre>
         *     用来动态调整设置内容页面的宽度时使用；
         *     布局中主要包含: 歌曲名、歌手、专辑名；
         * </pre>
         */
        private View mId3InfoCardLayout;
        private int mId3InfoCardLayoutWidth;

        /**
         * ID3 信息卡片布局元素内容占位符与占位宽度
         * <pre>
         *    用来动态调整设置内容页面的宽度时使用；
         *    公式：抽屉布局内容页卡片的宽度 - 抽屉页已经滑出来的宽度；
         * </pre>
         */
        private View mId3TitleLeftSpace;
        private View mId3AlbumLeftSpace;
        private View mId3ArtistLeftSpace;

        /**
         * ID3 信息文本控件
         * <pre>
         *     TextView：歌曲名称；
         *     TextView：专辑名称；
         *     TextView：艺术家名称；
         * </pre>
         */
        private View mId3TitleName;
        private View mId3AlbumName;
        private View mId3ArtistName;

        /**
         * Constructor
         * <p> 在此初始化所有与当前目标抽屉布局相关的内容对象；
         *
         * @param drawerLayout 目标抽屉对象
         */
        @SuppressLint("ClickableViewAccessibility")
        public DrawerListenerImpl(@NonNull DrawerLayoutEx drawerLayout) {
            // 抽屉的内容页视图
            mContentPage = findViewByName("drawer_content_page");
            if (mContentPage != null) {
                initDrawerContentPage();
            }

            // 抽屉的侧滑页视图
            mSideslipPage = findViewByName("drawer_sideslip_page");
            if (mSideslipPage != null) {
                initDrawerSideslipPage();

                // 如果是分屏显示状态
                if (requireActivity().isInMultiWindowMode()) {
                    mSideslipPage.setVisibility(View.GONE);
                }
            }

            // 监听抽屉布局的构建状态
            mDrawerLayoutRef = new WeakReference<>(drawerLayout);
            drawerLayout.setBuildStateListener(new DrawerBuildStateListener());
            drawerLayout.setOnTouchListener((v, event) -> {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        DrawerLayoutEx drawerView = mDrawerLayoutRef.get();
                        if (drawerView != null
                                && drawerView.isDrawerOpen(GravityCompat.END)) {
                            drawerView.closeDrawers();
                        }
                        break;
                    case MotionEvent.ACTION_MOVE:
                    case MotionEvent.ACTION_UP:
                    default:
                        break;
                }

                return false;
            });
        }

        /**
         * 监听 DrawerLayoutEx 构建过程
         * <pre>
         *    用来监听当前抽屉布局的测量、布局、绘制状态；
         *    我们可以通过捕获不同的状态点，获取需要的数据信息；
         * </pre>
         */
        private final class DrawerBuildStateListener implements DrawerLayoutEx.IStateListener {
            /**
             * 首次绘制标记
             * <p> 用来标记视图是否已经执行过一次绘制函数；
             */
            private boolean mFirstDrawed;

            @Override
            public void onMeasured() {
                // TODO 先测量大小
            }

            @Override
            public void onLayouted() {
                // TODO 再计算布局
                if (Objects.isNull(mContentPageCard)) {
                    return;
                }

                // 读取部分元素的测量初始大小（缩放前置条件）
                if (mContentPageCardWidth == 0 || !mFirstDrawed) {
                    // 动态宽度需要 Layout 完成后才能获取到；
                    mContentPageCardWidth = mContentPageCard.getMeasuredWidth();

                    // ID3 左右填充宽度设计的时候是一样的
                    if (mId3LeftSpace != null) {
                        mId3LRSpaceWidth = mId3LeftSpace.getMeasuredWidth();
                    }

                    // ID3 信息卡片布局的占位宽度
                    if (mId3InfoCardLayout != null) {
                        mId3InfoCardLayoutWidth = mId3InfoCardLayout.getMeasuredWidth();
                    }

                    // 进度左右填充宽度设计的时候是一样的
                    if (mProgressLeftSpace != null) {
                        mProgressLRSpaceWidth = mProgressLeftSpace.getMeasuredWidth();
                    }
                }
            }

            @Override
            public void onDrawed() {
                // TODO 最后绘制元素
                mFirstDrawed = true;
            }
        }

        /**
         * 初始化抽屉内容页视图
         * <p> 我们这里指的就是抽屉外的视图（左侧视图）；
         */
        private void initDrawerContentPage() {
            mContentPageCard = findViewByName("music_info_card");

            // 初始化信息卡片内容元素
            if (mContentPageCard != null) {
                // ID3 信息左右填充
                mId3LeftSpace = findViewByName("id3_left_space");
                mId3RightSpace = findViewByName("id3_right_space");

                // ID3 信息卡片布局
                mId3InfoCardLayout = findViewByName("music_id3_card_layout");
                mId3TitleName = mTvTitle != null? mTvTitle: findViewByName("tvTitle");
                mId3AlbumName = mTvAlbum != null? mTvAlbum: findViewByName("tvAlbum");
                mId3ArtistName = mTvArtist != null? mTvArtist: findViewByName("tvArtist");

                // 进度布局左右填充
                mProgressLeftSpace = findViewByName("progress_left_space");
                mProgressRightSpace = findViewByName("progress_right_space");

                // ID3 卡片内容元素布局左填充
                mId3TitleLeftSpace = findViewByName("id3_title_left_space");
                mId3AlbumLeftSpace = findViewByName("id3_album_left_space");
                mId3ArtistLeftSpace = findViewByName("id3_artist_left_space");
            }
        }

        /**
         * 初始化抽屉滑动页视图
         * <p> 我们这里指的就是抽屉内的视图（右侧视图）；
         */
        private void initDrawerSideslipPage() {
            if (Objects.isNull(mSideslipPage)) {
                return;
            }

            // 设置侧滑页背景
            mSideslipPage.setBackgroundColor(Color.TRANSPARENT);

            // 当前播放列表 (不是每个主题都支持 Tab Page)
            mSimpleListLeftTab = (RadioButton) findViewByName("rbListLeftTab");
            if (mSimpleListLeftTab != null) {
                mSimpleListLeftTab.setOnClickListener(v -> {
                    // TODO: 点击播放列表，只显示当前播放列表信息；
                    mSimpleListViewPager2.setCurrentItem(ISimpleList.PAGE_PLAYLIST);
                });
            }

            // 当前收藏列表 (不是每个主题都支持 Tab Page)
            mSimpleListRightTab = (RadioButton) findViewByName("rbListRightTab");
            if (mSimpleListRightTab != null) {
                mSimpleListRightTab.setOnClickListener(v -> {
                    // TODO: 点击收藏列表，只显示当前收藏列表信息；
                    mSimpleListViewPager2.setCurrentItem(ISimpleList.PAGE_FAVORITE);
                });
            }

            // 默认显示播放列表
            updateSimpleListTabLayout(ISimpleList.PAGE_PLAYLIST);

            // Simple List 视图
            mSimpleListViewPager2 = (ViewPager2) findViewByName("music_list_pager");
            mSimpleListViewPager2.setUserInputEnabled(true);
            if (mSimpleListViewPager2 != null) {
                mSimpleListViewPager2.registerOnPageChangeCallback(new ViewPager2.OnPageChangeCallback() {
                    // 是滑动到了播放列表页
                    boolean isSlide2FirstPage = true;

                    @Override
                    public void onPageScrolled(int position,
                                               float positionOffset,
                                               int positionOffsetPixels) {
                        super.onPageScrolled(
                                position, positionOffset, positionOffsetPixels);

                        // 如果是已经滑动到左侧（第一页显示到位）
                        if (positionOffsetPixels == 0) {
                            switch (position) {
                                case ISimpleList.PAGE_PLAYLIST:
                                    updateSimpleListTabLayout(ISimpleList.PAGE_PLAYLIST);
                                    break;
                                case ISimpleList.PAGE_FAVORITE:
                                    updateSimpleListTabLayout(ISimpleList.PAGE_FAVORITE);
                                    break;
                                default:
                                    break;
                            }

                            // 播放列表页面滑动到位
                            if (position == ISimpleList.PAGE_PLAYLIST) {
                                if (isSlide2FirstPage) {
                                    // 如果已经滑动到位，再次操作直接关闭抽屉
                                    // 如果只有一个 Page 不要关闭抽屉（容易误操作）
                                    DrawerLayoutEx drawerView = mDrawerLayoutRef.get();
                                    int pageCount = mFragmentStateAdapter.getItemCount();
                                    if (drawerView != null
                                            && drawerView.isDrawerOpen(GravityCompat.END)
                                            && pageCount > 1) {
                                        drawerView.closeDrawers();
                                    }
                                }

                                isSlide2FirstPage = true;
                            } else {
                                isSlide2FirstPage = false;
                            }
                        } else {
                            isSlide2FirstPage = false;
                        }
                    }

                    @Override
                    public void onPageSelected(int position) {
                        super.onPageSelected(position);
                        isSlide2FirstPage = false;
                    }

                    @Override
                    public void onPageScrollStateChanged(int state) {
                        super.onPageScrollStateChanged(state);
                    }
                });
            }

            // 不支持 ListViewPager2 adapter 延迟初始化
            if (!SkinX.getBoolean(MediaFeatureKeys.SUPPORT_DELAY_DRAWER_LAYOUT)) {
                initViewPager2Adapter("don't support delay init. ");
                return;
            }

            // 延迟 500ms 加载，因为构建 ListViewPager 里面有 RecyclerView 会大量膨胀 View 初始化可见耗时
            if (mSimpleListViewPager2 != null) {
                mSimpleListViewPager2.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        initViewPager2Adapter("delay 500 ms init. ");
                    }
                }, 500);
            }
        }

        @Override
        public void onDrawerSlide(@NonNull View drawerView, float slideOffset, String reason) {
            if (Objects.isNull(mContentPageCard)) {
                return;
            }

            // 避免 requestLayout 被嵌套使用
            String onLayout = "onLayout";
            if (onLayout.equals(reason)) {
                H0.sendMessage(Message.obtain(
                        H0, MsgEx.MSG_UPDATE_DRAWER_LAYOUT_CONTENT, slideOffset));
                LogUtil.v(TAG, "onDrawerSlide: slideOffset = " + slideOffset);
            } else {
                // 避免闪烁和拖影效果
                H0.removeMessages(MsgEx.MSG_UPDATE_DRAWER_LAYOUT_CONTENT);
                updateDrawerSlideOffset(drawerView, slideOffset);
            }
        }

        /**
         * 根据侧滑页面打开的比例压缩移动内容页面的布局
         * <p> 由于内容页面布局需要动态伸缩，所以侧滑页变动后`内容页面也需要跟随调整；
         *
         * @param drawerView 侧滑页面视图
         * @param slideOffset 侧滑页面拉出来的比例
         */
        private void updateDrawerSlideOffset(@NonNull View drawerView, float slideOffset) {
            // 压缩卡片的宽度（自适应布局）
            int menuWidth = drawerView.getMeasuredWidth();
            float moveX = slideOffset * menuWidth;
            LinearLayout.LayoutParams layoutParams
                    = (LinearLayout.LayoutParams) mContentPageCard.getLayoutParams();
            layoutParams.weight = 0;
            layoutParams.width = mContentPageCardWidth - (int) moveX;
            mContentPageCard.setLayoutParams(layoutParams);

            // 进度信息布局随压缩变化动态调整
            if (mProgressLeftSpace != null && mProgressRightSpace != null) {
                ViewGroup.LayoutParams leftLayoutParams = mProgressLeftSpace.getLayoutParams();
                ViewGroup.LayoutParams rightLayoutParams = mProgressRightSpace.getLayoutParams();
                leftLayoutParams.width = (int) (mProgressLRSpaceWidth * (1 - slideOffset));
                rightLayoutParams.width = leftLayoutParams.width;
                mProgressLeftSpace.setLayoutParams(leftLayoutParams);
                mProgressRightSpace.setLayoutParams(rightLayoutParams);
            }

            // ID3 信息布局随压缩变化动态调整
            if (mId3LeftSpace != null && mId3RightSpace != null) {
                ViewGroup.LayoutParams leftLayoutParams = mId3LeftSpace.getLayoutParams();
                ViewGroup.LayoutParams rightLayoutParams = mId3RightSpace.getLayoutParams();
                leftLayoutParams.width = (int) (mId3LRSpaceWidth * (1 - slideOffset));
                rightLayoutParams.width = (int) (mId3LRSpaceWidth * (1 - slideOffset / 2.0f));
                mId3LeftSpace.setLayoutParams(leftLayoutParams);
                mId3RightSpace.setLayoutParams(rightLayoutParams);
            }

            // ID3 信息卡片布局，限定了必须是线性布局
            if (mId3InfoCardLayout instanceof LinearLayout) {
                // ID3 信息卡片布局的占位宽度
                mId3InfoCardLayoutWidth = mId3InfoCardLayout.getMeasuredWidth();
                int id3TitleWidth = mId3TitleName.getMeasuredWidth();
                int id3AlbumWidth = mId3AlbumName.getMeasuredWidth();
                int id3ArtistWidth = mId3ArtistName.getMeasuredWidth();

                // ID3 歌曲名称控件压缩变化动态调整
                if (mId3TitleLeftSpace != null) {
                    // ID3 Title 占位符当前平均宽度
                    int id3TitleSpaceOriginalAvgWidth = (mId3InfoCardLayoutWidth - id3TitleWidth) / 2;

                    // 动态移动 ID3 Title 的位置
                    LinearLayout.LayoutParams leftLayoutParams
                            = (LinearLayout.LayoutParams) mId3TitleLeftSpace.getLayoutParams();
                    leftLayoutParams.weight = 0;
                    leftLayoutParams.width = (int) (id3TitleSpaceOriginalAvgWidth * (1 - slideOffset));
                    mId3TitleLeftSpace.setLayoutParams(leftLayoutParams);
                }

                // ID3 专辑名称控件压缩变化动态调整
                if (mId3AlbumLeftSpace != null) {
                    // ID3 Album 占位符当前平均宽度
                    int id3AlbumSpaceOriginalAvgWidth = (mId3InfoCardLayoutWidth - id3AlbumWidth) / 2;

                    // 动态移动 ID3 Album 的位置
                    LinearLayout.LayoutParams leftLayoutParams
                            = (LinearLayout.LayoutParams) mId3AlbumLeftSpace.getLayoutParams();
                    leftLayoutParams.weight = 0;
                    leftLayoutParams.width = (int) (id3AlbumSpaceOriginalAvgWidth * (1 - slideOffset));
                    mId3AlbumLeftSpace.setLayoutParams(leftLayoutParams);
                }

                // ID3 艺术家名称控件压缩变化动态调整
                if (mId3ArtistLeftSpace != null) {
                    // ID3 Artist 占位符当前平均宽度
                    int id3ArtistSpaceOriginalAvgWidth = (mId3InfoCardLayoutWidth - id3ArtistWidth) / 2;

                    // 动态移动 ID3 Artist 的位置
                    LinearLayout.LayoutParams leftLayoutParams
                            = (LinearLayout.LayoutParams) mId3ArtistLeftSpace.getLayoutParams();
                    leftLayoutParams.weight = 0;
                    leftLayoutParams.width = (int) (id3ArtistSpaceOriginalAvgWidth * (1 - slideOffset));
                    mId3ArtistLeftSpace.setLayoutParams(leftLayoutParams);
                }
            }
        }

        @Override
        public void onDrawerOpened(@NonNull View drawerView) {
            LogUtil.v(TAG, "onDrawerOpened.");

            // 被动延迟加载
            initViewPager2Adapter("Passive lazy init.");

            DrawerLayoutEx drawerLayout = mDrawerLayoutRef.get();
            if (drawerLayout != null) {
                drawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_LOCKED_OPEN);
            }

            // 打开状态禁止点击
            if (mDrawerHandleView != null) {
                mDrawerHandleView.setClickable(false);
            }
        }

        @Override
        public void onDrawerClosed(@NonNull View drawerView) {
            LogUtil.v(TAG, "onDrawerClosed.");

            DrawerLayoutEx drawerLayout = mDrawerLayoutRef.get();
            if (drawerLayout != null) {
                drawerLayout.setDrawerLockMode(DrawerLayout.LOCK_MODE_UNLOCKED);
            }

            // 关闭状态允许点击
            if (mDrawerHandleView != null) {
                mDrawerHandleView.setClickable(true);
            }

            // 内容页面处理
            if (Objects.isNull(mContentPageCard)) {
                return;
            }

            LinearLayout.LayoutParams layoutParams
                    = (LinearLayout.LayoutParams) mContentPageCard.getLayoutParams();
            layoutParams.width = 0;
            layoutParams.weight = 1;
            mContentPageCard.setLayoutParams(layoutParams);

            // ID3 歌曲名称控件压缩变化动态调整
            if (mId3TitleLeftSpace != null) {
                // 恢复 ID3 歌曲名占位视图默认权重
                LinearLayout.LayoutParams leftLayoutParams
                        = (LinearLayout.LayoutParams) mId3TitleLeftSpace.getLayoutParams();
                leftLayoutParams.width = 0;
                leftLayoutParams.weight = 1;
                mId3TitleLeftSpace.setLayoutParams(leftLayoutParams);
            }

            // ID3 专辑名称控件压缩变化动态调整
            if (mId3AlbumLeftSpace != null) {
                // 恢复 ID3 专辑名占位视图默认权重
                LinearLayout.LayoutParams leftLayoutParams
                        = (LinearLayout.LayoutParams) mId3AlbumLeftSpace.getLayoutParams();
                leftLayoutParams.width = 0;
                leftLayoutParams.weight = 1;
                mId3AlbumLeftSpace.setLayoutParams(leftLayoutParams);
            }

            // ID3 艺术家名称控件压缩变化动态调整
            if (mId3ArtistLeftSpace != null) {
                // 恢复 ID3 艺术家占位视图默认权重
                LinearLayout.LayoutParams leftLayoutParams
                        = (LinearLayout.LayoutParams) mId3ArtistLeftSpace.getLayoutParams();
                leftLayoutParams.width = 0;
                leftLayoutParams.weight = 1;
                mId3ArtistLeftSpace.setLayoutParams(leftLayoutParams);
            }
        }

        @Override
        public void onDrawerStateChanged(int newState) {
            LogUtil.v(TAG, "onDrawerStateChanged: " + newState);

            // 内容页面处理
            if (Objects.isNull(mContentPageCard)) {
                return;
            }

            //TODO: 状态保留
        }

        private void initViewPager2Adapter(String reason) {
            if (mSimpleListViewPager2 == null) {
                return;
            }

            if (mFragmentStateAdapter != null) {
                return;
            }

            LogUtils.vTag(TAG, "init reason: " + reason);
            mFragmentStateAdapter = new FragmentStateAdapterEx(MusicInfoFragment.this);
            mSimpleListViewPager2.setAdapter(mFragmentStateAdapter);

            // 默认先显示播放列表
            mSimpleListViewPager2.setCurrentItem(ISimpleList.PAGE_PLAYLIST);
        }


        /**
         * 更新 Simple List 的标签布局
         * <p> 主要是当前显示哪个列表信息，高亮 Tab 选项；
         *
         * @param pageType 页面类型 {@link ISimpleList}
         */
        @SuppressLint("SwitchIntDef")
        private void updateSimpleListTabLayout(@ISimpleList int pageType) {
            if (Objects.isNull(mSimpleListLeftTab)
                    || Objects.isNull(mSimpleListRightTab)) {
                return;
            }

            switch (pageType) {
                case ISimpleList.PAGE_FAVORITE:
                    mSimpleListRightTab.setChecked(true);
                    break;
                case ISimpleList.PAGE_PLAYLIST:
                default:
                    mSimpleListLeftTab.setChecked(true);
                    break;
            }
        }
    }

    /**
     * ViewPager2 的适配器
     * <pre>
     *    ViewPager2 是基于 RecyclerView 实现的；
     *    所以当前适配器其实就是 RecyclerView.Adapter<T> 的实现；
     * </pre>
     */
    private static final class FragmentStateAdapterEx extends FragmentStateAdapter {
        private final Reference<MusicInfoFragment> mOwnerRef;
        private final SimpleListFragment[] mListFragment = new SimpleListFragment[ISimpleList.PAGE_SIZE];

        public FragmentStateAdapterEx(@NonNull MusicInfoFragment fragment) {
            super(fragment);

            mOwnerRef = new WeakReference<>(fragment);
        }

        @Override
        public int getItemCount() {
            final int theme = Argument.getThemeX();
            switch (theme) {
                case ThemeX.ET_GOD_400_016:
                case ThemeX.ET_GOD_400_022:
                    // 只显示一个 SimpleList 播放列表
                    return ISimpleList.PAGE_PLAYLIST + 1;
                case ThemeX.ET_GOD_NONE:
                    return 0;
                default:
                    break;
            }

            return SkinX.getInteger("simple_list_item_count");
        }

        @NonNull
        @Override
        public Fragment createFragment(int position) {
            switch (position) {
                case ISimpleList.PAGE_PLAYLIST:
                case ISimpleList.PAGE_FAVORITE: {
                    MusicInfoFragment fragment = mOwnerRef.get();
                    if (fragment != null) {
                        fragment.sendPageEvent(
                                PageEvent.PREPARE_CREATE_FRAGMENT,
                                "simple-list-" + position,
                                null);
                    }

                    mListFragment[position] = SimpleListFragment.newInstance(position);
                    return mListFragment[position];
                }

                default:
                    break;
            }

            throw new IndexOutOfBoundsException("createFragment position out of bounds!");
        }

        /**
         * 获取指定位置 SimpleListFragment 对象
         *
         * @param position 页面位置信息
         * @return {@link SimpleListFragment}
         */
        public SimpleListFragment getFragment(int position) {
            return mListFragment[position];
        }

        @Override
        public void onAttachedToRecyclerView(@NonNull RecyclerView recyclerView) {
            super.onAttachedToRecyclerView(recyclerView);
        }

        @Override
        public void onBindViewHolder(@NonNull FragmentViewHolder holder,
                                     int position, @NonNull List<Object> payloads) {
            super.onBindViewHolder(holder, position, payloads);

            // 构建视图的时候才会被调用
            LogUtil.v(TAG, "onBindViewHolder: position = " + position);
        }

        @Override
        public void onDetachedFromRecyclerView(@NonNull RecyclerView recyclerView) {
            super.onDetachedFromRecyclerView(recyclerView);
        }
    }

    /**
     * 初始化按钮控件；
     * @param fragmentView 页面视图
     */
    private void initClickView(@NonNull View fragmentView) {
        if (mInitView) {
            return;
        }

        // 列表切换
        View btnList = fragmentView.findViewById(xId(R.id.btnList));
        if (null != btnList) {
            btnList.setOnClickListener(this);
        }

        // 文件夹切换
        View btnFolder = fragmentView.findViewById(xId(R.id.btnFolder));
        if (null != btnFolder) {
            btnFolder.setOnClickListener(this);
        }

        // 播放/暂停按钮
        mBtnPlay = fragmentView.findViewById(xId(R.id.btnPlay));
        if (null != mBtnPlay) {
            mBtnPlay.setOnClickListener(this);
        }

        // 切换上一曲
        View btnPrev = fragmentView.findViewById(xId(R.id.btnPrev));
        if (null != btnPrev) {
            btnPrev.setOnClickListener(this);
        }

        // 切换下一曲
        View btnNext = fragmentView.findViewById(xId(R.id.btnNext));
        if (null != btnNext) {
            btnNext.setOnClickListener(this);
        }

        // 播放模式
        mBtnRepeatMode = fragmentView.findViewById(xId(R.id.btnRepeatMode));
        if (null != mBtnRepeatMode) {
            mBtnRepeatMode.setOnClickListener(this);
        }

        // 音效按钮
        mBtnEQ = fragmentView.findViewById(xId(R.id.btnEQ));
        if (null != mBtnEQ) {
            mBtnEQ.setOnClickListener(this);
        }

        // 频谱歌词切换
        mBtnChangeLyric = fragmentView.findViewById(xId(R.id.btn_change_lyric));
        if (null != mBtnChangeLyric) {
            mBtnChangeLyric.setOnClickListener(this);
        }
    }

    /**
     * 测试接口（调试使用）
     * <p> 用来测试接口的调用情况；
     *
     * @param isInMultiWindowMode 是否是分屏模式
     */
    private void aJustMultiWindowMode(boolean isInMultiWindowMode) {
        // 具体业务逻辑由皮肤包去实现
        if (mPageExtend != null) {
            String result = mPageExtend.tryCallMethod(
                    "aJustMultiWindowMode", isInMultiWindowMode);
            LogUtil.v(TAG, "tryCallMethod/aJustMultiWindowMode: " + result);
        }
    }

    /**
     * 在 onCreateView() 之后调用；
     *
     * @param view
     * @param savedInstanceState
     */
    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        // 判断是分屏状态
        boolean isInSplitScreenMode = false;
        Configuration configuration = getResources().getConfiguration();
        if (configuration != null) {
            String configText = configuration.toString();
            LogUtil.v(TAG, "onViewCreated: " + configText);

            if (!TextUtils.isEmpty(configText)) {
                boolean isContainsMultiWindow = Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU && configText.contains("mWindowingMode=multi-window");
                if (requireActivity().isInMultiWindowMode()
                        && (Build.VERSION.SDK_INT < Build.VERSION_CODES.P
                            || configText.contains("mWindowingMode=split-screen") || isContainsMultiWindow)) {
                    isInSplitScreenMode = true;
                }
            }
        }

        // 调整多窗口模式相关显示
        aJustMultiWindowMode(isInSplitScreenMode);

        // 是竖屏显示状态
        if (MiscUtils.isPortraitWindow(requireContext()) || isInSplitScreenMode) {
            // 隐藏动画视图
            if (mIvPlayingAnim != null && !mIsKeepPlayingAnim) {
                mIvPlayingAnim.setVisibility(View.GONE);
            }

            // 隐藏唱片的拾音器
            if (mIvPlayingPickup != null) {
                mIvPlayingPickup.setVisibility(View.GONE);
            }

            // 显示模式（按比例缩放居中）
            if (mMusicImage != null) {
                // 支持使用 CircleImageView 组件
                if (!(mMusicImage instanceof CircleImageView)) {
                    mMusicImage.setScaleType(ImageView.ScaleType.CENTER_INSIDE);
                }
            }
        }

        // 特定主体分屏显示状态处理
        if (Argument.isThemeGod(ThemeX.ET_GOD_204)) {
            if (mBtnEQ != null) {
                mBtnEQ.setVisibility(isInSplitScreenMode? View.GONE: View.VISIBLE);
            }
        }
        Feature mFeature = Feature.instance();
        if(mFeature.hasFeature(REMOTE_CONTROL_FOCUS)){
            if (mBtnEQ != null) {
                mBtnEQ.setVisibility(View.GONE);
            }
        }
    }

    /**
     * 处理页面事件
     *
     * @param event 事件 ID
     * @param obj1  附加数据对象 1
     * @param obj2  附加数据对象 2
     */
    @Override
    protected void onHandlePageEvent(int event, Object obj1, Object obj2) {
        Logger.t(TAG).v("onHandlePageEvent: " + PageEvent.name(event) + " / " + obj1);

        switch (event) {
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
                updateFavoriteStateIcon();
                break;
            case IMediaEvent.EVENT_CHANGE_REPEAT_MODE:
                // 这里其实不应该直接使用全局对象（后续调整）
                updatePlayRepeatMode(mAppData.musicRepeatMode());
                break;
            case IMediaEvent.EVENT_MUSIC_PLAYER_PREPARING:
                onMusicPlayerPreparing();
                break;
            case IMediaEvent.EVENT_MUSIC_FAVORITE_OPERATE:
                onMusicFavoriteOperateEvent(obj1, obj2);
                break;
            case IMediaEvent.EVENT_UPDATE_MUSIC_ID3:
                updateMusicTextInfo();
                break;
            case IMediaEvent.EVENT_SPLIT_SCREEN_UPDATE_PLAY_STATE:
                onChangePlayCtrl(IMusicState.E_PLAY_STATE_STOP);
                break;
            default:
                break;
        }

        // 通知扩展页面有媒体事件发生
        tryCallPageExtendMethod("onHandlePageEvent", event, obj1, obj2);
    }

    /**
     * 更新播放模式显示 UI
     * <p> 新的显示更新接口，支持 UI 元素的扩展设计；
     *
     * @param playMode 播放模式
     */
    private void updatePlayRepeatMode(Object playMode) {
        // 新的设计将统一元素命名
        if (Objects.isNull(mIvPlayMode)) {
            return;
        }

        // 参数检查，必须是整形变量
        if (!(playMode instanceof Integer)) {
            return;
        }

        // 请遵循统一的皮肤命名规则
        String resName = "simple_repeat_all";
        int mode = (int) playMode;
        switch (mode) {
            case IMusicState.REPEAT_MODE_ONE:
                resName = "simple_repeat_one";
                break;
            case IMusicState.REPEAT_MODE_RANDOM:
                resName = "simple_shuffle";
                break;
            case IMusicState.REPEAT_MODE_QUEUE:
            default:
                break;
        }

        // 更新对应的 UI 图标资源
        if (mIvPlayMode instanceof ImageView) {
            ImageView imageView = (ImageView) mIvPlayMode;
            imageView.setImageDrawable(xDrawable(resName));
        }
    }

    /**
     * 切曲成功更新状态
     * <p> 此处可以更新收藏 icon 状态、歌曲名称等信息；
     *
     * @see IMediaEvent#EVENT_MUSIC_PLAYER_PREPARING
     */
    private void onMusicPlayerPreparing() {
        // 更新收藏 icon 图标
        updateFavoriteStateIcon();
    }

    /**
     * 当前音乐收藏列表发生改变
     * @see IMediaEvent#EVENT_MUSIC_FAVORITE_OPERATE
     *
     * @param obj1 操作类型
     * @param obj2 附加参数
     */
    private void onMusicFavoriteOperateEvent(Object obj1, Object obj2) {
        // 最喜欢列表操作参数有效性检查
        if (!(obj1 instanceof String)) {
            Logger.t(TAG).w("onMusicFavoriteOperateEvent: error parameter type!");
            return;
        }

        final String operateType = (String) obj1;
        switch (operateType) {
            case FavoriteManager.OPERATE_MAX_LIMIT:
                Logger.t(TAG).w("onMusicFavoriteOperateEvent:" +
                        " The collection list has reached the maximum threshold!");
                if (MiscUtils.isPortraitWindow(requireContext())) {
                    HToastUtils.make()
                            .setLeftIcon(R.drawable.ic_favorite_48)
                            .show(R3.string.maximum_collection_limit_prompt);
                } else {
                    HSnackbarUtils.with(requireView())
                            .setRadius(6)
                            .setBottomMargin(32)
                            .setAlpha(0.9f)
                            .setLeftDrawable(ContextCompat.getDrawable(requireContext(), R.drawable.ic_favorite_48))
                            .setMessage(getString(R3.string.maximum_collection_limit_prompt))
                            .setMessageSize(24)
                            .setBgColor(HColorUtils.getRandomColor(false))
                            .setDuration(HSnackbarUtils.LENGTH_LONG)
                            .show();
                }
                break;
            case FavoriteManager.OPERATE_INITED:
                // 检查收藏列表初始化状态
                FavoriteManager fm = FavoriteManager.getInstance();
                boolean musicFavoriteInited = fm.isInitCompleted(FavoriteManager.Type.MUSIC);
                if (mIvFavorite != null) {
                    mIvFavorite.setEnabled(musicFavoriteInited);
                }
                break;
            case FavoriteManager.OPERATE_UPDATE:
                onMusicFavoriteUpdateOperate();
                break;
            default:
                break;
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
            case "isHorizontalDevicePortraitShow":
                return isHorizontalDevicePortraitShow();
            case "test-case":
                LogUtil.v(TAG, "requestExecuteMethod_Impl/this is a test case!");
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
    public void onSaveInstanceState(@NonNull Bundle outState) {
        super.onSaveInstanceState(outState);
        Log.d(TAG, "onSaveInstanceState");
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        // 分屏的时候大多数情况好像收不到回调
        LogUtil.d(TAG, "onConfigurationChanged: newConfig = " + newConfig.toString());

        // mWallpaperSelectorLayout 需要隐藏掉
        hideWallpaperLayout();
    }

    /**
     * 初始化动画
     * <pre>
     *     专辑封面的旋转动画；
     *     低配机器上建议不要打开，消耗 CPU 性能；
     * </pre>
     */
    private void initAnimation() {
        // 获取 CPU 核数
        int cpuCoreNum = Runtime.getRuntime().availableProcessors();
        if (cpuCoreNum < 8) {
            // 是否使用专辑封面旋转动画
            if (!HMediaConfig.USE_ALBUM_COVER_ROTATE_ANIM) {
                return;
            }
        }

        // 初始化专辑封面旋转动画
        if (null != mIvPlayingAnim) {
            mRotateAnim = ObjectAnimator.ofFloat(mIvPlayingAnim, "rotation", 0, 360);
            mRotateAnim.setDuration(9000);
            mRotateAnim.setInterpolator(new LinearInterpolator());
            mRotateAnim.setRepeatCount(ObjectAnimator.INFINITE);
            mRotateAnim.setRepeatMode(ObjectAnimator.RESTART);
            mRotateAnim.start();
        }
    }

    /**
     * 初始化进度布局
     * <pre>
     *    显示播放时间文字信息；
     *    更新播放进度的进度条控件；
     * </pre>
     *
     * @param fragmentView 页面视图
     */
    private void initSeekbarProgress(View fragmentView) {
        if (mInitView) {
            return;
        }

        // 播放进度信息布局
        View progressView = fragmentView.findViewById(xId(R.id.layout_progress));
        if (!Objects.isNull(progressView)) {
            progressView.setOnTouchListener(this);
        }

        // 播放进度文字信息
        mTvCurrentTime = fragmentView.findViewById(xId(R.id.tvCurrentTime));
        mTvTotalTime = fragmentView.findViewById(xId(R.id.tvTotalTime));

        // 播放进度条 SeekBar
        mSeekbarProgress = fragmentView.findViewById(xId(R.id.seekbar_progress));
        if (mSeekbarProgress != null) {
            mSeekbarProgress.setMax(SEEKBAR_MAX_VALUE);

            // 设置播放进度条拖动事件监听
            mSeekbarProgress.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {
                    mSeekbarTracking = false;

                    int progress = seekBar.getProgress();
                    int totalTime = mAppData.mPlayTimeInfo.mTotalTime;
                    int time = (int) (totalTime * (progress * 1.0f / SEEKBAR_MAX_VALUE));
                    requestMediaAction(IMediaAction.seekToTime, time);

                    onHideSeekTimeLayout();
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {
                    Log.d(TAG, "onStartTrackingTouch");
                    mSeekbarTracking = true;

                    if (mListener != null) {
                        mListener.onMediaEvent(
                                IMediaEvent.EVENT_SCROLL_SEEKBAR, null, null);
                    }

                    if (mSeekbarProgress.getParent() != null) {
                        mSeekbarProgress.getParent().requestDisallowInterceptTouchEvent(true);
                    }
                }

                @Override
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                    // TODO Auto-generated method stub

                    if (mSeekbarTracking) {
                        float percentage = progress * 1.0f / SEEKBAR_MAX_VALUE;
                        int newPlayTime = (int) (mAppData.mPlayTimeInfo.mTotalTime * percentage);

                        int targetTime = newPlayTime / 1000;
                        int deltaTime = (newPlayTime - mAppData.mPlayTimeInfo.mCurrentTime) / 1000;

                        onShowSeekTimeLayout(targetTime,deltaTime);
                    }

                }
            });
        }
    }

    /**
     * 更新显示播放事件进度相关视图信息
     *
     * @param newTime 为需要 Seek 的目标时间;
     * @param delayTime 为需要 Seek 的时间差;
     */
    private void onShowSeekTimeLayout(int newTime, int delayTime) {
        if (mSeekTimeLayout != null) {
            mSeekTimeLayout.setVisibility(View.VISIBLE);
        }

        String[] seekTime = computeSeekBarDelayTime(newTime, delayTime);

        if (m_tvNewTime != null) {
            m_tvNewTime.setText(seekTime[0]);
        }

        if (m_tvDelayTime != null) {
            m_tvDelayTime.setText(seekTime[1]);
        }
    }

    private void onHideSeekTimeLayout() {
        if (mSeekTimeLayout != null) {
            mSeekTimeLayout.setVisibility(View.GONE);
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (getId(v)) {
            case R.id.btnList:
                onListEvent();
                break;
            case R.id.btnFolder:
                mAppData.mMusicListPageType
                        = IMusicState.PAGE_INDEX_FOLDER;
                onListEvent();
                break;
            case R.id.btnPlay:
                onPlayEvent();
                break;
            case R.id.btnPrev:
                onPrevEvent();
                break;
            case R.id.btnNext:
                onNextEvent();
                break;
            case R.id.btnRepeatMode:
                onRepeatModeEvent();
                break;
            case R.id.btnEQ:
                HFuncUtils.instance().gotoEQ(mContext);
                break;
            case R.id.btn_change_lyric:
                switchLyricsAndSpectrum();
                break;
            default:
                break;
        }
    }

    /**
     * 切换频谱和歌词显示
     * <p> 歌词显示布局和频谱视图之间做互斥显示
     */
    private void switchLyricsAndSpectrum() {
        // 不是同类型布局设计，不处理；
        if (ThemeEx.useMusicDrawerLayoutStyle()) {
            return;
        }

        if (null != mLrcLayout && null != mFrequencyView) {
            int resID = R.drawable.btn_freq;

            if (mLrcLayout.getVisibility() == View.VISIBLE) {
                mLrcLayout.setVisibility(View.INVISIBLE);
                mFrequencyView.setVisibility(View.VISIBLE);
            } else {
                mLrcLayout.setVisibility(View.VISIBLE);
                mFrequencyView.setVisibility(View.INVISIBLE);
                resID = R.drawable.btn_lrc;
            }

            if (mBtnChangeLyric != null) {
                if (mBtnChangeLyric instanceof ImageView) {
                    ((ImageView) mBtnChangeLyric).setImageResource(xDrawableId2(resID));
                } else if (mBtnChangeLyric instanceof Button) {
                    mBtnChangeLyric.setBackgroundResource(xDrawableId2(R.drawable.btn_lrc));
                }
            }
        }
    }

    /**
     * 显示歌词布局
     * <p> 对应就隐藏频谱视图；
     */
    private void showLyricsLayout() {
        // 不是同类型布局设计，不处理；
        if (ThemeEx.useMusicDrawerLayoutStyle()) {
            return;
        }

        // 显示歌词
        if (mLrcLayout != null) {
            mLrcLayout.setVisibility(View.VISIBLE);
        }

        // 隐藏频谱
        if (mFrequencyView != null) {
            mFrequencyView.setVisibility(View.INVISIBLE);
        }

        // 切换图标
        if (mBtnChangeLyric != null) {
            if (mBtnChangeLyric instanceof ImageView) {
                ((ImageView) mBtnChangeLyric).setImageResource(xDrawableId2(R.drawable.btn_lrc));
            } else if (mBtnChangeLyric instanceof Button) {
                ((Button) mBtnChangeLyric).setBackgroundResource(xDrawableId2(R.drawable.btn_lrc));
            }
        }
    }

    /**
     * 显示频谱视图
     * <p> 对应就隐藏歌词布局视图；
     */
    private void showFreqSpectrumView() {
        // 不是同类型布局设计，不处理；
        if (ThemeEx.useMusicDrawerLayoutStyle()) {
            return;
        }

        // 隐藏歌词
        if (mLrcLayout != null) {
            mLrcLayout.setVisibility(View.INVISIBLE);
        }

        // 显示频谱
        if (mFrequencyView != null) {
            mFrequencyView.setVisibility(View.VISIBLE);
        }

        // 切换图标
        if (mBtnChangeLyric != null) {
            if (mBtnChangeLyric instanceof ImageView) {
                ((ImageView) mBtnChangeLyric).setImageResource(xDrawableId2(R.drawable.btn_freq));
            } else if (mBtnChangeLyric instanceof Button) {
                mBtnChangeLyric.setBackgroundResource(xDrawableId2(R.drawable.btn_freq));
            }
        }
    }

    private void onListEvent() {
        mMusicViewModel.fragment2MainUi().execute(
                t -> t.onEvent(IMediaEvent.EVENT_SHOW_MUSIC_FRAGMENT,
                        IMusicPage.MCC204_E_GROUP_SHOW_MUSIC_LIST, null));
    }

    private void onPlayEvent() {
        if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PAUSE)) {
            requestMediaAction(IMediaAction.playControl, IMusicState.PLAY_CMD_PLAY);
        } else {
            requestMediaAction(IMediaAction.playControl, IMusicState.PLAY_CMD_PAUSE);
        }
    }

    private void onPrevEvent() {
        requestMediaAction(IMediaAction.setSeekTimeZero);
        requestMediaAction(IMediaAction.playControl, IMusicState.PLAY_CMD_PREV);
    }

    private void onNextEvent() {
        requestMediaAction(IMediaAction.setSeekTimeZero);
        requestMediaAction(IMediaAction.playControl, IMusicState.PLAY_CMD_NEXT);
    }

    /**
     * 点击播放循环模式按钮
     * <p> 全部循环、单曲循环、随机循环...
     */
    private void onRepeatModeEvent() {
        // 请求执行播放模式改变活动
        requestMediaAction(IMediaAction.switchPlayRepeatMode);

        // 更新循环模式的 UI 显示状态
        onChangeRepeatPlayMode(mAppData.musicRepeatMode());

        tryCallPageExtendMethod("onHandlePageEvent", IMediaEvent.EVENT_CHANGE_REPEAT_MODE, null, null);
    }

    private void onEQEvent() {
        McuManager.getsInstance().injectKeyEventTimeout(K_EQ, 50);
    }

    /**
     * 更新播放状态显示控件
     * <p> 播放/暂停图标、启停播放相关动画等；
     *
     * @param playState 播放状态
     */
    private void onChangePlayCtrl(int playState) {
        int nResId = 0;

        // 播放状态
        switch (playState) {
            case IMusicState.E_PLAY_STATE_PAUSE:
            case IMusicState.E_PLAY_STATE_STOP:
                nResId = R.drawable.btn_play_bg;
                if (mRotateAnim != null) {
                    mRotateAnim.pause();
                }
                break;
            case IMusicState.E_PLAY_STATE_PLAY:
                nResId = R.drawable.btn_pause_bg;
                if (mRotateAnim != null) {
                    // 如果是倒车状态停止刷新
                    if (AutoStatus.isReversing()) {
                        mRotateAnim.pause();
                        Log.d(TAG, "reversing, pause rotate animation!");
                    } else {
                        mRotateAnim.resume();
                    }
                }
                break;
            default:
                break;
        }

        // 更换播放状态图标
        if (mBtnPlay != null) {
            if (mBtnPlay instanceof ImageView) {
                ((ImageView) mBtnPlay).setImageResource(xDrawableId2(nResId));
            } else if (mBtnPlay instanceof Button) {
                mBtnPlay.setBackgroundResource(xDrawableId2(nResId));
            }
        }
    }

    /**
     * 更新循环播放模式 UI 元素
     * <p> 当播放模式改变的时候触发调用，用来刷新播放模式 UI 显示；
     *
     * @param playState 播放模式
     */
    private void onChangeRepeatPlayMode(int playState) {
        // 新旧设计兼容处理
        if (Objects.isNull(mBtnRepeatMode)) {
            updatePlayRepeatMode(playState);
            return;
        }

        int nResId = 0;
        switch (playState) {
            case IMusicState.REPEAT_MODE_QUEUE:
                nResId = R.drawable.btn_repeat_queue_bg;
                break;
            case IMusicState.REPEAT_MODE_ALL:
                nResId = R.drawable.btn_repeat_all_bg;
                break;
            case IMusicState.REPEAT_MODE_ONE:
                nResId = R.drawable.btn_repeat_one_bg;
                break;
            case IMusicState.REPEAT_MODE_RANDOM:
                nResId = R.drawable.btn_repeat_random_bg;
                break;
            default:
                break;
        }

        if (mBtnRepeatMode instanceof Button) {
            mBtnRepeatMode.setBackgroundResource(xDrawableId2(nResId));
        } else if (mBtnRepeatMode instanceof ImageView) {
            ((ImageView) mBtnRepeatMode).setImageResource(xDrawableId2(nResId));
        }
    }

    /**
     * 处理收藏点击事件
     * <pre>
     *    当前歌曲如果未在收藏列表，点击后添加收藏；
     *    当前歌曲如果已在收藏列表，点击后取消收藏；
     * </pre>
     */
    private void onClickFavoriteEvent() {
        // 当前媒体播放信息检查
        MusicInfo info = mAppData.mCurrentMediaInfo;
        if (Objects.isNull(info)) {
            return;
        }

        // 获取收藏列表管理器
        FavoriteManager fm = FavoriteManager.getInstance();
        if (info.mFavorite) {
            info.mFavorite = false;
            fm.removeFavoriteMusic(info);
        } else {
            // 收藏成功与否
            if (fm.addFavoriteMusic(info, false)) {
                info.mFavorite = true;
            }
        }

        // 更新收藏状态图标
        updateFavoriteStateIcon();
    }

    /** 更新当前页面收藏状态图标 **/
    private void updateFavoriteStateIcon() {
        // 新的设计将统一元素命名
        if (Objects.isNull(mIvFavorite)) {
            return;
        }

        // 当前媒体播放信息检查
        MusicInfo info = mAppData.currentMediaInfo();
        if (Objects.isNull(info)) {
            LogUtil.v(TAG, "updateFavoriteStateIcon: media info is null!");
            return;
        }

        // 请遵循统一的皮肤命名规则
        String resName = info.mFavorite?
                "simple_collect": "simple_uncollect";
        LogUtil.v(TAG, "updateFavoriteStateIcon: " +
                "info = " + info.mFileName + ", favorite = " + info.mFavorite);

        // 更新对应的 UI 图标资源
        if (mIvFavorite instanceof ImageView) {
            ImageView imageView = (ImageView) mIvFavorite;
            imageView.setImageDrawable(xDrawable(resName));
        }
    }

    /**
     * 处理音乐收藏夹更新操作
     * <pre>
     *    存储设备发生改变（插入、移除）；
     *    休眠唤醒触发挂载相关事件等；
     *    这里主要是用来修正当前播放的收藏显示状态图标；
     * </pre>
     */
    private void onMusicFavoriteUpdateOperate() {
        // 是否有合法播放信息（非收藏状态才需要检查）
        MusicInfo currentInfo = mAppData.currentMediaInfo();
        if (Objects.isNull(currentInfo) || currentInfo.mFavorite) {
            return;
        }

        FavoriteManager fm = FavoriteManager.getInstance();
        final List<MusicInfo> favoriteList = new ArrayList<>(fm.favoriteMusicList());
        String currentFilePath = currentInfo.mFilePath;

        // 更新检查当前播放歌曲是否在收藏歌曲列表中
        Observable.create((ObservableOnSubscribe<Boolean>) emitter -> {
                    if (!emitter.isDisposed()) {
                        boolean needUpdate = false;
                        for (MusicInfo info : favoriteList) {
                            if (Objects.isNull(info)) {
                                continue;
                            }

                            // 找到匹配的媒体信息
                            if (MiscUtils.reverseEquals(info.mFilePath, currentFilePath)) {
                                needUpdate = true;
                                break;
                            }
                        }

                        emitter.onNext(needUpdate);
                        emitter.onComplete();
                    }
                }).subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new ObserverEx<Boolean>() {

                    @Override
                    public void onNext(@NonNull Boolean update) {
                        if (!update) {
                            return;
                        }

                        // 回到主线程需要重新判定
                        // 检查更新当前播放信息收藏状态
                        MusicInfo info = mAppData.currentMediaInfo();
                        if (!Objects.isNull(info)
                                && !info.mFavorite
                                && MiscUtils.reverseEquals(info.mFilePath, currentFilePath)) {
                            info.mFavorite = true;
                        }

                        // 需在初始化状态才需要更新
                        if (mInitView) {
                            updateFavoriteStateIcon();
                        }
                    }
                });
    }

    @Override
    public void onExternalEvent(String event, int arg1, int arg2) {
        super.onExternalEvent(event, arg1, arg2);

        // 倒车事件改变
        if ("reverse".equals(event)) {
            if (!isResumed() || isHidden()) {
                return;
            }

            Log.d(TAG, "onExternalEvent, reverse!");
            onChangePlayCtrl(mAppData.mMediaPlayState);
        }
    }

    /**
     * 更新当前页面显示信息
     * <p> ID3、播放状态、播放进度等等；
     */
    public void updateFragment() {
        // 更新歌词显示信息
        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            MusicInfo info = mAppData.musicPlayPositionInfo();
            if (mFirstChanged) {
                updateLrcRowList(info);
            }
        }

        // 更新播放相关信息
        updateMusicInfo();
        updateFavoriteStateIcon();
        onUpdateSeekbar(mAppData.mMediaPlayState);
        onChangePlayCtrl(mAppData.mMediaPlayState);
        onChangeRepeatPlayMode(mAppData.musicRepeatMode());
        onChangeSeekbarValue(mAppData.mPlayTimeInfo);

        // 音频数据捕获设置
        initVisualizer();
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE: {
                if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_MUSIC) {
                    onChangePlayCtrl(IMusicState.E_PLAY_STATE_STOP);
                    return;
                }

                if (!mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                    onUpdateSeekbar(mAppData.mMediaPlayState);
                }

                onChangePlayCtrl(mAppData.mMediaPlayState);
                if (!mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                    uninitVisualizer();
                }
                break;
            }

            case IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME: {
                if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_MUSIC) {
                    return;
                }

                // avoid when on play moment seek SeekBar
                if (!Objects.isNull(mSeekbarProgress)
                        && !mSeekbarProgress.isEnabled()) {
                    onUpdateSeekbar(mAppData.mMediaPlayState);
                }

                onChangeSeekbarValue(mAppData.mPlayTimeInfo);
                Visualizer visualizer = mMusicViewModel.getVisualizer();
                if (visualizer != null && !visualizer.getEnabled()) {
                    initVisualizer();
                }

                onChangeLyricsView(mAppData.mPlayTimeInfo);
                break;
            }

            case IMediaEvent.EVENT_CHANGE_REPEAT_MODE: {
                onChangeRepeatPlayMode(mAppData.musicRepeatMode());
                break;
            }

            case IMediaEvent.EVENT_UPDATE_MUSIC_LIST:
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST: {
                updateMusicInfo();
                break;
            }

            case IMediaEvent.EVENT_SEEK_TO_COMPLETE: {
                if (mAppData.mMediaType != IMusicState.MEDIA_TYPE_MUSIC) {
                    return;
                }

                // [补充更新 ID3 信息]
                if (!ViewUtilsEx.isVisible(mMusicImage, View.VISIBLE)) {
                    updateId3Info(currentPlayInfo());
                }
                break;
            }

            default:
                break;
        }
    }

    /**
     * 获取当前播放信息对象
     * <p> 注意它可能返回 null;
     *
     * @return {@link MusicInfo}
     */
    private MusicInfo currentPlayInfo() {
        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            return mAppData.musicPlayPositionInfo();
        }
        return null;
    }

    /**
     * 播放时间改变，对应歌词显示也改变
     * <p> 歌词焦点随时间变化效果，歌词滚动效果；
     *
     * @param info 时间信息
     */
    private void onChangeLyricsView(MediaTimeInfo info) {
        if (Objects.isNull(mLyricsView)) {
            return;
        }

        if (mLyricsList.size() > 0) {
            mLyricsView.seekTo(info.mCurrentTime, true, false);
        }
    }

    /**
     * 更新频谱视图
     * <p> 非前台页面的时候都可以不更新，倒车状态也不更新（影响效率）;
     *
     * @param data 音频数据
     */
    public void updateVisualizer(@NonNull byte[] data) {
        // 倒车状态不更新
        if (AutoStatus.isReversing()) {
            return;
        }

        // 没有频谱控件不显示
        if (Objects.isNull(mFrequencyView)) {
            return;
        }

        mFrequencyView.updateVisualizer(data);
    }

    private int getStatusBarHeight() {
        int statusBarHeight = 0;
        Resources res = mContext.getResources();
        @SuppressLint("InternalInsetResource")
        int resourceId = res.getIdentifier("status_bar_height", "dimen", "android");
        if (resourceId > 0) {
            statusBarHeight = res.getDimensionPixelSize(resourceId);
        }

        return statusBarHeight;
    }

    /**
     * 更新专辑封面
     * <p> 专辑封面以及整个播放背景（模斯效果）；
     *
     * @param info
     */
    private void updateMusicImage(MusicInfo info) {
        // 无专辑图片空间
        if (Objects.isNull(mMusicImage)) {
            if (mBgView != null) {
                mBgView.clearAnimation();
                mBgView.setVisibility(View.GONE);
            }

            if (mBgMaskView != null) {
                mBgMaskView.clearAnimation();
                mBgMaskView.setVisibility(View.GONE);
            }
            return;
        }

        // 避免重复解析专辑封面
        Object tag = mMusicImage.getTag();
        if (!Objects.isNull(tag) && (tag instanceof String)) {
            String filePath = (String) tag;
            if (MiscUtils.reverseEquals(filePath, info.mFilePath)) {
                return;
            }
        }

        // 先隐藏背景，再显示
        if (mBgView != null) {
            mBgView.clearAnimation();
            mBgView.setVisibility(View.GONE);
        }

        if (mBgMaskView != null) {
            mBgMaskView.clearAnimation();
            mBgMaskView.setVisibility(View.GONE);
        }

        // 解析并更新专辑图片
        mMusicImage.setTag(info.mFilePath);
        mMusicImage.setVisibility(View.VISIBLE);

        // 检查是否支持 ID3 信息
        if (info.mID3Type == MusicInfo.ID3_TYPE_ERROR) {
            mMusicImage.setImageResource(xId(R.drawable.default_thumbnails_bg));
        } else {
            if (HMediaConfig.SUPPORT_MUSIC_UI_BLUR_EFFECT) {
                DisplayMetrics metrics = mContext.getResources().getDisplayMetrics();
                int width = metrics.widthPixels;
                int height = metrics.heightPixels;
                int statusBarHeight = getStatusBarHeight();
                height = height - statusBarHeight;

                BitmapCache.getInstance().loadNativeImage(
                        info.mFilePath, mMusicImage, xId(R.drawable.default_thumbnails_bg),
                        mBgView, width, height, mBgMaskView, true);
            } else {
                BitmapCache.getInstance().loadNativeImage(
                        info.mFilePath, mMusicImage, xId(R.drawable.default_thumbnails_bg), true);
            }
        }
    }

    /**
     * 更新 ID3 信息
     * <p> 歌曲名、艺术家、专辑名称、专辑封面；
     *
     * @param info 当前播放信息
     */
    private void updateId3Info(MusicInfo info) {
        if (info == null) {
            return;
        }

        // 更新歌曲名称
        updateMusicTitle(info);

        // 更新艺术家
        updateMusicArtist(info);

        // 更新专辑名称
        updateMusicAlbum(info);

        // 更新专辑封面
        updateMusicImage(info);
    }

    /**
     * 更新当前歌曲名
     * @param info 当前播放信息
     */
    private void updateMusicTitle(MusicInfo info) {
        if (Objects.isNull(mTvTitle)) {
            return;
        }

        // 不支持 ID3 解析歌曲名称的时候显示文件名
        if (!Utility.supportMediaId3Title(mContext) ||
                TextUtils.isEmpty(info.mTitle) ||
                "<Unknown>".equals(info.mTitle)) {
            if (TextUtils.isEmpty(info.mFileName)) {
                mTvTitle.setText(getString(R3.string.text_unknown));
            } else {
                int pos = info.mFileName.lastIndexOf(".");
                if (pos != -1) {
                    String name = info.mFileName.substring(0, pos);
                    if (TextUtils.isEmpty(name)) {
                        mTvTitle.setText(getString(R3.string.text_unknown));
                    } else {
                        mTvTitle.setText(name);
                    }
                } else {
                    mTvTitle.setText(info.mFileName);
                }
            }
        } else {
            int pos = info.mTitle.lastIndexOf(".");
            if (pos != -1) {
                String title = info.mTitle.substring(0, pos);
                if (TextUtils.isEmpty(title)) {
                    mTvTitle.setText(getString(R3.string.text_unknown));
                } else {
                    mTvTitle.setText(title);
                }
            } else {
                mTvTitle.setText(info.mTitle);
            }
        }
    }

    /**
     * 更新当前艺术家
     * @param info 当前播放信息
     */
    private void updateMusicArtist(MusicInfo info) {
        if (Objects.isNull(mTvArtist)) {
            return;
        }

        if (TextUtils.isEmpty(info.mArtist) || "<Unknown>".equals(info.mArtist)) {
            mTvArtist.setText(getString(R3.string.text_unknown));
        } else {
            mTvArtist.setText(info.mArtist);
        }
    }

    /**
     * 更新当前专辑名
     * @param info 当前播放信息
     */
    private void updateMusicAlbum(MusicInfo info) {
        if (Objects.isNull(mTvAlbum)) {
            return;
        }

        if (TextUtils.isEmpty(info.mAlbum) || "<Unknown>".equals(info.mAlbum)) {
            mTvAlbum.setText(getString(R3.string.text_unknown));
        } else {
            mTvAlbum.setText(info.mAlbum);
        }
    }

    /**
     * 更新音乐播放信息
     * <p> ID3、歌词、播放索引等；
     */
    private void updateMusicInfo() {
        MusicInfo info = mAppData.mCurrentMediaInfo;
        if (null != info) {
            updateId3Info(info);
            updateLrcRowList(info);
        }

        changeTotalValue(
                mAppData.musicPlayPosition(),
                mAppData.musicPlaylist().size());
    }

    /**
     * 只更新文字相关信息
     * @see IMediaEvent#EVENT_UPDATE_MUSIC_ID3
     */
    private void updateMusicTextInfo() {
        // 检查索引的有效性
        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            // 只更新文字信息（专辑封面单独处理）
            MusicInfo info = mAppData.musicPlayPositionInfo();
            if (!Objects.isNull(info)) {
                updateMusicTitle(info);
                updateMusicArtist(info);
                updateMusicAlbum(info);
            }

            changeTotalValue(mAppData.musicPlayPosition(), mAppData.musicPlaylist().size());
        }
    }

    /**
     * 更新歌词显示
     * <p> UI 状态显示和歌词列表显示；
     *
     * @param info 当前播放信息
     */
    private void updateLrcRowList(MusicInfo info) {
        if (Objects.isNull(mLyricsView)) {
            return;
        }

        mFirstChanged = false;
        final int result = mLyricsManager
                .lyricsInfoInMemory(info.mFilePath);
        switch (result) {
            case LyricsManager.NO_LYRICS_FILE:
                mLyricsList.clear();
                updateLyricsInfo();
                return;
            case LyricsManager.LYRICS_IN_MEMORY:
                mLyricsList.clear();
                mLyricsList.addAll(mLyricsManager
                        .getLrcFromMemory(info.mFilePath));
                updateLyricsInfo();
                return;
            case LyricsManager.LYRICS_NOT_IN_MEMORY:
            default:
                break;
        }

        // 避免恶意的兼容性歌词文件（软件不能保证歌词文件的合法性）
        Observable.create((ObservableOnSubscribe<List<LyricsRow>>) emitter -> {
            List<LyricsRow> lyricsList =
                    mLyricsManager.getLrcFromSong(info.mFilePath);

            if (!emitter.isDisposed()) {
                if (lyricsList != null) {
                    emitter.onNext(lyricsList);
                    emitter.onComplete();
                } else {
                    emitter.onError(new Throwable("no lyrics info!"));
                }
            }
        }).subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new ObserverEx<>() {

                    @Override
                    public void onNext(@NonNull List<LyricsRow> lyricsRows) {
                        mLyricsList.clear();
                        mLyricsList.addAll(lyricsRows);

                        // 更新歌词显示信息
                        updateLyricsInfo();
                    }

                    @Override
                    public void onError(@NonNull Throwable e) {
                        super.onError(e);

                        // 更新歌词显示信息
                        mLyricsList.clear();
                        updateLyricsInfo();
                    }
                });
    }

    /** 更新歌词显示信息 **/
    private void updateLyricsInfo() {
        if (!isResumed()
                || Objects.isNull(mLyricsView)) {
            return;
        }

        if (mLyricsList.size() > 0) {
            mLyricsView.setLrcRows(mLyricsList);
            mLyricsView.setVisibility(View.VISIBLE);

            if (mTvNoLyrics != null) {
                mTvNoLyrics.setVisibility(View.GONE);
            }
        } else {
            mLyricsView.setVisibility(View.GONE);

            if (mTvNoLyrics != null) {
                mTvNoLyrics.setVisibility(View.VISIBLE);
            }
        }
    }

    /**
     * 更新播放索引信息
     *
     * @param index 当前播放索引
     * @param total 当前播放歌曲总数
     */
    private void changeTotalValue(int index, int total) {
        if (Objects.isNull(mTvTotalValue)) {
            return;
        }

        if (total > 0) {
            String text = String.format(
                    Locale.getDefault(), "%d/%d", index + 1, total);
            mTvTotalValue.setText(text);
        }
    }

    /**
     * 设置进度条使能
     * <p> 播放状态可以拖动，非播放状态禁止拖动；
     *
     * @param playState 播放状态
     */
    private void onUpdateSeekbar(int playState) {
        if (Objects.isNull(mSeekbarProgress)) {
            return;
        }

        if (playState == IMusicState.E_PLAY_STATE_PLAY) {
            mSeekbarProgress.setEnabled(true);
        } else {
            mSeekbarProgress.setEnabled(false);
        }
    }

    private void uninitVisualizer() {
        Visualizer visualizer = mMusicViewModel.getVisualizer();
        if (visualizer != null) {
            if (visualizer.getEnabled()) {
                visualizer.setEnabled(false);
            }
        }
    }

    /**
     * 更新播放进度信息
     * <p> 播放时间/总时间、进度条进度更新等；
     *
     * @param state 播放时间信息
     */
    @SuppressLint("SetTextI18n")
    private void onChangeSeekbarValue(MediaTimeInfo state) {
        if (Objects.isNull(mSeekbarProgress)) {
            return;
        }

        int nTotalTime = state.mTotalTime / 1000;
        int nCurrentTime = state.mCurrentTime / 1000;

        if (nTotalTime > 0) {
            int value = nCurrentTime * SEEKBAR_MAX_VALUE / nTotalTime;
            String totalTime;
            String currentTime;
            if (nTotalTime >= 6000) {
                totalTime = String.format(Locale.getDefault(),
                        "%03d:%02d", nTotalTime / 60, nTotalTime % 60);
                currentTime = String.format(Locale.getDefault(),
                        "%03d:%02d", nCurrentTime / 60, nCurrentTime % 60);
            } else {
                totalTime = String.format(Locale.getDefault(),
                        "%02d:%02d", nTotalTime / 60 % 100, nTotalTime % 60);
                currentTime = String.format(Locale.getDefault(),
                        "%02d:%02d", nCurrentTime / 60 % 100, nCurrentTime % 60);
            }

            if (mTvCurrentTime != null
                    && mTvTotalTime != null) {
                mTvCurrentTime.setText(currentTime);
                mTvTotalTime.setText(totalTime);
            }

            if (!mSeekbarTracking) {
                mSeekbarProgress.setProgress(value);
            }
        } else {
            if (mTvCurrentTime != null
                    && mTvTotalTime != null) {
                mTvCurrentTime.setText("00:00");
                mTvTotalTime.setText("00:00");
            }

            if (!mSeekbarTracking) {
                mSeekbarProgress.setProgress(0);
            }
        }
    }

    /**
     * 设置音频数据捕获配置
     * <p> 捕获当前播放的音频 PCM 数据，用来做傅里叶变换模拟频谱效果；
     */
    private void initVisualizer() {
        // 不在播放状态不用捕获音效数据
        if (!mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
            return;
        }

        Visualizer visualizer = mMusicViewModel.getVisualizer();
        if (visualizer != null) {
            if (visualizer.getEnabled()) {
                visualizer.setEnabled(false);
            }

            visualizer.setCaptureSize(128);// 128---1024
            visualizer.setDataCaptureListener(this,
                    Visualizer.getMaxCaptureRate() / 2, false, true);

            if (!visualizer.getEnabled()) {
                visualizer.setEnabled(true);
            }
        }
    }

    @Override
    public void onFftDataCapture(Visualizer arg0, byte[] fft, int arg2) {
        if (Objects.isNull(fft)) {
            return;
        }

        updateVisualizer(fft);
    }

    @Override
    public void onWaveFormDataCapture(Visualizer arg0, byte[] fft, int arg2) {
        if (Objects.isNull(fft)) {
            return;
        }

        updateVisualizer(fft);
    }

    @Override
    public void onSeekTo(int progress) {
        requestMediaAction(IMediaAction.seekToTime, progress);
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public boolean onTouch(View v, MotionEvent event) {
        LogUtil.e("test", "v:" + (v.getId() == xId(R.id.layout_progress)));

        // 播放页面无进度条元素
        if (Objects.isNull(mSeekbarProgress)) {
            return false;
        }

        // 是触摸进度信息布局视图
        if (v.getId() == xId(R.id.layout_progress)) {
            Rect seekRect = new Rect();
            mSeekbarProgress.getHitRect(seekRect);
            LogUtil.e("test", "bottom:" + seekRect.bottom + "  top:" + seekRect.top);

            if ((event.getY() >= (seekRect.top - 10)) && (event.getY() <= (seekRect.bottom + 10))) {
                float y = seekRect.top + seekRect.height() / 2.0f;

                // seekBar only accept relative x
                float x = event.getX() - seekRect.left;
                if (x < 0) {
                    x = 0;
                } else if (x > seekRect.width()) {
                    x = seekRect.width();
                }

                MotionEvent me = MotionEvent.obtain(event.getDownTime(),
                        event.getEventTime(), event.getAction(), x, y, event.getMetaState());
                return mSeekbarProgress.onTouchEvent(me);
            }
        }

        return false;
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.d(TAG, "onPause");
    }

    @Override
    public void onStop() {
        super.onStop();
        Log.d(TAG, "onStop");
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        Log.d(TAG, "onDestroyView");

        mInitView = false;

        // 释放提示相关资源
        HToastUtils.cancel();
        HSnackbarUtils.dismiss();
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

    public void hideWallpaperLayout() {
        // 具体业务逻辑由皮肤包去实现
        if (mPageExtend != null) {
            String result = mPageExtend.tryCallMethod("hideWallpaperLayout");
            LogUtils.vTag(TAG, "tryCallMethod/hideWallpaperLayout: " + result);
        }
    }

    /**
     * 更新播放资源，重新触发设置图标资源
     * <p> 用于白天黑夜切换 </>
     *
     * @see #onUpdateUiModeView(boolean)
     */
    protected void updatePlayerResource() {
        // 检查索引的有效性
        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            MusicInfo info = mAppData.musicPlayPositionInfo();
            if (info != null) {
                updateMusicImage(info);
            }
        }

        onChangePlayCtrl(mAppData.mMediaPlayState);
        onChangeRepeatPlayMode(mAppData.musicRepeatMode());
    }
}
