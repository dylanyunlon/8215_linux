package com.hcn.media_data.notify;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * 对外通知媒体信息结构体
 * <p> 主界面、原厂 Can 信息接收它；
 *
 * @author 86158
 */
public class NotifyMediaState implements Parcelable {
    private static final String TAG = NotifyMediaState.class.getSimpleName();

    public static final Parcelable.Creator<NotifyMediaState> CREATOR =
            new Creator<NotifyMediaState>() {
                @Override
                public NotifyMediaState createFromParcel(Parcel source) {
                    return new NotifyMediaState(source);
                }

                @Override
                public NotifyMediaState[] newArray(int size) {
                    return new NotifyMediaState[size];
                }
            };

    public int mMediaType = 0;
    public int mUsbState = 0;
    public int mSDCardState = 0;
    public int mPlayMode = 0;
    public int mCurrentIndex = 0;
    public int mTotalNum = 0;

    public NotifyMediaState() {
        // TODO Auto-generated constructor stub
    }

    public NotifyMediaState(Parcel source) {
        mMediaType = source.readInt();
        mUsbState = source.readInt();
        mSDCardState = source.readInt();
        mPlayMode = source.readInt();
        mCurrentIndex = source.readInt();
        mTotalNum = source.readInt();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mMediaType);
        dest.writeInt(mUsbState);
        dest.writeInt(mSDCardState);
        dest.writeInt(mPlayMode);
        dest.writeInt(mCurrentIndex);
        dest.writeInt(mTotalNum);
    }
}
