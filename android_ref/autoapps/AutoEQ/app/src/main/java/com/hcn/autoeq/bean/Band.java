package com.hcn.autoeq.bean;

import android.os.Parcel;
import android.os.Parcelable;

public class Band implements Parcelable {
    private int index;
    private int gain;
    private int q;
    private int freq;
    private int type;
    private int bypass;

    public Band() {

    }

    public Band(int index, int gain, int q, int freq, int type, int bypass) {
        this.index = index;
        this.gain = gain;
        this.q = q;
        this.freq = freq;
        this.type = type;
        this.bypass = bypass;
    }

    protected Band(Parcel in) {
        index = in.readInt();
        gain = in.readInt();
        q = in.readInt();
        freq = in.readInt();
        type = in.readInt();
        bypass = in.readInt();
    }

    public static final Creator<Band> CREATOR = new Creator<Band>() {
        @Override
        public Band createFromParcel(Parcel in) {
            return new Band(in);
        }

        @Override
        public Band[] newArray(int size) {
            return new Band[size];
        }
    };

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(index);
        dest.writeInt(gain);
        dest.writeInt(q);
        dest.writeInt(freq);
        dest.writeInt(type);
        dest.writeInt(bypass);
    }

    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
    }

    public int getGain() {
        return gain;
    }

    public void setGain(int gain) {
        this.gain = gain;
    }

    public int getQ() {
        return q;
    }

    public void setQ(int q) {
        this.q = q;
    }

    public int getFreq() {
        return freq;
    }

    public void setFreq(int freq) {
        this.freq = freq;
    }

    public int getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
    }

    public int getBypass() {
        return bypass;
    }

    public void setBypass(int bypass) {
        this.bypass = bypass;
    }

    @Override
    public String toString() {
        return "Band{" +
                "index=" + index +
                ", gain=" + gain +
                ", q=" + q +
                ", freq=" + freq +
                ", type=" + type +
                ", bypass=" + bypass +
                '}';
    }
}
