package com.hcn.media_model.impl.data;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.text.TextUtils;

import androidx.annotation.NonNull;

import com.hcn.common.lang.RunnableEx;
import com.hcn.common.misc.HBroadcastUtils;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_common.HBroadcastEx;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_model.base.BaseModel;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.Objects;

/**
 * 数据模式基类
 *
 * @author 65821
 */
public abstract class DataBaseModel extends BaseModel {
    protected static final String TAG = DataBaseModel.class.getSimpleName();

    /**
     * 本地广播事件接收者
     * <p> 监听由 LocalService 下发的媒体相关状态事件；
     */
    private BroadcastReceiver mMediaEventReceiver;

    /**
     * BaseModel 构造函数
     *
     * @param context 上下文环境
     * @param name    线程池的名字（如果为空，不创建线程池）
     */
    public DataBaseModel(Context context, String name) {
        super(context, name);

        // 媒体状态事件接收处理器
        connectMediaEventReceiver();
    }

    @Override
    protected void close() {
        super.close();

        // 媒体状态事件接收处理器
        disconnectMediaEventReceiver();
    }

    /**
     * 接入本地媒体事件广播接收者
     * <pre>
     *    这是一个模拟广播机制的 callback 事件分发组件；
     *    我们可以用它实现监听 Service 组件下发的事件状态；
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
     * 处理存储设备事件
     * <pre>
     *    1、存储设备挂载事件；
     *    2、存储设备卸载事件；
     *    3、存储设备媒体数据开始扫描事件；
     *    3、存储设备媒体数据扫描完成事件；
     * </pre>
     *
     * @param event  事件定义 {@link IMediaEvent}
     * @param wParam 附加参数 1
     * @param lParam 附加参数 2
     */
    protected abstract void onStorageDeviceEvent(int event, Object wParam, Object lParam);

    /**
     * 本地广播事件处理类
     * <p> 由本地 Service/Model 下发的媒体播放相关事件状态；
     */
    private static final class MediaEventReceiver extends BroadcastReceiver {
        private final Reference<DataBaseModel> mOwnerRef;

        public MediaEventReceiver(DataBaseModel model) {
            super();
            mOwnerRef = new WeakReference<>(model);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (TextUtils.isEmpty(action)) {
                return;
            }

            // 处理本地广播事件（LocalService）
            if (action.equals(HBroadcastEx.SpecialChain.ACTION_LOCAL_CALLBACK)) {
                DataBaseModel model = mOwnerRef.get();
                if (Objects.isNull(model)) {
                    return;
                }

                // 读取本地事件广播参数
                int event = intent.getIntExtra(
                        HBroadcastEx.SpecialChain.EXTRA_CALLBACK_TYPE, IMediaEvent.EVENT_NONE);
                String data = intent.getStringExtra(HBroadcastEx.SpecialChain.EXTRA_CALLBACK_DATA);

                // DataModel 只需要处理存储设备相关事件
                switch (event) {
                    case IMediaEvent.EVENT_MEDIA_UNMOUNTED:
                    case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
                        model.onStorageDeviceEvent(event, data, null);
                        break;
                    case IMediaEvent.EVENT_MEDIA_MOUNTED:
                    case IMediaEvent.EVENT_MEDIA_LOADING_START:
                    default:
                        break;
                }
            }
        }
    }

    /**
     * 数据操作任务基类封装
     * <pre>
     *    用来简化数据操作任务类实现；
     *    如要实现异步任务都可以去实现它；
     * </pre>
     */
    protected static abstract class BaseTask<Result> implements Runnable {
        protected final Reference<DataBaseModel> mOwnerRef;
        protected final RunnableEx onPostExecute;

        public BaseTask(DataBaseModel model,
                        @NonNull Runnable mainThreadResult) {
            super();

            mOwnerRef = new WeakReference<>(model);
            onPostExecute = (RunnableEx) mainThreadResult;
        }

        /**
         * 后台任务实现函数
         * @return 返回执行结果；
         */
        protected abstract Result doInBackground();

        /**
         * 后台任务扩展函数
         * <p> 任务扩展支持接口，支持任务收尾工作；
         */
        protected void doInBackgroundLast() {
            // 按需求扩展重载
        }

        @Override
        public void run() {
            Result result = null;
            try {
                result = doInBackground();
            } catch (Throwable tr) {
                throw tr;
            } finally {
                postTaskResult(result);
            }

            // 后台任务扩展
            doInBackgroundLast();
        }

        /**
         * 转发数据结果到主线程
         * <p> 处理 doInBackground 的执行结果；
         */
        public void postTaskResult(Result result) {
            // 当前模式有效性检查
            DataBaseModel model = mOwnerRef.get();
            if (Objects.isNull(model)) {
                return;
            }

            onPostExecute.setObject(result);
            model.H0.post(onPostExecute);
        }
    }
}
