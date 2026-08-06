package com.hcn.media.adapter;

import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_data.base.MusicKeyInfo;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.skinx.extend.SkinExExpandableListAdapter;
import com.hcn.skinx_night.NightModeManager;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class ExpandableListViewAdapter extends SkinExExpandableListAdapter {

    private boolean mIsScroll = false;
    private String mPlayingFilePath = "";
    private List<String> mKeyInfoList;
    HashMap<String, List<MusicInfo>> mListInfoMap = null;

    private final LayoutInflater mInflater;
    private final AppGlobalData mAppData;
    private final int mDefaultImageResId;

    public ExpandableListViewAdapter(Context context, int defaultImageResId) {
        super(context);

        mAppData = AppGlobalData.getInstance();
        mInflater = LayoutInflater.from(context);

        mKeyInfoList = new ArrayList<>();
        mDefaultImageResId = defaultImageResId;
    }

    public void setDataList(HashMap<String, List<MusicInfo>> infoMap) {
        if (null == mKeyInfoList) {
            mKeyInfoList = new ArrayList<>();
        }

        mKeyInfoList.clear();
        mListInfoMap = infoMap;

        if (infoMap != null) {
            mKeyInfoList.addAll(infoMap.keySet());
        }
    }

    public MusicKeyInfo getItemInfo(int groupPosition) {
        String keyInfo = mKeyInfoList.get(groupPosition);

        if (mListInfoMap != null) {
            List<MusicInfo> list = mListInfoMap.get(keyInfo);
            return new MusicKeyInfo(keyInfo, list);
        }

        return null;
    }

    public void setScrollState(boolean isScroll) {
        mIsScroll = isScroll;
    }

    public void setPlayingFilePath(String path) {
        mPlayingFilePath = path;
    }

    private void updateItem(ViewHolder viewHolder, MusicInfo info, boolean bIsPlay) {
        if (viewHolder != null && info != null) {
            viewHolder.mSongTitle.setText(info.mFileName);
            viewHolder.mSongArtist.setText(info.mArtist);
            viewHolder.mFavoriteIcon.setVisibility(info.mFavorite ? View.VISIBLE : View.INVISIBLE);
            if (bIsPlay) {
                AnimationDrawable animator =
                        (AnimationDrawable) viewHolder.mPlayingIcon.getBackground();
                if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
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

            viewHolder.mMusicIcon.setTag(info.mFilePath);
            int defaultImageResId = xDrawableId2(R.drawable.icon_list_song);
            if (info.mID3Type == MusicInfo.ID3_TYPE_ERROR) {
                viewHolder.mMusicIcon.setImageResource(defaultImageResId);
            } else {
                BitmapCache.getInstance().loadNativeImage(
                        info.mFilePath, viewHolder.mMusicIcon, defaultImageResId, !mIsScroll);
            }
        }
    }

    private void updateItem(ViewHolder2 viewHolder, MusicKeyInfo info) {
        if (viewHolder != null && info != null) {
            viewHolder.tvAlbumName.setText(info.mKey);
            viewHolder.tvAlbumTotal.setText(String.valueOf(info.mInfoList.size()));
            if (info.mInfoList != null) {
                if (info.mInfoList.size() > 0) {
                    String path = info.mInfoList.get(0).mFilePath;
                    viewHolder.ivAlbumIcon.setTag(path);
                    if (info.mInfoList.get(0).mID3Type == 2) {
                        viewHolder.ivAlbumIcon.setImageResource(xDrawableId2(mDefaultImageResId));
                    } else {
                        BitmapCache.getInstance().loadNativeImage(
                                path, viewHolder.ivAlbumIcon, xDrawableId2(mDefaultImageResId), !mIsScroll);
                    }
                }
            }
        }
    }

    @Override
    public Object getChild(int groupPosition, int childPosition) {
        if (groupPosition < mKeyInfoList.size()) {
            String keyInfo = mKeyInfoList.get(groupPosition);

            if (mListInfoMap != null) {
                List<MusicInfo> list = mListInfoMap.get(keyInfo);

                if (list != null) {
                    return list.get(childPosition);
                }
            }
        }

        return null;
    }

    @Override
    public long getChildId(int groupPosition, int childPosition) {
        return childPosition;
    }

    @Override
    public int getChildrenCount(int groupPosition) {
        if (groupPosition < mKeyInfoList.size()) {
            String keyInfo = mKeyInfoList.get(groupPosition);

            if (mListInfoMap != null) {
                List<MusicInfo> list = mListInfoMap.get(keyInfo);

                if (list != null) {
                    return list.size();
                }
            }
        }

        return 0;
    }

    @Override
    public int getLayoutRes(int itemViewType) {
        switch (itemViewType) {
            case ItemViewType.ALBUM_LIST_ITEM:
                return R.layout.item_artist_list;
            case ItemViewType.TITLE_LIST_ITEM:
                return R.layout.item_artist_child_list;
        }

        return super.getLayoutRes(itemViewType);
    }

    @Override
    public View getChildView(int groupPosition, int childPosition, boolean isLastChild,
                             View convertView,
                             ViewGroup parent) {
        final ViewHolder viewHolder;
        if (convertView == null) {
            int itemType = ItemViewType.TITLE_LIST_ITEM;
            convertView = inflateItemView(itemType, parent, false);

            viewHolder = new ViewHolder();
            viewHolder.mSongTitle = convertView.findViewById(xId(itemType, R.id.tvSongTitle));
            viewHolder.mSongArtist = convertView.findViewById(xId(itemType, R.id.tvSongArtist));
            viewHolder.mMusicIcon = convertView.findViewById(xId(itemType, R.id.ivMusicIcon));
            viewHolder.mFavoriteIcon = convertView.findViewById(xId(itemType, R.id.ivFavoriteIcon));
            viewHolder.mPlayingIcon = convertView.findViewById(xId(itemType, R.id.ivPlayingIcon));

            convertView.setTag(viewHolder);
        } else {
            viewHolder = (ViewHolder) convertView.getTag();
        }

        if (groupPosition < mKeyInfoList.size()) {
            String keyInfo = mKeyInfoList.get(groupPosition);

            if (mListInfoMap != null) {
                List<MusicInfo> list = mListInfoMap.get(keyInfo);

                if (mPlayingFilePath != null) {
                    if (childPosition < list.size()) {
                        MusicInfo info = list.get(childPosition);
                        updateItem(viewHolder, info, info.mFilePath.equals(mPlayingFilePath));
                    }
                }
            }
        }

        NightModeManager.updateViewWithHelper(this, convertView);
        return convertView;
    }

    @Override
    public Object getGroup(int groupPosition) {
        return mKeyInfoList.get(groupPosition);
    }

    @Override
    public int getGroupCount() {
        return mKeyInfoList.size();
    }

    @Override
    public long getGroupId(int groupPosition) {
        return groupPosition;
    }

    @Override
    public View getGroupView(int groupPosition, boolean isExpanded, View convertView,
                             ViewGroup parent) {
        final ViewHolder2 viewHolder;
        if (convertView == null) {
            int itemType = ItemViewType.ALBUM_LIST_ITEM;
            convertView = inflateItemView(itemType, parent, false);

            viewHolder = new ViewHolder2();
            viewHolder.tvAlbumName = convertView.findViewById(xId(itemType, R.id.tvAlbumName));
            viewHolder.tvAlbumTotal = convertView.findViewById(xId(itemType, R.id.tvAlbumTotal));
            viewHolder.ivAlbumIcon = convertView.findViewById(xId(itemType, R.id.ivAlbumIcon));
            convertView.setTag(viewHolder);
        } else {
            viewHolder = (ViewHolder2) convertView.getTag();
        }

        if (groupPosition < mKeyInfoList.size()) {
            String keyInfo = mKeyInfoList.get(groupPosition);

            if (mListInfoMap != null) {
                List<MusicInfo> list = mListInfoMap.get(keyInfo);
                updateItem(viewHolder, new MusicKeyInfo(keyInfo, list));
            }
        }

        NightModeManager.updateViewWithHelper(this, convertView);
        return convertView;
    }

    @Override
    public boolean hasStableIds() {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public boolean isChildSelectable(int groupPosition, int childPosition) {
        return true;
    }

    private static class ViewHolder {
        public TextView mSongTitle = null;
        public TextView mSongArtist = null;
        public ImageView mMusicIcon = null;
        public ImageView mFavoriteIcon = null;
        public ImageView mPlayingIcon = null;
    }

    private static class ViewHolder2 {
        public ImageView ivAlbumIcon = null;
        public TextView tvAlbumName = null;
        public TextView tvAlbumTotal = null;
    }
}
