package com.autochips.bluetooth.fragment;

import android.accounts.Account;
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
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Phone;

import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.ProgressBar;
import android.widget.SimpleAdapter;
import android.widget.TextView;

import androidx.annotation.NonNull;

import com.autochips.bluetooth.IFragmentCallback;
import com.autochips.bluetooth.MyApplication;
import com.autochips.bluetooth.R;
import com.autochips.bluetooth.skin.SkinUtils;
import com.autochips.bluetooth.view.SideBar;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;
import com.hcn.bluetooth.api.ConnectionListener;
import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;
import com.hcn.bluetooth.api.Utils;
import com.hcn.bluetooth.service.IPbapCallback;
import com.hcn.skin.support.app.SkinCompatFragment;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;


public class PBFragment extends SkinCompatFragment implements View.OnClickListener,
        SideBar.OnTouchingLetterChangedListener {
    public static final String TAG = "PBFragment";
    private static final String ITEM_PHONEBOOK_NAME = "item_phonebook_name";
    private static final String ITEM_PHONEBOOK_NUMBER = "item_phonebook_number";
    private static final String ITEM_PHONEBOOK_PATH = "item_phonebook_path";
    private static final String ITEM_PHONEBOOK_LABEL = "item_phonebook_label";
    //用于数据库查询名称首字母
    private static final String PHONEBOOK_LABEL = "phonebook_label";
    private static final String PHONEBOOK_LABEL_ALT = "phonebook_label_alt";

    private Button mUpdatePBCtrl = null;

    private AbsListView mPBGridView = null;
    private PBListAdapter mPBListAdapter;
    public ArrayList<HashMap<String, String>> mPBList = new ArrayList<>();

    private MainHandler mHandler;
    private static final int MSG_DOWNLOAD_START = 0;//从手机下载数据状态
    private static final int MSG_DOWNLOAD_FINISH = 1;
    private static final int MSG_DOWNLOAD_FAILED = 2;
    private static final int MSG_UI_LOAD_START = 3;//从数据库读取数据状态
    private static final int MSG_UI_LOAD_FINISH = 4;
    private static final int MSG_UI_PB_ADD = 5;
    private static final int MSG_UI_HIDE_DOWNLOAD_GROUP = 6;

    private LoaderHandler mLoaderHandler;
    private HandlerThread mLoaderThread;
    //通知LoaderHandler从数据库读取联系人数据
    private static final int LOADER_CONTRACT = 1;
    //通知LoaderHandler从数据库删除联系人数据
    private static final int DELETE_CONTACTS = 2;

    private String mSearchKey = "";
    private CancellationSignal mCancellationSignal = null;

    //ui
    private LinearLayout mDownloadLayout;
    private TextView mDownloadTextView;
    //mSearchText的hint显示联系人数量
    private EditText mSearchText;
    private SideBar mSideBar;
    //mEditSearchText配合mContactCountTextView一起使用，textview显示联系人数量
    private TextView mContactCountTextView;
    private EditText mEditSearchText;

    private LocalBluetoothAdapterManager mAdapterManager;
    private Context mContext;
    private View mClearContacts;

    private View root = null;
    private PopupWindow mClearContactsDialog = null;
    /**
     *     0：正在下载 1：不下载或下载完成
     */
    public int mDownLoadState =1;
    /**
     * 通知Activity更新背景
     */
    private IFragmentCallback fragmentCallback = null;

    @Override
    public void onTouchingLetterChanged(String s) {
        Log.e(TAG, "onTouchingLetterChanged:" + s);
        if (mPBList != null && mPBList.size() > 0) {
            int i;
            for (i = 0; i < mPBList.size(); i++) {
                String label = mPBList.get(i).get(ITEM_PHONEBOOK_LABEL);
                if (!TextUtils.isEmpty(label)) {
                    if ("#".equals(s)) {
                        if (label.equals(s)) {
                            break;
                        }
                    } else {
                        char c = label.charAt(0);
                        if (c >= 'A' && c <= 'Z') {
                            if (label.compareTo(s) >= 0) {
                                Log.d(TAG, "select=" + s + " letter=" + label + " index=" + i);
                                break;
                            }
                        }
                    }
                }
            }

            if (mPBList.size() > 0 && i > mPBList.size() - 1) {
                i = mPBList.size() - 1;
            }
            Log.d(TAG, "index=" + i);
            mPBGridView.setSelection(Math.min(i, mPBList.size() - 1));
        }

    }

    private static class MainHandler extends Handler {
        private WeakReference<PBFragment> mWeakReferencePBFragment;

        public MainHandler(PBFragment fragment) {
            super(Looper.getMainLooper());
            mWeakReferencePBFragment = new WeakReference<>(fragment);
        }

        @Override
        public void handleMessage(Message msg) {
            PBFragment fragment = mWeakReferencePBFragment.get();
            if (fragment != null) {
                int what = msg.what;
                switch (what) {
                    case MSG_DOWNLOAD_START:
                        fragment.syncDownLoadState(0);
                        fragment.showPBDownloadGroup();
                        fragment.cancelLoadContact();
                        fragment.clearContactList();
                        break;
                    case MSG_DOWNLOAD_FINISH:
                        fragment.syncDownLoadState(1);
                        fragment.startLoadContact("");
                        break;
                    case MSG_DOWNLOAD_FAILED:
                        fragment.syncDownLoadState(1);
                        int error = msg.arg1;
                        if (error == LocalBluetoothAdapterManager.STATE_FAILED_UNKNOWN
                                || error == LocalBluetoothAdapterManager.STATE_FAILED_NO_CONNECT) {
                            Utils.showToast(fragment.getContext(),
                                    R.string.str_download_phonebook_fail);
                        }
                        sendEmptyMessage(MSG_UI_HIDE_DOWNLOAD_GROUP);
                        break;
                    case MSG_UI_LOAD_START:
                        fragment.clearContactList();
                        break;
                    case MSG_UI_PB_ADD:
                        fragment.mPBList.addAll((List<HashMap<String, String>>) msg.obj);
                        fragment.changeDownloadHint(fragment.mPBList.size());
                        fragment.mPBListAdapter.notifyDataSetChanged();
                        break;
                    case MSG_UI_LOAD_FINISH:
                        if (null != msg.obj) {
                            fragment.mPBList.addAll((List<HashMap<String, String>>) msg.obj);
                        }
                        fragment.changeDownloadHint(fragment.mPBList.size());

                        if (fragment.mContactCountTextView != null) {
                            fragment.mContactCountTextView.setText(String.format(SkinUtils.getString(R.string.bt_phonebook_search_hint),
                                    fragment.mPBList.size()));
                        }

                        if (fragment.mSearchText != null) {
                            fragment.mSearchText.setHint(String.format(SkinUtils.getString(R.string.bt_phonebook_search_hint),
                                    fragment.mPBList.size()));
                        }

                        fragment.mPBListAdapter.notifyDataSetChanged();
                        sendEmptyMessageDelayed(MSG_UI_HIDE_DOWNLOAD_GROUP, 500);
                        Log.d(TAG, "MSG_UI_LOAD_FINISH size=" + fragment.mPBList.size());
                        break;
                    case MSG_UI_HIDE_DOWNLOAD_GROUP:
                        fragment.hidePBDownloadGroup();
                        break;
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
            switch (msg.what) {
                case LOADER_CONTRACT:
                    try {
                        loadContact((String) msg.obj);
                    } catch (Exception e) {
                        Log.d(TAG, "handleMessage: MSG_PB_SEARCH Exception:" + e.toString());
                        mHandler.sendEmptyMessage(MSG_UI_HIDE_DOWNLOAD_GROUP);
                    }
                    break;
                case DELETE_CONTACTS:
                    try {
                        deleteContacts();
                    } catch (Exception e) {
                        Log.d(TAG, "handleMessage: DELETE_CONTACTS Exception");
                    }
                    break;
                default:
                    break;
            }
        }
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        Log.d(TAG, "PBFragment onAttach");
        mHandler = new MainHandler(this);
        if (SkinUtils.useSkinPackage()) {
            mContext = SkinUtils.getContext();
        } else {
            mContext = context;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        IntentFilter filter = new IntentFilter();
        filter.addAction(LocalBluetoothAdapterManager.ACTION_CONNECTION_STATE_CHANGED);
        getActivity().registerReceiver(pbReceiver, filter);

        mAdapterManager = MyApplication.getInstance().getAdapterManager();
        mAdapterManager.addConnectListener(mAdapterListener);
        mAdapterManager.registerPbapCallback(mIPbapCallback);
        mLoaderThread = new HandlerThread("loader_thread");
        mLoaderThread.start();
        mLoaderHandler = new LoaderHandler(mLoaderThread.getLooper());
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");
        // View root = inflater.inflate(R.layout.bt_phonebook, container, false);
        root = super.onCreateView(inflater, container, savedInstanceState);
        initView(root);
        initAdapterView();
        return root;
    }

    @Override
    public void onBindViewData() {

    }

    @Override
    public int getLayoutRes() {
        return R.layout.bt_phonebook;
    }

    private void initAdapterView() {
        mPBListAdapter = new PBListAdapter(mContext, mPBList, SkinUtils.getId(R.layout.phonebook_listitem),
                new String[]{
                        ITEM_PHONEBOOK_NAME, ITEM_PHONEBOOK_NUMBER, ITEM_PHONEBOOK_PATH
                },
                new int[]{
                        SkinUtils.getId(R.id.item_phonebook_name), SkinUtils.getId(R.id.item_phonebook_number),
                        SkinUtils.getId(R.id.btn_phonebook_path)
                }
        );
        mPBGridView.setAdapter(mPBListAdapter);
        mPBGridView.setOnItemClickListener(mPhoneBookListClickListener);
        mPBGridView.setOnScrollListener(new AbsListView.OnScrollListener() {
            int first_position = 0;

            @Override
            public void onScrollStateChanged(AbsListView absListView, int scrollState) {
                Log.d(TAG, "onScrollStateChanged: state=" + scrollState + " pos="
                        + mPBGridView.getFirstVisiblePosition());
                if (scrollState == SCROLL_STATE_IDLE) {
                    HashMap<String, String> item = mPBListAdapter.getItem(first_position);
                    if (null == item) {
                        return;
                    }
                    String label = item.get(ITEM_PHONEBOOK_LABEL);
                    if (!TextUtils.isEmpty(label)) {
                        char c = label.charAt(0);
                        if (c >= 'A' && c <= 'Z') {
                            int pos = c - 'A';
                            if (mSideBar != null) {
                                mSideBar.setChoose(pos);
                            }
                        } else {
                            int pos = 26;//#的下标
                            if (mSideBar != null) {
                                mSideBar.setChoose(pos);
                            }
                        }
                    }
                }
            }

            @Override
            public void onScroll(AbsListView view, int firstItem, int visibleCount,
                                 int totalCount) {
                first_position = firstItem;
            }
        });
    }

    private void initView(View root) {
        if (root == null) {
            Log.e(TAG, "initView root is null");
            return;
        }
        mUpdatePBCtrl = root.findViewById(SkinUtils.getId(R.id.bt_phonebook_syn));
        mUpdatePBCtrl.setOnClickListener(this);

        //begin: za01皮肤包新增控件
        mContactCountTextView = root.findViewById(SkinUtils.getId(R.id.tv_contact_count));
        mClearContacts = root.findViewById(SkinUtils.getId(R.id.btn_contact_clear));
        if (mClearContacts != null) {
            mClearContacts.setOnClickListener(this);
        }
        mEditSearchText = root.findViewById(SkinUtils.getId(R.id.edit_search_text));
        if (mEditSearchText != null) {
            mEditSearchText.clearFocus();
            mEditSearchText.setHint(SkinUtils.getString(R.string.bt_phonebook_text_hint));
            mEditSearchText.addTextChangedListener(mSearchEditWatcher);
        }
        //end

        mSearchText = root.findViewById(SkinUtils.getId(R.id.search_edittext));
        if (mSearchText != null) {
            mSearchText.clearFocus();
            mSearchText.setHint(SkinUtils.getString(R.string.bt_phonebook_search_hint));
            mSearchText.addTextChangedListener(mSearchEditWatcher);
        }

        mSideBar = root.findViewById(SkinUtils.getId(R.id.phonebook_sidebar));
        if (mSideBar != null) {
            mSideBar.setOnTouchingLetterChangedListener(this);
            if (Utils.isPortrait(getContext())) {
                mSideBar.setDirection(1);
                mSideBar.setTextSize(13);
            }
        }

        mDownloadLayout = root.findViewById(SkinUtils.getId(R.id.download_phonebook_layout));

        ProgressBar progressBar = root.findViewById(SkinUtils.getId(R.id.progressbar_download_phonebook));
        if (progressBar != null){
            progressBar.setIndeterminate(false);
        }
        mDownloadTextView = root.findViewById(SkinUtils.getId(R.id.tv_download_phonebook_text));
        mPBGridView = root.findViewById(SkinUtils.getId(R.id.phonebook_listview));
        mPBList = new ArrayList<>();
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
    }

    @Override
    public void setUserVisibleHint(boolean isVisibleToUser) {
        Log.d(TAG, "setUserVisibleHint:" + isVisibleToUser);
        super.setUserVisibleHint(isVisibleToUser);

        if (isVisibleToUser) {
            if (mAdapterManager != null && mAdapterManager.isReady()) {
                BluetoothDeviceInfo deviceInfo = mAdapterManager.getConnectDevice();
                if (null != deviceInfo) {
                    if (mAdapterManager.getPbapDownLoadState(
                            LocalBluetoothAdapterManager.PARAM_DOWNLOAD_PB)) {
                        showPBDownloadGroup();
                    } else {
                        hidePBDownloadGroup();
                        if (mPBList.isEmpty()) {
                            startLoadContact("");
                        }
                    }
                } else {
                    clearContactList();
                }
            }
        } else {
            hideSoftInput();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause");
        super.onPause();
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        Log.d(TAG, "onHiddenChanged hidden=" + hidden);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        Log.d(TAG, "onDestroyView");
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        if (null != getContext()) {
            getContext().unregisterReceiver(pbReceiver);
        }
        if (null != mAdapterManager) {
            mAdapterManager.unregisterPbapCallback(mIPbapCallback);
            mAdapterManager.removeConnectListener(mAdapterListener);
        }
        if (null != mLoaderThread) {
            mLoaderThread.quitSafely();
        }
        super.onDestroy();
    }

    @Override
    public void onDetach() {
        super.onDetach();
        Log.d(TAG, "onDetach");
        if (null != mHandler) {
            mHandler.removeCallbacksAndMessages(null);
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        int viewId = SkinUtils.getViewId(v);
        switch (viewId) {
            case R.id.bt_phonebook_syn:
                hideSoftInput();
                if (!mAdapterManager.pbapStartDownLoad(
                        LocalBluetoothAdapterManager.PARAM_DOWNLOAD_PB)) {
                    Utils.showToast(mContext, SkinUtils.getId(R.string.str_download_wait_history_tip));
                }
                break;
            case R.id.btn_contact_clear:
                if (mDownLoadState == 0) {
                    Log.d(TAG, "phone book downloading...");
                    Utils.showToast(mContext, SkinUtils.getId(R.string.str_download_wait_phonebook_tip));
                    return;
                }
                showContactsDeleteDialog();
                break;
            case R.id.contacts_delete_confirm:
                clearContacts();
                hideContactsDeleteDialog();
                break;
            case R.id.contacts_delete_cancel:
                hideContactsDeleteDialog();
                break;
            default:
                break;
        }
    }

    /**
     * 删除联系人和通话记录
     */
    private void clearContacts() {
        Log.d(TAG, "clearContacts");
        //清空联系人
        if (mPBList.size() > 0) {
            clearContactList();
            if (mLoaderHandler != null) {
                Message msg = Message.obtain(mLoaderHandler, DELETE_CONTACTS);
                mLoaderHandler.removeMessages(DELETE_CONTACTS);
                mLoaderHandler.sendMessage(msg);
            }
        }
       //清空通话记录,
        if (fragmentCallback != null) {
            fragmentCallback.clearRecordList();
        }

        //[8581][8163]清空系统属性：bluetooth apk需要根据该属性进行判断是否重新下载联系人
        //该属性用于相同设备断开不删除联系人数据库，下次连接后可以快速获取到联系人
        SkinUtils.setSystemProp("persist.bluetooth.preaddress", "unkown");

    }

    /**
     * 同步电话本下载状态
     * @param state
     */
    public void syncDownLoadState(int state){
        mDownLoadState = state;
    }

    /**
     * 删除联系人数据库数据
     */
    private void deleteContacts() {
        if (null == mContext) {
            Log.e(TAG, "deleteContacts：mContext()==null!");
            return;
        }

        BluetoothDeviceInfo deviceInfo = mAdapterManager.getConnectDevice();
        if (null == deviceInfo) {
            Log.e(TAG, "deleteContacts deviceInfo==null!");
            return;
        }

        Account account = new Account(deviceInfo.getDeviceAddr(),
                Utils.ACCOUNT_TYPE);
        try {
            Log.d(TAG, "To delete RawContacts database data");
            // need to check call table is exist ?
            if (mContext.getContentResolver() == null) {
                Log.d(TAG, "Contacts ContentResolver is not found");
                return;
            }
            mContext.getContentResolver().delete(ContactsContract.RawContacts.CONTENT_URI,
                    ContactsContract.RawContacts.ACCOUNT_NAME + "=?", new String[]{account.name});

        } catch (IllegalArgumentException e) {
            Log.d(TAG, "Contacts could not be deleted, they may not exist yet.");
        }
    }

    /**
     * 显示通讯录删除确认框
     */
    private void showContactsDeleteDialog() {
        if (null == mContext) {
            Log.e(TAG, "showContactsDeleteDialog: mContext is null");
            return;
        }
        if (mClearContactsDialog == null) {
            View view = SkinUtils.inflate(R.layout.contacts_delete_confirm_dialog);
            if (view != null) {
                mClearContactsDialog = new PopupWindow(view, ViewGroup.LayoutParams.WRAP_CONTENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT);
                mClearContactsDialog.setOutsideTouchable(true);
                mClearContactsDialog.setFocusable(true);
                mClearContactsDialog.setAnimationStyle(R.style.PopupAnimation);
                mClearContactsDialog.setOnDismissListener(new PopupWindow.OnDismissListener() {
                    @Override
                    public void onDismiss() {
                        if (fragmentCallback != null) {
                            fragmentCallback.updateBackground(false);
                        }
                    }
                });

                Button btnConfirm = view.findViewById(SkinUtils.getId(R.id.contacts_delete_confirm));
                if (btnConfirm != null) {
                    btnConfirm.setOnClickListener(PBFragment.this);
                }
                Button btnCancel = view.findViewById(SkinUtils.getId(R.id.contacts_delete_cancel));
                if (btnCancel != null) {
                    btnCancel.setOnClickListener(PBFragment.this);
                }
            }
        }

        if (fragmentCallback != null) {
            fragmentCallback.updateBackground(true);
        }
        if (mClearContactsDialog != null) {
            mClearContactsDialog.showAtLocation(root, Gravity.CENTER, 0, 0);
        }
    }

    /**
     * 隐藏删除通讯录确认框
     */
    private void hideContactsDeleteDialog() {
        if (mClearContactsDialog != null) {
            mClearContactsDialog.dismiss();
        }
        if (fragmentCallback != null) {
            fragmentCallback.updateBackground(false);
        }
    }

    public void setFragmentCallback(IFragmentCallback callback) {
        fragmentCallback = callback;
    }

    private void showPBDownloadGroup() {
        mHandler.removeMessages(MSG_UI_HIDE_DOWNLOAD_GROUP);
        mUpdatePBCtrl.setEnabled(false);
        changeDownloadHint(0);
        mDownloadLayout.setVisibility(View.VISIBLE);
    }

    private void hidePBDownloadGroup() {
        mDownloadLayout.setVisibility(View.GONE);
        mUpdatePBCtrl.setEnabled(true);
    }

    private void changeDownloadHint(int count) {
        String strText = String.format(SkinUtils.getString(R.string.str_download_phonebook), count);
        if (mDownloadTextView != null) {
            Log.d(TAG, "changeDownloadHint:" + strText);
            mDownloadTextView.setText(strText);
        }
    }

    private BroadcastReceiver pbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(LocalBluetoothAdapterManager.ACTION_CONNECTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothProfile.STATE_DISCONNECTED);
                if (state == BluetoothProfile.STATE_CONNECTED) {
                    startLoadContact("");
                } else if (state == BluetoothProfile.STATE_DISCONNECTED) {
                    clearContactList();
                }
            }
        }
    };


    private void pbapConnectStateChange(int state) {
        Log.i(TAG, "pbapConnectStateChange: " + state);
        switch (state) {
            case BluetoothProfile.STATE_CONNECTED:
                break;
            case BluetoothProfile.STATE_DISCONNECTED:
                break;
            default:
                break;
        }
    }

    public void clearContactList() {
        if (mPBList.size() > 0) {
            mPBList.clear();
            mPBListAdapter.notifyDataSetChanged();
        }
        if (mSideBar != null) {
            mSideBar.setChoose(-1);
        }

        if (mContactCountTextView != null) {
            mContactCountTextView.setText(String.format(SkinUtils.getString(R.string.bt_phonebook_search_hint), 0));
        }

        if (mSearchText != null) {
            mSearchText.setHint(String.format(SkinUtils.getString(R.string.bt_phonebook_search_hint), 0));
        }
    }

    private void startLoadContact(String searchKey) {
        Log.d(TAG, "startLoadContact");
        cancelLoadContact();
        clearContactList();
        Message msg = Message.obtain(mLoaderHandler, LOADER_CONTRACT);
        msg.obj = TextUtils.isEmpty(searchKey) ? "" : searchKey;
        mLoaderHandler.sendMessage(msg);
    }

    private void cancelLoadContact() {
        Log.d(TAG, "cancelLoadContact!!!");
        mLoaderHandler.removeMessages(LOADER_CONTRACT);
        if (null != mCancellationSignal) {
            Log.d(TAG, "mCancellationSignal cancel!!!");
            mCancellationSignal.cancel();
        }
    }

    public void loadContact(String searchKey) {
        if (null == getContext()) {
            Log.e(TAG, "getContext()==null!");
            Message msg = Message.obtain(mHandler, MSG_UI_LOAD_FINISH);
            mHandler.sendMessage(msg);
            return;
        }
        BluetoothDeviceInfo deviceInfo = mAdapterManager.getConnectDevice();
        if (null == deviceInfo) {
            Log.e(TAG, "loadContact deviceInfo==null!");
            Message msg = Message.obtain(mHandler, MSG_UI_LOAD_FINISH);
            mHandler.sendMessage(msg);
            return;
        }

        ContentResolver resolver = getContext().getContentResolver();
        Cursor cursor = null;

        String nameColumn = Phone.DISPLAY_NAME;
        String labelColumn = PHONEBOOK_LABEL;
        if (Utils.isReverseName()) {
            nameColumn = Phone.DISPLAY_NAME_ALTERNATIVE;
            labelColumn = PHONEBOOK_LABEL_ALT;
        }
        try {
            synchronized (this) {
                if (null != mCancellationSignal) {
                    mCancellationSignal.cancel();
                    mCancellationSignal = null;
                }
                mCancellationSignal = new CancellationSignal();
            }
            if (TextUtils.isEmpty(searchKey)) {
                String[] projection = new String[]{nameColumn, Phone.NUMBER, labelColumn};
                String[] selectionArgs = new String[]{deviceInfo.getDeviceAddr()};
                String selection = ContactsContract.RawContacts.ACCOUNT_NAME + "=?";

                Log.d(TAG, "loadContact1: " + Arrays.toString(projection) + selection + Arrays.toString(selectionArgs));
                cursor = resolver.query(Phone.CONTENT_URI, projection, selection, selectionArgs, "sort_key COLLATE LOCALIZED asc", mCancellationSignal);
            } else {
                String[] projection = new String[]{nameColumn, Phone.NUMBER, labelColumn};
                String[] selectionArgs = new String[]{"%" + searchKey + "%", "%" + searchKey + "%", deviceInfo.getDeviceAddr()};
                String selection = String.format(Locale.ENGLISH,
                        "(%s LIKE ? or %s LIKE ?) AND %s=?", Phone.NUMBER, nameColumn,
                        ContactsContract.RawContacts.ACCOUNT_NAME);

                Log.d(TAG, "loadContact2: " + Arrays.toString(projection) + selection + Arrays.toString(selectionArgs));
                cursor = resolver.query(Phone.CONTENT_URI, projection, selection, selectionArgs, "sort_key COLLATE LOCALIZED asc", mCancellationSignal);
            }
        } catch (OperationCanceledException e) {
            Log.d(TAG, "loadContact: OperationCanceledException!!!!!!");
            if (cursor != null) {
                cursor.close();
            }
            synchronized (this) {
                mCancellationSignal = null;
            }
            return;
        }

        ArrayList<HashMap<String, String>> list = new ArrayList<>();
        mHandler.sendEmptyMessage(MSG_UI_LOAD_START);
        boolean bLoadContract = !mLoaderHandler.hasMessages(LOADER_CONTRACT) && !mCancellationSignal.isCanceled();
        Log.d(TAG, "bLoadContract=" + bLoadContract + " cursor.getCount()=" + cursor.getCount());
        while (cursor.moveToNext() && bLoadContract) {
            HashMap<String, String> map = new HashMap<>();
            String name = cursor.getString(cursor.getColumnIndex(nameColumn));
            String phonenum = cursor.getString(cursor.getColumnIndex(Phone.NUMBER));
            String label = cursor.getString(cursor.getColumnIndex(labelColumn));

            map.put(ITEM_PHONEBOOK_NAME, name);
            map.put(ITEM_PHONEBOOK_NUMBER, phonenum);
            map.put(ITEM_PHONEBOOK_PATH, "1");//暂时无法区分是sim卡还是手机联系人
            map.put(ITEM_PHONEBOOK_LABEL, label);

            list.add(map);

            if (list.size() >= 10) {
                //update list every 10.
                Message msg = Message.obtain(mHandler, MSG_UI_PB_ADD);
                msg.obj = list;
                mHandler.sendMessage(msg);
                list = new ArrayList<>();
            }
        }
        cursor.close();
        if (mCancellationSignal.isCanceled()) {
            Log.d(TAG, "loadContact: CancelLoader!!");
            mHandler.removeMessages(MSG_UI_PB_ADD);
        } else {
            Log.d(TAG, "loadContact finish [list.size ]= "+ list.size());
            Message msg = Message.obtain(mHandler, MSG_UI_LOAD_FINISH);
            msg.obj = list;
            mHandler.sendMessage(msg);
        }
        synchronized (this) {
            mCancellationSignal = null;
        }
    }

    /**
     * 隐藏软键盘
     */
    private void hideSoftInput() {
        if (null == getContext()) {
            return;
        }
        InputMethodManager imm = (InputMethodManager) getContext().getSystemService(
                Context.INPUT_METHOD_SERVICE);
        if (imm.isActive()) {
            if (mSearchText != null) {
                imm.hideSoftInputFromWindow(mSearchText.getWindowToken(), 0);
            } else if (mEditSearchText != null) {
                imm.hideSoftInputFromWindow(mEditSearchText.getWindowToken(), 0);
            }
        }
    }

    private TextWatcher mSearchEditWatcher = new TextWatcher() {

        @Override
        public void afterTextChanged(Editable s) {
            mSearchKey = s.toString().trim();
            startLoadContact(mSearchKey);
        }

        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {

        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {

        }
    };

    class PBListAdapter extends SimpleAdapter {
        private LayoutInflater mInflater;
        private int mSelectIdx = -1;
        private ArrayList<HashMap<String, String>> mList;

        public PBListAdapter(Context context,
                             ArrayList<HashMap<String, String>> data, int resource,
                             String[] from, int[] to) {
            super(context, data, resource, from, to);
            this.mInflater = LayoutInflater.from(context);
            mSelectIdx = -1;
            mList = data;
        }

        public void setSelect(int index) {
            mSelectIdx = index;
        }

        @Override
        public HashMap<String, String> getItem(int position) {
            if (null == mList || position >= mList.size()) {
                return null;
            }
            return mList.get(position);
        }

        public int getSelect() {
            return mSelectIdx;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder = null;
            if (convertView == null) {
                holder = new ViewHolder();
                convertView = SkinUtils.inflate(R.layout.phonebook_listitem);
                holder.nameTextView = convertView.findViewById(SkinUtils.getId(R.id.item_phonebook_name));
                holder.phoneTextView = convertView.findViewById(SkinUtils.getId(R.id.item_phonebook_number));
                holder.pathImageView = convertView.findViewById(SkinUtils.getId(R.id.btn_phonebook_path));
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            String name = "";
            String number = "";
            String path = "";
            if (position < mList.size()) {
                name = mList.get(position).get(ITEM_PHONEBOOK_NAME);
                if (TextUtils.isEmpty(name)) {
                    name = getString(R.string.phonebook_unknow_name);
                }
                name = String.format(Locale.getDefault(), "%d. %s", (position + 1), name);
                number = mList.get(position).get(ITEM_PHONEBOOK_NUMBER);
                path = mList.get(position).get(ITEM_PHONEBOOK_PATH);
            }

            if (holder.nameTextView != null) {
                holder.nameTextView.setText(name);
            }
            if (holder.phoneTextView != null) {
                holder.phoneTextView.setText(number);
            }
            if (holder.pathImageView != null) {
                if ("1".equals(path)) {
                    holder.pathImageView.setImageResource(R.drawable.bt_phonebook_path_sim);
                } else {
                    holder.pathImageView.setImageResource(R.drawable.bt_phonebook_path_phone);
                }
            }

            return convertView;
        }

        public final class ViewHolder {
            public TextView nameTextView;
            public TextView phoneTextView;
            public ImageView pathImageView;
        }
    }

    private OnItemClickListener mPhoneBookListClickListener = new OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> adapterView, View view, int position, long id) {
            mPBListAdapter.setSelect(position);
            mPBListAdapter.notifyDataSetChanged();

            String tele = mPBList.get(position).get(ITEM_PHONEBOOK_NUMBER);
            MyApplication.getInstance().getHfpclientManager().dial(tele);
        }
    };

    private ConnectionListener mAdapterListener = new ConnectionListener() {
        @Override
        public void onServiceConnected() {
            mAdapterManager.registerPbapCallback(mIPbapCallback);
            if (mAdapterManager.getPbapDownLoadState(
                    LocalBluetoothAdapterManager.PARAM_DOWNLOAD_PB)) {
                showPBDownloadGroup();
            }
        }

        @Override
        public void onServiceDisconnected() {

        }
    };

    IPbapCallback mIPbapCallback = new IPbapCallback.Stub() {
        @Override
        public void onPbapDownloadStateChanged(int state, int type) throws RemoteException {
            if (type == LocalBluetoothAdapterManager.PARAM_DOWNLOAD_PB) {
                if (state == LocalBluetoothAdapterManager.STATE_DOWNLOADED) {
                    mHandler.removeMessages(MSG_DOWNLOAD_FINISH);
                    mHandler.sendEmptyMessage(MSG_DOWNLOAD_FINISH);
                } else if (state == LocalBluetoothAdapterManager.STATE_DOWNLOADING) {
                    Log.e(TAG, "Pbap Downloading...");
                    mHandler.removeMessages(MSG_DOWNLOAD_START);
                    mHandler.sendEmptyMessage(MSG_DOWNLOAD_START);
                } else {
                    mHandler.removeMessages(MSG_DOWNLOAD_FAILED);
                    Message msg = mHandler.obtainMessage(MSG_DOWNLOAD_FAILED);
                    msg.arg1 = state;//失败原因
                    Log.e(TAG, "onPbapDownloadStateChanged fail! state =" + state);
                    mHandler.sendMessage(msg);
                }
            }
        }

        @Override
        public void onPbapConnectStateChanged(int state) throws RemoteException {
            pbapConnectStateChange(state);
        }
    };
}
