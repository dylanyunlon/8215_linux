package com.hcn.media.music.mcc201;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Handler;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;
import android.widget.TextView;

import androidx.annotation.NonNull;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media.adapter.mcc201.Mcc201MusicListAdapter;
import com.hcn.media.R3;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IPlaylistType;
import com.hcn.media_base.impl.MediaEventPostbox;
import com.hcn.media.base.layout.FrameLayoutEx;
import com.hcn.media.vm.action.IPlayerEx;
import com.hcn.mediaservice.data.MusicInfo;

import java.util.ArrayList;
import java.util.List;

/**
 * mcc201 音乐列表布局
 * <pre>
 *    为特定客户需求创建，后续不再维护；
 *    这类布局不可以在布局资源文件（[@layout].xml）中直接使用；
 * </pre>
 *
 * @author 65821
 */
@SuppressLint("ViewConstructor")
public class Mcc201ListMusicLayout extends FrameLayoutEx
        implements OnItemClickListener, OnScrollListener,
        OnClickListener, OnTouchListener {
    private static final String TAG = Mcc201ListMusicLayout.class.getSimpleName();

    private ListView mMusicGridView = null;
    private Mcc201MusicListAdapter mMusicListAdapter = null;
    private TextView mShowTipCtrl = null;
    private Handler mUserHandler = null;
    private List<MusicInfo> mListMusicInfo = null;

    private final Runnable mTimeRunnable = () -> {
        mUserHandler.removeCallbacksAndMessages(null);
        mMusicGridView.setSelection(mAppData.musicPlayPosition());
    };

    public Mcc201ListMusicLayout(Context context,
                                 @NonNull IPlayerEx player) {
        this(context, null, player);
    }

    public Mcc201ListMusicLayout(Context context,
                                 AttributeSet attrs,
                                 @NonNull IPlayerEx player) {
        this(context, attrs, 0, player);
    }

    public Mcc201ListMusicLayout(Context context,
                                 AttributeSet attrs,
                                 int defStyle,
                                 @NonNull IPlayerEx player) {
        super(context, attrs, defStyle, player);

        initView();
    }

    private void initView() {
        LayoutInflater.from(mContext)
                .inflate(R.layout.mcc201_layout_listmusic, this, true);

        mUserHandler = new Handler(mContext.getMainLooper());

        mShowTipCtrl = findViewById(R.id.tvShowTip);

        mMusicGridView = findViewById(R.id.gridview_musics);
        mMusicListAdapter = new Mcc201MusicListAdapter(mContext, mMusicGridView);
        if (null != mMusicGridView) {
            mMusicGridView.setAdapter(mMusicListAdapter);
            mMusicGridView.setOnItemClickListener(this);
            mMusicGridView.setOnScrollListener(this);
        }
    }

    @Override
    public void initLayout() {
        mUserHandler.removeCallbacksAndMessages(null);
        initData();
    }

    private void initData() {
        mListMusicInfo = new ArrayList<>();
        for (int i = 0; i < mAppData.mStorageDeviceList.size(); i++) {
            mListMusicInfo.addAll(mAppData.mStorageDeviceList.get(i).mMusicInfoList);
        }

        mMusicListAdapter.updateInfoList(mListMusicInfo);
        updatePlayIndex();
        updateTipCtrl();
    }

    private void updateTipCtrl() {
        boolean isMusicInfoIsEmpty = mListMusicInfo.isEmpty();
        mShowTipCtrl.setText(xString(R3.string.tip_no_music_file));
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

            case IMediaEvent.EVENT_CONTROL_SMART_ENTER:
                playerRelay().accept(
                        t -> t.requestPlayTarget(
                                IPlaylistType.DEVICE_LIST,
                                mListMusicInfo,
                                mAppData.musicSelectPosition()));
                mAppData.updateMusicSelectPosition(-1);
                break;

            case IMediaEvent.EVENT_CONTROL_SMART_CW:
                mUserHandler.removeCallbacksAndMessages(null);
                if (mAppData.musicSelectPosition() == -1) {
                    mAppData.updateMusicSelectPosition(mAppData.musicPlayPosition() - 1);
                } else if (mAppData.musicSelectPosition() <= 0) {
                    mAppData.updateMusicSelectPosition(mListMusicInfo.size());
                } else {
                    mAppData.updateMusicSelectPosition(mAppData.musicSelectPosition() - 1);
                }
                onUpdateSelection(mAppData.musicSelectPosition());
                break;

            case IMediaEvent.EVENT_CONTROL_SMART_CCW:
                mUserHandler.removeCallbacksAndMessages(null);
                if (mAppData.musicSelectPosition() == -1) {
                    mAppData.updateMusicSelectPosition(mAppData.musicPlayPosition() + 1);
                } else if (mAppData.musicSelectPosition() >= mListMusicInfo.size()) {
                    mAppData.updateMusicSelectPosition(0);
                } else {
                    mAppData.updateMusicSelectPosition(mAppData.musicSelectPosition() + 1);
                }
                onUpdateSelection(mAppData.musicSelectPosition());
                break;

            case IMediaEvent.EVENT_CANCEL_SMART_CONTROL:
                mAppData.updateMusicSelectPosition(-1);
                mMusicListAdapter.updateSelectIndex(-1);
                mUserHandler.removeCallbacksAndMessages(null);
                mUserHandler.postDelayed(mTimeRunnable, 1000);
                break;

            case IMediaEvent.EVENT_MEDIA_LOADING_START:
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
            case IMediaEvent.EVENT_CHANGE_MUSIC_STORAGE:
            case IMediaEvent.EVENT_ID3_SCAN_FINISHED:
            case IMediaEvent.EVENT_MEDIA_MOUNTED:
            case IMediaEvent.EVENT_MEDIA_UNMOUNTED:
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

    private void onUpdateSelection(int pos) {
        mMusicListAdapter.updateSelectIndex(pos);
        mMusicGridView.setSelection(pos);
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

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        playerRelay().accept(t -> t.requestPlayTarget(
                IPlaylistType.DEVICE_LIST, mListMusicInfo, position));
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
}
