package com.hcn.autoradio.service;

import static com.hcn.autoradio.data.RadioData.BAND_FM_1;
import static com.hcn.autoradio.data.RadioData.BAND_SIZE;
import static com.hcn.autoradio.data.RadioData.PRESET_PLAY;
import static com.hcn.autoradio.data.RadioData.SEEK_ALL;
import static com.hcn.autoradio.data.RadioData.SEEK_PLAY;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothHeadsetClient;
import android.bluetooth.BluetoothProfile;
import android.carstatus.CarStatus;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.radio.RadioPlayer;
import android.text.TextUtils;
import android.util.Log;
import android.widget.RemoteViews;

import com.hcn.autoradio.IRadioCallBack;
import com.hcn.autoradio.IRadioServiceAPI;
import com.hcn.autoradio.R;
import com.hcn.autoradio.RadioMain;
import com.hcn.autoradio.api.IEvent;
import com.hcn.autoradio.audio.RadioAudioManager;
import com.hcn.autoradio.data.FMDataControl;
import com.hcn.autoradio.data.FMDataControl.UpdateDataListener;
import com.hcn.autoradio.data.RadioData;
import com.hcn.autoradio.util.RadioUtils;

import java.lang.ref.WeakReference;

// startService() + bindService()
// onCreate - onStart() - onBind() - onUnbind(- onRebind -) - onDestroy()
public class FMPlugService extends Service {
    private final String TAG = "FMPlugService";
    private FMBinder mFMBinder = null;
    private RadioBroadcastReceiver mRadioReceiver = null;

    private IBinder mRadioClientBinder = null;
    private RadioDeathRecipient mRadioDeathRecipient = null;
    private RemoteCallbackList<IRadioCallBack> mCallBackList;
    /**
     * Data Control Center
     */
    private FMDataControl mFMDCC = null;
    private OnUpdateDataListener mDataListener = null;
    private FMUpdateHandler mFMUpdateHandler = null;

    /**
     * 车载相关状态
     */
    private final CarStatus mCarStatus = new CarStatus();

    public static final String RADIO_ACTION_NOTIFICATION_PREV =
            "com.hcn.autoradio.notification.PREV";
    public static final String RADIO_ACTION_NOTIFICATION_NEXT =
            "com.hcn.autoradio.notification.NEXT";

    private void registerRadioBroadcastListener() {
        if (null == mRadioReceiver) {
            mRadioReceiver = new RadioBroadcastReceiver();
        }
        IntentFilter filter = new IntentFilter();
        filter.addAction(RADIO_ACTION_NOTIFICATION_SHOW);
        filter.addAction(RADIO_ACTION_NOTIFICATION_PREV);
        filter.addAction(RADIO_ACTION_NOTIFICATION_NEXT);
        filter.addAction(CarStatus.ACTION_ACC);
        filter.addAction(CarStatus.ACTION_REVSTATUS);
        registerReceiver(mRadioReceiver, filter);
    }

    private void unregisterRadioBroadcastListener() {
        if (null != mRadioReceiver) {
            unregisterReceiver(mRadioReceiver);
        }
    }

    // receive external broadcast
    private final class RadioBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (RADIO_ACTION_NOTIFICATION_SHOW.equals(action)) {
                showRadioMainActivity();
            } else if (RADIO_ACTION_NOTIFICATION_PREV.equals(action)) {
                Log.i(TAG, "--------- FMPlugService seekUp");
                RadioPlayer.getRadioPlayer().prev();
            } else if (RADIO_ACTION_NOTIFICATION_NEXT.equals(action)) {
                Log.i(TAG, "---------FMPlugService  seekDown");
                RadioPlayer.getRadioPlayer().next();
            } else if (CarStatus.ACTION_ACC.equals(action)) {
                if (intent.getBooleanExtra(CarStatus.EXTRA_ACC, true)) {//ACC on
                    mFMUpdateHandler.removeMessages(
                            FMUpdateHandler.EXIT_INSIDE_RADIO_RENDER_THREAD);
                    RadioAudioManager.getInstance().startRender();
                } else {//ACC off
                    mFMUpdateHandler.removeMessages(
                            FMUpdateHandler.EXIT_INSIDE_RADIO_RENDER_THREAD);
                    mFMUpdateHandler.sendEmptyMessageDelayed(
                            FMUpdateHandler.EXIT_INSIDE_RADIO_RENDER_THREAD, 5000);
                }
            } else if (CarStatus.ACTION_REVSTATUS.equals(action)) {
                // 获取倒车状态
                boolean isReverseState = intent.getBooleanExtra(CarStatus.EXTRA_REVSTATUS, false);
                if (isReverseState && mFMDCC.isTAWindowShow()) {
                    mFMDCC.hideTAWindow();
                }
            }
        }
    }

    public void showRadioMainActivity() {
        Intent intentRadio = new Intent(this, RadioMain.class);
        intentRadio.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intentRadio);
    }

    // data change event
    private final class OnUpdateDataListener implements UpdateDataListener {

        @Override
        public void updateRadioUIElement(int nType) {

            switch (nType) {
                case UPDATE_DATA_RANGE:
                    break;
                case UPDATE_DATA_INFO:
                    if (null != mFMUpdateHandler) {
                        mFMUpdateHandler.removeMessages(FMUpdateHandler.UPDATE_NOTIFICATION_INFO);
                        mFMUpdateHandler.sendEmptyMessageDelayed(
                                FMUpdateHandler.UPDATE_NOTIFICATION_INFO, 500);

                        if (mFMDCC.getScanType() != SEEK_ALL
                                && mFMDCC.getScanType() != SEEK_PLAY
                                && mFMDCC.getScanType() != PRESET_PLAY) {
                            mFMUpdateHandler.removeMessages(FMUpdateHandler.SAVE_RADIO_INFO);
                            mFMUpdateHandler.sendEmptyMessageDelayed(
                                    FMUpdateHandler.SAVE_RADIO_INFO, 1000);
                        }
                    }
                    break;
                case UPDATE_DATA_FREQLIST:
                    break;
                default:
                    break;
            }
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "FMPlugService onCreate");

        if (null == mFMUpdateHandler) {
            mFMUpdateHandler = new FMUpdateHandler(this);
        }
        // listener radio data
        if (null == mDataListener) {
            mDataListener = new OnUpdateDataListener();
        }

        registerRadioBroadcastListener();
        registerBluetoothReceiver();

        RadioAudioManager mRadioAudioManager = RadioAudioManager.getInstance();
        mRadioAudioManager.registerMediaButtonEvent();
        mRadioAudioManager.stopRender();
        mRadioAudioManager.requestAudioFocus(AudioManager.AUDIOFOCUS_GAIN,
                AudioAttributes.USAGE_MEDIA,AudioAttributes.CONTENT_TYPE_MUSIC);

        if (null == mFMDCC) {
            mFMDCC = FMDataControl.getInstance();
            mFMDCC.registerDataChangeListener(FMPlugService.class.getSimpleName(), mDataListener);
            mFMDCC.initFMCollectionFunction();
            mFMDCC.initRadioPlayerEventListener();
            mFMDCC.initCurrentFreq();
        }

        updateRadioNotification();
        mRadioAudioManager.startRender();

        mCallBackList = new RemoteCallbackList<IRadioCallBack>();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "FMPlugService onStartCommand");
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "FMPlugService onBind");
        if (null == mFMBinder) {
            mFMBinder = new FMBinder();
        }
        return (IBinder) mFMBinder;
    }

    @Override
    public void onRebind(Intent intent) {
        super.onRebind(intent);
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.d(TAG, "FMPlugService onUnbind");
        return super.onUnbind(intent);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "FMPlugService onDestroy");

        // Radio Broadcast
        cancelNotification();
        if (null != mFMUpdateHandler) {
            mFMUpdateHandler.removeCallbacksAndMessages(null);
        }
        if (null != mFMDCC) {
            mFMDCC.unRegisterDataChangeListener(FMPlugService.class.getSimpleName());
            mFMDCC.saveRadioData(true);
            mFMDCC.unInitRadioPlayEventListener();
        }

        RadioAudioManager.getInstance().exitRenderThread();
        RadioAudioManager.getInstance().removeCallbacksAndMessages();
        unregisterBluetoothReceiver();
        unregisterRadioBroadcastListener();

        RadioAudioManager.getInstance().unregisterMediaButtonEvent();
        RadioAudioManager.getInstance().releaseAudioFocus();
        System.exit(0);
    }

    /**
     * [注册蓝牙状态相关广播]
     */
    private void registerBluetoothReceiver() {
        // [FM 是否启用处理蓝牙状态逻辑]
        String monitor = RadioUtils.getProp("persist.fm.monitor.bt", "0");
        if (monitor.equals("0")) {
            return;
        }

        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothHeadsetClient.ACTION_AUDIO_STATE_CHANGED);
        Intent it = registerReceiver(mBluetoothStatusReceiver, filter);
        mIsRegisterBluetoothBroadcast = it != null;
    }

    private void unregisterBluetoothReceiver() {
        // [检查广播监听状态]
        if (!mIsRegisterBluetoothBroadcast) {
            // [unregisterReceiver 如果和 registerReceiver 不匹配会异常]
            return;
        }

        try {
            unregisterReceiver(mBluetoothStatusReceiver);
            mIsRegisterBluetoothBroadcast = false;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * [监听蓝牙相关状态广播]
     */
    private boolean mIsRegisterBluetoothBroadcast = false;
    private final BroadcastReceiver mBluetoothStatusReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String mAction = intent.getAction();
            if (mAction.equals(BluetoothHeadsetClient.ACTION_AUDIO_STATE_CHANGED)) {
                int extraState = intent.getIntExtra(BluetoothProfile.EXTRA_STATE, -1);
                Log.d(TAG, "ACTION_AUDIO_STATE_CHANGED:"
                        + " BluetoothProfile State = " + String.valueOf(extraState));

                mFMUpdateHandler.removeMessages(H.MSG_BT_HEADSET_AUDIO_CONNECTED);
                mFMUpdateHandler.removeMessages(H.MST_BT_HEADSET_AUDIO_DISCONNECTED);

                switch (extraState) {
                    case BluetoothProfile.STATE_CONNECTED:
                    case BluetoothProfile.STATE_CONNECTING:
                        mFMUpdateHandler.sendEmptyMessage(H.MSG_BT_HEADSET_AUDIO_CONNECTED);
                        break;
                    case BluetoothProfile.STATE_DISCONNECTED:
                        mFMUpdateHandler.sendEmptyMessageDelayed(H.MST_BT_HEADSET_AUDIO_DISCONNECTED, 1000);
                        break;
                    default:
                        break;
                }
            }
        }
    };

    private Notification mRadioNotification = null;
    private RemoteViews mRadioRemoteViews = null;
    private Notification.Builder mNotifyBuilder = null;
    private NotificationManager mNotifyManager = null;

    private static final int RADIO_PLAYER_NOTIFY_ID = 0;
    private static final String RADIO_ACTION_NOTIFICATION_SHOW =
            "com.hcn.autoradio.notification_show";
    private int mRemoteViewsActionSize = 0;
    private static final int REMOTE_VIEWS_ACTION_MAX_SIZE = 50;

    private void updateRadioNotification() {
        //RemoteViews.setTextViewText过多导致的TransactionTooLargeException
        if (mRemoteViewsActionSize > REMOTE_VIEWS_ACTION_MAX_SIZE) {
            cancelNotification();
            mRadioRemoteViews = null;
            mRemoteViewsActionSize = 0;
        }
        initRadioPlayerNotification();
        if (null != mRadioRemoteViews) {
            mRadioRemoteViews.setTextViewText(R.id.notification_title,
                    getString(R.string.app_name));
            if (null != mFMDCC) {
                String str = String.format("%s: %s", getString(R.string.listening),
                        mFMDCC.getFormatFreq(mFMDCC.currentFreq(), true));
                mRadioRemoteViews.setTextViewText(R.id.notification_text, str);
            }

            mRemoteViewsActionSize += 1;
        }

        if (null != mNotifyManager && null != mRadioNotification) {
            mRadioNotification.flags = Notification.FLAG_ONGOING_EVENT;
            mNotifyManager.notify(RADIO_PLAYER_NOTIFY_ID, mRadioNotification);
        }
    }

    private void initRadioPlayerNotification() {
        if (null == mNotifyManager) {
            mNotifyManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        }

        if (null == mRadioNotification) {
            if (null == mRadioRemoteViews) {
                mRadioRemoteViews = new RemoteViews(this.getPackageName(),
                        R.layout.radio_notification);
            }

            if (null == mNotifyBuilder) {
                mNotifyBuilder = new Notification.Builder(this);
            }

            Intent intent = new Intent(RADIO_ACTION_NOTIFICATION_SHOW);
            PendingIntent pIntent = PendingIntent.getBroadcast(this, 0, intent,
                    PendingIntent.FLAG_UPDATE_CURRENT);

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                String id = "channel_002";
                String name = "Radio";
                NotificationChannel mChannel = new NotificationChannel(id, name,
                        NotificationManager.IMPORTANCE_LOW);
                mNotifyManager.createNotificationChannel(mChannel);
                mRadioNotification = new Notification.Builder(this)
                        .setChannelId(id)
                        .setOngoing(true)
                        .setSmallIcon(R.drawable.fm_notification_icon)
                        .setTicker(getString(R.string.app_name))
                        .setContent(mRadioRemoteViews).setContentIntent(pIntent)
                        .build();
            } else {
                mRadioNotification = mNotifyBuilder.setOngoing(true)
                        .setSmallIcon(R.drawable.fm_notification_icon)
                        .setTicker(getString(R.string.app_name))
                        .setContent(mRadioRemoteViews).setContentIntent(pIntent)
                        .build();
            }

            mRadioNotification.bigContentView = mRadioRemoteViews;
        }
    }

    private void cancelNotification() {
        if (mNotifyManager != null) {
            mNotifyManager.cancel(RADIO_PLAYER_NOTIFY_ID);
            mRadioNotification = null;
        }
    }

    // Update Handler
    private static class H extends FMUpdateHandler {
        public H(FMPlugService service) {
            super(service);
        }
    }

    private static class FMUpdateHandler extends Handler {
        public static final int UPDATE_NOTIFICATION_INFO = 0;
        public static final int SAVE_RADIO_INFO = 1;
        public static final int EXIT_INSIDE_RADIO_RENDER_THREAD = 2;

        // [蓝牙 HFP Audio 状态]
        public static final int MSG_BT_HEADSET_AUDIO_CONNECTED = 3;
        public static final int MST_BT_HEADSET_AUDIO_DISCONNECTED = 4;

        //申请切换音频
        public static final int MSG_REQUEST_AUDIO_PLAY = 5;

        private final WeakReference<FMPlugService> mRadioService;

        public FMUpdateHandler(FMPlugService service) {
            mRadioService = new WeakReference<FMPlugService>(service);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            FMPlugService service = mRadioService.get();
            if (service == null) {
                return;
            }

            switch (msg.what) {
                case UPDATE_NOTIFICATION_INFO:
                    service.updateRadioNotification();
                    break;
                case SAVE_RADIO_INFO:
                    if (service.mFMDCC != null) {
                        service.mFMDCC.saveRadioData(false);
                    }
                    break;
                case EXIT_INSIDE_RADIO_RENDER_THREAD:
                    RadioAudioManager.getInstance().stopRender();
                    break;
                case MSG_BT_HEADSET_AUDIO_CONNECTED:
                    service.onMsgBtHeadsetAudioConnected();
                    break;
                case MST_BT_HEADSET_AUDIO_DISCONNECTED:
                    service.onMsgBtHeadsetAudioDisconnected();
                    break;
                case MSG_REQUEST_AUDIO_PLAY:
                    service.requestPlayAudio();
                default:
                    break;
            }
        }
    }

    // [蓝牙 HFP Audio 状态连接]
    private void onMsgBtHeadsetAudioConnected() {
        Log.d(TAG, "onMsgBtHeadsetAudioConnected");
        RadioAudioManager.getInstance().stopRender();
    }

    // [蓝牙 HFP Audio 状态断开]
    private void onMsgBtHeadsetAudioDisconnected() {
        Log.d(TAG, "onMsgBtHeadsetAudioDisconnected");
        if (mCarStatus.getAccStatus() > 0) {
            RadioAudioManager.getInstance().startRender();
        }
    }

    /**
     * 预留回调，后续扩展
     * @param method
     */
    private synchronized void radioCallBack(final int method) {
        if (mCallBackList != null) {
            int count = mCallBackList.beginBroadcast();
            try {
                for (int i = 0; i < count; i++) {
                    IRadioCallBack c = mCallBackList.getBroadcastItem(i);
                    switch (method) {
                        case IEvent.EVENT_RADIO_APP_EXIT:
                            c.onEvent(method, "radio-app-exit");
                            break;
                        default:
                            break;
                    }
                }
            } catch (RemoteException e) {
                 e.printStackTrace();
            } finally {
                mCallBackList.finishBroadcast();
            }
        } else {
            Log.d(TAG, "mCallBackList is null!!!");
        }
    }

    /**
     * 客户端死亡监听
     */
    class RadioDeathRecipient implements IBinder.DeathRecipient {

        @Override
        public void binderDied() {
            // TODO Auto-generated method stub
            unRegisterRadioClientBinder();
            Log.i(TAG, "RadioDeathRecipient binderDied");
        }
    }

    /**
     * 注册客户端binder
     *
     * @param clientBinder
     * @throws RemoteException
     */
    public synchronized void registerRadioClientBinder(IBinder clientBinder) throws RemoteException {
        Log.i(TAG, "registerRadioClientBinder");
        mRadioClientBinder = clientBinder;
        if (null != mRadioClientBinder) {
            if (null == mRadioDeathRecipient) {
                mRadioDeathRecipient = new RadioDeathRecipient();
            }
            mRadioClientBinder.linkToDeath(mRadioDeathRecipient, 0);
        }
    }

    /**
     * 取消注册客户端binder
     */
    public synchronized void unRegisterRadioClientBinder() {
        Log.i(TAG, "unRegisterRadioClientBinder");
        if (null != mRadioClientBinder && null != mRadioDeathRecipient) {
            mRadioClientBinder.unlinkToDeath(mRadioDeathRecipient, 0);
        }
        mRadioDeathRecipient = null;
        mRadioClientBinder = null;
    }

    /**
     * 注册回调事件
     *
     * @param callback
     */
    void registerRadioCallback(IRadioCallBack callback) {
        if (null == callback) {
            return;
        }
        mCallBackList.register(callback);
        Log.d(TAG, "registerRadioCallback: size=" + mCallBackList.getRegisteredCallbackCount());
    }

    /**
     * 取消注册回调事件
     *
     * @param callback
     */
    public void unRegisterRadioCallback(IRadioCallBack callback) {
        if (null == callback) {
            return;
        }
        mCallBackList.unregister(callback);
        Log.d(TAG, "unRegisterRadioCallback: size=" + mCallBackList.getRegisteredCallbackCount());
    }

    /**
     * FM/AM切换波段
     */
    public void onBandEvent() {
        if (mFMDCC != null) {
            mFMDCC.Band((mFMDCC.currentBand() + 1) % BAND_SIZE);
        }
    }

    /**
     * 自动搜索存台
     */
    public void onASEvent() {
        if (mFMDCC != null) {
            mFMDCC.AS();
        }
    }

    /**
     * 浏览存储电台
     */
    public void onPSEvent() {
        if (mFMDCC != null) {
            mFMDCC.PS();
        }
    }

    /**
     * 远近程切换
     */
    public void onLocDxEvent() {
        if (mFMDCC != null) {
            mFMDCC.Local();
        }
    }

    /**
     * 向下收搜有效台
     */
    public void onSeekDownEvent() {
        if (mFMDCC != null) {
            mFMDCC.seekDown();
        }
    }

    /**
     * 向上收搜有效台
     */
    public void onSeekUpEvent() {
        if (mFMDCC != null) {
            mFMDCC.seekUp();
        }
    }

    /**
     * 步进一个单位
     */
    public void onManualUpEvent() {
        if (mFMDCC != null) {
            mFMDCC.stepUp();
        }
    }

    /**
     * 步进一个单位
     */
    public void onManualDownEvent() {
        if (mFMDCC != null) {
            mFMDCC.stepDown();
        }
    }

    /**
     * 全域收搜有效电台
     */
    public void onScanEvent() {
        if (mFMDCC != null) {
            mFMDCC.scan();
        }
    }

    /**
     * 切到对应频点
     *
     * @param freq
     */
    public void gotoFreq(int freq) {
        if (mFMDCC != null) {
            mFMDCC.setFreq(freq);
        }
    }

    /**
     * 切到对应频点
     *
     * @param freq
     */
    public void gotoFreq(String freq) {
        if (mFMDCC == null) {
            return;
        }
        mFMDCC.gotoFreq(freq);
    }

    /**
     * 切到预存台
     *
     * @param index
     */
    public void gotoFreqIndex(int index) {

    }

    /**
     * 获取当前FM/AM波段
     *
     * @return
     */
    public int getCurrentBand() {
        if (null == mFMDCC) {
            return BAND_FM_1;
        }
        return mFMDCC.currentBand();
    }

    /**
     * 获取当前电台
     *
     * @return
     */
    public int getCurrentFreq() {
        if (null == mFMDCC) {
            return 87500;
        }
        return mFMDCC.currentFreq();
    }

    /**
     * 是否处于自动搜索存台中
     *
     * @return
     */
    public boolean IsAS() {
        if (null == mFMDCC) {
            return false;
        }
        int scanType = mFMDCC.getScanType();
        return scanType == RadioData.SEEK_ALL;
    }

    /**
     * 是否处于电台浏览中
     *
     * @return
     */
    public boolean IsPS() {
        if (null == mFMDCC) {
            return false;
        }
        int scanType = mFMDCC.getScanType();
        return scanType == RadioData.PRESET_PLAY;
    }

    /**
     * 是否在进行搜台
     *
     * @return
     */
    public boolean IsScan() {
        if (null == mFMDCC) {
            return false;
        }
        int scanType = mFMDCC.getScanType();
        return scanType == RadioData.SEEK_PLAY;
    }

    /**
     * 是否在进行上下一个有效台收搜
     *
     * @return
     */
    public boolean IsSeek() {
        if (null == mFMDCC) {
            return false;
        }
        int scanType = mFMDCC.getScanType();
        return scanType == RadioData.SEEK_UP || scanType == RadioData.SEEK_DOWN;
    }

    /**
     * 是否是stereo状态（立体声状态）
     *
     * @return
     */
    public boolean IsStereo() {
        if (null == mFMDCC) {
            return false;
        }
        return mFMDCC.isStereo();
    }

    /**
     * 是否远近程
     *
     * @return
     */
    public boolean IsDxLocal() {
        if (null == mFMDCC) {
            return false;
        }
        return mFMDCC.isLocal();
    }

    /**
     * 请求切源
     */
    public boolean requestPlayAudio() {
        return RadioUtils.requestPlayAudio(this);
    }
    public boolean requestPlayAudio(boolean isBinder) {
        Log.v(TAG, " requestPlayAudio isBinder = " + isBinder);
        // 不要在 Binder 的调用栈中使用反射接口；
        if (isBinder) {
            if (null != mFMUpdateHandler) {
                mFMUpdateHandler.removeMessages(H.MSG_REQUEST_AUDIO_PLAY);
                mFMUpdateHandler.sendEmptyMessage(H.MSG_REQUEST_AUDIO_PLAY);
            }
            return true;
        }
        return requestPlayAudio();
    }

    /**
     * 定义Binder类
     */
    public class FMBinder extends IRadioServiceAPI.Stub {

        @Override
        public void registerRadioClientBinder(IBinder clientBinder) throws RemoteException {
                FMPlugService.this.registerRadioClientBinder(clientBinder);
        }

        @Override
        public void unRegisterRadioClientBinder() throws RemoteException {
            FMPlugService.this.unRegisterRadioClientBinder();
        }

        @Override
        public void registerRadioCallback(IRadioCallBack callback) throws RemoteException {
            FMPlugService.this.registerRadioCallback(callback);
        }

        @Override
        public void unRegisterRadioCallback(IRadioCallBack callback) throws RemoteException {
            FMPlugService.this.unRegisterRadioCallback(callback);
        }

        @Override
        public void onBandEvent() throws RemoteException {
            FMPlugService.this.onBandEvent();
        }

        @Override
        public void onASEvent() throws RemoteException {
            FMPlugService.this.onASEvent();
        }

        @Override
        public void onPSEvent() throws RemoteException {
            FMPlugService.this.onPSEvent();
        }

        @Override
        public void onLocDxEvent() throws RemoteException {
            FMPlugService.this.onLocDxEvent();
        }

        @Override
        public void onSeekDownEvent() throws RemoteException {
            FMPlugService.this.onSeekDownEvent();
        }

        @Override
        public void onSeekUpEvent() throws RemoteException {
            FMPlugService.this.onSeekUpEvent();
        }

        @Override
        public void onManualUpEvent() throws RemoteException {
            FMPlugService.this.onManualUpEvent();
        }

        @Override
        public void onManualDownEvent() throws RemoteException {
            FMPlugService.this.onManualDownEvent();
        }

        @Override
        public void onScanEvent() throws RemoteException {
            FMPlugService.this.onScanEvent();
        }

        @Override
        public void gotoFreq(int freq) throws RemoteException {
            FMPlugService.this.gotoFreq(freq);
        }

        @Override
        public void gotoFreq2(String freq) throws RemoteException {
            FMPlugService.this.gotoFreq(freq);
        }

        @Override
        public void gotoFreqIndex(int index) throws RemoteException {

        }

        @Override
        public int getCurrentBand() throws RemoteException {
            return FMPlugService.this.getCurrentBand();
        }

        @Override
        public int getCurrentFreq() throws RemoteException {
            return FMPlugService.this.getCurrentFreq();
        }

        @Override
        public boolean IsAS() throws RemoteException {
            return FMPlugService.this.IsAS();
        }

        @Override
        public boolean IsPS() throws RemoteException {
            return FMPlugService.this.IsPS();
        }

        @Override
        public boolean IsScan() throws RemoteException {
            return FMPlugService.this.IsScan();
        }

        @Override
        public boolean IsSeek() throws RemoteException {
            return FMPlugService.this.IsSeek();
        }

        @Override
        public boolean IsStereo() throws RemoteException {
            return FMPlugService.this.IsStereo();
        }

        @Override
        public boolean IsDxLocal() throws RemoteException {
            return FMPlugService.this.IsDxLocal();
        }

        @Override
        public boolean requestPlayAudio() throws RemoteException {
            return FMPlugService.this.requestPlayAudio(true);
        }

    }
}


