package com.hcn.media.video.common;

import android.content.Context;
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

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.auto_compat.PlatformUtils;
import com.hcn.media.R3;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_data.storage.IStorageDevice;
import com.hcn.media_theme.ThemeEx;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.skinx.extend.SkinExBaseAdapter;
import com.hcn.skinx_night.NightModeManager;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * 视频查找页面
 *
 * @author 86158
 */
public class VideoSearchFragment extends MediaFragment
        implements OnItemClickListener, OnScrollListener {
    private final static String FRAGMENT_NAME = "video-search";
    private static final String TAG = VideoSearchFragment.class.getSimpleName();

    private boolean mInitView = false;
    private View mVideoSearchLayout = null;

    private String mInputText = "";
    private EditText mInputTextView = null;
    private TextView mTvStorageType = null;

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

    public VideoSearchFragment() {
        super(FRAGMENT_NAME);
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        LogUtil.v(TAG, "onCreate.");
    }

    @Override
    public int getLayoutRes() {
        if (ThemeEx.useVideoListExpandLayout()) {
            return requestUiModel().videoShowInBottomHalfScreen() ?
                    R.layout.fragment_videosearch_expand : R.layout.fragment_videosearch;
        }

        return R.layout.fragment_videosearch;
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        LogUtil.v(TAG, "onCreateView.");

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
        mVideoSearchLayout = layout.findViewById(xId(R.id.llVideoSearch));

        // 查找显示列表
        mMusicGridView = layout.findViewById(xId(R.id.gridview_music));
        if (mMusicGridView != null) {
            String tag = (String) mMusicGridView.getTag();
            if (!Objects.isNull(tag)) {
                mIsSpecialListView = true;
            }

            mSearchInfoListAdapter = new SearchInfoListAdapter(mContext, null);
            mMusicGridView.setAdapter(mSearchInfoListAdapter);
            mMusicGridView.setOnItemClickListener(this);
            mMusicGridView.setOnScrollListener(this);
        }

        // 承储类型标识
        mTvStorageType = layout.findViewById(xId(R.id.tvStorageType));

        // 查找输入文本框
        mInputTextView = layout.findViewById(xId(R.id.etSearchText));
        if (mInputTextView != null) {
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
                    if (Objects.isNull(mSearchInfoListAdapter)) {
                        return;
                    }

                    mInputText = mInputTextView.getText().toString();
                    if (mInputText.length() > 0) {
                        mSearchInfoListAdapter.updateInfoList(search(mInputText));
                    } else {
                        mSearchInfoListAdapter.updateInfoList(mAppData.mSelectedDevice.mVideoInfoList);
                    }

                    mMusicGridView.setSelection(0);
                }
            });
        }

        adjustLayoutByStatusBar();
    }

    /**
     * 依据状态栏调整局部参数
     * <pre>
     *    不同的主题需要调整的位置不一样；
     *    这里主要调整需要配置 android:paddingTop="@*android:dimen/status_bar_height" 的元素；
     *    原因，AndroidStudio 编译的 apk 运行时系统不认识这个常量；
     * </pre>
     */
    private void adjustLayoutByStatusBar() {
        if (Objects.isNull(mVideoSearchLayout)) {
            return;
        }

        // 如果视频显示在分屏的下半部分
        if (requestUiModel().videoShowInBottomHalfScreen()) {
            // 如果没有使用扩展布局，不需要预留状态栏高度
            if (!ThemeEx.useVideoListExpandLayout()) {
                mVideoSearchLayout.setPadding(0, 0, 0, 0);
            }

            return;
        }

        // 使用了显示过扫描配置，不需要预留状态栏高度
        if (PlatformUtils.isDisplayOverscanning()) {
            return;
        }

        // 调整状态栏高度，否则列表页面显示会和状态栏重叠（视频是全屏窗口模式）；
        int statusBarHeight = MiscUtils.statusBarHeight(mContext, R.dimen.status_bar_height);
        mVideoSearchLayout.setPadding(0, statusBarHeight, 0, 0);
    }

    @Override
    public void initFragment() {
        if (mAppData.mSelectedDevice != null) {
            changeStorageInfo(mAppData.mSelectedDevice.storageType());
            mSearchInfoListAdapter.updateInfoList(mAppData.mSelectedDevice.mVideoInfoList);
        }

        if (mInputTextView != null) {
            mInputText = "";
            mInputTextView.setText(mInputText);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // TODO: 预留接口
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
                    mSearchInfoListAdapter.updateInfoList(mAppData.mSelectedDevice.mVideoInfoList);
                }
                break;

            case IMediaEvent.EVENT_CHANGE_VIDEO_ITEM:
            case IMediaEvent.EVENT_CHANGE_VIDEO_LIST:
            default:
                break;
        }
    }

    /**
     * 显示当前承储类型字符描述信息
     * <p> 鉴于扩展皮肤包不可能包含所有多国语言的问题，文本需要特殊处理；
     *
     * @param nStorageType 承储类型
     */
    private void changeStorageInfo(int nStorageType) {
        switch (nStorageType) {
            case IStorageDevice.STORAGE_TYPE_FLASH:
                mTvStorageType.setText(getString(
                        R3.string.storage_flash_label));
                break;
            case IStorageDevice.STORAGE_TYPE_SDCARD:
                mTvStorageType.setText(getString(
                        R3.string.storage_sdcard_label));
                break;
            case IStorageDevice.STORAGE_TYPE_USB:
                mTvStorageType.setText(getString(
                        R3.string.storage_usb1_label));
                break;
            default:
                break;
        }
    }

    /**
     * 查找接口
     * <p> 缺点: 效率低下;
     *
     * @param str 目标字符串
     * @return 匹配的列表
     */
    private List<MusicInfo> search(String str) {
        List<MusicInfo> filterList = new ArrayList<MusicInfo>();
        String simpleStr = str.replaceAll("\\-|\\s", "");
        mInputText = simpleStr;
        for (MusicInfo info : mAppData.mSelectedDevice.mVideoInfoList) {
            if (!TextUtils.isEmpty(info.mFileName)) {
                if (info.mFileName.contains(simpleStr)) {
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

    /**
     * 点击调转播放
     *
     * @param parent
     * @param view
     * @param position
     * @param id
     */
    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        onHideInputMethod();

        if (mAppData.mSelectedDevice != null) {
            mAppData.mCurrentDevice = mAppData.mSelectedDevice;
            mAppData.mVideoPlayIndex =
                    ((MusicInfo) mSearchInfoListAdapter
                            .getItem(position)).mIndex;

            List<MusicInfo> infoList = mAppData.mSelectedDevice.mVideoInfoList;
            for (int index = 0; index < infoList.size(); ++index) {
                MusicInfo info = infoList.get(index);
                if (mAppData.mVideoPlayIndex == info.mIndex) {
                    // 确定更新播放列表前，我们可以强制更新播放位置信息
                    mAppData.updateVideoPlayPosition(index, false);
                    break;
                }
            }

            // 请求播放指定列表位置歌曲对象
            mVideoViewModel.playerRelay().accept(
                    t -> t.requestPlayTarget(
                            IPlaylistType.DEVICE_LIST,
                            infoList,
                            mAppData.videoPlayPosition()));
        }
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
        LogUtil.v(TAG, "onDestroyView.");

        mInitView = false;
        mSearchInfoListAdapter = null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        LogUtil.v(TAG, "onDestroy.");
    }

    @Override
    public void onUpdateUiModeView(boolean isNightMode) {
        super.onUpdateUiModeView(isNightMode);

        // 刷新资源
        mSearchInfoListAdapter.notifyDataSetChanged();
    }

    /**
     * 查找列表 Item 视图持有者
     * <p> 管理查找列表中的 ItemView 选项；
     */
    private static class ViewHolder {
        public TextView ivVideoTitle = null;
        public ImageView ivVideoIcon = null;
        public ImageView ivPlayingIcon = null;
    }

    /**
     * 查找列表的适配器
     * <p> 提供并创建当前选择的存储设备的所有数据视图或者查找结果视图元素；
     */
    private class SearchInfoListAdapter extends SkinExBaseAdapter {
        private List<MusicInfo> mInfoList = null;
        private boolean mIsScroll = false;

        public SearchInfoListAdapter(Context context, List<MusicInfo> infoList) {
            super(context);

            updateInfoList(infoList);
        }

        /**
         * 更新列表数据信息
         *
         * @param infoList 列表数据
         */
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
            if (itemViewType == SkinExBaseAdapter.ItemViewType.MEDIA_LIST_ITEM) {
                // 使用专用查找项布局资源
                if (ThemeEx.useSearchListItemLayout()) {
                    return R.layout.video_search_list_item;
                }

                // 使用视频列表页面列表项布局资源
                return R.layout.item_video_list;
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
                viewHolder.ivVideoIcon = convertView.findViewById(xId(itemType, R.id.ivVideoIcon));
                viewHolder.ivVideoTitle = convertView.findViewById(xId(itemType, R.id.ivVideoTitle));
                viewHolder.ivPlayingIcon = convertView.findViewById(xId(itemType, R.id.ivPlayingIcon));
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

        private void updateItem(ViewHolder viewHolder, MusicInfo info, boolean bIsPlay) {
            if (viewHolder != null && info != null) {
                int index = info.mFileName.indexOf(mInputText);
                SpannableStringBuilder style2 = new SpannableStringBuilder(info.mFileName);
                if (index >= 0) {
                    style2.setSpan(new ForegroundColorSpan(0xFF0000FF), index,
                            index + mInputText.length(), Spannable.SPAN_EXCLUSIVE_INCLUSIVE);
                }

                viewHolder.ivVideoTitle.setText(style2);
                if (null != viewHolder.ivPlayingIcon) {
                    if (bIsPlay) {
                        viewHolder.ivPlayingIcon.setVisibility(View.VISIBLE);
                    } else {
                        viewHolder.ivPlayingIcon.setVisibility(View.INVISIBLE);
                    }
                }

                int defaultImageResId = xDrawableId2(R.drawable.video_item_bg);
                viewHolder.ivVideoIcon.setTag(info.mFilePath);
                BitmapCache.getInstance().loadVideoInfoImage(info.mFilePath,
                        viewHolder.ivVideoIcon, mMusicGridView, defaultImageResId, !mIsScroll);
            }
        }
    }
}
