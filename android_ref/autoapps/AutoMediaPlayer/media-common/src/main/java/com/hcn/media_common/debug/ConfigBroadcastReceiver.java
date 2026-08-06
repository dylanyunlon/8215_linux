package com.hcn.media_common.debug;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.text.TextUtils;

/**
 * 配置更新广播
 * @author 86158
 */
public class ConfigBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = "ConfigBroadcastReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (null == intent) {
            return; // ignore
        }

        String action = intent.getAction();
        if (TextUtils.isEmpty(action)) {
            return; // ignore
        }

        String extraInfo = intent.getStringExtra(IBroadcast.HMEDIA_CONFIG_EXTRA_KEY);
        if (TextUtils.isEmpty(extraInfo)) {
            return; // ignore
        }

        if (IBroadcast.HMEDIA_CONFIG_ACTION.equals(action)) {
            if (IBroadcast.HMEDIA_CONFIG_EXTRA_INFO_MEM.equals(extraInfo)) {
                MediaConfigEx.mem_monitor_config();
            }
        }
    }
}
