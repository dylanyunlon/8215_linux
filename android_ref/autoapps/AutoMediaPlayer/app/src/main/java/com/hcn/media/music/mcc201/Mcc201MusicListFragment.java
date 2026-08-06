package com.hcn.media.music.mcc201;

import static android.carsource.McuConstant.K_EQ;

import android.annotation.SuppressLint;
import android.carsource.McuManager;
import android.content.Context;
import android.content.Intent;
import android.media.audiofx.AudioEffect;
import android.os.Bundle;
import android.sourceservice.ExtAudioMuxer;

import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.viewpager.widget.PagerAdapter;
import androidx.viewpager.widget.ViewPager;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media.R3;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media.vm.action.IMediaAction;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media.local.utils.HFuncUtils;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media_view.NoScrollViewPager;

import java.util.Objects;

/**
 * 音乐列表页面（mcc201）
 * @author 86158
 */
public class Mcc201MusicListFragment extends MediaFragment
        implements OnClickListener, ViewPager.OnPageChangeListener, OnCheckedChangeListener {
    private final static String FRAGMENT_NAME = "music-info-mcc201";
    private static final String TAG = Mcc201MusicListFragment.class.getSimpleName();

    private boolean mInitView = false;
    public NoScrollViewPager mViewPager = null;
    private ViewPagerAdapter mViewPagerAdapter = null;

    private RadioGroup mRadioGroup = null;
    private RadioButton btnMusics = null;
    private RadioButton btnFiles = null;
    private RadioButton btnArtists = null;
    private RadioButton btnSearch = null;
    private RadioButton btnFavorite = null;
    private TextView tvSongSums = null;
    private TextView tvTitleBar = null;
    private Button btnAuto = null;

    // 分类列表布局视图
    private Mcc201ListMusicLayout mMcc201ListMusicLayout = null;
    private Mcc201ListFileMusicLayout mMcc201ListFileMusicLayout = null;
    private Mcc201ListArtistMusicLayout mMcc201ListArtistMusicLayout = null;
    private Mcc201MusicSearchLayout mMcc201MusicSearchLayout = null;

    public Mcc201MusicListFragment() {
        super(FRAGMENT_NAME);
    }

    @Override
    public void initFragment() {
        mViewPager.setCurrentItem(mAppData.mMusicListPageType);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public void uninitFragment() {
        super.uninitFragment();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.mcc201_fragment_music_list, container, false);
        initView(view);
        initFragment();
        mInitView = true;
        return view;
    }

    private void initView(View layout) {
        btnMusics = layout.findViewById(R.id.btnMusics);
        btnFiles = layout.findViewById(R.id.btnFiles);
        btnArtists = layout.findViewById(R.id.btnArtists);
        btnSearch = layout.findViewById(R.id.btnSearch);
        btnFavorite = layout.findViewById(R.id.btnFavorite);

        mRadioGroup = layout.findViewById(R.id.rgList);
        mRadioGroup.setOnCheckedChangeListener(this);

        tvTitleBar = layout.findViewById(R.id.tvTitleBar);
        if (mAppData.mCurrentMediaInfo != null) {
            changeMusicInfo(mAppData.mCurrentMediaInfo);
        }

        tvSongSums = layout.findViewById(R.id.tvSongSums);
        btnAuto = layout.findViewById(R.id.btnAuto);
        btnAuto.setOnClickListener(this);

        btnMusics.setOnClickListener(this);
        btnFiles.setOnClickListener(this);
        btnArtists.setOnClickListener(this);
        btnSearch.setOnClickListener(this);
        btnFavorite.setOnClickListener(this);

        mViewPager = layout.findViewById(R.id.viewpager_center);
        mViewPagerAdapter = new ViewPagerAdapter(mContext);
        mViewPager.setAdapter(mViewPagerAdapter);
        mViewPager.setOnPageChangeListener(this);
        mViewPager.setOffscreenPageLimit(4);

        layout.findViewById(R.id.btnHome).setOnClickListener(this);
        layout.findViewById(R.id.btnBack).setOnClickListener(this);
    }

    @Override
    public void doCallbackEvent(int eventId) {
        if (!mInitView) {
            return;
        }

        switch (eventId) {
            case IMediaEvent.EVENT_MEDIA_LOADING_START:
            case IMediaEvent.EVENT_MEDIA_LOADING_COMPLETE:
                enableUpdateListCtrl();
                break;
            case IMediaEvent.EVENT_SCROLL_SEEKBAR:
                mViewPager.setNoScroll(true);
                break;
            case IMediaEvent.EVENT_STOP_SCROLL_SEEKBAR:
                mViewPager.setNoScroll(false);
                break;
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
                if (mAppData.mCurrentMediaInfo != null) {
                    changeMusicInfo(mAppData.mCurrentMediaInfo);
                }
                break;
            case IMediaEvent.EVENT_MEDIA_MOUNTED:
            case IMediaEvent.EVENT_MEDIA_UNMOUNTED:
            default:
                break;
        }

        dispatchCallbackEvent(eventId);
    }

    /** 分发事件到子元素视图 **/
    private void dispatchCallbackEvent(int nEventID) {
        if (mMcc201ListMusicLayout != null) {
            mMcc201ListMusicLayout.doCallbackEvent(nEventID);
        }

        if (mMcc201ListFileMusicLayout != null) {
            mMcc201ListFileMusicLayout.doCallbackEvent(nEventID);
        }

        if (mMcc201ListArtistMusicLayout != null) {
            mMcc201ListArtistMusicLayout.doCallbackEvent(nEventID);
        }

        if (mMcc201MusicSearchLayout != null) {
            mMcc201MusicSearchLayout.doCallbackEvent(nEventID);
        }
    }

    private void changeMusicInfo(MusicInfo info) {
        Context context = getContext();
        if (Objects.isNull(context)) {
            return;
        }

        if (info != null) {
            String text_unknown = getString(R3.string.text_unknown);
            if (TextUtils.isEmpty(info.mTitle) || "<Unknown>".equals(info.mTitle)) {
                if (TextUtils.isEmpty(info.mFileName)) {
                    tvTitleBar.setText(text_unknown);
                } else {
                    int pos = info.mFileName.lastIndexOf(".");
                    if (pos != -1) {
                        String Name = info.mFileName.substring(0, pos);
                        if (TextUtils.isEmpty(Name)) {
                            tvTitleBar.setText(text_unknown);
                        } else {
                            tvTitleBar.setText(Name);
                        }
                    } else {
                        tvTitleBar.setText(info.mFileName);
                    }
                }
            } else {
                int pos = info.mTitle.lastIndexOf(".");
                if (pos != -1) {
                    String title = info.mTitle.substring(0, pos);
                    if (TextUtils.isEmpty(title)) {
                        tvTitleBar.setText(text_unknown);
                    } else {
                        tvTitleBar.setText(title);
                    }
                } else {
                    tvTitleBar.setText(info.mTitle);
                }
            }
        }
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        mMusicViewModel.fragment2MainUi().execute(
                t -> t.onEvent(eventId, wParam, lParam));
    }

    @Override
    public void onPageScrolled(int arg0, float arg1, int arg2) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onPageScrollStateChanged(int arg0) {
    }

    @Override
    public void onPageSelected(int arg0) {
        // TODO Auto-generated method stub
        onChangeListType(arg0);
        switch (arg0) {
            case 0:
            case 1:
            case 2:
            case 3:
                btnAuto.setText(getString(R3.string.favor_all_label));
                break;
            case 4:
                btnAuto.setText(getString(R3.string.delete_all_label));
                break;

            default:
                break;
        }
    }

    private void onChangeListType(int nListType) {
        switch (nListType) {
            case 0:
                btnMusics.setChecked(true);
                break;
            case 1:
                btnFiles.setChecked(true);
                break;
            case 2:
                btnArtists.setChecked(true);
                break;
            case 3:
                btnSearch.setChecked(true);
                break;
            case 4:
                btnFavorite.setChecked(true);
                break;

            default:
                break;
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btnListSong:
                mViewPager.setCurrentItem(0, true);
                break;
            case R.id.btnListAlbum:
                mViewPager.setCurrentItem(1, true);
                break;
            case R.id.btnListArtist:
                mViewPager.setCurrentItem(2, true);
                break;
            case R.id.btnStorageSD:
                mViewPager.setCurrentItem(3, true);
                break;
            case R.id.btnListFolder:
                mViewPager.setCurrentItem(4, true);
                break;
            case R.id.btnUpdate:
                onUpdateListEvent();
                break;
            case R.id.btnPlaying:
                onBackEvent();
                break;
            case R.id.btnEQ:
                HFuncUtils.instance().gotoEQ(mContext);
                break;
            case R.id.btnHome:
                onHomeEvent();
                break;
            case R.id.btnBack:
                requireActivity().onBackPressed();
                break;
            case R.id.btnAuto:
            default:
                break;
        }
    }

    private void onHomeEvent() {
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_HOME);
        mContext.startActivity(intent);
    }

    private void onUpdateListEvent() {
        if (mAppData.mSelectedDevice != null) {
            mMusicViewModel.playerRelay().accept(
                    t -> t.requestExecuteAction(
                            IMediaAction.scanStorageDeviceInfo,
                            mAppData.mSelectedDevice.mFilePath,
                            null));
        }
    }

    private void enableUpdateListCtrl() {
        Context context = getContext();
        if (Objects.isNull(context)) {
            return;
        }
    }

    private void onBackEvent() {
        mMusicViewModel.fragment2MainUi().execute(
                t -> t.onEvent(IMediaEvent.EVENT_GOTO_MUSIC_INFO_PAGE, null, null));
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onCheckedChanged(RadioGroup rg, int checkId) {
        switch (checkId) {
            case R.id.btnMusics:
                mViewPager.setCurrentItem(0, true);
                break;
            case R.id.btnFiles:
                mViewPager.setCurrentItem(1, true);
                break;
            case R.id.btnArtists:
                mViewPager.setCurrentItem(2, true);
                break;
            case R.id.btnSearch:
                mViewPager.setCurrentItem(3, true);
                break;
            case R.id.btnFavorite:
                mViewPager.setCurrentItem(4, true);
                break;
            default:
                break;
        }
    }

    private void onEQEvent() {
        if (ExtAudioMuxer.ExtAudioAvailable) {
            McuManager.getsInstance().injectKeyEventTimeout(K_EQ, 50);
        } else {

            try {
                int audioSessionId = mMusicViewModel.getAudioSessionId();
                Intent intent = new Intent(
                        AudioEffect.ACTION_DISPLAY_AUDIO_EFFECT_CONTROL_PANEL);
                intent.putExtra(AudioEffect.EXTRA_AUDIO_SESSION, audioSessionId);
                requireActivity().startActivity(intent);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        LogUtil.d(TAG, "onDestroyView.");

        // 清理 ViewPager 中的 View;
        mViewPager.removeAllViews();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    private class ViewPagerAdapter extends PagerAdapter {
        static final int VIEW_TAG_KEY = 0xEE00F001;

        public ViewPagerAdapter(Context context) {
        }

        @Override
        public int getCount() {
            return 4;
        }

        @Override
        public boolean isViewFromObject(@NonNull View view, @NonNull Object object) {
            return view == object;
        }

        @NonNull
        @Override
        public View instantiateItem(@NonNull ViewGroup container, int position) {
            View view = null;
            if (position == 0) {
                if (mMcc201ListMusicLayout == null) {
                    mMcc201ListMusicLayout = new Mcc201ListMusicLayout(mContext, mMusicViewModel);
                    mMcc201ListMusicLayout.setTag(VIEW_TAG_KEY, "list");
                    mMcc201ListMusicLayout.initDataObject();
                    mMcc201ListMusicLayout.setMediaEventListener(mPostbox);
                } else {
                    mMcc201ListMusicLayout.initLayout();
                }

                view = mMcc201ListMusicLayout;
            } else if (position == 1) {
                if (null == mMcc201ListFileMusicLayout) {
                    mMcc201ListFileMusicLayout = new Mcc201ListFileMusicLayout(mContext, mMusicViewModel);
                    mMcc201ListFileMusicLayout.setTag(VIEW_TAG_KEY, "file");
                    mMcc201ListFileMusicLayout.initDataObject();
                    mMcc201ListFileMusicLayout.setMediaEventListener(mPostbox);
                } else {
                    mMcc201ListFileMusicLayout.initLayout();
                }

                view = mMcc201ListFileMusicLayout;
            } else if (position == 2) {
                if (null == mMcc201ListArtistMusicLayout) {
                    mMcc201ListArtistMusicLayout = new Mcc201ListArtistMusicLayout(mContext, mMusicViewModel);
                    mMcc201ListArtistMusicLayout.setTag(VIEW_TAG_KEY, "artist");
                    mMcc201ListArtistMusicLayout.initDataObject();
                    mMcc201ListArtistMusicLayout.setMediaEventListener(mPostbox);
                } else {
                    mMcc201ListArtistMusicLayout.initLayout();
                }

                view = mMcc201ListArtistMusicLayout;
            } else if (position == 3) {
                if (null == mMcc201MusicSearchLayout) {
                    mMcc201MusicSearchLayout = new Mcc201MusicSearchLayout(mContext, mMusicViewModel);
                    mMcc201MusicSearchLayout.setTag(VIEW_TAG_KEY, "search");
                    mMcc201MusicSearchLayout.initDataObject();
                    mMcc201MusicSearchLayout.setMediaEventListener(mPostbox);
                } else {
                    mMcc201MusicSearchLayout.initLayout();
                }

                view = mMcc201MusicSearchLayout;
            }

            assert view != null;
            String viewTag = (String) view.getTag(VIEW_TAG_KEY);
            int childCount = container.getChildCount();
            LogUtil.v(TAG, "instantiateItem: "
                    + "position = " + position + "/" + childCount +  ", tag = " + viewTag);

            // 重复添加 view 到父对象将导致 IllegalStateException
            if (view.getParent() != null) {
                LogUtil.v(TAG, "instantiateItem: The specified child already has a parent.");
                container.removeView(view);
            }

            container.addView(view, LinearLayout.LayoutParams.MATCH_PARENT,
                    LinearLayout.LayoutParams.MATCH_PARENT);
            return view;
        }

        @Override
        public void destroyItem(ViewGroup container, int position, @NonNull Object object) {
            View view = (View) object;
            int childCount = container.getChildCount();
            LogUtil.v(TAG, "destroyItem: "
                    + "position = " + position + "/" + childCount
                    +  ", tag = " + view.getTag(VIEW_TAG_KEY));

            container.removeView(view);
        }
    }
}
