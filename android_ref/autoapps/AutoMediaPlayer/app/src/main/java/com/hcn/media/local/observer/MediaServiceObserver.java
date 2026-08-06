package com.hcn.media.local.observer;

import android.Configures.HConfig;
import android.app.Service;
import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.provider.Settings;

import androidx.annotation.NonNull;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleOwner;

import com.hcn.auto_compat.app.Wallpaper;
import com.hcn.common.lang.Listenable;
import com.hcn.common.misc.HBusUtils;
import com.hcn.config.HSettings;
import com.hcn.media.base.xbus.IBusTag;
import com.hcn.media.base.service.ServiceLifecycleObserver;
import com.hcn.media.local.event.VehicleConfigEx;
import com.hcn.media_common.HBroadcastEx;
import com.hcn.media_common.HMessage;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.common.utils.HHandler;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_data.FavoriteManager;
import com.hcn.media_data.ui.MediaPageState;
import com.hcn.media_data.ui.base.PageDataKV;
import com.hcn.media_model.MediaModel;
import com.hcn.media_model.player.base.IMediaPlayer;
import com.hcn.media_view.lyrics.LyricsManager;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.rxrelay3.PublishRelay;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.Objects;
import java.util.concurrent.ExecutorService;

/**
 * 媒体服务观察者
 * <pre>
 *    主要是观察 LocalService 生命周期，简化服务类中的代码逻辑；
 *    后续本地服务的扩展功能都可以放到这里处理；
 * </pre>
 *
 * @author 65821
 */
public class MediaServiceObserver extends ServiceLifecycleObserver {
    private static final String TAG = "MediaServiceObserver";

    /** 全局数据对象 **/
    protected final AppGlobalData mAppData;

    /**
     * 外部输入参数
     * <p> 这 3 个是由构造函数传入的参数，不可篡改；
     */
    private final Reference<Service> mOwnerRef;
    private final LifecycleOwner mLifecycleOwner;
    private final Reference<ExecutorService> mExecutorServiceRef;

    /**
     * 消息处理器
     * <p> 与当前进程的主线程消息循环体绑定；
     */
    private final HHandler H0 = new TaskHandler(Looper.getMainLooper(), this);

    /** 监听 Settings 相关的键值 **/
    private ContentResolver mContentResolver = null;
    private final ContentObserver mContentObserver = new ContentObserver(
            new Handler(Looper.getMainLooper())) {

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            super.onChange(selfChange, uri);

            String path = uri.getPath();
            String key = Objects.requireNonNull(path).replace("/system/", "");
            LogUtil.d(TAG, "mIsCanWatchVideo onChange :" + mAppData.mDrivingWatchVideoEnable);

            switch (key) {
                // 行车中能否观看视频[设计反了]
                case HConfig.driving_disable_video:
                    onDrivingDisableVideo();
                    break;
                // 车辆低速状态释放可以观看视频
                case VehicleConfigEx.VEHICLE_SPEED_VIDEO_STATE:
                    onVehicleSpeedVideoState();
                    break;
                case "none":
                default:
                    break;
            }
        }
    };

    /**
     * 构造函数
     * <p> 注意构造函数尽量只干成员初始化工作；
     *
     * @param lifecycleOwner 所有者
     * @param service 上下文环境
     * @param executorService 线程池
     */
    public MediaServiceObserver(
            @NonNull LifecycleOwner lifecycleOwner,
            Service service,
            ExecutorService executorService) {
        if (null == service) {
            throw new IllegalArgumentException("u can't instantiate me...");
        }

        LogUtil.v(TAG, "Constructor.");
        mOwnerRef = new WeakReference<>(service);
        mExecutorServiceRef = new WeakReference<>(executorService);

        // 监听 MiscService 生命周期
        mLifecycleOwner = lifecycleOwner;
        mAppData = AppGlobalData.getInstance();
    }

    /**
     * 当前是否在目标状态
     *
     * @param state 目标状态
     * @return {@code true} 匹配目标状态。
     */
    public boolean isState(Lifecycle.State state) {
        if (mLifecycleOwner != null) {
            Lifecycle.State currentState = mLifecycleOwner.getLifecycle().getCurrentState();
            return currentState.equals(state);
        }

        return false;
    }

    @Override
    public void onCreate(@NonNull LifecycleOwner owner) {
        LogUtil.v(TAG, "onCreate.");
        Service service = mOwnerRef.get();
        mContentResolver = service.getContentResolver();

        // 获取系统平台播放组件
        IMediaPlayer player =
                MediaModel.call()
                        .playerModel()
                        .corePlayer();

        // 转发播放组件媒体事件（避免绕到 Application 回调）
        mCompositeDisposable.add(player.eventRelay().subscribe(message -> {
            // 多媒体事件
            switch (message.what) {
                case IMediaEvent.EVENT_MUSIC_PLAYER_PREPARING:
                    onMusicPlayerPreparing(message.obj0);
                    break;
                case IMediaEvent.EVENT_VIDEO_PLAYER_PREPARING:
                    onVideoPlayerPreparing(message.obj0);
                    break;
                case IMediaEvent.EVENT_NONE:
                default:
                    break;
            }

            // 不要在此回收这个事件（嵌套使用注意风险）
            eventRelay().accept(message);
        }));

        // 壁纸设置监听
        Wallpaper.instance().register((s, o) -> {
            // 用户切换了壁纸功能
            if (s.equals(Wallpaper.ET_SAVE_PATH)) {
                if (!(o instanceof String)) {
                    return;
                }

                String path = (String) o;
                MediaPageState.instance().saveWallpaperData(path);
            }
        });

        // 监听 Settings/System 键值
        registerSettingsObserver();
    }

    /**
     * 拦截音乐切曲成功事件
     * <pre>
     *    如果音乐播放器触发 Preparing 事件，那肯定是切曲成功了；
     *    我们可以在此做一些检查性质的工作，例如：收藏状态检查更新；
     * </pre>
     *
     * @param obj 播放数据对象
     * @see IMediaEvent#EVENT_MUSIC_PLAYER_PREPARING
     */
    private void onMusicPlayerPreparing(Object obj) {
        if (!(obj instanceof MusicInfo)) {
            return;
        }

        // 更新收藏状态信息
        MusicInfo info = (MusicInfo) obj;
        info.mFavorite = FavoriteManager.getInstance().inFavoriteMusicList(info);

        // 通知更新播放信息
        HBusUtils.post(IBusTag.UPDATE_MUSIC_PLAY_INFO, info);

        // 通知检查歌词文件
        HBusUtils.post(IBusTag.UPDATE_MUSIC_LYRICS_INFO,
                LyricsManager.instance().getLyricsFilePath(info.mFilePath));
    }

    /**
     * 拦截视频切曲成功事件
     * <pre>
     *    如果视频播放器触发 Preparing 事件，那肯定是切曲成功了；
     *    我们可以在此做一些检查性质的工作，例如：收藏状态检查更新；
     * </pre>
     *
     * @param obj 播放数据对象
     * @see IMediaEvent#EVENT_VIDEO_PLAYER_PREPARING
     */
    private void onVideoPlayerPreparing(Object obj) {
        if (!(obj instanceof MusicInfo)) {
            return;
        }

        MusicInfo info = (MusicInfo) obj;
        info.mFavorite = FavoriteManager.getInstance().inFavoriteVideoList(info);
    }

    @Override
    public void onStart(@NonNull LifecycleOwner owner) {
        LogUtil.v(TAG, "onStart.");
    }

    /** 注册 Settings/System 键值监听 **/
    private void registerSettingsObserver() {
        // 行车中是否允许观看视频
        mContentResolver.registerContentObserver(
                Settings.System.getUriFor(HConfig.driving_disable_video),
                false,
                mContentObserver);

        // 车辆速度视频状态(当前车速是否可以观看视频)
        mContentResolver.registerContentObserver(
                Settings.System.getUriFor(VehicleConfigEx.VEHICLE_SPEED_VIDEO_STATE),
                false,
                mContentObserver);
    }

    /** 取消 Settings/System 键值监听 **/
    private void unregisterSettingsObserver() {
        if (null != mContentResolver) {
            mContentResolver.unregisterContentObserver(mContentObserver);
        }
    }

    /**
     * 处理行车中是否可以观看视频状态改变
     * @see HConfig#driving_disable_video
     */
    private void onDrivingDisableVideo() {
        mAppData.mDrivingWatchVideoEnable = (1 == Settings.System.getInt(
                mContentResolver, HConfig.driving_disable_video, 0));
        // 行车中可以看视频-关（视频播放中）
        // 暂停视频；修复[BUG]PIP/后台播放视频->关闭“行车中可以看视频”，视频还会继续播放
        if ((mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY))
                && mAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO
                && !mAppData.mDrivingWatchVideoEnable) {
            // 分发媒体事件
            dispatchMediaEvent(
                    new HMessage(IMediaEvent.EVENT_REQUEST_MEDIA_PAUSE));
        }
    }

    /**
     * 处理车辆速度视频状态（低速可看视频状态）
     * <pre>
     *    由于业务逻辑和刹车状态改变类似；
     *    这里统一当做刹车状态改变事件处理；
     * </pre>
     *
     * @see VehicleConfigEx#VEHICLE_SPEED_VIDEO_STATE
     */
    private void onVehicleSpeedVideoState() {
        Context context = mOwnerRef.get();
        if (Objects.isNull(context)) {
            return;
        }

        HBroadcastEx.sendLocalBroadcast(
                context, IMediaEvent.EVENT_UPDATE_AUTO_BRAKE_STATUS);
    }

    @Override
    public void onStop(LifecycleOwner owner) {
        LogUtil.v(TAG, "onStop.");
    }

    @Override
    public void onDestroy(@NonNull LifecycleOwner owner) {
        super.onDestroy(owner);
        LogUtil.v(TAG, "onDestroy.");

        H0.removeCallbacksAndMessages(null);
        unregisterSettingsObserver();
    }

    /**
     * 主消息处理器
     * <p> Looper.getMainLooper()
     */
    private static final class TaskHandler extends HHandler {
        private final Reference<MediaServiceObserver> mOwnerRef;

        public TaskHandler(@NonNull Looper looper, MediaServiceObserver owner) {
            super(looper);
            mOwnerRef = new WeakReference<>(owner);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);

            MediaServiceObserver so = mOwnerRef.get();
            if (so != null) {
                so.onHandleMessage(msg);
            }
        }
    }

    /**
     * 处理自定义消息
     * @see TaskHandler
     * @param msg 消息对象
     */
    private void onHandleMessage(@NonNull Message msg) {
        Service service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        // TODO: 处理自定义消息
    }
}
