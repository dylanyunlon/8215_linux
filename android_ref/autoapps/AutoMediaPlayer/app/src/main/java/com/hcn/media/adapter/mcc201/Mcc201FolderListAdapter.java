package com.hcn.media.adapter.mcc201;

import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media_data.folder.MusicFilesInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.IMediaEventListener;
import com.hcn.media_data.AppGlobalData;

import java.util.ArrayList;
import java.util.List;

/**
 * @author 65821
 */
public class Mcc201FolderListAdapter extends BaseAdapter {

    private Context mContext;
    private AppGlobalData mAppData;
    private IMediaEventListener mListener;

    private LayoutInflater mInflater;
    private List<MusicFilesInfo> mInfoList;
    private AbsListView mGridView;

    private String path;
    private long mSelectIndex = -2;

    public Mcc201FolderListAdapter(Context context, AbsListView gridView) {
        mAppData = AppGlobalData.getInstance();
        mInflater = LayoutInflater.from(context);
        mInfoList = new ArrayList<>();
        mGridView = gridView;
        this.mContext = context;
    }

    public void updateInfoList(List<MusicFilesInfo> infoList) {
        if (mInfoList == null) {
            mInfoList = new ArrayList<>();
        }

        mInfoList.clear();
        if (infoList != null) {
            for (MusicFilesInfo info : infoList) {
                mInfoList.add(info);
            }
        }
        notifyDataSetChanged();
    }

    public void setMediaEventListener(IMediaEventListener listener) {
        this.mListener = listener;
    }

    public void updatePlayIndex(String path) {
        this.path = path;
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

    private void updateItem(ViewHolder viewHolder, MusicFilesInfo info, boolean bIsPlay) {
        if (viewHolder != null && info != null) {
            if (bIsPlay) {
                if (mAppData.isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                    AnimationDrawable animator =
                            (AnimationDrawable) viewHolder
                                    .mPlayingIcon.getBackground();
                    animator.start();
                } else {
                    AnimationDrawable animator =
                            (AnimationDrawable) viewHolder
                                    .mPlayingIcon.getBackground();
                    animator.stop();
                }
                viewHolder.mPlayingIcon.setVisibility(View.VISIBLE);
            } else {
                viewHolder.mPlayingIcon.setVisibility(View.INVISIBLE);
            }

        }
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        final ViewHolder viewHolder;
        if (convertView == null) {
            convertView = mInflater.inflate(R.layout.mcc201_folder_item, parent, false);
            viewHolder = new ViewHolder();
            viewHolder.tv_path_name = convertView.findViewById(R.id.tv_path_name);
            viewHolder.mPlayingIcon = convertView.findViewById(R.id.ivPlayingIcon);

            convertView.setTag(viewHolder);

        } else {
            viewHolder = (ViewHolder) convertView.getTag();
        }

        if (position < mInfoList.size()) {
            MusicFilesInfo info = mInfoList.get(position);
            viewHolder.tv_path_name.setText(info.mPathName);
            updateSelectedItem(viewHolder, mSelectIndex == position);
        }
        return convertView;
    }

    private void updateSelectedItem(ViewHolder viewHolder, boolean isSelect) {
        if (viewHolder != null) {
            if (isSelect) {
                viewHolder.tv_path_name.setBackgroundResource(R.drawable.mcc201_folder_item_p);
                viewHolder.tv_path_name.setTextColor(
                        mContext.getResources().getColor(R.color.mcc201_button_text_color_checked));
            } else {
                viewHolder.tv_path_name.setBackgroundResource(R.drawable.mcc201_folder_item_n);
                viewHolder.tv_path_name.setTextColor(mContext.getResources().getColor(
                        R.color.mcc201_button_text_color_unchecked));
            }
        }
    }

    private static class ViewHolder {
        public TextView tv_path_name = null;
        public ImageView mPlayingIcon = null;
        public ImageButton btn_delete = null;
    }

}
