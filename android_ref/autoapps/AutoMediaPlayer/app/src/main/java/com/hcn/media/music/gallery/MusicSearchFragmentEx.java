package com.hcn.media.music.gallery;

import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.os.Bundle;
import android.text.Editable;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.style.ForegroundColorSpan;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media.R3;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_data.storage.IStorageDevice;
import com.hcn.media_theme.ThemeEx;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.skinx.extend.SkinExBaseAdapter;
import com.hcn.skinx_night.NightModeManager;

import java.util.ArrayList;
import java.util.List;

/**
 * 音乐查找页面（mcc154）
 * @author 65821
 */
public class MusicSearchFragmentEx extends MediaFragment
        implements OnItemClickListener, OnScrollListener {
    public final static String FRAGMENT_NAME = "music-search-mcc154";
    private static final String TAG = MusicSearchFragmentEx.class.getSimpleName();

    private boolean mCreateView = false;
    private boolean mInitView = false;

    private String mInputText = "";
    private EditText mInputTextView = null;
    private TextView mTvStorageType = null;

    private AbsListView mMusicGridView = null;
    private SearchInfoListAdapter mSearchInfoListAdapter = null;

    public MusicSearchFragmentEx() {
        super(FRAGMENT_NAME);
    }

    @Override
    public void initFragment() {
        if (!mCreateView) {
            return;
        }

        if (mAppData.mSelectedDevice != null) {
            changeStorageInfo(mAppData.mSelectedDevice.storageType());
            mSearchInfoListAdapter.updateInfoList(mAppData.mSelectedDevice.mMusicInfoList);
        }

        if (mInputTextView != null) {
            mInputText = "";
            mInputTextView.setText(mInputText);
        }
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_MUSIC_STORAGE:
                if (mAppData.mSelectedDevice != null) {
                    changeStorageInfo(mAppData.mSelectedDevice.storageType());
                    mSearchInfoListAdapter.updateInfoList(mAppData.mSelectedDevice.mMusicInfoList);
                }
                break;
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
            default:
                break;
        }
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fragment_search;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);

        assert view != null;
        initView(view);
        mCreateView = true;
        initFragment();
        mInitView = true;
        return view;
    }

    private void initView(View layout) {
        mMusicGridView = layout.findViewById(xId(R.id.gridview_music));
        mSearchInfoListAdapter = new SearchInfoListAdapter(mContext, null);
        mMusicGridView.setAdapter(mSearchInfoListAdapter);
        mMusicGridView.setOnItemClickListener(this);
        mMusicGridView.setOnScrollListener(this);

        mTvStorageType = layout.findViewById(xId(R.id.tvStorageType));
        mInputTextView = layout.findViewById(xId(R.id.etSearchText));
        mInputTextView.setText(mInputText);
        mInputTextView.addTextChangedListener(new TextWatcher() {

            @Override
            public void afterTextChanged(Editable s) {
                // TODO Auto-generated method stub
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                // TODO Auto-generated method stub
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                mInputText = mInputTextView.getText().toString();
                if (mInputText.length() > 0) {
                    mSearchInfoListAdapter.updateInfoList(search(mInputText));
                } else {
                    mSearchInfoListAdapter.updateInfoList(mAppData.mSelectedDevice.mMusicInfoList);
                }

                mMusicGridView.setSelection(0);
            }
        });
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // TODO: 预留接口
    }

    private void changeStorageInfo(int nStorageType) {
        switch (nStorageType) {
            case IStorageDevice.STORAGE_TYPE_FLASH:
                mTvStorageType.setText(getText(R3.string.storage_flash_label));
                break;
            case IStorageDevice.STORAGE_TYPE_SDCARD:
                mTvStorageType.setText(getText(R3.string.storage_sdcard_label));
                break;
            case IStorageDevice.STORAGE_TYPE_USB:
                mTvStorageType.setText(getText(R3.string.storage_usb1_label));
                break;
            default:
                break;
        }
    }

    private List<MusicInfo> search(String str) {
        List<MusicInfo> filterList = new ArrayList<MusicInfo>();
        String simpleStr = str.replaceAll("\\-|\\s", "");
        mInputText = simpleStr;
        for (MusicInfo info : mAppData.mSelectedDevice.mMusicInfoList) {
            if (!TextUtils.isEmpty(info.mFileName)) {
                if (info.mFileName.contains(simpleStr)) {
                    filterList.add(info);
                }
            }
        }
        return filterList;
    }

    private void onHideInputMethod() {
        InputMethodManager imm = (InputMethodManager) mContext.getSystemService(
                Context.INPUT_METHOD_SERVICE);
        if (null != imm) {
            imm.hideSoftInputFromWindow(mInputTextView.getWindowToken(), 0);
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        onHideInputMethod();

        if (mAppData.mSelectedDevice != null) {
            List<MusicInfo> infoList = mAppData.mSelectedDevice.mMusicInfoList;
            mAppData.mCurrentDevice = mAppData.mSelectedDevice;
            mAppData.mMusicPlayIndex =
                    ((MusicInfo) mSearchInfoListAdapter
                            .getItem(position)).mIndex;
            for (int index = 0; index < infoList.size(); ++index) {
                MusicInfo info = infoList.get(index);
                if (mAppData.mMusicPlayIndex == info.mIndex) {
                    // 确定更新播放列表前，我们可以强制更新播放位置信息
                    mAppData.updateMusicPlayPosition(index, false);
                    break;
                }
            }

            mMusicViewModel.playerRelay().accept(
                    t -> t.requestPlayTarget(
                            IPlaylistType.DEVICE_LIST,
                            infoList,
                            mAppData.musicPlayPosition()));
        }
    }

    @Override
    public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,
            int totalItemCount) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onScrollStateChanged(AbsListView view, int scrollState) {
        if (scrollState == OnScrollListener.SCROLL_STATE_IDLE) {
            mSearchInfoListAdapter.setScrollState(false);
            mSearchInfoListAdapter.notifyDataSetChanged();
        } else {
            mSearchInfoListAdapter.setScrollState(true);
        }
    }

    private static class ViewHolder {
        public TextView mSongTitle = null;
        public TextView mSongArtist = null;
        public ImageView mMusicIcon = null;
        public ImageView mPlayingIcon = null;
    }

    private class SearchInfoListAdapter extends SkinExBaseAdapter {
        private List<MusicInfo> mInfoList = null;
        private boolean mIsScroll = false;

        public SearchInfoListAdapter(Context context, List<MusicInfo> infoList) {
            super(context);
            updateInfoList(infoList);
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

        private void updateItem(ViewHolder viewHolder, MusicInfo info, boolean bIsPlay) {
            if (viewHolder != null && info != null) {
                int index = info.mFileName.indexOf(mInputText);
                SpannableStringBuilder style2 = new SpannableStringBuilder(info.mFileName);
                if (index >= 0) {
                    style2.setSpan(new ForegroundColorSpan(0xFF0000FF), index,
                            index + mInputText.length(),
                            Spannable.SPAN_EXCLUSIVE_INCLUSIVE);
                }

                viewHolder.mSongTitle.setText(style2);
                viewHolder.mSongArtist.setText(info.mArtist);

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

                int defaultImageResId = xDrawableId2(R.drawable.icon_list_song);
                viewHolder.mMusicIcon.setTag(info.mFilePath);
                BitmapCache.getInstance().loadNativeImage(info.mFilePath,
                        viewHolder.mMusicIcon, defaultImageResId, !mIsScroll);
            }
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
                // 使用专用查找项布局资源
                if (ThemeEx.useSearchListItemLayout()) {
                    return R.layout.music_search_list_item;
                }

                // 使用音乐列表页面列表项布局资源
                return R.layout.item_music_list;
            }

            return super.getLayoutRes(itemViewType);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder viewHolder;

            if (convertView == null) {
                int itemType = ItemViewType.MEDIA_LIST_ITEM;
                convertView = inflateItemView(itemType, parent, false);

                viewHolder = new ViewHolder();
                viewHolder.mSongTitle = convertView.findViewById(xId(itemType, R.id.tvSongTitle));
                viewHolder.mSongArtist = convertView.findViewById(xId(itemType, R.id.tvSongArtist));
                viewHolder.mMusicIcon = convertView.findViewById(xId(itemType, R.id.ivMusicIcon));
                viewHolder.mPlayingIcon = convertView.findViewById(xId(itemType, R.id.ivPlayingIcon));

                convertView.setTag(viewHolder);
            } else {
                viewHolder = (ViewHolder) convertView.getTag();
            }

            if (position < mInfoList.size()) {
                MusicInfo info = mInfoList.get(position);
                updateItem(viewHolder, info, false);
            }

            NightModeManager.updateViewWithHelper(this, convertView);
            return convertView;
        }
    }

    @Override
    public void onUpdateUiModeView(boolean isNightMode) {
        super.onUpdateUiModeView(isNightMode);

        // 刷新资源
        mSearchInfoListAdapter.notifyDataSetChanged();
    }
}
