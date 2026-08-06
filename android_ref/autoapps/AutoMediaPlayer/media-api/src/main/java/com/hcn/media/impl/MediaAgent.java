package com.hcn.media.impl;

import android.annotation.SuppressLint;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.Binder;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.UserHandle;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import com.hcn.common.lang.HLockUtils;
import com.hcn.media.api.IReceptionService;
import com.hcn.media.api.IMediaCallback;
import com.hcn.media.base.IConnectionState;
import com.hcn.media.base.IEvent;
import com.hcn.media.base.IEventCallback;
import com.hcn.media.base.IMedia;
import com.hcn.media.base.IMediaAgent;
import com.hcn.media.base.IMediaApi;
import com.hcn.media.utils.LogUtils;
import com.hcn.media.utils.UtilsEx;
import com.hcn.media_view.lyrics.LyricsRow;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Objects;

/**
 * 访问媒体接口服务
 * @author 65821
 */
class MediaAgent extends Handler implements IMediaAgent, IMedia {
    private static final String TAG = MediaAgent.class.getSimpleName();

    /** 应用上下文环境 **/
    private final Reference<Context> mContextRef;

    /** 客户端 Binder 对象 **/
    private final IBinder mClientBinder = new Binder();

    /** 媒体监听者名称 **/
    private String mClientName;

    /** 媒体事件监听者 **/
    private IEventCallback mCallback;

    /** 多媒体接待服务 **/
    private IReceptionService mMediaService;

    /** 媒体服务连接状态 **/
    private String mConnectionState = IConnectionState.IDLE;

    /**
     * 服务连接状态是否是由用户操作触发的
     * <pre>
     *    连接成功触发 nServiceConnected 回调时；
     *    如果当前触发标记为 true，表示是由用户 bindService 后首次触发的回调;
     *    否则（触发标记为 false），则表示是异常退出后，被系统重启服务导致的连接;
     *    区别：用户直接触发的，我们可以通知媒体直接播放音乐，否则不播放；
     * </pre>
     */
    private boolean mIsUserOperationTriggered = false;

    /**
     * 是否已经触发音乐播放模式
     * <pre>
     *    每次断开连接后，再次连接播放需要执行标记一次；
     *    不调用 MediaService#requestPlayMusicMode 接口可能无法触发音乐播放；
     * </pre>
     */
    private boolean mIsMusicPlayModeTriggered = false;

    /**
     * 连接多媒体接待服务
     * <pre>
     *    多媒体如果没有在运行，将会被从后台拉起；
     *    所有对多媒体的外部操作动作都通过该服务来实现；
     * </pre>
     *
     * @see com.hcn.media.api.IReceptionService
     */
    private ServiceConnection mMediaConnection = new MediaServiceConnection();

    /**
     * 媒体接待服务连接器类
     * <p> 用来确保每次 bind 都是不一样的连接对象；
     *
     * @see IReceptionService
     */
    private final class MediaServiceConnection implements ServiceConnection {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            LogUtils.d("onServiceConnected.");
            mMediaService = IReceptionService.Stub.asInterface(service);

            try {
                // 注册客户端身份信息
                mMediaService.setClientBinder(mClientName, mClientBinder);

                // 监听远程媒体服务状态事件
                mMediaService.setMediaCallback(new IMediaCallback.Stub() {
                    @Override
                    public String clientName() throws RemoteException {
                        return mClientName;
                    }

                    @Override
                    public void onEvent(String event,
                                        String arg0,
                                        String arg1) throws RemoteException {
                        // 速度解除服务绑定（不要再转发）
                        if (IEvent.MEDIA_APP_WILL_EXIT.equals(event)) {
                            unbindMediaService("MEDIA_APP_WILL_EXIT");
                            return;
                        }

                        onRemoteMediaCallback(event, arg0, arg1);
                    }
                });

                // 是用户操作触发的连接状态绑定（通知可以播放）
                if (mIsUserOperationTriggered) {
                    mIsUserOperationTriggered = false;
                    mIsMusicPlayModeTriggered = true;

                    LogUtils.d("onServiceConnected/requestPlayMusicMode");
                    mMediaService.requestPlayMusicMode(TriggerReason.USER_OPERATION);
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }

            // 进入连接状态
            mConnectionState = IConnectionState.CONNECTED;
            postMediaEvent(IEvent.CONNECTION_STATE,
                    IConnectionState.CONNECTED, null);
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            LogUtils.d("onServiceDisconnected.");
            // 当与服务的连接丢失时调用（正常关闭的情况是不会被调用的）；
            // 当托管服务的进程崩溃或被终止时触发 onServiceDisconnected 回调；
            // 这不会删除 ServiceConnection 本身, 与服务的绑定将保持活动状态;
            // 并且在服务下次运行时，您将收到对 onServiceConnected 的调用。

            // 理论上无需重新绑定（待测试效果）
            mConnectionState = IConnectionState.DISCONNECTED;
            postMediaEvent(IEvent.CONNECTION_STATE,
                    IConnectionState.DISCONNECTED, null);

            // 强制结束绑定
            unbindMediaService("onServiceDisconnected");
        }

        @Override
        public void onBindingDied(ComponentName name) {
            ServiceConnection.super.onBindingDied(name);
            LogUtils.d("onBindingDied.");
            // 当到此连接的绑定失效时调用，这意味着接口将永远不会接收到另一个连接；
            // 应用程序需要解除绑定（unbindService）并重新绑定连接才能再次激活它。
            // 例如，如果托管它绑定到的服务的应用程序已经更新，则可能会发生这种情况。

            // 已经解除绑定，不再处理；
            if (Objects.isNull(mMediaService)) {
                return;
            }

            mConnectionState = IConnectionState.DIED;
            postMediaEvent(IEvent.CONNECTION_STATE,
                    IConnectionState.DIED, null);
            unbindMediaService("onBindingDied");
        }

        @Override
        public void onNullBinding(ComponentName name) {
            ServiceConnection.super.onNullBinding(name);
            LogUtils.d("onNullBinding.");
            // 当被绑定的服务从其 onBind() 方法返回 null 时调用;
            // 这表示此 ServiceConnection 所代表的正在尝试的服务绑定将永远无法使用。
            // 请求绑定的应用程序仍然必须调用 unbindService() 来释放与此 ServiceConnection 关联的跟踪资源;
            // 即此回调（onNullBinding）是在 bindService() 之后被调用的。

            // 已经解除绑定，不再处理；
            if (Objects.isNull(mMediaService)) {
                return;
            }

            mConnectionState = IConnectionState.DIED;
            postMediaEvent(IEvent.CONNECTION_STATE,
                    IConnectionState.DIED, null);
            unbindMediaService("onNullBinding");
        }
    };

    /** 禁止实例化无参对象 **/
    private MediaAgent() {
        throw new UnsupportedOperationException("u can't instantiate me...");
    }

    /** 默认的带参构造函数 **/
    public MediaAgent(@NonNull Context context) {
        super(Looper.getMainLooper());

        mContextRef = new WeakReference<>(context);
    }

    /**
     * 返回当前关联的 {@link Context}
     *
     * @throws IllegalStateException 如果当前未与上下文相关联。
     * @see #mContextRef
     */
    @NonNull
    public final Context requireContext() {
        Context context = mContextRef.get();
        if (context == null) {
            throw new IllegalStateException("HMediaApi " + this + " not attached to a context.");
        }
        return context;
    }

    @Override
    public void setClientName(@NonNull String name) {
        mClientName = name;
    }

    @Override
    public void setMediaCallback(IEventCallback callback) {
        mCallback = callback;

        // 取消媒体服务绑定
        if (Objects.isNull(callback)) {
            unbindMediaService("setMediaCallback(null)");
        }
    }

    @Override
    public boolean isConnectionState(@NonNull String state) {
        return mConnectionState == state;
    }

    @Override
    public String connectionState() {
        return mConnectionState;
    }

    /**
     * 获取指定文件歌词信息
     * <p> 媒体服务连接成功后有效；
     *
     * @param path 歌曲文件路径
     * @return {@link LyricsRow}
     */
    @Override
    public List<LyricsRow> getLyricsRowInfo(String path) {
        if (Objects.isNull(mMediaService)) {
            LogUtils.w("getLyricsRowInfo: Media service not connected!");
            return null;
        }

        try {
            return mMediaService.getLyricsInfo(path);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return null;
    }

    @SuppressLint("NewApi")
    @Override
    public void requestStartApp(String reason) {
        // 是连接状态，说明媒体已经在运行状态；
        if (isConnectionState(IConnectionState.CONNECTED)) {
            LogUtils.d("requestStartApp: is connected state!");
            return;
        }

        // 是连接中状态，说明已经请求启动任务；
        if (isConnectionState(IConnectionState.CONNECTING)) {
            LogUtils.d("requestStartApp: is connecting state!");
            return;
        }

        // 判断进程是否在运行状态
        if (UtilsEx.isServiceRunning(MEDIA_SERVICE_CLASS_NAME)) {
            LogUtils.d("requestStartApp: media service is running!");
            return;
        }

        // 打赢触发调用的原因
        LogUtils.d("requestStartApp/" + reason);

        // 开始执行绑定服务（拉起媒体）
        PackageManager pm = requireContext().getPackageManager();
        if (pm != null) {
            try {
                // android.uid.system 的 uid 是 1000，正规点直接动态去获取；
                int uid = pm.getApplicationInfo(requireContext().getPackageName(), 0).uid;
                tryBindMediaService(uid, !"no-auto-play".equals(reason));
            } catch (PackageManager.NameNotFoundException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void requestBindApp(String reason) {
        // 只 bind 服务，不自动播放；
        if (Objects.isNull(mMediaService)) {
            requestStartApp("no-auto-play");
        }
    }

    /**
     * 尝试绑定媒体服务
     * @param uid 当前执行程序的 uid
     * @param autoPlay 绑定成功后自动播放；
     */
    @RequiresApi(api = Build.VERSION_CODES.R)
    private void tryBindMediaService(int uid, boolean autoPlay) {
        // 已经绑定成功（过滤）
        if (!Objects.isNull(mMediaService)) {
            LogUtils.w("tryBindMediaService:" +
                    " Is media reception service bind state!");
            return;
        }

        // 禁止重复执行绑定动作
        if (isConnectionState(IConnectionState.CONNECTED)
                || isConnectionState(IConnectionState.CONNECTING)) {
            LogUtils.w("tryBindMediaService:" +
                    " Is media reception service " + connectionState() + " state!");
            return;
        }

        // 构建绑定服务意图
        Intent intent = new Intent(MEDIA_SERVICE_ACTION);
        intent.setClassName(MEDIA_SERVICE_PACKAGE_NAME, MEDIA_SERVICE_CLASS_NAME);
        intent.putExtra(START_REASON_EXTRA_KEY, "BIND_MEDIAPLAYER");

        // 构建绑定服务连接器
        if (Objects.isNull(mMediaConnection)) {
            mMediaConnection = new MediaServiceConnection();
        }

        // 执行绑定服务动作
        boolean bindResult;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            bindResult = UtilsEx.bindServiceAsUser(
                    requireContext(), intent, mMediaConnection,
                    Context.BIND_AUTO_CREATE, UserHandle.getUserHandleForUid(uid));
        } else {
            bindResult = requireContext().bindService(
                    intent, mMediaConnection, Context.BIND_AUTO_CREATE);
        }

        // 绑定执行成功（等待结果）
        if (bindResult) {
            LogUtils.d("execute/bindService.");

            // 执行成功，进入媒体服务连接中状态
            mConnectionState = IConnectionState.CONNECTING;
            postMediaEvent(IEvent.CONNECTION_STATE,
                    IConnectionState.CONNECTING, null);

            // 必须在分发连接状态（postMediaEvent 会重置它）后调用
            mIsUserOperationTriggered = autoPlay;

            // 如果 bindService 后 5s 都不返回结果，我们认为绑定异常
            sendEmptyMessageDelayed(
                    H.MSG_BIND_SERVICE_TIMEOUT, 5000);
        } else {
            LogUtils.e("tryBindMediaService: bindService return false!");
        }
    }

    /**
     * 取消媒体服务绑定动作
     * <pre>
     *    多次重复调用会报错，所以只有 bind 过才可以 unbind;
     *    注意：如服务端进程已退出后再调用 unbind，系统还是会拉起服务端进程；
     *    所以：最好的做法是在服务端进程退出前 unbind 解除绑定关系；
     *    这个函数会被多线程调用，所以需要添加安全锁；
     * </pre>
     *
     * @param caller 调用者
     */
    private void unbindMediaService(String caller) {
        // 无绑定操作，无需解除绑定
        if (isConnectionState(IConnectionState.IDLE)) {
            return;
        }

        // 自动锁（可避免多线程重入）
        try (HLockUtils.AutoLock autoLock =
                     HLockUtils.getAutoLock("mediaApi-unbind")) {
            // 重复调用 unbindService 会异常
            if (mMediaService != null && mMediaConnection != null) {
                LogUtils.d("execute/unbindService. caller: " + caller);
                requireContext().unbindService(mMediaConnection);
                mMediaConnection = null;
                mMediaService = null;
            }
        } catch (Exception ignored) {
        }

        // 更新连接状态到 IDLE
        mConnectionState = IConnectionState.IDLE;
        postMediaEvent(IEvent.CONNECTION_STATE,
                IConnectionState.IDLE, null);
    }

    /**
     * 检查并切换到音乐模式
     *
     * @param reason 触发原因
     * @return 当前是否在音乐模式；
     */
    private boolean checkAndSwitchToMusicMode(@NonNull String reason) {
        if (Objects.isNull(mMediaService)) {
            LogUtils.w("checkAndSwitchToMusicMode: " +
                    "The media reception service is null!");
            return false;
        }

        try {
            // 检查当前媒体进程的媒体类型
            int mediaType = mMediaService.getCurrentMediaType();
            if (mediaType == Type.MEDIA_TYPE_MUSIC
                    && mIsMusicPlayModeTriggered) {
                // 播放模式也需要是触发过的，不然无法播放；
                return true;
            }

            LogUtils.w("checkAndSwitchToMusicMode/" + reason);

            mIsMusicPlayModeTriggered = true;
            mMediaService.requestPlayMusicMode(reason);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return false;
    }

    /**
     * 执行音乐 Api
     * @param mediaApi {@link IMediaApi}
     * @return -1: 未绑定音乐服务；-2: 当前不是音乐模式，需要手动再次调用执行音乐 API；0：成功执行音乐 API
     */
    @Override
    public int requestExecuteMusicApi(@IMediaApi String mediaApi) {
        if (Objects.isNull(mMediaService)) {
            LogUtils.w("requestExecuteMusicApi: Media service not connected!");
            requestStartApp(mediaApi);
            return -1;
        }

        // 检查当前媒体模式到音乐
        if (!checkAndSwitchToMusicMode(TriggerReason.USER_OPERATION)) {
            LogUtils.w("requestExecuteMusicApi: Current not in music mode!");
            return -2;
        }

        try {
            mMediaService.requestExecuteMusicApi(mediaApi, "media-api");
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return 0;
    }

    @Override
    public void requestExitApp(String reason) {
        if (Objects.isNull(mMediaService)) {
            LogUtils.e("requestExitApp: Media service not connected!");
            return;
        }

        try {
            mMediaService.requestExitApp(reason);
            unbindMediaService("requestExitApp");
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    /**
     * 处理多媒体服务事件
     *
     * @param event {@link IEvent}
     * @param arg0 附加参数 1
     * @param arg1 附加参数 2
     */
    private void onRemoteMediaCallback(String event, String arg0, String arg1) {
        LogUtils.d("onRemoteMediaCallback: "
                + event + ", arg0 = " + arg0 + ", arg1 = " + arg1);

        // 避免把服务端搞阻塞了
        postDelayed(() -> {
            // 过滤对外的媒体事件（限制）
            switch (event) {
                case IEvent.MUSIC_PLAY_INFO:
                case IEvent.MUSIC_PLAY_STATE:
                case IEvent.MUSIC_PLAY_TIME:
                case IEvent.MUSIC_LYRICS_INFO:
                    postMediaEvent(event, arg0, arg1);
                    break;
                default:
                    break;
            }
        }, 10);
    }

    /**
     * 对外抛出事件状态
     *
     * @param event 事件类型 {@link IEvent}
     * @param obj0 附加数据对象 1
     * @param obj1 附加数据对象 2
     */
    private void postMediaEvent(@IEvent String event,
                                Object obj0,
                                Object obj1) {
        // 过滤处理特殊事件状态
        switch (event) {
            case IEvent.CONNECTION_STATE:
                // 连接状态改变移除连接超时消息
                removeMessages(H.MSG_BIND_SERVICE_TIMEOUT);

                // 重置用户操作触发标记
                mIsUserOperationTriggered = false;

                // 非连接状态重置播放模式触发标记
                if (!isConnectionState(IConnectionState.CONNECTED)) {
                    mIsMusicPlayModeTriggered = false;
                }
                break;
            case IEvent.MEDIA_EXTEND_EVENT:
            default:
                break;
        }

        // 回调事件给客户端对象
        if (mCallback != null) {
            mCallback.onEvent(event, obj0, obj1);
        }
    }

    /** 消息定义 **/
    private interface H {
        int MSG_NONE = -1;

        /** bindService 连接状态超时 **/
        int MSG_BIND_SERVICE_TIMEOUT = 1;
    }

    @Override
    public void handleMessage(@NonNull Message msg) {
        super.handleMessage(msg);

        switch (msg.what) {
            case H.MSG_BIND_SERVICE_TIMEOUT:
                onMsgBindMediaServiceTimeout();
                break;
            case H.MSG_NONE:
            default:
                break;
        }
    }

    /**
     * 绑定多媒体服务超时
     * <pre>
     *    如果 bindService 执行成功后，超过 5s 得不到连接状态反馈，我们可以重置连接状态；
     *    系统设备是否存在有这种情况，还是待观察（预留处理，为了测试覆盖率）；
     * </pre>
     * @see H#MSG_BIND_SERVICE_TIMEOUT
     */
    private void onMsgBindMediaServiceTimeout() {
        LogUtils.e("onMsgBindMediaServiceTimeout.");

        // 超时也需要解除绑定
        unbindMediaService("timeout");
    }
}
