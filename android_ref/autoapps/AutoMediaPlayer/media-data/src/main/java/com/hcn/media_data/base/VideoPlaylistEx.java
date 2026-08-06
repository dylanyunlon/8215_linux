package com.hcn.media_data.base;

import com.hcn.media_base.constant.IMusicState;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.ArrayList;
import java.util.List;

/**
 * 视频播放列表数据对象扩展
 * <pre>
 *    用来管理视频播放列表相关的数据状态；
 *    视频播放列表暂时只有第一播放列表（后续可以扩展第二播放列表）
 *    第一播放列表：点击存储列表、文件夹、专辑列表等触发播放的列表；
 *    第二播放列表：保留、待扩展...
 * </pre>
 *
 * @author 65821
 */
public class VideoPlaylistEx {
    /**
     * 当前播放列表
     * <pre>
     *    在存储盘符列表点击，就是整个盘符的媒体文件；
     *    在文件夹中点击，就是当前文件夹下媒体文件；
     *    #mRandomPositionList 是用来实现随机播放的，确保随机时每个文件都被播放一次；
     * </pre>
     */
    public List<MusicInfo> mPlaylist = new ArrayList<>();
    public List<Integer> mRandomPositionList = new ArrayList<>();

    /**
     * 以下变量是视频播放相关全局变量
     * <p> 后续可以慢慢消灭掉这里面的部分垃圾对象；
     */
    public int mPosition = 0;
    public int mRepeatMode = IMusicState.REPEAT_MODE_QUEUE;

    /** 循环播放模式 **/
    public int repeatMode() {
        return mRepeatMode;
    }

    /**
     * 设置循环播放模式
     * @param mode {@link IMusicState#REPEAT_MODE_QUEUE ...}
     */
    public void setRepeatMode(int mode) {
        mRepeatMode = mode;
    }

    /**
     * 当前播放列表
     * @return {@link List}
     */
    public List<MusicInfo> playList() {
        return mPlaylist;
    }

    /**
     * 更新播放列表
     * <p> 先清除掉列表数据，再替换期望数据；
     *
     * @param list {@link List}
     */
    public void updatePlaylist(List<MusicInfo> list) {
        mPlaylist.clear();
        mPlaylist.addAll(list);
    }

    /**
     * 当前播放索引
     * @return 索引
     */
    public int playPosition() {
        return mPosition;
    }

    /**
     * 更新当前视频播放位置
     * <pre>
     *    当设置 validityCheck = false 时候，当前函数是非安全的；
     *    建议默认执行有效性检查（为兼容部分历史代码逻辑，我们保留了检查设置参数）
     * </pre>
     *
     * @param position 位置索引
     * @param validityCheck 检查合法性
     */
    public void updatePlayPosition(int position, boolean validityCheck) {
        // 严格模式/是否越界
        if (validityCheck) {
            // 注意：不管数组有没有数据，我们认为 0 是安全的
            if (position > mPlaylist.size() || position < 0) {
                throw new IndexOutOfBoundsException(
                        "Video/Playlist/updatePlayPosition," +
                                " position[" + position + "] out of bounds!");
            }
        }

        mPosition = position;
    }

    /**
     * 更新当前视频播放位置
     * @param position 位置索引
     */
    public void updatePlayPosition(int position) {
        // 注意：不管数组有没有数据，我们认为 0 是安全的
        updatePlayPosition(position, true);
    }

    /**
     * 获取播放列表当前播放位置信息
     * @return {@link MusicInfo}
     */
    public MusicInfo playPositionInfo() {
        return mPlaylist.get(mPosition);
    }

    /**
     * 获取当前播放列表随机位置列表
     * <p> 根据当前播放列表对象总数，随机打乱生成的索引序列；
     * @return {@link List}
     */
    public List<Integer> randomPositionList() {
        return mRandomPositionList;
    }

    /**
     * 更新播放列表对应的随机位置列表
     * <p> 一般是在更新播放列表后，调用它；
     */
    public void updateRandomPositionList() {
        mRandomPositionList.clear();
        if (mRepeatMode == IMusicState.REPEAT_MODE_RANDOM) {
            for (int i = 0; i < mPlaylist.size(); ++i) {
                mRandomPositionList.add(i);
            }
        }
    }

    /**
     * 从随机位置列表中移除目标索引
     * <p> 当一首歌播放完成或上下切曲，需要调用移除动作；
     *
     * @param position 索引；
     */
    public void removeFromRandomPositionList(int position) {
        if (mRepeatMode != IMusicState.REPEAT_MODE_RANDOM) {
            return;
        }

        // 随机位置列表如果是空，则强制更新
        if (mRandomPositionList.isEmpty()) {
            updateRandomPositionList();
        }

        // 存在播放列表信息（随机列表不为空）
        if (!mRandomPositionList.isEmpty()) {
            mRandomPositionList.remove(Integer.valueOf(position));
        }
    }

    /**
     * 获取音乐播放列表随机索引
     * @return 下一个播放信息索引
     */
    public int getNextRandomPosition() {
        if (mRepeatMode != IMusicState.REPEAT_MODE_RANDOM) {
            return 0;
        }

        // 随机位置列表如果是空，则强制更新
        if (mRandomPositionList.isEmpty()) {
            updateRandomPositionList();
        }

        // 存在播放列表信息（随机列表不为空）
        if (!mRandomPositionList.isEmpty()) {
            int pos = (int) (Math.random() * (mRandomPositionList.size() - 1));
            if (pos < mRandomPositionList.size()) {
                return mRandomPositionList.get(pos);
            }
        }

        return 0;
    }

    /**
     * 更新播放位置（相对播放列表）
     * @param isNextSong 是切换下一曲/否则就是切换上一曲
     */
    public void adjustPlayPosition(boolean isNextSong) {
        // 检测当前音乐播放列表是否为空
        if (mPlaylist.isEmpty()) {
            return;
        }

        switch (mRepeatMode) {
            case IMusicState.REPEAT_MODE_QUEUE:
                if (isNextSong) {
                    updatePlayPosition((mPosition + 1) % mPlaylist.size());
                } else {
                    updatePlayPosition(
                            (mPlaylist.size() + mPosition - 1) % mPlaylist.size());
                }
                break;
            case IMusicState.REPEAT_MODE_RANDOM:
                updatePlayPosition(getNextRandomPosition());
                break;
            case IMusicState.REPEAT_MODE_ONE:
            default:
                break;
        }
    }
}
