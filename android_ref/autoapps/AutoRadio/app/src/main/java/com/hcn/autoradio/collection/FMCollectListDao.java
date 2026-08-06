package com.hcn.autoradio.collection;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.os.Build;

import com.hcn.org.litepal.crud.DataSupport;
import com.hcn.org.litepal.tablemanager.Connector;

import java.util.List;

/**
 * @author simon
 */
public class FMCollectListDao {
    private static SQLiteDatabase db = null;
    private static String column_name_band = "column_band";
    private static String column_name_freq = "column_freq";
    private static String column_name_pos = "column_pos";

    public FMCollectListDao() {
        db = Connector.getDatabase();
        if (Build.VERSION.SDK_INT>=Build.VERSION_CODES.O){
            db.disableWriteAheadLogging();
        }
    }

    public static void initFMCollectListDateBase(Context context) {
        db = Connector.getDatabase();
        if (Build.VERSION.SDK_INT>=Build.VERSION_CODES.O){
            db.disableWriteAheadLogging();
        }
    }

    public static List<FMCollectListInfo> queryCollectListAllInfos() {
        List<FMCollectListInfo> collectListInfo = DataSupport.findAll(FMCollectListInfo.class);
        return collectListInfo;
    }

    public static List<FMCollectListInfo> queryFreByColumn_Band(int band) {
        List<FMCollectListInfo> collectListInfo = null;
        collectListInfo = DataSupport.select("column_band").where("id > ? and column_band = ? ","0",
                String.valueOf(band)).find(FMCollectListInfo.class);
        return collectListInfo;
    }

    public static List<FMCollectListInfo> queryFreByColumn_BandAndPos(int band, int pos) {
        List<FMCollectListInfo> collectListInfo = null;
        collectListInfo = DataSupport.select("column_band", "column_pos").where("id > ? and column_band = ?" +
                        " and column_pos = ?",
                String.valueOf(band), String.valueOf(pos)).find(FMCollectListInfo.class);
        return collectListInfo;
    }

    public static int deleteAll() {
        return DataSupport.deleteAll(FMCollectListInfo.class);
    }

    public static int deleteFreqByColumn(int column_band) {
        return  DataSupport.deleteAll(FMCollectListInfo.class, "column_band = ?",
                String.valueOf(column_band));
    }

    public static int deleteFreqByColumn(int column_band, String column_freq) {
        return DataSupport.deleteAll(FMCollectListInfo.class,"column_band = ? and column_freq = " +
                "?",String.valueOf(column_band), column_freq);
    }

    public static int deleteFreqByColumn(int column_band, int column_pos) {
        return DataSupport.deleteAll(FMCollectListInfo.class,"column_band = ? and column_pos = " +
                "?",String.valueOf(column_band), String.valueOf(column_pos));
    }

    public static boolean addCollectInfo(int band, String freq, int pos) {
        FMCollectListInfo collectListInfo = new FMCollectListInfo();
        collectListInfo.setColumnBand(band);
        collectListInfo.setColumnFreq(freq);
        collectListInfo.setColumnPos(pos);
        return collectListInfo.save();
    }

    public static boolean updateCollectedInfo(int srcBand, int srcPos, String newFreq) {
        FMCollectListInfo collectListInfo = new FMCollectListInfo();
        collectListInfo.setColumnFreq(newFreq);
        collectListInfo.updateAll("column_band = ? and column_pos = ?",
                String.valueOf(srcBand), String.valueOf(srcPos));
        return true;
    }

    public static boolean updateCollectedInfo(int srcBand, String oldFreq, String newFreq) {
        FMCollectListInfo collectListInfo = new FMCollectListInfo();
        collectListInfo.setColumnFreq(newFreq);
        collectListInfo.updateAll("column_band = ? and column_name_freq = ?",
                String.valueOf(srcBand), String.valueOf(oldFreq));
        return true;
    }
}