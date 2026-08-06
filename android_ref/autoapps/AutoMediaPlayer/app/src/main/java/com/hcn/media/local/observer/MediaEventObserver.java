package com.hcn.media.local.observer;

import com.hcn.common.misc.HBusUtils;
import com.hcn.media.base.xbus.IBusTag;
import com.hcn.media.local.base.IEventObserver;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_view.lyrics.LyricsManager;
import com.hcn.mediaservice.data.MediaTimeInfo;
import com.hcn.mediaservice.data.MusicInfo;

/**
 * 媒体事件观察者
 * <pre>
 *    对外分发媒体状态事件，可以统一在此处理；
 *    主要是为 media-api-release.aar 提供数据支持；
 * </pre>
 *
 * @author 65821
 */
public class MediaEventObserver implements IEventObserver {

    /**
     * 分发媒体对外事件
     *
     * @param event  {@link com.hcn.media_base.IMediaEvent}
     * @param wParam 附加参数 w
     * @param lParam 附加参数 l
     */
    @Override
    public void dispatchMediaEvent(int event, Object wParam, Object lParam) {
        switch (event) {
            case IMediaEvent.EVENT_CHANGE_MEDIA_PLAYTIME:
                // 更新媒体播放时间（MediaTimeInfo）
                HBusUtils.post(IBusTag.UPDATE_MUSIC_PLAY_TIME, wParam);
                break;
            case IMediaEvent.EVENT_UPDATE_MUSIC_ID3:
                // 更新媒体 ID3 信息（MusicInfo）
                HBusUtils.post(IBusTag.UPDATE_MUSIC_PLAY_INFO, wParam);
                break;
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE:
                // 更新媒体播放状态（int）
                HBusUtils.post(IBusTag.UPDATE_MUSIC_PLAY_STATE, wParam);
                break;
            case IMediaEvent.EVENT_REQUEST_BROADCAST_MUSIC_PLAY_INFO:
                // 请求广播媒体播放信息（MusicInfo）
                MusicInfo info = BaseMediaData.call().mCurrentMediaInfo;
                HBusUtils.post(
                        IBusTag.UPDATE_MUSIC_PLAY_INFO,
                        info);
                HBusUtils.post(
                        IBusTag.UPDATE_MUSIC_LYRICS_INFO,
                        LyricsManager.instance().getLyricsFilePath(info.mFilePath));
                HBusUtils.post(
                        IBusTag.UPDATE_MUSIC_PLAY_STATE,
                        BaseMediaData.call().mediaPlayState());
                HBusUtils.post(
                        IBusTag.UPDATE_MUSIC_PLAY_TIME,
                        BaseMediaData.call().mPlayTimeInfo);
                break;
            case IMediaEvent.EVENT_NONE:
            default:
                break;
        }
    }
}
