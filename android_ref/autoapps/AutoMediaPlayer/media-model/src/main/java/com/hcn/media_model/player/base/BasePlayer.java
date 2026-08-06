package com.hcn.media_model.player.base;

import android.content.Context;

import androidx.annotation.NonNull;

import com.hcn.media_common.HMessage;
import com.hcn.media_base.IState;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_model.base.ILocalzModel;
import com.hcn.media_model.base.IPlayerModel;
import com.hcn.rxrelay3.PublishRelay;
import com.hcn.rxrelay3.Relay;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;

/**
 * 播放器基础类
 * <p> 抽取公共部分实现；
 *
 * @author 65821
 */
public abstract class BasePlayer implements IState {

    /** 全局数据与模型引用 **/
    protected final AppGlobalData mAppData;
    protected final Reference<Context> mContextRef;

    protected final ILocalzModel mLocalzModel;
    protected final IPlayerModel mPlayerModel;

    /** 当前播放文件信息 **/
    protected String mFilePath = "";

    /** 当前播放尺寸信息 **/
    protected int mVideoWidth;
    protected int mVideoHeight;
    protected boolean mVideoSizeChanged = false;

    /** 当前播放时间信息 **/
    protected int mSeekTime = 0;
    protected boolean mSeekToFlag = false;

    /** 对外事件中继器 **/
    private @NonNull
    final Relay<HMessage> mEventRelay = PublishRelay.create();

    /**
     * 默认构造函数
     *
     * @param context 应用上下文环境
     * @param localzModel {@link ILocalzModel} 业务模型
     * @param playerModel {@link IPlayerModel} 播放模型
     */
    public BasePlayer(Context context,
                      ILocalzModel localzModel,
                      IPlayerModel playerModel) {
        mAppData = AppGlobalData.getInstance();

        mContextRef = new WeakReference<>(context);
        mLocalzModel = localzModel;
        mPlayerModel = playerModel;
    }

    /**
     * 事件中继器
     * <p> 播放组件对外传输事件使用；
     *
     * @return CameraModel 事件中继器对象
     */
    public @NonNull Relay<HMessage> eventRelay() {
        return mEventRelay;
    }

    /**
     * 调度媒体事件
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
}
