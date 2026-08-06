package com.hcn.bluetooth.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class BootReceiver extends BroadcastReceiver {
    private static final String TAG = "BootReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub
        String action = intent.getAction();
        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            Log.d(TAG, "ACTION_BOOT_COMPLETED");
        } else if (action.equals(Intent.ACTION_MEDIA_BUTTON)) {
            BluetoothMusicService.getInstance().onMediaKeyEvent(intent);
        }
    }

}
