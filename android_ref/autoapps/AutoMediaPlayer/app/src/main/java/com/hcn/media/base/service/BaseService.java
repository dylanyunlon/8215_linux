package com.hcn.media.base.service;

import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.lifecycle.LifecycleService;

import com.hcn.auto_compat.file.MediaUtilsEx;
import com.hcn.media_common.HBroadcastEx;
import com.hcn.common.HConfig;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.debug.MediaDebug;
import com.hcn.common.utils.HHandler;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_data.AppGlobalData;
import com.hcn.skinx.extend.SkinExCompatService;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.concurrent.ExecutorService;

import io.reactivex.rxjava3.disposables.CompositeDisposable;

/**
 * 服务基类
 * <p> 为弱化单个类的功能，可把公共部分抽取到此基类函数中；
 *
 * @author 65821
 */
public abstract class BaseService extends SkinExCompatService {
    private static final String TAG = BaseService.class.getSimpleName();

    /** 上下文与全局数据对象 **/
    protected Context mContext = null;
    protected AppGlobalData mAppData = null;

    /**
     * 消息处理器
     * <p> 在主线程运行，不要做耗时的操作；
     */
    protected TaskHandler H0;

    /**
     * Disposable 对象管理器
     * <pre>
     *    RxJava 容易造成内存泄漏，在某些情况下没有及时取消订阅导致内存泄漏；
     *    CompositeDisposable 可以将 Disposable 对象统一管理，避免内存泄露。
     * </pre>
     */
    protected final CompositeDisposable mCompositeDisposable = new CompositeDisposable();

    /**
     * 线程池对象
     * <pre>
     *    耗时工作都由它来处理，如需要可以创建；
     *    用来处理临时异步任务，避免频繁创建线程；
     * </pre>
     */
    protected ExecutorService mThreadPool = null;

    /** 提示信息显示对象 **/
    protected Toast mToast = null;

    @Override
    protected void attachBaseContext(Context newBase) {
        super.attachBaseContext(newBase);

        // 创建观察者
        onCreateObserver();
    }

    /** 创建服务扩展（观察者） **/
    protected abstract void onCreateObserver();

    @Override
    public void onCreate() {
        super.onCreate();

        // 初始化环境
        mContext = getApplicationContext();
        mAppData = AppGlobalData.getInstance();

        // 是前台服务
        if (MediaDebug.START_FOREGROUND_SERVICE) {
            onStartForeground();
        }

        // 消息处理器
        H0 = new TaskHandler(Looper.getMainLooper(), this);
    }

    /** 设置前台服务 **/
    protected void onStartForeground() {
        // TODO: 前台服务重载实现
    };

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String reason = "null";
        if (intent != null) {
            reason = intent.getStringExtra(HConfig.START_REASON_EXTRA_KEY);
        }

        Log.v(TAG, "onStartCommand: " + reason);
        return super.onStartCommand(intent, flags, startId);
    }

    /**
     * <p> 注意: 同一个 context 多次执行 bindService 接口，
     * <p> onBind 方法并不会被多次调用，即不会多次创建绑定。
     *
     * @param intent 绑定意图
     * @return 返回 Binder 代理
     */
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        String reason = "null";
        if (intent != null) {
            reason = intent.getStringExtra(HConfig.START_REASON_EXTRA_KEY);
        }

        Log.v(TAG, "onBind: " + reason);
        return super.onBind(intent);
    }

    /**
     * 倾倒信息到终端
     *
     *@param fd dump 要发送到的原始文件描述符。
     *@param fout 您应该将状态 dump 到的 PrintWriter。您返回后这里将为您关闭。
     *@param args dump 请求的其他参数。
     */
    @Override
    protected void dump(@NonNull FileDescriptor fd,
                        @NonNull PrintWriter fout,
                        @Nullable String[] args) {
        super.dump(fd, fout, args);
        // TODO: 子类根据情况实现
    }

    /**
     * 请求执行目标类型方法
     * <pre>
     *    由外部请求触发执行，具体由有需求的子类去实现；
     *    返回 {@link MediaUtilsEx#UNSUPPORTED} 表示请求执行的方法类型不支持
     * </pre>
     *
     * @param method 方法类型
     * @param objects 参数集
     * @return 执行结果 {根据实际情况约定}
     */
    @Override
    protected Object requestExecuteMethod_Impl(String method, Object... objects) {
        return MediaUtilsEx.UNSUPPORTED;
    }

    /**
     * 对外发送本地广播事件
     * @see #sendLocalBroadcast(int, String)
     *
     * @param eventId {@link IMediaEvent}
     */
    protected void sendLocalBroadcast(int eventId) {
        if (eventId != IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME) {
            LogUtil.low_i(TAG, ">>>>> sendLocalBroadcast event: " + eventId);
        }

        HBroadcastEx.sendLocalBroadcast(this, eventId);
    }

    /**
     * 对外发送本地广播事件
     * <pre>
     *    其实现的本质原理还是一个监听回调；
     *    对本进程内通讯可以使用，用来做代码隔离；
     *    如果指定当前服务作为独立的进程运行，它是不适合的；
     * </pre>
     *
     * @param eventId {@link IMediaEvent}
     * @param data 额外的附加数据
     */
    protected void sendLocalBroadcast(int eventId, String data) {
        if (eventId != IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME) {
            LogUtil.low_i(TAG, ">>>>> localSendBroadcast, event: " + eventId);
        }

        HBroadcastEx.sendLocalBroadcast(this, eventId, data);
    }

    /**
     * 实现该接口可以处理自定义消息
     *
     * @param msg 消息对象
     * @return 消息是否已经被处理
     */
    protected abstract boolean onHandleMessage(@NonNull Message msg);

    /**
     * 主消息处理器
     * <p> Looper.getMainLooper()
     */
    protected static final class TaskHandler extends HHandler {
        /**
         * 当前服务引用
         */
        private final Reference<BaseService> mServiceRef;

        public TaskHandler(@NonNull Looper looper, BaseService service) {
            super(looper);
            mServiceRef = new WeakReference<>(service);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);

            BaseService s = mServiceRef.get();
            if (s != null) {
                s.onHandleMessage(msg);
            }
        }
    }

    /** 停止前台服务 **/
    protected void onStopForeground() {
        // TODO: 前台服务重载实现
    }

    /** 销毁服务扩展（观察者） **/
    protected abstract void onDestroyObserver();

    @Override
    public void onDestroy() {
        super.onDestroy();

        // 销毁观察者
        onDestroyObserver();

        // 是前台服务
        if (MediaDebug.START_FOREGROUND_SERVICE) {
            onStopForeground();
        }

        // 释放容器资源
        mCompositeDisposable.dispose();

        // 清除当前消息队列
        H0.removeCallbacksAndMessages(null);
    }
}
