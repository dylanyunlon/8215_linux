package com.hcn.media_common.thread;

import android.os.Handler;
import android.os.Looper;

import androidx.annotation.MainThread;

import com.hcn.common.concurrent.NamedThreadFactory;
import com.hcn.common.lang.HThreadUtils;
import com.hcn.common.lang.RunnableEx;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.Objects;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * 公共任务执行者
 * <pre>
 *    就是一个公共的线程池，让整个进程都可以使用它，避免重复创建线程池对象；
 *    禁止在非主线程调用它的提交任务方法 {@link #submitTask(RunnableEx) #...}，否则异常；
 * </pre>
 *
 * @author 65821
 */
public class HPublicExecutor {

    /** 唯一实例 */
    private static HPublicExecutor sInstance = null;

    /** 线程池对象 */
    private final ExecutorService mAsyncTaskExecutor;

    /**
     * 线程 ID
     * <p> 每创建一个新的线程，就自加一次；
     */
    private int mThreadId = 0;

    /** 主线程消息处理器 */
    private final Handler mMainHandler;

    /** 公共执行器唯一实例 */
    public static HPublicExecutor instance() {
        if (sInstance == null) {
            sInstance = new HPublicExecutor();
        }

        return sInstance;
    }

    private HPublicExecutor() {
        // 构建检查(主线程)
        callerValidityCheck();

        // 主线程消息处理器
        mMainHandler = new Handler(Looper.getMainLooper());

        // 公共异步任务线程池
        mAsyncTaskExecutor = new ThreadPoolExecutor(0, 5,
                30L, TimeUnit.SECONDS, new LinkedBlockingQueue<>(),
                new NamedThreadFactory(
                        "public-task#" + mThreadId++) {

                    @Override
                    public Thread newThread(Runnable r) {
                        Thread thread = super.newThread(r);
                        int priority = thread.getPriority();
                        if (priority != Thread.NORM_PRIORITY) {
                            thread.setPriority(Thread.NORM_PRIORITY);
                        }
                        return thread;
                    }
                });
    }

    /**
     * 调用者有效性检查
     * <p> 非主线程调用，将直接抛运行异常；
     */
    private void callerValidityCheck() {
        // 当前不在主线程
        if (!HThreadUtils.isMainThread()) {
            throw new RuntimeException("HPublicExecutor#" +
                    "submitTask/Prohibit calls from non main threads!");
        }
    }

    /**
     * 提交一个执行任务
     * <pre>
     *    Future 可以获取当前任务状态，Future<?>#get() 是阻塞的；
     *    接口回调事件 {@link HTaskRunnable.OnCompletionListener#onCompletion} 运行在主线程；
     * </pre>
     *
     * @param command  任务对象
     * @param listener 返回结果
     * @return a Future representing pending completion of the task
     * @throws RejectedExecutionException – if the task cannot be scheduled for
     *          execution NullPointerException – if the task is null
     */
    @MainThread
    public Future<?> submitTask(Callable<MusicInfo> command,
                                HTaskRunnable.OnCompletionListener listener) {
        callerValidityCheck();
        return mAsyncTaskExecutor.submit(
                new HTaskRunnable<>(command, mMainHandler, listener));
    }

    /**
     * 提交一个执行任务
     * <pre>
     *    Future 可以获取当前任务状态，Future<?>#get() 是阻塞的；
     *    接口回调事件 {@link HTaskRunnable.OnCompletionListener#onCompletion} 运行在主线程；
     * </pre>
     *
     * @param command  任务对象
     * @param listener 返回结果
     * @return a Future representing pending completion of the task
     * @throws RejectedExecutionException – if the task cannot be scheduled for
     *          execution NullPointerException – if the task is null
     */
    @MainThread
    public Future<?> submitTask2(Callable<Object> command,
                                HTaskRunnable.OnCompletionListener listener) {
        callerValidityCheck();
        return mAsyncTaskExecutor.submit(
                new HTaskRunnable<>(command, mMainHandler, listener));
    }

    /**
     * 提交一个异步回调状态的任务
     * <pre>
     *    Future 可以获取当前任务状态，Future<?>#get() 是阻塞的；
     *    接口回调事件 {@link HTaskRunnable.OnCompletionListener#onCompletion} 运行在任务执行线程；
     * </pre>
     *
     * @param command  任务对象
     * @param listener 返回结果
     * @return a Future representing pending completion of the task
     * @throws RejectedExecutionException – if the task cannot be scheduled for
     *          execution NullPointerException – if the task is null
     */
    public Future<?> submitAsyncResultTask(Callable<MusicInfo> command,
                                           HTaskRunnable.OnCompletionListener listener) {
        callerValidityCheck();
        return mAsyncTaskExecutor.submit(
                new HTaskRunnable<>(command, listener));
    }

    /**
     * 提交一个执行任务
     * <p> Future 可以获取当前任务状态，Future<?>#get() 是阻塞的；
     *
     * @param command  任务对象
     * @return a Future representing pending completion of the task
     * @throws RejectedExecutionException – if the task cannot be scheduled for
     *          execution NullPointerException – if the task is null
     */
    public Future<?> submitTask(RunnableEx command) {
        callerValidityCheck();
        return mAsyncTaskExecutor.submit(command);
    }

    /**
     * 提交一个执行任务
     * <pre>
     *    Future 可以获取当前任务状态，Future<?>#get() 是阻塞的；
     *    接口回调事件 {@link HTaskRunnable.OnCompletionListener#onCompletion} 运行在主线程；
     * </pre>
     *
     * @param command  任务对象
     * @param listener 返回结果
     * @return a Future representing pending completion of the task
     * @throws RejectedExecutionException – if the task cannot be scheduled for
     *          execution NullPointerException – if the task is null
     */
    public Future<?> submitTask(RunnableEx command,
                                HTaskRunnable.OnCompletionListener listener) {
        callerValidityCheck();
        return mAsyncTaskExecutor.submit(() -> {
            command.run();

            // 回调必需在主线程处理
            mMainHandler.post(() -> {
                if (listener != null) {
                    listener.onCompletion("ok");
                }
            });
        });
    }
}
