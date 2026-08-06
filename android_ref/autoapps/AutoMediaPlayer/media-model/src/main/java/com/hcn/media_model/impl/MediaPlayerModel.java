package com.hcn.media_model.impl;

import android.content.Context;
import android.text.TextUtils;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;

import com.hcn.media_common.HBroadcastEx;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_model.player.PlayerFactory;
import com.hcn.media_model.player.base.IMediaPlayer;
import com.hcn.media_theme.Argument;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.IMediaEventListener;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_model.base.BaseModel;
import com.hcn.media_model.base.ILocalzModel;
import com.hcn.media_model.base.IPlayerModel;
import com.hcn.media_view.lyrics.LyricsManager;
import com.hcn.media_view.lyrics.LyricsRow;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.List;
import java.util.Objects;

/**
 * MVVM/Model
 * <pre>
 *    主要是提供给 Player 组件和 LocalService 使用；
 *    服务调用 IPlayerModel 访问播放相关的状态和接口（播放/暂停等）；
 *    Player 组件对应的回调媒体事件到当前 Model，当前 Model 再把事件传递给 ILocalzModel;
 *    LocalService ----> IPlayerModel
 *    MediaPlayer <----> IPlayerModel ----> ILocalzModel ---- LocalService
 * </pre>
 *
 * @author 65821
 */
final class MediaPlayerModel extends BaseModel
        implements IMediaEventListener, IPlayerModel {
    private static final String TAG = MediaPlayerModel.class.getSimpleName();

    /** Model 必须是唯一实例设计 **/
    private static MediaPlayerModel sInstance = null;

    /** PlayerModel 对外接口实例 **/
    public static MediaPlayerModel instance() {
        if (Objects.isNull(sInstance)) {
            throw new RuntimeException(
                    "Please initialize [MediaPlayerModel] Object!");
        }

        return sInstance;
    }

    /**
     * 初始化 MediaPlayerModel 模型
     * <p> 注意 PlayerModel 可以访问 ILocalzModel；
     *
     * @param context 当前应用上下文环境
     * @param localzModel {@link MediaLocalzModel}
     */
    public static void init(@NonNull Context context, @NonNull ILocalzModel localzModel) {
        if (Objects.isNull(sInstance)) {
            sInstance = new MediaPlayerModel(context, localzModel);
        } else {
            throw new RuntimeException(
                    "[MediaPlayerModel] already initialized!");
        }
    }

    /**
     * 播放组件
     * <pre>
     *    1、mPlatformPlayer 走平台的 MediaPlayer 封装；
     *    2、mVitamioPlayer 走第三方对 ffmpeg 的封装；
     * </pre>
     */
    private @NonNull final IMediaPlayer mPlatformPlayer;
    private final IMediaPlayer mVitamioPlayer;

    /** 主业务逻辑/当前 Model 与外部沟通的桥梁 **/
    private @NonNull final ILocalzModel mLocalzModel;

    /** 禁止构造无参对象 **/
    private MediaPlayerModel() {
        super(null, null);
        throw new RuntimeException(
                "Prohibit the construction of parameterless objects");
    }

    /**
     * MediaModel 构造函数
     * <p> 禁止在外部直接访问它；
     *
     * @param context 当前应用上下文环境
     * @param localzModel {@link MediaLocalzModel}
     */
    private MediaPlayerModel(@NonNull Context context, @NonNull ILocalzModel localzModel) {
        super(context, null);
        mLocalzModel = localzModel;

        // 初始化平台播放组件
        PlayerFactory.setPlayer(PlayerFactory.PLATFORM_PLAYER);
        mPlatformPlayer = Objects.requireNonNull(
                PlayerFactory.buildPlayer(context, localzModel, this));

        // 初始化软解播放组件
        PlayerFactory.setPlayer(PlayerFactory.VITAMIO_PLAYER);
        mVitamioPlayer = Argument.isSupportVitamio() ?
                PlayerFactory.buildPlayer(context, localzModel, this) : null;
    }

    /**
     * 获取系统平台播放组件
     * <p> 系统平台播放器是使用硬解码还是软解码取决于平台本身；
     *
     * @return {@link IMediaPlayer}
     */
    @Override
    public IMediaPlayer corePlayer() {
        return mPlatformPlayer;
    }

    /**
     * 获取 Vitamio 播放组件
     * <p> Vitamio 是基于 ffmpeg 的软解码播放组件；
     *
     * @return {@link IMediaPlayer}
     */
    @Override
    public IMediaPlayer vitamioPlayer() {
        return mVitamioPlayer;
    }

    @Override
    public boolean existsValidMediaPlayer() {
        if (sAppData.mSoftCodeFlag) {
            return isVitamioPlayerValid();
        } else {
            return isMediaPlayerValid();
        }
    }

    @Override
    public boolean isMediaPlayerValid() {
        return mPlatformPlayer.isInited();
    }

    @Override
    public boolean isVitamioPlayerValid() {
        if (mVitamioPlayer != null) {
            return mVitamioPlayer.isInited();
        }

        return false;
    }

    /**
     * 在软解播放高清视频
     * <p> 该状态用来前台提高当前进程优先级使用；
     * @return 是/否
     */
    @Override
    public boolean inSoftDecodingHDVideo() {
        if (isVitamioPlayerValid()) {
            return vitamioPlayer().is1080PVideoSource();
        }

        return false;
    }

    public void onPlayControlEvent(int nCommand) {
        onPlayControlEvent(nCommand, 0);
    }

    /**
     * 下发播放控制事件
     *
     * @param nCommand 播放/暂停/上下曲/...
     * @param reason 调用调试原因
     */
    @Override
    public void onPlayControlEvent(int nCommand, int reason) {
        LogUtil.d(TAG, "onPlayControlEvent, nCommand: " + nCommand);

        // [理论上没必要特殊处理, 但是我们尽可能不随意动它]
        if (IMusicState.PLAY_CMD_STOP == nCommand) {
            mPlatformPlayer.onPlayControlEvent(nCommand, reason);

            if (null != mVitamioPlayer) {
                mVitamioPlayer.onPlayControlEvent(nCommand, reason);
            }
            return;
        }

        if (!sAppData.mSoftCodeFlag) {
            if (mPlatformPlayer.isInited()) {
                mPlatformPlayer.onPlayControlEvent(nCommand, reason);
            } else {
                LogUtil.d(TAG, "onPlayControlEvent, uninitialized!");
            }
        } else if (null != mVitamioPlayer) {
            mVitamioPlayer.onPlayControlEvent(nCommand, reason);
        }
    }

    /** 不用关心软解还是硬解，需要同步生效设置的目标值 **/
    @Override
    public void requestSetVolume(float volume) {
        if (null != mPlatformPlayer) {
            mPlatformPlayer.requestSetVolume(volume);
        }

        if (null != mVitamioPlayer) {
            mVitamioPlayer.requestSetVolume(volume);
        }
    }

    @Override
    public int getCurrentPosition() {
        if (!sAppData.mSoftCodeFlag) {
            if (mPlatformPlayer.isInited()) {
                return mPlatformPlayer.getCurrentPosition();
            }
        } else {
            if (Argument.isSupportVitamio()) {
                return mVitamioPlayer.getCurrentPosition();
            }
        }

        return -1;
    }

    @Override
    public List<LyricsRow> getLyricsInfo(String path) {
        return LyricsManager.instance().readLrcFromSong(path);
    }

    /**
     * 该接口只能表示 MediaPlayer 正在播放状态
     * <p> 它无法用来表示播放曲目切曲的衔接状态, 用它来判定是否可以进入 PIP 不严谨
     * @return
     */
    public boolean getPlayerStatus() {
        if (!sAppData.mSoftCodeFlag) {
            if (mPlatformPlayer.isInited()) {
                return mPlatformPlayer.isPlayState();
            }
        } else {
            if (Argument.isSupportVitamio()) {
                return mVitamioPlayer.isPlayState();
            }
        }

        return false;
    }

    @Override
    public int getTotalTime() {
        if (!sAppData.mSoftCodeFlag) {
            if (mPlatformPlayer.isInited()) {
                return mPlatformPlayer.getTotalTime();
            }
        } else {
            if (Argument.isSupportVitamio()) {
                return mVitamioPlayer.getTotalTime();
            }
        }

        return -1;
    }

    @Override
    public void seekToTime(int nTime) {
        if (!sAppData.mSoftCodeFlag) {
            if (mPlatformPlayer.isInited()) {
                mPlatformPlayer.seekToTime(nTime);
            }
        } else if (null != mVitamioPlayer) {
            mVitamioPlayer.seekToTime(nTime);
        }
    }

    /**
     * 设置多媒体播放源
     * <pre>
     *    这些类似的接口放这里有些莫名其妙；
     *    理论上播放组件（HCorePlayer/VitamioPlayer）对象都不能放 Application 初始化;
     * </pre>
     *
     * @param info 播放信息对象
     */
    @Override
    public void onLocalSetDataSourceEvent(MusicInfo info) {
        String strPath = info.mFilePath;
        if (TextUtils.isEmpty(strPath)) {
            return;
        }

        if (Argument.isSupportVitamio()) {
            // [如果是支持软解先停止播放, 且会同时上报播放状态]
            mVitamioPlayer.onPlayControlEvent(IMusicState.PLAY_CMD_STOP, 1);
        }

        // [如果前一个目标是软解播放，走硬解播放需要替换 SurfaceView]
        if (sAppData.mMediaType == IMusicState.MEDIA_TYPE_VIDEO) {
            if (sAppData.mSoftCodeFlag) {
                HBroadcastEx.sendLocalBroadcast(mContextRef.get(),
                        IMediaEvent.EVENT_CHANGE_SURFACE_VIEW_TARGET);
            }
        }

        sAppData.mSoftCodeFlag = false;
        mPlatformPlayer.onSetDataSourceEvent(info);
    }

    @Override
    public void updateCoreSurfaceHolder() {
        mPlatformPlayer.updateSurfaceHolder(false);
    }

    @Override
    public void updateRearSurfaceHolder() {
        if (!sAppData.mSoftCodeFlag) {
            if (mPlatformPlayer.isInited()) {
                mPlatformPlayer.updateSurfaceHolder(false);
            }
        } else {
            mVitamioPlayer.updateSurfaceHolder(false);
        }
    }

    @Override
    public void onSetSeekTimeZero() {
        // [停止写记录]
        mPlatformPlayer.onSetSeekTimeZero();
        if (null != mVitamioPlayer) {
            mVitamioPlayer.onSetSeekTimeZero();
        }

        // [写播放记录]
        if (null != sAppData.mCurrentMediaInfo) {
            mLocalzModel.writeMediaTime(
                    sAppData.mLastMediaType,
                    sAppData.mCurrentMediaInfo.mFilePath,
                    0,
                    101);
        }
    }

    @Override
    public void updateVitamioSurfaceHolder() {
        if (null != mVitamioPlayer) {
            mVitamioPlayer.updateSurfaceHolder(false);
        }
    }

    public void onAttachSurface(SurfaceHolder holder) {
        mPlatformPlayer.updateSurfaceHolder(false);

        if (null != mVitamioPlayer) {
            mVitamioPlayer.updateSurfaceHolder(false);
        }
    }

    public void onDetachSurface() {
        mPlatformPlayer.updateSurfaceHolder(false);

        if (null != mVitamioPlayer) {
            mVitamioPlayer.updateSurfaceHolder(false);
        }
    }

    /**
     * HCorePlayer/VitamioPlayer 调用
     * <p> 禁止把这个接口用作其它对象之间传递媒体事件；
     *
     * @param eventId 事件 ID
     * @param wParam  附加参数 1
     * @param lParam  附加阐述 2
     */
    @Override
    public void onMediaEvent(int eventId, Object wParam, Object lParam) {
        switch (eventId) {
            case IMediaEvent.EVENT_UNSUPPORT_VIDEO_CODE:
            case IMediaEvent.EVENT_CODE_UNSUPPORT: {
                boolean vitamioIdle = false;

                // [视频需要特殊处理, Vitamio 支持视频软解码]
                if (IMusicState.MEDIA_TYPE_VIDEO == sAppData.mMediaType) {
                    LogUtil.e(TAG, "onMediaClickEvent: [UNSUPPORT-VIDEO] Soft Decoding.");
                    vitamioIdle = Argument.isSupportVitamio() && (null != wParam)
                            && !mVitamioPlayer.isAsyncReleasing() && !BaseMediaData.isLowMemory();

                    // [如果软解不是 IDLE 状态, 不能使用软解]
                    if (vitamioIdle) {
                        sAppData.mSoftCodeFlag = true;
                        HBroadcastEx.sendLocalBroadcast(mContextRef.get(),
                                IMediaEvent.EVENT_CHANGE_SURFACE_VIEW_TARGET);
                        mPlatformPlayer.onPlayControlEvent(IMusicState.PLAY_CMD_STOP);
                        mVitamioPlayer.onSetDataSourceEvent((MusicInfo) wParam);
                    } else {
                        mPlatformPlayer.onPlayControlEvent(IMusicState.PLAY_CMD_STOP);
                    }
                }

                // [没有跳转软解, 需要触发硬解码下一曲流程]
                if (!vitamioIdle) {
                    if (Argument.isSupportVitamio()) {
                        dispatchMusicEvent(IMediaEvent.EVENT_MEDIA_COMPLETION);
                    } else {
                        // [不支持软解码情况下, 需要做 Toast 提示]
                        onMediaEvent(IMediaEvent.EVENT_VITAMIO_CODE_UNSUPPORT, null, null);
                    }
                }
                return;
            }

            case IMediaEvent.EVENT_VITAMIO_CODE_UNSUPPORT: {
                dispatchMusicEvent(IMediaEvent.EVENT_CODE_UNSUPPORT);
                dispatchMusicEvent(IMediaEvent.EVENT_MEDIA_COMPLETION);
                return;
            }

            case IMediaEvent.EVENT_UNSUPPORT_VIDEO_CODE2: {
                dispatchMusicEvent(IMediaEvent.EVENT_UNSUPPORT_VIDEO_CODE);
                return;
            }

            default:
                break;
        }

        mLocalzModel.dispatchMusicEvent(eventId, wParam, lParam);
    }

    /** @see #dispatchMusicEvent(int, Object) **/
    private void dispatchMusicEvent(int eventId) {
        dispatchMusicEvent(eventId, null);
    }

    /** @see ILocalzModel#dispatchMusicEvent(int, Object, Object) **/
    private void dispatchMusicEvent(int eventId, Object wParam) {
        mLocalzModel.dispatchMusicEvent(eventId, wParam, null);
    }
}
