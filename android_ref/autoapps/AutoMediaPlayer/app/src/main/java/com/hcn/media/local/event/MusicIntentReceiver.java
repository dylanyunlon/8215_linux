package com.hcn.media.local.event;

import android.carsource.McuConstant;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.UserHandle;
import android.sourceservice.SourceInfo;
import android.view.KeyEvent;

import com.hcn.media_base.IMediaBroadcast;
import com.hcn.media_base.constant.IConstant;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_data.base.BaseMediaData;

import java.util.Objects;


/**
 * MediaButton
 * <p> [KeyEvent][Voice][Key]
 *
 * @author 65821
 */
public class MusicIntentReceiver extends BroadcastReceiver {
    public final static String MEDIA_PACKAGE = "com.hcn.AutoMediaPlayer";

    /** 当前源信息 **/
    private SourceInfo mSourceInfo = null;

    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub
        LogUtil.d("MusicIntentReceiver", "action: " + intent.getAction());

        // 当前源必须是在本地多媒体下
        mSourceInfo = SourceInfo.getInstance();
        String sourcePackage = mSourceInfo.getSourcePackage();
        if (!MEDIA_PACKAGE.equals(sourcePackage)) {
            return;
        }

        // 处理系统多媒体按键事件广播
        if (intent.getAction().equals(Intent.ACTION_MEDIA_BUTTON)) {
            onActionMediaButton(context, intent);
        }
    }

    /**
     * 处理广播媒体按键意图
     * <p> Intent.ACTION_MEDIA_BUTTON
     *
     * @param context 上下文
     * @param intent 广播意图
     */
    private void onActionMediaButton(Context context, Intent intent) {
        // 检查按键事件对象
        KeyEvent event = intent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);
        if (Objects.isNull(event)) {
            return;
        }

        // 检查按键事件状态
        int action = event.getAction();
        if (KeyEvent.ACTION_UP != action) {
            return;
        }

        // 获取当前按键类型
        int keycode = event.getKeyCode();
        Intent intSend = new Intent();
        switch (keycode) {
            case KeyEvent.KEYCODE_MEDIA_PLAY:
                intSend.setAction(IMediaBroadcast.ACTION_VOICE_EVENT_PLAY);
                break;
            case KeyEvent.KEYCODE_MEDIA_STOP:
            case KeyEvent.KEYCODE_MEDIA_PAUSE:
                intSend.setAction(IMediaBroadcast.ACTION_VOICE_EVENT_PAUSE);
                break;
            case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
                intSend.setAction(IConstant.ACTION_NOTIFICATION_PLAYPAUSE);
                break;
            case KeyEvent.KEYCODE_MEDIA_NEXT:
                intSend.setAction(IMediaBroadcast.ACTION_VOICE_EVENT_NEXT);
                break;
            case KeyEvent.KEYCODE_MEDIA_PREVIOUS:
                intSend.setAction(IMediaBroadcast.ACTION_VOICE_EVENT_PREV);
                break;
            case KeyEvent.KEYCODE_R: {
                LogUtil.e("MusicIntentReceiver",
                        "keycode:" + McuConstant.K_REPEAT);
                LogUtil.e("MusicIntentReceiver",
                        "event.getMetaState():" + (event.getMetaState() == KeyEvent.META_ALT_ON));

                if (event.getMetaState() == KeyEvent.META_ALT_ON) {
                    intSend.setAction(IMediaBroadcast.ACTION_VOICE_EVENT_MODE_LOOP_ALL);
                } else if (event.getMetaState() == KeyEvent.META_SHIFT_ON) {
                    intSend.setAction(IMediaBroadcast.ACTION_EVENT_K_SCROLL_R);
                }
                break;
            }
            case KeyEvent.KEYCODE_S: {
                LogUtil.e("MusicIntentReceiver", "keycode:" + McuConstant.K_REPEAT);

                if (event.getMetaState() == KeyEvent.META_ALT_ON) {
                    intSend.setAction(IMediaBroadcast.ACTION_VOICE_EVENT_MODE_RANDOM);
                }
                break;
            }
            case KeyEvent.KEYCODE_L:
            case McuConstant.K_SCROLL_L:
                intSend.setAction(IMediaBroadcast.ACTION_EVENT_K_SCROLL_L);
                break;
            case McuConstant.K_SCROLL_R:
                intSend.setAction(IMediaBroadcast.ACTION_EVENT_K_SCROLL_R);
                break;
            case McuConstant.K_ENTER:
                intSend.setAction(IMediaBroadcast.ACTION_EVENT_K_ENTER);
                break;
            case KeyEvent.KEYCODE_MEDIA_REWIND:
                intSend.setAction(IMediaBroadcast.ACTION_VOICE_EVENT_REWIND);
                break;
            case KeyEvent.KEYCODE_MEDIA_FAST_FORWARD:
                intSend.setAction(IMediaBroadcast.ACTION_VOICE_EVENT_FAST_FORWARD);
                break;
            default:
                break;
        }

        context.sendBroadcastAsUser(intSend, UserHandle.getUserHandleForUid(BaseMediaData.UID));
    }
}
