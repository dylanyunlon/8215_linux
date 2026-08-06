package com.hcn.media.music;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.LinearSnapHelper;
import androidx.recyclerview.widget.RecyclerView;
import androidx.recyclerview.widget.SimpleItemAnimator;

import com.hcn.auto_compat.app.Wallpaper;
import com.hcn.common.lang.Listenable;
import com.hcn.common.misc.LogUtils;
import com.hcn.common.utils.HDeviceUtils;
import com.hcn.common.utils.HImageUtils;
import com.hcn.common.utils.HMessage;
import com.hcn.common.utils.HMessageUtils;
import com.hcn.common.utils.HSizeUtils;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.common.widget.HViewUtils;
import com.hcn.media.adapter.base.IRvDecoration;
import com.hcn.media.adapter.RvWallpaperAdapter;
import com.hcn.media.adapter.base.IRvListener;
import com.hcn.media.adapter.base.WallpaperItemDecoration;
import com.hcn.media.extend.PageExtend;
import com.hcn.media_common.utils.ViewUtilsEx;
import com.hcn.media_view.recyclerview.TopLinearLayoutManger;
import com.hcn.media_view.recyclerview.TopLinearSnapHelper;
import com.hcn.media_view.resx.IR;
import com.hcn.skinx.R3;
import com.hcn.skinx.SkinX;
import com.hcn.skinx.base.ISkinExFragment;

import java.util.List;
import java.util.Objects;

/**
 * 音乐信息页扩展
 * <pre>
 *    1、类名不可以修改，这个是插件的规则；
 *    2、主要用来对 MusicInfoFragment 页面操作扩展；
 *    3、最终由 MusicInfoFragment 实例化并调用；
 * </pre>
 *
 * @author 65821
 */
public class MusicInfoPageExtend extends PageExtend {

    private static final String TAG = MusicInfoPageExtend.class.getSimpleName();

    /**
     * 界面背景视图
     * <pre>
     *    用来设置选择的壁纸背景图片；
     *    backgroundView: main_bg [Level:0]
     * </pre>
     */
    private View mBackgroudView = null;

    /**
     * 专辑封面视图
     * <pre>
     *    用来设置选择的壁纸背景时候隐藏（它会遮挡壁纸）；
     *    AlbumCoverView: bg [Level:1]
     * </pre>
     */
    private View mAlbumCoverView = null;

    /**
     * 专辑封面掩图
     * <pre>
     *    无专辑封面的时候，它也会被隐藏；
     *    AlbumCoverMaskView: bg_mask [Level:2]
     * </pre>
     */
    private View mBackgroundMaskView = null;

    /**
     * 壁纸旋转器布局
     * <p> 部分主体支持壁纸选择器，用来选择设置当前模块背景；
     */
    private View mWallpaperSelectorLayout = null;

    /**
     * 控制按钮（统一命名）
     * <p> ImageView/ivWallpaper：壁纸预览；
     */
    private View mIvWallpaper = null;

    /**
     * 壁纸预览列表
     * <pre>
     *    壁纸内容去 HMedia 的服务拿，这里只是显示；
     *    e.g. 初步设计路径在 apd/appWallpaper 目录下；
     * </pre>
     */
    private RecyclerView mWallpaperRecyclerView = null;
    private RvWallpaperAdapter mRvAdapter = null;
    private LinearLayoutManager mLayoutManager;
    private LinearSnapHelper mLinearSnapHelper;

    /**
     * 主线程消息处理器封装
     * <p> 仅在当前类有效，禁止外溢使用；
     */
    protected HMessageUtils MSG = null;

    /** 内部消息定义 */
    protected interface H {
        int MSG_NONE = -1;

        // 显示专辑封面图片
        int MSG_SHOW_ALBUMCOVER_VIEW = 1;

        // 延迟隐藏壁纸预览
        int MSG_HIDE_WALLPAPER_LIST_LAYOUT = 2;
    }

    /**
     * 壁纸状态监听者
     * <pre>
     *    监听检查是否有壁纸文件
     *    由 {@link com.hcn.auto_compat.app.Wallpaper} 管理；
     * </pre>
     */
    private final Listenable<String> mWallpaperListener = (s, o) -> {
        // 壁纸状态检查
        if (Objects.equals(s, Wallpaper.ST_COMPLETED)) {
            // 壁纸状态检查完成，加载壁纸
            List<Wallpaper.Info> list = Wallpaper.instance().getInfo();
            LogUtils.vTag(TAG,
                    "Wallpaper.ST_COMPLETED: "
                            + (list != null? list.size(): "null"));

            // 更新壁纸 RecyclerView 列表
            updateWallpaperRecyclerView(false);
            return;
        }

        // 壁纸设置回调
        if (Objects.equals(s, Wallpaper.ET_SAVE_PATH)) {
            // 隐藏专辑封面背景（它会遮挡壁纸）
            if (Objects.isNull(mAlbumCoverView)) {
                return;
            }

            // 专辑封面是显示的，需要先隐藏让用户看到壁纸效果
            if (ViewUtilsEx.isVisible(mAlbumCoverView, View.VISIBLE)) {
                mAlbumCoverView.clearAnimation();
                mAlbumCoverView.setVisibility(View.GONE);

                if (mBackgroundMaskView != null) {
                    mBackgroundMaskView.clearAnimation();
                    mBackgroundMaskView.setVisibility(View.GONE);
                }

                MSG.sendUnique(H.MSG_SHOW_ALBUMCOVER_VIEW, 5000);
            }
        }
    };

    /**
     * 音乐页面扩展构造函数
     * @param fragment 页面所有者
     */
    public MusicInfoPageExtend(ISkinExFragment fragment) {
        super(fragment);

        // 构造一个消息处理工具对象
        MSG = new HMessageUtils.Builder()
                .setName(TAG)
                .setSupportGlobalUsage(false)
                .build();
    }

    @Override
    public void onAttach(@NonNull Context context) {
        super.onAttach(context);
    }

    @Override
    public void onCreate(@Nullable Bundle bundle) {
        // 构建 RecyclerView 适配器
        mRvAdapter = new RvWallpaperAdapter(requireContext(),
                Wallpaper.instance().getInfo(), new IRvListener() {

            /**
             * 壁纸预览点击事件
             *
             * @param info     被点击的壁纸选项信息
             * @param position 选项在列表中的位置信息
             */
            @Override
            public void onItemClick(Wallpaper.Info info, int position) {
                // 保存壁纸路径
                if (Objects.isNull(info) || Objects.isNull(mBackgroudView)) {
                    return;
                }

                // 设置页面背景图片
                String path = info.wallpaperPath;
                if (TextUtils.isEmpty(path)) {
                    return;
                }

                Bitmap bitmap = HImageUtils.getBitmap(path);
                if (!Objects.isNull(bitmap)) {
                    // 解码并设置背景图片
                    Drawable newBitmapDrawable = new BitmapDrawable(
                            requireContext().getResources(), bitmap);
                    mBackgroudView.setBackground(newBitmapDrawable);
                    Wallpaper.instance().saveWallpaperPath(path);
                }

                // 重置隐藏壁纸预览列表布局消息
                MSG.sendUnique(
                        H.MSG_HIDE_WALLPAPER_LIST_LAYOUT, 6000);
            }

            @Override
            public void onRvAdapterEvent(@NonNull String event, Object obj1, Object obj2) {
            }
        });

        // 构建消息处理器
        MSG.addListener(H.MSG_SHOW_ALBUMCOVER_VIEW, uiMessage -> {
            Animation fadeoutAnim = AnimationUtils
                    .loadAnimation(requireContext(), IR.Anim.fadein);

            // 显示专辑封面背景（它会遮挡壁纸）
            if (mAlbumCoverView != null) {
                if (fadeoutAnim != null) {
                    mAlbumCoverView.startAnimation(fadeoutAnim);
                }

                mAlbumCoverView.setVisibility(View.VISIBLE);
            }

            // 显示专辑封面遮罩（它会模糊壁纸）
            if (mBackgroundMaskView != null) {
                if (fadeoutAnim != null) {
                    mBackgroundMaskView.startAnimation(fadeoutAnim);
                }

                mBackgroundMaskView.setVisibility(View.VISIBLE);
            }
        });

        // 隐藏壁纸预览列表布局
        MSG.addListener(H.MSG_HIDE_WALLPAPER_LIST_LAYOUT, uiMessage -> {
            if (mWallpaperSelectorLayout != null) {
                mWallpaperSelectorLayout.setVisibility(View.GONE);
            }
        });
    }

    @Override
    public View onCreateView(LayoutInflater inflater,
                             @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        // 当前主题调试的时候才显示
        if (SkinX.getBoolean("support_wallpaper_customized")) {
            // 注册壁纸状态监听者
            Wallpaper.instance().register(mWallpaperListener);
        }

        // 不创建视图（保留设计）
        return super.onCreateView(inflater, container, savedInstanceState);
    }

    /**
     * RecyclerView 滚动状态监听器
     * <pre>
     *    0: RecyclerView.SCROLL_STATE_IDLE 空闲状态，停止滚动
     *    1: RecyclerView.SCROLL_STATE_DRAGGING 手指滑动状态
     *    2: RecyclerView.SCROLL_STATE_SETTLING 松开手指，惯性滚动状态
     * </pre>
     */
    private final RecyclerView.OnScrollListener mRecyclerViewScrollLListener =
            new RecyclerView.OnScrollListener() {
                /** RecyclerView 状态描述符 **/
                private String stateName(int state) {
                    switch (state) {
                        case RecyclerView.SCROLL_STATE_DRAGGING:
                            return "SCROLL_STATE_DRAGGING";
                        case RecyclerView.SCROLL_STATE_SETTLING:
                            return "SCROLL_STATE_SETTLING";
                        case RecyclerView.SCROLL_STATE_IDLE:
                        default:
                            break;
                    }
                    return "SCROLL_STATE_IDLE";
                }

                @Override
                public void onScrollStateChanged(@NonNull RecyclerView recyclerView,
                                                 int newState) {
                    super.onScrollStateChanged(recyclerView, newState);
                    LogUtils.vTag(TAG, "onScrollStateChanged: " + stateName(newState));

                    // 停止滚动后，我们可以检查是否对齐了
                    if (RecyclerView.SCROLL_STATE_IDLE == newState) {
                        int firstVisibleItemPos =
                                mLayoutManager.findFirstVisibleItemPosition();
                        int firstCompletelyVisiblePos =
                                mLayoutManager.findFirstCompletelyVisibleItemPosition();
                        LogUtils.vTag(TAG, "First visibleI item position: "
                                + firstVisibleItemPos + " | " + firstCompletelyVisiblePos);

                        // 第一个可见的 Item 与 第一个完全可一件的 Item 不一致（未对齐）
                        if (firstVisibleItemPos != firstCompletelyVisiblePos) {
                            View firstVisibleItem =
                                    mLayoutManager.findViewByPosition(firstVisibleItemPos);
                            assert firstVisibleItem != null;
                            int width = firstVisibleItem.getWidth();
                            int xCoordinate = firstVisibleItem.getLeft();

                            // Item 显示不过半与过半处理
                            int targetPosition = firstVisibleItemPos;
                            if (Math.abs(xCoordinate) > width >> 1) {
                                targetPosition = firstCompletelyVisiblePos;
                            }

                            mLayoutManager.smoothScrollToPosition(
                                    mWallpaperRecyclerView, null, targetPosition);
                        } else {
                            // 滚动停止，隐藏壁纸预览列表布局
                            MSG.sendUnique(H.MSG_HIDE_WALLPAPER_LIST_LAYOUT, 6000);
                        }
                    } else {
                        // 滚动中，取消隐藏壁纸预览列表布局
                        MSG.remove(H.MSG_HIDE_WALLPAPER_LIST_LAYOUT);
                    }
                }

                @Override
                public void onScrolled(@NonNull RecyclerView recyclerView,
                                       int dx,
                                       int dy) {
                    super.onScrolled(mWallpaperRecyclerView, dx, dy);
                }
            };

    /**
     * RecyclerView 视图配置
     * <pre>
     *    1、配置列表 Item 的缓存个数；
     *    2、配置可回收视图的布局管理器；
     *    3、配置列表视图触摸不获取焦点；
     *    n、...
     * </pre>
     */
    protected void configRecyclerViewAttribute() {
        if (Objects.isNull(mWallpaperRecyclerView)) {
            return;
        }

        // ViewHolders 缓存个数
        RecyclerView.RecycledViewPool pool = new RecyclerView.RecycledViewPool();
        pool.setMaxRecycledViews(
                IRvDecoration.RV_WALLPAPER_ITEM_TYPE,
                IRvDecoration.LIST_MAX_RECYCLED_VIEWS);
        mWallpaperRecyclerView.setRecycledViewPool(pool);

        // 配置可回收视图的布局管理器
        mLayoutManager = new TopLinearLayoutManger(
                requireContext(), RecyclerView.HORIZONTAL, false);
        mWallpaperRecyclerView.setLayoutManager(mLayoutManager);
        int screenWidthDp = requireContext().getResources().getConfiguration().screenWidthDp;
        int screenWidthPx = HSizeUtils.dp2px(screenWidthDp);
        mWallpaperRecyclerView.addItemDecoration(
                new WallpaperItemDecoration(screenWidthPx < 1280? 30: 36));

        // RecyclerView 触摸不获取焦点
        mWallpaperRecyclerView.setFocusableInTouchMode(false);

        // 关闭 Item 的动画，避免刷新闪烁
        RecyclerView.ItemAnimator animator = mWallpaperRecyclerView.getItemAnimator();
        if (animator instanceof SimpleItemAnimator) {
            animator.setChangeDuration(0);
            ((SimpleItemAnimator) animator).setSupportsChangeAnimations(false);
        }

        // 添加 RecyclerView 滚动（Scroll）监听器（调试使用）
        mWallpaperRecyclerView.addOnScrollListener(mRecyclerViewScrollLListener);

        // 配置滑动顶端对齐线性捕获辅助器
        mLinearSnapHelper = new TopLinearSnapHelper();
        mLinearSnapHelper.attachToRecyclerView(mWallpaperRecyclerView);

        // 调试使用，不使用捕获辅助器的时候打开
        if (Objects.isNull(mLinearSnapHelper)) {
            // 添加 RecyclerView 抛投（Fling）监听器
            mWallpaperRecyclerView.setOnFlingListener(new RecyclerView.OnFlingListener() {
                /**
                 * 处理抛投事件
                 * <p> 只有 MotionEvent.ACTION_UP 后才会触发这个事件；
                 *
                 * @param velocityX 横向速度
                 * @param velocityY 纵向速度
                 * @return 是/否处理过了
                 */
                @Override
                public boolean onFling(int velocityX, int velocityY) {
                    LogUtils.vTag(TAG, "onFling: " + velocityX + ", " + velocityY);
                    return false;
                }
            });
        }
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle bundle) {
        super.onViewCreated(view, bundle);

        // 背景视图（自定义壁纸设置）
        ISkinExFragment fragment = mFragmentRef.get();
        if (fragment != null) {
            Activity activity = fragment.xRequireActivity();
            mBackgroudView = activity.findViewById(SkinX.xId("main_bg"));
        }

        // 专辑封面背景（它会遮挡壁纸）
        mAlbumCoverView = view.findViewById(R3.id.bg);
        mBackgroundMaskView = view.findViewById(R3.id.bg_mask);

        // 扩展的按钮图标/壁纸预览
        mIvWallpaper = view.findViewById(R3.id.ivWallpaper);
        if (mIvWallpaper != null) {
            // 壁纸预览显示布局隐藏处理
            mIvWallpaper.setOnClickListener(v -> {
                if (Objects.isNull(mWallpaperSelectorLayout)) {
                    return;
                }

                // 壁纸预览列表显示控制
                int visibility = mWallpaperSelectorLayout.getVisibility();
                if (visibility != View.VISIBLE) {
                    MSG.sendUnique(H.MSG_HIDE_WALLPAPER_LIST_LAYOUT, 6000);
                } else {
                    MSG.remove(H.MSG_HIDE_WALLPAPER_LIST_LAYOUT);
                }

                mWallpaperSelectorLayout.setVisibility(
                        visibility != View.VISIBLE? View.VISIBLE: View.GONE);
            });

            // 横屏设备竖屏显示状态处理
            ISkinExFragment f = mFragmentRef.get();
            Object result = f.requestExecuteMethod("isHorizontalDevicePortraitShow");
            if (result instanceof Boolean) {
                boolean isHorizontalDevicePortraitShow = (boolean) result;
                if (isHorizontalDevicePortraitShow) {
                    mIvWallpaper.setVisibility(View.GONE);
                }
            }
        }

        // 壁纸显示布局（预览使用）
        mWallpaperSelectorLayout = view.findViewById(R3.id.ll_wallpaper);
        if (mWallpaperSelectorLayout != null) {
            mWallpaperSelectorLayout.setVisibility(View.GONE);

            // 监听当前视图点击状态
            mWallpaperSelectorLayout.setOnClickListener(v -> {
                if (ViewUtilsEx.isVisible(mWallpaperSelectorLayout, View.VISIBLE)) {
                    mWallpaperSelectorLayout.setVisibility(View.GONE);
                }
            });
        }

        // Wallpaper RecyclerView 壁纸预览列表
        mWallpaperRecyclerView = view.findViewById(R3.id.wallpaper_recyclerview);

        // 配置 RecyclerView 视图
        configRecyclerViewAttribute();

        // 更新 RecyclerView 数据
        updateWallpaperRecyclerView(false);
    }

    /**
     * 更新壁纸 RecyclerView
     * @param forceUpdate 是否强制更新
     */
    @SuppressLint("NotifyDataSetChanged")
    private void updateWallpaperRecyclerView(boolean forceUpdate) {
        if (Objects.isNull(mWallpaperRecyclerView)) {
            return;
        }

        // 获取壁纸数据
        List<Wallpaper.Info> list = Wallpaper.instance().getInfo();
        boolean hideWallpaperIcon = Objects.isNull(list) || list.isEmpty();

        // 如果没有壁纸数据
        if (mIvWallpaper != null) {
            mIvWallpaper.setVisibility(hideWallpaperIcon? View.GONE: View.VISIBLE);
        }

        // 更新数据到适配器
        mRvAdapter.updateList(list);
        mWallpaperRecyclerView.setAdapter(mRvAdapter);

        if (forceUpdate) {
            mRvAdapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        mRvAdapter.onResume();
    }

    /**
     * 宿主调用扩展类方法入口
     *
     * @param method 方法类型
     * @param args 参数集
     * @return 结果
     * @throws IllegalArgumentException
     */
    @Override
    public String tryCallMethod(String method, Object... args) throws IllegalArgumentException {
        switch (method) {
            case "onHandlePageEvent":
                return onHandlePageEvent(args);
            case "aJustMultiWindowMode":
                return aJustMultiWindowMode(args);
            case "none":
            default:
                break;
        }

        return super.tryCallMethod(method, args);
    }

    /**
     * 处理媒体页面事件
     * <pre>
     *    扩展页面事件处理，由宿主触发调用到扩展页面；
     *    e.g. 参考 {@link com.hcn.media_base.IMediaEvent} 定义；
     * </pre>
     *
     * @param args 事件参数
     * @return  返回参数（扩展用，也可以返回 null）
     */
    protected String onHandlePageEvent(Object... args) {
        // 参数个数检查
        if (Objects.isNull(args)
                || args.length != 3) {
            throw new IllegalArgumentException("onHandlePageEvent.");
        }
        return super.onHandlePageEvent(args);
    }

    /**
     * 调整多窗口模式显示
     * <p> 适配多窗口模式下的 UI 显示（测试用）；
     *
     * @param args 参数集
     * @return 当前函数执行结果
     */
    private String aJustMultiWindowMode(Object... args) {
        // 参数个数检查
        if (Objects.isNull(args)
                || args.length != 1) {
            throw new IllegalArgumentException("aJustMultiWindowMode.");
        }
        return "false";
    }

    @Override
    public void onPause() {
        super.onPause();
        mRvAdapter.onPause();

        // 隐藏壁纸菜单显示
        if (mWallpaperSelectorLayout != null) {
            mWallpaperSelectorLayout.setVisibility(View.GONE);
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();

        // 注销壁纸状态监听者
        Wallpaper.instance().unregister(mWallpaperListener);
    }

    @Override
    public void onDestroy() {
        // 释放消息处理器
        MSG.clearMessageQueue();
        MSG.release();

        if (mRvAdapter != null) {
            mRvAdapter.onDestroy();
            mRvAdapter = null;
        }
    }
}