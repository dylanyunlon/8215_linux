package com.autochips.bluetooth.fragment;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.autochips.bluetooth.R;
import com.autochips.bluetooth.adapter.ContactsAdapter;
import com.autochips.bluetooth.bean.HContact;
import com.autochips.bluetooth.bean.SearchResult;
import com.autochips.bluetooth.util.HQueryThread;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;
import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ContactsFragment extends BaseFragment implements
        View.OnClickListener, AdapterView.OnItemClickListener {
    /**
     * 主副Handler及数据线程
     */
    private MainHandler mMainHandler = null;
    private QueryHandler mQueryHandler = null;
    private HQueryThread mQueryThread = null;
    //UI
    private ListView mLv = null;
    private View mSelectView = null;
    private TextView mTxtLoading = null;
    private ContactsAdapter mContactsAdapter = null;
    private List<HContact> mData = new ArrayList<>();
    //data
    private int[] mLabelIds = {R.id.id_contacts_key_a, R.id.id_contacts_key_b
            , R.id.id_contacts_key_c, R.id.id_contacts_key_d, R.id.id_contacts_key_e
            , R.id.id_contacts_key_f, R.id.id_contacts_key_g, R.id.id_contacts_key_h
            , R.id.id_contacts_key_i, R.id.id_contacts_key_j, R.id.id_contacts_key_k
            , R.id.id_contacts_key_l, R.id.id_contacts_key_m, R.id.id_contacts_key_n
            , R.id.id_contacts_key_o, R.id.id_contacts_key_p, R.id.id_contacts_key_q
            , R.id.id_contacts_key_r, R.id.id_contacts_key_s, R.id.id_contacts_key_t
            , R.id.id_contacts_key_u, R.id.id_contacts_key_v, R.id.id_contacts_key_w
            , R.id.id_contacts_key_x, R.id.id_contacts_key_y, R.id.id_contacts_key_z
            , R.id.id_contacts_key_shape};
    private String[] mLabels = null;
    private Map<String, Boolean> mMapLabelState = null;
    private Map<String, Integer> mMapLabelPos = null;
    private Map<String, View> mMapLabelView = null;

    /**
     * 需要更新电话本
     * 一般半分钟内持循环读
     */
    private boolean bUpdateContacts = true;
    /**
     * 是否正在处理查询流程中
     */
    private boolean bIsLoadingContact = false;

    /**
     *
     */
    private int mDownCount = 6;

    /**
     *
     * @return
     */
    private int mDownState = LocalBluetoothAdapterManager.STATE_DOWNLOADED;


    @Override
    protected int onLoadLayoutId() {
        return R.layout.fragment_contacts;
    }

    @Override
    protected Handler getHandler() {
        return mMainHandler;
    }

    @Override
    protected void onPbapDownState(int state, int type) {
        super.onPbapDownState(state, type);
        mDownState = state;
        log("onPbapDownState state :" + state + " , type : " + type);
        if (type == LocalBluetoothAdapterManager.PARAM_DOWNLOAD_PB) {
            if (state == LocalBluetoothAdapterManager.STATE_DOWNLOADED) {
                if(bUpdateContacts) {
                    if(mMainHandler.hasMessages(MainHandler.MSG_UPDATE_DOWN)){
                        mMainHandler.removeMessages(MainHandler.MSG_UPDATE_DOWN);
                    }
                }
                if(mDownCount > 0) {
                    mMainHandler.sendEmptyMessageDelayed(MainHandler.MSG_UPDATE_DOWN, 3000);
                    mDownCount --;
                }else{
                    bIsLoadingContact = false;
                    updateTxtLoading(true, getAppString(R.string.txt_loading_not_contact));
                }
            } else if (state == LocalBluetoothAdapterManager.STATE_DOWNLOADING) {
                updateTxtLoading(true, getAppString(R.string.txt_loading));
            } else {
                if (bUpdateContacts) {
                    updateTxtLoading(true, getAppString(R.string.txt_loading));
                    mMainHandler.sendEmptyMessageDelayed(MainHandler.MSG_UPDATE_DOWN, 5000);
                } else {
                    bIsLoadingContact = false;
                    updateTxtLoading(true, getAppString(R.string.txt_loading_not_contact));
                }
            }
        }
    }

    @Override
    protected void callbackDisconnect() {
        super.callbackDisconnect();
        mQueryHandler.sendEmptyMessage(QueryHandler.MSG_STOP_ALL_CONTACTS);
        if (mMainHandler.hasMessages(MainHandler.MSG_UPDATE_DOWN)) {
            mMainHandler.removeMessages(MainHandler.MSG_UPDATE_DOWN);
        }
        if (mMainHandler.hasMessages(MainHandler.MSG_UPDATE_STATE)) {
            mMainHandler.removeMessages(MainHandler.MSG_UPDATE_STATE);
        }
        mData.clear();
        if (mContactsAdapter != null) {
            mContactsAdapter.resetData();
        }
        bUpdateContacts = true;
        bIsLoadingContact = false;
        resetLabel();
        mDownCount = 5;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        TAG = "ContactsFragment";
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if (!hidden) {
            //下载失败后，每次进来增加一次加载次数，用于确认。因为下载失败传回来的是完成的状态。需要取一次确认是否有数据。
            if(mDownCount <= 0){
                mDownCount = 1;
            }
            init();
        }
    }

    @Override
    protected void init() {
        if (mMainHandler == null) {
            mMainHandler = new MainHandler(ContactsFragment.this);
            mQueryThread = new HQueryThread("contact_query_thread");
            mQueryThread.start();
            mQueryThread.setHandler(mMainHandler);
            mQueryHandler = new QueryHandler(mQueryThread.getLooper());

            //注册监听
            registerPbapCallback();
            registerLocalStateCallback();

            initView();
            initLabel();
        }
        log("bIsLoadingContact : " + bIsLoadingContact);
        if (mBluetoothManager.isBluetoothConnected() && mData.size() == 0 && !bIsLoadingContact) {
            if (allowCheck() && !mMainHandler.hasMessages(MainHandler.MSG_UPDATE_STATE)) {
                updateTxtLoading(true, getAppString(R.string.txt_loading));
                requestContact();
                if (bUpdateContacts) {
                    log("init 30  * 1000");
                    mMainHandler.sendEmptyMessageDelayed(MainHandler.MSG_UPDATE_STATE, 1000 * 30);
                }
            }
        }
    }

    private void requestContact() {
        log("requestContact .<>>> mBluetoothManager.isDowning() : " + mBluetoothManager.isDowning());
        BluetoothDeviceInfo info = mBluetoothManager.getConnectDevice();
        if (info != null && !mBluetoothManager.isDowning()) {
            bIsLoadingContact = true;
            mQueryHandler.obtainMessage(
                    QueryHandler.MSG_UPDATE_ALL_CONTACTS,
                    info.getDeviceAddr()).sendToTarget();
        }else{
            if(mMainHandler != null && !mMainHandler.hasMessages(MainHandler.MSG_UPDATE_DOWN)){
                mMainHandler.sendEmptyMessageDelayed(MainHandler.MSG_UPDATE_DOWN,5000);
            }
        }
    }

    private void initView() {
        if (mLv != null) {
            return;
        }
        mLv = (ListView) findViewById(R.id.id_contacts_listview);
        mTxtLoading = (TextView) findViewById(R.id.id_contact_loading);
        mContactsAdapter = new ContactsAdapter(getContext(), mData);
        mLv.setAdapter(mContactsAdapter);
        mLv.setOnScrollChangeListener(new View.OnScrollChangeListener() {
            @Override
            public void onScrollChange(View v, int scrollX, int scrollY, int oldScrollX, int oldScrollY) {
                updateLabelSelectState(mLv.getFirstVisiblePosition());
            }
        });
        mLv.setOnItemClickListener(this);
    }

    /**
     * 初始化右侧a-z快捷按钮及其相关联的数据
     */
    private void initLabel() {
        if (mMapLabelState == null) {
            mMapLabelState = new HashMap<>();
            mMapLabelView = new HashMap<>();
            mLabels = getAppStringArrays(R.array.arrays_contacts_labels);
            View v = null;
            for (int i = 0; i < mLabels.length; i++) {
                v = findViewById(mLabelIds[i]);
                v.setOnClickListener(this);
                v.setEnabled(false);
                mMapLabelState.put(mLabels[i], false);
                mMapLabelView.put(mLabels[i], v);
            }
        }
    }

    /**
     * 断开需要重置
     */
    private void resetLabel() {
        for (int i = 0; i < mLabels.length; i++) {
            mMapLabelState.put(mLabels[i], false);
        }
        updateLabel();
    }

    /**
     * 更新提示文本信息
     *
     * @param show
     * @param txt
     */
    private void updateTxtLoading(boolean show, String txt) {
        if (mTxtLoading != null) {
            log("updateTxtLoading:" + show + " , " + txt);
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
     * 第二步
     * 更新右侧快捷按钮的可用状态
     */
    private void updateLabel() {
        for (int i = 0; i < mLabels.length; i++) {
            if (mMapLabelState.get(mLabels[i]).booleanValue()) {
                mMapLabelView.get(mLabels[i]).setEnabled(true);
            } else {
                mMapLabelView.get(mLabels[i]).setEnabled(false);
            }
        }
    }

    /**
     * 更新当前列表滑动到的是哪个快捷按钮，进行聚焦显示按钮
     *
     * @param pos
     */
    private void updateLabelSelectState(int pos) {
        log("updateLabelSelectState " + pos);
        if (mData != null && mData.size() > pos) {
            String label = mData.get(pos).getLabel();
            if (mSelectView != null) {
                mSelectView.setSelected(false);
            }
            if (mMapLabelView != null && mMapLabelView.get(label) != null) {
                mSelectView = mMapLabelView.get(label);
                mSelectView.setSelected(true);
            }
        }
    }

    /**
     * 加载数据完成
     * 没数据进行一次从手机下载数据的操作
     */
    private void loadFinish() {
        log("loadFinish : size = " + mData.size() + " ,state = " + bUpdateContacts + " , mDownState = " + mDownState);
        //20230710概率出现只下载了几个就失败的情况。
        if (mData.size() > 0 && mDownState == LocalBluetoothAdapterManager.STATE_DOWNLOADED) {
            Collections.sort(mData);
            updateLabelEnableState();
            updateTxtLoading(false, null);
            bIsLoadingContact = false;
            bUpdateContacts = false;
            mContactsAdapter.setData(mData);
        } else {//没有内容做一次下载从手机
            if(mDownState != LocalBluetoothAdapterManager.STATE_DOWNLOADING){
                mBluetoothManager.startContactDownLoad();
            }
        }
    }

    /**
     * 第一步
     * 对加载完的数据做遍历，看有哪些快捷按钮可用
     * 就是查找所有数据，看有哪些首字母内容，map中填充对应字母的可用状态
     * mMapLabelPos使用来记录数据中每个类型字母的位置，用来定位刷listview
     */
    private void updateLabelEnableState() {
        List<String> map = new ArrayList<>();
        if (mMapLabelPos == null) {
            mMapLabelPos = new HashMap<>();
        } else {
            mMapLabelPos.clear();
        }
        //先读列表中有哪些label
        HContact contact = null;
        for (int i = 0; i < mData.size(); i++) {
            contact = mData.get(i);
            if (!map.contains(contact.getLabel())) {
                map.add(contact.getLabel());
                mMapLabelPos.put(contact.getLabel(), i);
            }
        }
        if (map.size() > 0) {
            //再重置所有的状态为false
            for (int i = 0; i < mLabels.length; i++) {
                mMapLabelState.put(mLabels[i], false);
            }
            //再改写实际存在的label为true
            for (String label : map) {
                mMapLabelState.put(label, true);
            }
        }
        updateLabel();
    }

    /**
     * 依据字母定位listview的位置
     *
     * @param label
     */
    private void updateListPosition(String label) {
        if (!TextUtils.isEmpty(label) && mMapLabelPos != null && mMapLabelPos.containsKey(label)) {
            int pos = mMapLabelPos.get(label);
            mLv.setSelection(pos);
        }
    }

    /**/
    @Override
    public void onClick(View v) {
        String tag = (String) v.getTag();
        updateListPosition(tag);
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        if (mData != null && mData.size() > position) {
            if (!TextUtils.isEmpty(mData.get(position).getPhone())) {
                mBluetoothManager.dial(mData.get(position).getPhone());
            }
        }
    }

    /**/
    class QueryHandler extends Handler {
        private final static int MSG_UPDATE_ALL_CONTACTS = 1;
        private final static int MSG_STOP_ALL_CONTACTS = 2;

        public QueryHandler(@NonNull Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            if (msg.what == MSG_UPDATE_ALL_CONTACTS) {
                //此处跑的代码就是在辅助线程中了
                //实际beginLoadHistoryCallLog不应该写在HandlerThread中。为方便整洁
                mQueryThread.beginSearchContact("", (String) msg.obj);
            } else {
                log("<<<MSG_STOP_ALL_CONTACTS>>>>");
                mQueryThread.stopLoadSearchContact();
            }
        }
    }

    /**
     * 主UIhandler
     */
    class MainHandler extends Handler {
        private final static int MSG_UPDATE_STATE = 1;
        private final static int MSG_UPDATE_DOWN = 2;
        private WeakReference<ContactsFragment> mRf = null;

        public MainHandler(ContactsFragment fragment) {
            mRf = new WeakReference<>(fragment);
        }

        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            ContactsFragment fragment = mRf.get();
            if (fragment == null) {
                return;
            }
            switch (msg.what) {
                case MSG_UPDATE_STATE:
                    log("MSG_UPDATE_STATE");
                    bUpdateContacts = false;
                    if(bIsLoadingContact) {
                        bIsLoadingContact = false;
                        //部分手机在下载记录时可能会把联系人挤掉,结束时再下一次
                        if(!hasMessages(MSG_UPDATE_DOWN)){
                            sendEmptyMessage(MSG_UPDATE_DOWN);
                        }
                    }
                    break;
                case MSG_UPDATE_DOWN:
                    requestContact();
                    break;
                case HQueryThread.MSG_STATE_QUERY_CONTACT_START:
                    if (mData != null) {
                        mData.clear();
                    }
                    break;
                case HQueryThread.MSG_STATE_QUERY_CONTACT_UPDATE:
                    SearchResult result = (SearchResult) msg.obj;
                    if (result != null && result.list != null) {
                        //logd(result.list);
                        mData.addAll(result.list);
                        //mContactsAdapter.setData(mData);
                    }
                    break;
                case HQueryThread.MSG_STATE_QUERY_CONTACT_FINISH:
                    fragment.loadFinish();
                    break;
                default:
                    break;
            }
        }
    }
}
