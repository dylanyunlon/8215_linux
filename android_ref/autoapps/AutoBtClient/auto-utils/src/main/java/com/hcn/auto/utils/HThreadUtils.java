package com.hcn.auto.utils;

import android.os.Handler;
import android.os.Looper;

import android.util.Log;

import androidx.annotation.CallSuper;
import androidx.annotation.IntRange;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

public class HThreadUtils {
    private static final Handler HANDLER = new Handler(Looper.getMainLooper());
    private static final Map<Integer, Map<Integer, ExecutorService>> TYPE_PRIORITY_POOLS = new HashMap();
    private static final Map<Task, ExecutorService> TASK_POOL_MAP = new ConcurrentHashMap();
    private static final int CPU_COUNT = Runtime.getRuntime().availableProcessors();
    private static final Timer TIMER = new Timer();
    private static final byte TYPE_SINGLE = -1;
    private static final byte TYPE_CACHED = -2;
    private static final byte TYPE_IO = -4;
    private static final byte TYPE_CPU = -8;
    private static Executor sDeliver;

    public HThreadUtils() {
    }

    public static boolean isMainThread() {
        return Looper.myLooper() == Looper.getMainLooper();
    }

    public static Handler getMainHandler() {
        return HANDLER;
    }

    public static void runOnUiThread(Runnable runnable) {
        if (Looper.myLooper() == Looper.getMainLooper()) {
            runnable.run();
        } else {
            HANDLER.post(runnable);
        }

    }

    public static void runOnUiThreadDelayed(Runnable runnable, long delayMillis) {
        HANDLER.postDelayed(runnable, delayMillis);
    }

    public static ExecutorService getFixedPool(@IntRange(from = 1L) int size) {
        return getPoolByTypeAndPriority(size);
    }

    public static ExecutorService getFixedPool(@IntRange(from = 1L) int size, @IntRange(from = 1L,to = 10L) int priority) {
        return getPoolByTypeAndPriority(size, priority);
    }

    public static ExecutorService getSinglePool() {
        return getPoolByTypeAndPriority(-1);
    }

    public static ExecutorService getSinglePool(@IntRange(from = 1L,to = 10L) int priority) {
        return getPoolByTypeAndPriority(-1, priority);
    }

    public static ExecutorService getCachedPool() {
        return getPoolByTypeAndPriority(-2);
    }

    public static ExecutorService getCachedPool(@IntRange(from = 1L,to = 10L) int priority) {
        return getPoolByTypeAndPriority(-2, priority);
    }

    public static ExecutorService getIoPool() {
        return getPoolByTypeAndPriority(-4);
    }

    public static ExecutorService getIoPool(@IntRange(from = 1L,to = 10L) int priority) {
        return getPoolByTypeAndPriority(-4, priority);
    }

    public static ExecutorService getCpuPool() {
        return getPoolByTypeAndPriority(-8);
    }

    public static ExecutorService getCpuPool(@IntRange(from = 1L,to = 10L) int priority) {
        return getPoolByTypeAndPriority(-8, priority);
    }

    public static <T> void executeByFixed(@IntRange(from = 1L) int size, Task<T> task) {
        execute(getPoolByTypeAndPriority(size), task);
    }

    public static <T> void executeByFixed(@IntRange(from = 1L) int size, Task<T> task, @IntRange(from = 1L,to = 10L) int priority) {
        execute(getPoolByTypeAndPriority(size, priority), task);
    }

    public static <T> void executeByFixedWithDelay(@IntRange(from = 1L) int size, Task<T> task, long delay, TimeUnit unit) {
        executeWithDelay(getPoolByTypeAndPriority(size), task, delay, unit);
    }

    public static <T> void executeByFixedWithDelay(@IntRange(from = 1L) int size, Task<T> task, long delay, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeWithDelay(getPoolByTypeAndPriority(size, priority), task, delay, unit);
    }

    public static <T> void executeByFixedAtFixRate(@IntRange(from = 1L) int size, Task<T> task, long period, TimeUnit unit) {
        executeAtFixedRate(getPoolByTypeAndPriority(size), task, 0L, period, unit);
    }

    public static <T> void executeByFixedAtFixRate(@IntRange(from = 1L) int size, Task<T> task, long period, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeAtFixedRate(getPoolByTypeAndPriority(size, priority), task, 0L, period, unit);
    }

    public static <T> void executeByFixedAtFixRate(@IntRange(from = 1L) int size, Task<T> task, long initialDelay, long period, TimeUnit unit) {
        executeAtFixedRate(getPoolByTypeAndPriority(size), task, initialDelay, period, unit);
    }

    public static <T> void executeByFixedAtFixRate(@IntRange(from = 1L) int size, Task<T> task, long initialDelay, long period, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeAtFixedRate(getPoolByTypeAndPriority(size, priority), task, initialDelay, period, unit);
    }

    public static <T> void executeBySingle(Task<T> task) {
        execute(getPoolByTypeAndPriority(-1), task);
    }

    public static <T> void executeBySingle(Task<T> task, @IntRange(from = 1L,to = 10L) int priority) {
        execute(getPoolByTypeAndPriority(-1, priority), task);
    }

    public static <T> void executeBySingleWithDelay(Task<T> task, long delay, TimeUnit unit) {
        executeWithDelay(getPoolByTypeAndPriority(-1), task, delay, unit);
    }

    public static <T> void executeBySingleWithDelay(Task<T> task, long delay, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeWithDelay(getPoolByTypeAndPriority(-1, priority), task, delay, unit);
    }

    public static <T> void executeBySingleAtFixRate(Task<T> task, long period, TimeUnit unit) {
        executeAtFixedRate(getPoolByTypeAndPriority(-1), task, 0L, period, unit);
    }

    public static <T> void executeBySingleAtFixRate(Task<T> task, long period, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeAtFixedRate(getPoolByTypeAndPriority(-1, priority), task, 0L, period, unit);
    }

    public static <T> void executeBySingleAtFixRate(Task<T> task, long initialDelay, long period, TimeUnit unit) {
        executeAtFixedRate(getPoolByTypeAndPriority(-1), task, initialDelay, period, unit);
    }

    public static <T> void executeBySingleAtFixRate(Task<T> task, long initialDelay, long period, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeAtFixedRate(getPoolByTypeAndPriority(-1, priority), task, initialDelay, period, unit);
    }

    public static <T> void executeByCached(Task<T> task) {
        execute(getPoolByTypeAndPriority(-2), task);
    }

    public static <T> void executeByCached(Task<T> task, @IntRange(from = 1L,to = 10L) int priority) {
        execute(getPoolByTypeAndPriority(-2, priority), task);
    }

    public static <T> void executeByCachedWithDelay(Task<T> task, long delay, TimeUnit unit) {
        executeWithDelay(getPoolByTypeAndPriority(-2), task, delay, unit);
    }

    public static <T> void executeByCachedWithDelay(Task<T> task, long delay, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeWithDelay(getPoolByTypeAndPriority(-2, priority), task, delay, unit);
    }

    public static <T> void executeByCachedAtFixRate(Task<T> task, long period, TimeUnit unit) {
        executeAtFixedRate(getPoolByTypeAndPriority(-2), task, 0L, period, unit);
    }

    public static <T> void executeByCachedAtFixRate(Task<T> task, long period, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeAtFixedRate(getPoolByTypeAndPriority(-2, priority), task, 0L, period, unit);
    }

    public static <T> void executeByCachedAtFixRate(Task<T> task, long initialDelay, long period, TimeUnit unit) {
        executeAtFixedRate(getPoolByTypeAndPriority(-2), task, initialDelay, period, unit);
    }

    public static <T> void executeByCachedAtFixRate(Task<T> task, long initialDelay, long period, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeAtFixedRate(getPoolByTypeAndPriority(-2, priority), task, initialDelay, period, unit);
    }

    public static <T> void executeByIo(Task<T> task) {
        execute(getPoolByTypeAndPriority(-4), task);
    }

    public static <T> void executeByIo(Task<T> task, @IntRange(from = 1L,to = 10L) int priority) {
        execute(getPoolByTypeAndPriority(-4, priority), task);
    }

    public static <T> void executeByIoWithDelay(Task<T> task, long delay, TimeUnit unit) {
        executeWithDelay(getPoolByTypeAndPriority(-4), task, delay, unit);
    }

    public static <T> void executeByIoWithDelay(Task<T> task, long delay, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeWithDelay(getPoolByTypeAndPriority(-4, priority), task, delay, unit);
    }

    public static <T> void executeByIoAtFixRate(Task<T> task, long period, TimeUnit unit) {
        executeAtFixedRate(getPoolByTypeAndPriority(-4), task, 0L, period, unit);
    }

    public static <T> void executeByIoAtFixRate(Task<T> task, long period, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeAtFixedRate(getPoolByTypeAndPriority(-4, priority), task, 0L, period, unit);
    }

    public static <T> void executeByIoAtFixRate(Task<T> task, long initialDelay, long period, TimeUnit unit) {
        executeAtFixedRate(getPoolByTypeAndPriority(-4), task, initialDelay, period, unit);
    }

    public static <T> void executeByIoAtFixRate(Task<T> task, long initialDelay, long period, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeAtFixedRate(getPoolByTypeAndPriority(-4, priority), task, initialDelay, period, unit);
    }

    public static <T> void executeByCpu(Task<T> task) {
        execute(getPoolByTypeAndPriority(-8), task);
    }

    public static <T> void executeByCpu(Task<T> task, @IntRange(from = 1L,to = 10L) int priority) {
        execute(getPoolByTypeAndPriority(-8, priority), task);
    }

    public static <T> void executeByCpuWithDelay(Task<T> task, long delay, TimeUnit unit) {
        executeWithDelay(getPoolByTypeAndPriority(-8), task, delay, unit);
    }

    public static <T> void executeByCpuWithDelay(Task<T> task, long delay, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeWithDelay(getPoolByTypeAndPriority(-8, priority), task, delay, unit);
    }

    public static <T> void executeByCpuAtFixRate(Task<T> task, long period, TimeUnit unit) {
        executeAtFixedRate(getPoolByTypeAndPriority(-8), task, 0L, period, unit);
    }

    public static <T> void executeByCpuAtFixRate(Task<T> task, long period, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeAtFixedRate(getPoolByTypeAndPriority(-8, priority), task, 0L, period, unit);
    }

    public static <T> void executeByCpuAtFixRate(Task<T> task, long initialDelay, long period, TimeUnit unit) {
        executeAtFixedRate(getPoolByTypeAndPriority(-8), task, initialDelay, period, unit);
    }

    public static <T> void executeByCpuAtFixRate(Task<T> task, long initialDelay, long period, TimeUnit unit, @IntRange(from = 1L,to = 10L) int priority) {
        executeAtFixedRate(getPoolByTypeAndPriority(-8, priority), task, initialDelay, period, unit);
    }

    public static <T> void executeByCustom(ExecutorService pool, Task<T> task) {
        execute(pool, task);
    }

    public static <T> void executeByCustomWithDelay(ExecutorService pool, Task<T> task, long delay, TimeUnit unit) {
        executeWithDelay(pool, task, delay, unit);
    }

    public static <T> void executeByCustomAtFixRate(ExecutorService pool, Task<T> task, long period, TimeUnit unit) {
        executeAtFixedRate(pool, task, 0L, period, unit);
    }

    public static <T> void executeByCustomAtFixRate(ExecutorService pool, Task<T> task, long initialDelay, long period, TimeUnit unit) {
        executeAtFixedRate(pool, task, initialDelay, period, unit);
    }

    public static void cancel(Task task) {
        if (task != null) {
            task.cancel();
        }
    }

    public static void cancel(Task... tasks) {
        if (tasks != null) {
            Task[] var1 = tasks;
            int var2 = tasks.length;

            for(int var3 = 0; var3 < var2; ++var3) {
                Task task = var1[var3];
                if (task != null) {
                    task.cancel();
                }
            }

        }
    }

    public static void cancel(List<Task> tasks) {
        if (tasks != null && tasks.size() != 0) {
            Iterator var1 = tasks.iterator();

            while(var1.hasNext()) {
                Task task = (Task)var1.next();
                if (task != null) {
                    task.cancel();
                }
            }

        }
    }

    public static void cancel(ExecutorService executorService) {
        if (executorService instanceof ThreadPoolExecutorEx) {
            Iterator var1 = TASK_POOL_MAP.entrySet().iterator();

            while(var1.hasNext()) {
                Map.Entry<Task, ExecutorService> taskTaskInfoEntry = (Map.Entry)var1.next();
                if (taskTaskInfoEntry.getValue() == executorService) {
                    cancel((Task)taskTaskInfoEntry.getKey());
                }
            }
        } else {
            Log.e("ThreadUtils", "The executorService is not ThreadUtils's pool.");
        }

    }

    public static void setDeliver(Executor deliver) {
        sDeliver = deliver;
    }

    private static <T> void execute(ExecutorService pool, Task<T> task) {
        execute(pool, task, 0L, 0L, (TimeUnit)null);
    }

    private static <T> void executeWithDelay(ExecutorService pool, Task<T> task, long delay, TimeUnit unit) {
        execute(pool, task, delay, 0L, unit);
    }

    private static <T> void executeAtFixedRate(ExecutorService pool, Task<T> task, long delay, long period, TimeUnit unit) {
        execute(pool, task, delay, period, unit);
    }

    private static <T> void execute(final ExecutorService pool, final Task<T> task, long delay, long period, TimeUnit unit) {
        synchronized(TASK_POOL_MAP) {
            if (TASK_POOL_MAP.get(task) != null) {
                Log.e("ThreadUtils", "Task can only be executed once.");
                return;
            }

            TASK_POOL_MAP.put(task, pool);
        }

        TimerTask timerTask;
        if (period == 0L) {
            if (delay == 0L) {
                pool.execute(task);
            } else {
                timerTask = new TimerTask() {
                    public void run() {
                        pool.execute(task);
                    }
                };
                TIMER.schedule(timerTask, unit.toMillis(delay));
            }
        } else {
            task.setSchedule(true);
            timerTask = new TimerTask() {
                public void run() {
                    pool.execute(task);
                }
            };
            TIMER.scheduleAtFixedRate(timerTask, unit.toMillis(delay), unit.toMillis(period));
        }

    }

    private static ExecutorService getPoolByTypeAndPriority(int type) {
        return getPoolByTypeAndPriority(type, 5);
    }

    private static ExecutorService getPoolByTypeAndPriority(int type, int priority) {
        synchronized(TYPE_PRIORITY_POOLS) {
            Map<Integer, ExecutorService> priorityPools = (Map)TYPE_PRIORITY_POOLS.get(type);
            ExecutorService pool;
            if (priorityPools == null) {
                priorityPools = new ConcurrentHashMap();
                pool = ThreadPoolExecutorEx.createPool(type, priority);
                priorityPools.put(priority, pool);
                TYPE_PRIORITY_POOLS.put(type, priorityPools);
            } else {
                pool = (ExecutorService)priorityPools.get(priority);
                if (pool == null) {
                    pool = ThreadPoolExecutorEx.createPool(type, priority);
                    priorityPools.put(priority, pool);
                }
            }

            return pool;
        }
    }

    private static Executor getGlobalDeliver() {
        if (sDeliver == null) {
            sDeliver = new Executor() {
                public void execute(@NonNull Runnable command) {
                    runOnUiThread(command);
                }
            };
        }

        return sDeliver;
    }

    public abstract static class Task<T> implements Runnable {
        private static final int NEW = 0;
        private static final int RUNNING = 1;
        private static final int EXCEPTIONAL = 2;
        private static final int COMPLETING = 3;
        private static final int CANCELLED = 4;
        private static final int INTERRUPTED = 5;
        private static final int TIMEOUT = 6;
        private final AtomicInteger state = new AtomicInteger(0);
        private volatile boolean isSchedule;
        private volatile Thread runner;
        private Timer mTimer;
        private long mTimeoutMillis;
        private OnTimeoutListener mTimeoutListener;
        private Executor deliver;

        public Task() {
        }

        public int hashCode() {
            return super.hashCode();
        }

        public boolean equals(@Nullable Object obj) {
            return super.equals(obj);
        }

        public abstract T doInBackground() throws Throwable;

        public abstract void onSuccess(T var1);

        public abstract void onCancel();

        public abstract void onFail(Throwable var1);

        public void run() {
            if (this.isSchedule) {
                if (this.runner == null) {
                    if (!this.state.compareAndSet(0, 1)) {
                        return;
                    }

                    this.runner = Thread.currentThread();
                    if (this.mTimeoutListener != null) {
                        Log.w("ThreadUtils", "Scheduled task doesn't support timeout.");
                    }
                } else if (this.state.get() != 1) {
                    return;
                }
            } else {
                if (!this.state.compareAndSet(0, 1)) {
                    return;
                }

                this.runner = Thread.currentThread();
                if (this.mTimeoutListener != null) {
                    this.mTimer = new Timer();
                    this.mTimer.schedule(new TimerTask() {
                        public void run() {
                            if (!Task.this.isDone() && Task.this.mTimeoutListener != null) {
                                Task.this.timeout();
                                Task.this.mTimeoutListener.onTimeout();
                                Task.this.onDone();
                            }

                        }
                    }, this.mTimeoutMillis);
                }
            }

            try {
                final T result = this.doInBackground();
                if (this.isSchedule) {
                    if (this.state.get() != 1) {
                        return;
                    }

                    this.getDeliver().execute(new Runnable() {
                        public void run() {
                            Task.this.onSuccess(result);
                        }
                    });
                } else {
                    if (!this.state.compareAndSet(1, 3)) {
                        return;
                    }

                    this.getDeliver().execute(new Runnable() {
                        public void run() {
                            Task.this.onSuccess(result);
                            Task.this.onDone();
                        }
                    });
                }
            } catch (InterruptedException var2) {
                this.state.compareAndSet(4, 5);
            } catch (final Throwable var3) {
                if (!this.state.compareAndSet(1, 2)) {
                    return;
                }

                this.getDeliver().execute(new Runnable() {
                    public void run() {
                        Task.this.onFail(var3);
                        Task.this.onDone();
                    }
                });
            }

        }

        public void cancel() {
            this.cancel(true);
        }

        public void cancel(boolean mayInterruptIfRunning) {
            synchronized(this.state) {
                if (this.state.get() > 1) {
                    return;
                }

                this.state.set(4);
            }

            if (mayInterruptIfRunning && this.runner != null) {
                this.runner.interrupt();
            }

            this.getDeliver().execute(new Runnable() {
                public void run() {
                    Task.this.onCancel();
                    Task.this.onDone();
                }
            });
        }

        private void timeout() {
            synchronized(this.state) {
                if (this.state.get() > 1) {
                    return;
                }

                this.state.set(6);
            }

            if (this.runner != null) {
                this.runner.interrupt();
            }

        }

        public boolean isCanceled() {
            return this.state.get() >= 4;
        }

        public boolean isDone() {
            return this.state.get() > 1;
        }

        public Task<T> setDeliver(Executor deliver) {
            this.deliver = deliver;
            return this;
        }

        public Task<T> setTimeout(long timeoutMillis, OnTimeoutListener listener) {
            this.mTimeoutMillis = timeoutMillis;
            this.mTimeoutListener = listener;
            return this;
        }

        private void setSchedule(boolean isSchedule) {
            this.isSchedule = isSchedule;
        }

        private Executor getDeliver() {
            return this.deliver == null ? getGlobalDeliver() : this.deliver;
        }

        @CallSuper
        protected void onDone() {
            TASK_POOL_MAP.remove(this);
            if (this.mTimer != null) {
                this.mTimer.cancel();
                this.mTimer = null;
                this.mTimeoutListener = null;
            }

        }

        public interface OnTimeoutListener {
            void onTimeout();
        }
    }

    static final class ThreadPoolExecutorEx extends ThreadPoolExecutor {
        private final AtomicInteger mSubmittedCount = new AtomicInteger();
        private LinkedBlockingQueueEx mWorkQueue;

        private static ExecutorService createPool(int type, int priority) {
            switch (type) {
                case -8:
                    return new ThreadPoolExecutorEx(CPU_COUNT + 1, 2 * CPU_COUNT + 1, 30L, TimeUnit.SECONDS, new LinkedBlockingQueueEx(true), new UtilsThreadFactory("cpu", priority));
                case -7:
                case -6:
                case -5:
                case -3:
                default:
                    return new ThreadPoolExecutorEx(type, type, 0L, TimeUnit.MILLISECONDS, new LinkedBlockingQueueEx(), new UtilsThreadFactory("fixed(" + type + ")", priority));
                case -4:
                    return new ThreadPoolExecutorEx(2 * CPU_COUNT + 1, 2 * CPU_COUNT + 1, 30L, TimeUnit.SECONDS, new LinkedBlockingQueueEx(), new UtilsThreadFactory("io", priority));
                case -2:
                    return new ThreadPoolExecutorEx(0, 128, 60L, TimeUnit.SECONDS, new LinkedBlockingQueueEx(true), new UtilsThreadFactory("cached", priority));
                case -1:
                    return new ThreadPoolExecutorEx(1, 1, 0L, TimeUnit.MILLISECONDS, new LinkedBlockingQueueEx(), new UtilsThreadFactory("single", priority));
            }
        }

        ThreadPoolExecutorEx(int corePoolSize, int maximumPoolSize, long keepAliveTime, TimeUnit unit, LinkedBlockingQueueEx workQueue, ThreadFactory threadFactory) {
            super(corePoolSize, maximumPoolSize, keepAliveTime, unit, workQueue, threadFactory);
            workQueue.mPool = this;
            this.mWorkQueue = workQueue;
        }

        private int getSubmittedCount() {
            return this.mSubmittedCount.get();
        }

        protected void afterExecute(Runnable r, Throwable t) {
            this.mSubmittedCount.decrementAndGet();
            super.afterExecute(r, t);
        }

        public void execute(@NonNull Runnable command) {
            if (!this.isShutdown()) {
                this.mSubmittedCount.incrementAndGet();

                try {
                    super.execute(command);
                } catch (RejectedExecutionException var3) {
                    Log.e("ThreadUtils", "This will not happen!");
                    this.mWorkQueue.offer(command);
                } catch (Throwable var4) {
                    this.mSubmittedCount.decrementAndGet();
                }

            }
        }
    }

    public static class SyncValue<T> {
        private CountDownLatch mLatch = new CountDownLatch(1);
        private AtomicBoolean mFlag = new AtomicBoolean();
        private T mValue;

        public SyncValue() {
        }

        public void setValue(T value) {
            if (this.mFlag.compareAndSet(false, true)) {
                this.mValue = value;
                this.mLatch.countDown();
            }

        }

        public T getValue() {
            if (!this.mFlag.get()) {
                try {
                    this.mLatch.await();
                } catch (InterruptedException var2) {
                    var2.printStackTrace();
                }
            }

            return this.mValue;
        }

        public T getValue(long timeout, TimeUnit unit, T defaultValue) {
            if (!this.mFlag.get()) {
                try {
                    this.mLatch.await(timeout, unit);
                } catch (InterruptedException var6) {
                    var6.printStackTrace();
                    return defaultValue;
                }
            }

            return this.mValue;
        }
    }

    public abstract static class SimpleTask<T> extends Task<T> {
        public SimpleTask() {
        }

        public void onCancel() {
            Log.e("ThreadUtils", "onCancel: " + Thread.currentThread());
        }

        public void onFail(Throwable t) {
            Log.e("ThreadUtils", "onFail: ", t);
        }
    }

    static final class UtilsThreadFactory extends AtomicLong implements ThreadFactory {
        private static final AtomicInteger POOL_NUMBER = new AtomicInteger(1);
        private static final long serialVersionUID = -9209200509960368598L;
        private final String namePrefix;
        private final int priority;
        private final boolean isDaemon;

        UtilsThreadFactory(String prefix, int priority) {
            this(prefix, priority, false);
        }

        UtilsThreadFactory(String prefix, int priority, boolean isDaemon) {
            this.namePrefix = prefix + "-pool-" + POOL_NUMBER.getAndIncrement() + "-thread-";
            this.priority = priority;
            this.isDaemon = isDaemon;
        }

        public Thread newThread(@NonNull Runnable r) {
            Thread t = new Thread(r, this.namePrefix + this.getAndIncrement()) {
                public void run() {
                    try {
                        super.run();
                    } catch (Throwable var2) {
                        Log.e("ThreadUtils", "Request threw uncaught throwable", var2);
                    }

                }
            };
            t.setDaemon(this.isDaemon);
            t.setUncaughtExceptionHandler(new Thread.UncaughtExceptionHandler() {
                public void uncaughtException(Thread t, Throwable e) {
                    System.out.println(e);
                }
            });
            t.setPriority(this.priority);
            return t;
        }
    }

    private static final class LinkedBlockingQueueEx extends LinkedBlockingQueue<Runnable> {
        private volatile ThreadPoolExecutorEx mPool;
        private int mCapacity = Integer.MAX_VALUE;

        LinkedBlockingQueueEx() {
        }

        LinkedBlockingQueueEx(boolean isAddSubThreadFirstThenAddQueue) {
            if (isAddSubThreadFirstThenAddQueue) {
                this.mCapacity = 0;
            }

        }

        LinkedBlockingQueueEx(int capacity) {
            this.mCapacity = capacity;
        }

        public boolean offer(@NonNull Runnable runnable) {
            return this.mCapacity <= this.size() && this.mPool != null && this.mPool.getPoolSize() < this.mPool.getMaximumPoolSize() ? false : super.offer(runnable);
        }
    }
}
