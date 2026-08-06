package com.hcn.media.adapter;

import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.ImageView;
import android.widget.TextView;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_theme.Argument;
import com.hcn.media_theme.StyleX;
import com.hcn.media_theme.ThemeX;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.skinx.extend.SkinExBaseAdapter;
import com.hcn.skinx_night.NightModeManager;

import java.util.ArrayList;
import java.util.List;

/**
 * 音乐列表适配器
 *
 * @author 86158
 */
public class MusicSongListAdapter extends SkinExBaseAdapter {
    private static final String TAG = MusicSongListAdapter.class.getSimpleName();

    private boolean mIsScroll = false;
    private String mPlayingFilePath = "";

    private final AbsListView mGridView;
    private List<MusicInfo> mInfoList;

    /**
     * Smart 按键选择索引
     * <p> 高亮用的标记，选择状态时候显示对应的 UI 风格；
     */
    private int mSelectIndex = -1;

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
        public TextView tvMusicNum = null;
        public TextView mSongTitle = null;
        public TextView mSongArtist = null;
        public ImageView mMusicIcon = null;
        public ImageView mPlayingIcon = null;

        public View convertView = null;
    }

    /**
     * 唯一构造函数
     *
     * @param context
     * @param gridView
     */
    public MusicSongListAdapter(Context context, AbsListView gridView) {
        super(context);

        mInfoList = new ArrayList<>();
        mGridView = gridView;

        // 读取 MusicSong 列表选项风格类型
        mListItemStyleType = xInteger("music_song_item_style_type");
    }

    public void setDataList(List<MusicInfo> infoList) {
        if (mInfoList == null) {
            mInfoList = new ArrayList<>();
        }

        mInfoList.clear();
        mInfoList.addAll(infoList);
    }

    public void setSelectIndex(int index) {
        mSelectIndex = index;
    }

    public void setScrollState(boolean isScroll) {
        mIsScroll = isScroll;
    }

    public void setPlayingFilePath(String path) {
        mPlayingFilePath = path;
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
            return R.layout.item_music_list;
        }

        return super.getLayoutRes(itemViewType);
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        final ViewHolder viewHolder;

        if (null == convertView) {
            int itemType = ItemViewType.MEDIA_LIST_ITEM;
            convertView = inflateItemView(itemType, parent, false);

            // 创建对应视图选项的视图持有者, 方便管理控制选项中的视图元素；
            viewHolder = new ViewHolder();
            viewHolder.mSongTitle = convertView.findViewById(xId(itemType, R.id.tvSongTitle));
            viewHolder.mSongArtist = convertView.findViewById(xId(itemType, R.id.tvSongArtist));
            viewHolder.mMusicIcon = convertView.findViewById(xId(itemType, R.id.ivMusicIcon));
            viewHolder.mPlayingIcon = convertView.findViewById(xId(itemType, R.id.ivPlayingIcon));

            switch (mListItemStyleType) {
                case StyleX.ListItemType.MusicSongType01:
                    viewHolder.convertView = convertView;
                    viewHolder.tvMusicNum = convertView.findViewById(xId(itemType, R.id.tvMusicNum));
                    break;
                case StyleX.ListItemType.None:
                default:
                    break;
            }

            convertView.setTag(viewHolder);
        } else {
            viewHolder = (ViewHolder) convertView.getTag();
        }

        if (position == mSelectIndex) {
            convertView.setBackgroundResource(xDrawableId2(R.drawable.fileframe_p));
        } else {
            convertView.setBackgroundResource(xDrawableId2(R.drawable.gridview_item_bg));
        }

        if (position < mInfoList.size()) {
            MusicInfo info = mInfoList.get(position);
            updateItem(viewHolder, info, info.mFilePath.equals(mPlayingFilePath));
        }

        if (viewHolder.tvMusicNum != null) {
            viewHolder.tvMusicNum.setText(MiscUtils.formatNum(position, mInfoList.size()));
        }

        NightModeManager.updateViewWithHelper(this, convertView);

        return convertView;
    }

    /**
     * 更新指定的视图持有者
     * <p> 播放选项聚焦，Title 颜色、播放标记等
     *
     * @param viewHolder
     * @param info
     * @param bIsPlay
     */
    private void updateItem(ViewHolder viewHolder, MusicInfo info, boolean bIsPlay) {
        if (viewHolder != null && info != null) {
            viewHolder.mSongTitle.setText(info.mFileName);
            viewHolder.mSongArtist.setText(info.mArtist);

            if (bIsPlay) {
                AnimationDrawable animator =
                        (AnimationDrawable) viewHolder.mPlayingIcon.getBackground();
                if (AppGlobalData.getInstance()
                        .isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                    animator.start();
                } else {
                    animator.stop();
                }

                viewHolder.mSongTitle.setSelected(true);
                viewHolder.mPlayingIcon.setVisibility(View.VISIBLE);
                if (Argument.isThemeGod(ThemeX.ET_GOD_405)) {
                    viewHolder.mPlayingIcon.setVisibility(View.INVISIBLE);
                }
            } else {
                viewHolder.mSongTitle.setSelected(false);
                viewHolder.mPlayingIcon.setVisibility(View.INVISIBLE);
            }

            int defaultImageResId = xDrawableId2(R.drawable.icon_list_song);
            viewHolder.mMusicIcon.setTag(info.mFilePath);
            if (info.mID3Type == MusicInfo.ID3_TYPE_ERROR
                    || info.mID3Type == MusicInfo.ID3_TYPE_NONE) {
                viewHolder.mMusicIcon.setImageResource(defaultImageResId);
            } else {
                BitmapCache.getInstance().loadNativeImage(info.mFilePath,
                        viewHolder.mMusicIcon, defaultImageResId, !mIsScroll);
            }

            switch (mListItemStyleType) {
                case StyleX.ListItemType.MusicSongType01:
                    if (viewHolder.tvMusicNum != null) {
                        viewHolder.tvMusicNum.setSelected(bIsPlay);
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
