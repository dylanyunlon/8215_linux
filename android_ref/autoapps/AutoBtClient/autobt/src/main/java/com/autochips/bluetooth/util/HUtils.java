package com.autochips.bluetooth.util;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.SystemClock;
import android.text.TextUtils;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.LinearInterpolator;
import android.view.animation.RotateAnimation;
import android.widget.Toast;

import com.autochips.bluetooth.R;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;

public class HUtils {

    public static void startApp(Context context, String packageName, String classname) {
        if (null == context || TextUtils.isEmpty(packageName)) {
            return;
        }
        try {
            Intent intent = new Intent(Intent.ACTION_MAIN);
            intent.setComponent(new ComponentName(packageName, classname));
            intent.addCategory(Intent.CATEGORY_LAUNCHER);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static boolean isConnect(BluetoothDeviceInfo device) {
        if (device != null) {
            if (device.getDeviceStatus() == BluetoothDeviceInfo.DeviceStatus.DEVICE_STATUS_CONNECTED) {
                return true;
            }
        }
        return false;
    }

    public static void startLoadingAnim(){
        RotateAnimation ra = new RotateAnimation(0.0f, 359.0f,
                RotateAnimation.RELATIVE_TO_SELF, 0.5f,
                RotateAnimation.RELATIVE_TO_SELF, 0.5f);
        ra.setRepeatCount(-1);
        ra.setRepeatMode(Animation.RESTART);
        ra.setDuration(1000);
        LinearInterpolator li = new LinearInterpolator();
        ra.setInterpolator(li);
    }

    static boolean toastHasShow = false;
    static long mLastTime = 0;
    public static void showTips(Context context){
        long time = System.currentTimeMillis();
        if(time - mLastTime < 5000){
            return ;
        }
        mLastTime = time;
        Toast toast = new Toast(context);
        toast.setDuration(Toast.LENGTH_LONG);
        toast.setGravity(Gravity.CENTER,0,0);
        toast.setView(View.inflate(context, R.layout.dialog_bt_unconnect,null));
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            toast.addCallback(new Toast.Callback() {
                @Override
                public void onToastHidden() {
                    toastHasShow = false;
                }
            });
        }
        toast.show();

    }
}
