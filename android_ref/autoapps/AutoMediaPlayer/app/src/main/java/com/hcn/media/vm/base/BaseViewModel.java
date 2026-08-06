package com.hcn.media.vm.base;

import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.audiofx.Visualizer;
import android.text.TextUtils;

import androidx.annotation.NonNull;
import androidx.lifecycle.AndroidViewModel;

import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_common.HBroadcastEx;
import com.hcn.common.misc.HBroadcastUtils;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.fragment.IPageEventListener;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_model.MediaModel;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media.vm.action.IPlayerEx;
import com.hcn.media_model.player.base.IMediaPlayer;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.rxrelay3.PublishRelay;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Objects;

import io.reactivex.rxjava3.disposables.CompositeDisposable;

/**
 * 基视图模型
 * <pre>
 *     这里主要是处理音视频共用的东西；
 *     例如：活动事件观察监听器、交互中继器等
 * </pre>
 *
 * @author 65821
 */
public abstract class BaseViewModel
        extends AndroidViewModel implements IPlayerEx {
    private static final String TAG = BaseViewModel.class.getSimpleName();

    /**
     * 专用事件接口
     * <p> Fragment 传递事件给 MusicUI/VideoUI 使用
     */
    public interface Fragment2MainUi {

        /**
         * 事件处理接口
         * <p> {@link IMediaEvent}
         *
         * @param event 事件类型
         * @param obj1 附加对象 1
         * @param obj2 附加对象 2
         */
        void onEvent(int event, Object obj1, Object obj2);
    }

    /**
     * 播放相关的接口
     * <p> 为 Fragment/Activity 调用播放相关接口时使用；
     */
    public interface IPlayer {
        /**
         * 请求执行播放任务
         * <p> 启动进入播放/模式切换播放调用
         */
        void requestPlayTask();

        /**
         * 请求播放目标选项
         * <p> UI 点击媒体列表选项后调用；
         *
         * @param type 播放列表类型
         * @param infoList 需要播放的列表
         * @param position 目标在列表中的位置
         */
        void requestPlayTarget(@IPlaylistType int type,
                               List<MusicInfo> infoList,
                               int position);

        /**
         * 请求更新播放列表
         * <p> 当播放列表内容发生改变的时候触发更新；
         *
         * @param type 播放列表类型
         * @param infoList 期望更新的数据
         * @return {@link Boolean} 更新成功/更新失败
         */
        boolean requestUpdatePlaylist(@IPlaylistType int type,
                                   List<MusicInfo> infoList);

        /**
         * 请求播放目标选项
         * <p> 指定播放目标，无关播放列表；
         *
         * @param info 需要播放的目标
         * @deprecated 测试函数，建议少用，容易导致播放列表管理混乱；
         */
        @Deprecated
        void requestPlayTarget(MusicInfo info);

        /**
         * 请求执行播放控制命令
         * <p> 播放、暂停、上一曲、下一曲、停止播放等
         *
         * @param command 命令 {@link IMusicState#PLAY_CMD_PLAY,...}
         */
        void requestPlayControl(int command);

        /**
         * 应用恢复播放事件
         * <p> 恢复播放任务（视频还可以触发新播放任务）；
         */
        void requestShouldPlayEvent();

        /**
         * 请求应用暂停/停止播放事件
         * <p> 暂停/停止播放任务（视频还可以触发新播放任务）；
         *
         * @param stop 是否停止播放
         * @param reason 请求原因
         */
        void requestShouldPauseEvent(boolean stop, int reason);

        /**
         * 请求查询状态
         * <p> 查询状态类别相关接口整合
         *
         * @param action @NonNull final String action
         * @param obj1 附加参数 1
         * @return {@link Boolean} 是/否
         */
        boolean requestQueryState(@NonNull final String action, Object obj1);

        /**
         * 请求执行指定的动作
         * <pre>
         *     把部分函数调用转变整合成一个接口；
         *     action 一般表示需要执行的函数名称，这里支持最多带 2 个参数的函数；
         *     e.g. requestExecuteAction("switchPlayRepeatMode", null, null);
         * </pre>
         *
         * @param action 动作名称
         * @param obj1 附加参数 1
         * @param obj2 附加参数 2
         */
        void requestExecuteAction(@NonNull final String action, Object obj1, Object obj2);
    }

    /**
     * 事件处理器：应用 ViewModel 给 MusicUI/VideoUI 下发事件
     * <pre>
     *    注意：它受关联 UI 生命周期的约束；
     *    生命周期的限制: [onResume，onPause]
     *    在 onStart 之前调用它，只会有最后一次会触发回调。
     * </pre>
     */
    private final VmCommand<Fragment2MainUi> mFragment2MainUi = new VmCommand<>();

    /**
     * 页面事件中继器
     * <p> 注意: 它不受 UI 生命周期的约束;
     */
    private final PublishRelay<VmCommand.Action<IPageEventListener>> mPageEventRelay = PublishRelay.create();

    /**
     * 播放器中继器
     * <pre>
     *    处理音乐相关的 UI 下发的播放事件；
     *    虽然接口差异不大，但是音乐和视频建议不要混用同一个 ViewModel;
     * </pre>
     */
    protected final PublishRelay<VmCommand.Action<IPlayer>> mPlayerRelay = PublishRelay.create();

    /**
     * 订阅回收管理器
     * <p> 它是线程安全的，可以用来回收 Observable 对象；
     */
    protected final CompositeDisposable mCompositeDisposable = new CompositeDisposable();

    /**
     * 本地广播事件接收者
     * <p> 由 HMediaPlayer/LocalService 下发的媒体播放相关事件状态；
     */
    private BroadcastReceiver mLocalEventReceiver;

    /** AndroidViewModel 标准构造函数 **/
    public BaseViewModel(@NonNull Application application) {
        super(application);

        // 接入本地事件接收处理器
        connectLocalBroadcastReceiver();
    }

    /**
     * 获取当前 VM 依赖的上下文对象
     * <p> 只要是正常流程创建的 VM，它都是当前应用全局上下文环境；
     *
     * @return 上下文对象
     */
    @NonNull
    public final Context requireContext() {
        Context context = getApplication().getApplicationContext();
        if (context == null) {
            throw new IllegalStateException("BaseViewModel " + this + " not attached to a context.");
        }
        return context;
    }

    /**
     * 获取可被观察的记录仪事件持有对象
     * <pre>
     *    它存在生命周期约束: [onResume，onPause]
     *    请不要越界使用，
     * </pre>
     *
     * @return {@link VmCommand<Fragment2MainUi>}
     */
    public VmCommand<Fragment2MainUi> fragment2MainUi() {
        return mFragment2MainUi;
    }

    /**
     * 提供给各个 Fragment 通讯用
     * <pre>
     *    Fragment 这种独立的封装，可以用此交互事件；
     *    场景使用场景：
     *      Fragment -- Fragment
     *      Fragment -- Activity
     *      ViewMode -- Fragment/Activity
     *    禁止使用场景：
     *      Fragment 和 自定义 View/Layout 之间不要使用它，会显得多此一举；
     *      Fragment 和 自定义 View/Layout 之间请直接用回调监听；
     * </pre>
     *
     * @return Fragment 页面事件中继器；
     */
    public PublishRelay<VmCommand.Action<IPageEventListener>> pageEventRelay() {
        return mPageEventRelay;
    }

    /** 获取音频可视化工具 **/
    @Override
    public Visualizer getVisualizer() {
        return MediaModel.call().localzModel().getVisualizer();
    }

    /** 获取媒体音频回话 ID **/
    @Override
    public int getAudioSessionId() {
        return MediaModel.call().localzModel().getAudioSessionId();
    }

    @Override
    public IMediaPlayer corePlayer() {
        return MediaModel.call().playerModel().corePlayer();
    }

    @Override
    public IMediaPlayer vitamioPlayer() {
        return MediaModel.call().playerModel().vitamioPlayer();
    }

    /**
     * 播放器中继器
     * <pre>
     *    音乐 UI 页面调用播放功能的统一接口（推荐使用）；
     *    后续不再容许 UI 直接调用 Application 对象访问播放接口；
     *    原因很简单，后续播放对象不一定放到 Application 中，可能调整到后台服务中；
     * </pre>
     *
     * @return 中继器对象
     */
    @Override
    public PublishRelay<VmCommand.Action<IPlayer>> playerRelay() {
        return mPlayerRelay;
    }

    /**
     * 接入本地事件广播接收端
     * <pre>
     *    这是一个模拟广播机制的 callback 事件分发组件；
     *    我们可以用它实现本地 Service 组件和 VM 通讯；
     * </pre>
     */
    private void connectLocalBroadcastReceiver() {
        if (Objects.isNull(mLocalEventReceiver)) {
            mLocalEventReceiver = new LocalEventReceiver(this);
        } else {
            LogUtil.w(TAG, "Function connectLocalBroadcastReceiver called repeatedly!");
            return;
        }

        // 注册本地事件广播接收者
        IntentFilter intentFilter= new IntentFilter();
        intentFilter.addAction(HBroadcastEx.SpecialChain.ACTION_LOCAL_CALLBACK);
        HBroadcastUtils.getInstance(requireContext())
                .registerReceiver(mLocalEventReceiver, intentFilter);
    }

    /**
     * 断开本地事件广播接收端
     * @see #connectLocalBroadcastReceiver() 接口
     */
    private void disconnectLocalBroadcastReceiver() {
        if (Objects.isNull(mLocalEventReceiver)) {
            return;
        }

        HBroadcastUtils.getInstance(requireContext())
                .unregisterReceiver(mLocalEventReceiver);
        mLocalEventReceiver = null;
    }

    /**
     * 处理本地广播事件
     *
     * @param event 事件定义 {@link IMediaEvent}
     * @param wParam 附加参数 1
     * @param lParam 附加参数 2
     */
    protected abstract void onLocalBroadCastEvent(int event, Object wParam, Object lParam);

    /**
     * 本地广播事件处理类
     * <p> 由 HMediaPlayer/LocalService 下发的媒体播放相关事件状态；
     */
    private static final class LocalEventReceiver extends BroadcastReceiver {
        private final Reference<BaseViewModel> mOwnerRef;

        public LocalEventReceiver(BaseViewModel viewModel) {
            super();
            mOwnerRef = new WeakReference<>(viewModel);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (TextUtils.isEmpty(action)) {
                return;
            }

            // 处理本地广播事件（LocalService）
            if (action.equals(HBroadcastEx.SpecialChain.ACTION_LOCAL_CALLBACK)) {
                BaseViewModel vm = mOwnerRef.get();
                if (Objects.isNull(vm)) {
                    return;
                }

                // 读取本地事件广播参数
                int event = intent.getIntExtra(
                        HBroadcastEx.SpecialChain.EXTRA_CALLBACK_TYPE, IMediaEvent.EVENT_NONE);
                String data = intent.getStringExtra(HBroadcastEx.SpecialChain.EXTRA_CALLBACK_DATA);
                vm.onLocalBroadCastEvent(event, data, null);
            }
        }
    }

    /**
     * 播放逻辑相关接口实现
     * <p> 这里只实现音视频的公共部分，差异部分需要在子类区分处理；
     */
    protected abstract static class PlayerImpl implements IPlayer {
        @Override
        public void requestPlayControl(int command) {
            MediaModel.call()
                    .localzModel()
                    .requestPlayControl(command);
        }

        @Override
        public void requestPlayTask() {
            MediaModel.call()
                    .localzModel()
                    .requestSwitchMediaType();
        }

        @Override
        public void requestShouldPlayEvent() {
            MediaModel.call()
                    .localzModel()
                    .doShouldPlayEvent();
        }

        @Override
        public void requestShouldPauseEvent(boolean stop, int reason) {
            MediaModel.call()
                    .localzModel()
                    .doShouldPauseEvent(stop, reason);
        }

        /**
         * 请求播放目标媒体信息
         * <p> 由音视频 VM 各自独立实现其内容；
         *
         * @param type 播放列表类型
         * @param infoList 需要播放的列表
         * @param position 目标在列表中的位置
         */
        @Override
        public abstract void requestPlayTarget(@IPlaylistType int type,
                                               List<MusicInfo> infoList,
                                               int position);

        /**
         * 请求更新播放列表
         * <p> 当播放列表内容发生改变的时候触发更新；
         *
         * @param type 播放列表类型
         * @param infoList 期望更新的数据
         */
        @Override
        public abstract boolean requestUpdatePlaylist(@IPlaylistType int type,
                                                   List<MusicInfo> infoList);

        /**
         * 指定播放对象
         * <pre>
         *    1、这是一个无关播放列表的接口；
         *    2、因为无关播放列表，那么使用它就容易引起播放列表的维护问题；
         *    3、建议它只作为测试函数用；
         * </pre>
         *
         * @param info 需要播放的目标
         * @deprecated 尽可能不要用这个函数，安全性差，使用不好会导致播放列表管理混乱；
         */
        @Deprecated
        @Override
        public void requestPlayTarget(MusicInfo info) {
            MediaModel.call()
                    .localzModel()
                    .requestPlayDataSource(info);
        }

        @Override
        public boolean requestQueryState(@NonNull String action, Object obj1) {
            switch (action) {
                case IMediaAction.inAccOnState:
                    return MediaModel.call().
                            localzModel().
                            inAccOnState();
                case IMediaAction.isLocalConnected:
                    return MediaModel.call()
                            .localzModel()
                            .isLocalConnected();
                case IMediaAction.isUsbMounted:
                    return MediaModel.call()
                            .localzModel()
                            .isUsbMounted();
                case IMediaAction.isSdcardMounted:
                    return MediaModel.call()
                            .localzModel()
                            .isSdcardMounted();
                default:
                    break;
            }
            return false;
        }

        @Override
        public void requestExecuteAction(@NonNull String action, Object obj1, Object obj2) {
            switch (action) {
                case IMediaAction.onRequestAudioFocus:
                    MediaModel.call()
                            .localzModel()
                            .onRequestAudioFocus();
                    break;
                case IMediaAction.scanStorageDeviceInfo:
                    MediaModel.call()
                            .localzModel()
                            .requestScanTargetPath((String) obj1);
                    break;
                case IMediaAction.seekToTime:
                    MediaModel.call()
                            .playerModel()
                            .seekToTime((Integer) obj1);
                    break;
                case IMediaAction.setSeekTimeZero:
                    MediaModel.call()
                            .playerModel()
                            .onSetSeekTimeZero();
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * 当前媒体后台准备就绪
     * <pre>
     *    需要满足以下条件
     *     1、本地服务连接成功；
     *     2、远程数据服务连接成功；
     *     3、可以开始后台播放；
     * </pre>
     *
     * @return 准备就绪/未准备就绪
     */
    public boolean isServiceReadyState() {
        return MediaModel.call()
                .localzModel()
                .isServiceReadyState();
    }

    /** 退出当前进程 **/
    public void exitApplication(int reason) {
        MediaModel.call()
                .localzModel()
                .requestExitApp(reason);
    }

    /**
     * ViewModel 销毁
     * <p> 由源码发现它在 Activity 的 onStop 和 onDestroy 之间触发；
     */
    @Override
    protected void onCleared() {
        super.onCleared();

        // 释放订阅资源
        mCompositeDisposable.dispose();

        // 断开本地事件接收处理器
        disconnectLocalBroadcastReceiver();
    }
}
