package com.autochips.bluetooth.util;

import android.accounts.Account;
import android.content.ContentResolver;
import android.database.Cursor;
import android.os.CancellationSignal;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.OperationCanceledException;
import android.provider.CallLog;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.text.TextUtils;
import android.util.Log;

import com.autochips.bluetooth.BaseApplication;
import com.autochips.bluetooth.bean.HContact;
import com.autochips.bluetooth.bean.HRecord;
import com.autochips.bluetooth.bean.SearchResult;
import com.hcn.bluetooth.api.Utils;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

public class HQueryThread extends HandlerThread {
    private final String TAG = "HQueryThread";
    private static final String PHONEBOOK_LABEL = "phonebook_label";
    private static final String PHONEBOOK_LABEL_ALT = "phonebook_label_alt";
    /**
     * 查询拨号提示联系人
     */
    public static final int MSG_STATE_QUERY_CONTACT_START = 11;
    public static final int MSG_STATE_QUERY_CONTACT_UPDATE = 12;
    public static final int MSG_STATE_QUERY_CONTACT_FINISH = 13;

    /**
     * 查询通话记录
     */
    public static final int MSG_STATE_QUERY_RECORD_START = 21;
    public static final int MSG_STATE_QUERY_RECORD_UPDATE = 22;
    public static final int MSG_STATE_QUERY_RECORD_FINISH = 23;
    /**
     *
     */
    private Object object = new Object();
    private CancellationSignal mSearchContactSignal = null;
    private CancellationSignal mSearchRecordSignal = null;
    private Handler mUiHandler = null;

    private static String SEARCH_SELECTION = String.format(Locale.ENGLISH,
            "%s LIKE ? AND %s=? AND %s =?",
            Phone.NUMBER,
            ContactsContract.RawContacts.ACCOUNT_NAME,
            ContactsContract.RawContacts.ACCOUNT_TYPE);
    private static String SEARCH_CONTACTS = String.format(Locale.ENGLISH,
            "%s=? AND %s =?",
            ContactsContract.RawContacts.ACCOUNT_NAME,
            ContactsContract.RawContacts.ACCOUNT_TYPE);
    private static final String[] CALLLOG_PROJECTION = {
            CallLog.Calls.NUMBER,
            CallLog.Calls.DATE,
            CallLog.Calls.TYPE,
            CallLog.Calls.CACHED_NAME
    };


    public HQueryThread(String name) {
        super(name);
    }

    public void setHandler(Handler handler) {
        this.mUiHandler = handler;
    }

    public void stopLoadSearchContact(){
        //updateUiData(MSG_STATE_QUERY_CONTACT_FINISH);
        if (null != mSearchContactSignal) {
            mSearchContactSignal.cancel();
            mSearchContactSignal = null;
        }
    }

    /**
     * @param key
     * @param account_name
     */
    public void beginSearchContact(String key, String account_name) {
        SearchResult result = new SearchResult();
        if (mSearchContactSignal != null) {
            mSearchContactSignal.cancel();
        }
        Log.d(TAG, "searchContact key : " + key);

        List<HContact> pbList = new ArrayList<>();
        HContact contact = null;
        ContentResolver resolver = BaseApplication.getInstance().getContentResolver();
        Cursor cursor = null;
        String nameColumn = Phone.DISPLAY_NAME;
        String labelColumn = PHONEBOOK_LABEL;
        if (Utils.isReverseName()) {
            nameColumn = Phone.DISPLAY_NAME_ALTERNATIVE;
            labelColumn = PHONEBOOK_LABEL_ALT;
        }
        updateUiData(MSG_STATE_QUERY_CONTACT_START);
        Log.d(TAG, "beginSearchContact1: MSG_STATE_QUERY_CONTACT_START");
        try {
            synchronized (object) {
                mSearchContactSignal = new CancellationSignal();
            }
            Log.d(TAG, "beginSearchContact1: new mSearchContactSignal");
            if(TextUtils.isEmpty(key)){
                cursor = resolver.query(Phone.CONTENT_URI,
                        new String[]{nameColumn,Phone.NUMBER,labelColumn},
                        SEARCH_CONTACTS,
                        new String[]{account_name, Utils.ACCOUNT_TYPE},
                        null,
                        mSearchContactSignal);
            }else {
                cursor = resolver.query(
                        Phone.CONTENT_URI,
                        new String[]{nameColumn, Phone.NUMBER},
                        SEARCH_SELECTION,
                        new String[]{"%" + key + "%", account_name, Utils.ACCOUNT_TYPE},
                        null,
                        mSearchContactSignal);
            }
            Log.d(TAG, "beginSearchContact1: cursor ");
            if(cursor != null) {
                List<String> compareData = new ArrayList<>();
                String compareStr = "";
                while (cursor.moveToNext()) {
                    int index_phone = cursor.getColumnIndex(Phone.NUMBER);
                    int index_name = cursor.getColumnIndex(nameColumn);
                    int index_label = cursor.getColumnIndex(labelColumn);
                    String phone_number = cursor.getString(index_phone);
                    String name = cursor.getString(index_name);
                    compareStr = name+"-"+phone_number;
                    if(compareData.contains(compareStr)){
                        Log.w(TAG, "beginSearchContact: is repeat data : " + compareStr);
                        continue;
                    }
                    compareData.add(compareStr);
                    String label = null;
                    if (TextUtils.isEmpty(key)) {
                        label = cursor.getString(index_label);
                    }
                    contact = new HContact();
                    contact.setName(name);
                    contact.setPhone(phone_number);
                    contact.setLabel(label);
//                    Log.d(TAG, "beginSearchContact: " +
//                            "displayName: " + name
//                            + " number:" + phone_number
//                            + " label : " + label);
                    pbList.add(contact);

                    if (pbList.size() >= 10) {
                        //update list every 10.
                        result.list = pbList;
                        result.searchKey = key;
                        updateUiData(MSG_STATE_QUERY_CONTACT_UPDATE, result);
                        result = new SearchResult();
                        pbList = new ArrayList<>();
                    }
                }
            }
            cursor.close();
        } catch (OperationCanceledException e) {
            e.printStackTrace();
            if (cursor != null) {
                cursor.close();
            }
            synchronized (object) {
                mSearchContactSignal = null;
            }
            updateUiData(MSG_STATE_QUERY_CONTACT_FINISH);
            Log.d(TAG, "beginSearchContact1: MSG_STATE_QUERY_CONTACT_FINISH");
            return;
        } finally {
            synchronized (object) {
                mSearchContactSignal = null;
            }
        }
        if (pbList.size() != 0) {
            result.list = pbList;
            result.searchKey = key;
            updateUiData(MSG_STATE_QUERY_CONTACT_UPDATE, result);
        }
        updateUiData(MSG_STATE_QUERY_CONTACT_FINISH);
        Log.d(TAG, "beginSearchContact: MSG_STATE_QUERY_CONTACT_FINISH");
    }

    public void updateUiData(int what, SearchResult result) {
        if (mUiHandler != null) {
            mUiHandler.obtainMessage(
                    what,
                    0,
                    0,
                    result).sendToTarget();
        }
    }

    public void updateUiData(int what, List<HRecord> result) {
        if (mUiHandler != null) {
            mUiHandler.obtainMessage(
                    what,
                    0,
                    0,
                    result).sendToTarget();
        }
    }

    private void updateUiData(int what) {
        if (mUiHandler != null) {
            mUiHandler.obtainMessage(what).sendToTarget();
        }
    }

    private void removeMsg(int what) {
        if (mUiHandler != null && mUiHandler.hasMessages(what)) {
            mUiHandler.removeMessages(what);
        }
    }

    /**
     * 查询通话记录
     */
    public void beginLoadHistoryCallLog(int callType, String address) {
        Log.d(TAG,"beginLoadHistoryCallLog : " + callType + ",  address : " +address);
        if (TextUtils.isEmpty(address)) {
            updateUiData(MSG_STATE_QUERY_RECORD_FINISH);
        }

        ContentResolver resolver = BaseApplication.getInstance().getContentResolver();
        Cursor cursor = null;

        try {
            synchronized (this) {
                if (null != mSearchRecordSignal) {
                    mSearchRecordSignal.cancel();
                    mSearchRecordSignal = null;
                }
                mSearchRecordSignal = new CancellationSignal();
            }
            Account account = new Account(address, Utils.ACCOUNT_TYPE);
            if (callType == 0) {
                cursor = resolver.query(CallLog.Calls.CONTENT_URI,
                        CALLLOG_PROJECTION,
                        CallLog.Calls.PHONE_ACCOUNT_ID + "=? or " + CallLog.Calls.PHONE_ACCOUNT_ID + "=?",
                        new String[]{String.valueOf(account.hashCode()), account.name},
                        CallLog.Calls.DATE + " desc limit 100", mSearchRecordSignal);
            } else {
                cursor = resolver.query(CallLog.Calls.CONTENT_URI,
                        CALLLOG_PROJECTION,
                        CallLog.Calls.TYPE + "=? AND " + CallLog.Calls.PHONE_ACCOUNT_ID + "=?",
                        new String[]{String.valueOf(callType), String.valueOf(account.hashCode())},
                        CallLog.Calls.DATE + " desc limit 100", mSearchRecordSignal);
                //INCOMING_TYPE  MISSED_TYPE
            }
        } catch (OperationCanceledException e) {
            if (cursor != null) {
                cursor.close();
            }
            synchronized (this) {
                mSearchRecordSignal = null;
            }
            Log.e(TAG, "loadHistoryCallLog: OperationCanceledException!");
            return;
        }
        ArrayList<HRecord> list = new ArrayList<>();
        HRecord record = null;
        SearchResult result = new SearchResult();
        //重置下载状态
        removeMsg(MSG_STATE_QUERY_RECORD_UPDATE);
        updateUiData(MSG_STATE_QUERY_RECORD_START);
        Log.d(TAG, "loadHistoryCallLog: send MSG_UI_LOAD_START" + (mSearchRecordSignal != null) + "" + !mSearchRecordSignal.isCanceled());
        while (cursor.moveToNext() && mSearchRecordSignal != null && !mSearchRecordSignal.isCanceled()) {

            int timeColIdx = cursor.getColumnIndex(CallLog.Calls.DATE);
            long time = cursor.getLong(timeColIdx);
            int numberColIdx = cursor.getColumnIndex(CallLog.Calls.NUMBER);
            String number = cursor.getString(numberColIdx);
            int typeColIdx = cursor.getColumnIndex(CallLog.Calls.TYPE);
            int type = cursor.getInt(typeColIdx);
            int nameIndex = cursor.getColumnIndex(CallLog.Calls.CACHED_NAME);
            String name = cursor.getString(nameIndex);

            String ext_number = "";
            if (number.length() > 15) {//05516531751182295
                ext_number = number.substring(12);
                number = number.substring(0, 12);
            } else if (number.length() > 14) {//653-175-1182295
                ext_number = number.substring(10);
                number = number.substring(0, 10);
            }

            if (!ext_number.equals("")) {
                StringBuffer sb = new StringBuffer();
                sb = sb.append(number);
                sb = sb.append(",");
                sb = sb.append(ext_number);
                number = sb.toString();
            }

            if(TextUtils.isEmpty(name)) {
                name = Utils.getContactNameByNumber(
                        BaseApplication.getInstance().getApplicationContext(),
                        number,
                        address);
            }

            if (TextUtils.isEmpty(name)) {
                name = number;
            }
            Log.d(TAG, "loadHistoryCallLog: " +
                    "displayName: " + name
                    + " number:" + number
                    + " time :" + time);
            record = new HRecord();
            record.setName(name);
            record.setPhone(number);
            record.setTime(time);
            record.setType(type);
            list.add(record);

            if (list.size() >= 10) {
                //update list every 10.
                updateUiData(MSG_STATE_QUERY_RECORD_UPDATE, list);

                list = new ArrayList<>();
            }
        }
        cursor.close();
        if (mSearchRecordSignal != null && mSearchRecordSignal.isCanceled()) {
            Log.d(TAG, "loadHistoryCallLog: finish cancel!!");
        } else {
            Log.d(TAG, "loadHistoryCallLog: send MSG_UI_LOAD_FINISH");
        }

        if (list.size() != 0) {
            updateUiData(MSG_STATE_QUERY_RECORD_UPDATE, list);
        }
        updateUiData(MSG_STATE_QUERY_RECORD_FINISH);
        synchronized (this) {
            mSearchRecordSignal = null;
        }
    }


    public void stopLoadHistoryCallLog(){
        //updateUiData(MSG_STATE_QUERY_RECORD_FINISH);
        if (null != mSearchRecordSignal) {
            mSearchRecordSignal.cancel();
            mSearchRecordSignal = null;
        }
    }
}
