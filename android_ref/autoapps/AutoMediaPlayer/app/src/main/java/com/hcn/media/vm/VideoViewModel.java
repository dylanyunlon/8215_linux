package com.hcn.media.vm;

import android.app.Application;

import androidx.annotation.NonNull;

import com.hcn.media_base.IMediaBroadcast;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_common.HBroadcastEx;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_model.MediaModel;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media.vm.base.BaseViewModel;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.List;

/**
 * 视频视图模型
 * <pre>
 *    为 UI 组件之间传递事件提供通道；
 *    为 Service 和 UI 组件之间传递事件提供通道；
 *    也可以绑定其它服务，为 UI 组件提供数据状态；
 * </pre>
 *
 * @author 65821
 */
public class VideoViewModel extends BaseViewModel {
    private static final String TAG = VideoViewModel.class.getSimpleName();

    public VideoViewModel(@NonNull Application application) {
        super(application);

        // 观察 Ui 请求的播放事件
        mCompositeDisposable.add(mPlayerRelay.subscribe(
                iPlayerAction -> iPlayerAction.exec(new PlayerImpl() {
                    @Override
                    public void requestPlayTarget(@IPlaylistType int type,
                                                  List<MusicInfo> infoList,
                                                  int position) {
                        MediaModel.call()
                                .localzModel()
                                .requestPlayVideoInfo(type, infoList, position);
                    }

                    @Override
                    public boolean requestUpdatePlaylist(int type,
                                                      List<MusicInfo> infoList) {
                        return MediaModel.call()
                                .localzModel()
                                .requestUpdateVideoPlaylist(type, infoList);
                    }

                    @Override
                    public boolean requestQueryState(@NonNull String action,
                                                     Object obj1) {
                        switch (action) {
                            case IMediaAction.isCanWatchVideo:
                                return MediaModel.call()
                                        .localzModel()
                                        .isCanWatchVideo();
                            case IMediaAction.existsValidMediaPlayer:
                                return MediaModel.call()
                                        .playerModel()
                                        .existsValidMediaPlayer();
                            case IMediaAction.isMediaPlayerValid:
                                return MediaModel.call()
                                        .playerModel()
                                        .isMediaPlayerValid();
                            case IMediaAction.isVitamioPlayerValid:
                                return MediaModel.call()
                                        .playerModel()
                                        .isVitamioPlayerValid();
                            default:
                                break;
                        }

                        return super.requestQueryState(action, obj1);
                    }

                    @Override
                    public void requestExecuteAction(@NonNull String action,
                                                     Object obj1,
                                                     Object obj2) {
                        super.requestExecuteAction(action, obj1, obj2);

                        switch (action) {
                            case IMediaAction.switchPlayRepeatMode:
                                MediaModel.call()
                                        .localzModel()
                                        .requestSwitchRepeatMode(IMusicState.MEDIA_TYPE_VIDEO);
                                break;
                            case IMediaAction.updateCoreSurfaceHolder:
                                MediaModel.call()
                                        .playerModel()
                                        .updateCoreSurfaceHolder();
                                break;
                            case IMediaAction.updateVitamioSurfaceHolder:
                                MediaModel.call()
                                        .playerModel()
                                        .updateVitamioSurfaceHolder();
                                break;
                            case IMediaAction.updateRearSurfaceHolder:
                                MediaModel.call()
                                        .playerModel()
                                        .updateRearSurfaceHolder();
                                break;
                            case IMediaAction.writeVideoScaleType:
                                MediaModel.call()
                                        .localzModel()
                                        .writeVideoScaleType((Integer) obj1);
                                break;
                            default:
                                break;
                        }
                    }
                })));
    }

    /** 当前是否正在软解播放高清视频 **/
    public boolean inSoftDecodingHDVideo() {
        return MediaModel.call()
                .playerModel()
                .inSoftDecodingHDVideo();
    }

    /**
     * 处理本地广播事件
     * @see HBroadcastEx.SpecialChain#ACTION_LOCAL_CALLBACK
     *
     * @param event 事件定义 {@link IMediaEvent}
     * @param wParam 附加参数 1
     * @param lParam 附加参数 2
     */
    @Override
    protected void onLocalBroadCastEvent(int event, Object wParam, Object lParam) {
        switch (event) {
            case IMediaEvent.EVENT_VIDEO_PLAYER_PREPARING:
                pageEventRelay().accept(t -> t.onPageEvent(event, wParam, lParam));
                break;
            case IMediaEvent.EVENT_NONE:
            default:
                break;
        }
    }

    @Override
    protected void onCleared() {
        super.onCleared();
    }
}
