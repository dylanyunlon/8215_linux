package com.autochips.bluetooth.fragment;

import android.os.Bundle;
import android.os.Handler;
import android.os.RemoteException;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.autochips.bluetooth.BaseApplication;
import com.autochips.bluetooth.bean.HContact;
import com.autochips.bluetooth.bean.HRecord;
import com.autochips.bluetooth.manager.HBluetoothManager;
import com.autochips.bluetooth.manager.HStateBroadReceiver;
import com.autochips.bluetooth.manager.HUserData;
import com.autochips.bluetooth.util.SkinUtils;
import com.hcn.bluetooth.service.IPbapCallback;
import com.hcn.skin.support.app.SkinCompatFragment;

import java.util.List;

public abstract class BaseFragment extends SkinCompatFragment {

    protected String TAG = "BaseFragment";
    protected HBluetoothManager mBluetoothManager = null;
    protected LocalStateCallback mLocalBack = null;
    private MyPbapCallback mPbapStateCallback;
    private HUserData mUserData;
    //
    private long mLastTime = 0;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        TAG = getClass().getName();
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        mRoot = super.onCreateView(inflater, container, savedInstanceState);
        mBluetoothManager = BaseApplication.getInstance().getBluetoothAdapter();
        mUserData = BaseApplication.getInstance().getUserData();
        init();
        return mRoot;
    }

    @Override
    public void onBindViewData() {

    }

    @Override
    public View findViewByName(@NonNull String name) {
        return super.findViewByName(name);
    }

    protected View findViewById(int id) {
        return mRoot.findViewById(SkinUtils.getId(id));
    }

    @Override
    public int getLayoutRes() {
        return onLoadLayoutId();
    }

    public void updateNotify(){}
    /**
     * 广播下载电话本信息
     *
     * @return
     */
    protected boolean isBroadDownPbap() {
        return true;
    }

    private View mRoot = null;

    protected abstract int onLoadLayoutId();

    protected abstract void init();

    protected void callbackConnect() {
    }

    protected void callbackDisconnect() {

    }

    protected void callbackPower() {
    }

    protected void callbackDataChange() {
    }


    protected void log(String msg) {
        Log.d(TAG, "*~* |> " + msg);
    }

    protected void low(String msg) {
        Log.w(TAG, "*~* |> " + msg);
    }

    protected void logd(List<HContact> data) {
        if (data != null) {
            log("loading size : " + data.size());
            for (HContact contact : data) {
                log("name:" + contact.getName() + " , label:" + contact.getLabel());
            }
        }
    }

    protected void hideView(View v) {
        if (v != null && v.getVisibility() == View.VISIBLE) {
            v.setVisibility(View.INVISIBLE);
        }
    }

    protected void showView(View v) {
        if (v != null && v.getVisibility() != View.VISIBLE) {
            v.setVisibility(View.VISIBLE);
        }
    }

    /**
     * getResources().getString()在某些情况下context概率是取不到。
     *
     * @param resId
     * @return
     */
    protected String getAppString(int resId) {
        //return BaseApplication.getInstance().getString(SkinUtils.getResId(resId));
        return SkinUtils.getString(resId);
    }

    protected String[] getAppStringArrays(int resId) {
        return BaseApplication.getInstance().getResources().getStringArray(SkinUtils.getResId(resId));
    }

    protected void syncUserRecordData(List<HRecord> data){
        if(mUserData != null && data != null){
            mUserData.setRecordAll(data);
            //通知另外的页面更新数据
            if(mBluetoothManager != null) {
                mBluetoothManager.notifyRecordChange();
            }
        }
    }

    protected List<HRecord> getRecordData(){
        if(mUserData != null){
            return mUserData.getRecordAll();
        }
        return null;
    }

    protected void resetUserData(){
        if(mUserData != null){
            mUserData.reset();
        }
    }
    /**
     * 电话本状态
     */
    private class MyPbapCallback extends IPbapCallback.Stub {

        @Override
        public void onPbapDownloadStateChanged(int state, int type) throws RemoteException {
            //log("onPbapDownloadStateChanged" + state + " , type = " + type);
            if (getHandler() != null) {
                getHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        if(mBluetoothManager.isBluetoothConnected()) {
                            onPbapDownState(state, type);
                        }
                    }
                });
            }
        }

        @Override
        public void onPbapConnectStateChanged(int state) throws RemoteException {
            if (getHandler() != null) {
                getHandler().post(new Runnable() {
                    @Override
                    public void run() {
                        onPbapConnectState(state);
                    }
                });
            }
        }
    }

    protected void onPbapDownState(int state, int i1) {

    }

    protected void onPbapConnectState(int state) {

    }

    /**
     * 过掉快速点击菜单操作
     * @return
     */
    protected boolean allowCheck(){
        long time = System.currentTimeMillis();
        //low("allowCheck :  " + (time - mLastTime));
        if(time - mLastTime < 2000){
            if(time < mLastTime){
                mLastTime = 0;
            }
            return false;
        }
        mLastTime = time;
        return true;
    }

    /**
     * 设置电话本下载监听
     */
    protected void registerPbapCallback() {
        if (mPbapStateCallback == null) {
            mPbapStateCallback = new MyPbapCallback();
            if (mBluetoothManager != null) {
                mBluetoothManager.registerPbapCallback(TAG,mPbapStateCallback);
            }
        }
    }

    /**
     * 状态监听，连接和电源更新，
     */
    protected void registerLocalStateCallback() {
        if (mLocalBack == null) {
            mLocalBack = new LocalStateCallback();
        }
        if (mBluetoothManager != null) {
            mBluetoothManager.setLocalCallback(mLocalBack);
        }
    }

    protected Handler getHandler() {
        return null;
    }

    private class LocalStateCallback implements HStateBroadReceiver.BtStateCallback {
        @Override
        public void callbackConnect() {
            BaseFragment.this.callbackConnect();
        }

        @Override
        public void callbackDisconnect() {
            BaseFragment.this.callbackDisconnect();
        }

        @Override
        public void callbackPower() {
            BaseFragment.this.callbackPower();
        }

        @Override
        public void callbackChange() {
            BaseFragment.this.callbackDataChange();
        }

        @Override
        public String callbackTag() {
            return TAG;
        }
    }
}
