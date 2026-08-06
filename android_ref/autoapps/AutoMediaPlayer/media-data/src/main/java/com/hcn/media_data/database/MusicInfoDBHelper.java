package com.hcn.media_data.database;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

/**
 * Created by Jne Date: 2015/1/6.
 * @author 65821
 */
public class MusicInfoDBHelper extends SQLiteOpenHelper {
    public static final String TABLE_NAME = "MusicInfo";
    private static final int DB_VERSION = 1;
    private static final String DB_NAME = "faver_music_list.db";

    public MusicInfoDBHelper(Context context) {
        super(context, DB_NAME, null, DB_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase sqLiteDatabase) {
        // create table Orders(Id integer primary key, CustomName text, OrderPrice integer,
        // Country text);
        String sql = "create table if not exists " + TABLE_NAME +
                " (Id integer primary key, mID3Type integer, mIndex integer, mTotalTime integer, "
                + "mTotalSize integer, mTitle text, mArtist text, mAlbum text, mFilePath text, "
                + "mFileName text)";
        sqLiteDatabase.execSQL(sql);
    }

    @Override
    public void onUpgrade(SQLiteDatabase sqLiteDatabase, int oldVersion, int newVersion) {
        String sql = "DROP TABLE IF EXISTS " + TABLE_NAME;
        sqLiteDatabase.execSQL(sql);
        onCreate(sqLiteDatabase);
    }
}