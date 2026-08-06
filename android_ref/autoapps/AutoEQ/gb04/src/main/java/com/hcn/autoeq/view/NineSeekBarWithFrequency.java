package com.hcn.autoeq.view;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.animation.LinearInterpolator;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.hcn.autoeq.adapter.CircularAdapter;
import com.hcn.autoeq.adapter.CircularLayoutManager;
import com.hcn.autoeq.R;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.SkinUtils;
import com.hcn_library.view.SeekBarRotator;

import java.util.Arrays;
import java.util.List;

public class NineSeekBarWithFrequency extends ConstraintLayout implements SeekBar.OnSeekBarChangeListener {
    private static int DEFAULT_HEIGHT;
    private Context context;
    private ImageView ivScale;
    private ImageView moveDown;
    private ImageView moveUp;
    private CompoundButton.OnCheckedChangeListener onCheckedChangeListener;
    private SeekBar.OnSeekBarChangeListener onSeekBarChangeListener;
    private int progress;
    private SeekBar seekBar;
    private SeekBarRotator sbr_band;
    private int seekBarMax = 100;
    private TextView tvDown;
    private TextView tvTop2;
    private RecyclerView recyclerView;
    private CircularLayoutManager circularLayoutManager;
    private CircularAdapter adapter;
    private List<String> dataList = Arrays.asList("40HZ", "60HZ", "100HZ", "150HZ", "200HZ", "250HZ", "300HZ", "400HZ");
    private boolean isClickScroll = true; // 点击滚动状态默认为true
    private Button _40;
    private Button _63;
    private Button _80;
    private Button _125;
    private FreqCallback freqCallback;


    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    public NineSeekBarWithFrequency(Context context) {
        super(context);
        init();
    }

    public NineSeekBarWithFrequency(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
        this.context = context;
        init();
        initStyle(attributeSet);
    }

    private void init() {
        LayoutInflater.from(context).inflate(SkinUtils.getId(R.layout.nine_seekbar_frequency), this);
        DEFAULT_HEIGHT = (int) SkinUtils.getDimension(R.dimen.nine_dts_subwoofer_seekbar_height);
        tvTop2 = (TextView) findViewById(SkinUtils.getId(R.id.tv_top2));
        tvDown = (TextView) findViewById(SkinUtils.getId(R.id.tv_down));
        seekBar = (SeekBar) findViewById(SkinUtils.getId(R.id.sb_nine));
        sbr_band = findViewById(SkinUtils.getId(R.id.sbr_band));
        ivScale = (ImageView) findViewById(SkinUtils.getId(R.id.iv_scale));
        moveUp = (ImageView) findViewById(SkinUtils.getId(R.id.iv_top));
        moveDown = (ImageView) findViewById(SkinUtils.getId(R.id.iv_down));
        seekBar.setMax(seekBarMax);
        seekBar.setOnSeekBarChangeListener(this);
        recyclerView = findViewById(SkinUtils.getId(R.id.rv_freq));
        circularLayoutManager = new CircularLayoutManager(context, RecyclerView.VERTICAL, false);
        recyclerView.setLayoutManager(circularLayoutManager);
        adapter = new CircularAdapter(dataList);
        recyclerView.setAdapter(adapter);
        // 手指滑动结束后，让最近一个item居中显示
        recyclerView.addOnScrollListener(new RecyclerView.OnScrollListener() {
            @Override
            public void onScrollStateChanged(RecyclerView recyclerView, int newState) {
                super.onScrollStateChanged(recyclerView, newState);
                int currentIndex = 0;
                Log.d("onScrollStateChanged", " circularLayoutManager.getTouchScroll(): " + circularLayoutManager.getTouchScroll() + " isClickScroll: " + isClickScroll);
                if (newState == RecyclerView.SCROLL_STATE_IDLE) {
                    LinearLayoutManager layoutManager = (LinearLayoutManager) recyclerView.getLayoutManager();
                    if (layoutManager != null && !isClickScroll) {
                        int firstVisibleItemPosition = layoutManager.findFirstVisibleItemPosition();
                        int lastVisibleItemPosition = layoutManager.findLastVisibleItemPosition();
                        if (firstVisibleItemPosition == lastVisibleItemPosition) {
                            // 只有一个可见项，无需比较
                            currentIndex = firstVisibleItemPosition;
                        }
                        View firstVisibleView = layoutManager.findViewByPosition(firstVisibleItemPosition);
                        View lastVisibleView = layoutManager.findViewByPosition(lastVisibleItemPosition);
                        if (firstVisibleView != null && lastVisibleView != null) {
                            int firstVisibleViewVisibleHeight = calculateVisibleHeight(firstVisibleView, layoutManager);
                            int lastVisibleViewVisibleHeight = calculateVisibleHeight(lastVisibleView, layoutManager);
                            int firstVisibleViewHeight = firstVisibleView.getHeight();
                            int lastVisibleViewHeight = lastVisibleView.getHeight();
                            float firstVisibleViewRatio = (float) firstVisibleViewVisibleHeight / firstVisibleViewHeight;
                            float lastVisibleViewRatio = (float) lastVisibleViewVisibleHeight / lastVisibleViewHeight;

                            if (firstVisibleViewRatio >= lastVisibleViewRatio) {
                                currentIndex = firstVisibleItemPosition;
                            } else {
                                currentIndex = lastVisibleItemPosition;
                            }
                            recyclerView.smoothScrollToPosition(currentIndex);
                            if (scrollResultInterface != null) {
                                scrollResultInterface.itemChange(currentIndex);
                            }
                            Log.d("onScrollStateChanged", "First visible item ratio: " + firstVisibleViewRatio + "  Last visible item ratio: " + lastVisibleViewRatio + " currentIndex: " + currentIndex);
                        }
                    }
                }
                isClickScroll = false; // 复原点击滚动模式的状态
            }
        });
        // 判断是否是手指滑动操作
        recyclerView.setOnTouchListener((v, event) -> {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                case MotionEvent.ACTION_MOVE:
                    circularLayoutManager.setTouchScroll(true);
                    break;
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    circularLayoutManager.setTouchScroll(false);
                    break;
            }
            return false;
        });
        if (EqUtils.isChip7739()) {
            initGB02Freq();
        }
    }

    private int calculateVisibleHeight(View view, LinearLayoutManager layoutManager) {
        int viewTop = layoutManager.getDecoratedTop(view);
        int viewBottom = layoutManager.getDecoratedBottom(view);
        int containerTop = layoutManager.getPaddingTop();
        int containerBottom = recyclerView.getHeight() - layoutManager.getPaddingBottom();
        int visibleHeight = Math.min(containerBottom, viewBottom) - Math.max(containerTop, viewTop);
        return visibleHeight;
    }

    private void initStyle(AttributeSet attributeSet) {
        TypedArray obtainStyledAttributes = getContext().obtainStyledAttributes(attributeSet, R.styleable.nine_seek_bar_attr);
        if (obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_seek_bar_top_name_two) != null) {
            tvTop2.setText(obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_seek_bar_top_name_two));
        }
        if (obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_seek_bar_down_name) != null) {
            tvDown.setText(obtainStyledAttributes.getString(R.styleable.nine_seek_bar_attr_nine_seek_bar_down_name));
        }
        seekBar.setTag(getTag());
    }

    public void setProgress(int progress, boolean showAnim) {
        if (showAnim) {
            volumeGradient(seekBar, seekBar.getProgress(), progress);
        } else {
            seekBar.setProgress(progress);
        }
    }

    public void setSeekBarMax(int i) {
        if (seekBar != null) {
            seekBarMax = i;
            seekBar.setMax(i);
        }
    }

    public void setSeekBarStatus(boolean enable) {
        if (seekBar != null) {
            if (enable) {
                seekBar.setThumb(SkinUtils.getDrawable(R.drawable.nine_seek_bar_thumb));
            } else {
                seekBar.setThumb(SkinUtils.getDrawable(R.drawable.nine_seek_bar_thumb_default));
            }
            seekBar.setEnabled(enable);
        }
    }

    public void setTopNameTwo(String str) {
        tvTop2.setText(str);
    }

    // 初始化和按钮点击scrollTo y 指定位置，滚动距离为中间item高度之和
    public void setTvFreq(int index) {
        LinearLayoutManager layoutManager = (LinearLayoutManager) recyclerView.getLayoutManager();
        recyclerView.post(() -> {
            int firstVisibleItemPosition = layoutManager.findFirstVisibleItemPosition();
            int offset = 0;
            int height = 0;
            for (int i = 0; i < dataList.size(); i++) {
                View itemView = layoutManager.findViewByPosition(i);
                if (itemView != null) {
                    height = itemView.getHeight();
                }
            }
            // 计算偏移量
            if (firstVisibleItemPosition < index) {
                for (int i = firstVisibleItemPosition; i < index; i++) {
                    offset += height;

                }
            } else if (firstVisibleItemPosition > index) {
                for (int i = index; i < firstVisibleItemPosition; i++) {
                    offset -= height;
                }
            }
            recyclerView.scrollBy(0, offset);
            isClickScroll = true;
            Log.d("setTvFreq", "index: " + index + " firstVisibleItemPosition: " + firstVisibleItemPosition + " offset: " + offset + " height: " + height);
            Log.d("setTvFreq after", "index: " + index + " firstVisibleItemPosition: " + layoutManager.findFirstVisibleItemPosition() + " offset: " + offset + " height: " + height);
        });

    }

    public int getProgress() {
        return seekBar.getProgress();
    }

    public void setOnSeekBarChangeListener(SeekBar.OnSeekBarChangeListener onSeekBarChangeListener) {
        this.onSeekBarChangeListener = onSeekBarChangeListener;
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int i, boolean z) {
        if (onSeekBarChangeListener != null) {
            onSeekBarChangeListener.onProgressChanged(seekBar, i, z);
            setTopNameTwo(i + "");
        }
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        if (onSeekBarChangeListener != null) {
            progress = seekBar.getProgress();
            setTopNameTwo(progress + "");
            onSeekBarChangeListener.onStopTrackingTouch(seekBar);
        }
    }

    private void volumeGradient(final SeekBar seekBar, int i, final int i2) {
        ValueAnimator ofInt = ValueAnimator.ofInt(i, i2);
        ofInt.setDuration(300L);
        ofInt.setInterpolator(new LinearInterpolator());
        ofInt.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator valueAnimator) {
                seekBar.setProgress(((Integer) valueAnimator.getAnimatedValue()).intValue());
            }
        });
        ofInt.addListener(new Animator.AnimatorListener() {
            @Override
            public void onAnimationRepeat(Animator animator) {
            }

            @Override
            public void onAnimationStart(Animator animator) {
            }

            @Override
            public void onAnimationEnd(Animator animator) {
                seekBar.setProgress(i2);
            }

            @Override
            public void onAnimationCancel(Animator animator) {
                try {
                    seekBar.setProgress(i2);
                } catch (Exception unused) {
                }
            }
        });
        ofInt.start();
    }


    public void setOnCheckedChangeListener(CompoundButton.OnCheckedChangeListener onCheckedChangeListener) {
        this.onCheckedChangeListener = onCheckedChangeListener;
    }


    public ImageView getMoveUp() {
        return moveUp;
    }

    public ImageView getMoveDown() {
        return moveDown;
    }

    ScrollResultInterface scrollResultInterface;

    public void setScrollResultInterface(ScrollResultInterface resultInterface) {
        scrollResultInterface = resultInterface;
    }

    public interface ScrollResultInterface {
        void itemChange(int currentIndex);
    }

    public TextView getTvDown() {
        return tvDown;
    }

    public void hideFreq() {
        recyclerView.setVisibility(GONE);
        moveDown.setVisibility(GONE);
        moveUp.setVisibility(GONE);
    }

    private void initGB02Freq() {
        seekBar.getLayoutParams().width = (int) SkinUtils.getDimension(R.dimen.x400);
        ivScale.getLayoutParams().height = (int) SkinUtils.getDimension(R.dimen.x400);
        _40 = findViewById(SkinUtils.getId(R.id.feqButton40));
        _63 = findViewById(SkinUtils.getId(R.id.freqButton63));
        _80 = findViewById(SkinUtils.getId(R.id.freqButton80));
        _125 = findViewById(SkinUtils.getId(R.id.freqButton125));
        findViewById(SkinUtils.getId(R.id.iv_bass_top_bg)).setVisibility(VISIBLE);
        findViewById(SkinUtils.getId(R.id.rl_freq)).setVisibility(VISIBLE);
        _40.setSelected(true);
        View.OnClickListener listener = v -> {
            _40.setSelected(false);
            _63.setSelected(false);
            _80.setSelected(false);
            _125.setSelected(false);
            switch (v.getId()) {
                case R.id.feqButton40:
                    _40.setSelected(true);
                    freqCallback.setFreqValue(0);
                    break;
                case R.id.freqButton63:
                    _63.setSelected(true);
                    freqCallback.setFreqValue(1);
                    break;
                case R.id.freqButton80:
                    _80.setSelected(true);
                    freqCallback.setFreqValue(2);
                    break;
                case R.id.freqButton125:
                    _125.setSelected(true);
                    freqCallback.setFreqValue(3);
                    break;
            }

        };
        _40.setOnClickListener(listener);
        _63.setOnClickListener(listener);
        _80.setOnClickListener(listener);
        _125.setOnClickListener(listener);
        ConstraintLayout.LayoutParams params = (LayoutParams) sbr_band.getLayoutParams();
        params.setMarginEnd(120);
        sbr_band.setLayoutParams(params);
    }

    public interface FreqCallback {
        void setFreqValue(int value);
    }

    public void setFreqCallback(FreqCallback freqCallback) {
        this.freqCallback = freqCallback;
    }

    public void initFreqStatus(int value){
        switch (value) {
            case 0:
                _40.performClick();
                break;
            case 1:
                _63.performClick();
                break;
            case 2:
                _80.performClick();
                break;
            case 3:
                _125.performClick();
                break;
            default:
                break;
        }
    }
}