package com.hcn.media_data.room;

import androidx.room.ColumnInfo;
import androidx.room.Entity;
import androidx.room.Ignore;
import androidx.room.PrimaryKey;

import java.util.Comparator;

/**
 * 媒体收藏文件信息表
 * <pre>
 *    这里 @Entity 修饰表示下面是一个数据表；
 *    数据表必须使用 JavaBean 命名规则来编码；
 * </pre>
 *
 * @deprecated 这个应该尽快升级数据库版本，添加 ID3 部分信息存储；
 * @author 65821
 */
@Entity(tableName ="favorite_table")
public class FavoriteMusic {
    @PrimaryKey(autoGenerate = true)
    private int uid;

    /**
     * 收藏媒体文件权重
     * <p> 只要每次检查文件有效存在就 +1（值越大表示有效次数越多）；
     */
    @ColumnInfo(name = "weight", typeAffinity = ColumnInfo.INTEGER)
    private int weight;

    /**
     * 当前媒体文件价值
     * <p> 每次检查文件不存在就 -1，存在就重置为 0（值越小表示越久未使用过）
     */
    @ColumnInfo(name = "value", typeAffinity = ColumnInfo.INTEGER)
    private int value;

    /** 收藏媒体文件名称 **/
    @ColumnInfo(name = "title", typeAffinity = ColumnInfo.TEXT)
    private String title;

    /** 收藏媒体文件路径 **/
    @ColumnInfo(name = "filepath", typeAffinity = ColumnInfo.TEXT)
    private String filePath;

    /** 非表字段（文件是否存在标记）**/
    @Ignore
    private boolean fileExist;

    /** 是否需要更新到数据库（标记）**/
    @Ignore
    private boolean allowUpdate;

    public FavoriteMusic(String title, String filePath) {
        this.title = title;
        this.filePath = filePath;
        this.fileExist = false;
        this.allowUpdate = false;
    }

    public int getUid() {
        return uid;
    }

    public void setUid(int uid) {
        this.uid = uid;
    }

    public int getWeight() {
        return this.weight;
    }

    public int getValue() {
        return this.value;
    }

    public String getTitle() {
        return this.title;
    }

    public String getFilePath() {
        return this.filePath;
    }

    public boolean isFileExist() {
        return this.fileExist;
    }

    public boolean isAllowUpdate() {
        return this.allowUpdate;
    }

    public void setWeight(int weight) {
        this.weight = weight;
    }

    public void setValue(int value) {
        this.value = value;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public void setFilePath(String filePath) {
        this.filePath = filePath;
    }

    public void setFileExist(boolean exist) {
        this.fileExist = exist;
    }

    public void setAllowUpdate(boolean allowUpdate) {
        this.allowUpdate = allowUpdate;
    }

    /**
     * 价值比较器（降序）
     * <p> 按价值降序排列, 返回负数和零不交换，返回正数交换 o1 和 02
     */
    public static final Comparator<FavoriteMusic> mValueComparator = (o1, o2) -> {
        int value1 = o1.getValue();
        int value2 = o2.getValue();
        return value2 - value1;
    };

    @Override
    public String toString() {
        return "FavoriteInfo{" +
                "uid=" + uid +
                ", weight='" + weight + '\'' +
                ", value='" + value + '\'' +
                ", title='" + title + '\'' +
                ", filePath='" + filePath + '\'' +
                ", fileExist='" + fileExist + '\'' +
                '}';
    }
}
