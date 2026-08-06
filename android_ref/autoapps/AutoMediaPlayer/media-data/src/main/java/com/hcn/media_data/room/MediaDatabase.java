package com.hcn.media_data.room;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.room.Database;
import androidx.room.Room;
import androidx.room.RoomDatabase;
import androidx.room.migration.Migration;
import androidx.sqlite.db.SupportSQLiteDatabase;

import com.hcn.media_common.debug.LogUtil;

/**
 * 媒体数据库
 * <pre>
 *    为了养成好习惯, 规则要写 exportSchema = false, 因为在升级过程中会记录所有的历史版本信息，
 *    因为内部要记录升级的所有副本,使用抽象类是为了能够在编译时期使用 apt 生成真正的类（标准用法）;
 *    暂时只有一个数据表 {@link FavoriteMusic}
 * </pre>
 *
 * @author 65821
 */
@Database(entities = {FavoriteMusic.class, MediaInfo.class}, version = 2, exportSchema = false)
public abstract class MediaDatabase extends RoomDatabase {
    private final static String TAG = MediaDatabase.class.getSimpleName();

    /**
     * 最喜欢的音乐收藏列表最大存储个数
     * <pre>
     *    我们约定一个收藏列表最多可以收藏 256 首歌曲（参考主流播放器：QQ 音乐）；
     *    对应的收藏列表收据库我们最多只存储 1024 首歌曲；
     *    一切都是为了效率（正常用户收藏这么多歌曲搓搓有余）；
     * </pre>
     */
    public static int FAVORITE_MUSIC_MAX_STORE_SIZE = 1024;

    /**
     * 最多存储 10000 首歌曲信息
     * <pre>
     *    京东卖的 32G U 盘也就带 6000 首歌曲；
     *    我们这里存储个 10000 个媒体信息，基本满足 99.99% 的客户了；
     *    一切还是为了效率，存储多没没有意义，反而违背初衷（原则：读取解析不能超过 2S）；
     * </pre>
     */
    public static int MEDIA_INFO_MAX_STORE_SIZE = 10000;

    /** 类唯一实例对象 **/
    private static MediaDatabase databaseInstance;

    /** 数据库文件名字 **/
    private final static String DATABASE_NAME = "auto_media.db";

    /**
     * 数据库唯一实例
     * @return {@link RoomDatabase}
     */
    public static synchronized MediaDatabase instance() {
        if (databaseInstance == null) {
            throw new NullPointerException(
                    "Please initialize [MediaDatabase] Object!");
        }
        return databaseInstance;
    }

    /**
     * 数据库唯一实例
     * @param context 应用上下文环境
     * @return {@link RoomDatabase}
     */
    public static synchronized MediaDatabase instance(Context context) {
        if (databaseInstance == null) {
            // 指定数据库文件路径
            final String databasePath =
                    context.getDatabasePath(DATABASE_NAME).getAbsolutePath();

            // 构建媒体数据库对象
            databaseInstance = Room.databaseBuilder(
                    context.getApplicationContext(), MediaDatabase.class, databasePath)
                    .addMigrations(MIGRATION_1_2)
                    .fallbackToDestructiveMigration()
                    .addCallback(new RoomDatabase.Callback() {
                        @Override
                        public void onCreate(@NonNull SupportSQLiteDatabase db) {
                            super.onCreate(db);
                            LogUtil.v(TAG, "onCreate, path = " + db.getPath());
                        }

                        @Override
                        public void onOpen(@NonNull SupportSQLiteDatabase db) {
                            super.onOpen(db);
                            LogUtil.v(TAG, "onOpen.");
                        }

                        @Override
                        public void onDestructiveMigration(@NonNull SupportSQLiteDatabase db) {
                            super.onDestructiveMigration(db);
                        }
                    }).build();
        }

        return databaseInstance;
    }

    @Override
    public void close() {
        super.close();
    }

    /**
     * 对外暴露 FavoriteMusicDao 对象访问接口
     * @return Data Access Objects
     */
    public abstract FavoriteMusicDao favoriteMusicDao();

    /**
     * 对外暴露 MediaInfoDao 对象访问接口
     * @return Data Access Objects
     */
    public abstract MediaInfoDao mediaInfoDao();

    /**
     * 数据库版本更新处理
     * <pre>
     *    这个是版本 1 更新到版本 2 的迁移处理；
     *    对表 favorite_table 增加 value 字段；
     * </pre>
     */
    static final Migration MIGRATION_1_2 = new Migration(1, 2) {

        @Override
        public void migrate(@NonNull SupportSQLiteDatabase database) {
            LogUtil.v(TAG, "migrate/1-2: favorite_table!");
            database.execSQL("ALTER TABLE 'favorite_table' ADD COLUMN 'value' INTEGER NOT NULL DEFAULT 0");
        }
    };
}
