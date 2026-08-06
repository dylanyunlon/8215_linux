package com.hcn.media_data.room;

import androidx.room.Dao;
import androidx.room.Delete;
import androidx.room.Insert;
import androidx.room.Query;
import androidx.room.Update;

import java.util.List;

/**
 * 媒体收藏信息数据访问对象
 * <pre>
 *    DAO: Data Access Objects
 *    请遵循命名规则（与关联的数据表：{tableName}Dao）;
 *    这是一个 @Dao 修饰的接口，编译的时候 apt 工具会实现它；
 *    APT: Annotation Processing Tool
 * </pre>
 *
 * @author 65821
 */
@Dao
public interface FavoriteMusicDao {
    /**
     * 插入数据对象集
     * @param favoriteInfos 媒体文件对象集
     */
    @Insert
    void insert(FavoriteMusic... favoriteInfos);

    /**
     * 插入数据对象集
     * @param list 媒体文件对象集
     */
    @Insert
    void insert(List<FavoriteMusic> list);

    /**
     * 删除数据对象
     * @param favoriteInfo 媒体文件对象
     */
    @Delete
    void delete(FavoriteMusic favoriteInfo);

    /**
     * 更新数据对象
     * @param favoriteInfo 媒体文件对象
     */
    @Update
    void update(FavoriteMusic favoriteInfo);

    /**
     * 查找表中的所有数据
     * @return 媒体文件表数据集合
     */
    @Query("select * from favorite_table")
    List<FavoriteMusic> getAll();

    /**
     * 删除指定路径的数据信息
     * @param filePath 文件路径
     */
    @Query("delete from favorite_table where filepath like:filePath")
    void deleteInfo(String filePath);

    /**
     * 数组查询多个记录
     *
     * @param userIds 目标序列集合
     * @return 媒体文件数据集合
     */
    @Query("select * from favorite_table where uid in(:userIds)")
    List<FavoriteMusic> getAllId(int[] userIds);

    /**
     * 从文件标题查询记录
     * <p> like:title 匹配参数
     *
     * @param title 目标标题
     * @return 媒体文件对象
     */
    @Query("select * from favorite_table where title like:title")
    List<FavoriteMusic> findByTitle(String title);

    /**
     * 从文件路径查询记录
     * <p> like:filePath 匹配参数
     *
     * @param filePath 目标文件路径
     * @return 媒体文件对象
     */
    @Query("select * from favorite_table where filepath like:filePath")
    List<FavoriteMusic> findByFilePath(String filePath);
}
