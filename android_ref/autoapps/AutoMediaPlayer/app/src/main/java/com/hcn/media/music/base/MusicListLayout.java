package com.hcn.media.music.base;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
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

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media.adapter.MusicSongListAdapter;
import com.hcn.media.R3;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media.base.layout.FrameLayoutEx;
import com.hcn.media.music.ITouchEventListener;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media.vm.action.IPlayerEx;
import com.hcn.media_data.ListSceneManager;
import com.hcn.media_data.ui.base.PageDataKV;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_data.storage.StorageDeviceEx;
import com.hcn.media_common.debug.LogUtil;

import java.util.List;
import java.util.Objects;

/**
 * 音乐列表布局
 * <p> 子项元素基本都是使用的帧布局，扩展性强；
 *
 * @author 86158
 */
public class MusicListLayout extends FrameLayoutEx
        implements OnItemClickListener, View.OnClickListener, OnScrollListener {
    private static final String TAG = MusicListLayout.class.getSimpleName();

    private AbsListView mMusicGridView = null;
    private MusicSongListAdapter mMusicSongListAdapter = null;
    private TextView mTvShowPrompt = null;

    /**
     * 反馈触摸事件触发状态用
     * <p> 给外部 Fragment 或者其它父容器反馈当前列表活跃状态；
     */
    private ITouchEventListener mTouchEventListener = null;

    /**
     *  smart 按键选择
     */
    private int mSelectItemIndex = -1;
    private StorageDeviceEx mStorageDeviceEx = null;

    public MusicListLayout(Context context,
                           @NonNull IPlayerEx player) {
        this(context, null, player);
    }

    public MusicListLayout(Context context,
                           AttributeSet attrs,
                           @NonNull IPlayerEx player) {
        this(context, attrs, 0, player);
    }

    public MusicListLayout(Context context,
                           AttributeSet attrs,
                           int defStyle,
                           @NonNull IPlayerEx player) {
        super(context, attrs, defStyle, player);

        // [注意：不可以提取放到父类中使用/否则 inflate 的视图会被系统回收]
        initContentView(getLayoutRes(), this);
    }

    @Override
    protected int getLayoutRes() {
        return R.layout.layout_listmusic;
    }

    @Nullable
    @Override
    protected View initContentView(int layoutRes, ViewGroup root) {
        LogUtil.d(TAG, "initContentView.");
        View view = super.initContentView(layoutRes, root);

        // 显示提示
        assert view != null;
        mTvShowPrompt = view.findViewById(xId(R.id.tvShowTip));

        // 刷新按钮(保留)
        View btnRefresh = view.findViewById(xId(R.id.btn_refresh));
        if (null != btnRefresh) {
            btnRefresh.setOnClickListener(this);
        }

        // 查找按钮(保留)
        View btnSearch = view.findViewById(xId(R.id.btn_search));
        if (null != btnSearch) {
            btnSearch.setOnClickListener(this);
        }

        // 列表视图
        mMusicGridView = view.findViewById(xId(R.id.gridview_music));
        if (mMusicGridView != null) {
            mMusicSongListAdapter = new MusicSongListAdapter(mContext, mMusicGridView);
            mMusicGridView.setAdapter(mMusicSongListAdapter);
            mMusicGridView.setOnItemClickListener(this);
            mMusicGridView.setOnScrollListener(this);
        }

        return view;
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        Log.d(TAG, "onAttachedToWindow: ");
    }

    @Override
    public void onUpdateUiModeView(boolean isNightMode) {
        super.onUpdateUiModeView(isNightMode);

        // 通知适配器刷新，重新设置资源
        mMusicSongListAdapter.notifyDataSetChanged();
    }


    /**
     * 外部设置接口
     * @param storageDevice 存储设备
     */
    public void setStorageDevice(StorageDeviceEx storageDevice) {
        mStorageDeviceEx = storageDevice;

        setPlayingFilePath();
        updateListStatus("init/set");
        updateDataList();
    }

    @Override
    public void initLayout() {
        LogUtil.e(TAG, ">>>>> initLayout");

        // [Android P开始 ListView 只有在显示的时候才会更新]
        postDelayed(this::updateDataList, 10);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        if (mTouchEventListener != null) {
            mTouchEventListener.onTouchTrigger();
        }

        return super.dispatchTouchEvent(ev);
    }

    /**
     * 页面是否是空闲的
     * <p> 如果当前列表页面没有任何加载刷新任务，说明其在空闲状态;
     * <p> 后续还可以添加其它可能的 busy 任务状态。
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
        if (Objects.isNull(mTvShowPrompt)) {
            return false;
        }

        Object tag = mTvShowPrompt.getTag(0xDB00F001);
        if (tag instanceof String) {
            String strTag = (String) tag;
            return mTvShowPrompt.getVisibility() == View.VISIBLE && "loading".equals(strTag);
        }

        return false;
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

    private void onBtnRefresh() {
        refresh();
    }

    private void onBtnSearch() {
        search();
    }

    /** 对外刷新接口 **/
    public void refresh() {
        tryUpdateMediaList();
    }

    /**
     * 尝试扫描当前路径并更新列表
     * <pre>
     *    如果当前已经有一个 U 盘在播放状态，且当前在媒体列表页面；
     *    用户如果再次插入 U 盘（或者多个 U 盘其中一个发生抖动），后台将进入静默扫描状态；
     *    如果后台在静默扫描状态，那么点击刷新按钮将暂时无效；
     * </pre>
     */
    private void tryUpdateMediaList() {
        // 扫描中不处理
        if (mStorageDeviceEx == null
                || mStorageDeviceEx.isLoading()) {
            return;
        }

        // 投递播放目标活动
        if (mMediaEventPostbox != null) {
            mMediaEventPostbox.onMediaAction(
                    IMediaAction.scanStorageDeviceInfo,
                    mStorageDeviceEx.mFilePath,
                    null);
        }
    }

    /**
     * 跳转到查找 UI 界面
     */
    public void search() {
        if (mStorageDeviceEx != null) {
            mAppData.mSearchList = mStorageDeviceEx.mMusicInfoList;
            ListSceneManager.getInstance().updateListScenePath("");
        }

        if (mMediaEventPostbox != null) {
            mMediaEventPostbox.onMediaEvent(
                    IMediaEvent.EVENT_GOTO_MUSIC_SEARCH_PAGE,
                    null,
                    null);
        }
    }

    /**
     * 更新播放列表信息
     * <p> 同步列表数据到适配器；
     */
    private void updateDataList() {
        if (Objects.isNull(mStorageDeviceEx)) {
            return;
        }

        if (null != mMusicSongListAdapter) {
            mMusicSongListAdapter.setDataList(mStorageDeviceEx.mMusicInfoList);
            mMusicSongListAdapter.notifyDataSetChanged();
        }
    }

    private void updateListStatus(String reason) {
        if (Objects.isNull(mStorageDeviceEx)) {
            return;
        }

        LogUtil.d(TAG, "updateListStatus, reason: " + reason);

        if (mStorageDeviceEx.isLoading()) {
            mTvShowPrompt.setText(getResources().getString(R3.string.tip_loading));
            mTvShowPrompt.setVisibility(View.VISIBLE);

            // 设置 Loading 标记
            mTvShowPrompt.setTag(0xDB00F001, "loading");
        } else {
            mTvShowPrompt.setText(getResources().getString(R3.string.tip_no_music_file));
            boolean bShowTip = mStorageDeviceEx.mMusicInfoList.isEmpty();
            mTvShowPrompt.setVisibility(bShowTip ? View.VISIBLE : View.GONE);

            // 如果是 loading 中，说明正好 Loading 结束。
            if (isShowLoading()) {
                // 模拟一个触摸事件，让外部监听重新计时超时时间。
                if (mTouchEventListener != null) {
                    mTouchEventListener.onTouchTrigger();
                }

                // 取消 Loading 标记
                mTvShowPrompt.setTag(0xDB00F001, "other");
            }
        }
    }

    @Override
    public void doCallbackEvent(int eventId) {
        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE:
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
                setPlayingFilePath();
                updateDataList();
                break;

            case IMediaEvent.EVENT_MEDIA_LOADING_START:
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
            case IMediaEvent.EVENT_CHANGE_MUSIC_STORAGE:
            case IMediaEvent.EVENT_UPDATE_MUSIC_LIST:
                updateDataList();
                updateListStatus("event/" + eventId);
                break;

            case IMediaEvent.EVENT_CONTROL_SMART_ENTER:
                onEventControlSmartEnter();
                break;

            case IMediaEvent.EVENT_CONTROL_SMART_CW:
                onEventControlSmartCW();
                break;

            case IMediaEvent.EVENT_CONTROL_SMART_CCW:
                onEventControlSmartCCW();
                break;

            case IMediaEvent.EVENT_CANCEL_SMART_CONTROL:
                onEventCancelSmartControl();
                break;

            default:
                break;
        }
    }

    /**
     * 处理 SMART_ENTER 事件
     * <p> IMediaEvent.EVENT_CONTROL_SMART_ENTER
     */
    private void onEventControlSmartEnter() {
        if (Objects.isNull(mStorageDeviceEx)) {
            return;
        }

        if (mSelectItemIndex >= 0) {
            List<MusicInfo> infoList = mStorageDeviceEx.mMusicInfoList;
            mAppData.mCurrentDevice = mStorageDeviceEx;

            if (mMediaEventPostbox != null) {
                mMediaEventPostbox.onMediaAction(
                        IMediaAction.requestPlayMusicInfo, infoList, mSelectItemIndex);
            }

            mSelectItemIndex = -1;
        }
    }

    /**
     * 处理 SMART_CW 事件
     * <p> IMediaEvent.EVENT_CONTROL_SMART_CW
     */
    private void onEventControlSmartCW() {
        if (Objects.isNull(mStorageDeviceEx)) {
            return;
        }

        if (mSelectItemIndex == -1) {
            if (mAppData.mCurrentDevice == mStorageDeviceEx) {
                mSelectItemIndex = mAppData.musicPlayPosition();
            } else {
                mSelectItemIndex = 0;
            }
        } else if (mSelectItemIndex <= 0) {
            mSelectItemIndex = mStorageDeviceEx.getMusicInfoList().size();
        } else {
            mSelectItemIndex = mSelectItemIndex - 1;
        }

        Log.d(TAG, "doCallbackEvent: EVENT_CONTROL_SMART_CW pos = " + mSelectItemIndex);
        try {
            mMusicSongListAdapter.setSelectIndex(mSelectItemIndex);
            mMusicGridView.post(mRunnable);
        } catch (Exception ignored) {
        }

        updateDataList();
    }

    /**
     * 处理 SMART_CCW 事件
     * <p> IMediaEvent.EVENT_CONTROL_SMART_CCW
     */
    private void onEventControlSmartCCW() {
        if (Objects.isNull(mStorageDeviceEx)) {
            return;
        }

        if (mSelectItemIndex == -1) {
            if (mAppData.mCurrentDevice == mStorageDeviceEx) {
                mSelectItemIndex = mAppData.musicPlayPosition();
            } else {
                mSelectItemIndex = 0;
            }
        } else if (mSelectItemIndex >= mStorageDeviceEx.getMusicInfoList().size()) {
            mSelectItemIndex = 0;
        } else {
            mSelectItemIndex = mSelectItemIndex + 1;
        }

        Log.d(TAG, "doCallbackEvent: EVENT_CONTROL_SMART_CCW pos = " + mSelectItemIndex);
        try {
            mMusicSongListAdapter.setSelectIndex(mSelectItemIndex);
            mMusicGridView.post(mRunnable);
        } catch (Exception ignored) {
        }

        updateDataList();
    }

    /**
     * 结束 Smart 控制操作
     * <p> IMediaEvent.EVENT_CANCEL_SMART_CONTROL
     */
    private void onEventCancelSmartControl() {
        mSelectItemIndex = -1;
        mMusicSongListAdapter.setSelectIndex(mSelectItemIndex);
        updateDataList();
    }

    /**
     * 设置当前播放曲目路径并更新列表
     */
    private void setPlayingFilePath() {
        if (Objects.isNull(mMusicSongListAdapter)) {
            return;
        }

        try {
            mMusicSongListAdapter.setPlayingFilePath(mAppData.mCurrentMediaInfo.mFilePath);
        } catch (Exception ignored) {
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View view) {
        switch (getId(view)) {
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

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        if (Objects.isNull(mStorageDeviceEx)) {
            return;
        }

        List<MusicInfo> infoList = mStorageDeviceEx.mMusicInfoList;
        mAppData.mCurrentDevice = mStorageDeviceEx;

        // 投递播放目标活动
        if (mMediaEventPostbox != null) {
            mMediaEventPostbox.onMediaAction(
                    IMediaAction.requestPlayMusicInfo, infoList, position);
            ListSceneManager.getInstance().saveUserListScene(PageDataKV.ActionSceneValue.NORMAL, "");
        }
    }

    @Override
    public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount, int totalItemCount) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onScrollStateChanged(AbsListView view, int scrollState) {
        if (scrollState == OnScrollListener.SCROLL_STATE_IDLE) {
            mMusicSongListAdapter.setScrollState(false);
            mMusicSongListAdapter.notifyDataSetChanged();
        } else {
            mMusicSongListAdapter.setScrollState(true);
        }
    }

    private final Runnable mRunnable = () -> {
        try {
            mMusicGridView.setSelection(mSelectItemIndex);
        } catch (Exception ignored) {
        }
    };

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        Log.d(TAG, "onDetachedFromWindow: ");
    }
}
