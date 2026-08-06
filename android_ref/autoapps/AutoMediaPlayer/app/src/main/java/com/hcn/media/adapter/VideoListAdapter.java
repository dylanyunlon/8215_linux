package com.hcn.media.adapter;

import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.ImageView;
import android.widget.TextView;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_theme.StyleX;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.skinx.extend.SkinExBaseAdapter;
import com.hcn.skinx_night.NightModeManager;

import java.util.ArrayList;
import java.util.List;

/**
 * 视频列表适配器
 * @author 65821
 */
public class VideoListAdapter extends SkinExBaseAdapter {

    private AbsListView mGridView;
    private List<MusicInfo> mInfoList;

    private boolean mIsScroll = false;
    private String mPlayingFilePath = "";

    /**
     * 列表布局风格约束
     * <@seee> {@link R.integer#simple_list_item_style_type}
     */
    private final int mListItemStyleType;

    /**
     * 视图选项的持有者
     * <p> 适配器把列表中的每一个 Item 实例化为一个该对象；
     */
    private static class ViewHolder {
        public TextView tvVideoNum = null;
        public ImageView ivVideoIcon = null;
        public TextView ivVideoTitle = null;
        public ImageView ivPlayingIcon = null;
        public View convertView = null;
    }

    public VideoListAdapter(Context context, AbsListView gridView) {
        super(context);

        mInfoList = new ArrayList<>();
        mGridView = gridView;

        // 读取 VideoList 列表选项风格类型
        mListItemStyleType = xInteger("video_list_item_style_type");
    }

    public void setDataList(List<MusicInfo> infoList) {
        if (mInfoList == null) {
            mInfoList = new ArrayList<>();
        }

        mInfoList.clear();
        mInfoList.addAll(infoList);
    }

    public void setPlayingFilePath(String path) {
        mPlayingFilePath = path;
    }

    public void setScrollState(boolean isScroll) {
        mIsScroll = isScroll;
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
            return R.layout.item_video_list;
        }

        return super.getLayoutRes(itemViewType);
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        final ViewHolder viewHolder;

        if (null == convertView) {
            int itemType = ItemViewType.MEDIA_LIST_ITEM;
            convertView = inflateItemView(itemType, parent, false);

            viewHolder = new ViewHolder();
            viewHolder.ivVideoIcon = convertView.findViewById(xId(itemType, R.id.ivVideoIcon));
            viewHolder.ivVideoTitle = convertView.findViewById(xId(itemType, R.id.ivVideoTitle));
            viewHolder.ivPlayingIcon = convertView.findViewById(xId(itemType, R.id.ivPlayingIcon));

            switch (mListItemStyleType) {
                case StyleX.ListItemType.VideoListType01:
                    viewHolder.convertView = convertView;
                    viewHolder.tvVideoNum = convertView.findViewById(xId(itemType, R.id.tvVideoNum));
                    break;
                case StyleX.ListItemType.None:
                default:
                    break;
            }

            convertView.setTag(viewHolder);
        } else {
            viewHolder = (ViewHolder) convertView.getTag();
        }

        if (position < mInfoList.size()) {
            MusicInfo info = mInfoList.get(position);
            updateItem(viewHolder, info, info.mFilePath.equals(mPlayingFilePath));
        }

        if (viewHolder.tvVideoNum != null) {
            viewHolder.tvVideoNum.setText(MiscUtils.formatNum(position, mInfoList.size()));
        }

        NightModeManager.updateViewWithHelper(this, convertView);
        return convertView;
    }

    private void updateItem(ViewHolder viewHolder, MusicInfo info, boolean bIsPlay) {
        if (viewHolder != null && info != null) {
            viewHolder.ivVideoTitle.setText(info.mFileName);
            if (null != viewHolder.ivPlayingIcon) {
                if (bIsPlay) {
                    AnimationDrawable animator =
                            (AnimationDrawable) viewHolder.ivPlayingIcon.getBackground();
                    if (AppGlobalData.getInstance()
                            .isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                        animator.start();
                    } else {
                        animator.stop();
                    }

                    viewHolder.ivVideoTitle.setSelected(true);
                    viewHolder.ivPlayingIcon.setVisibility(View.VISIBLE);
                } else {
                    viewHolder.ivVideoTitle.setSelected(false);
                    viewHolder.ivPlayingIcon.setVisibility(View.INVISIBLE);
                }
            }

            if (viewHolder.ivVideoTitle != null) {
                viewHolder.ivVideoTitle.setSelected(bIsPlay);
            }
            if (viewHolder.ivVideoIcon != null) {
                viewHolder.ivVideoIcon.setSelected(bIsPlay);
            }

            int defaultImageResId = xDrawableId2(R.drawable.video_item_bg);
            viewHolder.ivVideoIcon.setTag(info.mFilePath);
            BitmapCache.getInstance().loadVideoInfoImage(info.mFilePath,
                    viewHolder.ivVideoIcon, mGridView, defaultImageResId, !mIsScroll);

            switch (mListItemStyleType) {
                case StyleX.ListItemType.VideoListType01:
                    if (viewHolder.tvVideoNum != null) {
                        viewHolder.tvVideoNum.setSelected(bIsPlay);
                    }
                    if (viewHolder.convertView != null) {
                        viewHolder.convertView.setBackgroundResource(
                                bIsPlay ? xDrawableId2(R.drawable.fileframe_p) : xDrawableId2(R.drawable.gridview_item_bg));
                    }
                    break;
                case StyleX.ListItemType.None:
                default:
                    break;
            }
        }
    }
}
