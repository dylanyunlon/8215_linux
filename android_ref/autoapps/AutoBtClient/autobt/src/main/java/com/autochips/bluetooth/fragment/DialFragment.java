package com.autochips.bluetooth.fragment;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.Editable;
import android.text.InputType;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.autochips.bluetooth.R;
import com.autochips.bluetooth.adapter.ContactsAdapter;
import com.autochips.bluetooth.adapter.RecordAdapter;
import com.autochips.bluetooth.bean.HContact;
import com.autochips.bluetooth.bean.HRecord;
import com.autochips.bluetooth.bean.SearchResult;
import com.autochips.bluetooth.util.HQueryThread;
import com.autochips.bluetooth.util.SkinUtils;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;
import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;

import java.util.ArrayList;
import java.util.List;

public class DialFragment extends BaseFragment implements View.OnClickListener
        , TextWatcher, View.OnLongClickListener, AdapterView.OnItemClickListener {
    private int[] mNumIds = {R.id.id_dial_keyboard0, R.id.id_dial_keyboard1,
            R.id.id_dial_keyboard2, R.id.id_dial_keyboard3, R.id.id_dial_keyboard4,
            R.id.id_dial_keyboard5, R.id.id_dial_keyboard6, R.id.id_dial_keyboard7,
            R.id.id_dial_keyboard8, R.id.id_dial_keyboard9};
    private TextView mTxtLoading = null;
    private EditText mEditInput = null;
    private ListView mLv = null;
    private ContactsAdapter mContactsAdapter = null;
    private RecordAdapter mRecordAdapter = null;
    //查询
    private HQueryThread mQueryThread = null;
    private QueryHandler mQueryHandler = null;
    private MainHandler mMainHandler = null;
    //数据
    private List<HRecord> mRecords = new ArrayList<>();
    private List<HContact> mContacts = new ArrayList<>();
    private String mQueryNumber = null;
    /**
     * 通话后会设置上次记录，导致会再查询号码，过掉
     */
    private boolean bResetNumber = false;

    /**
     * 是否正在查找通讯录
     */
    private boolean bIsLoadingRecord = false;

    @Override
    protected Handler getHandler() {
        return mMainHandler;
    }

    @Override
    protected void callbackDisconnect() {
        super.callbackDisconnect();
        log("callbackDisconnect");
        if (mContacts != null) {
            mContacts.clear();
        }
        if (mRecords != null) {
            mRecords.clear();
        }
        if(mContactsAdapter != null){
            mContactsAdapter.resetData();
        }
        if(mRecordAdapter != null){
            mRecordAdapter.reset();
        }
        resetUserData();
        if(mMainHandler != null && mMainHandler.hasMessages(MainHandler.MSG_UPDATE_CHECK)){
            mMainHandler.removeMessages(MainHandler.MSG_UPDATE_CHECK);
        }
        bIsLoadingRecord = false;
    }

    @Override
    protected void callbackDataChange() {
        super.callbackDataChange();
        Log.d(TAG, "callbackDataChange: ");

    }

    @Override
    protected void callbackConnect() {
        super.callbackConnect();
        //resetUserData();
        //startSearchRecord();
    }

    @Override
    protected void onPbapDownState(int state, int type) {
        super.onPbapDownState(state, type);
        log("onPbapDownState : " + state + " , " + type);
        if (type == LocalBluetoothAdapterManager.PARAM_DOWNLOAD_MIOCH) {
            if (state == LocalBluetoothAdapterManager.STATE_DOWNLOADED) {
                if (mRecords == null || mRecords.size() == 0) {
                    startSearchRecord();
                }else{
                    mQueryHandler.sendEmptyMessageDelayed(QueryHandler.MSG_START_QUERY_LIST, 1500);
                }
            }
        }
    }

    @Override
    protected int onLoadLayoutId() {
        return R.layout.fragment_dial;
    }

    @Override
    public void updateNotify() {
        super.updateNotify();
        log("*updateNotify*");
        if(mEditInput != null && !mBluetoothManager.isInCall()) {
            call(mEditInput.getText().toString());
        }
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        TAG = "DialFragment";
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if (!hidden) {
            startSearchRecord();
        }
        log("*onHiddenChanged*");
        deleteOneDialPadString(true);
    }

    @Override
    public void onStart() {
        super.onStart();
        log("*onStart*");
        deleteOneDialPadString(true);
    }

    @Override
    public void onResume() {
        super.onResume();
        log("*onResume*");
        //deleteOneDialPadString(true);
    }

    @Override
    public void onPause() {
        super.onPause();
        log("*onPause*");
        //deleteOneDialPadString(true);
    }

    @Override
    protected void init() {
        if(mQueryThread == null) {
            initView();

            mQueryThread = new HQueryThread("dial_query_thread");
            mQueryThread.start();
            mQueryHandler = new QueryHandler(mQueryThread.getLooper());
            mMainHandler = new MainHandler();
            mQueryThread.setHandler(mMainHandler);

        }
        startSearchRecord();
        registerPbapCallback();
        registerLocalStateCallback();
    }

    /**
     * 读取历史数据
     * 如果record页面已经读了数据，这里直接拿过来显示
     */
    private void startReadRecordData(){
        log("startReadRecordData start");
        List<HRecord> data = getRecordData();
        if (data != null && !data.isEmpty()) {
            if (mRecords == null) {
                mRecords = new ArrayList<>();
            }
            if(!mRecords.isEmpty() && mRecords.get(0).getPhone().equals(data.get(0).getPhone())){
                log("startReadRecordData : data is same!");
                return;
            }
            mRecords.clear();
            mRecords.addAll(data);
        }
        log("startReadRecordData end");
    }
    /**
     * 查询通话记录列表
     */
    private void startSearchRecord() {
        log("#startSearchRecord# <start>");
        startReadRecordData();
        if (mRecords == null || mRecords.size() == 0) {
            log("startSearchRecord : " + bIsLoadingRecord);
            if (!bIsLoadingRecord) {
                bIsLoadingRecord = true;
                updateTxtLoading(true, getAppString(R.string.txt_loading));
                mMainHandler.sendEmptyMessageDelayed(MainHandler.MSG_UPDATE_CHECK,3000);
            }
        } else {
            if (TextUtils.isEmpty(mEditInput.getText().toString())) {
                mMainHandler.sendEmptyMessage(MainHandler.MSG_SHOW_HISTORY);
            }
        }
        log("#startSearchRecord# <end>");
    }

    private void startDown(){
        log("startDown");
        if(mQueryHandler.hasMessages(QueryHandler.MSG_START_QUERY_LIST)){
            mQueryHandler.removeMessages(QueryHandler.MSG_START_QUERY_LIST);
        }
        mQueryHandler.sendEmptyMessageDelayed(QueryHandler.MSG_START_QUERY_LIST, 1000);
    }

    private void initView() {
        mTxtLoading = (TextView) findViewById(R.id.id_dial_record_loading);
        for (int resId : mNumIds) {
            findViewById(resId).setOnClickListener(this);
        }
        findViewById(R.id.id_dial_keyboard_star).setOnClickListener(this);
        findViewById(R.id.id_dial_keyboard_sharp).setOnClickListener(this);
        findViewById(R.id.id_dial_keyboard_call).setOnClickListener(this);
        findViewById(R.id.id_dial_keyboard_call).setOnClickListener(this);
        findViewById(R.id.id_dial_keyboard_delete).setOnClickListener(this);
        findViewById(R.id.id_dial_keyboard_delete).setOnLongClickListener(this);
        mEditInput = (EditText) findViewById(R.id.id_dial_keyboard_input);
        mEditInput.setInputType(InputType.TYPE_NULL);
        mEditInput.addTextChangedListener(this);

        mRecordAdapter = new RecordAdapter(getContext());
        mContactsAdapter = new ContactsAdapter(getContext(), mContacts);
        mLv = (ListView) findViewById(R.id.id_dial_listview);
        mLv.setOnItemClickListener(this);
        mLv.setAdapter(mContactsAdapter);
    }

    private void updateTxtLoading(boolean show, String txt) {
        log("updateTxtLoading: " + show + " , " + txt);
        if (mTxtLoading != null) {
            if (show) {
                showView(mTxtLoading);
                if (txt != null) {
                    mTxtLoading.setText(txt);
                }
            } else {
                hideView(mTxtLoading);
            }
        }
    }

    /**
     * 按键输入
     *
     * @param str
     */
    private void addDialPadInputString(CharSequence str) {
        if (str == null || mEditInput == null) {
            return;
        }
        int index = mEditInput.getSelectionStart();
        Editable callnumstrEdit = mEditInput.getText();

        if (callnumstrEdit.length() > 14) {
            //showToast(R.string.str_call_number_is_too_long);
            return;
        }
        if (index < 0 || index > callnumstrEdit.length()) {
            callnumstrEdit.append(str);
        } else {
            callnumstrEdit.insert(index, str);
        }
        mEditInput.setText(callnumstrEdit);
        mEditInput.setSelection(index + str.length());
    }

    /**
     * 删除输入
     *
     * @param clean
     */
    private void deleteOneDialPadString(boolean clean) {
        if (mEditInput == null) {
            return;
        }
        if (clean) {
            if(TextUtils.isEmpty(mEditInput.getText().toString())){
                bResetNumber = true;
            }
            log("deleteOneDialPadString*setText*");
            mEditInput.setText("");
            return;
        }
        int index = mEditInput.getSelectionStart();
        Editable callnumstrEdit = mEditInput.getText();

        if (index >= 1) {
            callnumstrEdit.delete(index - 1, index);
        } else {
            return;
        }
        mEditInput.setText(callnumstrEdit);
        mEditInput.setSelection(index - 1);
    }

    private void call(String number) {
        log("*call*" + number);
        if (TextUtils.isEmpty(number)) {
            number = mBluetoothManager.getLastCall();
            if(TextUtils.isEmpty(number)) {
                if (mRecords != null && mRecords.size() > 0) {
                    number = mRecords.get(0).getPhone();
                }
            }
            bResetNumber = true;
            mEditInput.setText(number);
            mEditInput.setSelection(number.length());
            return;
        } else {
            //限制拨出
            //if(number.length() < 3){
            //    return;
            //}
        }

        if (!TextUtils.isEmpty(number)) {
            mBluetoothManager.dial(number);
            deleteOneDialPadString(true);
        }
    }

    @Override
    public void onClick(View view) {
        int id = SkinUtils.getId(view.getId());
        switch (id) {
            case R.id.id_dial_keyboard_call:
                call(mEditInput.getText().toString());
                break;
            case R.id.id_dial_keyboard_delete:
                deleteOneDialPadString(false);
                break;
            case R.id.id_dial_keyboard_star:
                addDialPadInputString("*");
                break;
            case R.id.id_dial_keyboard_sharp:
                addDialPadInputString("#");
                break;
            default:
                addDialPadInputString((String) view.getTag());
                break;
        }
    }

    @Override
    public boolean onLongClick(View view) {
        deleteOneDialPadString(true);
        return true;
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        if (mQueryNumber != null && mContacts.size() > position) {
            call(mContacts.get(position).getPhone());
        } else if (mRecords != null && mRecords.size() > position) {
            call(mRecords.get(position).getPhone());
        }
    }

    @Override
    public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {

    }

    @Override
    public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {

    }

    @Override
    public void afterTextChanged(Editable editable) {
        String number = editable.toString();
        low("afterTextChanged : " + number + " , isResetNumber :" + bResetNumber);
        if (bResetNumber) {
            bResetNumber = false;
            return;
        }
        if (editable != null && number.length() > 0) {
            if (number.equals(mQueryNumber)) {
                return;
            }
            mQueryNumber = number;
            mQueryHandler.obtainMessage(QueryHandler.MSG_START_QUERY_KEY, editable.toString()).sendToTarget();
            mLv.setAdapter(mContactsAdapter);
        } else {
            mQueryNumber = null;
            mMainHandler.obtainMessage(MainHandler.MSG_SHOW_HISTORY).sendToTarget();
        }
    }

    /**
     * 查询线程Handler
     */
    private class QueryHandler extends Handler {
        private static final int MSG_START_QUERY_KEY = 1;
        private static final int MSG_START_QUERY_LIST = 11;

        public QueryHandler(@NonNull Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            BluetoothDeviceInfo info = mBluetoothManager.getConnectDevice();
            if (info != null) {
                low("handleMessage : " + info.getDeviceAddr());
                switch (msg.what) {
                    case MSG_START_QUERY_KEY:
                        mQueryThread.beginSearchContact((String) msg.obj, info.getDeviceAddr());
                        break;
                    case MSG_START_QUERY_LIST:
                        mQueryThread.beginLoadHistoryCallLog(0/*所有内容*/, info.getDeviceAddr());
                        break;
                }
            }
        }
    }

    /**
     * UI线程Handler
     */
    private class MainHandler extends Handler {
        //优先联系人查询，两个一起查时可能导致联系人卡住等很久
        private final static int MSG_UPDATE_CHECK = 1;
        private final static int MSG_SHOW_HISTORY = 99;

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_UPDATE_CHECK:
                    if(mRecords != null && mRecords.size() > 0){
                        log("MSG_UPDATE_CHECK  is not null");
                        return;
                    }
                    log("MSG_UPDATE_CHECK " + mBluetoothManager.isDowning());
                    if(mBluetoothManager.isDowning()){
                       sendEmptyMessageDelayed(MSG_UPDATE_CHECK,3000);
                    }else{
                        startDown();
                    }
                    break;
                //查询所有记录
                case HQueryThread.MSG_STATE_QUERY_RECORD_START:
                    log("MSG_STATE_QUERY_RECORD_START");
                    //TODO 开始正式加载
                    if (mRecords == null) {
                        mRecords = new ArrayList<>();
                    } else {
                        mRecords.clear();
                    }
                    break;
                case HQueryThread.MSG_STATE_QUERY_RECORD_UPDATE:
                    log("MSG_STATE_QUERY_RECORD_UPDATE");
                    //TODO 每10个做一次更新
                    List<HRecord> data = (List<HRecord>) msg.obj;
                    if (data != null) {
                        mRecords.addAll(data);
                    }
                    break;
                case HQueryThread.MSG_STATE_QUERY_RECORD_FINISH:
                    log("MSG_STATE_QUERY_RECORD_FINISH");
                    //TODO 加载完成
                    if (TextUtils.isEmpty(mEditInput.getText().toString())) {
                        sendEmptyMessage(MSG_SHOW_HISTORY);
                    }
                    syncUserRecordData(mRecords);
                    bIsLoadingRecord = false;
                    break;
                //依照输入查询匹配的联系人列表
                case HQueryThread.MSG_STATE_QUERY_CONTACT_START:
                    log("MSG_STATE_QUERY_CONTACT_START");
                    if (mContacts == null) {
                        mContacts = new ArrayList<>();
                    } else {
                        mContacts.clear();
                    }
                    break;
                case HQueryThread.MSG_STATE_QUERY_CONTACT_UPDATE:
                    SearchResult result = (SearchResult) msg.obj;
                    if (result.list != null) {
                        mContacts.addAll(result.list);
                        mContactsAdapter.setData(mContacts);
                    }
                    break;
                case HQueryThread.MSG_STATE_QUERY_CONTACT_FINISH:
                    log("MSG_STATE_QUERY_CONTACT_FINISH");
                    if (mContacts != null && mContacts.size() == 0) {
                        if(mContactsAdapter != null){
                            mContactsAdapter.resetData();
                        }
                        updateTxtLoading(true, getAppString(R.string.txt_loading_not_contact));
                    } else {
                        updateTxtLoading(false, "");
                    }
                    break;

                case MSG_SHOW_HISTORY:
                    log("MSG_SHOW_HISTORY");
                    if (mContactsAdapter != null) {
                        mContactsAdapter.resetData();
                    }
                    if (mRecordAdapter != null) {
                        mRecordAdapter.setData(mRecords);
                        mLv.setAdapter(mRecordAdapter);
                    }
                    if (mRecords != null && mRecords.size() == 0) {
                        updateTxtLoading(true, getAppString(R.string.txt_loading_not_call));
                    } else {
                        updateTxtLoading(false, "");
                    }
                    break;
            }
        }
    }
}
