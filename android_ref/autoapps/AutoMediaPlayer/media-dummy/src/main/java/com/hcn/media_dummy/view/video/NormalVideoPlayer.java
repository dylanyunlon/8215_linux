package com.hcn.media_dummy.view.video;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.ImageView;

import com.hcn.media_dummy.R;

/**
 * 正常的视频播放器
 * <p> 使用正常播放按键 和 loading 的播放器;
 *
 * @author 65821
 */
public class NormalVideoPlayer extends StandardVideoPlayer {

    public NormalVideoPlayer(Context context, Boolean fullFlag) {
        super(context, fullFlag);
    }

    public NormalVideoPlayer(Context context) {
        super(context);
    }

    public NormalVideoPlayer(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public int getLayoutId() {
        return R.layout.video_layout_normal;
    }

    @Override
    protected void updateStartImage() {
        if (mStartButton instanceof ImageView) {
            ImageView imageView = (ImageView) mStartButton;
            if (mCurrentState == CURRENT_STATE_PLAYING) {
                imageView.setImageResource(
                        R.drawable.video_click_pause_selector);
            } else if (mCurrentState == CURRENT_STATE_ERROR) {
                imageView.setImageResource(
                        R.drawable.video_click_play_selector);
            } else {
                imageView.setImageResource(
                        R.drawable.video_click_play_selector);
            }
        }
    }
}
