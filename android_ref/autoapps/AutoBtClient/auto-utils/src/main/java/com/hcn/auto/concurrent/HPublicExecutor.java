package com.hcn.auto.concurrent;

import android.os.Handler;
import android.os.Looper;

import androidx.annotation.MainThread;

import com.hcn.auto.app.base.RunnableEx;
import com.hcn.auto.concurrent.factory.NamedThreadFactory;
import com.hcn.auto.utils.HThreadUtils;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * @description:
 * @author: guohonglan
 * @date: 2024/2/23 9:58
 */
public class HPublicExecutor {
    private static HPublicExecutor sInstance = null;
    private final ExecutorService mAsyncTaskExecutor;
    private int mThreadId = 0;
    private final Handler mMainHandler;

    public static HPublicExecutor instance() {
        if (sInstance == null) {
            sInstance = new HPublicExecutor();
        }

        return sInstance;
    }

    private HPublicExecutor() {
        this.callerValidityCheck();
        this.mMainHandler = new Handler(Looper.getMainLooper());
        this.mAsyncTaskExecutor = new ThreadPoolExecutor(0, 5, 30L, TimeUnit.SECONDS, new LinkedBlockingQueue(), new NamedThreadFactory("public-task#" + this.mThreadId++) {
            public Thread newThread(Runnable r) {
                Thread thread = super.newThread(r);
                int priority = thread.getPriority();
                if (priority != 5) {
                    thread.setPriority(5);
                }

                return thread;
            }
        });
    }

    private void callerValidityCheck() {
        if (!HThreadUtils.isMainThread()) {
            throw new RuntimeException("HPublicExecutor#submitTask/Prohibit calls from non main threads!");
        }
    }

    @MainThread
    public Future<?> submitTask(long taskUniqueId, Callable<Object> command, HTaskRunnable.OnCompletionListener listener) {
        this.callerValidityCheck();
        return this.mAsyncTaskExecutor.submit(new HTaskRunnable(taskUniqueId, command, this.mMainHandler, listener));
    }

    public Future<?> submitAsyncResultTask(long taskUniqueId, Callable<Object> command, HTaskRunnable.OnCompletionListener listener) {
        this.callerValidityCheck();
        return this.mAsyncTaskExecutor.submit(new HTaskRunnable(taskUniqueId, command, listener));
    }

    public Future<?> submitTask(RunnableEx command) {
        this.callerValidityCheck();
        return this.mAsyncTaskExecutor.submit(command);
    }

    public Future<?> submitTask(long taskUniqueId, RunnableEx command, HTaskRunnable.OnCompletionListener listener) {
        this.callerValidityCheck();
        return this.mAsyncTaskExecutor.submit(() -> {
            command.run();
            this.mMainHandler.post(() -> {
                if (listener != null) {
                    listener.onCompletion(taskUniqueId, "ok");
                }

            });
        });
    }
}
