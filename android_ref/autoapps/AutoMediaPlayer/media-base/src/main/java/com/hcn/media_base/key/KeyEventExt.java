package com.hcn.media_base.key;

import android.view.KeyEvent;

/**
 * 多媒体 KeyEvent 扩展
 * <pre>
 *    根据车载系统需求扩展定义；
 *    这里主要是定义与多媒体场景关联的扩展 Key；
 *    注意：这里的按键值定义（不要和平台冲突）
 * </pre>
 *
 * @author 65821
 */
public class KeyEventExt extends KeyEvent {
    public KeyEventExt(int action, int code) {
        super(action, code);
    }

    /** 媒体播放模式-列表内循环播放 **/
    public static final int KEYCODE_MEDIA_REPEAT_ALL = 512;

    /** 媒体播放模式-列表内随机播放 **/
    public static final int KEYCODE_MEDIA_RANDOM_ALL = KEYCODE_MEDIA_REPEAT_ALL + 1;

    /** 媒体列表控制-逆时针旋转旋钮  **/
    public static final int KEYCODE_MEDIA_SMART_DECREMENT = KEYCODE_MEDIA_RANDOM_ALL + 1;

    /** 媒体列表控制-顺时针旋转旋钮 **/
    public static final int KEYCODE_MEDIA_SMART_INCREMENT = KEYCODE_MEDIA_SMART_DECREMENT + 1;

    /** 媒体列表控制-按下旋转旋钮 **/
    public static final int KEYCODE_MEDIA_SMART_ENTER = KEYCODE_MEDIA_SMART_INCREMENT + 1;
}
