package com.autochips.bluetooth.fragment;

import static com.hcn.bluetooth.api.BluetoothDeviceInfo.DeviceStatus.DEVICE_STATUS_UNKOWN;
import static com.hcn.bluetooth.api.LocalBluetoothAdapterManager.THIRD_PART_ZJ_CARPLAY_MODE;

import android.app.AlertDialog;
import android.app.Dialog;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.ArrayMap;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StringRes;

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

public class PairedFragment extends SkinCompatFragment implements View.OnClickListener {
    private static final String TAG = "ParedFragment";
    private static final String REMOTE_DEVICE_NAME = "remote_device_name";
    private static final String REMOTE_CONNECT_STATE = "remote_connect_status";
    private static final String REMOTE_DEVICE_MACADDR = "remote_device_macaddr";


    private LocalBluetoothAdapterManager mAdapterManager;
    private ArrayList<ArrayMap<String, Object>> mBluetoothUnpairedDevices =
            new ArrayList<ArrayMap<String, Object>>();
    private ListAdapter mBluetoothUnpairedDevicesAdapter;
    private ArrayList<ArrayMap<String, Object>> mBluetoothPairedDevices =
            new ArrayList<ArrayMap<String, Object>>();
    private ListAdapter mBluetoothPairedDevicesAdapter;
    //搜索等待框
    LinearLayout scanLayout;
    private Button mDeviceName;
    private boolean isMarqueeOn = false;
    private TextView mConnectState;
    private Context mContext;


    private Dialog mDeviceProfileConfigDialog = null;
    //保存Dialog操作的设备信息
    private ArrayMap<String, Object> mConfigDeviceMap;
    private CheckBox hfpSwitchBtn;
    private CheckBox a2dpSwitchBtn;
    private View root;
    private MainHandler mMainHandler;
    private static final int MSG_BLUETOOTH_STATE_CHANGE = 0x00;
    private static final int MSG_DISCOVERY_STATE_CHANGE = 0x01;
    private static final int MSG_DISCOVERY_DEVICE_FOUND = 0x02;
    private static final int MSG_DISCOVERY_DEVICE_NAME_CHANGE = 0x03;
    private static final int MSG_BOND_STATE_CHANGE = 0x04;
    private static final int MSG_CONNECT_STATE_CHANGE = 0x05;
    private static final int MSG_UPDATE_FRAGMENT = 0x06;

    private class MainHandler extends Handler {
        public MainHandler(@NonNull Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_BLUETOOTH_STATE_CHANGE:
                    int bt_state = msg.arg1;
                    Log.d(TAG, "MSG_BLUETOOTH_STATE_CHANGE " + bt_state);
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
                    } else if (device.getDeviceStatus() == BluetoothDevice.BOND_NONE) {
                        mBluetoothPairedDevicesAdapter.setSelect("");
                        mBluetoothUnpairedDevicesAdapter.setSelect(device.getDeviceAddr());
                    }

                    updateBondedDevicesList();
                    updateUnpairedDevicesList();
                    break;
                case MSG_CONNECT_STATE_CHANGE:
                    Log.d(TAG, "MSG_CONNECT_STATE_CHANGE");
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
                        updateBondedDevicesList();
                        updateUnpairedDevicesList();
                    } else if (conn_state == BluetoothProfile.STATE_DISCONNECTING) {
                        resID = SkinUtils.getId(R.string.disconnect);
                    } else {
                        device_name = "";
                        resID = SkinUtils.getId(R.string.str_dicconnected);
                    }
                    updateConnectState(resID, device_name);
                    break;
                case MSG_UPDATE_FRAGMENT:
                    if (null != mAdapterManager) {
                        BluetoothDeviceInfo deviceInfo = mAdapterManager.getConnectDevice();
                        if (null != deviceInfo) {
                            mBluetoothPairedDevicesAdapter.setSelect(deviceInfo.getDeviceAddr());
                            mBluetoothUnpairedDevicesAdapter.setSelect("");
                            updateConnectState(SkinUtils.getId(R.string.connect), deviceInfo.getDeviceName());
                        } else {
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

    @Override
    public void onHiddenChanged(boolean hidden) {
        Log.d(TAG, "onHiddenChanged: ");
        super.onHiddenChanged(hidden);
    }

    @Override
    public void onAttach(Context context) {
        Log.d(TAG, "onAttach: ");
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
    public void onCreate(@Nullable Bundle savedInstanceState) {
        Log.d(TAG, "onCreate: ");
        super.onCreate(savedInstanceState);
        mMainHandler = new MainHandler(Looper.getMainLooper());
        mAdapterManager.registerCallback(mIAdapterCallback);
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");
        // View root = inflater.inflate(R.layout.bt_pairedhistory, container, false);
        root = super.onCreateView(inflater, container, savedInstanceState);

        mBluetoothUnpairedDevicesAdapter = new ListAdapter(mContext,
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

        mBluetoothPairedDevicesAdapter = new ListAdapter(mContext,
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
        scanLayout = root.findViewById(SkinUtils.getId(R.id.scan_device_layout));

        mDeviceName = root.findViewById(SkinUtils.getId(R.id.bt_device_connected));
        mConnectState = root.findViewById(SkinUtils.getId(R.id.bt_device));

        View scanButton = root.findViewById(SkinUtils.getId(R.id.btn_scan_bt));
        View unPairedButton = root.findViewById(SkinUtils.getId(R.id.btn_unpair_bt));
        View connectButton = root.findViewById(SkinUtils.getId(R.id.btn_connect_bt));
        View disconnectButton = root.findViewById(SkinUtils.getId(R.id.btn_disconnect_bt));
        if (scanButton != null) {
            scanButton.setOnClickListener(this);
        }
        if (unPairedButton != null) {
            unPairedButton.setOnClickListener(this);
        }
        if (connectButton != null) {
            connectButton.setOnClickListener(this);
        }
        if (disconnectButton != null) {
            disconnectButton.setOnClickListener(this);
        }
        buildDeviceProfileConfigDialog();
        return root;
    }

    @Override
    public void onBindViewData() {

    }

    @Override
    public int getLayoutRes() {
        return R.layout.bt_pairedhistory;
    }

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
            case R.id.btn_unpair_bt:
                onClickUnpair();
                break;
            case R.id.btn_connect_bt:
                onClickConnect();
                break;
            case R.id.btn_disconnect_bt:
                onClickDisconnect();
                break;
            case R.id.btn_config_profile:
                mConfigDeviceMap = (ArrayMap<String, Object>) view.getTag();
                showDeviceProfileConfigDialog();
                break;
            default:
                break;
        }
    }

    private void onClickConnect() {
        if (mAdapterManager == null) {
            return;
        }

        String deviceAddr = mBluetoothUnpairedDevicesAdapter.getSelect();
        if (deviceAddr.length() == 0) {
            deviceAddr = mBluetoothPairedDevicesAdapter.getSelect();
        }
        if (deviceAddr.length() == 0) {
            return;
        }
        mAdapterManager.connectDevice(deviceAddr);
    }

    private void onClickDisconnect() {
        if (mAdapterManager == null) {
            return;
        }

        String deviceAddr = mBluetoothPairedDevicesAdapter.getSelect();
        if (deviceAddr.length() == 0) {
            return;
        }
        mAdapterManager.disconnectDevice(deviceAddr);
    }

    private void onClickUnpair() {
        if (mAdapterManager == null) {
            return;
        }

        String deviceAddr = mBluetoothPairedDevicesAdapter.getSelect();
        if (deviceAddr.length() > 0) {
            mAdapterManager.unpairDevice(deviceAddr);
        }
    }

    private AdapterView.OnItemClickListener
            mUnpairedClickListener = new AdapterView.OnItemClickListener() {

        @Override
        public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
                                long arg3) {
            String addr = (String) mBluetoothUnpairedDevicesAdapter.getItem(arg2).get(
                    REMOTE_DEVICE_MACADDR);
            mBluetoothUnpairedDevicesAdapter.setSelect(addr);
            mBluetoothUnpairedDevicesAdapter.notifyDataSetChanged();
            mBluetoothPairedDevicesAdapter.setSelect("");
            mBluetoothPairedDevicesAdapter.notifyDataSetChanged();

        }
    };

    private AdapterView.OnItemClickListener
            mPairedClickListener = new AdapterView.OnItemClickListener() {

        @Override
        public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
            // TODO Auto-generated method stub
            mBluetoothUnpairedDevicesAdapter.setSelect("");
            mBluetoothUnpairedDevicesAdapter.notifyDataSetChanged();
            String addr = (String) mBluetoothPairedDevicesAdapter.getItem(arg2).get(
                    REMOTE_DEVICE_MACADDR);
            mBluetoothPairedDevicesAdapter.setSelect(addr);
            mBluetoothPairedDevicesAdapter.notifyDataSetChanged();
        }
    };

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        Log.d(TAG, "onViewCreated: ");
        super.onViewCreated(view, savedInstanceState);
    }

    @Override
    public void onStart() {
        super.onStart();
        Log.d(TAG, "onStart: ");
    }

    private void onActionDiscoveryStarted() {
        if (scanLayout != null) {
            scanLayout.setVisibility(View.VISIBLE);
        }
    }

    private void onActionDiscoveryFinished() {
        if (scanLayout != null) {
            scanLayout.setVisibility(View.GONE);
        }
    }

    /**
     * 更新连接状态
     *
     * @param resid      连接状态字符串资源id
     * @param deviceName 设备名称
     */
    private void updateConnectState(@StringRes int resid, String deviceName) {
        mConnectState.setText(resid);
        mDeviceName.setText(deviceName);
        mDeviceName.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!isMarqueeOn) {
                    // 开启走马灯效果
                    mDeviceName.setEllipsize(TextUtils.TruncateAt.MARQUEE);
                    mDeviceName.setMarqueeRepeatLimit(-1); // 无限循环
                    mDeviceName.setSelected(true);
                    isMarqueeOn = true;
                } else {
                    // 停止走马灯效果
                    mDeviceName.setEllipsize(null);
                    mDeviceName.setMarqueeRepeatLimit(0);
                    mDeviceName.setSelected(false);
                    isMarqueeOn = false;
                }
            }
        });

    }

    private void updateUnpairedDevicesList() {
        if (null != mAdapterManager) {
            mBluetoothUnpairedDevices.clear();
            List<BluetoothDeviceInfo> unpairedDeviceList = mAdapterManager.getDeviceList();
            if (unpairedDeviceList != null) {
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

    @Override
    public void onResume() {
        Log.d(TAG, "onResume: ");
        super.onResume();
        mMainHandler.removeMessages(MSG_UPDATE_FRAGMENT);
        mMainHandler.sendEmptyMessage(MSG_UPDATE_FRAGMENT);
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause: ");
        super.onPause();
    }

    @Override
    public void onStop() {
        Log.d(TAG, "onStop: ");
        super.onStop();
    }

    @Override
    public void onDestroyView() {
        Log.d(TAG, "onDestroyView: ");
        super.onDestroyView();
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy: ");
        super.onDestroy();
        mAdapterManager.unregisterCallback(mIAdapterCallback);
        mMainHandler.removeCallbacksAndMessages(null);
    }

    @Override
    public void onDetach() {
        Log.d(TAG, "onDetach: ");
        super.onDetach();
        mAdapterManager.removeConnectListener(mAdapterListener);
    }

    class ListAdapter extends SimpleAdapter {
        private LayoutInflater mInflater;
        private String mSelectAddr = "";
        List<ArrayMap<String, Object>> mDeviceList;
        private String str_hfp = null;
        private String str_hfp_connected = null;

        public ListAdapter(Context context, List<ArrayMap<String, Object>> data,
                           int resource, String[] from, int[] to) {
            super(context, data, resource, from, to);
            // TODO Auto-generated constructor stub
            this.mInflater = LayoutInflater.from(context);
            this.mSelectAddr = "";
            this.mDeviceList = data;
            str_hfp = SkinUtils.getString(R.string.str_hfp_label);
            str_hfp_connected = SkinUtils.getString(R.string.bt_status_connected);
        }

        public void setSelect(String addr) {
            mSelectAddr = addr;
        }

        public String getSelect() {
            return mSelectAddr;
        }

        @Override
        public ArrayMap<String, Object> getItem(int position) {
            return mDeviceList.get(position);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder = null;
            if (convertView == null) {
                holder = new ViewHolder();
                convertView = SkinUtils.inflate(R.layout.device_listitem);
                holder.nameTextView = convertView.findViewById(SkinUtils.getId(R.id.item_remote_device_name));
                holder.statusTextView = convertView.findViewById(SkinUtils.getId(R.id.item_remote_connect_status));
                holder.macaddTextView = convertView.findViewById(SkinUtils.getId(R.id.item_remote_device_macaddr));
                holder.profileConfigImageButton = convertView.findViewById(SkinUtils.getId(R.id.btn_config_profile));
                if (holder.profileConfigImageButton != null) {
                    holder.profileConfigImageButton.setOnClickListener(PairedFragment.this);
                }

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
            if (holder.nameTextView != null) {
                holder.nameTextView.setText(name);
            }
            if (holder.statusTextView != null) {
                holder.statusTextView.setText(String.valueOf(state));
            }
            if (holder.macaddTextView != null) {
                holder.macaddTextView.setText(macaddr);
            }

            ImageView backImageView = convertView.findViewById(SkinUtils.getId(R.id.device_select));
            if (backImageView != null) {
                if (mSelectAddr.length() > 0 && mSelectAddr.equals(macaddr)) {
                    backImageView.setBackgroundResource(SkinUtils.getId(R.drawable.set_rb_on));
                } else {
                    backImageView.setBackgroundResource(SkinUtils.getId(R.drawable.set_rb_off));
                }
            }
            return convertView;
        }

        public final class ViewHolder {
            public TextView nameTextView;
            public TextView statusTextView;
            public TextView macaddTextView;
            public Button profileConfigImageButton;
        }
    }

    private void showDeviceProfileConfigDialog() {
        if (mDeviceProfileConfigDialog == null) {
            return;
        }

        if (mConfigDeviceMap == null) {
            Log.e(TAG, "showDeviceProfile mConfigDeviceMap is null !");
            return;
        }

        String deviceName = (String) mConfigDeviceMap.get(REMOTE_DEVICE_NAME);
        String deviceAddr = (String) mConfigDeviceMap.get(REMOTE_DEVICE_MACADDR);
        BluetoothDeviceInfo connectDevice = mAdapterManager.getConnectDevice();
        if (!mDeviceProfileConfigDialog.isShowing()) {
            mDeviceProfileConfigDialog.setTitle(deviceName);

            if (hfpSwitchBtn != null) {
                if (null != connectDevice && connectDevice.getDeviceAddr().equals(deviceAddr)) {
                    hfpSwitchBtn.setChecked(true);
                } else {
                    hfpSwitchBtn.setChecked(false);
                }
            }

            if (a2dpSwitchBtn != null) {
                if (mAdapterManager.isA2dpConnected(deviceAddr)) {
                    a2dpSwitchBtn.setChecked(true);
                } else {
                    a2dpSwitchBtn.setChecked(false);
                }
            }
            mDeviceProfileConfigDialog.show();
        }
    }

    private void buildDeviceProfileConfigDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(getContext(),
                android.R.style.Theme_DeviceDefault_Light_Dialog_Alert);
        builder.setTitle(SkinUtils.getString(R.string.bt_device_info));

        hfpSwitchBtn = new CheckBox(getContext());
        hfpSwitchBtn.setText(SkinUtils.getString(R.string.str_hfp_label));
        hfpSwitchBtn.setTextColor(SkinUtils.getColor(android.R.color.black));
        a2dpSwitchBtn = new CheckBox(getContext());
        a2dpSwitchBtn.setText(SkinUtils.getString(R.string.str_a2dp_label));
        a2dpSwitchBtn.setTextColor(SkinUtils.getColor(android.R.color.black));
        //a2dpSwitchBtn.setVisibility(View.GONE);

        LinearLayout layout = new LinearLayout(getContext());
        layout.setOrientation(LinearLayout.VERTICAL);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        params.setMargins(24, 12, 24, 0);
        layout.addView(hfpSwitchBtn, params);
        layout.addView(a2dpSwitchBtn, params);

        builder.setView(layout);
        builder.setNegativeButton(android.R.string.cancel, null);
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                onConfigDeviceProfile();
                dialog.dismiss();
            }
        });
        mDeviceProfileConfigDialog = builder.create();
    }

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

        String deviceAddr = (String) mConfigDeviceMap.get(REMOTE_DEVICE_MACADDR);
        BluetoothDeviceInfo connectDevice = mAdapterManager.getConnectDevice();
        boolean isHfpConnected;
        if (null != connectDevice && connectDevice.getDeviceAddr().equals(deviceAddr)) {
            isHfpConnected = true;
        } else {
            isHfpConnected = false;
        }

        if (needHfp && !isHfpConnected) {
            mAdapterManager.connectDevice(deviceAddr);
        } else if (!needHfp && isHfpConnected) {
            mAdapterManager.disconnectDevice(deviceAddr);
        }

        boolean isA2dpConnected = mAdapterManager.isA2dpConnected(deviceAddr);
        if (needA2dp && isHfpConnected && !isA2dpConnected) {
            mAdapterManager.connectA2dp(deviceAddr);
        } else if (!needA2dp && isA2dpConnected) {
            mAdapterManager.disconnectA2dp(deviceAddr);
        }
    }

    private ConnectionListener mAdapterListener = new ConnectionListener() {
        @Override
        public void onServiceConnected() {
            Log.d(TAG, "onServiceConnected");
            mAdapterManager.registerCallback(mIAdapterCallback);
            mMainHandler.removeMessages(MSG_UPDATE_FRAGMENT);
            mMainHandler.sendEmptyMessage(MSG_UPDATE_FRAGMENT);
        }

        @Override
        public void onServiceDisconnected() {

        }
    };

    private IAdapterCallback mIAdapterCallback = new IAdapterCallback.Stub() {
        @Override
        public void onBluetoothStateChanged(int state) throws RemoteException {
            Message msg = Message.obtain(mMainHandler, MSG_BLUETOOTH_STATE_CHANGE);
            msg.arg1 = state;
            mMainHandler.sendMessage(msg);
        }

        @Override
        public void onDiscoveryStateChanged(int state) throws RemoteException {
            Message msg = Message.obtain(mMainHandler, MSG_DISCOVERY_STATE_CHANGE);
            msg.arg1 = state;
            mMainHandler.sendMessage(msg);
        }

        @Override
        public void onDiscoveryDeviceFound(BluetoothDeviceInfo deviceInfo)
                throws RemoteException {
            Message msg = Message.obtain(mMainHandler, MSG_DISCOVERY_DEVICE_FOUND);
            msg.obj = deviceInfo;
            mMainHandler.sendMessage(msg);
        }

        @Override
        public void onDiscoveryDeviceNameChanged(BluetoothDeviceInfo deviceInfo)
                throws RemoteException {
            Message msg = Message.obtain(mMainHandler, MSG_DISCOVERY_DEVICE_NAME_CHANGE);
            msg.obj = deviceInfo;
            mMainHandler.sendMessage(msg);
        }

        @Override
        public void onDeviceBondStateChanged(BluetoothDeviceInfo deviceInfo, int state)
                throws RemoteException {
            Message msg = Message.obtain(mMainHandler, MSG_BOND_STATE_CHANGE);
            msg.obj = deviceInfo;
            mMainHandler.sendMessage(msg);
        }

        @Override
        public void onConnectionStateChanged(BluetoothDeviceInfo deviceInfo, int state)
                throws RemoteException {
            Message msg = Message.obtain(mMainHandler, MSG_CONNECT_STATE_CHANGE);
            msg.arg1 = state;
            msg.obj = deviceInfo;
            mMainHandler.sendMessage(msg);
        }
    };
}
