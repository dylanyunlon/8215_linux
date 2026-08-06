package com.hcn.media_model.base;

import android.content.Context;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;

import androidx.annotation.NonNull;

import com.hcn.common.concurrent.NamedThreadFactory;
import com.hcn.common.utils.HHandler;
import com.hcn.media_data.AppGlobalData;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.Objects;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * 基础 Model
 * <pre>
 *    由于历史原因应用组件对象被滥用了，导致代码结构非常不清晰；
 *    我们这里借鉴 MVVM/MVP 的思想，在不影响历史出货产品的情况下，对代码上做改动和重构；
 *    目标：尽可能的弱化 Application 对象的作用，让代码层次更加清晰；
 * </pre>
 *
 * @author 65821
 */
public abstract class BaseModel {

    /** 应用上下文环境引用 **/
    protected final Reference<Context> mContextRef;

    /** 应用全局数据对象引用 **/
    protected static AppGlobalData sAppData = null;

    /**
     * 消息处理器对象（主线程）
     * <p> 可以用来处理线程交互、延时任务等；
     */
    protected final HHandler H0;

    /**
     * 数据库操作需要在线程中处理
     * <p> 安全起见，我们只拉一个线程；
     */
    private ExecutorService mExecutorService;

    /**
     * 当前模式是否是关闭状态
     * <p> 关闭状态下不可以再执行其它外部请求任务；
     */
    private boolean mIsClosed = false;

    /**
     * BaseModel 构造函数
     *
     * @param context 上下文环境
     * @param name 线程池的名字（如果为空，不创建线程池）
     */
    public BaseModel(Context context, String name) {
        mContextRef = new WeakReference<>(context);
        sAppData = AppGlobalData.getInstance();

        // 创建任务执行相关对象
        H0 = new TaskHandler(Looper.getMainLooper(), this);
        if (!TextUtils.isEmpty(name)) {
            mExecutorService = new ThreadPoolExecutor(1, 5, 1,
                    TimeUnit.MINUTES, new LinkedBlockingQueue<>(), new NamedThreadFactory(name) {
                @Override
                public Thread newThread(Runnable r) {
                    Thread thread =  super.newThread(r);
                    int priority = thread.getPriority();
                    if (priority != Thread.NORM_PRIORITY) {
                        thread.setPriority(Thread.NORM_PRIORITY);
                    }
                    return thread;
                }
            });
        }
    }

    /**
     * 获取当前 Model 依赖的上下文对象
     * <p> 只要是正常流程创建的 Model，它都是当前应用全局上下文环境；
     *
     * @return 上下文对象
     */
    @NonNull
    protected final Context requireContext() {
        Context context = mContextRef.get();
        if (context == null) {
            throw new IllegalStateException("BaseModel " + this + " not attached to a context.");
        }
        return context;
    }

    /** 当前模式是否是关闭状态 **/
    protected boolean isClosed() {
        return mIsClosed;
    }

    /** 关闭函数（释放相关资源）**/
    protected void close() {
        if (mIsClosed) {
            return;
        }

        mIsClosed = true;
        shutdownAllAsyncTask();
        H0.removeCallbacksAndMessages(null);
    }

    /**
     * 实现该接口可以处理自定义消息
     * @param msg 消息对象
     */
    protected void onHandleMessage(@NonNull Message msg) {
        // TODO: 扩展
    }

    /**
     * 任务处理器（主线程）
     * <p> 消息处理器，对所有继承 BaseModel 扩展使用；
     */
    private static class TaskHandler extends HHandler {
        /** 当前所有者对象引用 */
        private final Reference<BaseModel> mOwnerRef;

        public TaskHandler(@NonNull Looper looper, BaseModel owner) {
            super(looper);
            mOwnerRef = new WeakReference<>(owner);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            BaseModel owner = mOwnerRef.get();
            if (owner != null) {
                owner.onHandleMessage(msg);
            }
        }
    }

    /**
     * 执行异步任务
     * @param runnable 任务实体
     */
    protected void executeAsyncTask(@NonNull Runnable runnable) {
        if (Objects.isNull(mExecutorService)) {
            return;
        }

        mExecutorService.execute(runnable);
    }

    /** 关闭所有的异步任务 **/
    private void shutdownAllAsyncTask() {
        // 线程池对象有效性检查
        if (Objects.isNull(mExecutorService)) {
            return;
        }

        // 启动有序关闭，不再接受新任务
        mExecutorService.shutdown();

        // 等待线程池中所有线程运行完成
        while (!mExecutorService.isTerminated()) {
            try {
                TimeUnit.MILLISECONDS.sleep(50);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}
