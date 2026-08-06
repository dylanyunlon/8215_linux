package com.hcn.bluetooth.service;

import static android.bluetooth.BluetoothAdapter.SCAN_MODE_CONNECTABLE;
import static android.bluetooth.BluetoothAdapter.SCAN_MODE_CONNECTABLE_DISCOVERABLE;
import static android.bluetooth.BluetoothAdapter.SCAN_MODE_NONE;
import static com.hcn.bluetooth.api.LocalBluetoothAdapterManager.THIRD_PART_LETTER_HICAR_MODE;
import static com.hcn.bluetooth.api.LocalBluetoothAdapterManager.THIRD_PART_NORMAL_MODE;
import static com.hcn.bluetooth.api.LocalBluetoothAdapterManager.THIRD_PART_ZJ_CARPLAY_MODE;

import android.annotation.SuppressLint;
import android.app.Service;
import android.bluetooth.BluetoothA2dpSink;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothHeadsetClient;
import android.bluetooth.BluetoothHeadsetClientCall;
import android.bluetooth.BluetoothPan;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.ArraySet;
import android.util.Log;
import android.view.KeyEvent;

import com.hcn.bluetooth.api.BluetoothDeviceInfo;
import com.hcn.bluetooth.api.Utils;
import com.hcn.bluetooth.bean.CacheData;
import com.hcn.bluetooth.protocol.A2dpSinkProfile;
import com.hcn.bluetooth.protocol.CachedBluetoothDevice;
import com.hcn.bluetooth.protocol.CachedBluetoothDeviceManager;
import com.hcn.bluetooth.protocol.HeadsetClientProfile;
import com.hcn.bluetooth.protocol.LocalBluetoothAdapter;
import com.hcn.bluetooth.protocol.LocalBluetoothManager;
import com.hcn.bluetooth.protocol.LocalBluetoothProfile;
import com.hcn.bluetooth.protocol.LocalBluetoothProfileManager;
import com.hcn.bluetooth.protocol.PanProfile;
import com.hcn.bluetoothservice.R;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

public class BluetoothAdapterManagerService extends Service {

    public static final String TAG = "BTAdapterService";
    public static final String DEFAULT_NAME = "Car BT";
    public static final String DEFAULT_PINCODE = "0000";
    //蓝牙reset后延时1s打开
    public static final int DEFAULT_BT_RESET_TIME = 1000;
    //蓝牙通话自动连接延时
    public static final int AUTO_CONNECT_DELAY = 5000;
    //蓝牙网络自动连接打开时延时10s尝试一次
    public static final int AUTO_NETWORK_CONNECT_DELAY = 10000;
    //ACC off后延时2s关闭蓝牙
    public static final int ACC_OFF_DELAY_CLOSE_BT = 2000;
    //Power off后延时1s关闭蓝牙
    public static final int POWER_OFF_DELAY_CLOSE_BT = 500;
    //Power键广播
    public static final String ACTION_SILENCE_BUTTON = "android.mcu.device.action.SILENCE_BUTTON";
    public static int KEYEVENT_POWER_OFF = 32;
    public static int KEYEVENT_POWER_ON = 48;
    //acc 广播
    public static final String ACTION_ACC = "android.mcu.device.action.ACC";
    private boolean mAccStatus = true;

    //至简手机互联及CarPlay状态广播
    public static final String ZJ_MIRROR_CARPLAY_STATUS = "com.zjinnova.zlink";
    private static final String ZJ_CARPLAY_WIRED = "carplay_wired";
    private static final String ZJ_CARPLAY_WIRELESS = "carplay_wireless";
    private static final String ZJ_ANDROID_AUTO_WIRED = "auto_wired";
    private static final String ZJ_ANDROID_AUTO_WIRELESS = "auto_wireless";
    private static final String ZJ_MIRROR_WIRED = "android_mirror_wired";
    private static final String ZJ_MIRROR_WIRELESS = "android_mirror_wireless";

    //莱特手机互联
    private static final String LETTER_ANDROID_AUTO_WIRED = "aauto_wired";
    private static final String LETTER_ANDROID_AUTO_WIRELESS = "aauto_wireless";

    //莱特carlife
    private static final String ACTION_LETTER_CAR_LIFE_LINK = "com.hcn.link";
    private static final String CAR_LIFE_MODE = "carlife";

    //HiCar连接状态广播
    public static String ACTION_HICAR_LINK = "com.hicar.connect.status";
    public static String EXTRA_BT_ADDRESS = "btAddress";

    public static String STATUS_CONNECT = "CONNECTED";
    public static String STATUS_DISCONNECT = "DISCONNECT";

    //莱特HiCar:断开/恢复连接蓝牙  至简HiCar:断开/恢复连接A2DP
    public static final String ACTION_HCN_LINK = "com.hcn.link";
    public static final String EXTRA_HCN_LINK = "bluetooth";
    public static final String DATA_BLUETOOTH_STOP_A2DP = "stopA2dp";
    public static final String DATA_BLUETOOTH_RESUME_A2DP = "resumeA2dp";
    public static final String DATA_BLUETOOTH_CLOSE_BT = "closeBt";
    public static final String DATA_BLUETOOTH_OPEN_BT = "openBt";
    public static final String DATA_BLUETOOTH_NULL = "NULL";
    private boolean isInLatterHiCarMode = false;
    private boolean isInZjHiCarMode = false;

    //卡比特(亿连提示点击后发送跳转蓝牙界面广播)
    public static String ACTION_EASYCONN_BT_SETTING = "net.easyconn.bt.setting";

    //部分第三方app在使用中，需要与蓝牙互斥，如Carplay
    private int mThirdPartMode = THIRD_PART_NORMAL_MODE;

    private static BluetoothAdapterManagerService sInstance;
    private Context mContext;

    private LocalBluetoothManager mLocalManager = null;
    private LocalBluetoothAdapter mLocalAdapter = null;
    private LocalBluetoothProfileManager mProfileManager = null;
    private CachedBluetoothDeviceManager mCachedDeviceManager = null;

    /**
     * 用户手动断开蓝牙标志
     */
    private boolean mIsDisconnectByUser = false;
    /**
     * 如果当前状态已连接，再去连接其它蓝牙设备时，先断开当前的，用此变量暂存新设备地址
     */
    private String mDelayConnectAddress;

    private List<BluetoothDeviceInfo> mBondedDevices = null;
    private List<BluetoothDeviceInfo> mDiscoveryDevices = null;

    private IBluetoothAdapterManagerService.Stub mBinder = null;

    private int mScanMode = SCAN_MODE_CONNECTABLE_DISCOVERABLE;
    private String mBTName = null;
    private String mBTPincode = null;
    private static String mLastConnectBtAddress = null;
    //蓝牙网络出错计数
    private int mNetworkErrorTimes = 0;

    //回调列表
    private RemoteCallbackList<IAdapterCallback> mCallBackList;
    //对应IAdapterCallback的回调方法
    public static final int CALL_BLUETOOTH_STATE_CHANGED = 1;
    public static final int CALL_DISCOVERY_STATE_CHANGED = 2;
    public static final int CALL_DISCOVERY_DEVICE_FOUND = 3;
    public static final int CALL_DISCOVERY_NAME_CHANGED = 4;
    public static final int CALL_BOND_STATE_CHANGED = 5;
    public static final int CALL_CONNECTION_STATE_CHANGED = 6;

    BluetoothPbapClientHelper mBluetoothPbapClientHelper;


    public static synchronized BluetoothAdapterManagerService getInstance() {
        if (sInstance == null) {
            sInstance = new BluetoothAdapterManagerService();
        }
        return sInstance;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        init();
        super.onCreate();
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        deinit();
        if (mBinder != null) {
            mBinder = null;
        }
        super.onDestroy();
    }

    @Override
    public void onStart(Intent intent, int startId) {
        Log.d(TAG, "onStart");
        super.onStart(intent, startId);
    }

    @Override
    public IBinder onBind(Intent arg0) {
        Log.d(TAG, "onBind");
        if (mBinder == null) {
            mBinder = new BluetoothAdapterManagerBinder();
        }
        return mBinder;
    }

    public void init() {
        Log.i(TAG, "init");
        sInstance = this;
        mContext = getApplicationContext();
        mCallBackList = new RemoteCallbackList<IAdapterCallback>();
        mLocalManager = LocalBluetoothManager.getInstance(mContext, null);
        mProfileManager = mLocalManager.getProfileManager();
        mLocalAdapter = mLocalManager.getBluetoothAdapter();
        mCachedDeviceManager = mLocalManager.getCachedDeviceManager();
        mBluetoothPbapClientHelper = new BluetoothPbapClientHelper(this, mProfileManager);

        mBondedDevices = new ArrayList<>();
        mDiscoveryDevices = new ArrayList<>();

        IntentFilter filter = new IntentFilter();

        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        filter.addAction(BluetoothAdapter.ACTION_SCAN_MODE_CHANGED);
        filter.addAction(BluetoothAdapter.ACTION_LOCAL_NAME_CHANGED);
        //搜索设备相关广播
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_STARTED);
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        filter.addAction(BluetoothDevice.ACTION_FOUND);
        filter.addAction(BluetoothDevice.ACTION_ALIAS_CHANGED);
        filter.addAction(BluetoothDevice.ACTION_NAME_CHANGED);

        filter.addAction(BluetoothDevice.ACTION_ACL_CONNECTED);
        filter.addAction(BluetoothDevice.ACTION_ACL_DISCONNECTED);
        filter.addAction(BluetoothDevice.ACTION_ACL_DISCONNECT_REQUESTED);
        filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        filter.addAction(BluetoothDevice.ACTION_PAIRING_REQUEST);
        filter.addAction(BluetoothDevice.ACTION_PAIRING_CANCEL);

        filter.addAction(BluetoothHeadsetClient.ACTION_CONNECTION_STATE_CHANGED);
        filter.addAction(BluetoothA2dpSink.ACTION_CONNECTION_STATE_CHANGED);
        filter.addAction(BluetoothPan.ACTION_CONNECTION_STATE_CHANGED);
        //系统广播
        filter.addAction(ACTION_ACC);
        filter.addAction(ACTION_SILENCE_BUTTON);
        //至简CarPlay及手机互联状态广播
        filter.addAction(ZJ_MIRROR_CARPLAY_STATUS);
        filter.addAction(ACTION_HICAR_LINK);
        filter.addAction(ACTION_EASYCONN_BT_SETTING);
        filter.addAction(ACTION_HCN_LINK);
        filter.addAction(ACTION_LETTER_CAR_LIFE_LINK);

        mContext.registerReceiver(mReceiver, filter);
        mBluetoothPbapClientHelper.registerReceiver();

        mHanlder.sendEmptyMessageDelayed(MSG_BT_INIT, 500);
    }

    public void deinit() {
        mContext.unregisterReceiver(mReceiver);
        mBluetoothPbapClientHelper.unRegisterReceiver();
    }

    private void generalSetting() {
        if (THIRD_PART_NORMAL_MODE == getThirdPartAPPMode()) {
            // set discoverable
            mLocalAdapter.setScanMode(SCAN_MODE_CONNECTABLE_DISCOVERABLE, 0);
        } else {
            mLocalAdapter.setScanMode(SCAN_MODE_NONE, 0);
        }
        setBTName(getBTName(), false);
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(android.content.Context context,
                android.content.Intent intent) {
            String action = intent.getAction();

            if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE,
                        BluetoothAdapter.STATE_OFF);
                handleStateChanged(state);
            } else if (action.equals(BluetoothAdapter.ACTION_DISCOVERY_STARTED)) {
                Log.i(TAG, "ACTION_DISCOVERY_STARTED");
                callListener(CALL_DISCOVERY_STATE_CHANGED, null, 0x01);
            } else if (action.equals(BluetoothDevice.ACTION_FOUND)) {
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                handleDeviceFound(device);
            } else if (action.equals(BluetoothAdapter.ACTION_DISCOVERY_FINISHED)) {
                Log.i(TAG, "ACTION_DISCOVERY_FINISHED");
                callListener(CALL_DISCOVERY_STATE_CHANGED, null, 0x00);
            } else if (action.equals(BluetoothAdapter.ACTION_SCAN_MODE_CHANGED)) {
                int mode = intent.getIntExtra(BluetoothAdapter.EXTRA_SCAN_MODE,
                        BluetoothAdapter.ERROR);
                handleScanModeChanged(mode);
            } else if (action.equals(BluetoothDevice.ACTION_BOND_STATE_CHANGED)) {
                //配对状态变化广播
                BluetoothDevice device = intent
                        .getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                int state = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE,
                        BluetoothDevice.BOND_NONE);
                int reason = intent.getIntExtra(BluetoothDevice.EXTRA_REASON,
                        BluetoothDevice.ERROR);
                handleBondStateChange(device, state, reason);
            } else if (action.equals(BluetoothAdapter.ACTION_LOCAL_NAME_CHANGED)) {
                String newName = intent.getStringExtra(BluetoothAdapter.EXTRA_LOCAL_NAME);
                Log.e(TAG, "ACTION_LOCAL_NAME_CHANGED:" + newName);
                if (!newName.equals(getBTName())) {
                    Log.e(TAG, "name error,need set again!");
                    setBTName(getBTName(), false);
                }
            } else if (action.equals(BluetoothDevice.ACTION_ALIAS_CHANGED) ||
                    action.equals(BluetoothDevice.ACTION_NAME_CHANGED)) {
                BluetoothDevice bt_device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                if (null != bt_device) {
                    for (BluetoothDeviceInfo device : mDiscoveryDevices) {
                        if (device.getDeviceAddr().equals(bt_device.getAddress())) {
                            Log.i(TAG, "update device name");
                            String aliasName = bt_device.getAliasName();
                            String name = bt_device.getName();
                            if (!TextUtils.isEmpty(aliasName)) {
                                device.mDeviceName = aliasName;
                            } else if (!TextUtils.isEmpty(name)) {
                                device.mDeviceName = name;
                            } else {
                                device.mDeviceName = bt_device.getAddress();
                            }
                            callListener(CALL_DISCOVERY_NAME_CHANGED, device, 0x00);
                        }
                    }
                }

            } else if (action.equals(BluetoothHeadsetClient.ACTION_CONNECTION_STATE_CHANGED)) {
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothProfile.STATE_DISCONNECTED);
                handleConnectStateChanged(device, state);
            } else if (action.equals(BluetoothA2dpSink.ACTION_CONNECTION_STATE_CHANGED)) {
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                int profilestate = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothA2dpSink.STATE_DISCONNECTED);
                handleA2dpsinkConnectStatus(device, profilestate);
            } else if (action.equals(BluetoothDevice.ACTION_PAIRING_REQUEST)) {
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                int variant = intent.getIntExtra(BluetoothDevice.EXTRA_PAIRING_VARIANT, -1);
                int passkey = intent.getIntExtra(BluetoothDevice.EXTRA_PAIRING_KEY, -1);
                if (null == device) {
                    return;
                }
                BluetoothClass bt_class = device.getBluetoothClass();
                int deviceType = BluetoothClass.Device.Major.PHONE;
                if (null != bt_class) {
                    deviceType = bt_class.getMajorDeviceClass();
                }
                Log.d(TAG, "Pairing type=" + deviceTypeToString(deviceType) + " PAIR_VARIANT:"
                        + variant + " passkey:" + passkey);
                switch (variant) {
                    case BluetoothDevice.PAIRING_VARIANT_PIN:
                    case BluetoothDevice.PAIRING_VARIANT_PIN_16_DIGITS:
                        byte[] pin = BluetoothDevice.convertPinToBytes(getBTPincode());
                        if (pin != null) {
                            device.setPin(pin);
                        }
                        abortBroadcast();
                        break;
                    case BluetoothDevice.PAIRING_VARIANT_PASSKEY:
                        device.setPasskey(passkey);
                        break;
                    case BluetoothDevice.PAIRING_VARIANT_PASSKEY_CONFIRMATION:
                    case BluetoothDevice.PAIRING_VARIANT_CONSENT:
                        //屏蔽蓝牙耳机设备,redmi耳机为AUDIO_VIDEO设备
                        if (deviceType == BluetoothClass.Device.Major.WEARABLE
                                || deviceType == BluetoothClass.Device.Major.TOY
                                || deviceType == BluetoothClass.Device.Major.AUDIO_VIDEO) {
                            device.setPairingConfirmation(false);
                        } else {
                            device.setPairingConfirmation(true);
                        }
                        abortBroadcast();
                        break;
                    case BluetoothDevice.PAIRING_VARIANT_DISPLAY_PASSKEY:
                    case BluetoothDevice.PAIRING_VARIANT_DISPLAY_PIN:
                        // Do nothing.
                        break;
                    case BluetoothDevice.PAIRING_VARIANT_OOB_CONSENT:
                        device.setRemoteOutOfBandData();
                        break;
                    default:
                        Log.e(TAG, "Incorrect pairing type received");
                        break;
                }
            } else if (action.equals(BluetoothDevice.ACTION_PAIRING_CANCEL)) {
                Log.e(TAG, "ACTION_PAIRING_CANCEL");
            } else if (action.equals(BluetoothDevice.ACTION_ACL_CONNECTED)) {
                Log.e(TAG, "ACTION_ACL_CONNECTED");
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                handleACLConnected(device);
            } else if (action.equals(BluetoothDevice.ACTION_ACL_DISCONNECTED)) {
                Log.e(TAG, "ACTION_ACL_DISCONNECTED");
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                handleACLDisconnected(device);
            } else if (action.equals(BluetoothDevice.ACTION_ACL_DISCONNECT_REQUESTED)) {
                Log.e(TAG, "ACTION_ACL_DISCONNECT_REQUESTED");
            } else if (action.equals(ACTION_ACC)) {
                mAccStatus = intent.getBooleanExtra("AccStatus", true);
                if (mAccStatus) {
                    mHanlder.removeMessages(MSG_CLOSE_BT);
                    mHanlder.removeMessages(MSG_OPEN_BT);
                    if (getSavedBtStatus()) {
                        mHanlder.sendEmptyMessage(MSG_OPEN_BT);
                    }
                } else {
                    mHanlder.removeMessages(MSG_CLOSE_BT);
                    mHanlder.removeMessages(MSG_OPEN_BT);
                    mHanlder.sendEmptyMessageDelayed(MSG_CLOSE_BT, ACC_OFF_DELAY_CLOSE_BT);
                }

            } else if (action.equals(ACTION_SILENCE_BUTTON)) {
                if (!mAccStatus) {
                    Log.e(TAG, "Acc off ignore Silence Button event!");
                    return;
                }
                KeyEvent keyEvent = intent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);
                if (keyEvent.getKeyCode() == KeyEvent.KEYCODE_POWER) {
                    if (keyEvent.getMetaState() == KEYEVENT_POWER_OFF) {
                        Log.d(TAG, "onSilenceButton power off");
                        mHanlder.removeMessages(MSG_OPEN_BT);
                        mHanlder.removeMessages(MSG_CLOSE_BT);
                        mHanlder.sendEmptyMessageDelayed(MSG_CLOSE_BT, POWER_OFF_DELAY_CLOSE_BT);
                    } else if (keyEvent.getMetaState() == KEYEVENT_POWER_ON) {
                        Log.d(TAG, "onSilenceButton power on");
                        mHanlder.removeMessages(MSG_CLOSE_BT);
                        mHanlder.removeMessages(MSG_OPEN_BT);
                        if (getSavedBtStatus()) {
                            mHanlder.sendEmptyMessageDelayed(MSG_OPEN_BT,
                                    POWER_OFF_DELAY_CLOSE_BT * 3);
                        }
                    }
                }
            } else if (action.equals(BluetoothPan.ACTION_CONNECTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothProfile.STATE_DISCONNECTED);
                Log.d(TAG, "onReceive: PanProfile state=" + state);
                if (state == BluetoothProfile.STATE_DISCONNECTED) {
                    mNetworkErrorTimes = 0;
                }
            } else if (action.equals(ZJ_MIRROR_CARPLAY_STATUS)) {
                handleZJStatus(intent);
            } else if (action.equals(ACTION_HICAR_LINK)) {
                handleHiCarStatus(intent);
            } else if (action.equals(ACTION_HCN_LINK)) {
                handleHcnLinkStatus(intent);
            } else if (action.equals(ACTION_EASYCONN_BT_SETTING)) {
                Utils.startApp(mContext, Utils.BT_PACKAGE_NAME, Utils.BT_ACTIVITY_NAME);
            } else if (action.equals(ACTION_LETTER_CAR_LIFE_LINK)) {
                handleLetterCarLife(intent);
            }
        }
    };

    private static void writeLastConnectedDevice(String address) {
        if (!TextUtils.isEmpty(address)) {
            mLastConnectBtAddress = address;
            SystemProperties.set("persist.sys.bt_address", address);
        }
    }

    private static String readLastConnectedDevice() {
        if (null == mLastConnectBtAddress) {
            mLastConnectBtAddress = SystemProperties.get("persist.sys.bt_address", "");
        }
        return mLastConnectBtAddress;
    }

    private void handleStateChanged(int state) {
        Log.d(TAG, "handleStateChanged:state->" + state);

        switch (state) {
            case BluetoothAdapter.STATE_ON:
                Log.e(TAG, "BT_ON");
                mHanlder.sendEmptyMessage(MSG_BT_ON);
                break;
            case BluetoothAdapter.STATE_TURNING_ON:
                Log.d(TAG, "TURNING_ON");
                break;
            case BluetoothAdapter.STATE_TURNING_OFF:
                Log.e(TAG, "TURNING_OFF");
                break;
            case BluetoothAdapter.STATE_OFF:
                Log.e(TAG, "BT_OFF");
                mHanlder.sendEmptyMessage(MSG_BT_OFF);
                break;
            default:
                Log.e(TAG, "BT_UNKOWN");
                break;
        }
        mBluetoothPbapClientHelper.handleAdapterStateChange(state);
        callListener(CALL_BLUETOOTH_STATE_CHANGED, null, state);
    }

    private void handleScanModeChanged(int mode) {
        mScanMode = mode;
        if (mode == SCAN_MODE_NONE) {
            Log.d(TAG, "handleScanModeChanged SCAN_MODE_NONE");
        } else if (mode == SCAN_MODE_CONNECTABLE) {
            Log.d(TAG, "handleScanModeChanged SCAN_MODE_CONNECTABLE");
        } else if (mode == SCAN_MODE_CONNECTABLE_DISCOVERABLE) {
            Log.d(TAG, "handleScanModeChanged SCAN_MODE_CONNECTABLE_DISCOVERABLE");
        }
    }

    private void handleBondStateChange(BluetoothDevice device, int bondState, int reason) {
        Log.d(TAG, device + " ACTION_BOND_STATE_CHANGED " + bondState);
        if (null == device) {
            return;
        }
        if (bondState == BluetoothDevice.BOND_BONDED) {
            for (int i = 0; i < mDiscoveryDevices.size(); i++) {
                if (mDiscoveryDevices.get(i).getDeviceAddr().equals(device.getAddress())) {
                    mDiscoveryDevices.remove(i);
                    break;
                }
            }
        } else if (bondState == BluetoothDevice.BOND_NONE) {
            boolean contains = false;
            for (int i = 0; i < mDiscoveryDevices.size(); i++) {
                if (mDiscoveryDevices.get(i).getDeviceAddr().equals(device.getAddress())) {
                    contains = true;
                    break;
                }
            }
            //避免数据重复
            if (!contains) {
                BluetoothDeviceInfo info = new BluetoothDeviceInfo();
                String name = device.getName();
                if (TextUtils.isEmpty(name)) {
                    info.setDeviceName(device.getAddress());
                } else {
                    info.setDeviceName(name);
                }
                info.setDeviceStatus(
                        BluetoothDeviceInfo.DeviceStatus.DEVICE_STATUS_DISCONNECTING);
                info.setDeviceAddr(device.getAddress());
                mDiscoveryDevices.add(info);
            }
            switch (reason) {
                case BluetoothDevice.UNBOND_REASON_AUTH_FAILED:
                    Log.d(TAG, "handleBondChanged: " + getString(
                            R.string.bluetooth_pairing_pin_error_message));
                    break;
                case BluetoothDevice.UNBOND_REASON_AUTH_REJECTED:
                    Log.d(TAG, "handleBondChanged: " + getString(
                            R.string.bluetooth_pairing_rejected_error_message));
                    break;
                case BluetoothDevice.UNBOND_REASON_REMOTE_DEVICE_DOWN:
                    Log.d(TAG, "handleBondChanged: " + getString(
                            R.string.bluetooth_pairing_device_down_error_message));
                    break;
                case BluetoothDevice.UNBOND_REASON_DISCOVERY_IN_PROGRESS:
                case BluetoothDevice.UNBOND_REASON_AUTH_TIMEOUT:
                case BluetoothDevice.UNBOND_REASON_REPEATED_ATTEMPTS:
                case BluetoothDevice.UNBOND_REASON_REMOTE_AUTH_CANCELED:
                    Log.d(TAG, "handleBondChanged: " + getString(
                            R.string.bluetooth_pairing_error_message));
                    break;
                default:
                    break;
            }
        }
        BluetoothDeviceInfo deviceInfo = new BluetoothDeviceInfo();
        deviceInfo.setDeviceName(device.getName());
        deviceInfo.setDeviceAddr(device.getAddress());
        deviceInfo.setDeviceStatus(device.getBondState());
        callListener(CALL_BOND_STATE_CHANGED, deviceInfo, bondState);
    }

    private void handleDeviceFound(BluetoothDevice device) {
        if (device != null) {
            String address = device.getAddress();
            int type = device.getType();
            Log.d(TAG, "Found Device: " + device.getName() + " " + address + " type:" + type);
            if (BluetoothDevice.DEVICE_TYPE_LE == type) {
                return;
            }
            if (mDiscoveryDevices == null) {
                mDiscoveryDevices = new ArrayList<>();
            }
            //列表中已存在，更新数据
            for (BluetoothDeviceInfo bluetoothDevice : mDiscoveryDevices) {
                if (bluetoothDevice.getDeviceAddr().equals(device.getAddress())) {
                    if (device.getAliasName() != null) {
                        bluetoothDevice.mDeviceName = device.getAliasName();
                    } else if (device.getName() != null) {
                        bluetoothDevice.mDeviceName = device.getName();
                    } else {
                        bluetoothDevice.mDeviceName = device.getAddress();
                    }
                    Log.i(TAG, "update Info");
                    callListener(CALL_DISCOVERY_DEVICE_FOUND, bluetoothDevice, 0x00);
                    return;
                }
            }
            if (device.getBondState() != BluetoothDevice.BOND_BONDED) {
                BluetoothDeviceInfo info = new BluetoothDeviceInfo();
                if (device.getAliasName() != null) {
                    info.mDeviceName = device.getAliasName();
                } else if (device.getName() != null) {
                    info.mDeviceName = device.getName();
                } else {
                    info.mDeviceName = device.getAddress();
                }
                info.mDeviceStatus = BluetoothDeviceInfo.DeviceStatus.DEVICE_STATUS_UNKOWN;
                info.mDeviceAddr = device.getAddress();
                mDiscoveryDevices.add(info);
                callListener(CALL_DISCOVERY_DEVICE_FOUND, info, 0x00);
            }
        }
    }

    private void handleConnectStateChanged(BluetoothDevice device, int state) {
        if (null == device) {
            return;
        }
        Log.d(TAG, "onReceive: HFP ACTION_CONNECTION_STATE_CHANGED " + state);
        switch (state) {
            case BluetoothProfile.STATE_DISCONNECTED:
                if (mIsDisconnectByUser) {
                    Log.d(TAG, "handleConnectStateChanged DisconnectByUser");
                    mIsDisconnectByUser = false;
                } else if (!mHanlder.hasMessages(MSG_AUTO_CONNECT)) {
                    startAutoConnect(AUTO_CONNECT_DELAY);
                }
                stopAutoConnectedNetwork();
                break;
            case BluetoothProfile.STATE_CONNECTING:
                break;
            case BluetoothProfile.STATE_CONNECTED:
                if(!Utils.isT5Platform()) {
                    Utils.showToast(mContext, R.string.str_handsfree_device_connected);
                }
                stopAutoConnect();
                if (isAutoConnectedNetwork()) {
                    startAutoConnectedNetwork(AUTO_NETWORK_CONNECT_DELAY);
                }
                writeLastConnectedDevice(device.getAddress());
                break;
            case BluetoothProfile.STATE_DISCONNECTING:

                break;
            default:
                break;
        }
        BluetoothDeviceInfo deviceInfo = new BluetoothDeviceInfo();
        deviceInfo.setDeviceAddr(device.getAddress());
        deviceInfo.setDeviceName(device.getName());
        deviceInfo.setDeviceStatus(device.getBondState());
        callListener(CALL_CONNECTION_STATE_CHANGED, deviceInfo, state);
    }

    private void handleA2dpsinkConnectStatus(BluetoothDevice device, int status) {
        Log.d(TAG, "a2dpsink connectstatus:" + status);
        switch (status) {
            case BluetoothProfile.STATE_CONNECTED:
                if(!Utils.isT5Platform()) {
                    if (isInZjHiCarMode) {
                        Log.d(TAG, "isInZjHiCarMode ->disconnect a2dp device:" + device.getAddress() + " after 1 seconds");
                        mHanlder.removeMessages(MSG_DISCONNECT_A2DP);
                        mHanlder.sendEmptyMessageDelayed(MSG_DISCONNECT_A2DP,1000);
                    } else {
                        Utils.showToast(mContext, R.string.str_a2dp_device_connected);
                    }
                }
                break;
            case BluetoothProfile.STATE_DISCONNECTED:
                break;
            default:
                break;
        }
    }

    private void handleACLConnected(BluetoothDevice device) {
        Log.d(TAG, "handleACLConnected");
        mDelayConnectAddress = "";
    }

    private void handleACLDisconnected(BluetoothDevice device) {
        if (device == null) {
            Log.e(TAG, "handleRemoteDeviceDisconnected device is null");
            return;
        }
        if (!TextUtils.isEmpty(mDelayConnectAddress)) {
            Log.d(TAG, "handleACLDisconnected try connect " + mDelayConnectAddress);
            connectDevice(mDelayConnectAddress);
        }
    }

    private boolean isAndroidAutoMode(String phoneMode) {
        return ZJ_ANDROID_AUTO_WIRED.equals(phoneMode) || ZJ_ANDROID_AUTO_WIRELESS.equals(phoneMode)
                || LETTER_ANDROID_AUTO_WIRED.equals(phoneMode) || LETTER_ANDROID_AUTO_WIRELESS.equals(phoneMode);
    }

    private boolean isCarplayMode(String phoneMode) {
        return ZJ_CARPLAY_WIRED.equals(phoneMode) || ZJ_CARPLAY_WIRELESS.equals(phoneMode);
    }

    private boolean isMirrorMode(String phoneMode) {
        return ZJ_MIRROR_WIRED.equals(phoneMode) || ZJ_MIRROR_WIRELESS.equals(phoneMode);
    }

    private void handleZJStatus(Intent i) {
        String status = i.getStringExtra("status");
        String phoneMode = i.getStringExtra("phoneMode");

        Log.d(TAG, "ZJ_MIRROR_CARPLAY_STATUS status=" + status + " phoneMode=" + phoneMode);
        if ("CONNECTED".equals(status)) {
            if (isMirrorMode(phoneMode)) {
                Log.d(TAG, "ZJ_MIRROR_STATUS CONNECTED");
            } else if (isCarplayMode(phoneMode)) {
                mThirdPartMode = THIRD_PART_ZJ_CARPLAY_MODE;
                //Carplay连接时关闭蓝牙，避免Carplay卡顿
                setBluetoothEnable(false, false);
            } else if (isAndroidAutoMode(phoneMode)) {
                disconnectA2dp();
            }
        } else if ("DISCONNECT".equals(status)) {
            if (isCarplayMode(phoneMode)) {
                mThirdPartMode = THIRD_PART_NORMAL_MODE;
                setBluetoothEnable(true, false);
            } else if (isAndroidAutoMode(phoneMode)) {
                restoreConnectA2dp();
            } else if (isMirrorMode(phoneMode)) {
                Log.d(TAG, "ZJ_MIRROR_STATUS DISCONNECTED");
            }
        } else if ("MAIN_PAGE_SHOW".equals(status)) {
            Log.d(TAG, "ZJ_MIRROR_STATUS MAIN_PAGE_SHOW");
        }
    }

    /**
     * 断开a2dp连接
     */
    private void disconnectA2dp() {
        A2dpSinkProfile a2dpSink = mProfileManager.getA2dpSinkProfile();
        if (null == a2dpSink) {
            return;
        }
        List<BluetoothDevice> devices = a2dpSink.getConnectedDevices();
        if (null != devices) {
            for (BluetoothDevice device : devices) {
                disconnectA2dp(device.getAddress());
                //a2dp_sink.setPreferred(device, false);
            }
        }
    }

    /**
     * 恢复a2dp连接
     */
    private void restoreConnectA2dp() {
        A2dpSinkProfile a2dpSink = mProfileManager.getA2dpSinkProfile();
        HeadsetClientProfile hfp = mProfileManager.getHeadsetClientProfile();
        if (null == a2dpSink || null == hfp) {
            return;
        }
        List<BluetoothDevice> devices = hfp.getConnectedDevices();
        if (null != devices && !devices.isEmpty()) {
            BluetoothDevice device = devices.get(0);
            connectA2dp(device.getAddress());
        }
    }

    private void handleHiCarStatus(Intent i) {
        if (null == i) {
            return;
        }
        String status = i.getStringExtra("status");
        Log.d(TAG, "handleHiCarStatus status=" + status);
        if (STATUS_CONNECT.equals(status)) {
            String address = i.getStringExtra(EXTRA_BT_ADDRESS);
            if (TextUtils.isEmpty(address)) {
                Log.e(TAG, "handleHiCarStatus address is empty !");
                return;
            }
            //Hicar要求连接后，如果没有连接蓝牙，需要连接
            HeadsetClientProfile profile = mProfileManager.getHeadsetClientProfile();
            if (null != profile) {
                int state = profile.getConnectionStatus(mLocalAdapter.getRemoteDevice(address));
                if (state == BluetoothProfile.STATE_DISCONNECTED) {
                    connectDevice(address);
                }
            }
        } else if (STATUS_DISCONNECT.equals(status)) {
            BluetoothHfpclientService hfpService = BluetoothHfpclientService.getInstance();
            if (null == hfpService) {
                return;
            }
            //微信语音通话中，HiCar断开后声音走BT，为避免无声音重启蓝牙
            List<BluetoothHeadsetClientCall> calls = hfpService.getCurrentCalls();
            if (null != calls && !calls.isEmpty()) {
                resetBT();
                Log.d(TAG, "handleHiCarStatus resetBT！！");
            }
        }
    }

    /**
     * 8581平台处理莱特/至简HiCar逻辑
     * 莱特HiCar: 连接时关闭蓝牙，断开连接打开蓝牙
     * 至简HiCar: 连接时关闭A2DP, 断开时恢复连接A2DP
     * @param intent
     */
    private void handleHcnLinkStatus(Intent intent) {
        if (null == intent) {
            return;
        }
        String status = intent.getStringExtra(EXTRA_HCN_LINK);
        Log.d(TAG, "handleHcnLinkStatus status=" + status);
        if (DATA_BLUETOOTH_STOP_A2DP.equals(status)) {
            isInZjHiCarMode = true;
            disconnectA2dp();
        } else if (DATA_BLUETOOTH_RESUME_A2DP.equals(status) && isInZjHiCarMode) {
            isInZjHiCarMode = false;
            mHanlder.removeMessages(MSG_DISCONNECT_A2DP);
            restoreConnectA2dp();
        } else if (DATA_BLUETOOTH_CLOSE_BT.equals(status)) {
            mThirdPartMode = THIRD_PART_LETTER_HICAR_MODE;
            isInLatterHiCarMode = true;
            setBluetoothEnable(false, false);
        } else if (DATA_BLUETOOTH_OPEN_BT.equals(status) && isInLatterHiCarMode) {
            mThirdPartMode = THIRD_PART_NORMAL_MODE;
            isInLatterHiCarMode = false;
            setBluetoothEnable(true, false);
        } else if (DATA_BLUETOOTH_NULL.equals(status)) {
            if (isInZjHiCarMode) {
                isInZjHiCarMode = false;
                mHanlder.removeMessages(MSG_DISCONNECT_A2DP);
                restoreConnectA2dp();
            } else if (isInLatterHiCarMode) {
                isInLatterHiCarMode = false;
                mThirdPartMode = THIRD_PART_NORMAL_MODE;
                setBluetoothEnable(true, false);
            }
        }

        checkAAModeChange(intent);
    }

    /**
     * 处理莱特carlife广播
     * @param intent
     */
    private void handleLetterCarLife(Intent intent) {
        if (null == intent) {
            return;
        }
        String status = intent.getStringExtra("status");
        String phoneMode = intent.getStringExtra("phoneMode");
        Log.d(TAG, "handleLetterCarLife status=" + status + " phoneMode=" + phoneMode);
        if (STATUS_CONNECT.equals(status)) {
            if (phoneMode.contains(CAR_LIFE_MODE)) {
                disconnectA2dp();
            }
        } else if (STATUS_DISCONNECT.equals(status)) {
            if (phoneMode.contains(CAR_LIFE_MODE)) {
                restoreConnectA2dp();
            }
        }
    }

    public synchronized List<BluetoothDeviceInfo> getDeviceList() {
        Log.d(TAG, "getDeviceList:" + mDiscoveryDevices.size());
        return mDiscoveryDevices;
    }

    public synchronized List<BluetoothDeviceInfo> getBondedDevices() {
        Set<BluetoothDevice> pairedDevices = mLocalAdapter.getBondedDevices();
        if (pairedDevices == null) {
            Log.e(TAG, "getBondedDevices is null!!");
            return null;
        }
        Log.i(TAG, "getBondedDevices count = " + pairedDevices.size());
        mBondedDevices.clear();
        for (BluetoothDevice device : pairedDevices) {
            BluetoothDeviceInfo info = new BluetoothDeviceInfo(device);
            mBondedDevices.add(info);
        }
        return mBondedDevices;
    }

    public synchronized BluetoothDeviceInfo getConnectDevice() {
        Log.d(TAG, "getConnectDevice");
        HeadsetClientProfile profile = mProfileManager.getHeadsetClientProfile();
        if (null != profile) {
            List<BluetoothDevice> connectedDevices = profile.getDevicesMatchingConnectionStates(
                    new int[]{BluetoothProfile.STATE_CONNECTED});
            if (!connectedDevices.isEmpty()) {
                BluetoothDevice device = connectedDevices.get(0);
                BluetoothDeviceInfo info = new BluetoothDeviceInfo();
                info.setDeviceName(device.getName());
                info.setDeviceStatus(
                        BluetoothDeviceInfo.DeviceStatus.DEVICE_STATUS_CONNECTED);
                info.setDeviceAddr(device.getAddress());
                return info;
            }
        }
        return null;
    }

    /**
     * 蓝牙配对功能
     *
     * @param address
     */
    public synchronized void pairDevice(String address) {
        if (TextUtils.isEmpty(address)) {
            Log.e(TAG, "pairDevice address=null");
            return;
        }
        stopDiscovery();
        BluetoothDevice device = mLocalAdapter.getRemoteDevice(address);
        CachedBluetoothDevice cachedDevice = mCachedDeviceManager.findDevice(device);
        if (cachedDevice != null) {
            cachedDevice.setConnectAfterPairing(false);
            cachedDevice.startPairing();
        }
    }

    /**
     * 如果是未配对设备，先配对，再连接；已配对的设备直接连接，如果当前是连接状态，先断开连接
     *
     * @param address
     */
    public synchronized void connectDevice(String address) {
        if (TextUtils.isEmpty(address)) {
            Log.e(TAG, "connectDevice address=null");
            return;
        }
        CachedBluetoothDevice cachedDevice = mCachedDeviceManager.findDeviceByAddr(address);
        if (null == cachedDevice) {
            Log.e(TAG, "connectDevice cachedDevice null!");
            return;
        }
        if (mProfileManager.isBusy()) {
            Log.e(TAG, "connectDevice ProfileManager isBusy");
            return;
        }

        ArraySet<BluetoothDevice> devices = mProfileManager.getConnectedDevices();
        if (!devices.isEmpty()) {
            boolean delayConnect = false;
            for (BluetoothDevice d : devices) {
                String current = d.getAddress();
                if (!address.equals(current)) {
                    //暂存新设备地址，断开当前连接设备后再去连接新设备
                    mDelayConnectAddress = address;
                    Log.d(TAG, "connectDevice disconnect " + current);
                    disconnectDevice(current);
                    delayConnect = true;
                }
            }
            if (delayConnect) {
                return;
            }
        }

        mDelayConnectAddress = "";
        int bondState = cachedDevice.getBondState();
        if (bondState == BluetoothDevice.BOND_BONDED) {
            if (THIRD_PART_NORMAL_MODE == getThirdPartAPPMode()) {
                Log.d(TAG, "startConnect " + cachedDevice.getAddress());
                cachedDevice.connect(false);
            } else {
                Log.e(TAG, "connectDevice failed mThirdPartMode=" + getThirdPartAPPMode());
            }
        } else if (bondState == BluetoothDevice.BOND_NONE) {
            Log.d(TAG, "startPairing " + cachedDevice.getAddress());
            cachedDevice.setConnectAfterPairing(true);
            cachedDevice.startPairing();
        } else {
            Log.e(TAG, "connectDevice failed bondState=" + bondState);
        }
    }

    public synchronized void disconnectDevice(String address) {
        if (TextUtils.isEmpty(address)) {
            Log.e(TAG, "disconnectDevice address isEmpty!");
            return;
        }
        CachedBluetoothDevice cachedDevice = mCachedDeviceManager.findDeviceByAddr(address);
        if (cachedDevice == null) {
            Log.e(TAG, "disconnectDevice cacheDevice == Null");
            return;
        }
        cachedDevice.disconnect();
        Log.i(TAG, "disconnectDevice:" + address);
    }

    public synchronized void unpairDevice(String address) {
        if (TextUtils.isEmpty(address)) {
            Log.e(TAG, "deleteDevice address=null");
            return;
        }
        Log.i(TAG, "deleteDevice");
        // disPairing is unreliable while scanning, so cancel discovery
        stopDiscovery();

        CachedBluetoothDevice cachedDevice = mCachedDeviceManager.findDeviceByAddr(address);
        if (null != cachedDevice) {
            if (cachedDevice.isConnected()) {
                cachedDevice.setUnpairAfterDisconnect(true);
                cachedDevice.disconnect();
            } else if (cachedDevice.getBondState() == BluetoothDevice.BOND_BONDED) {
                cachedDevice.unpair();
            }
        }
    }

    public synchronized boolean isBluetoothEnable() {
        boolean bRet = mLocalAdapter.isEnabled();
        Log.d(TAG, "isBluetoothEnable:" + bRet);
        return bRet;
    }

    public synchronized boolean isBluetoothConnected() {
        HeadsetClientProfile profile = mProfileManager.getHeadsetClientProfile();
        if (null != profile) {
            return !profile.getDevicesMatchingConnectionStates(
                    new int[]{BluetoothProfile.STATE_CONNECTED}).isEmpty();
        }
        return false;
    }

    /**
     * 调用protocol 开关蓝牙
     *
     * @param enable
     */
    public synchronized void setBluetoothEnable(boolean enable, boolean save) {
        if (enable && !mAccStatus) {
            Log.e(TAG, "Acc off can not enable bluetooth !!!");
            return;
        }
        Log.i(TAG, "setBluetoothEnable:" + enable);
        mHanlder.removeMessages(MSG_CLOSE_BT);
        mHanlder.removeMessages(MSG_OPEN_BT);
        mLocalAdapter.setBluetoothEnabled(enable);

        if (save) {
            SystemProperties.set("persist.sys.BT_Status", String.valueOf(enable));
        }
    }

    /**
     * 获取保存的蓝牙开关状态
     * @return true:打开    false:关闭     默认：打开
     */
    private boolean getSavedBtStatus(){
        return SystemProperties.getBoolean("persist.sys.BT_Status",true);
    }

    public synchronized String getBTName() {
        if (null == mBTName) {
            mBTName = SystemProperties.get("persist.sys.BTName", DEFAULT_NAME);
        }
        Log.d(TAG, "getBTName: " + mBTName);
        return mBTName;
    }

    public synchronized void setBTName(String name, boolean save) {
        if (TextUtils.isEmpty(name)) {
            return;
        }
        Log.d(TAG, "setBTName: " + name + " save=" + save);
        mBTName = name;
        mLocalAdapter.setName(name);
        if (save) {
            SystemProperties.set("persist.sys.BTName", name);
        }
    }

    public synchronized String getBTPincode() {
        if (null == mBTPincode) {
            mBTPincode = SystemProperties.get("persist.sys.bt.pin.code", DEFAULT_PINCODE);
        }
        Log.d(TAG, "getBTPincode " + mBTPincode);
        return mBTPincode;
    }

    public synchronized void setBTPincode(String pincode) {
        if (TextUtils.isEmpty(pincode)) {
            return;
        }
        pincode = pincode.trim();
        if (pincode.length() >= 0x04 && pincode.length() <= 0x10) {
            mBTPincode = pincode;
            Log.d(TAG, "setBTPincode =" + mBTPincode);
            SystemProperties.set("persist.sys.bt.pin.code", pincode);
        }
    }

    public synchronized boolean isBluetoothAutoAnswer() {
        boolean isAutoAnswer = SystemProperties.getBoolean("persist.sys.AutoAnswerBt",
                false);
        Log.i(TAG, "isBluetoothAutoAnswer " + isAutoAnswer);
        return isAutoAnswer;
    }

    public synchronized void setBluetoothAutoAnswer(boolean enable) {
        Log.i(TAG, "setBluetoothAutoAnswer:" + enable);
        SystemProperties.set("persist.sys.AutoAnswerBt", String.valueOf(enable));
    }

    public synchronized void resetBT() {
        Log.i(TAG, "resetBT:");
        if (mLocalAdapter.getBluetoothState() == BluetoothAdapter.STATE_ON) {
            setBluetoothEnable(false, false);
        }
        if (!mHanlder.hasMessages(MSG_OPEN_BT)) {
            mHanlder.sendEmptyMessageDelayed(MSG_OPEN_BT, DEFAULT_BT_RESET_TIME);
        }
    }

    public synchronized void setBluetoothAutoConnect(boolean enable) {
        Log.i(TAG, "setBluetoothAutoConnect:" + enable);
        SystemProperties.set("persist.sys.AutoConnectBt", String.valueOf(enable));
        if (enable) {
            startAutoConnect(0);
        } else {
            stopAutoConnect();
        }
    }

    public synchronized boolean isBluetoothAutoConnect() {
        boolean isAutoConnect = SystemProperties.getBoolean("persist.sys.AutoConnectBt",
                true);
        return isAutoConnect;
    }

    /**
     * 开始自动连接
     *
     * @param delay 延时，单位ms
     */
    private void startAutoConnect(int delay) {
        mAutoConnectTimes = 0;
        if (!isBluetoothAutoConnect()) {
            return;
        }
        mHanlder.removeMessages(MSG_AUTO_CONNECT);
        Log.d(TAG, "Bluetooth will auto connect after " + delay);
        mHanlder.sendEmptyMessageDelayed(MSG_AUTO_CONNECT, delay);
    }

    private void stopAutoConnect() {
        Log.d(TAG, "stopAutoConnect !!!!");
        mAutoConnectTimes = 0;
        mHanlder.removeMessages(MSG_AUTO_CONNECT);
    }

    //设置蓝牙网络自动连接
    public synchronized void setAutoConnectedNetwork(boolean auto) {
        PanProfile panProfile = mProfileManager.getPanProfile();
        if (null == panProfile) {
            return;
        }
        if (auto) {
            startAutoConnectedNetwork(0);
        } else {
            stopAutoConnectedNetwork();
            panProfile.disconnect();
        }
        notifySystemUINetworkState(auto);
        panProfile.saveAutoConnectState(auto);
    }

    public synchronized boolean isAutoConnectedNetwork() {
        PanProfile panProfile = mProfileManager.getPanProfile();
        if (null == panProfile) {
            return false;
        }
        return panProfile.isAutoConnectable();
    }

    private void startAutoConnectedNetwork(int delay) {
        mHanlder.removeMessages(MSG_AUTO_CONNECT_NETWORK);
        mHanlder.sendEmptyMessageDelayed(MSG_AUTO_CONNECT_NETWORK, delay);
    }

    private void stopAutoConnectedNetwork() {
        Log.d(TAG, "stopAutoConnectedNetwork!!!");
        mHanlder.removeMessages(MSG_AUTO_CONNECT_NETWORK);
    }

    /**
     * 通知状态栏更新蓝牙网络图标显示及隐藏状态
     */
    private void notifySystemUINetworkState(boolean state) {
        PanProfile panProfile = mProfileManager.getPanProfile();
        if (null == panProfile) {
            return;
        }
        panProfile.notifySystemUINetworkState(state);
    }

    public synchronized void connectA2dp(String address) {
        CachedBluetoothDevice cachedDevice = mCachedDeviceManager.findDeviceByAddr(address);
        if (cachedDevice == null) {
            Log.e(TAG, "connectA2dp: cachedDevice null");
            return;
        }

        HeadsetClientProfile headset = mProfileManager.getHeadsetClientProfile();
        //HFP连接后，才可以连接A2dp
        if (cachedDevice.isConnectedProfile(headset)) {
            LocalBluetoothProfile a2dp_sink = mProfileManager.getA2dpSinkProfile();
            cachedDevice.connectProfile(a2dp_sink);
        }
    }

    public synchronized void disconnectA2dp(String address) {
        CachedBluetoothDevice cachedDevice = mCachedDeviceManager.findDeviceByAddr(address);
        if (cachedDevice == null) {
            Log.e(TAG, "disconnectA2dp: cachedDevice null");
            return;
        }
        LocalBluetoothProfile a2dp_sink = mProfileManager.getA2dpSinkProfile();
        if (cachedDevice.isConnectedProfile(a2dp_sink)) {
            cachedDevice.disconnect(a2dp_sink);
        }
    }

    public synchronized boolean isA2dpConnected(String address) {
        BluetoothDevice device;
        try {
            device = mLocalAdapter.getRemoteDevice(address);
        } catch (Exception e) {
            Log.d(TAG, "isA2dpConnected: address=" + address + " Exception!!!");
            return false;
        }

        CachedBluetoothDevice cachedDevice = mCachedDeviceManager.findDevice(device);
        if (cachedDevice == null) {
            return false;
        }
        LocalBluetoothProfile a2dp_sink = mProfileManager.getA2dpSinkProfile();
        return cachedDevice.isConnectedProfile(a2dp_sink);
    }

    public synchronized void startDiscovery() {
        // TODO Auto-generated method stub
        if (mDiscoveryDevices != null) {
            mDiscoveryDevices.clear();
        }
        Log.i(TAG, "start.........discovery");
        mLocalAdapter.startScanning(true);
    }

    public synchronized void stopDiscovery() {
        // TODO Auto-generated method stub
        if (isDiscovering()) {
            mLocalAdapter.stopScanning();
        }
    }

    public synchronized boolean isDiscovering() {
        return mLocalAdapter.isDiscovering();
    }

    public synchronized void registerCallback(IAdapterCallback callback) {
        if (null == callback) {
            return;
        }
        mCallBackList.unregister(callback);
        mCallBackList.register(callback);
    }

    public synchronized void unregisterCallback(IAdapterCallback callback) {
        if (null == callback) {
            return;
        }
        mCallBackList.unregister(callback);
    }

    public synchronized void callListener(final int method, BluetoothDeviceInfo device, int state) {
        int count = mCallBackList.beginBroadcast();
        Log.d(TAG, "callListener: count=" + count);
        try {
            for (int i = 0; i < count; i++) {
                IAdapterCallback c = mCallBackList.getBroadcastItem(i);
                switch (method) {
                    case CALL_BLUETOOTH_STATE_CHANGED:
                        c.onBluetoothStateChanged(state);
                        break;
                    case CALL_DISCOVERY_STATE_CHANGED:
                        c.onDiscoveryStateChanged(state);
                        break;
                    case CALL_DISCOVERY_DEVICE_FOUND:
                        c.onDiscoveryDeviceFound(device);
                        break;
                    case CALL_DISCOVERY_NAME_CHANGED:
                        //c.onDiscoveryDeviceNameChanged();
                        break;
                    case CALL_BOND_STATE_CHANGED:
                        c.onDeviceBondStateChanged(device, state);
                        break;
                    case CALL_CONNECTION_STATE_CHANGED:
                        c.onConnectionStateChanged(device, state);
                        break;
                    default:
                        break;
                }
            }
        } catch (RemoteException e) {

        } finally {
            mCallBackList.finishBroadcast();
        }
    }

    /**
     * 与蓝牙互斥的第三方app是否正在使用中
     *
     * @return
     */
    private int getThirdPartAPPMode() {
        return mThirdPartMode;
    }

    /**
     * 返回蓝牙设备类型
     *
     * @return
     */
    private String deviceTypeToString(int type) {
        String name = "";
        switch (type) {
            case BluetoothClass.Device.Major.MISC:
                name = "misc";
                break;
            case BluetoothClass.Device.Major.COMPUTER:
                name = "computer";
                break;
            case BluetoothClass.Device.Major.PHONE:
                name = "phone";
                break;
            case BluetoothClass.Device.Major.NETWORKING:
                name = "networking";
                break;
            case BluetoothClass.Device.Major.AUDIO_VIDEO:
                name = "audio_video";
                break;
            case BluetoothClass.Device.Major.PERIPHERAL:
                name = "peripheral";
                break;
            case BluetoothClass.Device.Major.IMAGING:
                name = "imaging";
                break;
            case BluetoothClass.Device.Major.WEARABLE:
                name = "wearable";
                break;
            case BluetoothClass.Device.Major.TOY:
                name = "toy";
                break;
            case BluetoothClass.Device.Major.HEALTH:
                name = "health";
                break;
            default:
                break;
        }
        return name;
    }

    private static final int MSG_BT_INIT = 0;//初始化蓝牙
    private static final int MSG_BT_ON = 1;//收到蓝牙打开状态
    private static final int MSG_BT_OFF = 2;//收到蓝牙关闭状态
    private static final int MSG_OPEN_BT = 3;//打开蓝牙
    private static final int MSG_CLOSE_BT = 4;//关闭蓝牙
    private static final int MSG_AUTO_CONNECT = 5;//蓝牙自动连接
    private static final int MSG_AUTO_CONNECT_NETWORK = 6;//蓝牙网络自动连接
    private static final int MSG_DISCONNECT_A2DP = 7;//断开A2DP

    private int mAutoConnectTimes = 0;

    @SuppressLint("HandlerLeak")
    private final Handler mHanlder = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            if (msg.what == MSG_BT_INIT) {
                int state = mLocalAdapter.getState();
                Log.d(TAG, "handleMessage: MSG_BT_INIT state=" + state);
                if (BluetoothAdapter.STATE_OFF == state) {
                    //蓝牙默认打开
                    if (getSavedBtStatus()) {
                        setBluetoothEnable(true, false);
                    }
                } else if (BluetoothAdapter.STATE_ON == state) {
                    //软件重启时，蓝牙可能已经是打开状态，补发状态广播给BluetoothEventManager
                    Intent it = new Intent(BluetoothAdapter.ACTION_STATE_CHANGED);
                    it.putExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.STATE_ON);
                    sendBroadcast(it);
                }
            } else if (msg.what == MSG_AUTO_CONNECT) {
                Log.d(TAG, "MSG_AutoConnect");
                if (!isBluetoothEnable()) {
                    Log.e(TAG, "MSG_AUTO_CONNECT Bluetooth is not enabled");
                    stopAutoConnect();
                    return;
                }
                String address = readLastConnectedDevice();
                if (TextUtils.isEmpty(address)) {
                    Log.e(TAG, "MSG_AUTO_CONNECT address is Empty");
                    stopAutoConnect();
                    return;
                }
                BluetoothDevice device = null;
                try {
                    device = mLocalAdapter.getRemoteDevice(address);
                } catch (IllegalArgumentException e) {
                    Log.e(TAG, "MSG_AUTO_CONNECT getRemoteDevice failed!!");
                    stopAutoConnect();
                    return;
                }
                if (null != mProfileManager) {
                    HeadsetClientProfile profile = mProfileManager.getHeadsetClientProfile();
                    if (null != profile) {
                        int state = profile.getConnectionStatus(device);
                        if (state == BluetoothProfile.STATE_CONNECTED) {
                            Log.e(TAG, "MSG_AUTO_CONNECT Bluetooth is already Connected!!");
                            stopAutoConnect();
                            return;
                        }
                    }
                }

                if (device.getBondState() == BluetoothDevice.BOND_BONDED) {
                    int delay;
                    if (mAutoConnectTimes < 12) {
                        delay = AUTO_CONNECT_DELAY * 2;
                    } else {
                        delay = AUTO_CONNECT_DELAY * 6;
                    }
                    Log.d(TAG, "MSG_AUTO_CONNECT times=" + mAutoConnectTimes + " delay:" + delay);
                    mHanlder.removeMessages(MSG_AUTO_CONNECT);
                    mHanlder.sendEmptyMessageDelayed(MSG_AUTO_CONNECT, delay);
                    mAutoConnectTimes++;
                    connectDevice(address);
                } else {
                    stopAutoConnect();
                    Log.e(TAG, "handleMessage AutoConnect device not bonded");
                }
            } else if (msg.what == MSG_BT_ON) {
                Log.d(TAG, "MSG_BTON");
                generalSetting();
                if (mDiscoveryDevices != null) {
                    mDiscoveryDevices.clear();
                }
                //延时1s开启自动连接，降低连接失败概率，蓝牙网络连接将在hfp连接后再尝试连接
                startAutoConnect(1000);
                //通知状态栏更新蓝牙网络图标
                notifySystemUINetworkState(isAutoConnectedNetwork());
            } else if (msg.what == MSG_BT_OFF) {
                Log.d(TAG, "MSG_BTOFF");
                if (mDiscoveryDevices != null) {
                    mDiscoveryDevices.clear();
                }
                //取消蓝牙自动连接
                stopAutoConnect();
                //取消蓝牙网络自动连接
                stopAutoConnectedNetwork();
                //通知状态栏隐藏蓝牙网络图标
                notifySystemUINetworkState(false);
            } else if (msg.what == MSG_OPEN_BT) {
                setBluetoothEnable(true, false);
            } else if (msg.what == MSG_CLOSE_BT) {
                BluetoothDeviceInfo device = getConnectDevice();
                if (null != device) {
                    disconnectDevice(device.getDeviceAddr());
                }
                setBluetoothEnable(false, false);
            } else if (msg.what == MSG_AUTO_CONNECT_NETWORK) {
                HeadsetClientProfile headsetClientProfile =
                        mProfileManager.getHeadsetClientProfile();
                PanProfile panProfile = mProfileManager.getPanProfile();
                if (null != headsetClientProfile) {
                    if (null != panProfile) {
                        List<BluetoothDevice> devices = headsetClientProfile.getConnectedDevices();
                        if (!devices.isEmpty()) {
                            BluetoothDevice device = devices.get(0);
                            int state = panProfile.getConnectionStatus(device);
                            if (state == BluetoothProfile.STATE_DISCONNECTED) {
                                Log.d(TAG, "Pan auto connect " + device.getAddress());
                                panProfile.connect(device);
                            } else if (state == BluetoothProfile.STATE_CONNECTED) {
                                if (panProfile.isBluetoothNetworkError()) {
                                    mNetworkErrorTimes++;
                                    if (mNetworkErrorTimes >= 2) {
                                        Log.e(TAG, "Pan error disconnect!");
                                        panProfile.disconnect(device);
                                        mNetworkErrorTimes = 0;
                                    }
                                } else {
                                    mNetworkErrorTimes = 0;
                                }
                            }
                        }
                    } else {
                        Log.e(TAG, "MSG_AUTO_CONNECT_NETWORK panProfile null");
                    }
                } else {
                    Log.e(TAG, "MSG_AUTO_CONNECT_NETWORK headsetClientProfile null");
                }

                startAutoConnectedNetwork(AUTO_NETWORK_CONNECT_DELAY);
            } else if (msg.what == MSG_DISCONNECT_A2DP) {
                disconnectA2dp();
            }
            super.handleMessage(msg);
        }

    };

    private final class BluetoothAdapterManagerBinder extends
            IBluetoothAdapterManagerService.Stub {

        @Override
        public List<BluetoothDeviceInfo> getDeviceList() {
            return BluetoothAdapterManagerService.this.getDeviceList();
        }

        @Override
        public List<BluetoothDeviceInfo> getBondedDevices() {
            return BluetoothAdapterManagerService.this.getBondedDevices();
        }

        @Override
        public BluetoothDeviceInfo getConnectDevice() {
            return BluetoothAdapterManagerService.this.getConnectDevice();
        }

        @Override
        public void pairDevice(String address) {
            BluetoothAdapterManagerService.this.pairDevice(address);
        }

        @Override
        public void unpairDevice(String address) {
            BluetoothAdapterManagerService.this.unpairDevice(address);
        }

        @Override
        public void connectDevice(String address) {
            // 手动连接其他设备时，延迟久一点再自动连接，避免刚断开上一个设备，又自动连接刚断开的设备
            if (BluetoothAdapterManagerService.this.isBluetoothAutoConnect()) {
                mHanlder.removeMessages(MSG_AUTO_CONNECT);
                Log.d(TAG, "connectDevice will auto connect after " + AUTO_CONNECT_DELAY * 4);
                mHanlder.sendEmptyMessageDelayed(MSG_AUTO_CONNECT, AUTO_CONNECT_DELAY * 4);
            }
            stopDiscovery();
            Log.d(TAG, "connectDevice stopDiscovery");
            BluetoothAdapterManagerService.this.connectDevice(address);
        }

        @Override
        public void disconnectDevice(String address) {
            if (BluetoothAdapterManagerService.this.isBluetoothConnected()) {
                mIsDisconnectByUser = true;
            }
            BluetoothAdapterManagerService.this.disconnectDevice(address);
        }

        @Override
        public boolean isBluetoothEnable() {
            return BluetoothAdapterManagerService.this.isBluetoothEnable();
        }

        @Override
        public boolean isBluetoothConnected() {
            return BluetoothAdapterManagerService.this.isBluetoothConnected();
        }

        @Override
        public void setBluetoothEnable(boolean enable) {
            BluetoothAdapterManagerService.this.setBluetoothEnable(enable, true);
        }

        @Override
        public String getBTName() {
            Log.d(TAG, "binder getBTName");
            return BluetoothAdapterManagerService.this.getBTName();
        }

        @Override
        public void setBTName(String name) {
            Log.d(TAG, "binder setBTName");
            BluetoothAdapterManagerService.this.setBTName(name, true);
        }

        @Override
        public String getBTPincode() {
            return BluetoothAdapterManagerService.this.getBTPincode();
        }

        @Override
        public void setBTPincode(String pincode) {
            BluetoothAdapterManagerService.this.setBTPincode(pincode);
        }

        @Override
        public boolean isBluetoothAutoAnswer() {
            return BluetoothAdapterManagerService.this.isBluetoothAutoAnswer();
        }

        @Override
        public void setBluetoothAutoAnswer(boolean enable) {
            BluetoothAdapterManagerService.this.setBluetoothAutoAnswer(enable);
        }

        @Override
        public boolean isBluetoothAutoConnect() {
            return BluetoothAdapterManagerService.this.isBluetoothAutoConnect();
        }

        @Override
        public void setBluetoothAutoConnect(boolean enable) {
            BluetoothAdapterManagerService.this.setBluetoothAutoConnect(enable);
        }

        @Override
        public void resetBT() {
            //手动reset时重置状态，第三方广播不可靠时，可以通过reset恢复正常
            mThirdPartMode = THIRD_PART_NORMAL_MODE;
            BluetoothAdapterManagerService.this.resetBT();
        }

        @Override
        public void startDiscovery() {
            // TODO Auto-generated method stub
            BluetoothAdapterManagerService.this.startDiscovery();
        }

        @Override
        public void stopDiscovery() {
            BluetoothAdapterManagerService.this.stopDiscovery();
        }

        @Override
        public boolean isDiscovering() {
            return BluetoothAdapterManagerService.this.isDiscovering();
        }

        public void setAutoConnectedNetwork(boolean auto) {
            BluetoothAdapterManagerService.this.setAutoConnectedNetwork(auto);
        }

        public boolean isAutoConnectedNetwork() {
            return BluetoothAdapterManagerService.this.isAutoConnectedNetwork();
        }

        public void connectA2dp(String address) {
            BluetoothAdapterManagerService.this.connectA2dp(address);
        }

        public void disconnectA2dp(String address) {
            BluetoothAdapterManagerService.this.disconnectA2dp(address);
        }

        public boolean isA2dpConnected(String address) {
            return BluetoothAdapterManagerService.this.isA2dpConnected(address);
        }

        public boolean isPbapConnected() {
            return BluetoothAdapterManagerService.this.mBluetoothPbapClientHelper.isPbapConnected();
        }

        public void connectPbap() {
            BluetoothAdapterManagerService.this.mBluetoothPbapClientHelper.connectPbap();
        }

        public void disConnectPbap() {
            BluetoothAdapterManagerService.this.mBluetoothPbapClientHelper.disConnectPbap();
        }

        public boolean pbapStartDownLoad(int type) {
            return BluetoothAdapterManagerService.this.mBluetoothPbapClientHelper.pbapStartDownLoad(type);
        }

        public boolean getPbapDownLoadState(int type) {
            return BluetoothAdapterManagerService.this.mBluetoothPbapClientHelper.getPbapDownLoadState(type);
        }

        public void registerPbapCallback(IPbapCallback callback) {
            BluetoothAdapterManagerService.this.mBluetoothPbapClientHelper.registerPbapCallback(callback);
        }

        public void unregisterPbapCallback(IPbapCallback callback) {
            BluetoothAdapterManagerService.this.mBluetoothPbapClientHelper.unregisterPbapCallback(callback);
        }

        public void registerCallback(IAdapterCallback callback) {
            BluetoothAdapterManagerService.this.registerCallback(callback);
        }

        public void unregisterCallback(IAdapterCallback callback) {
            BluetoothAdapterManagerService.this.unregisterCallback(callback);
        }

        public int getThirdPartAPPMode() {
            return BluetoothAdapterManagerService.this.getThirdPartAPPMode();
        }
    }


    private void checkAAModeChange(Intent intent){
        String type = intent.getStringExtra(LinkInterface.EXTRA_TYPE);
        Log.d(TAG, "checkAAModeChange type=" + type);
        if(LinkInterface.DATA_TYPE_AUTO.equals(type)){
            String state = intent.getStringExtra(LinkInterface.EXTRA_CONNECT_STATE);
            if(LinkInterface.DATA_STATUS_CONNECTED.equals(state)){
                Log.d(TAG, "checkAAModeChange type=" + type);
                CacheData.getInstance().setAndroidAutoMode(true);
                BluetoothHfpclientService hfpService = BluetoothHfpclientService.getInstance();
                if (null != hfpService) {
                    hfpService.onAAConenctStateChange(true);
                }
            }else if(LinkInterface.DATA_STATUS_DISCONNECTED.equals(state)){
                Log.d(TAG, "checkAAModeChange type=" + type);
                CacheData.getInstance().setAndroidAutoMode(false);
                BluetoothHfpclientService hfpService = BluetoothHfpclientService.getInstance();
                if (null != hfpService) {
                    hfpService.onAAConenctStateChange(false);
                }
            }
        }
    }



}
	

