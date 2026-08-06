package com.hcn.media_model.player;

import android.content.Context;

import androidx.annotation.NonNull;

import com.hcn.media_model.base.ILocalzModel;
import com.hcn.media_model.base.IPlayerModel;
import com.hcn.media_model.player.base.IMediaPlayer;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;

/**
 * 播放组件工具类
 * @author 65821
 */
public class PlayerFactory {
    /** 需要构建的播放器类型 **/
    private static String sPlayerType = "null";

    /** 播放器类型 **/
    public static final String PLATFORM_PLAYER = "HCorePlayer";
    public static final String VITAMIO_PLAYER = "VitamioPlayer";

    /**
     * 设置需要构建的播放器组件
     * @param playerType 播放器类型
     */
    public static void setPlayer(String playerType) {
        sPlayerType = playerType;
    }

    /**
     * 构建播放器对象
     * <p> 默认构建 PLATFORM_PLAYER；
     * @see #setPlayer(String)
     *
     * @param context 应用上下文环境
     * @param localzModel {@link ILocalzModel}
     * @param playerModel {@link IPlayerModel}
     * @return {@link IMediaPlayer}
     */
    public static IMediaPlayer buildPlayer(@NonNull Context context,
                                           @NonNull ILocalzModel localzModel,
                                           @NonNull IPlayerModel playerModel) {
        Class<? extends IMediaPlayer> mediaPlayer;
        switch (sPlayerType) {
            case VITAMIO_PLAYER:
                mediaPlayer = VitamioPlayer.class;
                break;
            case PLATFORM_PLAYER:
            default:
                mediaPlayer = HCorePlayer.class;
                break;
        }

        try {
            Constructor<? extends IMediaPlayer> constructor =
                    mediaPlayer.getConstructor(Context.class, ILocalzModel.class, IPlayerModel.class);
            return constructor.newInstance(context, localzModel, playerModel);
        } catch (InstantiationException
                | IllegalAccessException
                | InvocationTargetException
                | NoSuchMethodException e) {
            e.printStackTrace();
        }

        return null;
    }
}
