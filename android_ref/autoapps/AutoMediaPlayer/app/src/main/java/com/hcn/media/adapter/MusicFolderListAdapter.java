package com.hcn.media.adapter;

import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.text.TextUtils;
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
import com.hcn.media_theme.StyleX;
import com.hcn.media_theme.ThemeEx;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.skinx.SkinX;
import com.hcn.skinx.extend.SkinExBaseAdapter;
import com.hcn.skinx_night.NightModeManager;

import java.util.ArrayList;
import java.util.List;

/**
 * 音乐文件列表适配器
 *
 * @author 65821
 */
public class MusicFolderListAdapter extends SkinExBaseAdapter {
    private static final String TAG = MusicFolderListAdapter.class.getSimpleName();

    private String mPlayingFilePath = "";
    private List<MusicInfo> mInfoList;

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
        public TextView mSongNum = null;
        public TextView mSongTitle = null;
        public TextView mSongArtist = null;
        public ImageView mMusicIcon = null;
        public ImageView mPlayingIcon = null;

        public View convertView = null;
    }

    public MusicFolderListAdapter(Context context, AbsListView gridView) {
        super(context);
        mInfoList = new ArrayList<>();

        // 读取  MusicFolder 列表选项风格类型
        mListItemStyleType = xInteger("music_folder_item_style_type");
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

            switch (mListItemStyleType) {
                case StyleX.ListItemType.MusicFolderType01:
                    viewHolder.convertView = convertView;
                    viewHolder.mSongNum = convertView.findViewById(xId(itemType, R.id.tvSongNum));
                    viewHolder.mSongArtist = convertView.findViewById(xId(itemType, R.id.tvSongArtist));
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

        if (viewHolder.mSongNum != null) {
            viewHolder.mSongNum.setText(MiscUtils.formatNum(position, mInfoList.size()));
        }

        NightModeManager.updateViewWithHelper(this, convertView);
        return convertView;
    }

    /**
     * 更新显示效果
     *
     * @param viewHolder
     * @param info
     * @param bIsPlay
     */
    private void updateItem(ViewHolder viewHolder, MusicInfo info, boolean bIsPlay) {
        if (viewHolder != null && info != null) {
            // 更新标题显示
            viewHolder.mSongTitle.setText(info.mFileName);

            // 检查更新播放状态
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
            } else {
                viewHolder.mSongTitle.setSelected(false);
                viewHolder.mPlayingIcon.setVisibility(View.INVISIBLE);
            }

            // 更新显示背景与高亮（索引 -1 是文件夹）
            if (info.mIndex == -1) {
                viewHolder.mMusicIcon.setTag("<unknown>");
                viewHolder.mMusicIcon.setImageResource(xDrawableId2(R.drawable.icon_list_file));
                if (SkinX.getBoolean("n91_style_folder_item", false)) {
                    viewHolder.mMusicIcon.setVisibility(View.VISIBLE);
                    viewHolder.mSongNum.setVisibility(View.GONE);
                }
            } else {
                int defaultImageResId = xDrawableId2(R.drawable.icon_list_file);
                viewHolder.mMusicIcon.setTag(info.mFilePath);
                if (SkinX.getBoolean("n91_style_folder_item", false)) {
                    viewHolder.mMusicIcon.setVisibility(View.GONE);
                    viewHolder.mSongNum.setVisibility(View.VISIBLE);
                }
                // 文件列表不显示专辑图片
                // 支持指定的资源显示，不一定非要用默认值（可使用 android:contentDescription 约定）；
                String contentDescription = (String) viewHolder.mMusicIcon.getContentDescription();
                if (TextUtils.isEmpty(contentDescription)) {
                    if (ThemeEx.supportFolderListAlbumCover()) {
                        BitmapCache.getInstance().loadNativeImage(info.mFilePath,
                                viewHolder.mMusicIcon, defaultImageResId, true);
                    } else {
                        viewHolder.mMusicIcon.setImageResource(defaultImageResId);
                    }
                } else {
                    contentDescription += "n";
                    viewHolder.mMusicIcon.setImageDrawable(xDrawable(contentDescription));
                }
            }

            switch (mListItemStyleType) {
                case StyleX.ListItemType.MusicFolderType01:
                    if (viewHolder.mSongNum != null) {
                        viewHolder.mSongNum.setSelected(bIsPlay);
                    }
                    if (viewHolder.mSongArtist != null) {
                        viewHolder.mSongArtist.setText(info.mArtist);
                        viewHolder.mSongArtist.setVisibility(info.mArtist.isEmpty() ? View.GONE : View.VISIBLE);
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
