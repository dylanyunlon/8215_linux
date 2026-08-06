package com.autochips.bluetooth.manager;

import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Log;

import com.hcn.bluetooth.api.BluetoothDeviceInfo;
import com.hcn.bluetooth.api.ConnectionListener;
import com.hcn.bluetooth.api.LocalBTMusicManager;
import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;
import com.hcn.bluetooth.api.LocalBluetoothHfpclientManager;
import com.hcn.bluetooth.service.IAdapterCallback;
import com.hcn.bluetooth.service.IMusicCallback;
import com.hcn.bluetooth.service.IPbapCallback;

import java.util.List;

public class HBluetoothManager {

    private void log(String msg) {
        Log.d("HBluetoothManager", "" + msg);
    }

    private LocalBluetoothAdapterManager mBluetoothManager = null;
    private LocalBluetoothHfpclientManager mBluetoothHfpClientManager = null;
    private LocalBTMusicManager mBluetoothMusicManager = null;
    private HBluetoothDownManager mDownManager = null;
    //callback
    private IAdapterCallback mAdapterCallback = null;
    //private IPbapCallback mPbapCallback = null;
    private IMusicCallback mMusicCallback = null;
    //state
    private HStateBroadReceiver mReceiver = null;
    //
    private boolean bNeedDiscovery = true;

    public HBluetoothManager(Context context) {
        initBluetooth(context);
        registerBtState(context);
    }

    void initBluetooth(Context context) {
        if (null == mBluetoothManager) {
            mBluetoothManager = LocalBluetoothAdapterManager.getInstance();
            mBluetoothManager.init(context);
        }
        if (null == mBluetoothHfpClientManager) {
            mBluetoothHfpClientManager = LocalBluetoothHfpclientManager.getInstance();
            mBluetoothHfpClientManager.init(context);
        }
        if (null == mBluetoothMusicManager) {
            mBluetoothMusicManager = LocalBTMusicManager.getInstance();
            mBluetoothMusicManager.init(context);
        }
        if(mDownManager == null){
            mDownManager = new HBluetoothDownManager(mBluetoothManager);
        }
    }

    protected LocalBTMusicManager getMusicService(){
        return mBluetoothMusicManager;
    }

    //TODO start BtState

    /**
     * 蓝牙状态监听，广播信息等
     */
    private void registerBtState(Context context) {
        mReceiver = new HStateBroadReceiver(context, this);
        mBluetoothManager.addConnectListener(mReceiver.mBluetoothConnectListener);
        mBluetoothMusicManager.addConnectListener(mReceiver.mMusicConnectListener);
    }

    public void unInit() {
        if (mReceiver != null) {
            mReceiver.unInit();
        }
    }

    public void setLocalCallback(HStateBroadReceiver.BtStateCallback callback) {
        if (mReceiver != null) {
            mReceiver.addCallback(callback);
        }
    }

    public void removeLocalCallback(HStateBroadReceiver.BtStateCallback callback) {
        if (mReceiver != null) {
            mReceiver.removeCallback(callback);
        }
    }

    //TODO end BtState

    /**
     * 重新注册，
     * registerCallback和registerPbapCallback没注册上时需要
     */
    protected void registerCallback() {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            if (mAdapterCallback != null) {
                mBluetoothManager.registerCallback(mAdapterCallback);
            }
            //if (mPbapCallback != null) {
                //mBluetoothManager.registerPbapCallback(mPbapCallback);
            //}
            if(mDownManager != null) {
                mDownManager.registerPbapCallback();
            }
        }
    }

    public void registerPbapCallback(String tag,IPbapCallback callback) {
        //if (mBluetoothManager != null && mBluetoothManager.isReady()) {
        //   mBluetoothManager.registerPbapCallback(callback);
        //} else {
        //    mPbapCallback = callback;
        //    log("registerPbapCallback failed : mBluetoothManager is not ready");
        //}
        if(mDownManager != null){
            mDownManager.addPbapCallback(tag,callback);
        }
    }

    public void registerCallback(IAdapterCallback callback) {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            mBluetoothManager.registerCallback(callback);
        } else {
            log("registerCallback failed : mBluetoothManager is not ready");
            mAdapterCallback = callback;
        }
    }

    public void notifyRecordChange(){
        if (mReceiver != null) {
            mReceiver.notifyDataChange();
        }
    }

    public boolean isCanPlay(){
        return !mReceiver.isScreenSaverState();
    }

    /**
     *
     */
    protected void registerMusicCallback(){
        if (mMusicCallback != null && mBluetoothMusicManager != null && mBluetoothMusicManager != null) {
            mBluetoothMusicManager.registerBTMusicCallback(mMusicCallback);
            try {
                mMusicCallback.onA2dpConnectStateChanged(
                        mBluetoothMusicManager.isA2dpConnected() ?
                                BluetoothProfile.STATE_CONNECTED : BluetoothProfile.STATE_DISCONNECTED);
            }catch (RemoteException e){
                e.printStackTrace();
            }
        }
    }

    public void registerMusicCallback(IMusicCallback callback){
        if(mBluetoothMusicManager != null && mBluetoothMusicManager.isReady()){
            mBluetoothMusicManager.registerBTMusicCallback(callback);
        }else{
            mMusicCallback = callback;
            log("registerMusicCallback failed : mBluetoothManager is not ready");
        }
    }



    public boolean isReady() {
        if (mBluetoothManager != null) {
            return mBluetoothManager.isReady();
        }
        return false;
    }

    public boolean isBluetoothConnected() {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            return mBluetoothManager.isBluetoothConnected();
        }
        return false;
    }

    public boolean isBluetoothEnable() {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            return mBluetoothManager.isBluetoothEnable();
        }
        return false;
    }

    public boolean isBluetoothAutoConnect() {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            return mBluetoothManager.isBluetoothAutoConnect();
        }else{
            log("isBluetoothAutoConnect failed : mBluetoothManager is not ready");
        }
        return true;
    }

    public boolean isBluetoothAutoAnswer() {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            return mBluetoothManager.isBluetoothAutoAnswer();
        }else{
            log("isBluetoothAutoAnswer failed : mBluetoothManager is not ready");
        }
        return false;
    }

    public String getBTName() {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            return mBluetoothManager.getBTName();
        }
        return "";
    }

    public BluetoothDeviceInfo getConnectDevice() {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            BluetoothDeviceInfo info = mBluetoothManager.getConnectDevice();
            if(info != null){
                log("getConnectDevice : "+info.getDeviceName());
            }else{
                log("getConnectDevice is null ");
            }
            return info;
        }
        return null;
    }

    public String getConnectAddress(){
        BluetoothDeviceInfo info = getConnectDevice();
        if(info != null){
            return info.getDeviceAddr();
        }
        return null;
    }

    public List<BluetoothDeviceInfo> getBondedDevices() {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            return mBluetoothManager.getBondedDevices();
        }
        return null;
    }

    public List<BluetoothDeviceInfo> getDeviceList() {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            return mBluetoothManager.getDeviceList();
        }
        return null;
    }

    public void setBluetoothEnable(boolean power) {
        if (mBluetoothManager != null && mBluetoothManager.isReady()
                && power != mBluetoothManager.isBluetoothEnable()) {
            log("(setBluetoothEnable)-> " + power);
            mBluetoothManager.setBluetoothEnable(power);
        } else {
            if (mBluetoothManager != null) {
                log("setBluetoothEnable failed : " + power + " reason:"
                        + " ready : " + mBluetoothManager.isReady()
                        + " power" + mBluetoothManager.isBluetoothEnable());
            } else {
                log("setBluetoothEnable failed : mBluetoothManager is not connected !");
            }
        }
    }

    public void reStartBluetooth() {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            log("(reStartBluetooth)-> ");
            mBluetoothManager.resetBT();
        } else {
            if (mBluetoothManager != null) {
                log("reStartBluetooth failed : reason:"
                        + " ready : " + mBluetoothManager.isReady()
                        + " power" + mBluetoothManager.isBluetoothEnable());
            } else {
                log("reStartBluetooth failed : mBluetoothManager is not connected !");
            }
        }
    }

    public void setBluetoothName(String name) {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            log("(setBluetoothName)-> " + name);
            mBluetoothManager.setBTName(name);
        } else {
            if (mBluetoothManager != null) {
                log("setBluetoothName failed : " + name + " reason:"
                        + " ready : " + mBluetoothManager.isReady());
            } else {
                log("setBluetoothName failed : mBluetoothManager is not connected !");
            }
        }
    }

    public void setAutoConnect(boolean open) {
        if (mBluetoothManager != null && mBluetoothManager.isReady()
                && open != mBluetoothManager.isBluetoothAutoConnect()) {
            mBluetoothManager.setBluetoothAutoConnect(open);
        } else {
            if (mBluetoothManager != null) {
                log("setAutoConnect failed : " + open + " open:"
                        + " ready : " + mBluetoothManager.isReady()
                        + " open" + mBluetoothManager.isBluetoothAutoConnect());
            } else {
                log("setAutoConnect failed : mBluetoothManager is not connected !");
            }
        }
    }

    public void setAutoAnswer(boolean open) {
        if (mBluetoothManager != null && mBluetoothManager.isReady()
                && open != mBluetoothManager.isBluetoothAutoAnswer()) {
            mBluetoothManager.setBluetoothAutoAnswer(open);
        } else {
            if (mBluetoothManager != null) {
                log("setAutoAnswer failed : " + open + " open:"
                        + " ready : " + mBluetoothManager.isReady()
                        + " open" + mBluetoothManager.isBluetoothAutoAnswer());
            } else {
                log("setAutoAnswer failed : mBluetoothManager is not connected !");
            }
        }
    }

    public boolean isNeedDiscovery() {
        return bNeedDiscovery;
    }

    public boolean isDiscovering() {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            return mBluetoothManager.isDiscovering();
        }else{
            log("isBluetoothAutoAnswer failed : mBluetoothManager is not ready");
        }
        return false;
    }

    public boolean startDiscovery() {
        if (mBluetoothManager != null && mBluetoothManager.isReady()
                && !mBluetoothManager.isDiscovering()) {
            bNeedDiscovery = false;
            mBluetoothManager.startDiscovery();
            return true;
        } else {
            if (mBluetoothManager != null) {
                log("startDiscovery failed :"
                        + " ready : " + mBluetoothManager.isReady()
                        + " isDiscovering : " + mBluetoothManager.isDiscovering());
            } else {
                log("startDiscovery failed : mBluetoothManager is not connected !");
            }
            return false;
        }
    }

    public void connectDevice(BluetoothDeviceInfo info) {
        if (info != null) {
            connectDevice(info.getDeviceAddr());
        }
    }

    public void connectDevice(String address){
        connectDevice(address,true);
    }

    public void connectDevice(String address,boolean nedCheck) {
        if (TextUtils.isEmpty(address)) {
            log("connectDevice#  address:" + address);
            return;
        }
        BluetoothDeviceInfo info = getConnectDevice();
        if (info != null && info.getDeviceAddr().equals(address)) {
            log("connectDevice : " + address + " device is already connected!");
            return;
        }
        if (mBluetoothManager != null && mBluetoothManager.isReady()
                && mBluetoothManager.isBluetoothEnable()) {
            if (mBluetoothManager.isDiscovering()) {
                mBluetoothManager.stopDiscovery();
            }
            log("(connectDevice)-> " + address);
            mBluetoothManager.connectDevice(address);
            if(nedCheck){
                mReceiver.checkDeviceConnect(address);
            }
        } else {
            if (mBluetoothManager != null) {
                log("connectDevice failed :"
                        + " ready : " + mBluetoothManager.isReady()
                        + " isBluetoothEnable : " + mBluetoothManager.isBluetoothEnable());
            } else {
                log("connectDevice failed : mBluetoothManager is not connected !");
            }
        }
    }

    public void connectA2dp(String address){
        if(mBluetoothManager != null){
            log("connectA2dp#  address:" + address);
            mBluetoothManager.connectA2dp(address);
        }
    }

    public void disConnectDevice(String address){
        if (TextUtils.isEmpty(address)) {
            log("disConnectDevice#  address:" + address);
            return;
        }
        if (mBluetoothManager != null && mBluetoothManager.isReady()
                && mBluetoothManager.isBluetoothEnable()) {
            log("(disConnectDevice)-> " + address);
            mBluetoothManager.disconnectDevice(address);
        }else{
            log("disConnectDevice# failed  address:" + address);
        }
    }

    public void unpairDevice(BluetoothDeviceInfo device) {
        if (device != null) {
            unpairDevice(device.getDeviceAddr());
        }
    }

    public void unpairDevice(String address) {
        if (mBluetoothManager != null && mBluetoothManager.isReady()) {
            log("(unpairDevice)-> " + address);
            mBluetoothManager.unpairDevice(address);
        } else {
            if (mBluetoothManager != null) {
                log("unpairDevice failed :"
                        + " ready : " + mBluetoothManager.isReady());
            } else {
                log("unpairDevice failed : mBluetoothManager is not connected !");
            }
        }
    }
    //----------------------------hfp-----------------

    /**
     *
     */
    public void dial(String number) {
        if (mBluetoothHfpClientManager != null && mBluetoothHfpClientManager.isReady()) {
            log("(dial)-> " + number);
            boolean incall = isInCall();
            mBluetoothHfpClientManager.dial(number);
            //需求和8368c一致
            if(incall){
                mReceiver.notifyCallStateToService();
            }
        } else {
            if (mBluetoothManager != null) {
                log("dial failed :"
                        + " ready : " + mBluetoothManager.isReady());
            } else {
                log("dial failed : mBluetoothManager is not connected !");
            }
        }
    }

    public void sendDTMF(byte bt) {
        if (mBluetoothHfpClientManager != null && mBluetoothHfpClientManager.isReady()) {
            log("(sendDTMF)-> " + bt);
            mBluetoothHfpClientManager.sendDTMF(bt);
        } else {
            if (mBluetoothManager != null) {
                log("dial failed :"
                        + " ready : " + mBluetoothManager.isReady());
            } else {
                log("dial failed : mBluetoothManager is not connected !");
            }
        }
    }

    /**
     * 取最后一次通话的号码
     */
    public String getLastCall() {
        if (mBluetoothHfpClientManager != null && mBluetoothHfpClientManager.isReady()) {
            return mBluetoothHfpClientManager.getLastCall();
        }
        return null;
    }

    /**
     * 通知下载手机端的电话本
     * 前置状态，蓝牙已连接，并且准备好
     */
    public boolean startContactDownLoad() {
        if (mBluetoothManager != null && mBluetoothManager.isReady() && mBluetoothManager.isBluetoothConnected()) {
            return mBluetoothManager.pbapStartDownLoad(LocalBluetoothAdapterManager.PARAM_DOWNLOAD_PB);
        } else {
            if (mBluetoothManager != null) {
                log("startContactDownLoad failed :"
                        + " ready : " + mBluetoothManager.isReady());
            } else {
                log("startContactDownLoad failed : mBluetoothManager is not connected !");
            }
            return false;
        }
    }

    public boolean startRecordDownLoad() {
        if (mBluetoothManager != null && mBluetoothManager.isReady() && mBluetoothManager.isBluetoothConnected()) {
            return mBluetoothManager.pbapStartDownLoad(LocalBluetoothAdapterManager.PARAM_DOWNLOAD_MIOCH);
        } else {
            if (mBluetoothManager != null) {
                log("startRecordDownLoad failed :"
                        + " ready : " + mBluetoothManager.isReady());
            } else {
                log("startRecordDownLoad failed : mBluetoothManager is not connected !");
            }
            return false;
        }
    }

    public boolean isDowning(){
        if(mDownManager != null){
            return mDownManager.isPbDowning();
        }
        return false;
    }

    public boolean isInCall(){
        return mReceiver.isCalling();
    }

    //TODO
    // MUSIC
    //TODO
    public boolean isAvrcpConnected(){
        if (mBluetoothMusicManager != null && mBluetoothMusicManager.isReady()) {
            return mBluetoothMusicManager.isAvrcpConnected();
        } else {
            if (mBluetoothMusicManager != null) {
                log("isAvrcpConnected failed :"
                        + " ready : " + mBluetoothMusicManager.isReady());
            } else {
                log("isAvrcpConnected failed : mBluetoothManager is not connected !");
            }
            return false;
        }
    }

    public boolean isA2dpConnected(){
        if (mBluetoothMusicManager != null && mBluetoothMusicManager.isReady()) {
            return mBluetoothMusicManager.isA2dpConnected();
        } else {
            if (mBluetoothMusicManager != null) {
                log("isA2dpConnected failed :"
                        + " ready : " + mBluetoothMusicManager.isReady());
            } else {
                log("isA2dpConnected failed : mBluetoothManager is not connected !");
            }
            return false;
        }
    }

    public void requestA2dp(){
        if (mBluetoothMusicManager != null && mBluetoothMusicManager.isReady()) {
            mBluetoothMusicManager.requestA2dp();
        } else {
            if (mBluetoothMusicManager != null) {
                log("requestA2dp failed :"
                        + " ready : " + mBluetoothMusicManager.isReady());
            } else {
                log("requestA2dp failed : mBluetoothManager is not connected !");
            }
        }
    }

    public void musicPrev(){
        musicCmd(LocalBTMusicManager.CMD_AVRCP_PREV);
    }

    public void musicNext(){
        musicCmd(LocalBTMusicManager.CMD_AVRCP_NEXT);
    }

    public void musicPause(){
        musicCmd(LocalBTMusicManager.CMD_AVRCP_PAUSE);
    }

    public void musicPlay(){
        musicCmd(LocalBTMusicManager.CMD_AVRCP_PLAY);
    }

    public void musicPauseOrPlay(){
        musicCmd(LocalBTMusicManager.CMD_AVRCP_PLAY_PAUSE);
    }

    public void musicStop(){
        musicCmd(LocalBTMusicManager.CMD_AVRCP_STOP);
    }

    public boolean isPlay(){
        if (mBluetoothMusicManager != null && mBluetoothMusicManager.isReady()) {
            return mBluetoothMusicManager.isA2dpPlaying();
        }else{
            if (mBluetoothMusicManager != null) {
                log("startDiscovery failed :"
                        + " ready : " + mBluetoothMusicManager.isReady());
            } else {
                log("setPower failed : mBluetoothManager is not connected !");
            }
        }
        return false;
    }
    private void musicCmd(int state){
        if (mBluetoothMusicManager != null && mBluetoothMusicManager.isReady()) {
            mBluetoothMusicManager.send_Avrcp_Cmd(state);
        }else{
            if (mBluetoothMusicManager != null) {
                log("startDiscovery failed :"
                        + " ready : " + mBluetoothMusicManager.isReady());
            } else {
                log("setPower failed : mBluetoothManager is not connected !");
            }
        }
    }

    public String[] getID3Info(){
        if(mBluetoothMusicManager != null && mBluetoothMusicManager.isReady()){
            return mBluetoothMusicManager.getID3Info();
        }
        return null;
    }
}
