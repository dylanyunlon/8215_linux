package com.autochips.bluetooth.fragment;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.provider.CallLog;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.autochips.bluetooth.BaseApplication;
import com.autochips.bluetooth.R;
import com.autochips.bluetooth.adapter.RecordAdapter;
import com.autochips.bluetooth.bean.HRecord;
import com.autochips.bluetooth.util.HQueryThread;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;
import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

public class RecordFragment extends BaseFragment implements
        View.OnClickListener, AdapterView.OnItemClickListener {

    private ListView mLv = null;
    private TextView mTextLoading = null;

    private List<HRecord> mAllData = new ArrayList<>();
    private List<HRecord> mUnAnswerData = new ArrayList<>();
    private RecordAdapter mRecordAdapter = null;

    private HQueryThread mQueryThread = null;
    private QueryHandler mQueryHandler = null;
    private MainHandler mMainHandler = null;

    /**
     * 是否已经在更新数据,
     */
    private boolean bHasUpdateData = false;
    /**
     * 显示所有数据还是未接的
     */
    private boolean bShowAllData = true;

    /**
     * 兼容问题：iphone手机下载失败也是回的成功，但是取不到数据
     */
    private int mDownCount = 5;

    /**
     * 概率取到的只有未接的
     * 需要重新取一次数据，
     */
    private boolean bReloadByUnAnswer = true;

    @Override
    protected int onLoadLayoutId() {
        return R.layout.fragment_record;
    }

    @Override
    protected Handler getHandler() {
        return mMainHandler;
    }

    @Override
    protected void onPbapDownState(int state, int type) {
        super.onPbapDownState(state, type);
        log("onPbapDownState" + state + " , type = " + type);
        if (type == LocalBluetoothAdapterManager.PARAM_DOWNLOAD_MIOCH) {
            if (state == LocalBluetoothAdapterManager.STATE_DOWNLOADED) {
                if (mAllData != null && mAllData.size() > 0) {
                    showDataList(bShowAllData);
                    startDownRecord();
                } else {
                    if(mDownCount > 0) {
                        mDownCount --;
                        startDownRecord();
                    }else{
                        showLoadingTxt(getAppString(R.string.txt_loading_not_call));
                    }
                }
            } else if (state == LocalBluetoothAdapterManager.STATE_DOWNLOADING) {
                showLoadingTxt(getAppString(R.string.txt_loading));
            } else {//下载失败提示
                showLoadingTxt(getAppString(R.string.txt_loading_not_call));
            }
        } else if (type == LocalBluetoothAdapterManager.PARAM_DOWNLOAD_PB &&
                state == LocalBluetoothAdapterManager.STATE_DOWNLOADED) {
            init();
        }
    }

    @Override
    protected void callbackDisconnect() {
        super.callbackDisconnect();
        if (mAllData != null) {
            mAllData.clear();
            mUnAnswerData.clear();
        }
        if (mRecordAdapter != null) {
            mRecordAdapter.reset();
        }
        if (mQueryThread != null) {
            mQueryThread.stopLoadHistoryCallLog();
        }
        if (mMainHandler != null && mMainHandler.hasMessages(MainHandler.MSG_UPDATE_CHECK)){
            mMainHandler.removeMessages(MainHandler.MSG_UPDATE_CHECK);
        }
        bHasUpdateData = false;
        bReloadByUnAnswer = true;
        mDownCount = 5;
        resetUserData();
    }

    @Override
    protected void callbackDataChange() {
        super.callbackDataChange();
        Log.d(TAG, "callbackDataChange: ");
        startReadHistory();
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        TAG = "RecordFragment";
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if (hidden) {//hide

        } else {//show
            init();
        }
    }

    @Override
    protected void init() {
        initView();
        if (mMainHandler == null) {
            mMainHandler = new MainHandler(RecordFragment.this);
            //查询线程
            mQueryThread = new HQueryThread("call_record_thread");
            mQueryThread.setHandler(mMainHandler);
            mQueryThread.start();
            mQueryHandler = new QueryHandler(mQueryThread.getLooper());

            //注册监听
            registerLocalStateCallback();
            registerPbapCallback();
        }
        startReadHistory();

        if ((mAllData == null || mAllData.size() == 0) ||
                (mAllData != null && mUnAnswerData != null && mAllData.size() == mUnAnswerData.size())) {
            //startDownRecord();
            showLoadingTxt(getAppString(R.string.txt_loading));
            mMainHandler.sendEmptyMessage(MainHandler.MSG_UPDATE_CHECK);
        }
        showDataList(bShowAllData);
    }

    private void initView() {
        if (mTextLoading != null) {
            return;
        }
        findViewById(R.id.id_call_record_all_calls).setOnClickListener(this);
        findViewById(R.id.id_call_record_unanswered_calls).setOnClickListener(this);
        mTextLoading = (TextView) findViewById(R.id.id_call_record_loading);
        mLv = (ListView) findViewById(R.id.id_call_record_listview);
        mRecordAdapter = new RecordAdapter(getContext());
        mLv.setAdapter(mRecordAdapter);
        mLv.setOnItemClickListener(this);
    }

    private void showLoadingTxt(String txt) {
        if (mTextLoading != null && !TextUtils.isEmpty(txt)) {
            mTextLoading.setText(txt);
        }
    }

    private void showDataList(boolean all) {
        bShowAllData = all;
        if (all) {
            mRecordAdapter.setData(mAllData);
        } else {
            mRecordAdapter.setData(mUnAnswerData);
        }
    }

    /**
     * 读取历史记录
     */
    private void startReadHistory(){
        List<HRecord> data = getRecordData();
        if(data != null && !data.isEmpty()){
            if(mAllData == null){
                mAllData = new ArrayList<>();
                mUnAnswerData = new ArrayList<>();
            }
            if(!mAllData.isEmpty() && mAllData.get(0).getPhone().equals(data.get(0).getPhone())){
                log("startReadHistory : data is same!");
                return;
            }
            mAllData.clear();
            mUnAnswerData.clear();
            updateData(data);
        }
    }
    /**
     * 查询全部记录，共一百条
     */
    private void startDownRecord() {
        if(!mMainHandler.hasMessages(MainHandler.MSG_START_DOWN)){
            mMainHandler.removeMessages(MainHandler.MSG_START_DOWN);
        }
        mMainHandler.sendEmptyMessageDelayed(MainHandler.MSG_START_DOWN,1500);
    }

    /**
     * 0：查询所有的
     * Calls.MISSED_TYPE :查询未接的1
     * Calls.INCOMING_TYPE :查询来电2
     * Calls.OUTGOING_TYPE :查询去电3
     *
     * @param type
     */
    private void searchCallRecord(int type) {
        if (bHasUpdateData) {
            log(" is begin update record data! wait");
            return;
        }
        if (!mMainHandler.hasMessages(MainHandler.MSG_UPDATE_STATE)) {
            mMainHandler.sendEmptyMessageDelayed(MainHandler.MSG_UPDATE_STATE, 15 * 1000);
        }
        BluetoothDeviceInfo info = mBluetoothManager.getConnectDevice();
        if (info != null) {
            log(" start down record info");
            mQueryHandler.obtainMessage(
                            QueryHandler.MSG_UPDATE_ALL_CALL,
                            type,
                            0,
                            info.getDeviceAddr())
                    .sendToTarget();
            bHasUpdateData = true;
        }
    }

    /**
     * 下载中的数据更新
     */
    private void updateData(List<HRecord> data) {
        if (data != null && data.size() > 0) {
            mAllData.addAll(data);
            log("updateData# size = " + mAllData.size());
            for (HRecord info : data) {
                if (info.getType() == CallLog.Calls.MISSED_TYPE) {//未接
                    mUnAnswerData.add(info);
                }
            }
            if (mRecordAdapter != null) {
                if(bShowAllData){
                    mRecordAdapter.setData(mAllData);
                }else{
                    mRecordAdapter.setData(mUnAnswerData);
                }
            }
        }
    }

    private void loadFinish() {
        log("loadFinish : " + mAllData.size());
        if (mAllData != null && mAllData.size() > 0) {
            //取到的数据全部的和未接的一样，重新去一次
            if(mUnAnswerData != null && mUnAnswerData.size() == mAllData.size() && bReloadByUnAnswer){
                bReloadByUnAnswer = false;
                startDownRecord();
            }else {
                showDataList(bShowAllData);
                showLoadingTxt("");
                //save data
                syncUserRecordData(mAllData);
            }
        } else {//加载完成后发现没数据,则去申请下载一次
            mBluetoothManager.startRecordDownLoad();
        }
        bHasUpdateData = false;
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.id_call_record_all_calls:
                showDataList(true);
                break;
            case R.id.id_call_record_unanswered_calls:
                showDataList(false);
                break;
            default:
                break;
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        String phone = null;
        if (bShowAllData) {
            if (mAllData != null && mAllData.size() > position) {
                phone = mAllData.get(position).getPhone();
            }
        } else {
            if (mUnAnswerData != null && mUnAnswerData.size() > position) {
                phone = mUnAnswerData.get(position).getPhone();
            }
        }
        if (!TextUtils.isEmpty(phone)) {
            mBluetoothManager.dial(phone);
        }
    }

    ////////////////////////////////////////////////消息处理，数据下载
    class QueryHandler extends Handler {
        private final static int MSG_UPDATE_ALL_CALL = 1;
        private final static int MSG_STOP_ALL_CALL = 2;

        public QueryHandler(@NonNull Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            if (msg.what == MSG_UPDATE_ALL_CALL) {
                //此处跑的代码就是在辅助线程中了
                //实际beginLoadHistoryCallLog不应该写在HandlerThread中。为方便整洁
                mQueryThread.beginLoadHistoryCallLog(msg.arg1, (String) msg.obj);
            } else {
                mQueryThread.stopLoadHistoryCallLog();
            }
        }
    }

    class MainHandler extends Handler {
        private final static int MSG_UPDATE_STATE = 1;
        private final static int MSG_UPDATE_CHECK = 2;
        private final static int MSG_START_DOWN = 9;
        private WeakReference<RecordFragment> mRf = null;

        public MainHandler(RecordFragment fragment) {
            mRf = new WeakReference<>(fragment);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            RecordFragment fragment = mRf.get();
            if (fragment == null) {
                return;
            }
            switch (msg.what) {
                case MSG_UPDATE_STATE:
                    bHasUpdateData = false;
                    break;
                case MSG_UPDATE_CHECK:
                    if(mAllData.size() > 0){
                        log("MSG_UPDATE_CHECK: " + mAllData.size());
                        return;
                    }
                    log("MSG_UPDATE_CHECK: isDowning" + mBluetoothManager.isDowning());
                    if(mBluetoothManager.isDowning()){
                        sendEmptyMessageDelayed(MSG_UPDATE_CHECK,5000);
                    }else{
                        startDownRecord();
                    }
                    break;
                case MSG_START_DOWN:
                    searchCallRecord(0);
                    break;
                case HQueryThread.MSG_STATE_QUERY_RECORD_START:
                    if (null == mAllData) {
                        mAllData = new ArrayList<>();
                        mUnAnswerData = new ArrayList<>();
                    } else {
                        mAllData.clear();
                        mUnAnswerData.clear();
                    }
                    break;
                case HQueryThread.MSG_STATE_QUERY_RECORD_UPDATE:
                    fragment.updateData((List<HRecord>) msg.obj);
                    break;
                case HQueryThread.MSG_STATE_QUERY_RECORD_FINISH:
                    fragment.loadFinish();
                    break;
            }
        }
    }
}