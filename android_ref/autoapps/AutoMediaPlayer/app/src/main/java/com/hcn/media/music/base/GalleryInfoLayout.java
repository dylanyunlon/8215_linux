package com.hcn.media.music.base;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.os.Handler;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ImageView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.common.misc.LogUtils;
import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_data.base.BaseMediaData;
import com.hcn.media_theme.Argument;
import com.hcn.media.base.layout.FrameLayoutEx;
import com.hcn.media.vm.action.IPlayerEx;
import com.hcn.media_theme.ThemeX;
import com.hcn.media.adapter.MusicGalleryAdapter;
import com.hcn.media_base.IMediaEvent;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.media_view.GalleryFlow;

import java.util.Objects;

/**
 * mcc154 音乐信息布局视图
 * @author 86158
 */
@SuppressLint("ViewConstructor")
@SuppressWarnings("deprecation")
public class GalleryInfoLayout extends FrameLayoutEx
        implements OnClickListener, OnItemClickListener,
        OnItemSelectedListener, OnTouchListener {

    private ImageView ivShowRight = null;
    private GalleryFlow mGalleryFlow = null;
    private MusicGalleryAdapter mMusicListAdapter = null;
    private Handler mUserHandler = null;
    private Handler mSelectorHandler = null;

    /**
     * 超时任务
     * <p> 滑动校准画廊使用;
     */
    private final Runnable mTimeRunnable = () -> {
        mUserHandler.removeCallbacksAndMessages(null);
        onUpdateSelection(mAppData.musicPlayPosition());
    };

    /**
     * 延迟任务
     * <p> 滑动后高亮当前选中的;
     */
    private final Runnable mSelectorRunnable = () -> {
        mMusicListAdapter.updateSelectIndex(mGalleryFlow.getSelectedItemPosition());
    };

    @Override
    protected String TAG() {
        return GalleryInfoLayout.class.getSimpleName();
    }

    public GalleryInfoLayout(Context context,
                             @NonNull IPlayerEx player) {
        this(context, null, player);
    }

    public GalleryInfoLayout(Context context,
                             AttributeSet attrs,
                             @NonNull IPlayerEx player){
        this(context, attrs, 0, player);
    }

    public GalleryInfoLayout(Context context,
                             AttributeSet attrs,
                             int defStyle,
                             @NonNull IPlayerEx player) {
        super(context, attrs, defStyle, player);

        // [注意：不可以提取放到父类中使用/否则 inflate 的视图会被系统回收]
        initContentView(getLayoutRes(), this);
    }

    @Override
    protected int getLayoutRes() {
        return R.layout.layout_musicinfo;
    }

    @Nullable
    @Override
    protected View initContentView(int layoutRes, ViewGroup root) {
        View view = super.initContentView(layoutRes, root);

        // 显示提示
        assert view != null;
        mUserHandler = new Handler(mContext.getMainLooper());
        mSelectorHandler = new Handler(mContext.getMainLooper());
        mMusicListAdapter = new MusicGalleryAdapter(mContext);

        mGalleryFlow = findViewById(xId(R.id.gallery_flow));
        if (Argument.isThemeGod(ThemeX.ET_GOD_405)) {
            mGalleryFlow.setSpacing(-30);
        } else {
            mGalleryFlow.setSpacing(3);
        }

        mGalleryFlow.setAnimationDuration(2000);
        mGalleryFlow.setGravity(Gravity.CENTER_VERTICAL);
        mGalleryFlow.setOnItemClickListener(this);
        mGalleryFlow.setOnItemSelectedListener(this);
        mGalleryFlow.setOnTouchListener(this);
        mGalleryFlow.setAdapter(mMusicListAdapter);

        ivShowRight = findViewById(xId(R.id.ivShowRight));

        initLayout();
        return view;
    }

    public void setAnimationState(boolean start) {
        if (Objects.isNull(ivShowRight)) {
            return;
        }

        AnimationDrawable right = (AnimationDrawable) ivShowRight.getBackground();
        if (null != right) {
            if (start && !right.isRunning()) {
                right.start();
            } else if (!start && right.isRunning()) {
                right.stop();
            }
        }
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();

        setAnimationState(true);
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();

        setAnimationState(false);
    }

    @Override
    public void onClick(View v) {
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), position)) {
            mAppData.updateMusicPlayPosition(position);
        }

        onChangeMusicEvent(position);
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        // 滑动时设置当前选中的 index
        mSelectorHandler.removeCallbacksAndMessages(null);
        mSelectorHandler.postDelayed(mSelectorRunnable, 500);
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {
        // 没有选中时，设置为-1
        mSelectorHandler.removeCallbacksAndMessages(null);
        mMusicListAdapter.updateSelectIndex(-1);
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        mUserHandler.removeCallbacksAndMessages(null);
        mUserHandler.postDelayed(mTimeRunnable, 10000);
        return false;
    }

    private void onChangeMusicEvent(int position) {
        if (position < mAppData.musicPlaylist().size()) {
            mAppData.mMusicPlayIndex =
                    mAppData.musicPlaylist().get(position).mIndex;
        }

        if (mMediaEventPostbox != null) {
            mMediaEventPostbox.onMediaEvent(
                    IMediaEvent.EVENT_CHANGE_MUSIC_ITEM,
                    position,
                    null);
        }
    }

    @Override
    public void initLayout() {
        mUserHandler.removeCallbacksAndMessages(null);

        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
            // 重置选择焦点项
            mAppData.updateMusicSelectPosition(-1);

            // 更新显示列表信息
            mMusicListAdapter.updateInfoList(mAppData.musicPlaylist());
            mMusicListAdapter.updatePlayIndex(mAppData.mMusicPlayIndex);
            mMusicListAdapter.updateSelectIndex(-1);

            // mGalleryFlow.setSelection(mAppData.mMusicPosition, false);
            onUpdateSelection(mAppData.musicPlayPosition());
        }
    }

    private void onUpdateSelection(int currentPos) {
        // 参数有效性检查
        if (!BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), currentPos)) {
            return;
        }

        mMusicListAdapter.updatePlayIndex(mAppData.mMusicPlayIndex);
        int position = mGalleryFlow.getSelectedItemPosition();

        if (BaseMediaData.isValidIndex(
                mAppData.musicPlaylist(), position)) {
            if (currentPos > position) {
                if (currentPos > 1) {
                    mGalleryFlow.setSelection(currentPos - 1, false);
                } else {
                    mGalleryFlow.setSelection(0, false);
                }

                mGalleryFlow.onKeyDown(KeyEvent.KEYCODE_DPAD_RIGHT, null);
            } else if (currentPos < position) {
                if (currentPos < mAppData.musicPlaylist().size() - 1) {
                    mGalleryFlow.setSelection(currentPos + 1, false);
                } else {
                    mGalleryFlow.setSelection(mAppData.musicPlaylist().size() - 1, false);
                }

                mGalleryFlow.onKeyDown(KeyEvent.KEYCODE_DPAD_LEFT, null);
            } else {
                mGalleryFlow.setSelection(currentPos);
            }
        } else {
            mGalleryFlow.setSelection(currentPos);
        }
    }

    @Override
    public void doCallbackEvent(int eventId) {
        switch (eventId) {
            case IMediaEvent.EVENT_CHANGE_MUSIC_ITEM:
                onUpdateSelection(mAppData.musicPlayPosition());
                break;

            case IMediaEvent.EVENT_UPDATE_MUSIC_LIST:
            case IMediaEvent.EVENT_CHANGE_MUSIC_LIST:
                if (BaseMediaData.isValidIndex(
                        mAppData.musicPlaylist(), mAppData.musicPlayPosition())) {
                    mAppData.updateMusicSelectPosition(-1);
                    mMusicListAdapter.updateInfoList(mAppData.musicPlaylist());
                    mMusicListAdapter.updatePlayIndex(mAppData.mMusicPlayIndex);
                    mMusicListAdapter.updateSelectIndex(-1);

                    // mGalleryFlow.setSelection(mAppData.mMusicPosition, false);
                    onUpdateSelection(mAppData.musicPlayPosition());
                }
                break;

            case IMediaEvent.EVENT_CONTROL_SMART_CW:
                mUserHandler.removeCallbacksAndMessages(null);
                if (mAppData.musicSelectPosition() == -1) {
                    mAppData.updateMusicSelectPosition(
                            mAppData.musicPlayPosition());
                }

                if (mAppData.musicSelectPosition() >= 0
                        && mAppData.musicSelectPosition() < mAppData.musicPlaylist().size()) {
                    mAppData.updateMusicSelectPosition(
                            (mAppData.musicPlaylist().size() + mAppData.musicSelectPosition() - 1)
                                    % mAppData.musicPlaylist().size());
                    MusicInfo info = mAppData.musicPlaylist().get(mAppData.musicSelectPosition());
                    mMusicListAdapter.updateSelectIndex(info.mIndex);
                    onUpdateSelection(mAppData.musicSelectPosition());
                }
                break;

            case IMediaEvent.EVENT_CONTROL_SMART_CCW:
                mUserHandler.removeCallbacksAndMessages(null);
                if (mAppData.musicSelectPosition() == -1) {
                    mAppData.updateMusicSelectPosition(mAppData.musicPlayPosition());
                }

                if (mAppData.musicSelectPosition() >= 0
                        && mAppData.musicSelectPosition() < mAppData.musicPlaylist().size()) {
                    mAppData.updateMusicSelectPosition(
                            (mAppData.musicSelectPosition() + 1) % mAppData.musicPlaylist().size());
                    MusicInfo info = mAppData.musicPlaylist().get(mAppData.musicSelectPosition());
                    mMusicListAdapter.updateSelectIndex(info.mIndex);
                    onUpdateSelection(mAppData.musicSelectPosition());
                }
                break;

            case IMediaEvent.EVENT_CANCEL_SMART_CONTROL:
                mAppData.updateMusicSelectPosition(-1);;
                mMusicListAdapter.updateSelectIndex(-1);
                mUserHandler.removeCallbacksAndMessages(null);
                mUserHandler.postDelayed(mTimeRunnable, 1000);
                break;

            default:
                break;
        }
    }

    @Override
    public void onUpdateUiModeView(boolean isNightMode) {
        super.onUpdateUiModeView(isNightMode);

        // 通知适配器更新,重新设置资源
        mMusicListAdapter.notifyDataSetChanged();
    }

    /**
     * 更新专辑封面
     * <p> 强制更新，这里表示内存中有了封面信息了；
     *
     * @param path 需要更新封面的歌曲路径
     */
    public void updateAlbumCover(String path) {
        if (Objects.isNull(mMusicListAdapter)) {
            return;
        }

        mMusicListAdapter.notifyDataSetChanged();
    }
}
