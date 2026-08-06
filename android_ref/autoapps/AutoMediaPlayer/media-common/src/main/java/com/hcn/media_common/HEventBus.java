package com.hcn.media_common;

import com.google.common.eventbus.EventBus;
import com.google.common.eventbus.AsyncEventBus;
import java.util.concurrent.Executor;

/**
 * [推荐解耦使用]
 * @author 86158
 */
public final class HEventBus {
    private static EventBus eventBus = null;
    private static AsyncEventBus asyncEventBus = null;

    private static final Executor executor = new Executor() {
        @Override
        public void execute(Runnable command) {
            new Thread(command).start();
        }
    };

    /**
     * [双重锁单例模式]
     * @return AsyncEventBus 对象
     */
    private static AsyncEventBus getAsyncEventBus() {
        if(null == asyncEventBus){
            synchronized (AsyncEventBus.class) {
                if(null == asyncEventBus){
                    asyncEventBus = new AsyncEventBus(executor);
                }
            }
        }

        return asyncEventBus;
    }

    /**
     * [双重锁单例模式]
     * @return EventBus 对象
     */
    private static EventBus getEventBus() {
        if(null == eventBus){
            synchronized (EventBus.class) {
                if(null == eventBus){
                    eventBus = new EventBus();
                }
            }
        }

        return eventBus;
    }

    /**
     * [发送同步事件]
     * @param event 事件对象
     */
    public static void post(Object event){
        getEventBus().post(event);
    }

    /**
     * [发送异步事件]
     * @param event 事件对象
     */
    public static void asyncPost(Object event){
        getAsyncEventBus().post(event);
    }

    /**
     *  [注册同步事件]
     * @param object 事件对象
     */
    public static void register(Object object) {
        getEventBus().register(object);
    }
    public static void unregister(Object object) {
        getEventBus().unregister(object);
    }

    /**
     *  [注册异步事件]
     * @param object 事件对象
     */
    public static void registerAsync(Object object) {
        getAsyncEventBus().register(object);
    }
    public static void unregisterAsync(Object object) {
        getAsyncEventBus().unregister(object);
    }
}
