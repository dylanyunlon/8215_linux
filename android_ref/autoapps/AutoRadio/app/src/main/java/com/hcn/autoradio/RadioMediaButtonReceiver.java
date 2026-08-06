package com.hcn.autoradio;

import static com.hcn.autoradio.data.RadioData.BAND_SIZE;

import android.annotation.SuppressLint;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.radio.RadioPlayer;
import android.util.Log;
import android.view.KeyEvent;

import com.hcn.autoradio.data.FMDataControl;
import com.hcn.autoradio.data.RadioData;
import com.hcn.autoradio.util.RadioUtils;

public class RadioMediaButtonReceiver extends BroadcastReceiver {

    private static final String TAG = "Radio";
    private static boolean isCancelActionUp;

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "onReceive  action==" + intent.getAction());
        if (RadioPlayer.ACTION_TA_MESSAGE.equals(intent.getAction())) {
            int value = intent.getIntExtra(RadioPlayer.EXTRA_KEY_MESSAGE, 0);
            if (value == 0) {
                FMDataControl.getInstance().removeTAWindow();
            } else {
                FMDataControl.getInstance().showTAWindow();
            }
        } else if (Intent.ACTION_MEDIA_BUTTON.equals(intent.getAction())) {
            KeyEvent event = intent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);
            if (event == null) {
                return;
            }
            int action = event.getAction();
            int keycode = event.getKeyCode();
            int metaState = event.getMetaState();
            boolean isLongPress = event.isLongPress();
            Log.d(TAG, "onReceive: action==" + action + "  keycode==" + keycode + "  isLongPress=="
                    + isLongPress);
            if (action == KeyEvent.ACTION_UP && !isLongPress) {
                if (isCancelActionUp) {
                    isCancelActionUp = false;
                } else {
                    Message message = Message.obtain();
                    message.what = keycode;
                    message.obj = false;
                    message.arg1 = metaState;
                    handler.sendMessage(message);
                }
            } else if (action == KeyEvent.ACTION_DOWN && isLongPress) {
                isCancelActionUp = true;
                Message message = Message.obtain();
                message.what = keycode;
                message.obj = true;
                message.arg1 = metaState;
                handler.sendMessage(message);
            }
        }


    }

    @SuppressLint("HandlerLeak")
    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {

            FMDataControl fmDataControl = FMDataControl.getInstance();
            boolean isLongPress = (boolean) msg.obj;
            switch (msg.what) {

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

                case KeyEvent.KEYCODE_C://scan
                    if (isLongPress) {
                        fmDataControl.AS();
                    } else {
                        fmDataControl.scan();
                    }
                    break;
                case KeyEvent.KEYCODE_R:
                    if (msg.arg1 == KeyEvent.META_SHIFT_ON) {
                        fmDataControl.presetDown();
                    } else if (msg.arg1 == KeyEvent.META_ALT_ON) {//repeat
                        Log.d(TAG, "handleMessage: META_ALT_ON repeat");
                    }
                    break;
                case KeyEvent.KEYCODE_L:
                    fmDataControl.presetUp();
                    break;
                case KeyEvent.KEYCODE_F:
                    int fmFreq = msg.arg1;
                    Log.d(TAG, "[KEYCODE_F]->fmFreq = " + fmFreq);
                    if (fmFreq < FMDataControl.mRadioParameters.FmMin || fmFreq > FMDataControl.mRadioParameters.FmMax) {
                        Log.e(TAG, "[KEYCODE_F]-> FM freq is invalid");
                        return;
                    }
                    if (fmDataControl.currentBand() < RadioData.BAND_AM_1) {
                        fmDataControl.setFreq(fmFreq);
                    } else {
                        fmDataControl.setFreq(RadioData.BAND_FM_1, fmFreq, -1);
                    }
                    break;
                case KeyEvent.KEYCODE_G:
                    int amFreq = msg.arg1;
                    Log.d(TAG, "[KEYCODE_G]->amFreq = " + amFreq);
                    if (amFreq < FMDataControl.mRadioParameters.AmMin || amFreq > FMDataControl.mRadioParameters.AmMax) {
                        Log.e(TAG, "[KEYCODE_G]-> AM freq is invalid");
                        return;
                    }
                    if (!RadioUtils.getRadioModel().equalsIgnoreCase(RadioUtils.RADIO_INSIDE)) {
                        Log.e(TAG, "inside Radio not support AM");
                        return;
                    }
                    if (fmDataControl.currentBand() < RadioData.BAND_AM_1) {
                        fmDataControl.setFreq(RadioData.BAND_AM_1, amFreq, -1);
                    } else {
                        fmDataControl.setFreq(amFreq);
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
                    int nIndex = msg.what - KeyEvent.KEYCODE_1;
                    if (nIndex < fmDataControl.BAND_STATION_TOTAL) {
                        int[] presets = fmDataControl.readPresetList(fmDataControl.currentBand());
                        Log.e(TAG, "goto index:"+nIndex + " presets="+presets[nIndex]);
                        fmDataControl.setFreq(presets[nIndex], nIndex);
                    }
                    break;
                default:
                    break;
            }
        }
    };
}
