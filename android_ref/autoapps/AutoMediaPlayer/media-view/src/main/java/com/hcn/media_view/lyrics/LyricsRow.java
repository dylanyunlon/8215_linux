package com.hcn.media_view.lyrics;

import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;

import androidx.annotation.NonNull;

import java.util.ArrayList;
import java.util.List;

/**
 * 歌词行信息类
 * <pre>
 *    用来描述歌词的每一行信息；
 *    [time] content
 * </pre>
 *
 * @author 86158
 */
public class LyricsRow implements Parcelable, Comparable<LyricsRow> {
    private String timeInfo = null;
    private int time;
    private String content;
    private int totalTime;

    public LyricsRow() {
        super();
    }

    public LyricsRow(String timeStr, int time, String content) {
        super();

        this.timeInfo = timeStr;
        this.time = time;
        this.content = content;
    }

    public static final Creator<LyricsRow> CREATOR =
            new Creator<LyricsRow>() {

                @Override
                public LyricsRow createFromParcel(Parcel source) {
                    return new LyricsRow(source);
                }

                @Override
                public LyricsRow[] newArray(int size) {
                    return new LyricsRow[size];
                }
            };

    private LyricsRow(Parcel source) {
        timeInfo = source.readString();
        time = source.readInt();
        content = source.readString();
        totalTime = source.readInt();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(@NonNull Parcel dest, int flags) {
        dest.writeString(timeInfo);
        dest.writeInt(time);
        dest.writeString(content);
        dest.writeInt(totalTime);
    }

    /**
     * 创建歌词信息
     * <pre>
     *     兼容不同的歌词时间标记格式；
     *     Format: [00:00:00] / [00:00:000]
     * </pre>
     *
     * @param lrcLine 歌词行信息
     * @return 歌词列（LyricsRow）集合
     */
    public static List<LyricsRow> createRows(String lrcLine) {
        boolean start = lrcLine.startsWith("[");
        int endFlagIndex = lrcLine.indexOf("]");
        if (!start || endFlagIndex != 9 && endFlagIndex != 10) {
            return null;
        }

        int lastIndexOfRightBracket = lrcLine.lastIndexOf("]");
        String content = lrcLine.substring(lastIndexOfRightBracket + 1);

        // System.out.println("lrcLine=" + lrcLine);

        // -03:33.02--00:36.37-
        String times = lrcLine.substring(0, lastIndexOfRightBracket + 1)
                .replace("[", "-").replace("]", "-");
        String[] timesArray = times.split("-");
        List<LyricsRow> lrcRows = new ArrayList<LyricsRow>();
        for (String tem : timesArray) {
            tem = tem.trim();
            if (TextUtils.isEmpty(tem)) {
                continue;
            }

            try {
                LyricsRow lrcRow = new LyricsRow(tem, formatTime(tem), content);
                lrcRows.add(lrcRow);
            } catch (Exception ignored) {
            }
        }
        return lrcRows;
    }

    private static int formatTime(String timeStr) {
        timeStr = timeStr.replace('.', ':');
        String[] times = timeStr.split(":");

        return Integer.parseInt(times[0]) * 60 * 1000
                + Integer.parseInt(times[1]) * 1000 + Integer.parseInt(times[2]);
    }

    public long getTotalTime() {
        return totalTime;
    }

    public void setTotalTime(int totalTime) {
        this.totalTime = totalTime;
    }

    public String getTimeInfo() {
        return timeInfo;
    }

    public void setTimeInfo(String timeStr) {
        this.timeInfo = timeStr;
    }

    public int getTime() {
        return time;
    }

    public void setTime(int time) {
        this.time = time;
    }

    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }

    @Override
    public int compareTo(LyricsRow anotherLrcRow) {
        return (int) (this.time - anotherLrcRow.time);
    }

    @NonNull
    @Override
    public String toString() {
        return "LrcRow [timeStr=" + timeInfo + ", time=" + time + ", content=" + content + "]";
    }
}
