package com.autochips.bluetooth.fragment;

import static com.hcn.bluetooth.api.BluetoothDeviceInfo.DeviceStatus.DEVICE_STATUS_UNKOWN;
import static com.hcn.bluetooth.api.LocalBluetoothAdapterManager.THIRD_PART_ZJ_CARPLAY_MODE;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;
import android.util.ArrayMap;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.ProgressBar;
import android.widget.RadioGroup;
import android.widget.SimpleAdapter;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;

import com.autochips.bluetooth.IFragmentCallback;
import com.autochips.bluetooth.MyApplication;
import com.autochips.bluetooth.R;
import com.autochips.bluetooth.skin.SkinUtils;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;
import com.hcn.bluetooth.api.ConnectionListener;
import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;
import com.hcn.bluetooth.api.Utils;
import com.hcn.bluetooth.service.IAdapterCallback;
import com.hcn.skin.support.app.SkinCompatFragment;

import java.util.ArrayList;
import java.util.List;

/**
 * 描述：弹框选择的配对界面  比如za01
 *
 * @author simon
 * @date 2023/3/22 20:03
 */
public class PairedFragmentEx extends SkinCompatFragment implements View.OnClickListener {
    private static final String TAG = "PairedFragmentEx";
    private static final String REMOTE_DEVICE_NAME = "remote_device_name";
    private static final String REMOTE_CONNECT_STATE = "remote_connect_status";
    private static final String REMOTE_DEVICE_MACADDR = "remote_device_macaddr";

    private LocalBluetoothAdapterManager mAdapterManager;
    private PairedHandler mPairedHandler;
    private View root;
    private Context mContext;

    /**
     * PairedHandler消息值定义
     */
    private static final int MSG_BLUETOOTH_STATE_CHANGE = 0x00;
    private static final int MSG_DISCOVERY_STATE_CHANGE = 0x01;
    private static final int MSG_DISCOVERY_DEVICE_FOUND = 0x02;
    private static final int MSG_DISCOVERY_DEVICE_NAME_CHANGE = 0x03;
    private static final int MSG_BOND_STATE_CHANGE = 0x04;
    private static final int MSG_CONNECT_STATE_CHANGE = 0x05;
    private static final int MSG_UPDATE_FRAGMENT = 0x06;

    /**
     * 配对列表ListAdapter
     */
    private PairedListAdapter mBluetoothUnpairedDevicesAdapter;
    private ArrayList<ArrayMap<String, Object>> mBluetoothUnpairedDevices =
            new ArrayList<ArrayMap<String, Object>>();

    private PairedListAdapter mBluetoothPairedDevicesAdapter;
    private ArrayList<ArrayMap<String, Object>> mBluetoothPairedDevices =
            new ArrayList<ArrayMap<String, Object>>();

    /**
     * 搜索等待框
     */
    LinearLayout scanLayout;
    private TextView mDeviceName;
    private TextView mConnectState;

    /**
     * 手机音频和媒体音频弹框
     */
    private PopupWindow mDeviceProfilePopupWindow = null;
    private View mAudioPopupView = null;
    private CheckBox hfpSwitchBtn;
    private CheckBox a2dpSwitchBtn;
    private ArrayMap<String, Object> mConfigDeviceMap;
    private TextView mPairDeviceName;
    /**
     * 已配对设备连接弹框
     */
    private PopupWindow popupWindow;
    private View mPairedPopupView = null;
    private RadioGroup pairedRadioGroup = null;

    /**
     * 通知Activity更新背景
     */
    private IFragmentCallback fragmentCallback = null;

    /**
     * 记录当前连接设备的MacAdder
     */
    private String strMacAdder;

    /**
     * 记录当前正在绑定的设备地址
     */
    private String mBoningMacAddress;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mPairedHandler = new PairedHandler(Looper.getMainLooper());
        mAdapterManager.registerCallback(mIAdapterCallback);
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");
        root = super.onCreateView(inflater, container, savedInstanceState);

        initPairedDevicesAdapter();
        initUnpairedDevicesAdapter();
        initView();
        return root;
    }

    @Override
    public int getLayoutRes() {
        return R.layout.bt_pairedhistory;
    }

    @Override
    public void onBindViewData() {

    }

    @Override
    public void onAttach(Context context) {
        Log.d(TAG, "onAttach");
        super.onAttach(context);
        mAdapterManager = MyApplication.getInstance().getAdapterManager();
        mAdapterManager.addConnectListener(mAdapterListener);
        if (SkinUtils.useSkinPackage()) {
            mContext = SkinUtils.getContext();
        } else {
            mContext = context;
        }
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
    }

    @Override
    public void onResume() {
        Log.d(TAG, "onResume");
        super.onResume();
        mPairedHandler.removeMessages(MSG_UPDATE_FRAGMENT);
        mPairedHandler.sendEmptyMessage(MSG_UPDATE_FRAGMENT);
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause");
        super.onPause();
    }

    @Override
    public void onStop() {
        Log.d(TAG, "onStop");
        super.onStop();
    }

    @Override
    public void onDestroyView() {
        Log.d(TAG, "onDestroyView");
        super.onDestroyView();
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();
        mAdapterManager.unregisterCallback(mIAdapterCallback);
        mPairedHandler.removeCallbacksAndMessages(null);
    }

    @Override
    public void onDetach() {
        Log.d(TAG, "onDetach");
        super.onDetach();
        mAdapterManager.removeConnectListener(mAdapterListener);
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View view) {
        if (THIRD_PART_ZJ_CARPLAY_MODE == mAdapterManager.getThirdPartAPPMode()) {
            Utils.showToast(mContext, SkinUtils.getString(R.string.str_zlink_mode));
            return;
        }
        int viewId = SkinUtils.getViewId(view);
        switch (viewId) {
            case R.id.btn_scan_bt:
                if (mAdapterManager.isDiscovering()) {
                    mAdapterManager.stopDiscovery();
                } else {
                    mAdapterManager.startDiscovery();
                }
                break;
            case R.id.btn_config_profile:
                mConfigDeviceMap = (ArrayMap<String, Object>) view.getTag();
                showDeviceProfilePopupDialog();
                break;
            case R.id.bt_paired_disconnect:
                onClickDisconnect();
                hidePairedOperationDialog();
                break;
            case R.id.bt_paired_connect:
                onClickConnect();
                hidePairedOperationDialog();
                break;
            case R.id.bt_paired_delete:
                onClickUnpair();
                hidePairedOperationDialog();
                break;
            case R.id.audio_profile_select_confirm:
                onConfigDeviceProfile();
                hideDeviceProfilePopupDialog();
                break;
            case R.id.audio_profile_select_cancel:
                hideDeviceProfilePopupDialog();
                break;
            default:
                break;
        }
    }

    public void setFragmentCallback(IFragmentCallback callback) {
        fragmentCallback = callback;
    }

    /**
     * 显示配对弹窗
     */
    public void showPairedOperationDialog() {
        if (popupWindow == null) {
            mPairedPopupView = LayoutInflater.from(mContext).inflate(SkinUtils.getId(R.layout.pair_list_operation_dialog), null);
            popupWindow = new PopupWindow(mPairedPopupView, ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);

            pairedRadioGroup = mPairedPopupView.findViewById(SkinUtils.getId(R.id.bt_paired_operation_radio_group));
            View disconnectButton = mPairedPopupView.findViewById(SkinUtils.getId(R.id.bt_paired_disconnect));
            View connectButton = mPairedPopupView.findViewById(SkinUtils.getId(R.id.bt_paired_connect));
            View unPairedButton = mPairedPopupView.findViewById(SkinUtils.getId(R.id.bt_paired_delete));

            if (unPairedButton != null) {
                unPairedButton.setOnClickListener(this);
            }
            if (connectButton != null) {
                connectButton.setOnClickListener(this);
            }
            if (disconnectButton != null) {
                disconnectButton.setOnClickListener(this);
            }
        }
        Log.e(TAG, "PairedFragment2=" + PairedFragmentEx.this.hashCode());
        if (fragmentCallback != null) {
            fragmentCallback.updateBackground(true);
        } else {
            Log.e(TAG, "fragmentCallback is null");
        }

        if (pairedRadioGroup != null) {
            pairedRadioGroup.clearCheck();
        }

        popupWindow.setOutsideTouchable(true);
        popupWindow.setFocusable(true);
        popupWindow.setAnimationStyle(R.style.PopupAnimation);
        popupWindow.showAtLocation(root, Gravity.CENTER, 0, SkinUtils.getInteger(R.integer.popupWindow_show_marginY));
        popupWindow.setOnDismissListener(new PopupWindow.OnDismissListener() {
            @Override
            public void onDismiss() {
                if (fragmentCallback != null) {
                    fragmentCallback.updateBackground(false);
                }
            }
        });
    }

    /**
     * 隐藏配对弹窗
     */
    public void hidePairedOperationDialog() {
        if (popupWindow != null) {
            popupWindow.dismiss();
        }
    }

    /**
     * 发起连接
     */
    private void onClickConnect() {
        if (mAdapterManager == null) {
            return;
        }

        String deviceAddress = mBluetoothUnpairedDevicesAdapter.getSelect();
        if (deviceAddress.length() == 0) {
            deviceAddress = mBluetoothPairedDevicesAdapter.getSelect();
        }
        if (deviceAddress.length() == 0) {
            return;
        }
        mAdapterManager.connectDevice(deviceAddress);
    }

    /**
     * 断开连接
     */
    private void onClickDisconnect() {
        if (mAdapterManager == null) {
            return;
        }

        String deviceAddress = mBluetoothPairedDevicesAdapter.getSelect();
        if (deviceAddress.length() == 0) {
            return;
        }
        mAdapterManager.disconnectDevice(deviceAddress);
    }

    /**
     * 取消配对(删除)
     */
    private void onClickUnpair() {
        if (mAdapterManager == null) {
            return;
        }

        String deviceAddress = mBluetoothPairedDevicesAdapter.getSelect();
        if (deviceAddress.length() > 0) {
            mAdapterManager.unpairDevice(deviceAddress);
        }
    }

    /**
     * 初始化控件
     */
    private void initView() {
        scanLayout = root.findViewById(SkinUtils.getId(R.id.scan_device_layout));

        mDeviceName = root.findViewById(SkinUtils.getId(R.id.bt_device_connected));
        mConnectState = root.findViewById(SkinUtils.getId(R.id.bt_device));

        View scanButton = root.findViewById(SkinUtils.getId(R.id.btn_scan_bt));
        if (scanButton != null) {
            scanButton.setOnClickListener(this);
        }
    }

    /**
     * 构建未配对设备列表的Adapter
     */
    private void initUnpairedDevicesAdapter() {
        mBluetoothUnpairedDevicesAdapter = new PairedListAdapter(mContext,
                mBluetoothUnpairedDevices, SkinUtils.getId(R.layout.device_listitem),
                new String[]{REMOTE_DEVICE_NAME, REMOTE_CONNECT_STATE,
                        REMOTE_DEVICE_MACADDR}, new int[]{
                SkinUtils.getId(R.id.item_remote_device_name),
                SkinUtils.getId(R.id.item_remote_connect_status),
                SkinUtils.getId(R.id.item_remote_device_macaddr)});

        ListView unpairedDeviceListView = root.findViewById(SkinUtils.getId(R.id.bluetooth_usable_devices));
        if (unpairedDeviceListView != null) {
            unpairedDeviceListView.setAdapter(mBluetoothUnpairedDevicesAdapter);
            unpairedDeviceListView.setOnItemClickListener(mUnpairedClickListener);
            unpairedDeviceListView.setEnabled(true);
        }
    }

    /**
     * 构建配对设备列表的Adapter
     */
    private void initPairedDevicesAdapter() {
        mBluetoothPairedDevicesAdapter = new PairedListAdapter(mContext,
                mBluetoothPairedDevices, SkinUtils.getId(R.layout.device_listitem),
                new String[]{REMOTE_DEVICE_NAME, REMOTE_CONNECT_STATE,
                        REMOTE_DEVICE_MACADDR}, new int[]{
                SkinUtils.getId(R.id.item_remote_device_name),
                SkinUtils.getId(R.id.item_remote_connect_status),
                SkinUtils.getId(R.id.item_remote_device_macaddr)});

        ListView pairedDeviceListView = root.findViewById(SkinUtils.getId(R.id.bluetooth_paired_devices));
        if (pairedDeviceListView != null) {
            pairedDeviceListView.setAdapter(mBluetoothPairedDevicesAdapter);
            pairedDeviceListView.setOnItemClickListener(mPairedClickListener);
            pairedDeviceListView.setEnabled(true);
        }
    }

    /**
     * 可用设备列表点击监听
     */
    private AdapterView.OnItemClickListener
            mUnpairedClickListener = new AdapterView.OnItemClickListener() {

        @Override
        public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
                                long arg3) {
            if (mBluetoothUnpairedDevicesAdapter.getPairedStatus()) {
                Log.d(TAG, " Pairing in progress!!!");
                return;
            }
            String address = (String) mBluetoothUnpairedDevicesAdapter.getItem(arg2).get(
                    REMOTE_DEVICE_MACADDR);
            mBluetoothUnpairedDevicesAdapter.setSelect(address);
            mBluetoothUnpairedDevicesAdapter.notifyDataSetChanged();
            mBluetoothPairedDevicesAdapter.setSelect("");
            mBluetoothPairedDevicesAdapter.notifyDataSetChanged();
            Log.d(TAG, "connectDevice = " + address);
            mAdapterManager.connectDevice(address);
        }
    };

    /**
     * 配对设备列表点击监听
     */
    private AdapterView.OnItemClickListener
            mPairedClickListener = new AdapterView.OnItemClickListener() {

        @Override
        public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
            // TODO Auto-generated method stub
            mBluetoothUnpairedDevicesAdapter.setSelect("");
            mBluetoothUnpairedDevicesAdapter.notifyDataSetChanged();
            String address = (String) mBluetoothPairedDevicesAdapter.getItem(arg2).get(
                    REMOTE_DEVICE_MACADDR);
            mBluetoothPairedDevicesAdapter.setSelect(address);
            mBluetoothPairedDevicesAdapter.notifyDataSetChanged();
            showPairedOperationDialog();
        }
    };

    /**
     * 显示手机音频/媒体音频功能勾选弹窗
     */
    public void showDeviceProfilePopupDialog() {
        if (mDeviceProfilePopupWindow == null) {
            mAudioPopupView = LayoutInflater.from(mContext).inflate(SkinUtils.getId(R.layout.pair_audio_profile_dialog), null);
            mDeviceProfilePopupWindow = new PopupWindow(mAudioPopupView, ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);
            mDeviceProfilePopupWindow.setOutsideTouchable(true);
            mDeviceProfilePopupWindow.setFocusable(true);
            mDeviceProfilePopupWindow.setAnimationStyle(R.style.PopupAnimation);
            mDeviceProfilePopupWindow.setOnDismissListener(new PopupWindow.OnDismissListener() {
                @Override
                public void onDismiss() {
                    if (fragmentCallback != null) {
                        fragmentCallback.updateBackground(false);
                    }
                }
            });

            mPairDeviceName = mAudioPopupView.findViewById(SkinUtils.getId(R.id.bt_pair_device_name));
            hfpSwitchBtn = mAudioPopupView.findViewById(SkinUtils.getId(R.id.cb_hfp_profile));
            a2dpSwitchBtn = mAudioPopupView.findViewById(SkinUtils.getId(R.id.cb_a2dp_profile));
            View audioConfirmButton = mAudioPopupView.findViewById(SkinUtils.getId(R.id.audio_profile_select_confirm));
            View audioCancelButton = mAudioPopupView.findViewById(SkinUtils.getId(R.id.audio_profile_select_cancel));
            if (audioConfirmButton != null) {
                audioConfirmButton.setOnClickListener(this);
            }
            if (audioCancelButton != null) {
                audioCancelButton.setOnClickListener(this);
            }
        }

        if (mConfigDeviceMap == null) {
            Log.e(TAG, "showDeviceProfile mConfigDeviceMap is null !");
            return;
        }
        String deviceName = (String) mConfigDeviceMap.get(REMOTE_DEVICE_NAME);
        String deviceAddress = (String) mConfigDeviceMap.get(REMOTE_DEVICE_MACADDR);
        BluetoothDeviceInfo connectDevice = mAdapterManager.getConnectDevice();
        if (mPairDeviceName != null) {
            mPairDeviceName.setText(deviceName);
        }
        if (hfpSwitchBtn != null) {
            if (null != connectDevice && connectDevice.getDeviceAddr().equals(deviceAddress)) {
                hfpSwitchBtn.setChecked(true);
            } else {
                hfpSwitchBtn.setChecked(false);
            }
        }
        if (a2dpSwitchBtn != null) {
            if (mAdapterManager.isA2dpConnected(deviceAddress)) {
                a2dpSwitchBtn.setChecked(true);
            } else {
                a2dpSwitchBtn.setChecked(false);
            }
        }
        if (fragmentCallback != null) {
            fragmentCallback.updateBackground(true);
        } else {
            Log.e(TAG, "fragmentCallback is null");
        }
        mDeviceProfilePopupWindow.showAtLocation(root, Gravity.CENTER, 0, SkinUtils.getInteger(R.integer.popupWindow_show_marginY));
    }

    /**
     * 隐藏手机音频/媒体音频功能勾选弹窗
     */
    public void hideDeviceProfilePopupDialog() {
        if (mDeviceProfilePopupWindow != null) {
            mDeviceProfilePopupWindow.dismiss();
        }
    }

    /**
     * 连接/断开音频相关协议
     */
    private void onConfigDeviceProfile() {
        if (mConfigDeviceMap == null) {
            Log.e(TAG, "onConfigDeviceProfile mConfigDeviceMap is null !");
            return;
        }

        boolean needHfp = false;
        boolean needA2dp = false;

        if (hfpSwitchBtn != null && hfpSwitchBtn.isChecked()) {
            needHfp = true;
        }

        if (a2dpSwitchBtn != null && a2dpSwitchBtn.isChecked()) {
            needA2dp = true;
        }

        String deviceAdder = (String) mConfigDeviceMap.get(REMOTE_DEVICE_MACADDR);
        BluetoothDeviceInfo connectDevice = mAdapterManager.getConnectDevice();
        boolean isHfpConnected;
        if (null != connectDevice && connectDevice.getDeviceAddr().equals(deviceAdder)) {
            isHfpConnected = true;
        } else {
            isHfpConnected = false;
        }

        if (needHfp && !isHfpConnected) {
            mAdapterManager.connectDevice(deviceAdder);
        } else if (!needHfp && isHfpConnected) {
            mAdapterManager.disconnectDevice(deviceAdder);
        }

        boolean isA2dpConnected = mAdapterManager.isA2dpConnected(deviceAdder);
        if (needA2dp && isHfpConnected && !isA2dpConnected) {
            mAdapterManager.connectA2dp(deviceAdder);
        } else if (!needA2dp && isA2dpConnected) {
            mAdapterManager.disconnectA2dp(deviceAdder);
        }
    }

    private ConnectionListener mAdapterListener = new ConnectionListener() {
        @Override
        public void onServiceConnected() {
            Log.d(TAG, "onServiceConnected");
            mAdapterManager.registerCallback(mIAdapterCallback);
            mPairedHandler.removeMessages(MSG_UPDATE_FRAGMENT);
            mPairedHandler.sendEmptyMessage(MSG_UPDATE_FRAGMENT);
        }

        @Override
        public void onServiceDisconnected() {
            Log.d(TAG, "onServiceDisconnected");
        }
    };

    private class PairedHandler extends Handler {
        public PairedHandler(@NonNull Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_BLUETOOTH_STATE_CHANGE:
                    int bt_state = msg.arg1;
                    Log.d(TAG, "MSG_BLUETOOTH_STATE_CHANGE " + bt_state);
                    if (bt_state == BluetoothAdapter.STATE_TURNING_OFF) {
                        if (mBoningMacAddress != null && mBoningMacAddress.length() > 0 &&
                                mBoningMacAddress.equals(mBluetoothUnpairedDevicesAdapter.getSelect())) {
                            Log.d(TAG, mBoningMacAddress + " setPairedStatus false");
                            mBluetoothUnpairedDevicesAdapter.setPairedStatus(false);
                        }
                    }
                    removeMessages(MSG_UPDATE_FRAGMENT);
                    sendEmptyMessageDelayed(MSG_UPDATE_FRAGMENT, 100);
                    break;
                case MSG_DISCOVERY_STATE_CHANGE:
                    if (msg.arg1 == 0x01) {//start
                        Log.d(TAG, "MSG_DISCOVERY_STATE_CHANGE start");
                        onActionDiscoveryStarted();
                        updateUnpairedDevicesList();
                    } else {
                        Log.d(TAG, "MSG_DISCOVERY_STATE_CHANGE end");
                        updateUnpairedDevicesList();
                        onActionDiscoveryFinished();
                    }
                    break;
                case MSG_DISCOVERY_DEVICE_FOUND:
                    updateUnpairedDevicesList();
                    break;
                case MSG_DISCOVERY_DEVICE_NAME_CHANGE:
                    updateUnpairedDevicesList();
                    break;
                case MSG_BOND_STATE_CHANGE:
                    BluetoothDeviceInfo device = (BluetoothDeviceInfo) msg.obj;
                    Log.d(TAG, "MSG_BOND_STATE_CHANGE name=" + device.getDeviceName() + " status=" + device.getDeviceStatus());
                    if (device.getDeviceStatus() == BluetoothDevice.BOND_BONDED) {
                        mBluetoothPairedDevicesAdapter.setSelect(device.getDeviceAddr());
                        mBluetoothUnpairedDevicesAdapter.setSelect("");
                        mBluetoothUnpairedDevicesAdapter.setPairedStatus(false);
                        mBoningMacAddress = "";
                    } else if (device.getDeviceStatus() == BluetoothDevice.BOND_NONE) {
                        mBluetoothPairedDevicesAdapter.setSelect("");
                        mBluetoothUnpairedDevicesAdapter.setSelect(device.getDeviceAddr());
                        mBluetoothUnpairedDevicesAdapter.setPairedStatus(false);
                        mBoningMacAddress = device.getDeviceAddr();
                    } else if (device.getDeviceStatus() == BluetoothDevice.BOND_BONDING) {
                        mBluetoothUnpairedDevicesAdapter.setSelect(device.getDeviceAddr());
                        mBluetoothUnpairedDevicesAdapter.setPairedStatus(true);
                        mBoningMacAddress = device.getDeviceAddr();
                    }

                    updateBondedDevicesList();
                    updateUnpairedDevicesList();
                    break;
                case MSG_CONNECT_STATE_CHANGE:
                    Log.d(TAG, "MSG_CONNECT_STATE_CHANGE conn_state = " + msg.arg1);
                    BluetoothDeviceInfo conn_device = (BluetoothDeviceInfo) msg.obj;
                    int conn_state = msg.arg1;
                    String device_name = conn_device.getDeviceName();
                    if (null == device_name) {
                        device_name = "";
                    }
                    int resID;
                    if (conn_state == BluetoothProfile.STATE_CONNECTING) {
                        resID = SkinUtils.getId(R.string.connecting);
                    } else if (conn_state == BluetoothProfile.STATE_CONNECTED) {
                        resID = SkinUtils.getId(R.string.connect);
                        mBluetoothPairedDevicesAdapter.setSelect(conn_device.getDeviceAddr());
                        mBluetoothUnpairedDevicesAdapter.setSelect("");
                        mBluetoothUnpairedDevicesAdapter.setPairedStatus(false);
                        strMacAdder = conn_device.getDeviceAddr();
                        updateBondedDevicesList();
                        updateUnpairedDevicesList();
                    } else if (conn_state == BluetoothProfile.STATE_DISCONNECTING) {
                        resID = SkinUtils.getId(R.string.disconnect);
                    } else {
                        device_name = "";
                        strMacAdder = "";
                        resID = SkinUtils.getId(R.string.str_dicconnected);
                        mBluetoothPairedDevicesAdapter.setSelect("");
                        mBluetoothUnpairedDevicesAdapter.setSelect("");
                        updateBondedDevicesList();
                        updateUnpairedDevicesList();
                    }
                    updateConnectState(resID, device_name);
                    break;
                case MSG_UPDATE_FRAGMENT:
                    if (null != mAdapterManager) {
                        BluetoothDeviceInfo deviceInfo = mAdapterManager.getConnectDevice();
                        if (null != deviceInfo) {
                            strMacAdder = deviceInfo.getDeviceAddr();
                            mBluetoothPairedDevicesAdapter.setSelect(deviceInfo.getDeviceAddr());
                            mBluetoothUnpairedDevicesAdapter.setSelect("");
                            updateConnectState(SkinUtils.getId(R.string.connect), deviceInfo.getDeviceName());
                        } else {
                            strMacAdder = "";
                            updateConnectState(SkinUtils.getId(R.string.str_dicconnected), "");
                        }
                    }
                    updateUnpairedDevicesList();
                    updateBondedDevicesList();
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * 更新连接状态
     *
     * @param resid      连接状态字符串资源id
     * @param deviceName 设备名称
     */
    private void updateConnectState(@StringRes int resid, String deviceName) {
        if (mConnectState != null) {
            mConnectState.setText(resid);
        }
        if (mDeviceName != null) {
            mDeviceName.setText(deviceName);
        }
    }

    /**
     * 更新未配对设备(可用设备)列表
     */
    private void updateUnpairedDevicesList() {
        if (null != mAdapterManager) {
            mBluetoothUnpairedDevices.clear();
            List<BluetoothDeviceInfo> unpairedDeviceList = mAdapterManager.getDeviceList();
            if (unpairedDeviceList != null) {
                Log.d(TAG, "updateUnpairedDevicesList: UnpairedDevices size="
                        + unpairedDeviceList.size());
                for (BluetoothDeviceInfo device : unpairedDeviceList) {
                    ArrayMap<String, Object> map = new ArrayMap<>();
                    map.put(REMOTE_DEVICE_NAME, device.getDeviceName());
                    map.put(REMOTE_DEVICE_MACADDR, device.getDeviceAddr());
                    map.put(REMOTE_CONNECT_STATE, device.getDeviceStatus());
                    mBluetoothUnpairedDevices.add(map);
                }
            }
            mBluetoothUnpairedDevicesAdapter.notifyDataSetChanged();
        }
    }

    /**
     * 更新绑定设备(配对设备)列表
     */
    private void updateBondedDevicesList() {
        if (null != mAdapterManager) {
            mBluetoothPairedDevices.clear();
            List<BluetoothDeviceInfo> bondedDeviceList = mAdapterManager.getBondedDevices();
            if (null != bondedDeviceList) {
                Log.d(TAG, "updateBondedDevicesList: getBondedDevices size="
                        + bondedDeviceList.size());
                for (BluetoothDeviceInfo device : bondedDeviceList) {
                    ArrayMap<String, Object> map = new ArrayMap<>();
                    map.put(REMOTE_DEVICE_NAME, device.getDeviceName());
                    map.put(REMOTE_DEVICE_MACADDR, device.getDeviceAddr());
                    map.put(REMOTE_CONNECT_STATE, device.getDeviceStatus());
                    mBluetoothPairedDevices.add(map);
                }
            }
            mBluetoothPairedDevicesAdapter.notifyDataSetChanged();
        }
    }

    /**
     * 搜索设备开始，显示搜索弹框
     */
    private void onActionDiscoveryStarted() {
        if (scanLayout != null) {
            scanLayout.setVisibility(View.VISIBLE);
        }
    }

    /**
     * 搜索设备结束，隐藏搜索弹框
     */
    private void onActionDiscoveryFinished() {
        if (scanLayout != null) {
            scanLayout.setVisibility(View.GONE);
        }
    }

    /**
     * 配对列表ListAdapter
     */
    class PairedListAdapter extends SimpleAdapter {
        private LayoutInflater mInflater;
        private String mSelectMacAddress = "";
        List<ArrayMap<String, Object>> mDeviceList;
        private String str_hfp = null;
        private String str_hfp_connected = null;
        private boolean pairedStatus;

        public PairedListAdapter(Context context, List<ArrayMap<String, Object>> data,
                                 int resource, String[] from, int[] to) {
            super(context, data, resource, from, to);
            // TODO Auto-generated constructor stub
            this.mInflater = LayoutInflater.from(context);
            this.mSelectMacAddress = "";
            this.mDeviceList = data;
            str_hfp = SkinUtils.getString(R.string.str_hfp_label);
            str_hfp_connected = SkinUtils.getString(R.string.bt_status_connected);
        }

        public void setSelect(String address) {
            mSelectMacAddress = address;
        }

        public String getSelect() {
            return mSelectMacAddress;
        }

        public void setPairedStatus(boolean status) {
            pairedStatus = status;
        }

        public boolean getPairedStatus() {
            return pairedStatus;
        }

        @Override
        public ArrayMap<String, Object> getItem(int position) {
            return mDeviceList.get(position);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder = null;
            if (convertView == null) {
                //convertView = mInflater.inflate(R.layout.device_listitem, null);
                convertView = SkinUtils.inflate(R.layout.device_listitem);
                holder = new ViewHolder(convertView);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            String name = "";
            String macaddr = "";
            int state = DEVICE_STATUS_UNKOWN;

            if (position < mDeviceList.size()) {
                ArrayMap<String, Object> deviceMap = mDeviceList.get(position);
                name = (String) deviceMap.get(REMOTE_DEVICE_NAME);
                state = (int) deviceMap.get(REMOTE_CONNECT_STATE);
                macaddr = (String) deviceMap.get(REMOTE_DEVICE_MACADDR);

                if (holder.profileConfigImageButton != null) {
                    holder.profileConfigImageButton.setTag(deviceMap);
                    if (state == DEVICE_STATUS_UNKOWN) {
                        holder.profileConfigImageButton.setVisibility(View.INVISIBLE);
                    } else {
                        if (state != BluetoothDevice.BOND_BONDED) {
                            holder.profileConfigImageButton.setVisibility(View.INVISIBLE);
                        } else {
                            holder.profileConfigImageButton.setVisibility(View.VISIBLE);
                        }
                    }
                }
            }

            if (holder.profileConfigImageButton != null) {
                holder.profileConfigImageButton.setOnClickListener(PairedFragmentEx.this);
            }

            if (holder.nameTextView != null) {
                holder.nameTextView.setText(name);
            }

            if (mAdapterManager != null) {
                if (macaddr != null && macaddr.equals(strMacAdder)) {
                    if (holder.statusTextView != null) {
                        holder.statusTextView.setText(SkinUtils.getId(R.string.bt_status_connected));
                        holder.statusTextView.setTextColor(SkinUtils.getColor(R.color.color_paired_p));
                    }
                    if (holder.nameTextView != null) {
                        holder.nameTextView.setTextColor(SkinUtils.getColor(R.color.color_paired_p));
                    }
                } else {
                    if (holder.statusTextView != null) {
                        holder.statusTextView.setText("");
                        holder.statusTextView.setTextColor(SkinUtils.getColor(R.color.color_paired_n));
                    }
                    if (holder.nameTextView != null) {
                        holder.nameTextView.setTextColor(SkinUtils.getColor(R.color.color_paired_n));
                    }
                }
            }

            if (holder.progressBar != null) {
                if (mSelectMacAddress.length() > 0 && mSelectMacAddress.equals(macaddr)) {
                    Log.d(TAG, "mSelectMacAddress=" + mSelectMacAddress + " pairedStatus=" + pairedStatus);
                    if (pairedStatus) {
                        holder.progressBar.setVisibility(View.VISIBLE);
                    } else {
                        holder.progressBar.setVisibility(View.INVISIBLE);
                    }
                } else {
                    holder.progressBar.setVisibility(View.INVISIBLE);
                }
            }

            return convertView;
        }

        public final class ViewHolder {
            public View convertView;
            public TextView nameTextView;
            public TextView statusTextView;
            public Button profileConfigImageButton;
            public ProgressBar progressBar;

            public ViewHolder(View view) {
                convertView = view;
                if (convertView != null) {
                    nameTextView = convertView.findViewById(SkinUtils.getId(R.id.item_remote_device_name));
                    statusTextView = convertView.findViewById(SkinUtils.getId(R.id.item_remote_connect_status));
                    profileConfigImageButton = convertView.findViewById(SkinUtils.getId(R.id.btn_config_profile));
                    progressBar = convertView.findViewById(SkinUtils.getId(R.id.bt_pair_device_progressbar));
                    if ("za10".equals(SkinUtils.getCurrentSkinID()) && getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
                        nameTextView.setEllipsize(TextUtils.TruncateAt.MARQUEE);
                        nameTextView.setMarqueeRepeatLimit(-1);
                        nameTextView.setSingleLine(true);
                        nameTextView.setFocusable(true);
                        nameTextView.setFocusableInTouchMode(true);
                        nameTextView.setSelected(true);
                    }
                }
            }
        }
    }

    /**
     * 配对状态回调
     */
    private IAdapterCallback mIAdapterCallback = new IAdapterCallback.Stub() {

        @Override
        public void onBluetoothStateChanged(int state) {
            Message msg = Message.obtain(mPairedHandler, MSG_BLUETOOTH_STATE_CHANGE);
            msg.arg1 = state;
            mPairedHandler.sendMessage(msg);
        }

        @Override
        public void onDiscoveryStateChanged(int state) {
            Message msg = Message.obtain(mPairedHandler, MSG_DISCOVERY_STATE_CHANGE);
            msg.arg1 = state;
            mPairedHandler.sendMessage(msg);
        }

        @Override
        public void onDiscoveryDeviceFound(BluetoothDeviceInfo deviceInfo) {
            Message msg = Message.obtain(mPairedHandler, MSG_DISCOVERY_DEVICE_FOUND);
            msg.obj = deviceInfo;
            mPairedHandler.sendMessage(msg);
        }

        @Override
        public void onDiscoveryDeviceNameChanged(BluetoothDeviceInfo deviceInfo) {
            Message msg = Message.obtain(mPairedHandler, MSG_DISCOVERY_DEVICE_NAME_CHANGE);
            msg.obj = deviceInfo;
            mPairedHandler.sendMessage(msg);
        }

        @Override
        public void onDeviceBondStateChanged(BluetoothDeviceInfo deviceInfo, int state) {
            Message msg = Message.obtain(mPairedHandler, MSG_BOND_STATE_CHANGE);
            msg.obj = deviceInfo;
            mPairedHandler.sendMessage(msg);
        }

        @Override
        public void onConnectionStateChanged(BluetoothDeviceInfo deviceInfo, int state) {
            Message msg = Message.obtain(mPairedHandler, MSG_CONNECT_STATE_CHANGE);
            msg.arg1 = state;
            msg.obj = deviceInfo;
            mPairedHandler.sendMessage(msg);
        }
    };
}
