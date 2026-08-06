package com.hcn.media.external.observer;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.text.TextUtils;

import androidx.annotation.NonNull;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleOwner;

import com.hcn.common.misc.HBroadcastUtils;
import com.hcn.media.base.service.ServiceLifecycleObserver;
import com.hcn.media.external.debug.BroadcastApi;
import com.hcn.media.external.debug.BroadcastApi.ITypeValue;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_common.HBroadcastEx;
import com.hcn.media_common.HMessage;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_data.AppGlobalData;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.Objects;
import java.util.concurrent.ExecutorService;

/**
 * 媒体接口服务观察者
 * <pre>
 *    主要是观察 ReceptionService 生命周期，简化服务类中的代码逻辑；
 *    后续外部接口服务的扩展功能都可以放到这里处理（例如：外部广播事件）；
 * </pre>
 *
 * @author 65821
 */
public class ReceptionServiceObserver extends ServiceLifecycleObserver {
    private static final String TAG = ReceptionServiceObserver.class.getSimpleName();

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
     * 本地广播事件接收者
     * <p> 监听由本地 Service/Model 下发的媒体相关状态事件；
     */
    private BroadcastReceiver mMediaEventReceiver;

    /** 禁止构造无参观察者对象 **/
    private ReceptionServiceObserver() {
        throw new UnsupportedOperationException("u can't instantiate me...");
    }

    /**
     * 构造函数
     * <p> 注意构造函数尽量只干成员初始化工作；
     *
     * @param lifecycleOwner 所有者
     * @param service 上下文环境
     * @param executorService 线程池
     */
    public ReceptionServiceObserver(
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

    /**
     * 获取当前 Observer 依赖的上下文对象
     * <p> 只要是正常流程创建的 Observer，它都有上下文环境；
     *
     * @return 上下文对象
     */
    @NonNull
    protected final Context requireContext() {
        Context context = mOwnerRef.get();
        if (context == null) {
            throw new IllegalStateException("ReceptionService is invalid!");
        }
        return context;
    }

    @Override
    public void onCreate(LifecycleOwner owner) {
        connectMediaEventReceiver();

        // 监听外部的调试接口事件
        BroadcastApi.registerDebugReceiver((event, obj) -> {
            switch (event) {
                case ITypeValue.START_BACKGROUND_PLAYBACK:
                case ITypeValue.TRIGGER_PLAY_TRACK:
                case ITypeValue.TRIGGER_PAUSE_TRACK:
                case ITypeValue.TRIGGER_PLAY_PAUSE_TRACK:
                case ITypeValue.TRIGGER_SWITCH_PREV_TRACK:
                case ITypeValue.TRIGGER_SWITCH_NEXT_TRACK:
                    // 统一分发给接待服务处理
                    dispatchMediaEvent(HMessage.obtain(
                            IMediaEvent.EVENT_DEBUG_MEDIA_API, event, obj));
                    break;
                default:
                    break;
            }
        });
    }

    @Override
    public void onStart(LifecycleOwner owner) {
        // TODO: 不处理
    }

    /**
     * 处理媒体状态事件
     *
     * @param event {@link IMediaEvent}
     * @param arg 整形参数
     * @param obj 附加对象参数
     */
    public void onLocalMediaEvent(int event, int arg, Object obj) {
        // 只需要处理与后台播放相关联的媒体事件
        switch (event) {
            case IMediaEvent.EVENT_SERVICE_INITIALIZED:
            case IMediaEvent.EVENT_PRE_NORMAL_EXIT_APP:
                // 回调给 ReceptionService 处理
                dispatchMediaEvent(HMessage.obtain(event, arg, obj));
                break;
            case IMediaEvent.EVENT_NONE:
            default:
                break;
        }
    }

    /**
     * 接入本地媒体事件广播接收者
     * <pre>
     *    这是一个模拟广播机制的 callback 事件分发组件；
     *    我们可以用它实现监听 MediaPlayer 核心组件下发的事件状态；
     * </pre>
     */
    private void connectMediaEventReceiver() {
        if (Objects.isNull(mMediaEventReceiver)) {
            mMediaEventReceiver = new MediaEventReceiver(this);
        } else {
            LogUtil.w(TAG, "Function connectMediaEventReceiver called repeatedly!");
            return;
        }

        // 注册本地事件广播接收者
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(HBroadcastEx.SpecialChain.ACTION_LOCAL_CALLBACK);
        HBroadcastUtils.getInstance(requireContext())
                .registerReceiver(mMediaEventReceiver, intentFilter);
    }

    /**
     * 断开本地媒体事件广播接收者
     *
     * @see #connectMediaEventReceiver() 接口
     */
    private void disconnectMediaEventReceiver() {
        if (Objects.isNull(mMediaEventReceiver)) {
            return;
        }

        HBroadcastUtils.getInstance(requireContext())
                .unregisterReceiver(mMediaEventReceiver);
        mMediaEventReceiver = null;
    }

    /**
     * 本地广播事件处理类
     * <p> 由本地 Service/Model 下发的媒体播放相关事件状态；
     */
    private static final class MediaEventReceiver extends BroadcastReceiver {
        private final Reference<ReceptionServiceObserver> mOwnerRef;

        public MediaEventReceiver(ReceptionServiceObserver observer) {
            super();
            mOwnerRef = new WeakReference<>(observer);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (TextUtils.isEmpty(action)) {
                return;
            }

            // 事件观察者有效性检查
            ReceptionServiceObserver observer = mOwnerRef.get();
            if (Objects.isNull(observer)) {
                return;
            }

            // 处理本地广播事件（Service/Model）
            if (action.equals(HBroadcastEx.SpecialChain.ACTION_LOCAL_CALLBACK)) {
                // 读取本地事件广播参数
                int event = intent.getIntExtra(
                        HBroadcastEx.SpecialChain.EXTRA_CALLBACK_TYPE, IMediaEvent.EVENT_NONE);
                String data = intent.getStringExtra(HBroadcastEx.SpecialChain.EXTRA_CALLBACK_DATA);
                observer.onLocalMediaEvent(event, 0, data);
            }
        }
    }

    @Override
    public void onDestroy(LifecycleOwner owner) {
        super.onDestroy(owner);
        disconnectMediaEventReceiver();
        BroadcastApi.unregisterDebugReceiver();
    }
}
