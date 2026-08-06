package com.hcn.bluetooth.api;

import android.bluetooth.BluetoothDevice;
import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;

public class BluetoothDeviceInfo implements Parcelable {
    public int mDeviceIndex;
    public int mDeviceStatus;
    public String mDeviceName = null;
    public String mDeviceAddr = null;

    public static final class DeviceStatus {
        public static final int DEVICE_STATUS_UNKOWN = -1;
        public static final int DEVICE_STATUS_PAIRED = 0;
        public static final int DEVICE_STATUS_CONNECTED = 1;
        public static final int DEVICE_STATUS_CONNECTEDPHONE = 2;
        public static final int DEVICE_STATUS_CONNECTEDMEDIA = 3;
        public static final int DEVICE_STATUS_CONNECTING = 4;
        public static final int DEVICE_STATUS_DISCONNECTING = 5;
    }

    public BluetoothDeviceInfo() {
        mDeviceIndex = -1;
        mDeviceStatus = -1;
        mDeviceName = "";
        mDeviceAddr = "";
    }

    public BluetoothDeviceInfo(BluetoothDevice device) {
        if (null == device) {
            mDeviceIndex = -1;
            mDeviceStatus = -1;
            mDeviceName = "";
            mDeviceAddr = "";
            return;
        }
        if (!TextUtils.isEmpty(device.getAliasName())) {
            mDeviceName = device.getAliasName();
        } else if (!TextUtils.isEmpty(device.getName())) {
            mDeviceName = device.getName();
        } else {
            mDeviceName = device.getAddress();
        }
        mDeviceAddr = device.getAddress();
        mDeviceStatus = device.getBondState();
    }

    private BluetoothDeviceInfo(Parcel source) {
        mDeviceIndex = source.readInt();
        mDeviceStatus = source.readInt();
        mDeviceName = source.readString();
        mDeviceAddr = source.readString();
    }

    public int getDeviceIndex() {
        return mDeviceIndex;
    }

    public void setDeviceIndex(int deviceIndex) {
        this.mDeviceIndex = deviceIndex;
    }

    public int getDeviceStatus() {
        return mDeviceStatus;
    }

    public void setDeviceStatus(int deviceStatus) {
        this.mDeviceStatus = deviceStatus;
    }

    public String getDeviceName() {
        return mDeviceName;
    }

    public void setDeviceName(String deviceName) {
        this.mDeviceName = deviceName;
    }

    public String getDeviceAddr() {
        return mDeviceAddr;
    }

    public void setDeviceAddr(String deviceAddr) {
        this.mDeviceAddr = deviceAddr;
    }

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mDeviceIndex);
        dest.writeInt(mDeviceStatus);
        dest.writeString(mDeviceName);
        dest.writeString(mDeviceAddr);
    }

    public static final Parcelable.Creator<BluetoothDeviceInfo> CREATOR =
            new Parcelable.Creator<BluetoothDeviceInfo>() {

                @Override
                public BluetoothDeviceInfo createFromParcel(Parcel source) {
                    return new BluetoothDeviceInfo(source);
                }

                @Override
                public BluetoothDeviceInfo[] newArray(int size) {
                    return new BluetoothDeviceInfo[size];
                }
            };
}
