package com.hcn.media_dummy.player;

import com.hcn.media_dummy.base.player.IPlayerManager;

/**
 * 播放组件工厂类
 * <p> 默认使用 ijk 播放；
 *
 * @author 65821
 */
public class PlayerFactory {
    private static Class<? extends IPlayerManager> sPlayerManager;

    public static void setPlayManager(Class<? extends IPlayerManager> playManager) {
        sPlayerManager = playManager;
    }

    public static IPlayerManager getPlayManager() {
        if (sPlayerManager == null) {
            sPlayerManager = IjkPlayerManager.class;
        }

        try {
            return sPlayerManager.newInstance();
        } catch (InstantiationException | IllegalAccessException e) {
            e.printStackTrace();
        }

        return null;
    }
}
