package com.autochips.bluetooth.fragment;

import android.bluetooth.BluetoothAdapter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.LinearInterpolator;
import android.view.animation.RotateAnimation;
import android.widget.ListView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.autochips.bluetooth.R;
import com.autochips.bluetooth.adapter.DeviceAdapter;
import com.autochips.bluetooth.manager.HStateBroadReceiver;
import com.autochips.bluetooth.view.HSettingItemLayout;
import com.autochips.bluetooth.view.HSettingsItemView;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;
import com.hcn.bluetooth.service.IAdapterCallback;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

/**
 * 设备信息区分
 * 1.搜索到的列表，也就是还没链接过的
 * 2.配对过的列表，也就是链接过的
 * 一般搜索完后，数据不会变化，
 * 只在配对和非配对之间交换。
 * 搜索列表的设备配对后，设备进入配对列表中显示。
 * <p>
 * 状态变化：
 * 1.蓝牙状态：指开关
 * 2.链接状态：指蓝牙设备(手机)链接的状态过程，有连接中-连接上-断开连接
 * 3.配对状态：配对中，配对上，和未配对。目前一般用到的就配对上和未配对
 */
public class SettingsFragment extends BaseFragment implements
        HSettingsItemView.OnItemViewCheckedChangedListener,
        HSettingsItemView.OnItemViewExpandChangedListener,
        View.OnClickListener, DeviceAdapter.OnDeviceClickListener
         {

    private HSettingsItemView mItemPower = null;
    private HSettingsItemView mItemSearch = null;
    private HSettingsItemView mItemPaired = null;
    private HSettingsItemView mItemAutoConnect = null;
    private HSettingsItemView mItemAutoAnswer = null;
    private HSettingsItemView mItemDeviceName = null;

    private TextView mTxtTitle = null;
    private View mViewContentRight = null;
    private View mLoadAnim = null;
    private ListView mLv;

    private List<BluetoothDeviceInfo> mPairedData = new ArrayList<>();
    private List<BluetoothDeviceInfo> mSearchData = new ArrayList<>();
    private DeviceAdapter mPairedAdapter = null;
    private DeviceAdapter mSearchAdapter = null;

    private LogicHandler mLogicHandler = null;
    private StateCallback mStateCallback = null;

    private boolean bIsSearchLogic = true;
    //快速开关使用
    //当前切换的状态
    private boolean bIsPowerOpen = true;
    //最后一次设置的状态
    private boolean mLastSetState = false;
    //当前实际状态
    private boolean mCurrentState = true;
    //
    private boolean bFromUser = false;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        TAG = "SettingsFragment";
    }

    @Override
    protected int onLoadLayoutId() {
        return R.layout.fragment_settings;
    }

    @Override
    protected void init() {
        initCallback();
        initView();
        initData();
        initState();
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if(!hidden){
            initState();
        }
    }

    private void initCallback() {
        if (null == mStateCallback) {
            mStateCallback = new StateCallback();
        }
        if (mBluetoothManager != null) {
            mBluetoothManager.registerCallback(mStateCallback);
            registerLocalStateCallback();
        }
    }

    private void initView() {
        log("initView" + SettingsFragment.this);
        mLogicHandler = new LogicHandler(SettingsFragment.this);
        //view
        mItemPower = (HSettingsItemView) findViewById(R.id.id_item_settings_bt);
        mItemSearch = (HSettingsItemView) findViewById(R.id.id_item_settings_search);
        mItemPaired = (HSettingsItemView) findViewById(R.id.id_item_settings_paired);
        mItemAutoConnect = (HSettingsItemView) findViewById(R.id.id_item_settings_auto_connect);
        mItemAutoAnswer = (HSettingsItemView) findViewById(R.id.id_item_settings_auto_answer);
        mItemDeviceName = (HSettingsItemView) findViewById(R.id.id_item_settings_device_name);

        ((HSettingItemLayout) findViewById(R.id.id_item_settings_content))
                .setOnItemViewCheckedChangedListener(this);
        ((HSettingItemLayout) findViewById(R.id.id_item_settings_content))
                .setOnItemViewExpandChangedListener(this);

        mViewContentRight = findViewById(R.id.id_settings_right_content);
        mLoadAnim = findViewById(R.id.id_settings_right_content_loading);
        mTxtTitle = (TextView) findViewById(R.id.id_settings_right_content_title);
        mTxtTitle.setOnClickListener(this);
        mLv = (ListView) findViewById(R.id.id_settings_right_content_lv);
        mSearchAdapter = new DeviceAdapter(getActivity(), mSearchData
                , getAppString(R.string.txt_bt_state_bound_none), getAppString(R.string.txt_bt_state_bounded), this);
        mPairedAdapter = new DeviceAdapter(getActivity(), mPairedData
                , getAppString(R.string.txt_bt_state_disconnected), getAppString(R.string.txt_bt_state_connected), this, true);

        mLv.setAdapter(mSearchAdapter);
    }

    private void updateState() {
        if (mBluetoothManager != null) {
            mCurrentState = mBluetoothManager.isBluetoothEnable();
            if (mLastSetState == bIsPowerOpen) {//外部用户操作
                mLastSetState = bIsPowerOpen = mCurrentState;
            } else {
                log("updateState L : " + mCurrentState + " , bIsPowerOpen:" + bIsPowerOpen);
                //连续开关时，off-on-off导致显示不对，需要关但是还是开了。
                if (mCurrentState != bIsPowerOpen) {
                    requestPower();
                    return;
                }
            }
            mItemPower.setChooseState(mCurrentState);
            mItemAutoConnect.setChooseState(mBluetoothManager.isBluetoothAutoConnect());
            mItemAutoAnswer.setChooseState(mBluetoothManager.isBluetoothAutoAnswer());
            mItemDeviceName.setStateText(mBluetoothManager.getBTName());
            updateUIByPower(mCurrentState);
        }
    }

    private void initState() {
        if (mBluetoothManager != null) {
            mLastSetState = bIsPowerOpen = mCurrentState = mBluetoothManager.isBluetoothEnable();
            log("mCurrentState: " + mCurrentState + " , bIsPowerOpen:" + bIsPowerOpen);
            mItemPower.setChooseState(mCurrentState);
            mItemDeviceName.setStateText(mBluetoothManager.getBTName());
            log("AutoConnect: " + mBluetoothManager.isBluetoothAutoConnect());
            log("AutoAnswer: " + mBluetoothManager.isBluetoothAutoAnswer());
            mItemAutoConnect.setChooseState(mBluetoothManager.isBluetoothAutoConnect());
            mItemAutoAnswer.setChooseState(mBluetoothManager.isBluetoothAutoAnswer());
            updateUIByPower(mCurrentState);

            //如是未连接下，切换语言后进来开关显示不对，SettingFragment先于Activity刷新了。
            mLogicHandler.postDelayed(()->{
                if(mCurrentState != mItemPower.isChooseState()){
                    mItemPower.setChooseState(mCurrentState);
                    mItemAutoConnect.setChooseState(mBluetoothManager.isBluetoothAutoConnect());
                    mItemAutoAnswer.setChooseState(mBluetoothManager.isBluetoothAutoAnswer());
                }

            },100);

            //刚进界面时服务可能还没绑定上，这里走不进去，需callbackPower中执行一次
            if (mCurrentState && !mBluetoothManager.isBluetoothConnected()) {
                requestDiscoverDevices();
            }
        }
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        log("____onConfigurationChanged___");
    }

    /**
     * 更新开关状态
     *
     * @param power
     */
    private void updateUIByPower(boolean power) {
        if (power) {
            showView(mItemSearch);
            showView(mItemPaired);
            showView(mItemAutoConnect);
            showView(mItemAutoAnswer);
            showView(mItemDeviceName);
            showView(mViewContentRight);
        } else {
            hideView(mItemSearch);
            hideView(mItemPaired);
            hideView(mItemAutoConnect);
            hideView(mItemAutoAnswer);
            hideView(mItemDeviceName);
            hideView(mViewContentRight);
            stopLoadingAnim();
        }
    }

    private void updateDiscoverUI(boolean start) {
        if (start) {
            startLoadingAnim();
        } else {
            stopLoadingAnim();
        }
    }
    /**
     * 概率点击搜索出现搜索失败的，但是没有结束状态，需要手动添加检测。
     */
    private void checkoutDiscoverUI(){
        if(mBluetoothManager != null && !mBluetoothManager.isDiscovering()){
            updateDiscoverUI(false);
        }
    }

    /**
     * 请求开关切换
     */
    private void requestPower() {
        if (mLogicHandler.hasMessages(LogicHandler.MSG_UPDATE_POWER_STATE)) {
            mLogicHandler.removeMessages(LogicHandler.MSG_UPDATE_POWER_STATE);
        }
        mLogicHandler.sendEmptyMessageDelayed(LogicHandler.MSG_UPDATE_POWER_STATE, 300);
    }

    /**
     * 检查切换条件
     */
    private void checkoutPower() {
        if (mLogicHandler.hasMessages(LogicHandler.MSG_UPDATE_POWER_STATE)) {
            mLogicHandler.removeMessages(LogicHandler.MSG_UPDATE_POWER_STATE);
        }
        //boolean curstate = mBluetoothManager.isBluetoothEnable();
        log("checkoutPower# mLastSetState = " + mLastSetState +
                ", bIsPowerOpen = " + bIsPowerOpen +
                ", mCurrentState = " + mCurrentState);
        if (mCurrentState != bIsPowerOpen) {
            //已设置的状态
            //String power = Utils.getSystemProperty(Constants.BT_POWER_PROP,"false");
            if (mLastSetState != bIsPowerOpen) {
                mBluetoothManager.setBluetoothEnable(bIsPowerOpen);
                mLastSetState = bIsPowerOpen;
            }
            mLogicHandler.sendEmptyMessageDelayed(LogicHandler.MSG_UPDATE_POWER_STATE, 300);
        } else {
            updateUIByPower(mCurrentState);
        }
    }

    private void startLoadingAnim() {
        RotateAnimation ra = new RotateAnimation(0.0f, 359.0f,
                RotateAnimation.RELATIVE_TO_SELF, 0.5f,
                RotateAnimation.RELATIVE_TO_SELF, 0.5f);
        ra.setRepeatCount(-1);
        ra.setRepeatMode(Animation.RESTART);
        ra.setDuration(1000);
        LinearInterpolator li = new LinearInterpolator();
        ra.setInterpolator(li);
        mLoadAnim.setVisibility(View.VISIBLE);
        mLoadAnim.startAnimation(ra);
    }

    private void stopLoadingAnim() {
        if (mLoadAnim.getVisibility() == View.VISIBLE) {
            mLoadAnim.setVisibility(View.GONE);
            mLoadAnim.clearAnimation();
        }
    }

    /**
     * 开始搜索设备，如刚进已经在搜索状态，或者有些没有开始搜索状态的需要执行添加
     * 开始搜索，然后1.5s后更新确认下。
    */
    private void requestDiscoverDevices(){
        log("requestDiscoverDevices");
        if (bIsSearchLogic && mBluetoothManager.isBluetoothEnable()) {
            if (mBluetoothManager.startDiscovery()) {
                //收不到开始发现设备的广播，手动开始动画
                mLogicHandler
                        .obtainMessage(LogicHandler.MSG_DISCOVERY_STATE_CHANGE, 1, 0)
                        .sendToTarget();
                mLogicHandler.sendEmptyMessageDelayed(LogicHandler.MSG_DISCOVERY_STATE_CHECK,1500);
                initData();
            }
        }
    }

    //listener

    /**
     * HSettingsItemView选择item事件监听
     *
     * @param v
     * @param isChecked
     */
    @Override
    public void onItemViewCheckedChanged(View v, boolean isChecked) {
        if (v.getId() == R.id.id_item_settings_bt) {
            updateUIByPower(isChecked);
            bIsPowerOpen = isChecked;
            requestPower();
        } else if (v.getId() == R.id.id_item_settings_auto_connect) {
            mBluetoothManager.setAutoConnect(isChecked);
        } else if (v.getId() == R.id.id_item_settings_auto_answer) {
            mBluetoothManager.setAutoAnswer(isChecked);
        }
    }

    /**
     * HSettingsItemView列表Item点击事件
     *
     * @param v
     * @param expandView
     * @param isExpand
     * @param isFormUser
     */
    @Override
    public void onItemViewExpand(View v, View expandView, boolean isExpand, boolean isFormUser) {
        if (v.getId() == R.id.id_item_settings_search) {
            bIsSearchLogic = true;
            mLv.setAdapter(mSearchAdapter);
            mTxtTitle.setText(getAppString(R.string.txt_settings_search_list));
            if(mBluetoothManager.isDiscovering()){
               startLoadingAnim();
            }
        } else if (v.getId() == R.id.id_item_settings_paired) {
            bIsSearchLogic = false;
            mLv.setAdapter(mPairedAdapter);
            mTxtTitle.setText(getAppString(R.string.txt_settings_paired_list));
            stopLoadingAnim();
        }
    }

    @Override
    public void onClick(View view) {
        requestDiscoverDevices();
    }

    /**
     * adapter 点击连接事件
     *
     * @param device
     */
    @Override
    public void onClickConnect(BluetoothDeviceInfo device) {
        if (bIsSearchLogic) {
            if (device != null) {
                mBluetoothManager.connectDevice(device.getDeviceAddr());
            }
        } else {
            if (device != null) {
                if (device.getDeviceStatus() == BluetoothDeviceInfo.DeviceStatus.DEVICE_STATUS_CONNECTED) {
                    mBluetoothManager.disConnectDevice(device.getDeviceAddr());
                } else {
                    mBluetoothManager.connectDevice(device.getDeviceAddr());
                }
            }
        }
    }

    /**
     * adapter 点击删除事件
     *
     * @param device
     */
    @Override
    public void onClickDelete(BluetoothDeviceInfo device) {
        if (device != null) {
            mBluetoothManager.unpairDevice(device.getDeviceAddr());
        }
    }

    /**
     * 转换handler。远程调用时转回本线程
     */
    private class LogicHandler extends Handler {
        private final static int MSG_UPDATE_LIST_PAIRED = 1;
        private final static int MSG_UPDATE_LIST_SEARCH = 2;
        private final static int MSG_DISCOVERY_STATE_CHANGE = 3;
        private final static int MSG_CONNECT_STATE_CHANGE = 4;
        private final static int MSG_BLUETOOTH_STATE_CHANGE = 5;
        private final static int MSG_UPDATE_POWER_STATE = 11;
        //request
        private final static int MSG_REQUEST_DISCOVER = 21;

        //check
        private final static int MSG_DISCOVERY_STATE_CHECK = 99;


        private WeakReference<SettingsFragment> mWr = null;

        public LogicHandler(SettingsFragment fragment) {
            this.mWr = new WeakReference<>(fragment);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            SettingsFragment fragment = mWr.get();
            if (fragment == null) {
                return;
            }
            switch (msg.what) {
                case MSG_DISCOVERY_STATE_CHANGE:
                    fragment.updateDiscoverUI(msg.arg1 == 0x01);//start discover
                    break;
                case MSG_DISCOVERY_STATE_CHECK:
                    fragment.checkoutDiscoverUI();
                    break;
                case MSG_UPDATE_LIST_PAIRED:
                case MSG_UPDATE_LIST_SEARCH:
                case MSG_CONNECT_STATE_CHANGE:
                case MSG_BLUETOOTH_STATE_CHANGE:
                    fragment.initData();
                    break;
                case MSG_UPDATE_POWER_STATE:
                    fragment.checkoutPower();
                    break;
                case MSG_REQUEST_DISCOVER:
                    //打开后1s，开始做自动连接，如马上搜索，连接时又会取消。
                    if(mPairedData != null && mPairedData.size() == 0) {
                        fragment.requestDiscoverDevices();
                    }
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * 初始化,更新两个列表数据
     * 也要把连接上的设备信息更新
     */
    private void initData() {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            log("initData start");
            //配对列表
            copyDevices(mBluetoothManager.getBondedDevices(), mPairedData);
            //搜索列表
            //奇瑞的这个会把配对的列表也放一份在搜索列表中
            copyDevices(mPairedData, mSearchData);
            copyDevices(false, mBluetoothManager.getDeviceList(), mSearchData);
            //适配已连接的设备
            BluetoothDeviceInfo device = mBluetoothManager.getConnectDevice();
            updateDeviceConnectState(0, device);
            log("initData end");
        }
    }

    /**
     * 更新设备连接状态信息
     */
    private void updateDeviceConnectState(int state, BluetoothDeviceInfo device) {
        BluetoothDeviceInfo info = null;
        if (device != null) {
            info = copyDevice(device);
            updateConnectDevice(mPairedData, info);
            updateConnectDevice(mSearchData, info);
        }
        if (mSearchAdapter != null) {
            mSearchAdapter.setData(mSearchData);
            mSearchAdapter.setConnectDevice(info);
            mPairedAdapter.setData(mPairedData);
            mPairedAdapter.setConnectDevice(info);
        }
    }

    //---------------------------------------------------------------
    //data处理

    /**
     * 拷贝设备列表
     *
     * @param source
     * @param data
     */
    protected void copyDevices(List<BluetoothDeviceInfo> source, List<BluetoothDeviceInfo> data) {
        copyDevices(true, source, data);
    }

    /**
     * 数据拷贝
     *
     * @param nedClean 是否需要先清除目标数据
     * @param source   源数据
     * @param data     目标数据
     */
    protected void copyDevices(boolean nedClean, List<BluetoothDeviceInfo> source, List<BluetoothDeviceInfo> data) {
        if (nedClean) {
            data.clear();
        }
        if (source == null || source.size() == 0) {
            return;
        }
        BluetoothDeviceInfo device = null;
        for (BluetoothDeviceInfo info : source) {
            if (!info.getDeviceAddr().equals(info.getDeviceName())) {
                device = new BluetoothDeviceInfo();
                device.setDeviceName(info.getDeviceName());
                device.setDeviceAddr(info.getDeviceAddr());
                device.setDeviceStatus(info.getDeviceStatus());
                device.setDeviceIndex(info.getDeviceIndex());
                data.add(device);
            }
        }
    }

    /**
     * 添加设备
     *
     * @param data
     * @param info
     */
    protected void addDevice(List<BluetoothDeviceInfo> data, BluetoothDeviceInfo info) {
        if (data != null && info != null) {
            if (data.size() == 0) {
                data.add(info);
                return;
            }
            for (BluetoothDeviceInfo device : data) {
                if (device.getDeviceAddr().equals(info.getDeviceAddr())) {
                    device.setDeviceName(info.getDeviceName());
                    device.setDeviceStatus(info.getDeviceStatus());
                    return;
                }
            }
            data.add(info);
        }
    }

    /**
     * 更新链接设备到列表中
     * 看需求，放在最前面
     *
     * @param data
     * @param info
     */
    protected void updateConnectDevice(List<BluetoothDeviceInfo> data, BluetoothDeviceInfo info) {
        if (data != null && info != null) {
            if (data.size() == 0) {
                data.add(info);
                return;
            }
            int position = -1;
            for (int i = 0; i < data.size(); i++) {
                if (data.get(i).getDeviceAddr().equals(info.getDeviceAddr())) {
                    position = i;
                    break;
                }
            }
            if (position >= 0) {
                data.remove(position);
            }
            data.add(0, info);
        }
    }

    protected BluetoothDeviceInfo copyDevice(BluetoothDeviceInfo remote) {
        BluetoothDeviceInfo device = new BluetoothDeviceInfo();
        device.setDeviceName(remote.getDeviceName());
        device.setDeviceAddr(remote.getDeviceAddr());
        device.setDeviceStatus(remote.getDeviceStatus());
        device.setDeviceIndex(remote.getDeviceIndex());
        return device;
    }
    //-------------------------------------------------------

    /**
     * 设备搜索配对搜索数据状态回调
     */
    private class StateCallback extends IAdapterCallback.Stub {

        @Override
        public void onBluetoothStateChanged(int state) throws RemoteException {
            log("onBluetoothStateChanged# state = " + state);
            mLogicHandler
                    .obtainMessage(LogicHandler.MSG_BLUETOOTH_STATE_CHANGE)
                    .sendToTarget();
            if(state == BluetoothAdapter.STATE_ON){
                mLogicHandler.sendEmptyMessageDelayed(LogicHandler.MSG_REQUEST_DISCOVER,500);
            }
        }

        @Override
        public void onDiscoveryStateChanged(int i) throws RemoteException {
            log("onDiscoveryStateChanged# state = " + i);
            mLogicHandler
                    .obtainMessage(LogicHandler.MSG_DISCOVERY_STATE_CHANGE, i, 0)
                    .sendToTarget();
        }

        @Override
        public void onDiscoveryDeviceFound(BluetoothDeviceInfo bluetoothDeviceInfo) throws RemoteException {
            log("onDiscoveryDeviceFound# " +
                    "name = " + bluetoothDeviceInfo.getDeviceName() +
                    "addr = " + bluetoothDeviceInfo.getDeviceAddr());
            if (bluetoothDeviceInfo.getDeviceName().equals(bluetoothDeviceInfo.getDeviceAddr())) {
                return;
            }

            mLogicHandler
                    .obtainMessage(LogicHandler.MSG_UPDATE_LIST_SEARCH, bluetoothDeviceInfo)
                    .sendToTarget();
        }

        @Override
        public void onDiscoveryDeviceNameChanged(BluetoothDeviceInfo bluetoothDeviceInfo) throws RemoteException {
            mLogicHandler
                    .obtainMessage(LogicHandler.MSG_UPDATE_LIST_SEARCH, bluetoothDeviceInfo)
                    .sendToTarget();
        }

        @Override
        public void onDeviceBondStateChanged(BluetoothDeviceInfo bluetoothDeviceInfo, int i) throws RemoteException {
            log("onDiscoveryDeviceFound# " +
                    "name = " + bluetoothDeviceInfo.getDeviceName() +
                    "addr = " + bluetoothDeviceInfo.getDeviceAddr());
            mLogicHandler
                    .obtainMessage(LogicHandler.MSG_UPDATE_LIST_PAIRED, bluetoothDeviceInfo)
                    .sendToTarget();
        }

        @Override
        public void onConnectionStateChanged(BluetoothDeviceInfo bluetoothDeviceInfo, int state) throws RemoteException {
            log("onConnectionStateChanged# " +
                    "name = " + bluetoothDeviceInfo.getDeviceName() +
                    "state = " + bluetoothDeviceInfo.getDeviceStatus());
            mLogicHandler
                    .obtainMessage(LogicHandler.MSG_CONNECT_STATE_CHANGE,
                            state, 0, bluetoothDeviceInfo).sendToTarget();

        }
    }

    //TODO 广播状态监听

    public void callbackConnect() {
        initData();
    }

    public void callbackDisconnect() {
        initData();
    }

    public void callbackPower() {
        log("callbackPower");
        updateStateData();

        if (mCurrentState && !mBluetoothManager.isBluetoothConnected()
            && mPairedData.size() == 0) {
            requestDiscoverDevices();
        }
    }

    public String callbackTag() {
        return TAG;
    }

    /**
     * 外部更新蓝牙状态
     * 目前主要是开关
     */
    public void updateStateData() {
        updateState();
        initData();
    }
}
