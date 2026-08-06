package com.hcn.media_common.thread;

import android.os.Handler;
import android.text.TextUtils;

import androidx.annotation.NonNull;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.concurrent.Callable;

/**
 * 带执行完成状态的任务 Runnable 工具类
 * <p> 主要用来配合线程池使用，实现任务执行完成后的状态返回；
 *
 * @author 65821
 */
public class HTaskRunnable<T> implements Runnable {

    /**
     * 需要执行的任务
     */
    private final Callable<T> mTask;

    /**
     * 任务完成监听器
     */
    private final OnCompletionListener mListener;

    /**
     * 回调处理器引用
     */
    private final Reference<Handler> mHandlerRef;

    /**
     * 不提供默认无参构造函数
     */
    private HTaskRunnable() {
        throw new UnsupportedOperationException("u can't instantiate me...");
    }

    /**
     * 任务对象构造函数
     *
     * @param task     任务对象
     * @param listener 任务完成返回监听对象
     */
    public HTaskRunnable(@NonNull Callable<T> task,
                         OnCompletionListener listener) {
        mTask = task;
        mHandlerRef = null;
        mListener = listener;
    }

    /**
     * 任务对象构造函数
     *
     * @param task     任务对象
     * @param handler  回调处理器
     * @param listener 任务完成返回监听对象
     */
    public HTaskRunnable(@NonNull Callable<T> task,
                         @NonNull Handler handler,
                         OnCompletionListener listener) {
        mTask = task;
        mHandlerRef = new WeakReference<>(handler);
        mListener = listener;
    }

    /**
     * 任务对象构造函数
     *
     * @param task 任务对象
     */
    public HTaskRunnable(@NonNull Callable<T> task) {
        mTask = task;
        mHandlerRef = null;
        mListener = null;
    }

    /**
     * 任务结果
     */
    public interface OnCompletionListener {
        /**
         * 任务执行完成后调用
         * <p> 后续可以扩展任务返回结果；
         *
         * @param result 任务结果
         */
        void onCompletion(Object result);

        /**
         * 任务执行完成后调用
         * <p> 后续可以扩展任务返回结果；
         *
         * @param taskTag 任务标记号
         * @param result 任务结果
         */
        default void onCompletion(long taskTag, Object result) {};
    }

    @Override
    public void run() {
        String exception = null;
        boolean completed = false;

        try {
            T result = mTask.call();
            completed = true;

            // 执行完成结果回调
            if (mListener != null) {
                if (mHandlerRef != null) {
                    Handler h0 = mHandlerRef.get();
                    if (h0 != null) {
                        h0.post(() -> mListener.onCompletion(result));
                    }
                } else {
                    mListener.onCompletion(result);
                }
            }
        } catch (Exception e) {
            // 任务（mTask.call()）执行异常
            if (!completed) {
                exception = e.getMessage();
            }

            throw new RuntimeException(e);
        } finally {
            // 如果执行异常（返回异常信息）
            if (mListener != null
                    && !completed
                    && !TextUtils.isEmpty(exception)) {
                if (mHandlerRef != null) {
                    Handler h0 = mHandlerRef.get();
                    if (h0 != null) {
                        String finalException = exception;
                        h0.post(() -> mListener.onCompletion(finalException));
                    }
                } else {
                    mListener.onCompletion(exception);
                }
            }
        }
    }
}
