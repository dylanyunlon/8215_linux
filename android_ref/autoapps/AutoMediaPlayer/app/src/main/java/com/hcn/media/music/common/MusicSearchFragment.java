package com.hcn.media.music.common;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.os.Bundle;
import android.text.Editable;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
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
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_data.ListSceneManager;
import com.hcn.media_data.ui.base.PageDataKV;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.skinx.extend.SkinExBaseAdapter;
import com.hcn.media_theme.ThemeEx;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * 音乐列表查找页面
 * @author 65821
 */
@SuppressLint("ValidFragment")
public class MusicSearchFragment extends MediaFragment
        implements OnItemClickListener, OnScrollListener {
    private static final String FRAGMENT_NAME = "music-search";
    private static final String TAG = MusicSearchFragment.class.getSimpleName();

    private boolean mInitView = false;

    private String mInputText = "";
    private EditText mInputTextView = null;

    /**
     * 查找列表视图
     * <pre>
     *    为了共用资源，特定的列表视图可能需要调整元素布局的位置；
     *    这里约定都是通过 android:tag="@string/search_list_special_tag" 来约束；
     *    当然后续可以调整不同的 tag 干不同的事情；
     * </pre>
     */
    private AbsListView mMusicGridView = null;
    private boolean mIsSpecialListView = false;
    private SearchInfoListAdapter mSearchInfoListAdapter = null;

    public MusicSearchFragment() {
        super(FRAGMENT_NAME);
    }

    @Override
    public void initFragment() {
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
            return;
        }

        LogUtil.low_i(TAG, "doCallbackEvent eventId: " + eventId);

        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_MUSIC_STORAGE:
            case IMediaEvent.EVENT_MEDIA_LOADING_START:
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
                updateList();
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
        Log.v(TAG, "onCreateView...");

        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);

        assert view != null;
        initView(view);
        initFragment();
        return view;
    }

    private void initView(View layout) {
        if (mInitView) {
            LogUtil.d(TAG, "It's already initialized!");
            return;
        }

        mInitView = true;

        // 查找显示列表
        mMusicGridView = layout.findViewById(xId(R.id.gridview_music));
        if (mMusicGridView != null) {
            String tag = (String) mMusicGridView.getTag();
            if (!Objects.isNull(tag)) {
                mIsSpecialListView = true;
            }

            mSearchInfoListAdapter = new SearchInfoListAdapter(mContext, mAppData.mSearchList);
            mMusicGridView.setAdapter(mSearchInfoListAdapter);
            mMusicGridView.setOnItemClickListener(this);
            mMusicGridView.setOnScrollListener(this);
        }

        // 查找文本输入框
        mInputTextView = layout.findViewById(xId(R.id.etSearchText));
        if (mInputTextView != null) {
            mInputTextView.setHint(xString(R3.string.text_search_hint));
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
                    if (Objects.isNull(mSearchInfoListAdapter)) {
                        return;
                    }

                    mInputText = mInputTextView.getText().toString();
                    if (mInputText.length() > 0) {
                        mSearchInfoListAdapter.setDataList(search(mInputText));
                    } else {
                        mSearchInfoListAdapter.setDataList(mAppData.mSearchList);
                    }

                    // 更新数据列表
                    updateList();

                    // 从列表第一行开始显示
                    mMusicGridView.setSelection(0);
                }
            });
        }
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        Log.v(TAG, "onHiddenChanged, hidden = " + hidden);

        if (hidden) {
            onHideInputMethod();
        } else {
            updateFragment();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.v(TAG, "onResume...");

        updateFragment();
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // TODO: 预留接口
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.d(TAG, "onPause");

        onHideInputMethod();
    }

    public void updateFragment() {
        if (mInputTextView != null) {
            mInputText = "";
            mInputTextView.setText(mInputText);
        }

        updateList();
    }

    public void updateList() {
        if (mSearchInfoListAdapter != null) {
            mSearchInfoListAdapter.notifyDataSetChanged();
        }
    }

    /**
     * 强制查找数据
     * <p> 效率非常差，数据多的情况下会牺牲性能；
     *
     * @param str 输入字符串
     * @return 匹配的列表
     */
    private List<MusicInfo> search(String str) {
        List<MusicInfo> filterList = new ArrayList<MusicInfo>();
        String simpleStr = str.replaceAll("\\-|\\s", "");
        mInputText = simpleStr;
        for (MusicInfo info : mAppData.mSearchList) {
            if (!TextUtils.isEmpty(info.mFileName)) {
                if (info.mFileName.toLowerCase().contains(simpleStr.toLowerCase())) {
                    filterList.add(info);
                }
            }
        }
        return filterList;
    }

    private void onHideInputMethod() {
        InputMethodManager imm =
                (InputMethodManager) mContext.getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(mInputTextView.getWindowToken(), 0);
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        if (Objects.isNull(mSearchInfoListAdapter)) {
            return;
        }

        onHideInputMethod();

        MusicInfo info = null;
        mAppData.mMusicPlayIndex =
                ((MusicInfo) mSearchInfoListAdapter
                        .getItem(position)).mIndex;
        for (int index = 0; index < mAppData.mSearchList.size(); ++index) {
            info = mAppData.mSearchList.get(index);
            if (mAppData.mMusicPlayIndex == info.mIndex) {
                // 确定更新播放列表前，我们可以强制更新播放位置信息
                mAppData.updateMusicPlayPosition(index, false);
                break;
            }
        }

        if (null != info) {
            mAppData.mSelectedDevice = mAppData.getStorageDeviceFromPath(info.mFilePath);
            mAppData.mCurrentDevice = mAppData.mSelectedDevice;
        }

        // 播放期望目标任务
        mMusicViewModel.playerRelay().accept(
                t -> t.requestPlayTarget(IPlaylistType.DEVICE_LIST,
                        mAppData.mSearchList, mAppData.musicPlayPosition()));
        ListSceneManager.getInstance().saveUserListScene(PageDataKV.ActionSceneValue.FOLDER, ListSceneManager.getInstance().getListScenePath());
    }

    @Override
    public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,
            int totalItemCount) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onScrollStateChanged(AbsListView view, int scrollState) {
        if (Objects.isNull(mSearchInfoListAdapter)) {
            return;
        }

        if (scrollState == OnScrollListener.SCROLL_STATE_IDLE) {
            mSearchInfoListAdapter.setScrollState(false);
            mSearchInfoListAdapter.notifyDataSetChanged();
        } else {
            mSearchInfoListAdapter.setScrollState(true);
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        Log.v(TAG, "onDestroyView...");

        mInitView = false;
        mSearchInfoListAdapter = null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.v(TAG, "onDestroy...");
    }

    /**
     * 视图选项的持有者
     * <p> 适配器把列表中的每一个 Item 实例化为一个该对象；
     */
    private static class ViewHolder {
        public TextView mSongTitle = null;
        public TextView mSongArtist = null;
        public ImageView mMusicIcon = null;
        public ImageView mPlayingIcon = null;
    }

    /**
     * 查找列表适配器
     * <p> 提供并创建当前选择的存储设备的所有数据视图或者查找结果视图元素；
     */
    private class SearchInfoListAdapter extends SkinExBaseAdapter {
        private List<MusicInfo> mInfoList;
        private boolean mIsScroll = false;

        public SearchInfoListAdapter(Context context, List<MusicInfo> infoList) {
            super(context);
            mInfoList = infoList;
        }

        public void setDataList(List<MusicInfo> infoList) {
            mInfoList = infoList;
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

            return convertView;
        }

        private void updateItem(ViewHolder viewHolder, MusicInfo info, boolean bIsPlay) {
            if (viewHolder != null && info != null) {
                int index = info.mFileName.indexOf(mInputText);
                SpannableStringBuilder style2 = new SpannableStringBuilder(info.mFileName);
                if (index >= 0) {
                    style2.setSpan(new ForegroundColorSpan(0xFF0000FF), index,
                            index + mInputText.length(), Spannable.SPAN_EXCLUSIVE_INCLUSIVE);
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

                int defaultImageResId = xId(R.drawable.icon_list_song);
                viewHolder.mMusicIcon.setTag(info.mFilePath);
                BitmapCache.getInstance().loadNativeImage(info.mFilePath,
                        viewHolder.mMusicIcon, mMusicGridView, defaultImageResId, !mIsScroll);
            }
        }
    }
}
