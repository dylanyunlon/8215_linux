package com.hcn.media_data.room;

import androidx.room.ColumnInfo;
import androidx.room.Entity;
import androidx.room.PrimaryKey;

/**
 * 媒体文件信息表
 * <pre>
 *    这里 @Entity 修饰表示下面是一个数据表；
 *    数据表必须使用 JavaBean 命名规则来编码；
 * </pre>
 *
 * @author 65821
 */
@Entity(tableName ="media_table")
public class MediaInfo {
    /** 主键 ID (自增) **/
    @PrimaryKey(autoGenerate = true)
    private int uid;

    /**
     * 媒体文件类型
     * <pre>
     *    0：音乐文件类型；
     *    1：视频文件类型；
     *    2：图片文件类型；
     * </pre>
     */
    @ColumnInfo(name = "mediaType", typeAffinity = ColumnInfo.INTEGER)
    private int mediaType;

    /**
     * 媒体文件所在存储设备名称
     * <pre>
     *    存储设备类型，使用存储文件路径前缀表示；
     *    "/storage/emulated/0"： 内置存储设备
     *    "/storage/ext_sdcard_"： 外置 SD 卡
     *    “/storage/udisk%”：外接 USB 存储设备
     * </pre>
     */
    @ColumnInfo(name = "deviceName", typeAffinity = ColumnInfo.TEXT)
    private String deviceName;

    /**
     * 存储 id3 类型
     * <p> 避免重复解析 id3 信息；
     */
    @ColumnInfo(name = "id3type", typeAffinity = ColumnInfo.INTEGER)
    private int id3type;

    /** 媒体文件流时长 **/
    @ColumnInfo(name = "duration", typeAffinity = ColumnInfo.INTEGER)
    private int duration;

    /** 媒体 ID3 名称 **/
    @ColumnInfo(name = "title", typeAffinity = ColumnInfo.TEXT)
    private String title;

    /** 媒体 ID3 专辑 **/
    @ColumnInfo(name = "album", typeAffinity = ColumnInfo.TEXT)
    private String album;

    /** 媒体 ID3 歌手 **/
    @ColumnInfo(name = "artist", typeAffinity = ColumnInfo.TEXT)

    private String artist;
    /** 媒体文件的名称 **/
    @ColumnInfo(name = "name", typeAffinity = ColumnInfo.TEXT)
    private String name;

    /** 媒体文件的路径 **/
    @ColumnInfo(name = "filepath", typeAffinity = ColumnInfo.TEXT)
    private String filePath;

    /** 媒体文件的大小 **/
    @ColumnInfo(name = "size", typeAffinity = ColumnInfo.INTEGER)
    private int totalSize;

    public int getUid() {
        return uid;
    }

    public void setUid(int uid) {
        this.uid = uid;
    }

    public int getMediaType() {
        return mediaType;
    }

    public void setMediaType(int mediaType) {
        this.mediaType = mediaType;
    }

    public String getDeviceName() {
        return deviceName;
    }

    public void setDeviceName(String deviceName) {
        this.deviceName = deviceName;
    }

    public int getId3type() {
        return id3type;
    }

    public void setId3type(int id3type) {
        this.id3type = id3type;
    }

    public int getDuration() {
        return duration;
    }

    public void setDuration(int duration) {
        this.duration = duration;
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public String getAlbum() {
        return album;
    }

    public void setAlbum(String album) {
        this.album = album;
    }

    public String getArtist() {
        return artist;
    }

    public void setArtist(String artist) {
        this.artist = artist;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getFilePath() {
        return filePath;
    }

    public void setFilePath(String filePath) {
        this.filePath = filePath;
    }

    public int getTotalSize() {
        return totalSize;
    }

    public void setTotalSize(int totalSize) {
        this.totalSize = totalSize;
    }

    @Override
    public String toString() {
        return "MediaInfo{" +
                "uid=" + uid +
                ", id3type='" + id3type + '\'' +
                ", duration='" + duration + '\'' +
                ", title='" + title + '\'' +
                ", album='" + album + '\'' +
                ", artist='" + artist + '\'' +
                ", name='" + name + '\'' +
                ", filePath='" + filePath + '\'' +
                ", totalSize='" + totalSize + '\'' +
                '}';
    }
}
