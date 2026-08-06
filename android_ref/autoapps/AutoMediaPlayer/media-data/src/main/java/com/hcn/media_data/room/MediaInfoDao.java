package com.hcn.media_data.room;

import androidx.room.Dao;
import androidx.room.Delete;
import androidx.room.Insert;
import androidx.room.Query;
import androidx.room.Update;

import java.util.List;

/**
 * 媒体文件数据访问对象
 * <pre>
 *    DAO: Data Access Objects
 *    请遵循命名规则（与关联的数据表：{tableName}Dao）;
 *    这是一个 @Dao 修饰的接口，编译的时候 apd 工具会实现它；
 * </pre>
 *
 * @author 65821
 */
@Dao
public interface MediaInfoDao {
    /**
     * 插入数据对象
     *
     * @param mediaInfos 媒体文件对象集
     */
    @Insert
    void insert(MediaInfo... mediaInfos);

    /**
     * 删除数据对象
     *
     * @param mediaInfo 媒体文件对象
     */
    @Delete
    void delete(MediaInfo mediaInfo);

    /**
     * 更新数据对象
     *
     * @param mediaInfo 媒体文件对象
     */
    @Update
    void update(MediaInfo mediaInfo);

    /**
     * 查找表中的所有数据
     *
     * @return 媒体文件表数据集合
     */
    @Query("select * from media_table")
    List<MediaInfo> getAll();

    /**
     * 数组查询多个记录
     *
     * @param userIds 目标序列集合
     * @return 媒体文件数据集合
     */
    @Query("select * from media_table where uid in(:userIds)")
    List<MediaInfo> getAllId(int[] userIds);

    /**
     * 从媒体类型查询记录
     * <p> like:mediaType 匹配参数
     *
     * @param mediaType 媒体类型
     * @return 媒体文件对象
     */
    @Query("select * from media_table where mediaType like:mediaType")
    List<MediaInfo> findByMediaType(String mediaType);

    /**
     * 从媒体设备名字查询记录
     * <p> like:deviceName 匹配参数
     *
     * @param deviceName 设备名字 (deviceName: "/storage/udisk%" ...);
     * @return 媒体文件对象
     */
    @Query("select * from media_table where deviceName like:deviceName")
    List<MediaInfo> findByDeviceName(String deviceName);

    /**
     * 从媒体类型和媒体设备名字查询记录
     * <pre>
     *    e.g. 查询 U 盘中的音乐信息：
     *      mediaType = 0(Music)
     *      deviceName = "/storage/udisk%"
     * </pre>
     *
     * @param mediaType  媒体类型
     * @param deviceName 设备名字 (deviceName: "/storage/udisk%" ...);
     * @return 媒体文件对象
     */
    @Query("select * from media_table" +
            " where mediaType like:mediaType" +
            " AND deviceName like:deviceName")
    List<MediaInfo> findByMediaTypeAndDeviceName(String mediaType, String deviceName);

    /**
     * 从文件标题查询记录
     * <p> like:title 匹配参数
     *
     * @param title 目标标题
     * @return 媒体文件对象
     */
    @Query("select * from media_table where title like:title")
    List<MediaInfo> findByTitle(String title);

    /**
     * 从文件专辑查询记录
     * <p> like:album 匹配参数
     *
     * @param album 目标专辑
     * @return 媒体文件对象
     */
    @Query("select * from media_table where album like:album")
    List<MediaInfo> findByAlbum(String album);

    /**
     * 从文件歌手查询记录
     * <p> like:artist 匹配参数
     *
     * @param artist 目标专辑
     * @return 媒体文件对象
     */
    @Query("select * from media_table where artist like:artist")
    List<MediaInfo> findByArtist(String artist);

    /**
     * 从文件名称查询记录
     * <p> like:name 匹配参数
     *
     * @param name 目标标题
     * @return 媒体文件对象
     */
    @Query("select * from media_table where name like:name")
    List<MediaInfo> findByName(String name);

    /**
     * 从文件路径查询记录
     * <p> like:filePath 匹配参数
     *
     * @param filePath 目标文件路径
     * @return 媒体文件对象
     */
    @Query("select * from media_table where filepath like:filePath")
    List<MediaInfo> findByFilePath(String filePath);
}
