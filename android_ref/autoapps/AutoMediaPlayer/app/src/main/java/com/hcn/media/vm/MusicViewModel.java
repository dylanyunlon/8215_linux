package com.hcn.media.vm;

import android.app.Application;

import androidx.annotation.NonNull;

import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_common.HBroadcastEx;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_data.FavoriteManager;

import com.hcn.media_model.MediaModel;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.media.vm.base.BaseViewModel;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.List;

/**
 * 音乐视图模型
 * <pre>
 *    为 UI 组件之间传递事件提供通道；
 *    为 Service 和 UI 组件之间传递事件提供通道；
 *    也可以绑定其它服务，为 UI 组件提供数据状态；
 * </pre>
 *
 * @author 65821
 */
public class MusicViewModel extends BaseViewModel {
    private static final String TAG = MusicViewModel.class.getSimpleName();

    /**
     * 构造函数
     * <p> AndroidViewModelFactory 标准构造；
     *
     * @param application 当前全局应用组件
     */
    public MusicViewModel(@NonNull Application application) {
        super(application);
        LogUtil.v(TAG, "Constructor.");

        // 观察 Ui 请求的播放事件
        mCompositeDisposable.add(mPlayerRelay.subscribe(
                iPlayerAction -> iPlayerAction.exec(new PlayerImpl() {

                    @Override
                    public void requestPlayTarget(@IPlaylistType int type,
                                                  List<MusicInfo> infoList,
                                                  int position) {
                        MediaModel.call()
                                .localzModel()
                                .requestPlayMusicInfo(type, infoList, position);
                    }

                    @Override
                    public boolean requestUpdatePlaylist(int type,
                                                      List<MusicInfo> infoList) {
                        return MediaModel.call()
                                .localzModel()
                                .requestUpdateMusicPlaylist(type, infoList);
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
                                        . requestSwitchRepeatMode(IMusicState.MEDIA_TYPE_MUSIC);
                                break;
                            case IMediaAction.none:
                            default:
                                break;
                        }
                    }
                })));

        // 监听音乐收藏动作事件
        FavoriteManager.getInstance().addOperateListener(
                FavoriteManager.Type.MUSIC, mFavoriteListOperateListener);
    }

    /** 最喜欢的列表操作监听器 **/
    private final FavoriteManager.IOperateListener
            mFavoriteListOperateListener = (listType, operate, obj0) -> {
                // 只处理关心的类型操作
                if (listType != FavoriteManager.Type.MUSIC) {
                    return;
                }

                // 当前必须在音乐播放模式
                if (!AppGlobalData.getInstance()
                        .isMediaType(IMusicState.MEDIA_TYPE_MUSIC)) {
                    return;
                }

                pageEventRelay().accept(t -> t.onPageEvent(
                        IMediaEvent.EVENT_MUSIC_FAVORITE_OPERATE, operate, obj0));
            };

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
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
            case IMediaEvent.EVENT_CHANGE_REPEAT_MODE:
            case IMediaEvent.EVENT_MUSIC_PLAYER_PREPARING:
            case IMediaEvent.EVENT_UPDATE_MUSIC_ID3:
            case IMediaEvent.EVENT_CHANGE_MUSIC_STORAGE:
            case IMediaEvent.EVENT_UPDATE_MUSIC_LIST:
            case IMediaEvent.EVENT_MEDIA_MOUNTED:
            case IMediaEvent.EVENT_MEDIA_UNMOUNTED:
            case IMediaEvent.EVENT_MEDIA_LOADING_START:
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE:
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
        LogUtil.v(TAG, "onCleared.");

        // 移除收藏列表操作监听器
        FavoriteManager.getInstance().removeOperateListener(
                FavoriteManager.Type.MUSIC, mFavoriteListOperateListener);
    }
}
