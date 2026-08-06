package com.hcn.media.music.mcc201;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.os.Handler;
import android.text.Editable;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.style.ForegroundColorSpan;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media.R3;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_base.impl.MediaEventPostbox;
import com.hcn.media.base.layout.FrameLayoutEx;
import com.hcn.media.vm.action.IPlayerEx;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_base.IMediaEventListener;
import com.hcn.media.music.ILayoutCallback;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * mcc201 音乐查找布局视图
 * <pre>
 *    为特定客户需求创建，后续不再维护；
 *    这类布局不可以在布局资源文件（[@layout].xml）中直接使用；
 * </pre>
 *
 * @author 65821
 */
@SuppressLint("ViewConstructor")
public class Mcc201MusicSearchLayout extends FrameLayoutEx
        implements OnItemClickListener, OnScrollListener,
        OnClickListener, OnTouchListener {
    private static final String TAG = Mcc201MusicSearchLayout.class.getSimpleName();

    private ListView mMusicGridView = null;
    private SearchInfoListAdapter mMusicListAdapter = null;
    private List<MusicInfo> mFilterList = null;
    private TextView mShowTipCtrl = null;

    private Handler mUserHandler = null;

    private ILayoutCallback mLayoutCallback = null;
    private List<MusicInfo> mListMusicInfos = null;
    private EditText mEditSearch = null;
    private String mInputText = "";

    private final Runnable mTimeRunnable = () -> {
        mUserHandler.removeCallbacksAndMessages(null);
        mMusicGridView.setSelection(mAppData.musicPlayPosition());
    };

    public Mcc201MusicSearchLayout(Context context,
                                   IPlayerEx player) {
        this(context, null, player);
    }

    public Mcc201MusicSearchLayout(Context context,
                                   AttributeSet attrs,
                                   IPlayerEx player) {
        this(context, attrs, 0, player);
    }

    public Mcc201MusicSearchLayout(Context context,
                                   AttributeSet attrs,
                                   int defStyle,
                                   IPlayerEx player) {
        super(context, attrs, defStyle, player);

        initView();
    }

    private void initView() {
        LayoutInflater.from(mContext).inflate(R.layout.mcc201_layout_searchmusic, this, true);

        mUserHandler = new Handler(mContext.getMainLooper());

        mEditSearch = findViewById(R.id.etSearchText);
        mShowTipCtrl = (TextView) findViewById(R.id.tvShowTip);
        mMusicGridView = (ListView) findViewById(R.id.gridview_music);
        mMusicListAdapter = new SearchInfoListAdapter(mContext, null);
        mMusicGridView.setAdapter(mMusicListAdapter);
        mMusicGridView.setOnItemClickListener(this);
        mMusicGridView.setOnScrollListener(this);
        mEditSearch.setText(mInputText);
        mEditSearch.addTextChangedListener(new TextWatcher() {

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
                mInputText = mEditSearch.getText().toString();
                if (mInputText.length() > 0) {
                    mFilterList = search(mInputText);
                    mMusicListAdapter.updateInfoList(search(mInputText));
                } else {
                    mMusicListAdapter.updateInfoList(mAppData.mMusicListFiles);
                }
                mMusicGridView.setSelection(0);
            }
        });
    }

    private List<MusicInfo> search(String str) {
        List<MusicInfo> filterList = new ArrayList<MusicInfo>();
        String simpleStr = str.replaceAll("\\-|\\s", "");
        mInputText = simpleStr;
        for (MusicInfo info : mListMusicInfos) {
            if (!TextUtils.isEmpty(info.mFileName)) {
                if (info.mFileName.contains(simpleStr)) {
                    filterList.add(info);
                }
            }
        }
        return filterList;
    }

    @Override
    public void initLayout() {
        mUserHandler.removeCallbacksAndMessages(null);
        initData();
    }

    private void initData() {
        mListMusicInfos = new ArrayList<>();
        for (int i = 0; i < mAppData.mStorageDeviceList.size(); i++) {
            mListMusicInfos.addAll(mAppData.mStorageDeviceList.get(i).mMusicInfoList);
        }
        mMusicListAdapter.updateInfoList(mListMusicInfos);

        updatePlayIndex();
        updateTipCtrl();
    }

    private void updateTipCtrl() {
        boolean isMusicInfoIsEmpty = mListMusicInfos.isEmpty();
        mShowTipCtrl.setText(getString(R3.string.tip_no_music_file));
        mShowTipCtrl.setVisibility(isMusicInfoIsEmpty ? View.VISIBLE : View.GONE);
    }

    @Override
    public void doCallbackEvent(int eventId) {
        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_PLAY_STATE:
                mMusicListAdapter.notifyDataSetChanged();
                break;
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
                updatePlayIndex();
                break;
            default:
                break;
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        mUserHandler.removeCallbacksAndMessages(null);
        mUserHandler.postDelayed(mTimeRunnable, 10000);
        return false;
    }

    private void updatePlayIndex() {
        mMusicListAdapter.updatePlayIndex(mAppData.mMusicPlayIndex);
        mMusicGridView.setSelection(getListOfPosition());
        mMusicListAdapter.notifyDataSetInvalidated();

    }

    public int getListOfPosition() {
        int size = (mAppData.mSelectedDevice.mMusicInfoList.size());
        for (int i = 0; i < size; i++) {
            if (mAppData.mCurrentMediaInfo != null && mAppData.mCurrentMediaInfo.mIndex
                    == mAppData.mSelectedDevice.mMusicInfoList.get(
                    i).mIndex) {
                return i;
            }
        }
        return 0;
    }

    @Override
    public void setMediaEventListener(MediaEventPostbox listener) {
        super.setMediaEventListener(listener);

        if (null != mMusicListAdapter) {
            mMusicListAdapter.setMediaEventListener(listener);
        }
    }

    @Override
    public void initDataObject() {
        initData();
    }

    private void onHideInputMethod() {
        InputMethodManager imm = (InputMethodManager) mContext.getSystemService(
                Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(mEditSearch.getWindowToken(), 0);
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        onHideInputMethod();

        if (mFilterList == null || mFilterList.isEmpty()) {
            playerRelay().accept(t -> t.requestPlayTarget(
                    IPlaylistType.DEVICE_LIST, mListMusicInfos, position));
        } else {
            playerRelay().accept(t -> t.requestPlayTarget(
                    IPlaylistType.DEVICE_LIST, mFilterList, position));
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
            mMusicListAdapter.setScrollState(false);
            mMusicListAdapter.notifyDataSetChanged();
        } else {
            mMusicListAdapter.setScrollState(true);
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View view) {
        // TODO Auto-generated method stub
        switch (view.getId()) {
            case R.id.btn_refresh:
            case R.id.btn_search:
            default:
                break;
        }
    }

    private static class ViewHolder {
        public TextView mSongTitle = null;
        public TextView mSongArtist = null;
        public ImageView mMusicIcon = null;
        public ImageButton mFavoriteIcon = null;
        public ImageView mPlayingIcon = null;
        public ImageButton btn_delete = null;
    }

    /** 查找列表适配器 **/
    private class SearchInfoListAdapter extends BaseAdapter {
        private long mIndex = -1;
        private LayoutInflater mInflater = null;
        private List<MusicInfo> mInfoList = null;
        private boolean mIsScroll = false;
        private IMediaEventListener mListener = null;

        public SearchInfoListAdapter(Context context, List<MusicInfo> infoList) {
            mInflater = LayoutInflater.from(context);
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

        public void setMediaEventListener(IMediaEventListener mListener) {
            this.mListener = mListener;
        }

        public void setScrollState(boolean isScroll) {
            mIsScroll = isScroll;
        }

        public void updatePlayIndex(long index) {
            mIndex = index;
            notifyDataSetChanged();
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
                viewHolder.mFavoriteIcon.setVisibility(View.GONE);
                viewHolder.mFavoriteIcon
                        .setImageResource(info.mFavorite ? R.drawable.mcc201_icon_favor_p
                                : R.drawable.mcc201_icon_favor_n);
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

                int defaultImageResId = R.drawable.icon_list_song;
                viewHolder.mMusicIcon.setTag(info.mFilePath);
                BitmapCache.getInstance().loadNativeImage(info.mFilePath, viewHolder.mMusicIcon,
                        mMusicGridView, defaultImageResId, !mIsScroll);
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
        public View getView(final int position, View convertView, ViewGroup parent) {
            ViewHolder viewHolder = null;
            if (convertView == null) {
                convertView = mInflater.inflate(R.layout.mcc201_item_music_list, parent, false);
                viewHolder = new ViewHolder();
                viewHolder.mSongTitle = (TextView) convertView.findViewById(R.id.tvSongTitle);
                viewHolder.mSongArtist = (TextView) convertView.findViewById(R.id.tvSongArtist);
                viewHolder.mMusicIcon = (ImageView) convertView.findViewById(R.id.ivMusicIcon);
                viewHolder.mFavoriteIcon = convertView.findViewById(R.id.ivFavoriteIcon);
                viewHolder.mPlayingIcon = (ImageView) convertView.findViewById(R.id.ivPlayingIcon);
                viewHolder.btn_delete = (ImageButton) convertView.findViewById(R.id.btn_delete);
                convertView.setTag(viewHolder);
            } else {
                viewHolder = (ViewHolder) convertView.getTag();
            }

            viewHolder.mFavoriteIcon.setVisibility(View.VISIBLE);
            viewHolder.mFavoriteIcon.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View arg0) {
                    // TODO Auto-generated method stub
                    final MusicInfo info = mInfoList.get(position);
                    new Thread(new Runnable() {

                        @Override
                        public void run() {
                            // TODO Auto-generated method stub
                        }
                    }).start();
                }
            });

            viewHolder.btn_delete.setVisibility(View.GONE);
            viewHolder.btn_delete.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View arg0) {
                    // TODO Auto-generated method stub
                    MusicInfo info = mInfoList.get(position);
                    mInfoList.remove(position);
                    File file = new File(info.mFilePath);
                    if (file.exists()) {
                        boolean ignored = file.delete();
                    }
                    notifyDataSetChanged();
                }
            });

            if (position < mInfoList.size()) {
                MusicInfo info = mInfoList.get(position);
                updateItem(viewHolder, info, mIndex == info.mIndex);
            }

            return convertView;
        }
    }
}
