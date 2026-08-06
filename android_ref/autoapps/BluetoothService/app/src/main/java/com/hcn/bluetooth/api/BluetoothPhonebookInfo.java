package com.hcn.bluetooth.api;

import android.os.Parcel;
import android.os.Parcelable;

import java.lang.Cloneable;

public class BluetoothPhonebookInfo implements Parcelable, Cloneable {
    public int mType;
    public String mTelephone = null;
    public String mUserName = null;
    public long id;

    private static final BluetoothPhonebookInfo sProtoInfo = new BluetoothPhonebookInfo();

    public static synchronized BluetoothPhonebookInfo getNewObject() {
        BluetoothPhonebookInfo info = (BluetoothPhonebookInfo) sProtoInfo.clone();
        return info;
    }

    @Override
    public String toString() {
        // TODO Auto-generated method stub
        return "id:" + id + "type: " + mType + "telephone: " + ((mTelephone == null) ? "null"
                : mTelephone) + "name: " + ((mUserName == null) ? "null" : mUserName);
    }

    public int getType() {
        return mType;
    }

    public void setType(int mType) {
        this.mType = mType;
    }

    public String getTelephone() {
        return mTelephone;
    }

    public void setTelephone(String mTelephone) {
        this.mTelephone = mTelephone;
    }

    public String getUserName() {
        return mUserName;
    }

    public void setUserName(String mUserName) {
        this.mUserName = mUserName;
    }

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }

    public BluetoothPhonebookInfo reset() {
        this.mType = -1;
        mTelephone = null;
        mUserName = null;
        this.id = 0;
        return this;
    }

    public Object clone() {
        BluetoothPhonebookInfo info = null;
        try {
            info = (BluetoothPhonebookInfo) super.clone();
        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }
        info.reset();
        return info;
    }

    public BluetoothPhonebookInfo() {
        mType = 0;
        mTelephone = new String();
        mUserName = new String();
        id = 0;
    }

    private BluetoothPhonebookInfo(Parcel source) {
        mType = source.readInt();
        mTelephone = source.readString();
        mUserName = source.readString();
        id = source.readLong();
    }

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mType);
        dest.writeString(mTelephone);
        dest.writeString(mUserName);
        dest.writeLong(id);
    }

    public static final Parcelable.Creator<BluetoothPhonebookInfo> CREATOR =
            new Parcelable.Creator<BluetoothPhonebookInfo>() {

                @Override
                public BluetoothPhonebookInfo createFromParcel(Parcel source) {
                    return new BluetoothPhonebookInfo(source);
                }

                @Override
                public BluetoothPhonebookInfo[] newArray(int size) {
                    return new BluetoothPhonebookInfo[size];
                }
            };
}
