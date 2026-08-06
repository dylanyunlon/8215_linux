package com.autochips.bluetooth.manager;

import com.autochips.bluetooth.bean.HRecord;

import java.util.ArrayList;
import java.util.List;

public class HUserData {

    //protected List<HContact> mContact = new ArrayList<>();
    //protected List<HRecord> mRecordUnRead = new ArrayList<>();
    protected List<HRecord> mRecordAll = new ArrayList<>();

    public List<HRecord> getRecordAll() {
        return mRecordAll;
    }

    public synchronized void setRecordAll(List<HRecord> all) {
        if (!mRecordAll.isEmpty()) {
            mRecordAll.clear();
        }
        this.mRecordAll.addAll(all);
    }

    //    public List<HContact> getContact() {
//        return mContact;
//    }
//
//    public List<HRecord> getRecordAll() {
//        return mRecordAll;
//    }
//
//    public List<HRecord> getRecordUnRead() {
//        return mRecordUnRead;
//    }

    public void reset() {
        mRecordAll.clear();
        //mRecordUnRead.clear();
        //mContact.clear();
    }
}
