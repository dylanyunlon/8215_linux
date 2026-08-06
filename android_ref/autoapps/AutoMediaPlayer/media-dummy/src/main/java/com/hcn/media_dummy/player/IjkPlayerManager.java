package com.hcn.media_dummy.player;

import android.annotation.SuppressLint;
import android.content.ContentResolver;
import android.content.Context;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.text.TextUtils;
import android.view.Surface;

import com.hcn.common.misc.LogUtils;
import com.hcn.media_dummy.base.cache.ICacheManager;
import com.hcn.media_dummy.base.model.FunModel;
import com.hcn.media_dummy.base.model.MediaOptionModel;
import com.hcn.media_dummy.base.player.BasePlayerManager;
import com.hcn.media_dummy.utils.FunVideoType;
import com.hcn.media_dummy.source.RawDataSourceProvider;
import com.hcn.media_dummy.source.StreamDataSourceProvider;

import java.io.BufferedInputStream;
import java.io.FileDescriptor;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import tv.danmaku.ijk.media.player.IMediaPlayer;
import tv.danmaku.ijk.media.player.IjkLibLoader;
import tv.danmaku.ijk.media.player.IjkMediaPlayer;
import tv.danmaku.ijk.media.player.misc.IjkTrackInfo;

/**
 * Ijk 播放器管理器
 * @author 65821
 */
public class IjkPlayerManager extends BasePlayerManager {
    private static final String TAG = "IjkPlayerManager";

    private static int logLevel = IjkMediaPlayer.IJK_LOG_DEFAULT;

    private static IjkLibLoader ijkLibLoader;

    private IjkMediaPlayer mediaPlayer;

    private List<MediaOptionModel> optionModelList;

    private Surface surface;

    @Override
    public IMediaPlayer getMediaPlayer() {
        return mediaPlayer;
    }

    @SuppressLint("Recycle")
    @Override
    public void initMediaPlayer(Context context,
                                Message msg,
                                List<MediaOptionModel> optionModelList,
                                ICacheManager cacheManager) {
        mediaPlayer = (ijkLibLoader == null)?
                new IjkMediaPlayer(): new IjkMediaPlayer(ijkLibLoader);
        mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
        mediaPlayer.setOnNativeInvokeListener(new IjkMediaPlayer.OnNativeInvokeListener() {
            @Override
            public boolean onNativeInvoke(int i, Bundle bundle) {
                return true;
            }
        });

        FunModel funModel = (FunModel) msg.obj;
        String url = funModel.getUrl();
        BufferedInputStream videoBufferedInputStream = funModel.getVideoBufferedInputStream();

        try {
            // 开启硬解码
            if (FunVideoType.isMediaCodec()) {
                LogUtils.iTag(TAG, "enable mediaCodec");

                mediaPlayer.setOption(
                        IjkMediaPlayer.OPT_CATEGORY_PLAYER,
                        "mediacodec",
                        1);
                mediaPlayer.setOption(
                        IjkMediaPlayer.OPT_CATEGORY_PLAYER,
                        "mediacodec-auto-rotate",
                        1);
                mediaPlayer.setOption(
                        IjkMediaPlayer.OPT_CATEGORY_PLAYER,
                        "mediacodec-handle-resolution-change",
                        1);
            }

            if (funModel.isCache() && cacheManager != null) {
                cacheManager.doCacheLogic(context, mediaPlayer,
                        url, funModel.getMapHeadData(), funModel.getCachePath());
            } else {
                if (!TextUtils.isEmpty(url)) {
                    Uri uri = Uri.parse(url);
                    if (uri != null && uri.getScheme() != null
                            && (uri.getScheme().equals(ContentResolver.SCHEME_ANDROID_RESOURCE)
                                || "assets".equals(uri.getScheme()))) {
                        RawDataSourceProvider rawDataSourceProvider
                                = RawDataSourceProvider.create(context, uri);
                        mediaPlayer.setDataSource(rawDataSourceProvider);
                    } else if (uri != null && uri.getScheme() != null
                            && uri.getScheme().equals(ContentResolver.SCHEME_CONTENT)) {
                        ParcelFileDescriptor descriptor;
                        try {
                            descriptor = context.getContentResolver().openFileDescriptor(uri, "r");
                            FileDescriptor fileDescriptor = descriptor.getFileDescriptor();
                            mediaPlayer.setDataSource(fileDescriptor);
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    } else {
                        mediaPlayer.setDataSource(url, funModel.getMapHeadData());
                    }
                } else if (videoBufferedInputStream != null) {
                    mediaPlayer.setDataSource(new StreamDataSourceProvider(videoBufferedInputStream));
                } else {
                    mediaPlayer.setDataSource(url, funModel.getMapHeadData());
                }
            }

            mediaPlayer.setLooping(funModel.isLooping());
            if (funModel.getSpeed() != 1 && funModel.getSpeed() > 0) {
                mediaPlayer.setSpeed(funModel.getSpeed());
            }

            IjkMediaPlayer.native_setLogLevel(logLevel);
            initIjkOption(mediaPlayer, optionModelList);
        } catch (IOException e) {
            e.printStackTrace();
        }

        initSuccess(funModel);
    }

    @Override
    public void showDisplay(Message msg) {
        if (msg.obj == null && mediaPlayer != null) {
            mediaPlayer.setSurface(null);
        } else {
            Surface holder = (Surface) msg.obj;
            surface = holder;
            if (mediaPlayer != null && holder.isValid()) {
                mediaPlayer.setSurface(holder);
            }
        }
    }

    @Override
    public void setSpeed(float speed, boolean soundTouch) {
        if (speed > 0) {
            try {
                if (mediaPlayer != null) {
                    mediaPlayer.setSpeed(speed);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }

            if (soundTouch) {
                MediaOptionModel mediaOptionModel =
                        new MediaOptionModel(
                                IjkMediaPlayer.OPT_CATEGORY_PLAYER,
                                "soundtouch", 1);
                List<MediaOptionModel> list = getOptionModelList();
                if (list != null) {
                    list.add(mediaOptionModel);
                } else {
                    list = new ArrayList<>();
                    list.add(mediaOptionModel);
                }
                setOptionModelList(list);
            }
        }
    }

    @Override
    public void setNeedMute(boolean needMute) {
        if (mediaPlayer != null) {
            if (needMute) {
                mediaPlayer.setVolume(0, 0);
            } else {
                mediaPlayer.setVolume(1, 1);
            }
        }
    }

    @Override
    public void setVolume(float left, float right) {
        if (mediaPlayer != null) {
            mediaPlayer.setVolume(left, right);
        }
    }

    @Override
    public void releaseSurface() {
        if (surface != null) {
            //surface.release();
            surface = null;
        }
    }

    @Override
    public void release() {
        if (mediaPlayer != null) {
            mediaPlayer.release();
            mediaPlayer = null;
        }
    }

    @Override
    public int getBufferedPercentage() {
        return -1;
    }

    @Override
    public long getNetSpeed() {
        if (mediaPlayer != null) {
            return mediaPlayer.getTcpSpeed();
        }
        return 0;
    }

    @Override
    public void setSpeedPlaying(float speed, boolean soundTouch) {
        if (mediaPlayer != null) {
            mediaPlayer.setSpeed(speed);
            mediaPlayer.setOption(
                    IjkMediaPlayer.OPT_CATEGORY_PLAYER,
                    "soundtouch", (soundTouch) ? 1 : 0);
        }
    }

    @Override
    public void start() {
        if (mediaPlayer != null) {
            mediaPlayer.start();
        }
    }

    @Override
    public void stop() {
        if (mediaPlayer != null) {
            mediaPlayer.stop();
        }
    }

    @Override
    public void pause() {
        if (mediaPlayer != null) {
            mediaPlayer.pause();
        }
    }

    @Override
    public int getVideoWidth() {
        if (mediaPlayer != null) {
            return mediaPlayer.getVideoWidth();
        }
        return 0;
    }

    @Override
    public int getVideoHeight() {
        if (mediaPlayer != null) {
            return mediaPlayer.getVideoHeight();
        }
        return 0;
    }

    @Override
    public boolean isPlaying() {
        if (mediaPlayer != null) {
            return mediaPlayer.isPlaying();
        }
        return false;
    }

    @Override
    public void seekTo(long time) {
        if (mediaPlayer != null) {
            mediaPlayer.seekTo(time);
        }
    }

    @Override
    public long getCurrentPosition() {
        if (mediaPlayer != null) {
            return mediaPlayer.getCurrentPosition();
        }
        return 0;
    }

    @Override
    public long getDuration() {
        if (mediaPlayer != null) {
            return mediaPlayer.getDuration();
        }
        return 0;
    }

    @Override
    public boolean isSurfaceSupportLockCanvas() {
        return true;
    }

    public IjkTrackInfo[] getTrackInfo() {
        if (mediaPlayer != null) {
            return mediaPlayer.getTrackInfo();
        }
        return null;
    }

    public int getSelectedTrack(int trackType) {
        if (mediaPlayer != null) {
            return mediaPlayer.getSelectedTrack(trackType);
        }
        return -1;
    }

    public void selectTrack(int track) {
        if (mediaPlayer != null) {
            mediaPlayer.selectTrack(track);
        }
    }

    public void deselectTrack(int track) {
        if (mediaPlayer != null) {
            mediaPlayer.deselectTrack(track);
        }
    }

    private void initIjkOption(IjkMediaPlayer ijkMediaPlayer,
                               List<MediaOptionModel> optionModelList) {
        if (optionModelList != null && optionModelList.size() > 0) {
            for (MediaOptionModel mediaOptionModel : optionModelList) {
                if (mediaOptionModel.getValueType() == MediaOptionModel.VALUE_TYPE_INT) {
                    ijkMediaPlayer.setOption(mediaOptionModel.getCategory(),
                            mediaOptionModel.getName(), mediaOptionModel.getValueInt());
                } else {
                    ijkMediaPlayer.setOption(mediaOptionModel.getCategory(),
                            mediaOptionModel.getName(), mediaOptionModel.getValueString());
                }
            }
        }
    }

    public List<MediaOptionModel> getOptionModelList() {
        return optionModelList;
    }

    public void setOptionModelList(List<MediaOptionModel> optionModelList) {
        this.optionModelList = optionModelList;
    }

    public static IjkLibLoader getIjkLibLoader() {
        return ijkLibLoader;
    }

    public static void setIjkLibLoader(IjkLibLoader ijkLibLoader) {
        IjkPlayerManager.ijkLibLoader = ijkLibLoader;
    }

    public static int getLogLevel() {
        return logLevel;
    }

    public static void setLogLevel(int logLevel) {
        IjkPlayerManager.logLevel = logLevel;
    }
}
