package com.hcn.autoeq.service;


import static com.hcn.autoeq.util.ConstantDsp.DEF_DSP_BANDS;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

import androidx.annotation.Nullable;

import com.hcn.autoeq.data.AspSettings;
import com.hcn.autoeq.data.CscAspBalanceSettings;
import com.hcn.autoeq.data.CscAspEqualizerChartSettings;
import com.hcn.autoeq.data.CscAspSubwooferSettings;
import com.hcn.autoeq.data.DspInternalSettings;
import com.hcn.autoeq.data.ExtDspAttenuateSettings;
import com.hcn.autoeq.data.ExtDspBalanceSettings;
import com.hcn.autoeq.data.ExtDspBandSettings;
import com.hcn.autoeq.data.ExtDspDbbSettings;
import com.hcn.autoeq.data.ExtDspDelaySettings;
import com.hcn.autoeq.data.ExtDspHLPFSettings;
import com.hcn.autoeq.data.ExtDspSurroundSettings;
import com.hcn.autoeq.data.FyDspHelper;
import com.hcn.autoeq.data.SIExtDspAttenuateSettings;
import com.hcn.autoeq.data.SIExtDspBalanceSettings;
import com.hcn.autoeq.data.SIExtDspBandSettings;
import com.hcn.autoeq.data.SIExtDspDelaySettings;
import com.hcn.autoeq.data.SIExtDspHLPFSettings;
import com.hcn.autoeq.util.ConstantExtDsp;
import com.hcn.autoeq.util.ECDConstantExtDsp;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.SIConstantExtDsp;

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
        if (EqUtils.DSP_CHIP_FY7604.equals(EqUtils.getEqChipType())) {
            FyDspHelper.nativeAllData(mContext);
        } else if (EqUtils.DSP_CHIP_7604.equals(EqUtils.getEqChipType())) {
            ExtDspBandSettings extDspBandSettings = ExtDspBandSettings.getInstance(mContext);
            extDspBandSettings.nativeReverbType();
            extDspBandSettings.nativeUserReverbType();

            ExtDspBalanceSettings extDspBalanceSettings = ExtDspBalanceSettings.getInstance(mContext);
            int[] balance = extDspBalanceSettings.getBalance();
            extDspBalanceSettings.nativeBalance(balance[0], balance[1]);

            ExtDspAttenuateSettings extDspAttenuateSettings = ExtDspAttenuateSettings.getInstance(mContext);
            extDspAttenuateSettings.nativeAll("LF", "RF", "LR", "RR", "SUBWOOFER");

            ExtDspHLPFSettings extDspHLPFSettings = ExtDspHLPFSettings.getInstance(mContext);
            extDspHLPFSettings.nativeAll(CHANNEL_FRONT_HIGH, CHANNEL_FRONT_LOW, CHANNEL_REAR_HIGH, CHANNEL_REAR_LOW, CHANNEL_SUBWOOFER_HIGH, CHANNEL_SUBWOOFER_LOW);

            ExtDspDbbSettings extDspDbbSettings = ExtDspDbbSettings.getInstance(mContext);
            extDspDbbSettings.nativeAll(DBB_CHANNEL_FLFR, DBB_CHANNEL_RLRR, DBB_CHANNEL_SUBWOOFER);

            ExtDspSurroundSettings extDspSurroundSettings = ExtDspSurroundSettings.getInstance(mContext);
            extDspSurroundSettings.nativeSurround(extDspSurroundSettings.getSurround());
            extDspSurroundSettings.nativeLoudness(extDspSurroundSettings.getLoudness());

            ExtDspDelaySettings extDspDelaySettings = ExtDspDelaySettings.getInstance(mContext);

            extDspDelaySettings.nativeAll();
            stopSelf();
        } else if (EqUtils.isGB04_05()) {
            // 目标 APK 的包名
            String targetPackageName = "com.hcn.autoeq.skin.gb04";
            // 目标服务的类名
            String targetServiceClassName = "com.example.gb04.service.SkinStartService";
            // 创建显式意图
            Intent skinIntent = new Intent();
            skinIntent.setClassName(targetPackageName, targetServiceClassName);

            try {
                // 启动服务
                startService(skinIntent);
            } catch (Exception e) {
                e.printStackTrace();
                // 处理启动服务失败的情况
            }
            stopSelf();
        } else if (EqUtils.DSP_CHIP_SI47925.equals(EqUtils.getEqChipType())) {
            SIExtDspBandSettings extDspBandSettings = SIExtDspBandSettings.getInstance(mContext);
            extDspBandSettings.nativeReverbType();
            extDspBandSettings.nativeUserReverbType();

            SIExtDspBalanceSettings extDspBalanceSettings = SIExtDspBalanceSettings.getInstance(mContext);
            int[] balance = extDspBalanceSettings.getBalance();
            extDspBalanceSettings.nativeBalance(balance[0], balance[1]);

            SIExtDspAttenuateSettings extDspAttenuateSettings = SIExtDspAttenuateSettings.getInstance(mContext);
            extDspAttenuateSettings.nativeAll("LF", "RF", "LR", "RR", "CENTER", "SUBWOOFER");

            SIExtDspHLPFSettings extDspHLPFSettings = SIExtDspHLPFSettings.getInstance(mContext);
            extDspHLPFSettings.nativeAll(SIConstantExtDsp.INDEX_LPF_HPF_F, SIConstantExtDsp.INDEX_LPF_HPF_R, SIConstantExtDsp.INDEX_LPF_HPF_CEN, SIConstantExtDsp.INDEX_LPF_HPF_SUB);

            ExtDspDbbSettings extDspDbbSettings = ExtDspDbbSettings.getInstance(mContext);
            extDspDbbSettings.nativeAll(DBB_CHANNEL_FLFR, DBB_CHANNEL_RLRR, DBB_CHANNEL_SUBWOOFER);

            ExtDspSurroundSettings extDspSurroundSettings = ExtDspSurroundSettings.getInstance(mContext);
            extDspSurroundSettings.nativeSurround(extDspSurroundSettings.getSurround());
            extDspSurroundSettings.nativeLoudness(extDspSurroundSettings.getLoudness());

            SIExtDspDelaySettings siExtDspDelaySettings = SIExtDspDelaySettings.getInstance(mContext);
            siExtDspDelaySettings.nativeAll(SIExtDspDelaySettings.CHANEL_FL, SIExtDspDelaySettings.CHANEL_FR,
                    SIExtDspDelaySettings.CHANEL_RL, SIExtDspDelaySettings.CHANEL_RR, SIExtDspDelaySettings.CHANEL_SUB, SIExtDspDelaySettings.CHANEL_CEN);
            Log.i(TAG, "DSP_CHIP_SI479x ");
            stopSelf();
        } else if (EqUtils.ASP_CHIP_CSC37534.equals(EqUtils.getEqChipType())) {
            CscAspEqualizerChartSettings cscAspEqualizerChartSettings = CscAspEqualizerChartSettings.getInstance(mContext);
            int loudness = cscAspEqualizerChartSettings.getCscAspLoudness();
            cscAspEqualizerChartSettings.nativeAllBand();
            cscAspEqualizerChartSettings.nativeLoudness(loudness);

            CscAspSubwooferSettings cscAspSubwooferSettings = CscAspSubwooferSettings.getInstance(mContext);
            int cscAspFreqValue = cscAspSubwooferSettings.getSurroundFre();
            cscAspSubwooferSettings.nativeSurround(cscAspFreqValue);

            CscAspBalanceSettings cscAspBalanceSettings = CscAspBalanceSettings.getInstance(mContext);
            int[] balance = cscAspBalanceSettings.getCscAspBalance();
            cscAspBalanceSettings.setCscAspBalance(balance[0], balance[1], true);

            Log.i(TAG, "csc37534 ");
            stopSelf();
        } else if (EqUtils.DSP_CHIP_7604_C.equals(EqUtils.getEqChipType())) {
            sendMessageWhenStart();
        } else {
            AspSettings.getInstance(mContext).startBootAspSetting();
            Log.i(TAG, "DSP Power = " + DspInternalSettings.getInstance(mContext).getDspPower());
            //MT8163 服务启动设置生效异常，需做延时处理.
            if (EqUtils.isRk3326()) {
                DspInternalSettings.getInstance(mContext).setDspPower(0);
                DspInternalSettings.getInstance(mContext).setupEqualizer(DEF_DSP_BANDS[0], true);
            } else {
                new Handler().postDelayed(() -> {
                    DspInternalSettings.getInstance(mContext).startBootDspSetting();
                    stopSelf();
                }, 1000);
            }
        }
        return super.onStartCommand(intent, flags, startId);
    }

    public  void sendMessageWhenStart() {
        try {
            ExtDspBandSettings extDspBandSettings = ExtDspBandSettings.getInstance(mContext);
            extDspBandSettings.nativeReverbType();
            extDspBandSettings.nativeUserReverbType();

            ExtDspBalanceSettings extDspBalanceSettings = ExtDspBalanceSettings.getInstance(mContext);
            int[] balance = extDspBalanceSettings.getBalance();
            extDspBalanceSettings.nativeBalanceDouble(balance[0], balance[1]);

            ExtDspAttenuateSettings extDspAttenuateSettings = ExtDspAttenuateSettings.getInstance(mContext);
            extDspAttenuateSettings.nativeAll("LF", "RF", "LR", "RR", "SUBWOOFER", "CENTER");

            ExtDspHLPFSettings extDspHLPFSettings = ExtDspHLPFSettings.getInstance(mContext);
            extDspHLPFSettings.nativeAll7604C(CHANNEL_FRONT_HIGH, CHANNEL_FRONT_LOW, CHANNEL_REAR_HIGH,
                    CHANNEL_REAR_LOW, CHANNEL_SUBWOOFER_HIGH, CHANNEL_SUBWOOFER_LOW, ECDConstantExtDsp.CHANNEL_CENTER_HIGH, ECDConstantExtDsp.CHANNEL_CENTER_LOW);
            int chanel = extDspHLPFSettings.getHLPFChannel();
            if (chanel == CHANNEL_FRONT_HIGH || chanel == CHANNEL_FRONT_LOW) {
                nativeHLPF(CHANNEL_FRONT_HIGH, extDspHLPFSettings);
                nativeHLPF(CHANNEL_FRONT_LOW, extDspHLPFSettings);
            } else if (chanel == CHANNEL_REAR_HIGH || chanel == CHANNEL_REAR_LOW) {
                nativeHLPF(CHANNEL_REAR_HIGH, extDspHLPFSettings);
                nativeHLPF(CHANNEL_REAR_LOW, extDspHLPFSettings);
            } else if (chanel == CHANNEL_SUBWOOFER_HIGH || chanel == CHANNEL_SUBWOOFER_LOW) {
                nativeHLPF(CHANNEL_SUBWOOFER_HIGH, extDspHLPFSettings);
                nativeHLPF(CHANNEL_SUBWOOFER_LOW, extDspHLPFSettings);
            } else if (chanel == ECDConstantExtDsp.CHANNEL_CENTER_HIGH || chanel == ECDConstantExtDsp.CHANNEL_CENTER_LOW) {
                nativeHLPF(ECDConstantExtDsp.CHANNEL_CENTER_HIGH, extDspHLPFSettings);
                nativeHLPF(ECDConstantExtDsp.CHANNEL_CENTER_LOW, extDspHLPFSettings);
            }

            ExtDspDbbSettings extDspDbbSettings = ExtDspDbbSettings.getInstance(mContext);
            if (EqUtils.isYuFeng()) {
                extDspDbbSettings.nativeAll(DBB_CHANNEL_FLFR, DBB_CHANNEL_RLRR, DBB_CHANNEL_SUBWOOFER, ECDConstantExtDsp.DBB_CHANNEL_SUBWOOFER2);
            } else {
                extDspDbbSettings.nativeAll(DBB_CHANNEL_FLFR, DBB_CHANNEL_RLRR, DBB_CHANNEL_SUBWOOFER);
            }

            ExtDspSurroundSettings extDspSurroundSettings = ExtDspSurroundSettings.getInstance(mContext);
            extDspSurroundSettings.nativeSurround(extDspSurroundSettings.getSurround());
            extDspSurroundSettings.nativeLoudness(extDspSurroundSettings.getLoudness());

            ExtDspDelaySettings extDspDelaySettings = ExtDspDelaySettings.getInstance(mContext);

            extDspDelaySettings.nativeAll8581();
            stopSelf();
        } catch (Exception e) {
            Log.i(TAG, "发送异常");
            e.printStackTrace();
        }

    }

    private void nativeHLPF(int chanel, ExtDspHLPFSettings extDspHLPFSettings) {
        int freq;
        int qValue;
        int slope;
        freq = extDspHLPFSettings.getFreq(chanel);
        qValue = extDspHLPFSettings.getQValue(chanel);
        slope = extDspHLPFSettings.getQValueSelect(qValue);
        Log.d(TAG, "sendMessageWhenStart: chanel = " + chanel + " freq = " + freq + " qValue = " + qValue + " slope = " + slope);
        extDspHLPFSettings.nativeHLPF(chanel, freq, slope);
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
