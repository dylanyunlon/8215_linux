package com.hcn.media.music.common.simple;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.LinearSnapHelper;
import androidx.recyclerview.widget.RecyclerView;
import androidx.recyclerview.widget.SimpleItemAnimator;
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media.R3;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_base.fragment.PageEvent;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media.adapter.BaseRvAdapter;
import com.hcn.media.adapter.simple.IRvDecoration;
import com.hcn.media.adapter.simple.ISimpleRvListener;
import com.hcn.media.adapter.simple.SimpleRvAdapter;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_data.FavoriteManager;
import com.hcn.media_data.storage.IStorageDevice;
import com.hcn.media_view.recyclerview.RecyclerViewUtils;
import com.hcn.media_view.recyclerview.TopLinearLayoutManger;
import com.hcn.media_view.recyclerview.TopLinearSnapHelper;
import com.hcn.mediaservice.data.MusicInfo;
import com.orhanobut.logger.Logger;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * 音乐简单列表页面
 * @author 65821
 */
public class SimpleListFragment extends MediaFragment {
    private static final String FRAGMENT_NAME = "music-simple-list";
    private static final String TAG = SimpleListFragment.class.getSimpleName();

    /**
     * 約束当前 Simple 列表页面类型
     * <p> 当前页面是为播放列表/收藏列表设计的，暂不可以乱用到其它需求；
     */
    @ISimpleList
    protected int mPageType = ISimpleList.PAGE_PLAYLIST;

    /**
     * 当前页面的根布局节点
     * <p> 用来调色页面背景颜色；
     */
    private View mSimpleListPage;

    /**
     * RecyclerView 的父节点
     * <pre>
     *    用来处理列表下拉操作；
     *    e.g. 下拉刷新当前播放列表；
     * </pre>
     */
    private SwipeRefreshLayout mSimplePullDownLayout;

    /**
     * 当前页面列表视图/适配器/布局管理器
     * <pre>
     *    RecyclerView 可以很好的管控内存资源；
     *    它需要结合 RecyclerView.Adapter<RecyclerView.ViewHolder> 适配器一起使用；
     * </pre>
     */
    protected RecyclerView mRecyclerView;
    protected SimpleRvAdapter mRvAdapter;
    protected LinearLayoutManager mLayoutManager;
    protected LinearSnapHelper mLinearSnapHelper;

    /**
     * 当前页面提示信息布局与标签
     * <pre>
     *    当无列表内容的时候提示 “没有歌曲信息”；
     *    当刷新列表或者插拔外部存储设备时提示 "正在更新数据";
     *    或其他，可扩展...
     * </pre>
     */
    private View mSimplePromptLayout;
    private TextView mTvSimplePromptLabel;

    /**
     * 当前页面的数据
     * <pre>
     *    这里让 Page 单独管理一份数据列表；
     *    有需要的时候，才去从外部更新数据到本地列表；
     *    好处：避免不必要的数据越界和异常；
     * </pre>
     */
    public final DataObject mDataObject = new DataObject();

    /**
     * 比较 MusicInfo 内容是否相等
     * <p> 注意这里为了效率只比较 2 个对象的文件名称；
     *
     * @param info1 媒体对象 1
     * @param info2 媒体对象 2
     * @return 相等/不相等
     */
    private static boolean compareMusicInfo(MusicInfo info1, MusicInfo info2) {
        // 数据必须是有效的
        if (Objects.isNull(info1)
                || Objects.isNull(info2)
                || TextUtils.isEmpty(info1.mFileName)
                || TextUtils.isEmpty(info2.mFileName)) {
            return false;
        }

        // 只比较文件名即可
        return info1.mFileName.equals(info2.mFileName);
    }

    /**
     * 页面数据对象
     * <pre>
     *    当前数据列表和当前数据所在存储设备类型；
     *    注意：{@link ISimpleList#PAGE_FAVORITE} 下存储设备类型无意义；
     * </pre>
     */
    private static final class DataObject implements Serializable {
        public int storageDeviceType = IStorageDevice.STORAGE_TYPE_NONE;
        public List<MusicInfo> list = new ArrayList<>();

        /** 判断是否存在有效数据 **/
        public boolean isEmpty() {
            return list.isEmpty();
        }

        /**
         * 检查数据是否匹配
         * <pre>
         *    据如果不匹配，就需要刷新替换数据对象；
         *    由于前期媒体状态事件设计不合理，所以这里需要强制匹配检查；
         *    待后续重写媒体状态事件后(细致化)，可解决需要匹配的问题；
         * </pre>
         *
         * @param deviceType 存储类型
         * @param list 数据列表
         * @return 匹配/不匹配
         */
        public boolean isMatched(int deviceType, @NonNull List<MusicInfo> list) {
            // 存储类型不匹配
            if (storageDeviceType != deviceType) {
                return false;
            }

            // 数据大小不一样
            if (list.isEmpty()
                    || this.list.size() != list.size()) {
                return false;
            }

            // 严谨起见，我们随机抽取一定的值比较
            if (list.size() <= 5) {
                for (int i = 0; i < list.size(); i++) {
                    // 比较文件是否相等
                    if (!compareMusicInfo(list.get(i), this.list.get(i))) {
                        return false;
                    }
                }
            } else {
                int size = list.size();
                int half = size/2;
                int interval = size/5;

                // 从前往后抽查比较
                for (int i = 0; i <= half; i = i + interval) {
                    // 比较文件是否相等
                    if (!compareMusicInfo(list.get(i), this.list.get(i))) {
                        return false;
                    }
                }

                // 从后往前抽查比较
                for (int i = size-1; i > half; i = i - interval) {
                    // 比较文件是否相等
                    if (!compareMusicInfo(list.get(i), this.list.get(i))) {
                        return false;
                    }
                }
            }

            // 校验磁盘路径（避免漏掉 U 盘口交换场景）
            MusicInfo info = this.list.get(0);
            if (!Objects.isNull(info)) {
                // 这个接口是比较文件路径的（这里我们暂时就比较一个，后续有特定场景再调整）
                return info.compareTo(list.get(0)) == 0;
            }

            return true;
        }

        /**
         * 有效数据索引
         * <p> 检查指定位置索引在数据列表是否有效;
         *
         * @param index 数据索引
         * @return 有效/无效
         */
        public boolean validDataIndex(int index) {
            return !list.isEmpty() && index < list.size();
        }

        /**
         * 替换列表数据
         * <p> 先清除所有数据，再增加指定数据列表；
         *
         * @param list 数据列表
         */
        public void replaceListInfo(@NonNull List<MusicInfo> list) {
            this.list.clear();

            if (!list.isEmpty()) {
                this.list.addAll(list);
            }
        }

        /**
         * 更新目标数据
         * <p> 注意：{@link ISimpleList#PAGE_FAVORITE} 没有存储设备概念；
         *
         * @param deviceType 数据所在存储设备类型
         * @param list 数据列表
         */
        public void updateDataInfo(int deviceType, @NonNull List<MusicInfo> list) {
            storageDeviceType = deviceType;
            replaceListInfo(list);
        }
    }

    /**
     * 构建 SimpleListFragment 实例
     *
     * @param page 页面类型
     * @return 实例对象
     */
    public static SimpleListFragment newInstance(int page) {
        Bundle args = new Bundle();
        args.putInt(PAGE_PARAM_KEY, page);
        SimpleListFragment fragment = new SimpleListFragment();
        fragment.setArguments(args);
        return fragment;
    }

    public SimpleListFragment() {
        super(FRAGMENT_NAME);
    }

    /**
     * 获取当前页面类型
     * @return {@link ISimpleList}
     */
    @ISimpleList
    public int pageType() {
        return mPageType;
    }

    /**
     * 判断当前页面类型是否是指定的页面类型
     * @param type {@link ISimpleList}
     * @return 是/否
     */
    public boolean isPageType(@ISimpleList int type) {
        return mPageType == type;
    }

    /**
     * 页面类型字符名称
     * @return {@link String}
     */
    @SuppressLint("SwitchIntDef")
    protected String pageTypeName() {
        switch (mPageType) {
            case ISimpleList.PAGE_PLAYLIST:
                return "page-playlist";
            case ISimpleList.PAGE_FAVORITE:
                return "page-favorite";
            default:
                break;
        }
        return "none";
    }

    @Override
    public void onAttach(@NonNull Context context) {
        super.onAttach(context);

        // 构建 RecyclerView 适配器
        mRvAdapter = new SimpleRvAdapter(
                requireContext(), null, new ISimpleRvListener() {

            @SuppressLint("SwitchIntDef")
            @Override
            public void onItemClick(MusicInfo info, int position) {
                Logger.t(TAG).v("SimpleList/onItemClick: " + info.mFileName);

                if (!mRvAdapter.isResume()) {
                    return;
                }

                // TODO: 处理点击事件，播放点击选项；
                if (mDataObject.validDataIndex(position)) {
                    // 从页面类型获取当前播放列表类型
                    @IPlaylistType int listType;
                    switch (pageType()) {
                        case ISimpleList.PAGE_FAVORITE:
                            listType = IPlaylistType.FAVORITE_LIST;
                            break;
                        case ISimpleList.PAGE_PLAYLIST:
                        default:
                            listType = IPlaylistType.DEVICE_LIST;
                            break;
                    }

                    // 请求播放目标列表的指定媒体对象
                    mMusicViewModel.playerRelay().accept(
                            t -> t.requestPlayTarget(listType, mDataObject.list, position));
                }
            }

            @Override
            public void onItemLongClick(MusicInfo info, int position) {
                // TODO: 扩展保留
            }

            @Override
            public void onItemDelete(MusicInfo info, int position) {
                // TODO: 扩展保留
            }

            @Override
            public void onRvAdapterEvent(
                    @NonNull final String event, Object obj1, Object obj2) {
                switch (event) {
                    case "list-size":
                        // 适配器数据列表改变
                        if (obj1 instanceof Integer) {
                            int size = (int) obj1;
                            setSimplePromptLayout(size > 0? View.INVISIBLE: View.VISIBLE);
                        }
                        break;
                    case "test-none":
                    default:
                        break;
                }
            }
        }, pageType(), BaseRvAdapter.FileType.FILE_MUSIC);
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // 当前页面类型
        if (getArguments() != null) {
            mPageType = getArguments().getInt(PAGE_PARAM_KEY);
        }

        // 列表类型需要同步更新给适配器
        mRvAdapter.setListType(pageType());
        Logger.t(TAG).v("onCreate, page = " + pageTypeName());
    }

    @Override
    public int getLayoutRes() {
        return R.layout.layout_simple_list_page;
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater,
                             @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        LogUtil.v(TAG, "onCreateView");

        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);
        assert view != null;
        return view;
    }

    @Override
    protected void onInitializeElements(@Nullable Bundle savedInstanceState) {
        super.onInitializeElements(savedInstanceState);

        // 初始化当前页面布局根节点视图
        mSimpleListPage = findViewByName("simpleListPage");
        if (mSimpleListPage != null) {
            mSimpleListPage.setBackgroundColor(0x0C4C4C4C);
        }

        // 初始化下拉列表触发刷新布局
        mSimplePullDownLayout =
                (SwipeRefreshLayout) findViewByName("simpleSwipeRefreshLayout");
        if (mSimplePullDownLayout != null) {
            // 初始化 RecyclerView 视图
            mRecyclerView = (RecyclerView) findViewByName("simpleRecyclerview");
        }

        // 初始化提示信息相关的视图内容
        mSimplePromptLayout = findViewByName("simplePromptLayout");
        mTvSimplePromptLabel = (TextView) findViewByName("tvSimplePrompt");
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        // 配置 SwipeRefreshLayout 视图
        configSimplePullDownLayout();

        // 配置 RecyclerView 视图
        configRecyclerViewAttribute();

        // 同步数据到 RecyclerView 适配器
        syncPlayInfo2RvAdapter(false);
        syncMediaList2RvAdapter(false);

        // 更新 SimpleList 上的显示元素
        updateSimpleListElement();
    }

    /**
     * 配置下拉布局视图
     * <pre>
     *    加载元素显示配置；
     *    下拉动作与状态监听；
     * </pre>
     */
    private void configSimplePullDownLayout() {
        if (Objects.isNull(mSimplePullDownLayout)) {
            return;
        }

        // 设置刷新进度
        mSimplePullDownLayout.setSize(SwipeRefreshLayout.LARGE);
        mSimplePullDownLayout.setColorSchemeColors(
                0x7F03DAC5, 0x7F4EFFE9, 0x7F0090FF, 0x7F6200EE);
        mSimplePullDownLayout.setProgressBackgroundColorSchemeColor(Color.GRAY);
        mSimplePullDownLayout.setProgressViewEndTarget(
                true, mSimplePullDownLayout.getProgressViewEndOffset());

        // 监听刷新动作
        mSimplePullDownLayout.setOnRefreshListener(() -> {
            LogUtil.v(TAG, "SwipeRefreshLayout, onRefresh.");

            // 默认暂时只做一个 UI 效果
            H0.postUniqueDelayed(
                    mStopSimpleRefreshRunnable, 1500);
        });
    }

    /**
     * 停止 SwipeRefreshLayout 的刷新效果
     * <p> 更新完成或者超时停止刷新动画；
     */
    private final Runnable mStopSimpleRefreshRunnable = new Runnable() {
        @Override
        public void run() {
            if (Objects.isNull(mSimplePullDownLayout)) {
                return;
            }

            mSimplePullDownLayout.setRefreshing(false);
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
        if (Objects.isNull(mRecyclerView)) {
            return;
        }

        // ViewHolders 缓存个数
        RecyclerView.RecycledViewPool pool = new RecyclerView.RecycledViewPool();
        pool.setMaxRecycledViews(
                IRvDecoration.SIMPLE_RV_ITEM_LIST_TYPE,
                IRvDecoration.SIMPLE_LIST_MAX_RECYCLED_VIEWS);
        mRecyclerView.setRecycledViewPool(pool);

        // 配置可回收视图的布局管理器
        mLayoutManager = new TopLinearLayoutManger(
                requireContext(), RecyclerView.VERTICAL, false);
        mRecyclerView.setLayoutManager(mLayoutManager);
        mRecyclerView.addItemDecoration(new SimpleItemDecoration(1));

        // RecyclerView 触摸不获取焦点
        mRecyclerView.setFocusableInTouchMode(false);

        // 关闭 Item 的动画，避免刷新闪烁
        RecyclerView.ItemAnimator animator = mRecyclerView.getItemAnimator();
        if (animator instanceof SimpleItemAnimator) {
            animator.setChangeDuration(0);
            ((SimpleItemAnimator) animator).setSupportsChangeAnimations(false);
        }

        // 添加 RecyclerView 滚动（Scroll）监听器（调试使用）
        mRecyclerView.addOnScrollListener(mRecyclerViewScrollLListener);

        // 配置滑动顶端对齐线性捕获辅助器
        mLinearSnapHelper = new TopLinearSnapHelper();
        // 已经在滚动监听器里做了对齐策略，不需要再额外配置
        // mLinearSnapHelper.attachToRecyclerView(mRecyclerView);

        // 调试使用，不使用捕获辅助器的时候打开
        if (Objects.isNull(mLinearSnapHelper)) {
            // 添加 RecyclerView 抛投（Fling）监听器
            mRecyclerView.setOnFlingListener(new RecyclerView.OnFlingListener() {
                /**
                 * 处理抛投事件
                 * <p> 只有 MotionEvent.ACTION_UP 后才会触发这个事件；
                 *
                 * @param velocityX 横向速度
                 * @param velocityY 纵向速度
                 * @return
                 */
                @Override
                public boolean onFling(int velocityX, int velocityY) {
                    LogUtil.v(TAG, "onFling: " + velocityX + ", " + velocityY);
                    return false;
                }
            });
        }
    }

    /**
     * RecyclerView 滚动状态监听器
     * <pre>
     *    0: RecyclerView.SCROLL_STATE_IDLE 空闲状态，停止滚动
     *    1: RecyclerView.SCROLL_STATE_DRAGGING 手指滑动状态
     *    2: RecyclerView.SCROLL_STATE_SETTLING 松开手指，惯性滚动状态
     * </pre>
     */
    private RecyclerView.OnScrollListener mRecyclerViewScrollLListener =
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
            LogUtil.v(TAG, "onScrollStateChanged: " + stateName(newState));

            // 停止滚动后，我们可以检查是否对齐了
            if (RecyclerView.SCROLL_STATE_IDLE == newState) {
                int firstVisibleItemPos =
                        mLayoutManager.findFirstVisibleItemPosition();
                int firstCompletelyVisiblePos =
                        mLayoutManager.findFirstCompletelyVisibleItemPosition();
                int lastVisibleItemPos =
                        mLayoutManager.findLastVisibleItemPosition();
                int lastCompletelyVisiblePos =
                        mLayoutManager.findLastCompletelyVisibleItemPosition();
                int lastPosition =
                        mLayoutManager.getItemCount() - 1;

                // 确认滑动到了底部,则对齐最后一个
                if (lastPosition == lastVisibleItemPos){
                    // 判断最后一个可见的 Item 与 最后一个完全可见的 Item 不一致（未对齐）
                    if (lastVisibleItemPos != lastCompletelyVisiblePos){
                        mLayoutManager.smoothScrollToPosition(mRecyclerView, null, lastVisibleItemPos);
                    }
                } else {
                    // 未滑动到底部，则对齐第一个可见的，判断第一个可见的 Item 与 第一个完全可见的 Item 不一致（未对齐）
                    if (firstVisibleItemPos != firstCompletelyVisiblePos) {
                        View firstVisibleItem =
                                mLayoutManager.findViewByPosition(firstVisibleItemPos);
                        assert firstVisibleItem != null;
                        int height = firstVisibleItem.getHeight();
                        int yCoordinate = firstVisibleItem.getTop();

                        // Item 显示不过半与过半处理
                        int targetPosition = firstVisibleItemPos;
                        if (Math.abs(yCoordinate) > height >> 1) {
                            targetPosition = firstCompletelyVisiblePos;
                        }

                        mLayoutManager.smoothScrollToPosition(mRecyclerView, null, targetPosition);
                    }
                }
            }
        }

        @Override
        public void onScrolled(@NonNull RecyclerView recyclerView,
                               int dx,
                               int dy) {
            super.onScrolled(recyclerView, dx, dy);
        }
    };

    /**
     * 同步当前播放信息到列表适配器
     * <p> 警惕当前播放信息对象是 null 的情况；
     *
     * @param forceUpdate 强制刷新列表
     */
    private void syncPlayInfo2RvAdapter(boolean forceUpdate) {
        if (Objects.isNull(mRecyclerView)) {
            return;
        }

        // mAppData.mCurrentMediaInfo 是当前播放信息对象
        mRvAdapter.updatePlayInfo(mAppData.mCurrentMediaInfo, forceUpdate);
    }

    /**
     * 同步媒体数据到列表适配器
     * <pre>
     *    1、同步当前播放存储设备列表数据；
     *    2、同步当前用户收藏的媒体列表数据；
     *    3、强制刷新会导致列表显示回到第一列；
     * </pre>
     *
     * @param forceUpdate 强制刷新列表
     */
    @SuppressLint({"NotifyDataSetChanged", "SwitchIntDef"})
    private void syncMediaList2RvAdapter(boolean forceUpdate) {
        if (Objects.isNull(mRecyclerView)) {
            return;
        }

        // 当前页面类型
        switch (pageType()) {
            case ISimpleList.PAGE_PLAYLIST:
                mDataObject.updateDataInfo(
                        mAppData.mCurrentDevice.storageType(),
                        mAppData.musicFirstPlaylist());
                break;
            case ISimpleList.PAGE_FAVORITE:
                FavoriteManager fm = FavoriteManager.getInstance();
                mDataObject.updateDataInfo(
                        IStorageDevice.STORAGE_TYPE_NONE,
                        fm.favoriteMusicList());
                break;
            default:
                break;
        }

        // 更新数据到适配器
        mRvAdapter.updateDataList(mDataObject.list);
        mRecyclerView.setAdapter(mRvAdapter);

        if (forceUpdate) {
            mRvAdapter.notifyDataSetChanged();
        }
    }

    /**
     * 获取当前可见的 ViewHolder 列表并根据更新图标状态
     */
    private void updateVisibleItems() {
        if (Objects.isNull(mRecyclerView) || Objects.isNull(mRvAdapter)) {
            return;
        }
        RecyclerView.LayoutManager layoutManager = mRecyclerView.getLayoutManager();
        if (layoutManager instanceof LinearLayoutManager) {
            LinearLayoutManager linearLayoutManager = (LinearLayoutManager) layoutManager;
            int firstVisiblePosition = linearLayoutManager.findFirstVisibleItemPosition();
            int lastVisiblePosition = linearLayoutManager.findLastVisibleItemPosition();
            for (int i = firstVisiblePosition; i <= lastVisiblePosition; i++) {
                mRvAdapter.notifyItemChanged(i);
            }
        }
    }


    /**
     * 设置提示信息布局显示状态
     * <p> 提示信息布局在 RecyclerView 之上；
     *
     * @param visibility {@link  View#VISIBLE,View#INVISIBLE}
     */
    private void setSimplePromptLayout(int visibility) {
        if (Objects.isNull(mSimplePromptLayout)) {
            return;
        }

        mSimplePromptLayout.setVisibility(visibility);
    }

    @SuppressLint("NotifyDataSetChanged")
    @Override
    public void onResume() {
        super.onResume();
        LogUtil.v(TAG, "onResume");

        mRvAdapter.onResume();

        // 如果没有显示则需要强制更新
        int childCount = mRecyclerView.getChildCount();
        if (childCount == 0) {
            mRvAdapter.notifyDataSetChanged();
        }

        // 强制移动显示到当前播放位置
        tryMoveToPlayPosition(true);
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);

        LogUtil.v(TAG, "onHiddenChanged: " + hidden);
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // TODO: 预留接口
    }

    @SuppressLint("SwitchIntDef")
    @Override
    protected void onUpdateLanguageSkinText() {
        super.onUpdateLanguageSkinText();

        // 更新提示信息文字
        if (mTvSimplePromptLabel != null) {
            @StringRes int promptRes;

            // 当前页面类型
            switch (pageType()) {
                case ISimpleList.PAGE_FAVORITE:
                    promptRes = R3.string.empty_collect_list_prompt;
                    break;
                case ISimpleList.PAGE_PLAYLIST:
                default:
                    promptRes = R3.string.empty_play_list_prompt;
                    break;
            }

            String promptLabel = getString(promptRes);
            mTvSimplePromptLabel.setText(promptLabel);
        }
    }

    /**
     * 页面事件处理函数
     * <p> 具体原理参见 ViewModel 实现；
     *
     * @param event 事件 ID
     * @param obj1  附加数据对象 1
     * @param obj2  附加数据对象 2
     */
    @Override
    protected void onHandlePageEvent(int event, Object obj1, Object obj2) {
        super.onHandlePageEvent(event, obj1, obj2);
        Logger.t(TAG).v("onHandlePageEvent/"
                + pageTypeName() + ": " + PageEvent.name(event) + " / " + obj1);

        switch (event) {
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
                onUpdateListChangedEvent(obj1, obj2);
                break;
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE:
                updateVisibleItems();
                break;
            case IMediaEvent.EVENT_MUSIC_FAVORITE_OPERATE:
                onMusicFavoriteOperateEvent(obj1, obj2);
                break;
            case IMediaEvent.EVENT_NONE:
            default:
                break;
        }
    }

    /**
     * 处理播放列表更新事件
     * @see IMediaEvent#EVENT_CHANGE_MUSIC_ITEM
     * @see IMediaEvent#EVENT_CHANGE_MUSIC_LIST
     *
     * @param obj1 附加参数 1
     * @param obj2 附加参数 2
     */
    @SuppressLint("SwitchIntDef")
    private void onUpdateListChangedEvent(Object obj1, Object obj2) {
        if (Objects.isNull(mRvAdapter)) {
            return;
        }

        // 列表数据改变
        boolean listChanged = true;

        // 当前页面类型
        switch (pageType()) {
            case ISimpleList.PAGE_PLAYLIST:
                // 存储设备类型匹配
                if (mDataObject.isMatched(
                        mAppData.mCurrentDevice.storageType(),
                        mAppData.musicFirstPlaylist())) {
                    listChanged = false;
                }
                break;
            case ISimpleList.PAGE_FAVORITE:
                // 收藏列表不带存储类型
                FavoriteManager fm = FavoriteManager.getInstance();
                if (mDataObject.isMatched(
                        IStorageDevice.STORAGE_TYPE_NONE,
                        fm.favoriteMusicList())) {
                    listChanged = false;
                }
            default:
                break;
        }

        // 打印当前播放信息
        if (!Objects.isNull(mAppData.mCurrentMediaInfo)) {
            LogUtil.v(TAG, "onUpdateListChangedEvent/"
                    + pageTypeName() +  ": " + mAppData.mCurrentMediaInfo.mFileName
                    + ", " + listChanged);
        } else {
            LogUtil.v(TAG, "onUpdateListChangedEvent/" + pageTypeName()
                    +  ": Current media info is empty, " + listChanged);
        }

        // 更新播放媒体信息
        syncPlayInfo2RvAdapter(!listChanged);
        if (listChanged) {
            // 如果是显示状态，可以强制更新；
            syncMediaList2RvAdapter(isResumed());
        }

        // 更新列表显示信息
        updateSimpleListElement();

        // 更新 SimpleList
        H0.postUniqueDelayed(
                mMoveToPlayPositionRunnable, 10);
    }

    /** 更新列表选项（跳转列表显示选项到播放选项） **/
    private final Runnable mMoveToPlayPositionRunnable = () -> {
        // 尝试平滑过渡到当前播放位置
        tryMoveToPlayPosition(false);
    };

    /**
     * 尝试移动到当前播放位置
     * <pre>
     *    1、如果当前 Fragment 不在显示状态，则不要跳转；
     *    2、如果 SimpleList 本身在滑动过程中，也不要跳转；
     *    3、长时间没有滑动过播放列表，也不需要跳转显示位置（建议 30s 未操作才可跳转）；
     *    4、显示 Fragment 的时候可以考虑强制显示跳转；
     * </pre>
     *
     * @param forceJump 强制跳转；
     */
    @SuppressLint("SwitchIntDef")
    private void tryMoveToPlayPosition(boolean forceJump) {
        // 非显示状态返回
        if (!isResumed()) {
            return;
        }

        // 页面事件状态的有效性检查（播放列表与当前页面类型不匹配）
        @IPlaylistType int playListType = mAppData.musicPlayListType();
        switch (pageType()) {
            case ISimpleList.PAGE_PLAYLIST:
                if (playListType == IPlaylistType.FAVORITE_LIST) {
                    return;
                }
                break;
            case ISimpleList.PAGE_FAVORITE:
                if (playListType != IPlaylistType.FAVORITE_LIST) {
                    return;
                }
                break;
            default:
                return;
        }

        // 当前播放位置与列表显示数据匹配检查
        int position = mAppData.musicPlayPosition();
        int listSize = mAppData.musicPlaylist().size();
        if (listSize != mDataObject.list.size()
                || !mDataObject.validDataIndex(position)) {
            return;
        }

        // 移动到指定位置
        if (forceJump) {
            RecyclerViewUtils.moveToPosition(
                    mLayoutManager, mRecyclerView, position);
        } else {
            RecyclerViewUtils.smoothScrollToPosition(
                    mLayoutManager, mRecyclerView, position);
        }
    }

    /**
     * 当前音乐收藏列表发生改变
     * @see IMediaEvent#EVENT_MUSIC_FAVORITE_OPERATE
     *
     * @param obj1 操作类型
     * @param obj2 附加参数
     */
    private void onMusicFavoriteOperateEvent(Object obj1, Object obj2) {
        // 非收藏列表不需要处理收藏事件
        if (!isPageType(ISimpleList.PAGE_FAVORITE)) {
            return;
        }

        // 最喜欢列表操作参数有效性检查
        if (!(obj1 instanceof String)) {
            Logger.t(TAG).w("onMusicFavoriteOperateEvent: error parameter type!");
            return;
        }

        final String operateType = (String) obj1;
        switch (operateType) {
            case FavoriteManager.OPERATE_ADD:
            case FavoriteManager.OPERATE_REMOVE:
                if (Objects.isNull(obj2)) {
                    return;
                }

                assert obj2 instanceof FavoriteManager.InfoPackage;
                final FavoriteManager.InfoPackage packet = (FavoriteManager.InfoPackage) obj2;
                switch (operateType) {
                    case FavoriteManager.OPERATE_ADD:
                        onMusicFavoriteAddOperate(packet.index);
                        break;
                    case FavoriteManager.OPERATE_REMOVE:
                        onMusicFavoriteRemoveOperate(packet.index);
                        break;
                    default:
                        break;
                }
                break;
            case FavoriteManager.OPERATE_UPDATE:
                onMusicFavoriteUpdateOperate();
                break;
            default:
                break;
        }
    }

    /**
     * 处理音乐收藏列表增加事件
     * <pre>
     *    如收藏列表不做排序，那么增加操作永远是添加到最后一个；
     *    如收藏列表要做排序，那就比较复杂了（默认设计不排序）；
     * </pre>
     *
     * @param index 数据插入的位置索引
     */
    private void onMusicFavoriteAddOperate(int index) {
        LogUtil.v(TAG, "onMusicFavoriteAddOperate: " + index);
        if (Objects.isNull(mRvAdapter)) {
            return;
        }

        // 添加数据到列表
        FavoriteManager fm = FavoriteManager.getInstance();
        mDataObject.updateDataInfo(
                IStorageDevice.STORAGE_TYPE_NONE,
                fm.favoriteMusicList());
        mRvAdapter.updateDataList(mDataObject.list);

        // 同步播放信息
        syncPlayInfo2RvAdapter(false);

        // 更新列表显示
        mRvAdapter.notifyItemInserted(index);
        if (isResumed()) {
            mRvAdapter.notifyItemRangeChanged(index, mRvAdapter.getItemCount());
        }

        // 更新检查列表提示信息
        updateSimpleListElement();
    }

    /**
     * 处理音乐收藏列表删除事件
     * <pre>
     *    可以刪除收藏列表中任意位置的数据对象；
     *    排序与否都不影响删除动作，删除是最简单的操作；
     *    但是需要考虑列表刷新情况（列表是局部删除还是会被刷新到第一项显示）；
     * </pre>
     *
     * @param index 被删除的数据在当前列表的位置；
     */
    private void onMusicFavoriteRemoveOperate(int index) {
        LogUtil.v(TAG, "onMusicFavoriteRemoveOperate: " + index);

        if (Objects.isNull(mRvAdapter)) {
            return;
        }

        // 添加数据到列表
        FavoriteManager fm = FavoriteManager.getInstance();
        mDataObject.updateDataInfo(
                IStorageDevice.STORAGE_TYPE_NONE,
                fm.favoriteMusicList());
        mRvAdapter.updateDataList(mDataObject.list);

        // 同步播放信息
        syncPlayInfo2RvAdapter(false);

        // 更新列表显示（非现实状态调用会导致消息积累）
        mRvAdapter.notifyItemRemoved(index);
        if (isResumed()) {
            mRvAdapter.notifyItemRangeChanged(index, mRvAdapter.getItemCount());
        }

        // 更新检查列表提示信息
        updateSimpleListElement();

        // 更新收藏夹播放列表
        H0.postUniqueDelayed(mUpdateFavoritePlaylist, 10);
    }

    /**
     * 处理音乐收藏夹更新操作
     * <pre>
     *    存储设备发生改变（插入、移除）；
     *    休眠唤醒触发挂载相关事件等；
     * </pre>
     */
    private void onMusicFavoriteUpdateOperate() {
        onUpdateListChangedEvent(null, null);
    }

    /**
     * 更新收藏夹播放列表
     * <p> 只有当前播放列表是收藏夹播放列表的时候才需要更新；
     */
    private final Runnable mUpdateFavoritePlaylist = () -> {
        // 如果当前播放列表不是收藏列表，不更新
        if (mAppData.musicPlayListType()
                != IPlaylistType.FAVORITE_LIST) {
            return;
        }

        mMusicViewModel.playerRelay().accept(
                t -> t.requestUpdatePlaylist(
                        IPlaylistType.FAVORITE_LIST, mDataObject.list));
    };

    /** 更新显示元素 **/
    private void updateSimpleListElement() {
        // 无数据时需显示提示信息
        setSimplePromptLayout(
                mDataObject.isEmpty()? View.VISIBLE: View.INVISIBLE);
    }

    @Override
    public void onPause() {
        super.onPause();
        LogUtil.v(TAG, "onPause");

        mRvAdapter.onPause();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LogUtil.v(TAG, "onDestroyView");

        // 移除列表滚动监听状态
        mRecyclerView.removeOnScrollListener(mRecyclerViewScrollLListener);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mRvAdapter.onDestroy();
    }

    @Override
    public void onDetach() {
        super.onDetach();
        mRvAdapter = null;
    }

    @Override
    public void onUpdateUiModeView(boolean isNightMode) {
        super.onUpdateUiModeView(isNightMode);

        // 重新设置资源
        mRvAdapter.notifyDataSetChanged();
    }
}
