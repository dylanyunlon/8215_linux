package com.hcn.media.external;

import static java.util.concurrent.TimeUnit.MILLISECONDS;

import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import com.hcn.common.lang.HThreadUtils;
import com.hcn.common.lang.RunnableEx;
import com.hcn.media.api.IReceptionService;
import com.hcn.media.api.IMediaCallback;
import com.hcn.media.base.IEvent;
import com.hcn.media.base.IMedia;
import com.hcn.media.base.IMediaApi;
import com.hcn.media.external.client.ClientDeathRecipient;
import com.hcn.media.external.client.ClientInfo;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_view.lyrics.LyricsRow;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * 对外接口 Binder 类
 * <p> 主要是用来对外提供访问媒体的接口；
 *
 * @author 65821
 */
public class ReceptionBinder extends IReceptionService.Stub {
    public static final String TAG = ReceptionService.TAG;

    /**
     * 客户端 Binder 对象；
     * <p> 存储客户端 Binder 对象，Map<clientName, clientBinder>;
     */
    private final ConcurrentHashMap<String, ClientInfo> mClientBinderMap = new ConcurrentHashMap<>();

    /**
     * 主线程消息处理器封装
     * <p> 處理 Binder 线程事情请求；
     */
    protected final Handler mHandler = new Handler(Looper.getMainLooper());

    /**
     * 当前 Binder 所有者引用
     * <pre>
     *    只有当 ReceptionService 对象是弱可达的时候才会被回收；
     *    ReceptionService 是服务组件，在 ActivityThread 中存在强引用；
     * </pre>
     */
    private final Reference<ReceptionService> mOwnerRef;

    public ReceptionBinder(ReceptionService service) {
        mOwnerRef = new WeakReference<>(service);
    }

    /**
     * 检查接口调用有效性
     * <p> 因为是 SDK 接口类，需要约束指引使用者正确调用；
     * @return {@link ReceptionService}
     */
    private ReceptionService checkOwnerValidity() {
        ReceptionService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            LogUtil.w(TAG, "checkOwnerValidity: The service is invalid!");
        }
        return service;
    }

    /**
     * 调用者有效性检查
     * <p> 如果是被主线程调用，将直接抛运行异常；
     */
    private void callerValidityCheck() {
        // 当前不在主线程
        if (HThreadUtils.isMainThread()) {
            throw new RuntimeException("ogc#" +
                    "media/interface calls from main threads!");
        }
    }

    @Override
    public void setClientBinder(String name,
                                IBinder iBinder) throws RemoteException {
        callerValidityCheck();

        // 参数有效性检查
        if (Objects.isNull(iBinder)
                || TextUtils.isEmpty(name)) {
            throw new RemoteException("parameter cannot be empty!");
        }

        // 服务有效性检查
        ReceptionService service = checkOwnerValidity();
        if (Objects.isNull(service)) {
            return;
        }

        addClientBinder(name, iBinder);
    }

    /**
     * 设置客户端 Binder 对象
     *
     * @param name 客户端名字（建议唯一）
     * @param client 客户端 binder
     */
    private void addClientBinder(@NonNull String name,
                                 @NonNull IBinder client) throws RemoteException {
        // 客户端信息维护对象禁止被同时访问
        synchronized (mClientBinderMap) {
            ClientInfo clientInfo = mClientBinderMap.get(name);
            if (!Objects.isNull(clientInfo)) {
                throw new RemoteException("cannot repeatedly register client objects!");
            }

            // 创建客户端相关信息
            clientInfo = new ClientInfo(name, client);
            clientInfo.setDeathRecipient(new ClientDeathRecipient(clientInfo, new RunnableEx() {
                @Override
                public void callback(Object obj) {
                    // 参数必须是字符串类型
                    if (!(obj instanceof String)) {
                        return;
                    }

                    synchronized (mClientBinderMap) {
                        String clientName = (String) obj;
                        mClientBinderMap.remove(clientName);
                        Log.w(TAG, "binderDied: " + clientName);
                    }
                }
            }));

            // 管理并监听客户端信息
            mClientBinderMap.put(name, clientInfo);
            client.linkToDeath(clientInfo.getDeathRecipient(), 0);
        }
    }

    @Override
    public void setMediaCallback(IMediaCallback iMediaCallback) throws RemoteException {
        callerValidityCheck();

        // 参数有效性检查
        if (Objects.isNull(iMediaCallback)) {
            return;
        }

        // 服务有效性检查
        ReceptionService service = checkOwnerValidity();
        if (Objects.isNull(service)) {
            return;
        }

        String name = iMediaCallback.clientName();
        if (!TextUtils.isEmpty(name)) {
            addMediaCallback(name, iMediaCallback);
        }
    }

    @Override
    public int getCurrentMediaType() throws RemoteException {
        callerValidityCheck();

        // 服务有效性检查
        ReceptionService service = checkOwnerValidity();
        if (Objects.isNull(service)) {
            return IMedia.Type.MEDIA_TYPE_IDLE;
        }

        return service.currentMediaType();
    }

    @Override
    public List<LyricsRow> getLyricsInfo(String path) throws RemoteException {
        callerValidityCheck();

        // 服务有效性检查
        ReceptionService service = checkOwnerValidity();
        if (Objects.isNull(service)) {
            return null;
        }

        return service.getCurrentPlayLyricsInfo(path);
    }

    @Override
    public int requestPlayMusicMode(String reason) throws RemoteException {
        callerValidityCheck();

        // 服务有效性检查
        ReceptionService service = checkOwnerValidity();
        if (Objects.isNull(service)) {
            return -1;
        }

        // 不要在 Binder 的调用栈中使用反射接口；
        AtomicInteger result = new AtomicInteger(-1);
        Lock lock = new ReentrantLock();
        Condition condition = lock.newCondition();

        try {
            lock.lock();
            mHandler.post(() -> {
                result.set(service.tryRequestEnterMusicMode(reason, true));
                lock.lock();
                condition.signal();
                lock.unlock();
            });

            boolean timeout = condition.await(1000, MILLISECONDS);
            Log.d(TAG, "requestPlayMusicMode: await = " + timeout);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        } finally {
            lock.unlock();
        }

        return result.get();
    }

    /**
     * 增加客户端的名字
     *
     * @param iMediaCallback
     * @throws RemoteException
     */
    private void addMediaCallback(@NonNull String name,
                                  @NonNull IMediaCallback iMediaCallback) throws RemoteException {
        // 客户端信息维护对象禁止被同时访问
        synchronized (mClientBinderMap) {
            if (mClientBinderMap.isEmpty()) {
                LogUtil.w(TAG, "addMediaCallback: " +
                        "Please call setClientBinder to register as an identity!");
                return;
            }

            ClientInfo clientInfo = mClientBinderMap.get(name);
            if (clientInfo != null) {
                clientInfo.setCallback(iMediaCallback);
            }
        }
    }

    /**
     * 发送媒体对外事件接口
     * <p> 主要事件参见  {@link IEvent} 的定义
     *
     * @param event 媒体信息事件
     * @param arg0 事件附加参数 1
     * @param arg1 事件附加参数 2
     */
    /* public */ void postMediaEvent(@IEvent String event,
                                     String arg0,
                                     String arg1) {
        // 服务有效性检查
        ReceptionService service = checkOwnerValidity();
        if (Objects.isNull(service)) {
            return;
        }

        // 需要及时处理的事件
        switch (event) {
            case IEvent.MEDIA_APP_WILL_EXIT:
                dispatchMediaEvent(event, arg0, arg1);
                return;
            case IEvent.MEDIA_EXTEND_EVENT:
            default:
                break;
        }

        // 不要在当前调用栈直接分发事件（养成好习惯）
        service.postRunnable(() -> {
            dispatchMediaEvent(event, arg0, arg1);
        });
    }

    /**
     * 分发媒体对外事件信息
     * <p> 主要事件参见  {@link IEvent} 的定义
     *
     * @param event 媒体信息事件
     * @param arg0 事件附加参数 1
     * @param arg1 事件附加参数 2
     * @see #postMediaEvent(String, String, String)
     */
    private void dispatchMediaEvent(@IEvent String event,
                                    String arg0,
                                    String arg1) {
        // 客户端信息维护对象禁止被同时访问
        synchronized (mClientBinderMap) {
            for (ClientInfo clientInfo : mClientBinderMap.values()) {
                IMediaCallback callback = clientInfo.getCallback();
                if (Objects.isNull(callback)) {
                    continue;
                }

                try {
                    callback.onEvent(event, arg0, arg1);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    @Override
    public void requestExecuteMusicApi(String mediaApi,
                                       String reason) throws RemoteException {
        callerValidityCheck();

        // 服务有效性检查
        ReceptionService service = checkOwnerValidity();
        if (Objects.isNull(service)) {
            return;
        }

        mHandler.post(() -> {
            switch (mediaApi) {
                case IMediaApi.PLAY:
                    service.tryTriggerMusicPlay(reason);
                    break;
                case IMediaApi.PAUSE:
                    service.tryTriggerMusicPause(reason);
                    break;
                case IMediaApi.PLAY_PAUSE:
                    service.tryTriggerMusicPlayPause(reason);
                    break;
                case IMediaApi.NEXT:
                    service.tryTriggerMusicNextTrack(reason);
                    break;
                case IMediaApi.PREV:
                    service.tryTriggerMusicPrevTrack(reason);
                    break;
                case IMediaApi.PLAY_MODE:
                    service.tryTriggerMusicPlayMode(reason);
                    break;
                case IMediaApi.PLAY_INFO:
                    service.tryRequestMusicPlayInfo(reason);
                    break;
                default:
                    break;
            }
        });
    }

    @Override
    public void requestExitApp(String reason) throws RemoteException {
        callerValidityCheck();

        // 服务有效性检查
        ReceptionService service = checkOwnerValidity();
        if (Objects.isNull(service)) {
            return;
        }

        mHandler.post(() -> service.requestExitApp(reason));
    }
}
