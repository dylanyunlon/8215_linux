package com.hcn.media_data.notify;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * 音乐通知信息结构体
 * <p> 未使用过，暂时保留；
 *
 * @author 86158
 */
public class NotifyMusicInfo implements Parcelable {
    private static final String TAG = NotifyMusicInfo.class.getSimpleName();

    public static final Parcelable.Creator<NotifyMusicInfo> CREATOR =
            new Creator<NotifyMusicInfo>() {
                @Override
                public NotifyMusicInfo createFromParcel(Parcel source) {
                    return new NotifyMusicInfo(source);
                }

                @Override
                public NotifyMusicInfo[] newArray(int size) {
                    return new NotifyMusicInfo[size];
                }
            };

    public int mMediaType = 0;
    public int mPlayState = 0;
    public String mFileName = "";
    public String mFilePath = "";
    public String mArtist = "";
    public String mAlbum = "";
    public int mCurrentTime = 0;
    public int mTotalTime = 0;

    public NotifyMusicInfo() {
        // TODO Auto-generated constructor stub
    }

    public NotifyMusicInfo(Parcel source) {
        mMediaType = source.readInt();
        mPlayState = source.readInt();
        mFileName = source.readString();
        mFilePath = source.readString();
        mArtist = source.readString();
        mAlbum = source.readString();
        mCurrentTime = source.readInt();
        mTotalTime = source.readInt();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mMediaType);
        dest.writeInt(mPlayState);
        dest.writeString(mFileName);
        dest.writeString(mFilePath);
        dest.writeString(mArtist);
        dest.writeString(mAlbum);
        dest.writeInt(mCurrentTime);
        dest.writeInt(mTotalTime);
    }
}
