package com.hcn.autoradio.collection;

import com.hcn.org.litepal.crud.DataSupport;

/**
 * @author simon
 */
public class FMCollectListInfo extends DataSupport {
    private int column_band;
    private String column_freq;
    private int column_pos;

    public int getColumnBand() {
        return column_band;
    }

    public String getColumnFreq() {
        return column_freq;
    }

    public int getColumnPos() {
        return column_pos;
    }

    public void setColumnBand(int column_band) {
        this.column_band = column_band;
    }

    public void setColumnFreq(String column_freq) {
        this.column_freq = column_freq;
    }

    public void setColumnPos(int column_pos) {
        this.column_pos = column_pos;
    }
}
