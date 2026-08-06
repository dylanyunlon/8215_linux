package com.hcn.media.base.layout;

import android.content.Context;
import android.util.AttributeSet;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;

import com.hcn.media_base.impl.MediaEventPostbox;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media.music.IMusicLayoutInterface;
import com.hcn.media.vm.action.IPlayerEx;
import com.hcn.media.vm.base.BaseViewModel;
import com.hcn.media.vm.base.VmCommand;
import com.hcn.rxrelay3.PublishRelay;
import com.hcn.skinx.extend.SkinExFrameLayout;

/**
 * 帧布局扩展
 * <p> 为支持 MVVM/ViewModel 设计模式；
 *
 * @author 65821
 */
public abstract class FrameLayoutEx extends SkinExFrameLayout
        implements IMusicLayoutInterface {

    /** 全局的数据对象 **/
    protected final AppGlobalData mAppData;

    /** 播放器相关接口 **/
    private final IPlayerEx mPlayer;

    /**  **/
    protected MediaEventPostbox mMediaEventPostbox = null;

    /**
     * 抽象类的构造函数/为子类构造时调用；
     *
     * @param context      上下文环境
     * @param attrs        使视图膨胀的 xml 标记属性
     * @param defStyleAttr 当前主题中的一个属性，它包含对样式资源的引用，该样式资源为视图提供默认值。<br>
     *                     如果为 0 表示不查找默认值。
     * @param player       播放器接口
     */
    public FrameLayoutEx(@NonNull Context context,
                         @Nullable AttributeSet attrs,
                         int defStyleAttr,
                         @NonNull IPlayerEx player) {
        super(context, attrs, defStyleAttr, 0);

        mPlayer = player;
        mAppData = AppGlobalData.getInstance();
    }

    /**
     * 媒体播放器相关接口
     * @return {@link IPlayerEx}
     */
    protected IPlayerEx player() {
        return mPlayer;
    }

    /**
     * 播放器 Action 中继器
     * @return {@link PublishRelay}
     */
    protected PublishRelay<VmCommand.Action<BaseViewModel.IPlayer>> playerRelay() {
        return mPlayer.playerRelay();
    }

    @Override
    public abstract void initLayout();

    @Override
    public void initDataObject() {
        // TODO: 需要就重载，为兼容历史代码而设计的接口
    }

    /**
     * 获取指定资源的字符串信息
     * <p> 只能获取本地资源中的字符串信息；
     *
     * @param id 本地资源 ID
     * @return 字符串信息
     */
    protected String getString(@StringRes int id) {
        return getResources().getString(id);
    }

    @Override
    public abstract void doCallbackEvent(int eventID);

    @Override
    public void setMediaEventListener(MediaEventPostbox listener) {
        mMediaEventPostbox = listener;
    }
}
