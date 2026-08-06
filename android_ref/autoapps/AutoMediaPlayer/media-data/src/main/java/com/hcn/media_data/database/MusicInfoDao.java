package com.hcn.media_data.database;

import android.annotation.SuppressLint;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteConstraintException;
import android.database.sqlite.SQLiteDatabase;
import android.widget.Toast;

import com.hcn.media_common.debug.LogUtil;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.ArrayList;
import java.util.List;


/**
 * 媒体信息数据结构
 * <p> 未使用，保留参考；
 *
 * @author 65821
 */
public class MusicInfoDao {
    private static final String TAG = "OrdersDao";

    /** 列定义 **/
    private final String[] ORDER_COLUMNS =
            new String[]{"mID3Type", "mIndex", "mTotalTime", "mTotalSize",
                    "mTitle", "mArtist", "mAlbum", "mFilePath", "mFileName"};

    private Context context;
    private MusicInfoDBHelper ordersDBHelper;

    public MusicInfoDao(Context context) {
        this.context = context;
        ordersDBHelper = new MusicInfoDBHelper(context);
    }

    /**
     * 判断表中是否有数据
     */
    public boolean isDataExist() {
        int count = 0;

        SQLiteDatabase db = null;
        Cursor cursor = null;

        try {
            db = ordersDBHelper.getReadableDatabase();


            // select count(Id) from Orders
            cursor = db.query(MusicInfoDBHelper.TABLE_NAME,
                    new String[]{"COUNT(Id)"}, null,
                    null, null, null, null);

            if (cursor.moveToFirst()) {
                count = cursor.getInt(0);
            }

            if (count > 0) {
                return true;
            }
        } catch (Exception e) {
            LogUtil.e(TAG, "" + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }

            if (db != null) {
                db.close();
            }
        }

        return false;
    }

    /**
     * 初始化数据
     */
    public void initTable() {
        SQLiteDatabase db = null;

        try {
            db = ordersDBHelper.getWritableDatabase();
            db.beginTransaction();

            db.execSQL("insert into " + MusicInfoDBHelper.TABLE_NAME
                    + " (Id, Question, Answer, Remark) values (1, '我帅么', '你好帅', 'null')");
            db.execSQL("insert into " + MusicInfoDBHelper.TABLE_NAME
                    + " (Id, Question, Answer, Remark) values (2, '我美么', '你好美', 'null')");
            db.execSQL("insert into " + MusicInfoDBHelper.TABLE_NAME
                    + " (Id, Question, Answer, Remark) values (3, '我美么', '你好美', 'null')");
            db.execSQL("insert into " + MusicInfoDBHelper.TABLE_NAME
                    + " (Id, Question, Answer, Remark) values (4, '我美么', '你好美', 'null')");
            db.execSQL("insert into " + MusicInfoDBHelper.TABLE_NAME
                    + " (Id, Question, Answer, Remark) values (5, '我帅么', '你好帅', 'null')");
            db.execSQL("insert into " + MusicInfoDBHelper.TABLE_NAME
                    + " (Id, Question, Answer, Remark) values (6, '我帅么', '你好帅', 'null')");

            db.setTransactionSuccessful();
        } catch (Exception e) {
            LogUtil.e(TAG, "" + e);
        } finally {
            if (db != null) {
                db.endTransaction();
                db.close();
            }
        }
    }

    /**
     * 执行自定义 SQL 语句
     */
    public void execSQL(String sql) {
        SQLiteDatabase db = null;

        try {
            if (sql.contains("select")) {
                Toast.makeText(context, "select", Toast.LENGTH_SHORT).show();
            } else if (sql.contains("insert") || sql.contains("update") || sql.contains("delete")) {
                db = ordersDBHelper.getWritableDatabase();
                db.beginTransaction();
                db.execSQL(sql);
                db.setTransactionSuccessful();
                Toast.makeText(context, "success", Toast.LENGTH_SHORT).show();
            }
        } catch (Exception e) {
            Toast.makeText(context, e.toString(), Toast.LENGTH_SHORT).show();
            LogUtil.e(TAG, "" + e);
        } finally {
            if (db != null) {
                db.endTransaction();
                db.close();
            }
        }
    }

    /**
     * 查询数据库中所有数据
     */
    public List<MusicInfo> getAllDate() {
        SQLiteDatabase db = null;
        Cursor cursor = null;

        try {
            db = ordersDBHelper.getReadableDatabase();
            // select * from Orders
            cursor = db.query(MusicInfoDBHelper.TABLE_NAME, ORDER_COLUMNS, null, null, null, null,
                    null);

            if (cursor.getCount() > 0) {
                List<MusicInfo> orderList = new ArrayList<MusicInfo>(cursor.getCount());
                while (cursor.moveToNext()) {
                    orderList.add(parseOrder(cursor));
                }
                return orderList;
            }
        } catch (Exception e) {
            LogUtil.e(TAG, "" + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
            if (db != null) {
                db.close();
            }
        }

        return null;
    }

    /**
     * 新增一条数据
     */
    public boolean insertData(int mID3Type, int mIndex, int mTotalTime, int mTotalSize,
            String mTitle, String mArtist, String mAlbum, String mFilePath, String mFileName) {
        SQLiteDatabase db = null;

        try {
            db = ordersDBHelper.getWritableDatabase();
            db.beginTransaction();

            // insert into Orders(Id, CustomName, OrderPrice, Country) values (7, "Jne", 700,
            // "China");
            ContentValues contentValues = new ContentValues();
            contentValues.put("mID3Type", mID3Type);
            contentValues.put("mIndex", mIndex);
            contentValues.put("mTotalTime", mTotalTime);
            contentValues.put("mTotalSize", mTotalSize);
            contentValues.put("mTitle", mTitle);
            contentValues.put("mArtist", mArtist);
            contentValues.put("mAlbum", mAlbum);
            contentValues.put("mFilePath", mFilePath);
            contentValues.put("mFileName", mFileName);
            db.insertOrThrow(MusicInfoDBHelper.TABLE_NAME, null, contentValues);

            db.setTransactionSuccessful();
            return true;
        } catch (SQLiteConstraintException e) {
            Toast.makeText(context, "主键重复", Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
            LogUtil.e(TAG, "" + e);
        } finally {
            if (db != null) {
                db.endTransaction();
                db.close();
            }
        }
        return false;
    }

    /**
     * 删除一条数据  此处删除Id为的数据
     */
    public boolean deleteInstruct(String mFilePath) {
        SQLiteDatabase db = null;

        try {
            db = ordersDBHelper.getWritableDatabase();
            db.beginTransaction();

            // delete from Orders where Id = 7
            db.delete(MusicInfoDBHelper.TABLE_NAME, "mFilePath = ?",
                    new String[]{String.valueOf(mFilePath)});
            db.setTransactionSuccessful();
            return true;
        } catch (Exception e) {
            LogUtil.e(TAG, "" + e);
        } finally {
            if (db != null) {
                db.endTransaction();
                db.close();
            }
        }
        return false;
    }

    /**
     * 修改一条数据  此处将Id为6的数据的OrderPrice修改了800
     */
    public boolean updateOrder(int id, String question, String answer) {
        SQLiteDatabase db = null;
        try {
            db = ordersDBHelper.getWritableDatabase();
            db.beginTransaction();

            // update Orders set OrderPrice = 800 where Id = 6
            ContentValues cv = new ContentValues();
            cv.put("Question", question);
            cv.put("Answer", answer);
            db.update(MusicInfoDBHelper.TABLE_NAME,
                    cv,
                    "Id = ?",
                    new String[]{String.valueOf(id)});
            db.setTransactionSuccessful();
            return true;
        } catch (Exception e) {
            LogUtil.e(TAG, "" + e);
        } finally {
            if (db != null) {
                db.endTransaction();
                db.close();
            }
        }

        return false;
    }

    /**
     * 数据查询  此处将用户名为"Bor"的信息提取出来
     */
    public List<MusicInfo> getBorOrder() {
        SQLiteDatabase db = null;
        Cursor cursor = null;

        try {
            db = ordersDBHelper.getReadableDatabase();

            // select * from Orders where CustomName = 'Bor'
            cursor = db.query(MusicInfoDBHelper.TABLE_NAME,
                    ORDER_COLUMNS,
                    "CustomName like ?",
                    new String[]{"Bor"},
                    null, null, null);

            if (cursor.getCount() > 0) {
                List<MusicInfo> orderList = new ArrayList<MusicInfo>(cursor.getCount());
                while (cursor.moveToNext()) {
                    MusicInfo order = parseOrder(cursor);
                    orderList.add(order);
                }
                return orderList;
            }
        } catch (Exception e) {
            LogUtil.e(TAG, "" + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
            if (db != null) {
                db.close();
            }
        }

        return null;
    }

    /**
     * 统计查询  此处查询Country为China的用户总数
     */
    public int getChinaCount() {
        int count = 0;

        SQLiteDatabase db = null;
        Cursor cursor = null;

        try {
            db = ordersDBHelper.getReadableDatabase();
            // select count(Id) from Orders where Country = 'China'
            cursor = db.query(MusicInfoDBHelper.TABLE_NAME,
                    new String[]{"COUNT(Id)"},
                    "Country = ?",
                    new String[]{"China"},
                    null, null, null);

            if (cursor.moveToFirst()) {
                count = cursor.getInt(0);
            }
        } catch (Exception e) {
            LogUtil.e(TAG, "" + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
            if (db != null) {
                db.close();
            }
        }

        return count;
    }

    /**
     * 比较查询  此处查询单笔数据中OrderPrice最高的
     */
    public MusicInfo getMaxOrderPrice() {
        SQLiteDatabase db = null;
        Cursor cursor = null;

        try {
            db = ordersDBHelper.getReadableDatabase();
            // select Id, CustomName, Max(OrderPrice) as OrderPrice, Country from Orders
            cursor = db.query(MusicInfoDBHelper.TABLE_NAME,
                    new String[]{"Id", "CustomName", "Max(OrderPrice) as OrderPrice", "Country"},
                    null, null, null, null, null);

            if (cursor.getCount() > 0) {
                if (cursor.moveToFirst()) {
                    return parseOrder(cursor);
                }
            }
        } catch (Exception e) {
            LogUtil.e(TAG, "" + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
            if (db != null) {
                db.close();
            }
        }

        return null;
    }

    /**
     * 将查找到的数据转换成Order类
     */
    @SuppressLint("Range")
    private MusicInfo parseOrder(Cursor cursor) {
        MusicInfo musicInfo = new MusicInfo();
        musicInfo.mID3Type = cursor.getInt(
                cursor.getColumnIndex("mID3Type"));
        musicInfo.mIndex = cursor.getInt(
                cursor.getColumnIndex("mIndex"));
        musicInfo.mTotalTime = cursor.getInt(
                cursor.getColumnIndex("mTotalTime"));
        musicInfo.mTotalSize = cursor.getInt(
                cursor.getColumnIndex("mTotalSize"));
        musicInfo.mTitle = cursor.getString(
                cursor.getColumnIndex("mTitle"));
        musicInfo.mArtist = cursor.getString(
                cursor.getColumnIndex("mArtist"));
        musicInfo.mAlbum = cursor.getString(
                cursor.getColumnIndex("mAlbum"));
        musicInfo.mFilePath = cursor.getString(
                cursor.getColumnIndex("mFilePath"));
        musicInfo.mFileName = cursor.getString(
                cursor.getColumnIndex("mFileName"));
        return musicInfo;
    }
}