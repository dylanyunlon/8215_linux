package com.hcn.media.adapter;


import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.ImageView;
import android.widget.TextView;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.skinx.extend.SkinExBaseAdapter;
import com.hcn.skinx_night.NightModeManager;

import java.util.ArrayList;
import java.util.List;

/**
 * 视频文件列表适配器
 *
 * @author 65821
 */
public class VideoFolderListAdapter extends SkinExBaseAdapter {
    private static final String TAG = VideoFolderListAdapter.class.getSimpleName();

    private List<MusicInfo> mInfoList;

    /**
     * 视图选项的持有者
     * <p> 适配器把列表中的每一个 Item 实例化为一个该对象；
     */
    private static class ViewHolder {
        public TextView mSongNum = null;
        public TextView mSongTitle = null;
        public ImageView mMusicIcon = null;
        public ImageView mPlayingIcon = null;
    }

    public VideoFolderListAdapter(Context context, AbsListView gridView) {
        super(context);
        mInfoList = new ArrayList<>();
    }

    public void setDataList(List<MusicInfo> infoList) {
        if (mInfoList == null) {
            mInfoList = new ArrayList<>();
        }

        mInfoList.clear();
        mInfoList.addAll(infoList);
    }

    @Override
    public int getCount() {
        return mInfoList.size();
    }

    @Override
    public Object getItem(int position) {
        return mInfoList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public int getLayoutRes(int itemViewType) {
        if (itemViewType == ItemViewType.MEDIA_LIST_ITEM) {
            return R.layout.item_folder_list;
        }

        return super.getLayoutRes(itemViewType);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final ViewHolder viewHolder;

        if (convertView == null) {
            int itemType = ItemViewType.MEDIA_LIST_ITEM;
            convertView = inflateItemView(itemType, parent, false);

            viewHolder = new ViewHolder();
            viewHolder.mSongTitle = convertView.findViewById(xId(itemType, R.id.tvSongTitle));
            viewHolder.mMusicIcon = convertView.findViewById(xId(itemType, R.id.ivMusicIcon));
            viewHolder.mPlayingIcon = convertView.findViewById(xId(itemType, R.id.ivPlayingIcon));
            if (viewHolder.mPlayingIcon != null) {
                viewHolder.mPlayingIcon.setVisibility(View.GONE);
            }
            convertView.setTag(viewHolder);
        } else {
            viewHolder = (ViewHolder) convertView.getTag();
        }

        if (position < mInfoList.size()) {
            MusicInfo info = mInfoList.get(position);
            assignToSubview(viewHolder, info);
        }

        if (viewHolder.mSongNum != null) {
            viewHolder.mSongNum.setText(MiscUtils.formatNum(position, mInfoList.size()));
        }

        NightModeManager.updateViewWithHelper(this, convertView);
        return convertView;
    }

    /**
     * 将信息分配给子视图
     *
     * @param viewHolder
     * @param info
     */
    private void assignToSubview(ViewHolder viewHolder, MusicInfo info) {
        if (viewHolder != null && info != null) {
            // 更新标题显示
            viewHolder.mSongTitle.setText(info.mFileName);

            // 更新显示背景与高亮（索引 -1 是文件夹）
            if (info.mIndex == -1) {
                if (viewHolder.mMusicIcon != null) {
                    viewHolder.mMusicIcon.setTag("<unknown>");
                    viewHolder.mMusicIcon.setImageResource(xDrawableId2(R.drawable.btn_file_dark_n));
                }
            } else {
                if (viewHolder.mMusicIcon != null) {
                    int defaultImageResId = xDrawableId2(R.drawable.video_item_bg);
                    viewHolder.mMusicIcon.setTag(info.mFilePath);
                    BitmapCache.getInstance().loadNativeImage(info.mFilePath,
                            viewHolder.mMusicIcon, defaultImageResId, true);
                }
            }
        }
    }
}
