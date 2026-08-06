package com.hcn.media.local.event;

import android.carsource.McuConstant;
import android.content.Context;
import android.content.Intent;
import android.support.v4.media.session.MediaSessionCompat;
import android.view.KeyEvent;

import androidx.annotation.NonNull;

import com.hcn.common.misc.LogUtils;
import com.hcn.media_base.key.IKeyEvent;
import com.hcn.media_base.key.KeyEventExt;
import com.hcn.media_common.debug.MediaDebug;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;

/**
 * Media Session 回调
 * 处理媒体回话事件（媒体按键事件、媒体状态等）
 * @author 65821
 */
public class MediaSessionCallback extends MediaSessionCompat.Callback {
    /**
     * 上下文环境引用
     * <p> 弱应用，避免未知场景内存泄露；
     */
    private Reference<Context> mContextRef;

    /**
     * 按键事件接口
     * <p> 用来统一处理按键相关的事件（KeyEvent）；
     */
    private final IKeyEvent mKeyEvent;

    public MediaSessionCallback(@NonNull Context context,
                                @NonNull IKeyEvent keyEvent) {
        super();

        mContextRef = new WeakReference<>(context);
        mKeyEvent = keyEvent;
    }

    @Override
    public void onPlay() {
        super.onPlay();
        LogUtils.vTag(MediaDebug.TAG, "onPlay.");

        mKeyEvent.onKeyEvent(KeyEvent.KEYCODE_MEDIA_PLAY);
    }

    @Override
    public void onPause() {
        super.onPause();
        LogUtils.vTag(MediaDebug.TAG, "onPause.");

        mKeyEvent.onKeyEvent(KeyEvent.KEYCODE_MEDIA_PAUSE);
    }

    @Override
    public void onSkipToNext() {
        super.onSkipToNext();
        LogUtils.vTag(MediaDebug.TAG, "onSkipToNext.");

        mKeyEvent.onKeyEvent(KeyEvent.KEYCODE_MEDIA_NEXT);
    }

    @Override
    public void onSkipToPrevious() {
        super.onSkipToPrevious();
        LogUtils.vTag(MediaDebug.TAG, "onSkipToPrevious.");

        mKeyEvent.onKeyEvent(KeyEvent.KEYCODE_MEDIA_PREVIOUS);
    }

    @Override
    public void onFastForward() {
        super.onFastForward();
        LogUtils.vTag(MediaDebug.TAG, "onFastForward.");

        mKeyEvent.onKeyEvent(KeyEvent.KEYCODE_MEDIA_FAST_FORWARD);
    }

    @Override
    public void onRewind() {
        super.onRewind();
        LogUtils.vTag(MediaDebug.TAG, "onRewind.");

        mKeyEvent.onKeyEvent(KeyEvent.KEYCODE_MEDIA_REWIND);
    }

    @Override
    public void onStop() {
        super.onStop();
        LogUtils.vTag(MediaDebug.TAG, "onStop.");

        mKeyEvent.onKeyEvent(KeyEvent.KEYCODE_MEDIA_STOP);
    }

    @Override
    public void onSeekTo(long pos) {
        super.onSeekTo(pos);
    }

    /**
     * 处理 Media Button 事件
     * <pre>
     *    例如，针对 KEYCODE_MEDIA_PLAY 事件：
     *         如果不在此处理，则返回 false，事件会继续向下传递到 this#onPlay() 接口；
     *         如果提前拦截处理，则返回 true，终止继续向下传输（不响应 onPlay() 接口）；
     * </pre>
     *
     * @param mediaButtonEvent
     * @return 返回 true 表示已经处理，否则继续传输给 Callback 的具体函数；
     */
    @Override
    public boolean onMediaButtonEvent(Intent mediaButtonEvent) {
        boolean result = super.onMediaButtonEvent(mediaButtonEvent);
        if (!result) {
            // 获取外部触发的按键事件
            KeyEvent keyEvent = mediaButtonEvent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);
            if (keyEvent == null || keyEvent.getAction() != KeyEvent.ACTION_DOWN) {
                return false;
            }

            int keyCode = keyEvent.getKeyCode();
            int metaState = keyEvent.getMetaState();
            LogUtils.vTag(MediaDebug.TAG, "onMediaButtonEvent: " + keyCode);
            switch (keyCode) {
                case KeyEvent.KEYCODE_R:
                    if (metaState == KeyEvent.META_ALT_ON) {
                        mKeyEvent.onKeyEvent(KeyEventExt.KEYCODE_MEDIA_REPEAT_ALL);
                    } else if (metaState == KeyEvent.META_SHIFT_ON) {
                        mKeyEvent.onKeyEvent(KeyEventExt.KEYCODE_MEDIA_SMART_INCREMENT);
                    }
                    return true;
                case KeyEvent.KEYCODE_S:
                    if (metaState == KeyEvent.META_ALT_ON) {
                        mKeyEvent.onKeyEvent(KeyEventExt.KEYCODE_MEDIA_RANDOM_ALL);
                    }
                    return true;
                case KeyEvent.KEYCODE_L:
                case McuConstant.K_SCROLL_L:
                    mKeyEvent.onKeyEvent(KeyEventExt.KEYCODE_MEDIA_SMART_DECREMENT);
                    return true;
                case McuConstant.K_SCROLL_R:
                    mKeyEvent.onKeyEvent(KeyEventExt.KEYCODE_MEDIA_SMART_INCREMENT);
                    return true;
                case McuConstant.K_ENTER:
                    mKeyEvent.onKeyEvent(KeyEventExt.KEYCODE_MEDIA_SMART_ENTER);
                    return true;
                case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
                    mKeyEvent.onKeyEvent(KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE);
                    return true;
                default:
                    break;
            }
        }

        return result;
    }
}
