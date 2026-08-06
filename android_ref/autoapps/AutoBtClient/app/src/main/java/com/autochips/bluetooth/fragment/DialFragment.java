package com.autochips.bluetooth.fragment;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.os.Bundle;
import android.os.CancellationSignal;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.OperationCanceledException;
import android.os.Parcel;
import android.os.Parcelable;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.text.Editable;
import android.text.InputType;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextWatcher;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.IdRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.autochips.bluetooth.MyApplication;
import com.autochips.bluetooth.R;
import com.autochips.bluetooth.skin.SkinUtils;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;
import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;
import com.hcn.bluetooth.api.LocalBluetoothHfpclientManager;
import com.hcn.bluetooth.api.Utils;
import com.hcn.skin.support.app.SkinCompatFragment;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Locale;

public class DialFragment extends SkinCompatFragment implements OnClickListener,
        View.OnLongClickListener, OnItemClickListener {
    private static final String TAG = "DialFragment";
    /**
     * 通话状态改变
     * BluetoothHeadsetClient.ACTION_CALL_CHANGED
     */
    public static final String ACTION_CALL_CHANGED = "android.bluetooth.headsetclient.profile.action.AG_CALL_CHANGED";
    /**
     * BluetoothHeadsetClient.EXTRA_CALL
     */
    public static final String EXTRA_CALL = "android.bluetooth.headsetclient.extra.CALL";
    public static final int MSG_CLEAR_PBLIST = 1;
    public static final int MSG_UI_PB_SEARCH_ADD = 2;
    public static final int MSG_LOAD_CONTACT = 3;
    public static final int MSG_PB_SEARCH = 4;
    private Context mParentContext;

    private View mViewRoot;
    private ListView mListViews;
    EditText callNumEditText;
    private ListAdapter mPBListAdapter;
    private Toast mToast = null;

    private static final int LOAD_CONTACT_ON_CONNECTED_DELAY = 200;
    private static final String ITEM_PHONEBOOK_NAME = "item_phonebook_name";
    private static final String ITEM_PHONEBOOK_NUMBER = "item_phonebook_number";
    private static final String ITEM_PHONEBOOK_PATH = "item_phonebook_path";
    private static String SEARCH_SELECTION = String.format(Locale.ENGLISH,
            "%s LIKE ? AND %s=? AND %s =?", Phone.NUMBER, ContactsContract.RawContacts.ACCOUNT_NAME,
            ContactsContract.RawContacts.ACCOUNT_TYPE);

    private ArrayList<HashMap<String, String>> mPBList;
    private contactLoaderHandle mContactLoaderHandle;
    private HandlerThread mcontactLoaderThread;
    private CancellationSignal mSearchCancellationSignal = null;

    private LocalBluetoothAdapterManager mAdapterManager;

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        if (SkinUtils.useSkinPackage()) {
            mParentContext = SkinUtils.getContext();
        } else {
            mParentContext = context;
        }
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mAdapterManager = MyApplication.getInstance().getAdapterManager();
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(android.content.Context context, Intent intent) {
            String action = intent.getAction();

            if (action.equals(LocalBluetoothAdapterManager.ACTION_CONNECTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothProfile.STATE_DISCONNECTED);
                if (state == BluetoothProfile.STATE_CONNECTED) {
                    mContactLoaderHandle.sendEmptyMessageDelayed(MSG_LOAD_CONTACT,
                            LOAD_CONTACT_ON_CONNECTED_DELAY);
                } else if (state == BluetoothProfile.STATE_DISCONNECTED) {
                    clearPBList();
                    deleteAllDialPadString();
                }
            }else if (action.equals(ACTION_CALL_CHANGED)){
                getBluetoothHeadsetClientCallAndClear(intent);
            }
        }
    };

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        // mViewRoot = inflater.inflate(R.layout.bt_call, container, false);
        mViewRoot = super.onCreateView(inflater, container, savedInstanceState);
        mcontactLoaderThread = new HandlerThread("contactloader_thread");
        mcontactLoaderThread.start();
        mContactLoaderHandle = new contactLoaderHandle(mcontactLoaderThread.getLooper());

        initView();
        IntentFilter filter = new IntentFilter();
        filter.addAction(LocalBluetoothAdapterManager.ACTION_CONNECTION_STATE_CHANGED);
        filter.addAction(ACTION_CALL_CHANGED);
        getActivity().registerReceiver(mReceiver, filter);
        return mViewRoot;
    }

    @Override
    public void onBindViewData() {

    }

    @Override
    public int getLayoutRes() {
        return R.layout.bt_call;
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        getActivity().unregisterReceiver(mReceiver);
    }

    @Override
    public void onDetach() {
        super.onDetach();
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
        Log.d(TAG, "onKeyDown:keyCode=" + keyCode);
        switch (keyCode) {
            case KeyEvent.KEYCODE_0:
                addDialPadInputString("0");
                break;
            case KeyEvent.KEYCODE_1:
                addDialPadInputString("1");
                break;
            case KeyEvent.KEYCODE_2:
                addDialPadInputString("2");
                break;
            case KeyEvent.KEYCODE_3:
                addDialPadInputString("3");
                break;
            case KeyEvent.KEYCODE_4:
                addDialPadInputString("4");
                break;
            case KeyEvent.KEYCODE_5:
                addDialPadInputString("5");
                break;
            case KeyEvent.KEYCODE_6:
                addDialPadInputString("6");
                break;
            case KeyEvent.KEYCODE_7:
                addDialPadInputString("7");
                break;
            case KeyEvent.KEYCODE_8:
                addDialPadInputString("8");
                break;
            case KeyEvent.KEYCODE_9:
                addDialPadInputString("9");
                break;

            default:
                break;
        }
        return true;
    }

    public boolean onKeyUp(int keyCode, KeyEvent event) {
        Log.d(TAG, "onKeyUp:keyCode=" + keyCode);
        return true;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
    }

    @Override
    public boolean onLongClick(View view) {
        int id =SkinUtils.getViewId(view);
        if (id == R.id.btn_del_num) {
            deleteAllDialPadString();
            return true;
        } else if (id == R.id.btn_call_zero) {
            addDialPadInputString("+");
            return true;
        }
        return false;
    }

    @Override
    public void onClick(View v) {
        int viewId = SkinUtils.getViewId(v);
        switch (viewId) {
            case R.id.btn_call:
                onCallClick();
                break;

            case R.id.btn_call_zero:
                addDialPadInputString("0");
                break;

            case R.id.btn_call_one:
                addDialPadInputString("1");
                break;

            case R.id.btn_call_two:
                addDialPadInputString("2");
                break;

            case R.id.btn_call_three:
                addDialPadInputString("3");
                break;

            case R.id.btn_call_four:
                addDialPadInputString("4");
                break;

            case R.id.btn_call_five:
                addDialPadInputString("5");
                break;

            case R.id.btn_call_six:
                addDialPadInputString("6");
                break;

            case R.id.btn_call_seven:
                addDialPadInputString("7");
                break;

            case R.id.btn_call_eight:
                addDialPadInputString("8");
                break;

            case R.id.btn_call_nine:
                addDialPadInputString("9");
                break;

            case R.id.btn_call_asterisk:
                addDialPadInputString("*");
                break;

            case R.id.btn_call_pound:
                addDialPadInputString("#");
                break;

            case R.id.btn_del_num:
                deleteOneDialPadString();
                break;

            default:
                break;
        }
    }

    private void initView() {
        try {
            mListViews = mViewRoot.findViewById(SkinUtils.getId(R.id.listview_calls));
            mPBList = new ArrayList<>();
            mPBListAdapter = new ListAdapter(mParentContext, mPBList);
            mListViews.setAdapter(mPBListAdapter);
            mListViews.setOnItemClickListener(this);

            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call)).setOnClickListener(this);
            //findViewById(SkinUtils.getId(R.id.btn_recall).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call_zero)).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call_zero)).setOnLongClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call_one)).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call_two)).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call_three)).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call_four)).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call_five)).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call_six)).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call_seven)).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call_eight)).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call_nine)).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call_asterisk)).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_call_pound)).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_del_num)).setOnClickListener(this);
            mViewRoot.findViewById(SkinUtils.getId(R.id.btn_del_num)).setOnLongClickListener(this);

        } catch (NullPointerException e) {
            e.printStackTrace();
        }

        callNumEditText = mViewRoot.findViewById(SkinUtils.getId(R.id.text_call_info));
        callNumEditText.addTextChangedListener(mSearchTextWatcher);
        if (callNumEditText != null) {
            callNumEditText.setInputType(InputType.TYPE_NULL);
            callNumEditText.setText("");
        }

    }

    private void onCallClick() {
        if (getCallNumber().length() == 0) {
            addDialPadInputString(getLastDialNumber());
        } else {
            call(getCallNumber());
        }
    }

    @SuppressLint("ResourceType")
    private void call(String number) {
        if (!mAdapterManager.isBluetoothConnected()) {
            Log.e(TAG, "device is not connected!");
            return;
        }

        if (checkCallNumberVaild()) {
            MyApplication.getInstance().getHfpclientManager().dial(number);
        } else {
            showToast(SkinUtils.getId(R.string.str_call_number_is_empty));
        }
    }

    private String getCallNumber() {
        String callNum = "";
        if (callNumEditText != null) {
            callNum = callNumEditText.getText().toString();
        }

        return callNum;
    }

    private boolean checkCallNumberVaild() {
        boolean isValid = false;

        if (callNumEditText != null) {
            Editable callnumstrEdit = callNumEditText.getText();

            if (callnumstrEdit.length() > 0) {
                isValid = true;
            }
        }

        return isValid;
    }

    private void addDialPadInputString(CharSequence str) {
        if (str == null) {
            return;
        }

        if (callNumEditText == null) {
            return;
        }
        int index = callNumEditText.getSelectionStart();
        Editable callnumstrEdit = callNumEditText.getText();

        if (callnumstrEdit.length() > 14) {
            showToast(SkinUtils.getId(R.string.str_call_number_is_too_long));
            return;
        }
        if (index < 0 || index > callnumstrEdit.length()) {
            callnumstrEdit.append(str);
        } else {
            callnumstrEdit.insert(index, str);
        }
        callNumEditText.setText(callnumstrEdit);
        callNumEditText.setSelection(index + str.length());
    }

    private void deleteOneDialPadString() {
        if (callNumEditText == null) {
            return;
        }

        int index = callNumEditText.getSelectionStart();
        Editable callnumstrEdit = callNumEditText.getText();

        if (index >= 1) {
            callnumstrEdit.delete(index - 1, index);
        } else {
            return;
        }
        callNumEditText.setText(callnumstrEdit);
        callNumEditText.setSelection(index - 1);
    }

    private void deleteAllDialPadString() {
        if (callNumEditText == null) {
            return;
        }

        callNumEditText.setText("");
    }

    private void clearPBList() {
        if (null != mPBList) {
            mPBList.clear();
        }
        if (null != mPBListAdapter) {
            mPBListAdapter.notifyDataSetChanged();
        }
    }

    private final class SearchResult {
        public ArrayList<HashMap<String, String>> pbList;
        public String searchKey;
    }

    private TextWatcher mSearchTextWatcher = new TextWatcher() {
        String mSearchKey = "";

        @Override
        public void afterTextChanged(Editable s) {
            String search = callNumEditText.getText().toString();
            Log.d(TAG, "afterTextChanged: search=" + search);
            if ((search.length() >= 2)) {
                if (!search.equals(mSearchKey)) {
                    mSearchKey = search;
                    clearPBList();
                    Message msg = mContactLoaderHandle.obtainMessage(MSG_PB_SEARCH, mSearchKey);
                    mContactLoaderHandle.removeMessages(MSG_PB_SEARCH);
                    mContactLoaderHandle.sendMessage(msg);
                    synchronized (DialFragment.this) {
                        if (mSearchCancellationSignal != null) {
                            mSearchCancellationSignal.cancel();
                        }
                    }
                }
            } else {
                clearPBList();
            }
        }

        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
        }
    };

    @SuppressWarnings("HandlerLeak")
    private Handler mHandler = new Handler() {

        @SuppressWarnings("unchecked")
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_CLEAR_PBLIST:
                    clearPBList();
                    break;

                case MSG_UI_PB_SEARCH_ADD:
                    SearchResult result = (SearchResult) msg.obj;
                    mPBList.addAll(result.pbList);
                    mPBListAdapter.setSearchText(result.searchKey);
                    mPBListAdapter.notifyDataSetChanged();
                    break;

                default:
                    break;
            }
        }
    };

    private class contactLoaderHandle extends Handler {

        public contactLoaderHandle(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_PB_SEARCH:
                    try {
                        searchContact((String) msg.obj);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    break;

                default:
                    break;
            }
        }

        public void searchContact(String key) throws Exception {
            BluetoothDeviceInfo deviceInfo = mAdapterManager.getConnectDevice();
            if (null == deviceInfo) {
                Log.e(TAG, "searchContact device is not connected!");
                mHandler.sendEmptyMessage(MSG_CLEAR_PBLIST);
                return;
            }
            SearchResult result;
            Log.d(TAG, "searchContact key : " + key);

            ArrayList<HashMap<String, String>> pbList = new ArrayList<>();
            ContentResolver resolver = getContext().getContentResolver();
            Cursor cursor = null;
            String nameColumn = Phone.DISPLAY_NAME;
            if (Utils.isReverseName()) {
                nameColumn = Phone.DISPLAY_NAME_ALTERNATIVE;
            }
            try {
                synchronized (DialFragment.this) {
                    mSearchCancellationSignal = new CancellationSignal();
                }
                cursor = resolver.query(Phone.CONTENT_URI, new String[]{nameColumn, Phone.NUMBER},
                        SEARCH_SELECTION, new String[]{"%" + key + "%", deviceInfo.getDeviceAddr(),
                                Utils.ACCOUNT_TYPE}, null, mSearchCancellationSignal);

                while (cursor.moveToNext()) {
                    String phone_number = cursor.getString(cursor.getColumnIndex(Phone.NUMBER));
                    String name = cursor.getString(cursor.getColumnIndex(nameColumn));

                    HashMap<String, String> map = new HashMap<>();
                    map.put(ITEM_PHONEBOOK_NAME, name);
                    map.put(ITEM_PHONEBOOK_NUMBER, phone_number);
                    map.put(ITEM_PHONEBOOK_PATH, "");
                    pbList.add(map);

                    if (pbList.size() >= 10) {
                        //update list every 10.
                        result = new SearchResult();
                        result.searchKey = key;
                        result.pbList = pbList;
                        Message msg = Message.obtain(mHandler, MSG_UI_PB_SEARCH_ADD, result);
                        mHandler.sendMessage(msg);
                        pbList = new ArrayList<>();
                    }
                }
                cursor.close();
            } catch (OperationCanceledException e) {
                e.printStackTrace();
                if (cursor != null) {
                    cursor.close();
                }
                synchronized (DialFragment.this) {
                    mSearchCancellationSignal = null;
                }
                return;
            } finally {
                synchronized (DialFragment.this) {
                    mSearchCancellationSignal = null;
                }
            }
            if (pbList.size() != 0) {
                result = new SearchResult();
                result.searchKey = key;
                result.pbList = pbList;
                Message msg = Message.obtain(mHandler, MSG_UI_PB_SEARCH_ADD, result);
                mHandler.sendMessage(msg);
            }
        }
    }

    class ViewHolder {

        public TextView mContactName;
        public TextView mContactNum;

        public ViewHolder(TextView name, TextView num) {

            mContactName = name;
            mContactNum = num;

        }
    }

    class ListAdapter extends BaseAdapter {

        private LayoutInflater mlayoutInflater;
        private Context mContext;
        private ArrayList<HashMap<String, String>> mPBDatas;
        private String mSearchText;

        public ListAdapter(Context context, ArrayList<HashMap<String, String>> mDatas) {

            this.mContext = context;
            this.mPBDatas = mDatas;
            mlayoutInflater = LayoutInflater.from(mContext);
        }

        public void setSearchText(String text) {
            mSearchText = text;
        }

        @Override
        public int getCount() {
            return mPBDatas.size();
        }

        @Override
        public Object getItem(int position) {
            return mPBDatas.get(position);
        }

        @Override
        public long getItemId(int arg0) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {

            ViewHolder mViewHolder = null;
            if (null == convertView) {
                convertView = SkinUtils.inflate(R.layout.dial_listitem, parent, false);
                TextView contacts = convertView.findViewById(SkinUtils.getId(R.id.dial_name));
                TextView phoneNum = convertView.findViewById(SkinUtils.getId(R.id.dial_num));
                mViewHolder = new ViewHolder(contacts, phoneNum);
                convertView.setTag(mViewHolder);
            } else {
                mViewHolder = (ViewHolder) convertView.getTag();
            }

            String number = mPBDatas.get(position).get(ITEM_PHONEBOOK_NUMBER);
            int index;
            if (number.contains(mSearchText)) {
                index = number.indexOf(mSearchText);
                SpannableStringBuilder style2 = new SpannableStringBuilder(number);
                style2.setSpan(new ForegroundColorSpan(mContext.getResources()
                        .getColor(SkinUtils.getId(R.color.dial_tv_color))), index, index
                        + mSearchText.length(), Spannable.SPAN_EXCLUSIVE_INCLUSIVE);
                mViewHolder.mContactNum.setText(style2);
            } else {
                mViewHolder.mContactNum.setText(mPBDatas.get(position).get(ITEM_PHONEBOOK_NUMBER));
            }

            mViewHolder.mContactName.setText(mPBDatas.get(position).get(ITEM_PHONEBOOK_NAME));

            return convertView;
        }

    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        call(mPBList.get(arg2).get(ITEM_PHONEBOOK_NUMBER));
    }

    private String getLastDialNumber() {
        String num = "";
        LocalBluetoothHfpclientManager hfpclientManager =
                MyApplication.getInstance().getHfpclientManager();
        if (hfpclientManager != null) {
            num = hfpclientManager.getLastCall();
        }
        return num;
    }

    @SuppressLint("ResourceType")
    private void showToast(@IdRes int resID) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(mParentContext, resID, Toast.LENGTH_SHORT);
        mToast.show();
    }

    /**
     * 获取蓝牙电话状态 并清空
     * 当挂断电话时清空 拨号界面 输入框中的内容
     * 不直接从 编译的 framework.jar 中获取 BluetoothHeadsetClientCall 的原因是：COMMON 分支共有，防止其他平台出错
     * @param intent 意图
     */
    private void getBluetoothHeadsetClientCallAndClear(Intent intent) {
        if (intent == null) {
            Log.e(TAG, "Intent is null");
            return;
        }

        try {
            // 尝试直接获取 BluetoothHeadsetClientCall 对象
            Parcelable callParcelable = intent.getParcelableExtra(EXTRA_CALL);
            if (callParcelable == null) {
                Log.e(TAG, "EXTRA_CALL is null or not a Parcelable object");
                return;
            }

            // 反射获取类信息，确保 call 对象属于 BluetoothHeadsetClientCall
            Class<?> bluetoothHeadsetClientCallClass = Class.forName("android.bluetooth.BluetoothHeadsetClientCall");
            if (!bluetoothHeadsetClientCallClass.isInstance(callParcelable)) {
                Log.e(TAG, "EXTRA_CALL is not an instance of BluetoothHeadsetClientCall");
                return;
            }

            Object call = bluetoothHeadsetClientCallClass.cast(callParcelable);

            // 反射调用 getState 方法
            Method getStateMethod = bluetoothHeadsetClientCallClass.getDeclaredMethod("getState");
            int state = (int) getStateMethod.invoke(call);
            Log.v(TAG, "state = " + state);

            // 获取 CALL_STATE_TERMINATED 常量值
            Field callStateTerminatedField = bluetoothHeadsetClientCallClass.getDeclaredField("CALL_STATE_TERMINATED");
            callStateTerminatedField.setAccessible(true);
            int CALL_STATE_TERMINATED = callStateTerminatedField.getInt(null);
            Log.v(TAG, "CALL_STATE_TERMINATED = " + CALL_STATE_TERMINATED);
            // 判断状态是否为 CALL_STATE_TERMINATED
            if (state == CALL_STATE_TERMINATED) {
                if (callNumEditText != null) {
                    callNumEditText.setText("");
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Reflection error: ", e);
        }
    }

}
