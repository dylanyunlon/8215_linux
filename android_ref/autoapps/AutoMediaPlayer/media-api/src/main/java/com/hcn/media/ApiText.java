package com.hcn.media;

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.View;

import com.hcn.media.api.HMediaApi;
import com.hcn.media.base.IConnectionState;
import com.hcn.media.base.IEvent;
import com.hcn.media.base.IPlayInfoChanged;
import com.hcn.media.base.MediaPlayInfo;
import com.hcn.media_view.lyrics.LyricsRow;

import java.util.List;

/**
 * 使用方法说明
 * @author 65821
 */
abstract class ApiText {

    Activity activity = null;

    /**
     * 禁止使用
     * @param context
     */
    private void textCode(Context context) {
        // 构建媒体接口对象
        HMediaApi mediaApi = HMediaApi.createMediaApi(context, "test");

        // 初始化 api 配置
        mediaApi.init()
                .setDebug(-1, true)
                .setDebug(Log.VERBOSE, true)
                .registerListener(new IConnectionState() {
                    @Override
                    public void onConnected() {
                        // 表示连接上媒体服务了
                    }

                    @Override
                    public void onDisconnected() {
                    }

                    @Override
                    public void onDied() {
                    }
                });

        // 启动媒体 app（可随意调用）
        mediaApi.requestStartApp();

        // 控制相关接口（可以随意调用）
        mediaApi.musicPlayPause();
        mediaApi.musicPlay();
        mediaApi.musicPause();
        mediaApi.musicPlayNext();
        mediaApi.musicPlayPrev();
        mediaApi.musicSwitchPlayMode();

        // 监听媒体信息改变（不用记得取消注册）
        mediaApi.registerListener(new IPlayInfoChanged() {
            @Override
            public void onPlayInfoChanged(String event, MediaPlayInfo info) {
                switch (event) {
                    case IEvent.MUSIC_PLAY_INFO:
                        // 音乐信息改变
                        break;
                    case IEvent.MUSIC_LYRICS_INFO:
                        // 歌词信息通知
                        // 可以处理歌词信息
                        break;
                    case IEvent.MUSIC_PLAY_STATE:
                        // 音乐播放状态改变
                        break;
                    case IEvent.MUSIC_PLAY_TIME:
                        // 音乐播放时间改变
                        // 可以处理歌词信息显示变化
                        break;
                    default:
                        break;
                }
            }
        });

        // 主动获取媒体信息
        MediaPlayInfo info = mediaApi.mediaPlayInfo();
        if (info != null) {
            //  获取到了媒体信息
            String file = info.getFilePath();
            String title = info.getTitle();
            String artist = info.getArtist();
            String album = info.getAlbum();

            String state = info.getState();
            int duration = info.getDuration();
            int current = info.getCurrentPosition();

            String lyrics = info.getLyricsFile();
        }

        // 请求更新媒体信息
        // 会触发 IPlayInfoChanged 回调接口
        // 参考 { @link registerListener(IPlayInfoChanged listener); }
        mediaApi.requestPlayInfo();

        // 通用歌词视图使用
        // 参考 { @link LyricsView }
        // View lyricsView = activity.findViewById(R.id.lyrics_view);
        // mediaApi.setLyricsView(lyricsView);

        // 获取歌词信息集合
        // 主要用来给需要做自定义歌词显示场景使用
        List<LyricsRow> list = mediaApi.mediaLyricsRowInfo();
    }
}
