package com.hcn.media.adapter.mcc201;

import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.IMediaEventListener;
import com.hcn.media_data.AppGlobalData;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * @author 65821
 */
public class Mcc201MusicListAdapter extends BaseAdapter {

    private boolean mIsScroll = false;
    private long mIndex = -1;
    private long mSelectIndex = -2;

    private LayoutInflater mInflater;
    private List<MusicInfo> mInfoList;
    private AbsListView mGridView;

    private AppGlobalData mAppData;
    private IMediaEventListener mListener;

    public Mcc201MusicListAdapter(Context context, AbsListView gridView) {
        mAppData = AppGlobalData.getInstance();
        mInflater = LayoutInflater.from(context);

        mInfoList = new ArrayList<>();
        mGridView = gridView;
    }

    public void updateInfoList(List<MusicInfo> infoList) {
        if (mInfoList == null) {
            mInfoList = new ArrayList<>();
        }

        mInfoList.clear();
        if (infoList != null) {
            for (MusicInfo info : infoList) {
                mInfoList.add(info);
            }
        }

        notifyDataSetChanged();
    }

    public void setMediaEventListener(IMediaEventListener listener) {
        this.mListener = listener;
    }

    public void setScrollState(boolean isScroll) {
        mIsScroll = isScroll;
    }

    public void updatePlayIndex(long index) {
        mIndex = index;
        notifyDataSetChanged();
    }

    public void updateSelectIndex(long index) {
        mSelectIndex = index;
        notifyDataSetChanged();
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

    private void updateItem(ViewHolder viewHolder, MusicInfo info, boolean bIsPlay) {
        if (viewHolder != null && info != null) {
            viewHolder.mSongTitle.setText(info.mFileName);
            viewHolder.mSongArtist.setText(info.mArtist);
            viewHolder.mFavoriteIcon.setImageResource(info.mFavorite ?
                    R.drawable.mcc201_icon_favor_p : R.drawable.mcc201_icon_favor_n);

            if (bIsPlay) {
                AnimationDrawable animator =
                        (AnimationDrawable) viewHolder.mPlayingIcon.getBackground();

                if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                    animator.start();
                } else {
                    animator.stop();
                }

                viewHolder.mPlayingIcon.setVisibility(View.VISIBLE);
            } else {
                viewHolder.mPlayingIcon.setVisibility(View.INVISIBLE);
            }

            int defaultImageResId = R.drawable.icon_list_song;
            viewHolder.mMusicIcon.setTag(info.mFilePath);
            if (info.mID3Type == MusicInfo.ID3_TYPE_ERROR
                    || info.mID3Type == MusicInfo.ID3_TYPE_NONE) {
                viewHolder.mMusicIcon.setImageResource(defaultImageResId);
            } else {
                BitmapCache.getInstance().loadNativeImage(info.mFilePath,
                        viewHolder.mMusicIcon, mGridView, defaultImageResId, !mIsScroll);
            }
        }
    }

    private void updateSelectedItem(ViewHolder viewHolder, MusicInfo info, boolean isSelect) {
        if (viewHolder != null && info != null) {
            viewHolder.mSongTitle.setText(info.mFileName);
            viewHolder.mSongArtist.setText(info.mArtist);
            viewHolder.mFavoriteIcon.setVisibility(View.GONE);
            // viewHolder.mFavoriteIcon.setImageResource(info.mFavorite ?
            //        R.drawable.mcc201_icon_favor_p : R.drawable.mcc201_icon_favor_n);

            if (isSelect) {
                viewHolder.mLinearLayout.setBackgroundResource(R.drawable.mcc201_list_item_p);
            } else {
                viewHolder.mLinearLayout.setBackgroundResource(R.drawable.mcc201_list_item_n);
            }
        }
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        final ViewHolder viewHolder;

        if (convertView == null) {
            convertView = mInflater.inflate(R.layout.mcc201_item_music_list, parent, false);

            viewHolder = new ViewHolder();
            viewHolder.mSongTitle = (TextView) convertView.findViewById(R.id.tvSongTitle);
            viewHolder.mSongArtist = (TextView) convertView.findViewById(R.id.tvSongArtist);
            viewHolder.mMusicIcon = (ImageView) convertView.findViewById(R.id.ivMusicIcon);
            viewHolder.mFavoriteIcon = (ImageButton) convertView.findViewById(R.id.ivFavoriteIcon);
            viewHolder.mPlayingIcon = (ImageView) convertView.findViewById(R.id.ivPlayingIcon);
            viewHolder.btn_delete = (ImageButton) convertView.findViewById(R.id.btn_delete);
            viewHolder.mLinearLayout = (LinearLayout) convertView.findViewById(R.id.layout_main);

            convertView.setTag(viewHolder);
        } else {
            viewHolder = (ViewHolder) convertView.getTag();
            // viewHolder.mImageView.setImageResource(R.drawable.icon_no_thumbnail);
        }

        viewHolder.btn_delete.setVisibility(View.GONE);
        viewHolder.btn_delete.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                // TODO Auto-generated method stub
                MusicInfo info = mInfoList.get(position);
                mInfoList.remove(position);
                File file = new File(info.mFilePath);
                if (file.exists()) {
                    file.delete();
                }

                notifyDataSetChanged();
            }
        });

        if (position < mInfoList.size()) {
            MusicInfo info = mInfoList.get(position);
            updateItem(viewHolder, info, mIndex == info.mIndex);
        }

        if (position < mInfoList.size()) {
            MusicInfo info = mInfoList.get(position);
            updateSelectedItem(viewHolder, info, mSelectIndex == position);
        }

        return convertView;
    }

    private static class ViewHolder {
        public LinearLayout mLinearLayout = null;
        public TextView mSongTitle = null;
        public TextView mSongArtist = null;
        public ImageView mMusicIcon = null;
        public ImageButton mFavoriteIcon = null;
        public ImageView mPlayingIcon = null;
        public ImageButton btn_delete = null;
    }
}
