package com.hcn.autoeq.service;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import androidx.annotation.Nullable;

import com.hcn_library.data.NineDspAttenuateSettings;
import com.hcn_library.data.NineDspBalanceSettings;
import com.hcn_library.data.NineDspBandSettings;
import com.hcn_library.data.NineDspDbbSettings;
import com.hcn_library.data.NineDspDelaySettings;
import com.hcn_library.data.NineDspDtsBassBoostSettings;
import com.hcn_library.data.NineDspDtsDtsSettings;
import com.hcn_library.data.NineDspDtsFilterSettings;
import com.hcn_library.data.NineDspDtsSoundFocusSettings;
import com.hcn_library.data.NineDspDtsSurroundSettings;
import com.hcn_library.data.NineDspDtsVirtualCenterSettings;
import com.hcn_library.data.NineDspHLPFSettings;
import com.hcn_library.util.ConstantExtDsp;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.NineConstantExtDsp;

public class RemoteControlService extends Service implements ConstantExtDsp {

    private static final String TAG = RemoteControlService.class.getSimpleName();
    private final static String NOTIFICATION_ID = String.valueOf(android.os.Process.myPid());
    private Context mContext;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate");
        mContext = this;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand");
        startForeground(android.os.Process.myPid(), getNotification());
        if ("gb04".equals(EqUtils.getSkinName())) {
            if (EqUtils.isChip7739()) {
                NineDspDtsVirtualCenterSettings.getInstance(mContext).nativeAll();
            }
            if (!EqUtils.isChip7739()) {
                NineDspDtsFilterSettings.getInstance(mContext).nativeAll();
            }
            NineDspDtsDtsSettings nineDspDtsDtsSettings = NineDspDtsDtsSettings.getInstance(mContext);
            if (EqUtils.isChip7739()) {
                nineDspDtsDtsSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_PROCESS_MODEL,
                        NineDspDtsFilterSettings.getInstance(mContext).getDtsSwitchEnable() ? nineDspDtsDtsSettings.getDtsModel() : 0);
            } else {
                nineDspDtsDtsSettings.nativeDTS(NineConstantExtDsp.NINE_DTS_PROCESS_MODEL, nineDspDtsDtsSettings.getDtsModel());
            }
            NineDspDtsSoundFocusSettings.getInstance(mContext).nativeAll();
            NineDspDtsSurroundSettings.getInstance(mContext).nativeAll();
            NineDspDtsBassBoostSettings.getInstance(mContext).nativeAll();
        }
        NineDspBandSettings.getInstance(mContext).nativeAll();
        NineDspBalanceSettings nineDspBalanceSettings = NineDspBalanceSettings.getInstance(mContext);
        int[] nineBalance = nineDspBalanceSettings.getBalance();
        nineDspBalanceSettings.nativeBalance(nineBalance[0], nineBalance[1]);
        NineDspAttenuateSettings.getInstance(mContext).nativeAll("LF", "RF", "LR", "RR", "CENTER", "SUBWOOFER");
        NineDspDelaySettings.getInstance(mContext).nativeAll("LF", "RF", "LR", "RR", "CENTER", "SUBWOOFER");
        NineDspHLPFSettings.getInstance(mContext).nativeAll(NineConstantExtDsp.INDEX_LPF_HPF_F, NineConstantExtDsp.INDEX_LPF_HPF_R, NineConstantExtDsp.INDEX_LPF_HPF_CEN, NineConstantExtDsp.INDEX_LPF_HPF_SUB);
        NineDspDbbSettings.getInstance(mContext).nativeAll(NineConstantExtDsp.NINE_DBB_CHANNEL_FLFR, NineConstantExtDsp.NINE_DBB_CHANNEL_RLRR, NineConstantExtDsp.NINE_DBB_CHANNEL_CEN, NineConstantExtDsp.NINE_DBB_CHANNEL_SUBWOOFER);
        Log.i(TAG, "gb04 or gb05 ");
        stopSelf();
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "onDestroy");
        stopForeground(true);
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private Notification getNotification() {
        //适配8.0service
        NotificationManager notificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        NotificationChannel mChannel = null;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            mChannel = new NotificationChannel(NOTIFICATION_ID, "autoeq", NotificationManager.IMPORTANCE_HIGH);
            notificationManager.createNotificationChannel(mChannel);
            Notification notification = new Notification.Builder(getApplicationContext(), NOTIFICATION_ID).build();
            return notification;
        }
        return null;
    }

}