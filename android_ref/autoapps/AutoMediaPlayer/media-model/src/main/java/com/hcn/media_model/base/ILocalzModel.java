package com.hcn.media_model.base;

import androidx.annotation.NonNull;

import com.hcn.media_base.IMediaEvent;

import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.List;

/**
 * 本地业务逻辑接口
 * <p> 提供访问 Localz 模型的接口；
 *
 * @author 65821
 */
public interface ILocalzModel extends IMediaAudio {

    /**
     * 本地服务是连接的
     * @return 是连接的/未连接的
     */
    boolean isLocalConnected();

    /**
     * 多媒体服务已经准备就绪
     * @return 准备好/未准备好
     */
    boolean isServiceReadyState();

    /**
     * 请求退出当前进程
     * @param reason 退出原因
     */
    void requestExitApp(int reason);

    /** 请求播放音频焦点 **/
    void onRequestAudioFocus();

    /** 开始触发播放流程 **/
    void requestSwitchMediaType();

    /**
     * 多媒体事件分发
     * <p> 这是 Player 类组件上报媒体事件到后台 Service 的通道；
     *
     * @param eventId 媒体事件 {@link IMediaEvent}
     * @param wParam 附加参数 1
     * @param lParam 附加参数 2
     */
    void dispatchMusicEvent(int eventId, Object wParam, Object lParam);

    /**
     * 判断目标路径所在存储设备是否在 Mounted 状态
     *
     * @param filePath 目标路径
     * @return 挂载状态/非挂载状态
     */
    boolean targetStorageMounted(String filePath);

    /**
     * 请求播放指定数据源事件
     * @param info 媒体文件对象
     */
    void requestPlayDataSource(MusicInfo info);

    /**
     * 读取匹配目标信息的媒体播放信息
     * <p> 如果没有与目标信息匹配的记忆就返回 0；
     *
     * @param type 播放类型
     * @param path 文件路径
     * @return
     */
    int readMediaTime(int type, String path);

    /**
     * 保存媒体播放信息
     * <p> 播放记忆存储待扩展；
     *
     * @param type 当前媒体类型
     * @param path 当前播放文件路径
     * @param time 当前时间
     * @param reason 触发原因
     */
    void writeMediaTime(int type, String path, int time, int reason);

    /** 请求注册媒体按键监听 **/
    void registerMediaButton();

    /**
     * 是否能够观看视频
     * <p> 刹车状态/行车中观看视频使能设置
     *
     * @return 能观看/不能观看
     */
    boolean isCanWatchVideo();

    /**
     * 是否能播放视频
     * <p> 行车中观看视频使能设置/刹车状态/后台播放/是否在前台视频模式等
     *
     * @return 能播放/不能播放
     */
    boolean isCanPlayVideo();

    /**
     * 存在高优先级状态
     * <p> 通话状态、ACC-OFF等
     *
     * @return 存在/不存在
     */
    boolean existsHighPriorityEvent();

    /**
     * 是否有 usb 在挂载状态
     * @return 是/否
     */
    boolean isUsbMounted();

    /**
     * 是否是 sdcard 挂载状态
     * @return 是/否
     */
    boolean isSdcardMounted();

    /**
     * 是否是 sdcard2 挂载状态
     * @return 是/否
     */
    boolean isSdCard2Mounted();

    /**
     * 是否在 ACC 点火状态
     * @return 是/否
     */
    boolean inAccOnState();

    /**
     * 请求对外广播当前音乐播放信息
     * <pre>
     *    用于通知外部应用更新当前音乐播放显示信息；
     *    如果当前不在音乐模式（则暂不处理，保留）；
     * </pre>
     */
    void requestBroadcastMusicPlayInfo();

    /**
     * 处理播放控制命令
     * @param command 命令 {@link com.hcn.media_base.constant.IMusicState}
     */
    void requestPlayControl(int command);

    /**
     * 处理音乐播放事件
     * <p> 这里的播放列表类型可以是：存储设备、文件夹、收藏等；
     *
     * @param playlistType 列表类型
     * @param infoList 目标列表
     * @param position 播放位置
     */
    void requestPlayMusicInfo(@IPlaylistType int playlistType,
                              List<MusicInfo> infoList,
                              int position);

    /**
     * 处理视频播放事件
     * <p> 这里的播放列表类型可以是：存储设备、文件夹、收藏等；
     *
     * @param type 列表类型
     * @param infoList 目标列表
     * @param position 播放位置
     */
    void requestPlayVideoInfo(@IPlaylistType int type,
                              List<MusicInfo> infoList,
                              int position);

    /**
     * 请求更新音乐播放列表
     * <pre>
     *    UI 操作了删除动作，导致播放列表改变；
     *    注意：它不一定会更新成功，更新成功将触发播放；
     *    如果当前播放列表类型不匹配，将更新失败；
     * </pre>
     *
     * @param playlistType 播放列表类型
     * @param infoList 待更新的列表数据
     * @return 更新成功/更新失败
     */
    boolean requestUpdateMusicPlaylist(@IPlaylistType int playlistType,
                                       @NonNull List<MusicInfo> infoList);

    /**
     * 请求更新视频播放列表
     * <p> 不一定更新成功，预留接口
     *
     * @param playlistType 播放列表类型
     * @param infoList 待更新的列表数据
     * @return 更新成功/更新失败
     */
    boolean requestUpdateVideoPlaylist(@IPlaylistType int playlistType,
                                       @NonNull List<MusicInfo> infoList);

    /**
     * 改变指定媒体类型的播放循环模式
     * @param mediaType {@link IMusicState#MEDIA_TYPE_MUSIC/VIDEO}
     */
    void requestSwitchRepeatMode(int mediaType);

    /**
     * 扫描刷新目标路径所在的存储设备媒体信息
     * @param filePath 指定路径
     */
    void requestScanTargetPath(String filePath);

    /** 应用开始或恢复播放事件 **/
    void doShouldPlayEvent();

    /**
     * 应用暂停或停止播放事件
     * @param stop 是否停止播放（Stop）
     * @param reason 执行原因
     */
    void doShouldPauseEvent(boolean stop, int reason);

    /**
     * 保存视频显示尺寸类型
     * @param type 1:1/16:9/4:3/Fullscreen
     */
    void writeVideoScaleType(int type);

    /**
     * 设置当前媒体音频会话 ID
     * @param Id audio session ID.
     */
    void setAudioSessionId(int Id);

    /**
     * 低内存的时候调用
     * @param reason 原因
     */
    void onLowMemory(int reason);
}
