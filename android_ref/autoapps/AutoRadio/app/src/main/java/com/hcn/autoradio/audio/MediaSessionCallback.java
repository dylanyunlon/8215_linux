package com.hcn.autoradio.audio;

import static com.hcn.autoradio.data.RadioData.BAND_SIZE;

import android.content.Context;
import android.content.Intent;
import android.support.v4.media.session.MediaSessionCompat;
import android.util.Log;
import android.view.KeyEvent;

import androidx.annotation.NonNull;

import com.hcn.autoradio.data.FMDataControl;
import com.hcn.autoradio.data.RadioData;
import com.hcn.autoradio.util.RadioUtils;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;

/**
 * Media Session 回调
 * 处理媒体回话事件（媒体按键事件、媒体状态等）
 *
 * @author simon
 */
public class MediaSessionCallback extends MediaSessionCompat.Callback {

    private static final String TAG = "Radio_MediaSessionCallback";
    /**
     * 上下文环境引用
     * <p> 弱应用，避免未知场景内存泄露；
     */
    private Reference<Context> mContextRef;


    public MediaSessionCallback(@NonNull Context context) {
        super();
        mContextRef = new WeakReference<>(context);
    }

    /**
     * 处理 Media Button 事件
     * <pre>
     *    例如，针对 KEYCODE_MEDIA_PLAY 事件：
     *         如果不在此处理，则返回 false，事件会继续向下传递；
     *         如果提前拦截处理，则返回 true，终止继续向下传输；
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
            boolean isLongPress = keyEvent.isLongPress();
            FMDataControl fmDataControl = FMDataControl.getInstance();
            Log.d(TAG, "onMediaButtonEvent: keyCode=" + keyCode + "  isLongPress=" + isLongPress);
            switch (keyCode) {
                case KeyEvent.KEYCODE_A:
                    if (isLongPress) {
                        fmDataControl.AS();
                    } else {
                        fmDataControl.PS();
                    }
                    break;
                case KeyEvent.KEYCODE_B:
                    if (!isLongPress) {
                        fmDataControl.Band((fmDataControl.currentBand() + 1) % BAND_SIZE);
                    }
                    break;
                case KeyEvent.KEYCODE_MEDIA_NEXT:
                    if (isLongPress) {
                        fmDataControl.seekDown();
                    } else {
                        fmDataControl.presetDown();
                    }
                    break;
                case KeyEvent.KEYCODE_MEDIA_PREVIOUS:
                    if (isLongPress) {
                        fmDataControl.seekUp();
                    } else {
                        fmDataControl.presetUp();
                    }
                    break;
                case KeyEvent.KEYCODE_MEDIA_STEP_FORWARD:
                    if (isLongPress) {
                        fmDataControl.seekDown();
                    } else {
                        fmDataControl.stepDown();
                    }
                    break;
                case KeyEvent.KEYCODE_MEDIA_STEP_BACKWARD:
                    if (isLongPress) {
                        fmDataControl.seekUp();
                    } else {
                        fmDataControl.stepUp();
                    }
                    break;
                case KeyEvent.KEYCODE_C:
                    if (isLongPress) {
                        fmDataControl.AS();
                    } else {
                        fmDataControl.scan();
                    }
                    break;
                case KeyEvent.KEYCODE_R:
                    if (metaState == KeyEvent.META_SHIFT_ON) {
                        fmDataControl.presetDown();
                    } else if (metaState == KeyEvent.META_ALT_ON) {
                        Log.d(TAG, "handleMessage: META_ALT_ON repeat");
                    }
                    break;
                case KeyEvent.KEYCODE_L:
                    fmDataControl.presetUp();
                    break;
                case KeyEvent.KEYCODE_F:
                    Log.d(TAG, "[KEYCODE_F]->fmFreq = " + metaState);
                    if (metaState < FMDataControl.mRadioParameters.FmMin || metaState > FMDataControl.mRadioParameters.FmMax) {
                        Log.e(TAG, "[KEYCODE_F]-> FM freq is invalid");
                        return false;
                    }
                    if (fmDataControl.currentBand() < RadioData.BAND_AM_1) {
                        fmDataControl.setFreq(metaState);
                    } else {
                        fmDataControl.setFreq(RadioData.BAND_FM_1, metaState, -1);
                    }
                    break;
                case KeyEvent.KEYCODE_G:
                    Log.d(TAG, "[KEYCODE_G]->amFreq = " + metaState);
                    if (metaState < FMDataControl.mRadioParameters.AmMin || metaState > FMDataControl.mRadioParameters.AmMax) {
                        Log.e(TAG, "[KEYCODE_G]-> AM freq is invalid");
                        return false;
                    }
                    if (!RadioUtils.getRadioModel().equalsIgnoreCase(RadioUtils.RADIO_INSIDE)) {
                        Log.e(TAG, "inside Radio not support AM");
                        return false;
                    }
                    if (fmDataControl.currentBand() < RadioData.BAND_AM_1) {
                        fmDataControl.setFreq(RadioData.BAND_AM_1, metaState, -1);
                    } else {
                        fmDataControl.setFreq(metaState);
                    }
                    break;
                case KeyEvent.KEYCODE_1:
                case KeyEvent.KEYCODE_2:
                case KeyEvent.KEYCODE_3:
                case KeyEvent.KEYCODE_4:
                case KeyEvent.KEYCODE_5:
                case KeyEvent.KEYCODE_6:
                case KeyEvent.KEYCODE_7:
                case KeyEvent.KEYCODE_8:
                case KeyEvent.KEYCODE_9:
                    int nIndex = keyCode - KeyEvent.KEYCODE_1;
                    if (nIndex < fmDataControl.BAND_STATION_TOTAL) {
                        int[] presets = fmDataControl.readPresetList(fmDataControl.currentBand());
                        Log.e(TAG, "goto index:" + nIndex + " presets=" + presets[nIndex]);
                        fmDataControl.setFreq(presets[nIndex], nIndex);
                    }
                    break;
                default:
                    break;
            }
        }
        return result;
    }
}
