package com.hcn.media.base;

import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;

import androidx.annotation.NonNull;

/**
 * 当前媒体播放信息
 * @author 65821
 */
public class MediaPlayInfo implements Parcelable {
    /** 媒体文件路径 **/
    private String filePath;

    /** 媒体文件标题 **/
    private String title;

    /** 媒体艺术家名 **/
    private String artist;

    /** 媒体专辑名称 **/
    private String album;

    /** 媒体歌词文件 */
    private String lyricsFile;

    /** 媒体播放状态 */
    private String state;

    /** 媒体持续时间 **/
    private int duration;

    /** 当前播放时间 **/
    private int position;

    public MediaPlayInfo() {
    }

    public MediaPlayInfo(String filePath, String title) {
        this.filePath = filePath;
        this.title = title;
    }

    public static final Creator<MediaPlayInfo> CREATOR =
            new Creator<MediaPlayInfo>() {

                @Override
                public MediaPlayInfo createFromParcel(Parcel source) {
                    return new MediaPlayInfo(source);
                }

                @Override
                public MediaPlayInfo[] newArray(int size) {
                    return new MediaPlayInfo[size];
                }
            };

    private MediaPlayInfo(Parcel source) {
        filePath = source.readString();
        title = source.readString();
        artist = source.readString();
        album = source.readString();
        lyricsFile = source.readString();
        state = source.readString();
        duration = source.readInt();
        position = source.readInt();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(@NonNull Parcel dest, int flags) {
        dest.writeString(filePath);
        dest.writeString(title);
        dest.writeString(artist);
        dest.writeString(album);
        dest.writeString(lyricsFile);
        dest.writeString(state);
        dest.writeInt(duration);
        dest.writeInt(position);
    }

    public boolean isValid() {
        return !TextUtils.isEmpty(this.filePath);
    }

    public String getFilePath() {
        return this.filePath;
    }

    public void setFilePath(String filePath) {
        // 文件信息变化需重置数据
        if (TextUtils.isEmpty(filePath)
                || !filePath.equals(this.filePath)) {
            this.reset();
        }

        this.filePath = filePath;
    }

    public String getTitle() {
        return this.title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public String getArtist() {
        return this.artist;
    }

    public void setArtist(String artist) {
        this.artist = artist;
    }

    public String getAlbum() {
        return this.album;
    }

    public void setAlbum(String album) {
        this.album = album;
    }

    public String getLyricsFile() {
        return this.lyricsFile;
    }

    public void setLyricsFile(String lyricsFile) {
        this.lyricsFile = lyricsFile;
    }

    public void setState(String state) {
        this.state = state;
    }

    public String getState() {
        return this.state;
    }

    public int getDuration() {
        return this.duration;
    }

    public void setDuration(int duration) {
        this.duration = duration;
    }

    public int getCurrentPosition() {
        return this.position;
    }

    public void setCurrentPosition(int position) {
        this.position = position;
    }

    public void reset() {
        this.filePath = "";
        this.title = "";
        this.artist = "";
        this.album = "";
        this.lyricsFile = "";
        this.duration = 0;
        this.position = 0;
    }

    public String toString() {
        return "MediaPlayInfo{filePath='" + this.filePath + '\''
                + ", title='" + this.title + '\''
                + ", artist='" + this.artist + '\''
                + ", album='" + this.album + '\''
                + ", lyricFile='" + this.lyricsFile + '\''
                + ", duration=" + this.duration + ", position=" + this.position + '}';
    }
}

