package com.hcn.media_data;

/**
 * 外部触发播放
 * <pre>
 *    可能是语音触发播放信息；
 *    e.g. 播放刘德华的忘情水；
 * </pre>
 *
 * @author 65821
 */
public class MusicRegInfo {
    public boolean mEnable = false;

    public String mTitle = null;
    public String mArtist = null;
    public String mAlbum = null;

    public MusicRegInfo() {
    }
}
