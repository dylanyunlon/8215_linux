package com.hcn.media_base.key;

/**
 * 处理按键事件接口
 * @author 65821
 */
public interface IKeyEvent {

    /**
     * 处理按键事件
     * @param keycode {@link android.view.KeyEvent}
     */
    void onKeyEvent(int keycode);
}
