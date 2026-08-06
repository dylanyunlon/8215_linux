package com.autochips.bluetooth.fragment;


import static android.provider.CallLog.Calls.INCOMING_TYPE;
import static android.provider.CallLog.Calls.MISSED_TYPE;
import static android.provider.CallLog.Calls.OUTGOING_TYPE;

import android.accounts.Account;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.CancellationSignal;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.OperationCanceledException;
import android.os.RemoteException;
import android.provider.CallLog;

import android.provider.ContactsContract;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RadioButton;
import android.widget.SimpleAdapter;
import android.widget.TextView;

import androidx.annotation.NonNull;

import com.autochips.bluetooth.MyApplication;
import com.autochips.bluetooth.R;
import com.autochips.bluetooth.skin.SkinUtils;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;
import com.hcn.bluetooth.api.ConnectionListener;
import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;
import com.hcn.bluetooth.api.Utils;
import com.hcn.bluetooth.service.IPbapCallback;
import com.hcn.skin.support.app.SkinCompatFragment;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;


public class RecordFragment extends SkinCompatFragment implements View.OnClickListener {
    public static final String TAG = "RecordFragment";

    private static final String ITEM_HISTORY_NAME = "item_history_name";
    private static final String ITEM_HISTORY_NUMBER = "item_history_number";
    private static final String ITEM_HISTORY_TIME = "item_history_time";
    private static final String ITEM_HISTORY_TYPE = "item_history_type";

    private Context mContext;
    // ui
    private LinearLayout mDownloadLayout;
    private TextView mDownloadTextView;
    private AbsListView historyListView = null;
    private int mFocusId = R.id.btn_missed;
    private RadioButton mMissedCall;

    private HistoryListAdapter mHistoryListAdapter;
    private ArrayList<HashMap<String, Object>> mHistoryList;

    private LoaderHandler mLoaderHandler;
    private HandlerThread mLoaderThread;
    //通知LoaderHandler从数据库读取通话记录
    private static final int LOADER_HISTORY = 1;
    //通知LoaderHandler从数据库删除通话记录
    private static final int CLEAR_CALL_lOG = 2;

    private CancellationSignal mCancellationSignal = null;

    private LocalBluetoothAdapterManager mAdapterManager;

    private static final String[] CALLLOG_PROJECTION = {
            CallLog.Calls.NUMBER,
            CallLog.Calls.DATE,
            CallLog.Calls.TYPE,
            CallLog.Calls.CACHED_NAME
    };

    private MainHandler mMainHandler;
    private static final int MSG_DOWNLOAD_START = 0;//从手机下载数据状态
    private static final int MSG_DOWNLOAD_FINISH = 1;
    private static final int MSG_DOWNLOAD_FAILED = 2;
    private static final int MSG_UI_LOAD_START = 3;//从数据库读取数据状态
    private static final int MSG_UI_LOAD_FINISH = 4;
    private static final int MSG_UI_HISTORY_ADD = 5;

    private class MainHandler extends Handler {
        private WeakReference<RecordFragment> mWeakRefFragment;

        public MainHandler(RecordFragment fragment) {
            super(Looper.getMainLooper());
            mWeakRefFragment = new WeakReference<>(fragment);
        }

        @Override
        public void handleMessage(Message msg) {
            RecordFragment fragment = mWeakRefFragment.get();
            if (fragment != null) {
                int what = msg.what;
                switch (what) {
                    case MSG_DOWNLOAD_START:
                        fragment.showDownloadGroup();
                        fragment.stopLoadHistoryCallLog();
                        fragment.clearHistoryList();
                        break;
                    case MSG_DOWNLOAD_FINISH:
                        fragment.startLoadHistoryCallLog(0);
                        break;
                    case MSG_DOWNLOAD_FAILED:
                        int error = msg.arg1;
                        if (error == LocalBluetoothAdapterManager.STATE_FAILED_UNKNOWN
                                || error == LocalBluetoothAdapterManager.STATE_FAILED_NO_CONNECT) {
                            Utils.showToast(mContext, SkinUtils.getId(R.string.str_download_phonebook_fail));
                        }
                        fragment.hideDownloadGroup();
                        break;
                    case MSG_UI_LOAD_START:
                        fragment.clearHistoryList();
                        break;
                    case MSG_UI_HISTORY_ADD:
                        fragment.mHistoryList.addAll((List<HashMap<String, Object>>) msg.obj);
                        fragment.mHistoryListAdapter.notifyDataSetChanged();
                        fragment.changeDownloadHint(fragment.mHistoryList.size());
                        Log.d(TAG, "MSG_UI_HISTORY_ADD");
                        break;
                    case MSG_UI_LOAD_FINISH:
                        if (null != msg.obj) {
                            fragment.mHistoryList.addAll((List<HashMap<String, Object>>) msg.obj);
                        }
                        fragment.mHistoryListAdapter.notifyDataSetChanged();
                        fragment.hideDownloadGroup();
                        Log.d(TAG, "MSG_UI_LOAD_FINISH size=" + fragment.mHistoryList.size());
                    default:
                        break;
                }
            }
            super.handleMessage(msg);
        }
    }

    private class LoaderHandler extends Handler {
        public LoaderHandler(@NonNull Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            switch (msg.what){
                case LOADER_HISTORY:
                    try {
                        loadHistoryCallLog(msg.arg1);
                    } catch (Exception e) {
                        Log.d(TAG, "handleMessage: MSG_PB_SEARCH Exception");
                        hideDownloadGroup();
                    }
                    break;
                case CLEAR_CALL_lOG:
                    deleteCallLog();
                    break;
            }

        }
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        mMainHandler = new MainHandler(this);
        if (SkinUtils.useSkinPackage()) {
            mContext = SkinUtils.getContext();
        } else {
            mContext = context;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "RecordFragment onCreate");
        mAdapterManager = MyApplication.getInstance().getAdapterManager();
        mAdapterManager.addConnectListener(mAdapterListener);
        mAdapterManager.registerPbapCallback(mIPbapCallback);

        mLoaderThread = new HandlerThread("work_thread");
        mLoaderThread.start();
        mLoaderHandler = new LoaderHandler(mLoaderThread.getLooper());
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // View root = inflater.inflate(R.layout.bt_callhistory, container, false);
        View root = super.onCreateView(inflater, container, savedInstanceState);

        mContext = SkinUtils.getContext();

        initView(root);
        Log.d(TAG, "onCreateView");
        return root;
    }

    @Override
    public void onBindViewData() {

    }

    @Override
    public int getLayoutRes() {
        return R.layout.bt_callhistory;
    }

    private void initView(View layout) {
        if (layout == null) {
            Log.e(TAG, "initView: layout is null");
            return;
        }
        // TODO
        layout.findViewById(SkinUtils.getId(R.id.bt_callhistory_syn)).setOnClickListener(this);
        layout.findViewById(SkinUtils.getId(R.id.btn_dialed)).setOnClickListener(this);
        layout.findViewById(SkinUtils.getId(R.id.btn_received)).setOnClickListener(this);
        mMissedCall = layout.findViewById(SkinUtils.getId(R.id.btn_missed));
        mMissedCall.setOnClickListener(this);
        mDownloadLayout = layout.findViewById(SkinUtils.getId(R.id.download_callhistory_layout));
        mDownloadTextView = layout.findViewById(SkinUtils.getId(R.id.tv_download_callhistory_text));

        ((ProgressBar) layout.findViewById(SkinUtils.getId(R.id.progressbar_download_callhistory)))
                .setIndeterminate(false);

        historyListView = layout.findViewById(SkinUtils.getId(R.id.history_listview));
        mHistoryList = new ArrayList<>();
        mHistoryListAdapter = new HistoryListAdapter(mContext, mHistoryList,
                SkinUtils.getId(R.layout.history_listitem), new String[]{ITEM_HISTORY_NAME,
                ITEM_HISTORY_NUMBER, ITEM_HISTORY_TIME,
                ITEM_HISTORY_TYPE}, new int[]{SkinUtils.getId(R.id.item_history_name),
                SkinUtils.getId(R.id.item_history_number), SkinUtils.getId(R.id.item_history_time),
                SkinUtils.getId(R.id.btn_call_history_img)});
        historyListView.setAdapter(mHistoryListAdapter);
        historyListView.setOnItemClickListener(mHistoryListClickListener);
    }

    @Override
    public void setUserVisibleHint(boolean isVisibleToUser) {
        Log.e(TAG, "setUserVisibleHint:" + isVisibleToUser);
        super.setUserVisibleHint(isVisibleToUser);
        if (isVisibleToUser) {
            if (mAdapterManager.isReady()) {
                BluetoothDeviceInfo deviceInfo = mAdapterManager.getConnectDevice();
                if (null != deviceInfo) {
                    if (mAdapterManager.getPbapDownLoadState(
                            LocalBluetoothAdapterManager.PARAM_DOWNLOAD_MIOCH)) {
                        showDownloadGroup();
                    } else {
                        hideDownloadGroup();
                        if (mHistoryList.isEmpty()) {
                            mFocusId = R.id.btn_missed;
                            if (null != mMissedCall) {
                                mMissedCall.setChecked(true);
                            }
                            startLoadHistoryCallLog(MISSED_TYPE);
                        }
                    }
                } else {
                    clearHistoryList();
                }
            }
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        Log.d(TAG, "RecordFragment onStart");
        IntentFilter filter = new IntentFilter();
        filter.addAction(LocalBluetoothAdapterManager.ACTION_CONNECTION_STATE_CHANGED);
        if (null != getContext()) {
            getContext().registerReceiver(mReceiver, filter);
        }
    }

    @Override
    public void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        Log.d(TAG, "RecordFragment onResume");
    }

    @Override
    public void onPause() {
        // TODO Auto-generated method stub
        Log.d(TAG, "RecordFragment onPause");
        super.onPause();
    }

    @Override
    public void onStop() {
        Log.d(TAG, "RecordFragment onStop");
        super.onStop();
        if (null != getContext()) {
            getContext().unregisterReceiver(mReceiver);
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (null != mAdapterManager) {
            mAdapterManager.unregisterPbapCallback(mIPbapCallback);
            mAdapterManager.removeConnectListener(mAdapterListener);
        }
        if (null != mLoaderThread) {
            mLoaderThread.quitSafely();
        }
    }

    @Override
    public void onDetach() {
        super.onDetach();
        if (null != mMainHandler) {
            mMainHandler.removeCallbacksAndMessages(null);
        }
    }

    private void showDownloadGroup() {
        changeDownloadHint(0);
        mDownloadLayout.setVisibility(View.VISIBLE);
    }

    private void hideDownloadGroup() {
        mDownloadLayout.setVisibility(View.GONE);
    }

    private void changeDownloadHint(int count) {
        String strText = String.format(SkinUtils.getString(R.string.str_download_history), count);
        if (mDownloadTextView != null) {
            Log.d(TAG, "changeDownloadHint:" + strText);
            mDownloadTextView.setText(strText);
        }
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(
                    LocalBluetoothAdapterManager.ACTION_CONNECTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothProfile.STATE_DISCONNECTED);
                if (state == BluetoothProfile.STATE_CONNECTED) {
                    mFocusId = R.id.btn_missed;
                    if (null != mMissedCall) {
                        mMissedCall.setChecked(true);
                    }
                    startLoadHistoryCallLog(MISSED_TYPE);
                } else if (state == BluetoothProfile.STATE_DISCONNECTED) {
                    stopLoadHistoryCallLog();
                    clearHistoryList();
                }
            }
        }
    };

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        Log.d(TAG, "onHiddenChanged hidden=" + hidden);
    }

    @Override
    public void onClick(View v) {
        int viewId = SkinUtils.getViewId(v);
        switch (viewId) {
            case R.id.btn_dialed:
                mFocusId = R.id.btn_dialed;
                clearHistoryList();
                startLoadHistoryCallLog(OUTGOING_TYPE);
                Log.d(TAG, "show download btn_dialed");
                break;
            case R.id.btn_received:
                mFocusId = R.id.btn_received;
                clearHistoryList();
                startLoadHistoryCallLog(INCOMING_TYPE);
                Log.d(TAG, "show download btn_received");
                break;
            case R.id.btn_missed:
                mFocusId = R.id.btn_missed;
                clearHistoryList();
                startLoadHistoryCallLog(MISSED_TYPE);
                Log.d(TAG, "show download btn_missed");
                break;
            case R.id.bt_callhistory_syn:
                stopLoadHistoryCallLog();
                if (!mAdapterManager.pbapStartDownLoad(
                        LocalBluetoothAdapterManager.PARAM_DOWNLOAD_MIOCH)) {
                    Utils.showToast(mContext, SkinUtils.getId(R.string.str_download_wait_phonebook_tip));
                } else {
                    mFocusId = R.id.bt_callhistory_syn;
                }
                break;
            default:
                break;
        }
    }

    public void clearHistoryList() {
        if (null == mHistoryList) {
            return;
        }
        if (mHistoryList.size() > 0) {
            mHistoryList.clear();
            mHistoryListAdapter.notifyDataSetChanged();
        }
    }

    /**
     * 清空通话记录
     */
    public void clearCallLog(){
        if (mHistoryList.size() > 0) {
            clearHistoryList();
            if (mLoaderHandler != null) {
                Message msg = Message.obtain(mLoaderHandler, CLEAR_CALL_lOG);
                mLoaderHandler.removeMessages(CLEAR_CALL_lOG);
                mLoaderHandler.sendMessage(msg);
            }
        }
    }

    /**
     * 删除通话记录数据库数据
     */
    private void deleteCallLog() {
        if (null == mContext) {
            Log.e(TAG, "deleteCallLog：mContext()==null!");
            return;
        }

        BluetoothDeviceInfo deviceInfo = mAdapterManager.getConnectDevice();
        if (null == deviceInfo) {
            Log.e(TAG, "deleteCallLog deviceInfo==null!");
            return;
        }

        Account account = new Account(deviceInfo.getDeviceAddr(),
                Utils.ACCOUNT_TYPE);
        try {
            Log.d(TAG, "To delete call log database data");
            // need to check call table is exist ?
            if (mContext.getContentResolver() == null) {
                Log.d(TAG, "Call log ContentResolver is not found");
                return;
            }

            mContext.getContentResolver().delete(CallLog.Calls.CONTENT_URI,
                    CallLog.Calls.PHONE_ACCOUNT_ID + "=?", new String[]{account.name});
        } catch (IllegalArgumentException e) {
            Log.d(TAG, "Call log could not be deleted, they may not exist yet.");
        }
    }

    public void stopLoadHistoryCallLog() {
        Log.d(TAG, "stopLoadHistoryCallLog");
        mLoaderHandler.removeMessages(LOADER_HISTORY);
        if (null != mCancellationSignal) {
            Log.d(TAG, "stopLoadHistoryCallLog mCancellationSignal");
            mCancellationSignal.cancel();
        }
    }

    public void startLoadHistoryCallLog(int type) {
        Message msg = Message.obtain(mLoaderHandler, LOADER_HISTORY);
        msg.arg1 = type;
        stopLoadHistoryCallLog();
        mLoaderHandler.sendMessage(msg);
    }

    public void loadHistoryCallLog(int callType) {
        if (null == getContext()) {
            Message msg = Message.obtain(mMainHandler, MSG_UI_LOAD_FINISH);
            mMainHandler.sendMessage(msg);
            return;
        }
        BluetoothDeviceInfo deviceInfo = mAdapterManager.getConnectDevice();
        if (null == deviceInfo) {
            Log.e(TAG, "loadHistoryCallLog: deviceInfo==null!");
            Message msg = Message.obtain(mMainHandler, MSG_UI_LOAD_FINISH);
            mMainHandler.sendMessage(msg);
            return;
        }

        ContentResolver resolver = getContext().getContentResolver();
        Cursor cursor = null;
        Log.d(TAG, "loadHistoryCallLog: callType=" + callType);
        try {
            synchronized (this) {
                if (null != mCancellationSignal) {
                    mCancellationSignal.cancel();
                    mCancellationSignal = null;
                }
                mCancellationSignal = new CancellationSignal();
            }
            Account account = new Account(deviceInfo.getDeviceAddr(),
                    Utils.ACCOUNT_TYPE);
            Log.d(TAG, "loadHistoryCallLog: name = "+ account.name + " hashCode = " + account.hashCode());
            if (callType == 0) {
                cursor = resolver.query(CallLog.Calls.CONTENT_URI,
                        CALLLOG_PROJECTION, CallLog.Calls.PHONE_ACCOUNT_ID + "=? or " + CallLog.Calls.PHONE_ACCOUNT_ID + "=?",
                        new String[]{String.valueOf(account.hashCode()), account.name},
                        CallLog.Calls.DATE + " desc", mCancellationSignal);
            } else {
                cursor = resolver.query(CallLog.Calls.CONTENT_URI,
                        CALLLOG_PROJECTION,
                        CallLog.Calls.TYPE + "=? AND (" + CallLog.Calls.PHONE_ACCOUNT_ID + "=? or " + CallLog.Calls.PHONE_ACCOUNT_ID + "=?)",
                        new String[]{String.valueOf(callType), String.valueOf(account.hashCode()), account.name},
                        CallLog.Calls.DATE + " desc", mCancellationSignal);
                //INCOMING_TYPE  MISSED_TYPE
            }
        } catch (OperationCanceledException e) {
            if (cursor != null) {
                cursor.close();
            }
            synchronized (this) {
                mCancellationSignal = null;
            }
            Log.e(TAG, "loadHistoryCallLog: OperationCanceledException!");
            return;
        }
        ArrayList<HashMap<String, Object>> list = new ArrayList<>();
        mMainHandler.removeMessages(MSG_UI_HISTORY_ADD);
        mMainHandler.sendEmptyMessage(MSG_UI_LOAD_START);
        Log.d(TAG, "loadHistoryCallLog: send MSG_UI_LOAD_START");
        while (cursor.moveToNext() && !mLoaderHandler.hasMessages(LOADER_HISTORY)
                && !mCancellationSignal.isCanceled()) {

            int timeColIdx = cursor.getColumnIndex(CallLog.Calls.DATE);
            long time = cursor.getLong(timeColIdx);
            int numberColIdx = cursor.getColumnIndex(CallLog.Calls.NUMBER);
            String number = cursor.getString(numberColIdx);
            int typeColIdx = cursor.getColumnIndex(CallLog.Calls.TYPE);
            int type = cursor.getInt(typeColIdx);

            String ext_number = "";
            if (number.length() > 15) {//05516531751182295
                ext_number = number.substring(12);
                number = number.substring(0, 12);
            } else if (number.length() > 14) {//653-175-1182295
                ext_number = number.substring(10);
                number = number.substring(0, 10);
            }

            if (!"".equals(ext_number)) {
                StringBuffer sb = new StringBuffer();
                sb = sb.append(number);
                sb = sb.append(",");
                sb = sb.append(ext_number);
                number = sb.toString();
            }

            String displayName = Utils.getContactNameByNumber(getContext(), number,
                    deviceInfo.getDeviceAddr());

            // 第一次没找到名字，可能是因为号码格式不匹配导致，转换格式继续匹配一次
            if (TextUtils.isEmpty(displayName)) {
                if (number.length() == 11){
                    number = formatPhoneNumber(number);
                    displayName = Utils.getContactNameByNumber(getContext(), number, deviceInfo.getDeviceAddr());
                }
            }

            if (TextUtils.isEmpty(displayName)) {
                displayName = "";
            }

            HashMap<String, Object> map = new HashMap<String, Object>();
            map.put(ITEM_HISTORY_NAME, displayName);
            map.put(ITEM_HISTORY_NUMBER, number);
            map.put(ITEM_HISTORY_TIME, time);
            map.put(ITEM_HISTORY_TYPE, type);

            list.add(map);

            if (list.size() >= 10) {
                //update list every 10.
                Message msg = Message.obtain(mMainHandler, MSG_UI_HISTORY_ADD);
                msg.obj = list;
                mMainHandler.sendMessage(msg);
                list = new ArrayList<>();
            }
        }
        cursor.close();
        if (mCancellationSignal.isCanceled()) {
            mMainHandler.removeMessages(MSG_UI_HISTORY_ADD);
            Log.d(TAG, "loadHistoryCallLog: finish cancel!!");
        } else {
            Message msg = Message.obtain(mMainHandler, MSG_UI_LOAD_FINISH);
            msg.obj = list;
            mMainHandler.sendMessage(msg);
            Log.d(TAG, "loadHistoryCallLog: send MSG_UI_LOAD_FINISH");
        }
        synchronized (this) {
            mCancellationSignal = null;
        }
    }

    class HistoryListAdapter extends SimpleAdapter {
        private LayoutInflater mInflater;
        private int mSelectIdx = -1;
        private ArrayList<HashMap<String, Object>> mList;

        public HistoryListAdapter(Context context,
                                  ArrayList<HashMap<String, Object>> data, int resource,
                                  String[] from, int[] to) {
            super(context, data, resource, from, to);
            // TODO Auto-generated constructor stub
            this.mInflater = LayoutInflater.from(context);
            mSelectIdx = -1;
            mList = data;
        }

        @Override
        public HashMap<String, Object> getItem(int position) {
            if (null != mList) {
                return mList.get(position);
            }
            return null;
        }

        public void setSelect(int index) {
            mSelectIdx = index;
        }

        public int getSelect() {
            return mSelectIdx;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder = null;
            if (convertView == null) {
                holder = new ViewHolder();
                convertView = mInflater
                        .inflate(SkinUtils.getId(R.layout.history_listitem), null);
                holder.nameTextView = convertView
                        .findViewById(SkinUtils.getId(R.id.item_history_name));
                holder.phoneTextView = convertView
                        .findViewById(SkinUtils.getId(R.id.item_history_number));
                holder.timeTextView = convertView
                        .findViewById(SkinUtils.getId(R.id.item_history_time));
                holder.imageView = convertView
                        .findViewById(SkinUtils.getId(R.id.btn_call_history_img));
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            String name = "";
            String number = "";
            String time_format = "";
            int image = -1;
            if (position < mList.size()) {
                try {
                    name = (String) mList.get(position).get(ITEM_HISTORY_NAME);
                    if (TextUtils.isEmpty(name)) {
                        name = SkinUtils.getString(R.string.phonebook_unknow_name);
                    }
                    name = String.format(Locale.getDefault(), "%d. %s", (position + 1), name);
                    number = (String) mList.get(position).get(ITEM_HISTORY_NUMBER);
                    image = (Integer) mList.get(position).get(ITEM_HISTORY_TYPE);
                    long time = (Long) mList.get(position).get(ITEM_HISTORY_TIME);
                    time_format = Utils.getDateToString(time);
                } catch (Exception e) {
                    Log.d(TAG, "getView: Exception!!!!!");
                }
            }

            holder.nameTextView.setText(name);
            holder.phoneTextView.setText(number);
            holder.timeTextView.setText(time_format);
            switch (image) {
                case OUTGOING_TYPE:
                    holder.imageView.setImageResource(SkinUtils.getId(R.drawable.icon_outgoing_call));
                    break;
                case INCOMING_TYPE:
                    holder.imageView.setImageResource(SkinUtils.getId(R.drawable.icon_incoming_call));
                    break;
                default:
                    holder.imageView.setImageResource(SkinUtils.getId(R.drawable.icon_missed_call));
                    break;
            }

            return convertView;
        }

        public final class ViewHolder {
            public TextView nameTextView;
            public TextView phoneTextView;
            public TextView timeTextView;
            public ImageView imageView;
        }
    }

    private OnItemClickListener mHistoryListClickListener = new OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            mHistoryListAdapter.setSelect(position);
            mHistoryListAdapter.notifyDataSetChanged();
            HashMap<String, Object> info = mHistoryListAdapter.getItem(position);
            if (null != info) {
                String number = (String) info.get(ITEM_HISTORY_NUMBER);
                MyApplication.getInstance().getHfpclientManager().dial(number);
            }
        }
    };

    private ConnectionListener mAdapterListener = new ConnectionListener() {
        @Override
        public void onServiceConnected() {
            mAdapterManager.registerPbapCallback(mIPbapCallback);
            if (mAdapterManager.getPbapDownLoadState(
                    LocalBluetoothAdapterManager.PARAM_DOWNLOAD_MIOCH)) {
                showDownloadGroup();
            }
        }

        @Override
        public void onServiceDisconnected() {

        }
    };

    IPbapCallback mIPbapCallback = new IPbapCallback.Stub() {
        @Override
        public void onPbapDownloadStateChanged(int state, int type) throws RemoteException {
            if (type == LocalBluetoothAdapterManager.PARAM_DOWNLOAD_MIOCH) {
                if (state == LocalBluetoothAdapterManager.STATE_DOWNLOADED) {
                    if (null != mMainHandler) {
                        mMainHandler.removeMessages(MSG_DOWNLOAD_FINISH);
                        mMainHandler.sendEmptyMessage(MSG_DOWNLOAD_FINISH);
                    }
                } else if (state == LocalBluetoothAdapterManager.STATE_DOWNLOADING) {
                    if (null != mMainHandler) {
                        mMainHandler.removeMessages(MSG_DOWNLOAD_START);
                        mMainHandler.sendEmptyMessage(MSG_DOWNLOAD_START);
                    }
                } else {
                    mMainHandler.removeMessages(MSG_DOWNLOAD_FAILED);
                    Message msg = mMainHandler.obtainMessage(MSG_DOWNLOAD_FAILED);
                    msg.arg1 = state;//失败原因
                    mMainHandler.sendMessage(msg);
                }
            }
        }

        @Override
        public void onPbapConnectStateChanged(int state) throws RemoteException {

        }
    };

    /**
     * 转换电话格式为 123 1234 1234 格式
     */
    public  String formatPhoneNumber(String phone) {
        // 分割为 3-4-4 并添加空格
        return String.format("%s %s %s",
                phone.substring(0, 3),   // 前 3 位
                phone.substring(3, 7),   // 中间 4 位
                phone.substring(7)       // 最后 4 位
        );
    }
}
