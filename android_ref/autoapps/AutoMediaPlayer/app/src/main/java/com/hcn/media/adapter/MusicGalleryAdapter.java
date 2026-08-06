package com.hcn.media.adapter;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Handler;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Gallery;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;
import android.widget.TextView;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_common.cache.BitmapCache.NativeImageCallBack;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.utils.BitmapUtil;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.skinx.extend.SkinExBaseAdapter;
import com.hcn.skinx_night.NightModeManager;

import java.util.ArrayList;
import java.util.List;

/**
 * 封面流适配器
 * <pre>
 *    GalleryFlow
 *    这个 Adapter 本身效率其实很低（有待优化）；
 * </pre>
 *
 * @author 86158
 */
public class MusicGalleryAdapter extends SkinExBaseAdapter implements NativeImageCallBack {
    private static final String TAG = "MusicGalleryAdapter";

    private final Handler mUserHandler;

    private List<MusicInfo> mInfoList;

    private long mIndex = -1;
    private long mSelectIndex = -2;
    private boolean mIsScroll = false;

    private long mCurrentTime = 0;
    private final Runnable mTimeRunnable = new Runnable() {
        @Override
        public void run() {
            mUserHandler.removeCallbacksAndMessages(null);
            notifyDataSetChanged();
        }
    };

    public MusicGalleryAdapter(Context context) {
        super(context);

        mUserHandler = new Handler(context.getMainLooper());
        mInfoList = new ArrayList<>();
    }

    public void updateInfoList(List<MusicInfo> infoList) {
        if (mInfoList == null) {
            mInfoList = new ArrayList<>();
        }

        mInfoList.clear();
        if (infoList != null) {
            mInfoList.addAll(infoList);
        }

        notifyDataSetChanged();
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

    @Override
    public void onImageLoader(Bitmap bitmap, String path) {
        mUserHandler.removeCallbacksAndMessages(null);
        mUserHandler.postDelayed(mTimeRunnable, 1000);
    }

    @Override
    public int getLayoutRes(int itemViewType) {
        if (itemViewType == ItemViewType.GALLERY_LIST_ITEM) {
            return R.layout.item_music_gallery;
        }

        return super.getLayoutRes(itemViewType);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final ViewHolder viewHolder;
        View tmpContentView;

        if (convertView == null) {
            convertView = new ImageView(mContext);
            convertView.setLayoutParams(new Gallery.LayoutParams(
                    xInteger(R.integer.gallery_cover_width), xInteger(R.integer.gallery_cover_height)));

            int itemType = ItemViewType.GALLERY_LIST_ITEM;
            tmpContentView = inflateItemView(itemType, parent, false);

            viewHolder = new ViewHolder();
            viewHolder.mLayout = tmpContentView.findViewById(xId(itemType, R.id.layout_music_item));
            viewHolder.mSongTitle = tmpContentView.findViewById(xId(itemType, R.id.tvSongTitle));
            viewHolder.mMusicIcon = tmpContentView.findViewById(xId(itemType, R.id.ivMusicIcon));

            tmpContentView.setTag(viewHolder);
            convertView.setTag(tmpContentView);
        } else {
            tmpContentView = (View) convertView.getTag();
            viewHolder = (ViewHolder) tmpContentView.getTag();
        }

        mCurrentTime = System.currentTimeMillis();
        ImageView imageView = (ImageView) convertView;
        if (position < mInfoList.size()) {
            MusicInfo info = mInfoList.get(position);
            updateItem(viewHolder, info, mIndex == info.mIndex, position);
        }

        try {
            Bitmap bitmap = BitmapUtil.getViewBitmap(tmpContentView, 135, 135);
            Bitmap bitmapArtwork = BitmapUtil.createReflectedBitmap(bitmap);

            imageView.setScaleType(ScaleType.FIT_XY);
            imageView.setImageBitmap(bitmapArtwork);
        } catch (Exception ex) {
            ex.printStackTrace();
        }

        NightModeManager.updateViewWithHelper(this, convertView);
        return imageView;
    }

    private void updateItem(ViewHolder viewHolder, MusicInfo info, boolean bIsPlay, int position) {
        if (viewHolder != null && info != null) {
            viewHolder.mSongTitle.setText(info.mFileName);
            LogUtil.v(TAG, "info.mFileName: " + info.mFileName);

            if (bIsPlay) {
                viewHolder.mLayout.setBackgroundResource(xDrawableId2(R.drawable.album_bg_p));
            } else if (mSelectIndex >= 0 && info.mIndex == mSelectIndex) {
                viewHolder.mLayout.setBackgroundResource(xDrawableId2(R.drawable.album_bg_s));
            } else {
                viewHolder.mLayout.setBackgroundResource(xDrawableId2(R.drawable.album_bg_n));
            }

            int defaultImageResId = xDrawableId2(R.drawable.default_thumbnails_bg);
            if (info.mID3Type == MusicInfo.ID3_TYPE_ERROR
                    || info.mID3Type == MusicInfo.ID3_TYPE_NONE) {
                viewHolder.mMusicIcon.setImageResource(defaultImageResId);
            } else {
                BitmapCache.getInstance().loadNativeImage(
                        info, viewHolder.mMusicIcon, this, defaultImageResId, !mIsScroll);
            }
        }
    }

    private static class ViewHolder {
        public View mLayout = null;
        public TextView mSongTitle = null;
        public ImageView mMusicIcon = null;
    }
}
