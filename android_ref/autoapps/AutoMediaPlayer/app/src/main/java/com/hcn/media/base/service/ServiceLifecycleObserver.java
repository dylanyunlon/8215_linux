package com.hcn.media.base.service;

import androidx.annotation.NonNull;
import androidx.lifecycle.DefaultLifecycleObserver;
import androidx.lifecycle.LifecycleOwner;

import com.hcn.media_common.HMessage;
import com.hcn.rxrelay3.PublishRelay;

import io.reactivex.rxjava3.disposables.CompositeDisposable;

/**
 * 服务生命周期观察者接口
 * @author 65821
 */
public class ServiceLifecycleObserver implements DefaultLifecycleObserver {
    /**
     * 事件中继器: 对外分发事件
     * <pre>
     *    为当前观察者对象所有者交互事件提供中继服务；
     *    HMessage 提供了消息对象池，使用后需要释放；
     * </pre>
     */
    protected PublishRelay<HMessage> mEventRelay = PublishRelay.create();

    /** Disposable 对象管理器 **/
    protected CompositeDisposable mCompositeDisposable = new CompositeDisposable();

    /**
     * 通知发生 ON_CREATE 事件。
     * <p> 将在调用 LifecycleOwner的 onCreate 方法之前调用此方法。
     *
     * @param owner 状态已更改的组件
     */
    @Override
    public void onCreate(LifecycleOwner owner) {
    }

    /**
     * 事件中继器接口对象
     * <pre>
     *    发送消息:
     *    eventRelay().accept(new HMessage(...));
     *    接收消息:
     *    Disposable d = eventRelay().subscribe(hMessage -> {
     *        Logger.t(TAG).v("Receive messages: " + hMessage);
     *    });
     *    d.dispose();
     * </pre>
     *
     * @return 中继器对象
     */
    public PublishRelay<HMessage> eventRelay() {
        return mEventRelay;
    }

    /**
     * 调度观察者事件
     * <pre>
     *    调度给所有事件观察者；
     *    为提高效率，事件使用完会被回收处理；
     * </pre>
     *
     * @param message 事件封装
     */
    protected void dispatchMediaEvent(@NonNull HMessage message) {
        // 调用处理是阻塞的
        eventRelay().accept(message);

        // 回收消息事件
        message.recycle();
    }

    /**
     * 通知发生 ON_START 事件。
     * <p> 将在调用 LifecycleOwner的 onStart 方法之前调用此方法。
     *
     * @param owner 状态已更改的组件
     */
    @Override
    public void onStart(LifecycleOwner owner) {
    }

    /**
     * 通知发生 ON_STOP 事件。
     * <p> 将在调用 LifecycleOwner的 onStop 方法之前调用此方法。
     *
     * @param owner 状态已更改的组件
     */
    @Override
    public void onStop(LifecycleOwner owner) {
    }

    /**
     * 通知发生 ON_DESTROY 事件。
     * <p> 将在调用 LifecycleOwner的 onDestroy 方法之前调用此方法。
     *
     * @param owner 状态已更改的组件
     */
    @Override
    public void onDestroy(LifecycleOwner owner) {
        mCompositeDisposable.dispose();
    }
}
