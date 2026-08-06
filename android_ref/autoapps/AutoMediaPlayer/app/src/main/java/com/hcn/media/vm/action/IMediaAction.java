package com.hcn.media.vm.action;

import com.hcn.media_model.base.ILocalzModel;
import com.hcn.media_model.base.IPlayerModel;

import java.util.List;

/**
 * 媒体活动定义
 * <p> 采用字符串常量定义，每个常量对应一个动作；
 *
 * @author 65821
 */
public interface IMediaAction {
    /** 什么也不是 **/
    String none = "none";

    /**
     * 请求播放音频焦点
     * <p> {@link android.media.AudioManager}
     * @see ILocalzModel#onRequestAudioFocus()
     */
    String onRequestAudioFocus = "onRequestAudioFocus";

    /**
     * 是否有 usb 在挂载状态
     * @see ILocalzModel#isUsbMounted()
     */
    String isUsbMounted = "isUsbMounted";

    /**
     * 是否是 sdcard 挂载状态
     * @see ILocalzModel#isSdcardMounted()
     */
    String isSdcardMounted = "isSdcardMounted";

    /**
     * 是否是 ACC 点火状态
     * @see ILocalzModel#inAccOnState()
     */
    String inAccOnState = "inAccOnState";

    /**
     * 播放控制事件
     * <p> 播放、暂停、停止、上一曲、下一曲等
     * @see ILocalzModel#requestPlayControl(int)
     */
    String playControl = "playControl";

    /**
     * 播放目标列表中指定位置的曲目
     * <p> 会直接触发播放动作，并改变当前播放列表；
     * @see ILocalzModel#requestPlayMusicInfo(int, List, int)
     */
    String requestPlayMusicInfo = "requestPlayMusicInfo";

    /**
     * 跳转当前播放媒体进度到指定时间点
     * <p> 通知播放器组件执行播放进度跳转动作；
     * @see IPlayerModel#seekToTime(int)
     */
    String seekToTime = "seekToTime";

    /**
     * 通知并保存当前播放进度归零状态
     * <p> 清理部分状态、并保存当前播放相关的简要信息；
     * @see IPlayerModel#onSetSeekTimeZero()
     */
    String setSeekTimeZero = "setSeekTimeZero";

    /**
     * 改变当前媒体类型的播放模式
     * <p> 随机、循环所有、单曲循环、文件夹循环等
     * @see ILocalzModel#requestSwitchRepeatMode(int)
     */
    String switchPlayRepeatMode = "switchPlayRepeatMode";

    /**
     * 从新扫描当前指定路径所在存储设备的媒体数据
     * <p> 会触发 HMediaService 进程扫描指定存储设备；
     * @see ILocalzModel#requestScanTargetPath(String) (String)
     */
    String scanStorageDeviceInfo = "scanStorageDeviceInfo";

    /**
     * 保存视频播放显示尺寸类型
     * <p> 1:1/16:9/4:3/Fullscreen
     * @see ILocalzModel#writeVideoScaleType(int)
     */
    String writeVideoScaleType = "writeVideoScaleType";

    /**
     * 存在有效媒体播放组件
     * @see IPlayerModel#existsValidMediaPlayer()
     */
    String existsValidMediaPlayer = "existsValidMediaPlayer";

    /**
     * 平台播放组件是否有效
     * @see IPlayerModel#isMediaPlayerValid()
     */
    String isMediaPlayerValid = "isMediaPlayerValid";

    /**
     * Vitamio 播放组件是否有效
     * @see IPlayerModel#isVitamioPlayerValid()
     */
    String isVitamioPlayerValid = "isVitamioPlayerValid";

    /**
     * 本地服务是否连接
     * <p> LocalService 绑定成功；
     * @see ILocalzModel#isLocalConnected()
     */
    String isLocalConnected = "isLocalConnected";

    /**
     * 是否能够观看视频
     * @see ILocalzModel#isCanWatchVideo()
     */
    String isCanWatchVideo = "isCanWatchVideo";

    /**
     * 更新平台播放显示 SurfaceHolder
     * @see IPlayerModel#updateCoreSurfaceHolder()
     */
    String updateCoreSurfaceHolder = "updateCoreSurfaceHolder";

    /**
     * 更新软解播放显示 SurfaceHolder
     * @see IPlayerModel#updateVitamioSurfaceHolder()
     */
    String updateVitamioSurfaceHolder = "updateVitamioSurfaceHolder";

    /**
     * 更新后台播放显示 SurfaceHolder
     * @see IPlayerModel#updateRearSurfaceHolder()
     */
    String updateRearSurfaceHolder = "updateRearSurfaceHolder";
}
