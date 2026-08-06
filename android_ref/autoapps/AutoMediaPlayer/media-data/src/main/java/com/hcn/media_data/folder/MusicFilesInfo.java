package com.hcn.media_data.folder;

import com.hcn.mediaservice.data.MusicInfo;

import java.util.ArrayList;
import java.util.List;

/**
 * 音乐文件信息
 *
 * @author 65821
 */
public class MusicFilesInfo {
    public int mTotal = 0;
    public String mPathName = "";
    public List<MusicInfo> mListMusicInfo = new ArrayList<>();

    public MusicFilesInfo() {
    }
}
