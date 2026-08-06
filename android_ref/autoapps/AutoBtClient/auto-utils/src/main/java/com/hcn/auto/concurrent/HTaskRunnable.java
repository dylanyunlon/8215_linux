package com.hcn.auto.concurrent;

import android.os.Handler;
import android.text.TextUtils;

import androidx.annotation.NonNull;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.concurrent.Callable;

/**
 * @description:
 * @author: guohonglan
 * @date: 2024/2/23 9:59
 */
public class HTaskRunnable<T> implements Runnable {
    private final Callable<T> mTask;
    private final long mTaskUniqueId;
    private final OnCompletionListener mListener;
    private final Reference<Handler> mHandlerRef;

    private HTaskRunnable() {
        throw new UnsupportedOperationException("u can't instantiate me...");
    }

    public HTaskRunnable(@NonNull Callable<T> task) {
        this(-1L, task, (OnCompletionListener)null);
    }

    public HTaskRunnable(long taskUniqueId, @NonNull Callable<T> task, OnCompletionListener listener) {
        this.mTaskUniqueId = taskUniqueId;
        this.mTask = task;
        this.mHandlerRef = null;
        this.mListener = listener;
    }

    public HTaskRunnable(long taskUniqueId, @NonNull Callable<T> task, @NonNull Handler handler, OnCompletionListener listener) {
        this.mTaskUniqueId = taskUniqueId;
        this.mTask = task;
        this.mHandlerRef = new WeakReference(handler);
        this.mListener = listener;
    }

    public void run() {
        String exception = null;
        boolean completed = false;
        boolean var10 = false;

        try {
            var10 = true;
            T result = this.mTask.call();
            completed = true;
            if (this.mListener != null) {
                if (this.mHandlerRef != null) {
                    Handler h0 = (Handler)this.mHandlerRef.get();
                    if (h0 != null) {
                        h0.post(() -> {
                            this.mListener.onCompletion(this.mTaskUniqueId, result);
                        });
                        var10 = false;
                    } else {
                        var10 = false;
                    }
                } else {
                    this.mListener.onCompletion(this.mTaskUniqueId, result);
                    var10 = false;
                }
            } else {
                var10 = false;
            }
        } catch (Exception var11) {
            if (!completed) {
                exception = var11.getMessage();
            }

            throw new RuntimeException(var11);
        } finally {
            if (var10) {
                if (this.mListener != null && !completed && !TextUtils.isEmpty(exception)) {
                    if (this.mHandlerRef != null) {
                        Handler h0 = (Handler)this.mHandlerRef.get();
                        if (h0 != null) {
                            String finalException = exception;
                            h0.post(() -> {
                                this.mListener.onCompletion(this.mTaskUniqueId, finalException);
                            });
                        }
                    } else {
                        this.mListener.onCompletion(this.mTaskUniqueId, exception);
                    }
                }

            }
        }

        if (this.mListener != null && !completed && !TextUtils.isEmpty(exception)) {
            if (this.mHandlerRef != null) {
                Handler h0 = (Handler)this.mHandlerRef.get();
                if (h0 != null) {
                    String finalException1 = exception;
                    h0.post(() -> {
                        this.mListener.onCompletion(this.mTaskUniqueId, finalException1);
                    });
                }
            } else {
                this.mListener.onCompletion(this.mTaskUniqueId, exception);
            }
        }

    }

    public interface OnCompletionListener {
        default void onCompletion(long taskTag, Object result) {
        }
    }
}
