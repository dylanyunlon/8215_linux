package com.hcn.media.vm.action;

import com.hcn.media_model.base.IMediaAudio;
import com.hcn.media.vm.base.BaseViewModel;
import com.hcn.media.vm.base.VmCommand;
import com.hcn.media_model.player.base.IMediaPlayer;
import com.hcn.rxrelay3.PublishRelay;

/**
 * 为视图元素提供播放相关接口
 * <pre>
 *    为了管控 VM 的接口作用域；
 *    像 Layout 这里元素，不容许直接使用 ViewModel 对象；
 * </pre>
 *
 * @author 65821
 */
public interface IPlayerEx extends IMediaAudio {
    /**
     * 获取当前视频平台解码组件
     * @return {@link IMediaPlayer}
     */
    IMediaPlayer corePlayer();

    /**
     * 获取当前视频软解码组件
     * @return {@link IMediaPlayer}
     */
    IMediaPlayer vitamioPlayer();

    /**
     * 播放器 Action 中继器
     * @return {@link PublishRelay}
     */
    PublishRelay<VmCommand.Action<BaseViewModel.IPlayer>> playerRelay();
}
