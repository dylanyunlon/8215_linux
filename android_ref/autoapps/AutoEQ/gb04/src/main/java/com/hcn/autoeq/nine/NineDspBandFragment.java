package com.hcn.autoeq.nine;

import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.GridView;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListAdapter;
import android.widget.PopupWindow;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.ToggleButton;

import androidx.constraintlayout.widget.ConstraintLayout;

import com.auto.hequalizer.EqualizerSurface;
import com.auto.hequalizer.OnCenterFreqChangedListener;
import com.auto.hequalizer.UIMode;
import com.hcn.autoeq.MainActivity;
import com.hcn.autoeq.R;
import com.hcn.autoeq.adapter.NineEQGridViewAdapter;
import com.hcn.autoeq.view.NineBandSeekBar;
import com.hcn.autoeq.view.NineCustomHorizontalScrollView;
import com.hcn_library.BaseFragment;
import com.hcn_library.data.NineDspBandSettings;
import com.hcn_library.util.BlurTask;
import com.hcn_library.util.EqUtils;
import com.hcn_library.util.FastBlurUtils;
import com.hcn_library.util.NineConstantExtDsp;
import com.hcn_library.util.SkinUtils;

import java.lang.reflect.Field;
import java.math.BigDecimal;
import java.text.DecimalFormat;
import java.util.concurrent.atomic.AtomicInteger;


public class NineDspBandFragment extends BaseFragment implements NineConstantExtDsp, SeekBar.OnSeekBarChangeListener, View.OnClickListener, View.OnTouchListener, CompoundButton.OnCheckedChangeListener
        , NineCustomHorizontalScrollView.ParentInterceptListener {
    private static final String TAG = "NineDspBandFragment";
    public static boolean isEditing = false;
    private int[][] bandValue;
    private int[][] bandValueBack;
    private ImageView btnEdit;
    private Button btnLoudness;
    private ToggleButton btnSurround;
    private ConstraintLayout clEdit;
    private ConstraintLayout clMain;
    private PopupWindow customPopupWindow;
    private String[] dataList;
    private GridView gridViewInPopup;
    private ImageView ivEditAbleDrawer;
    private ImageView ivSurfaceBg;
    private ImageView ivSurfaceEditBg;
    private LinearLayout llLoudnessPop;
    private LinearLayout llSurround;
    private Handler loudnessHandler;
    private String[] loudnessList;
    private EqualizerSurface mEqualizerSurface;
    private EqualizerSurface mEqualizerSurfaceEdit;
    private View mainView;
    private NineDspBandSettings nineDspBandSettings;
    private Handler reverbHandler;
    private Runnable runnable;
    private Runnable runnableReverb;
    private float startY;
    private View touchView;
    private Button triggerButton;
    private TextView tvApply;
    private TextView tvEditName;
    private TextView tvExit;
    private TextView tvHigh;
    private TextView tvLow;
    private TextView tvMedium;
    private TextView tvOff;
    private TextView tvReset;
    private LinearLayout llSeekBar = null;
    private LinearLayout llBarTextEdit = null;
    private LinearLayout llBarText = null;
    private HorizontalScrollView scrollView;
    private HorizontalScrollView scrollViewEdit;
    private int[][] freqValue = null;
    private int collapsedSelectedBackground = R.drawable.nine_loudness_expand_item_bg_selector;
    private int expandedSelectedBackground = R.drawable.nine_loudness_fold_item_bg_selector;
    private NineDspUserModeExitDialog dialog = new NineDspUserModeExitDialog();
    private int[] reverbBackgroundDrawables = {R.drawable.nine_scene_default, R.drawable.nine_scene_user, R.drawable.nine_scene_user, R.drawable.nine_scene_user
            , R.drawable.nine_scene_classical, R.drawable.nine_scene_news, R.drawable.nine_scene_popular, R.drawable.nine_scene_city
            , R.drawable.nine_scene_cinema, R.drawable.nine_scene_electronic, R.drawable.nine_scene_rock, R.drawable.nine_scene_high_tech, R.drawable.nine_scene_jazz};
    private View[] views; //views = new View[]{llSurround, gridViewInPopup, btnLoudness, btnEdit, triggerButton, llLoudnessPop, clEdit, ivSurfaceBg};
    private boolean[] ifBlur;
    private boolean[] ifWindow = new boolean[]{false, true, false, false, false, false, false, false};
    private int[] blurRadii = {12, 12, 12, 12, 12, 12, 12, 2};
    private int[] sampleSizes = {8, 8, 8, 8, 8, 8, 8, 8};
    private int[] drawableIds = {R.drawable.nine_eq_3d_btn_bg, R.drawable.nine_eq_pop_window_bg, collapsedSelectedBackground, R.drawable.nine_eq_edit_btn_bg_selector, R.drawable.nine_eq_expand_item_bg_arrow_up, R.drawable.nine_eq_loudness_window_bg, R.drawable.nine_main_radio_group_bg_eq, R.drawable.nine_eq_surface_bg};
    private float[] cornerRadii = {SkinUtils.getDimension(R.dimen.x12), SkinUtils.getDimension(R.dimen.x12), SkinUtils.getDimension(R.dimen.x12), SkinUtils.getDimension(R.dimen.x12), SkinUtils.getDimension(R.dimen.x12), SkinUtils.getDimension(R.dimen.x12), SkinUtils.getDimension(R.dimen.x24), 0};


    @Override
    public int getLayoutRes() {
        return R.layout.nine_dsp_fragment_band;
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
    }

    public static NineDspBandFragment newInstance() {
        return new NineDspBandFragment();
    }

    @Override
    public View onCreateView(LayoutInflater layoutInflater, ViewGroup viewGroup, Bundle bundle) {
        mainView = super.onCreateView(layoutInflater, viewGroup, bundle);
        nineDspBandSettings = NineDspBandSettings.getInstance(mContext);
        return mainView;
    }
    private Handler handler = new Handler();
    private boolean isScrollbarDrawableChanged = false;
    private Runnable scrollStoppedRunnable = new Runnable() {
        @Override
        public void run() {
            // 这里可以处理滚动停止后的逻辑
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                Log.d(TAG, "Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q");
                scrollView.setHorizontalScrollbarThumbDrawable(SkinUtils.getDrawable(R.drawable.nine_band_scrollview_bar));
                scrollView.invalidate();
                isScrollbarDrawableChanged = false;
            }
        }
    };
    private boolean isScrollbarEditDrawableChanged = false;
    private Runnable scrollEditStoppedRunnable = new Runnable() {
        @Override
        public void run() {
            // 这里可以处理滚动停止后的逻辑
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                Log.d(TAG, "change edit thumb drawable, stay");
                scrollViewEdit.setHorizontalScrollbarThumbDrawable(SkinUtils.getDrawable(R.drawable.nine_band_scrollview_bar));
                scrollViewEdit.invalidate();
                isScrollbarEditDrawableChanged = false;
            }
        }
    };
    private Runnable checkLayoutRunnable = new Runnable() {
        @Override
        public void run() {
            // 打印每个控件的宽高信息
            Log.d("LayoutInfo", "llSurround - Width: " + llSurround.getMeasuredWidth() + ", Height: " + llSurround.getMeasuredHeight());
            Log.d("LayoutInfo", "gridViewInPopup - Width: " + gridViewInPopup.getMeasuredWidth() + ", Height: " + gridViewInPopup.getMeasuredHeight());
            Log.d("LayoutInfo", "btnLoudness - Width: " + btnLoudness.getMeasuredWidth() + ", Height: " + btnLoudness.getMeasuredHeight());
            Log.d("LayoutInfo", "btnEdit - Width: " + btnEdit.getMeasuredWidth() + ", Height: " + btnEdit.getMeasuredHeight());
            Log.d("LayoutInfo", "llLoudnessPop - Width: " + llLoudnessPop.getMeasuredWidth() + ", Height: " + llLoudnessPop.getMeasuredHeight());
            Log.d("LayoutInfo", "clEdit - Width: " + clEdit.getMeasuredWidth() + ", Height: " + clEdit.getMeasuredHeight());
            Log.d("LayoutInfo", "ivSurfaceBg - Width: " + ivSurfaceBg.getMeasuredWidth() + ", Height: " + ivSurfaceBg.getMeasuredHeight());
            if (llSurround.getMeasuredWidth() > 0 && llSurround.getMeasuredHeight() > 0 && btnLoudness.getMeasuredWidth() > 0 && btnLoudness.getMeasuredHeight() > 0
                    && btnEdit.getMeasuredWidth() > 0 && btnEdit.getMeasuredHeight() > 0 && llLoudnessPop.getMeasuredWidth() > 0 && llLoudnessPop.getMeasuredHeight() > 0
                    && ivSurfaceBg.getMeasuredWidth() > 0 && ivSurfaceBg.getMeasuredHeight() > 0) {
                ifBlur = new boolean[]{true, false, true, true, false, true, false, true};
                FastBlurUtils.applyGlassEffectOptimized(ifBlur, views, blurRadii, sampleSizes, ifWindow, cornerRadii, drawableIds, reverbBackgroundDrawables[nineDspBandSettings.getReverb()], triggerButton, mContext);
            } else {
                // 延迟 100ms 后再次尝试
                new Handler(Looper.getMainLooper()).postDelayed(this, 100);
            }
        }
    };

    @Override
    public void onViewCreated(View view, Bundle bundle) {
        super.onViewCreated(view, bundle);
        Log.d(TAG, "onViewCreated width: " + view.getWidth() + " height: " + view.getHeight());
        // 控制滑动进度条样式，不滑动的时候置灰
        scrollView.getViewTreeObserver().addOnScrollChangedListener(new ViewTreeObserver.OnScrollChangedListener() {
            @Override
            public void onScrollChanged() {
                Log.d(TAG, "onScrollChanged()");
                // 每次滚动时，移除之前的延迟任务
                handler.removeCallbacks(scrollStoppedRunnable);
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                    if (!isScrollbarDrawableChanged) {
                        Log.d(TAG, "Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q");
                        scrollView.setHorizontalScrollbarThumbDrawable(SkinUtils.getDrawable(R.drawable.nine_band_scrollview_bar_moving));
                        scrollView.invalidate();
                        isScrollbarDrawableChanged = true;
                    }
                }
                // 延迟 300 毫秒后检查是否停止滚动
                handler.postDelayed(scrollStoppedRunnable, 300);
            }
        });

        scrollViewEdit.getViewTreeObserver().addOnScrollChangedListener(new ViewTreeObserver.OnScrollChangedListener() {
            @Override
            public void onScrollChanged() {
                Log.d(TAG, "onScrollChanged()");
                // 每次滚动时，移除之前的延迟任务
                handler.removeCallbacks(scrollEditStoppedRunnable);
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                    if (!isScrollbarEditDrawableChanged) {
                        Log.d(TAG, "change edit thumb drawable, moving");
                        scrollViewEdit.setHorizontalScrollbarThumbDrawable(SkinUtils.getDrawable(R.drawable.nine_band_scrollview_bar_moving));
                        scrollViewEdit.invalidate();
                        isScrollbarEditDrawableChanged = true;
                    }
                }
                // 延迟 300 毫秒后检查是否停止滚动
                handler.postDelayed(scrollEditStoppedRunnable, 300);
            }
        });

        // 当布局确认，获取到的高宽不为0时，开始控件背景的高斯模糊
        checkLayoutRunnable.run();
        // 浮窗和编译页面此时不可见，需要监听后处理
        clEdit.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
                int width = clEdit.getWidth();
                int height = clEdit.getHeight();
                if (width > 0 && height > 0) {
                    clEdit.getViewTreeObserver().removeOnGlobalLayoutListener(this);
                    ifBlur = new boolean[]{false, false, false, false, false, false, true, false}; // 初始化只模糊编辑页面
                    FastBlurUtils.applyGlassEffectOptimized(ifBlur, views, blurRadii, sampleSizes, ifWindow, cornerRadii, drawableIds, reverbBackgroundDrawables[nineDspBandSettings.getReverb()], triggerButton, mContext);
                }
                Log.d(TAG, "onGlobalLayout clEdit  width: " + width + "  height: " + height);
            }
        });
        gridViewInPopup.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
                int width = gridViewInPopup.getWidth();
                int height = gridViewInPopup.getHeight();
                if (width > 0 && height > 0) {
                    gridViewInPopup.getViewTreeObserver().removeOnGlobalLayoutListener(this);
                    ifBlur = new boolean[]{false, true, false, false, false, false, false, false}; // 初始化只模糊浮窗
                    FastBlurUtils.applyGlassEffectOptimized(ifBlur, views, blurRadii, sampleSizes, ifWindow, cornerRadii, drawableIds, reverbBackgroundDrawables[nineDspBandSettings.getReverb()], triggerButton, mContext);
                }
                Log.d(TAG, "onGlobalLayout gridViewInPopup  width: " + width + "  height: " + height);
            }
        });
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void initView() {
        tvOff = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_loudness_off));
        tvLow = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_loudness_low));
        tvMedium = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_loudness_medium));
        tvHigh = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_loudness_high));
        llLoudnessPop = (LinearLayout) mainView.findViewById(SkinUtils.getId(R.id.ll_loudness_pop));
        llSurround = (LinearLayout) mainView.findViewById(SkinUtils.getId(R.id.ll_btn));
        ivEditAbleDrawer = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_drawer_up));
        ivSurfaceBg = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_surface_bg));
        ivSurfaceEditBg = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_surface_edit_bg));
        clMain = (ConstraintLayout) mainView.findViewById(SkinUtils.getId(R.id.cl_nine_eq_main));
        clEdit = (ConstraintLayout) mainView.findViewById(SkinUtils.getId(R.id.cl_nine_eq_edit));
        triggerButton = (Button) mainView.findViewById(SkinUtils.getId(R.id.btn_eq));
        btnLoudness = (Button) mainView.findViewById(SkinUtils.getId(R.id.btn_loudness));
        btnSurround = (ToggleButton) mainView.findViewById(SkinUtils.getId(R.id.btn_surround));
        btnEdit = (ImageView) mainView.findViewById(SkinUtils.getId(R.id.iv_edit));
        tvEditName = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_eq_edit_name));
        tvReset = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_eq_edit_reset));
        tvApply = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_eq_edit_apply));
        tvExit = (TextView) mainView.findViewById(SkinUtils.getId(R.id.tv_eq_edit_exit));
        touchView = mainView.findViewById(SkinUtils.getId(R.id.edit_touch_view));
        scrollView = mainView.findViewById(SkinUtils.getId(R.id.hsv_band));
        scrollViewEdit = mainView.findViewById(SkinUtils.getId(R.id.hsv_band_edit));
        btnSurround.setOnCheckedChangeListener(this);
        btnEdit.setOnClickListener(this);
        tvReset.setOnClickListener(this);
        tvApply.setOnClickListener(this);
        tvExit.setOnClickListener(this);
        btnLoudness.setOnClickListener(this);
        tvOff.setOnTouchListener(this);
        tvLow.setOnTouchListener(this);
        tvMedium.setOnTouchListener(this);
        tvHigh.setOnTouchListener(this);
        clMain.setOnClickListener(this);
        dataList = SkinUtils.getStringArray(R.array.nine_dsp_band_reverb); // 初始化混响模式数据 标准 + 3用户 + 9预设
        loudnessList = SkinUtils.getStringArray(R.array.nine_dsp_spinner_loudness); // 初始化等响度数据
        final NineEQGridViewAdapter nineEQGridViewAdapter = new NineEQGridViewAdapter(mContext, dataList);
        // 初始化混响模型弹窗
        customPopupWindow = new PopupWindow();
        customPopupWindow.setContentView(LayoutInflater.from(mContext).inflate(R.layout.nine_dsp_eq_grid_layout, (ViewGroup) null));
        AtomicInteger atomicInteger = new AtomicInteger(triggerButton.getWidth());
        if (atomicInteger.get() > 0) {
            customPopupWindow.setWidth(atomicInteger.get() * 2);
        } else {
            customPopupWindow.setWidth((int) SkinUtils.getDimension(R.dimen.x480));
        }
        customPopupWindow.setHeight(ViewGroup.LayoutParams.WRAP_CONTENT);
        customPopupWindow.setHeight(ViewGroup.LayoutParams.WRAP_CONTENT);
        customPopupWindow.setOutsideTouchable(true);
        customPopupWindow.setFocusable(true);
        customPopupWindow.setOnDismissListener(new PopupWindow.OnDismissListener() {
            @Override
            public void onDismiss() {
                triggerButton.setBackground(SkinUtils.getDrawable(R.drawable.nine_eq_expand_item_bg_arrow_down));
                reverbHandler.removeCallbacks(runnableReverb);
                Log.d(TAG, "dismiss customPopupWindow  width: " + customPopupWindow.getWidth() + "  height: " + customPopupWindow.getHeight());
            }
        });
        gridViewInPopup = (GridView) customPopupWindow.getContentView().findViewById(R.id.grid_view_in_popup);
        gridViewInPopup.setAdapter((ListAdapter) nineEQGridViewAdapter); // 设置适配器
        // 混响模式悬浮窗的触发按钮
        triggerButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                // 混响按钮背景改变，需高斯模糊处理
                ifBlur = new boolean[]{false, false, false, false, true, false, false, false};
                FastBlurUtils.applyGlassEffectOptimized(ifBlur, views, blurRadii, sampleSizes, ifWindow, cornerRadii, drawableIds, reverbBackgroundDrawables[nineDspBandSettings.getReverb()], triggerButton, mContext);
                nineEQGridViewAdapter.setSelectedItem(nineDspBandSettings.getReverb());
                customPopupWindow.showAsDropDown(view, 0, 0, Gravity.BOTTOM);
                Log.d(TAG, "show customPopupWindow  width: " + customPopupWindow.getWidth() + "  height: " + customPopupWindow.getHeight());
                reverbHandler.removeCallbacks(runnableReverb);
                reverbHandler.postDelayed(runnableReverb, 5000L);
                Log.d(TAG, "triggerButton.onClick");
            }
        });
        // 实现点击跟随手指的效果，如果点击了item，则设置item选中状态
        gridViewInPopup.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    int position = gridViewInPopup.pointToPosition((int) event.getX(), (int) event.getY());
                    if (position != AdapterView.INVALID_POSITION) {
                        setGridItemStatus(nineEQGridViewAdapter, position);
                    }
                }
                return false;
            }
        });
        LinearLayout linearLayoutEdit = (LinearLayout) mainView.findViewById(SkinUtils.getId(R.id.ll_band_edit));
        LinearLayout linearLayout = (LinearLayout) mainView.findViewById(SkinUtils.getId(R.id.ll_band));
        int bandTotal = EqUtils.getBandTotal();
        if (bandTotal == EqUtils.BAND_TOTAL_14) {
            llSeekBar = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_seekbar_14);
            llBarTextEdit = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_text_14_edit);
            llBarText = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_text_14);
            freqValue = NineConstantExtDsp.DEF_EQ_14_FREQ_VALUES;
        } else if (bandTotal == EqUtils.BAND_TOTAL_16) {
            llSeekBar = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_seekbar_16);
            llBarTextEdit = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_text_16_edit);
            llBarText = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_text_16);
            freqValue = NineConstantExtDsp.DEF_EQ_16_FREQ_VALUES;
        } else if (bandTotal == EqUtils.BAND_TOTAL_32) {
            llSeekBar = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_seekbar_32);
            llBarTextEdit = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_text_32_edit);
            llBarText = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_text_32);
            freqValue = NineConstantExtDsp.DEF_EQ_32_FREQ_VALUES;
        } else if (bandTotal == EqUtils.BAND_TOTAL_36) {
            llSeekBar = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_seekbar_36);
            llBarTextEdit = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_text_36_edit);
            llBarText = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_text_36);
            freqValue = NineConstantExtDsp.DEF_EQ_36_FREQ_VALUES;
        } else if (bandTotal == EqUtils.BAND_TOTAL_48) {
            llSeekBar = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_seekbar_48);
            llBarTextEdit = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_text_48_edit);
            llBarText = (LinearLayout) SkinUtils.inflate(R.layout.nine_dsp_fragment_band_text_48);
            freqValue = NineConstantExtDsp.DEF_EQ_48_FREQ_VALUES;
        }
        // 遍历 LinearLayout 中的所有子视图
        for (int index = 0; index < bandTotal; index++) {
            // 读取数字，进行单位转换
            int netValue = freqValue[index][2];
            String strValue = "";
            if (netValue >= 1000) {
                float number = netValue * 1f / 1000;
                BigDecimal bd = new BigDecimal(number);
                bd = bd.setScale(1, BigDecimal.ROUND_HALF_UP);
                strValue = bd + " kHz";
            } else {
                strValue = netValue + " Hz";
            }
            // 展示区频段值文案
            View childEdit = llBarTextEdit.getChildAt(index);
            if (childEdit instanceof TextView) {
                TextView textViewEdit = (TextView) childEdit;
                String tag = (String) textViewEdit.getTag();
                if (tag != null) {
                    textViewEdit.setText(strValue);
                }
            }
            // 编辑区频段值文案
            View child = llBarText.getChildAt(index);
            if (child instanceof TextView) {
                TextView textView = (TextView) child;
                String tag = (String) textView.getTag();
                if (tag != null) {
                    textView.setText(strValue);
                }
            }
        }
        linearLayoutEdit.addView(llBarTextEdit);
        linearLayoutEdit.addView(llSeekBar);
        linearLayout.addView(llBarText);

        for (int i = 0; i < llSeekBar.getChildCount(); i++) {
            View child = llSeekBar.getChildAt(i);
            if (child instanceof NineBandSeekBar) {
                NineBandSeekBar bandSeekBarView = (NineBandSeekBar) child;
                bandSeekBarView.setCallback(new NineBandSeekBar.QValueCallback() {
                    @Override
                    public void onDialogQValueChanged(int position, int qValue) {
                        for (int i = 0; i < bandValue[1].length; i++) {
                            if (i == position) {
                                bandValue[1][position] = qValue;
                                break;
                            }
                        }
                    }
                });

            }
        }
        ((NineCustomHorizontalScrollView) mainView.findViewById(SkinUtils.getId(R.id.hsv_band))).setParentInterceptListener(this);
        // 编辑模式下，下拉收起编辑框的触摸监听
        touchView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                Log.d(TAG, "onTouch " + motionEvent);
                int action = motionEvent.getAction();
                if (action == MotionEvent.ACTION_DOWN) {
                    startY = motionEvent.getY();
                    return true;
                }
                if (action != MotionEvent.ACTION_MOVE) {
                    return true;
                }
                float y = motionEvent.getY();
                if (Math.abs(y - startY) <= 100.0f || y <= startY || dialog.isShow) {
                    return true;
                }
                judgeShowExitDialog(2);
                return true;
            }
        });
        if (EqUtils.isChip7739()) {
            llSurround.setVisibility(View.GONE);
        }
        views = new View[]{llSurround, gridViewInPopup, btnLoudness, btnEdit, triggerButton, llLoudnessPop, clEdit, ivSurfaceBg};
    }

    public void setGridItemStatus(NineEQGridViewAdapter nineEQGridViewAdapter, int i) {
        nineEQGridViewAdapter.setSelectedItem(i);
        triggerButton.setText(dataList[i]);
        nineDspBandSettings.saveReverb(i);
        refreshReverbBg();
        refreshData();
        boolean canEdit = i < NineConstantExtDsp.NINE_DSP_REVERB_PREVIEW_START_INDEX && i > NineConstantExtDsp.NINE_DSP_REVERB_STANDARD;
        btnEdit.setEnabled(canEdit);
        ivEditAbleDrawer.setVisibility(canEdit ? View.VISIBLE : View.GONE);
        reverbHandler.removeCallbacks(runnableReverb);
        reverbHandler.postDelayed(runnableReverb, 2000L);
    }

    private void refreshData() {
        bandValue = nineDspBandSettings.getUserBandValue(nineDspBandSettings.getReverb());
        refreshEqualizerSurfaceView();
        refreshEqualizerSurfaceViewEdit();
        refreshBandSeekBarStatus();
    }

    private void refreshEqualizerSurfaceView() {
        for (int mUiBand = 0; mUiBand < bandValue[0].length; mUiBand++) {
            mEqualizerSurface.setBand(mUiBand, bandValue[0][mUiBand], false);
        }
        equalizerEnabled();
    }

    private void refreshEqualizerSurfaceViewEdit() {
        for (int mUiBand = 0; mUiBand < bandValue[0].length; mUiBand++) {
            mEqualizerSurfaceEdit.setBand(mUiBand, bandValue[0][mUiBand], false);
        }
        equalizerEnabled();
    }

    @Override
    public void initData() {
        super.initData();
        int reverb = nineDspBandSettings.getReverb();
        boolean canEdit = reverb < NineConstantExtDsp.NINE_DSP_REVERB_PREVIEW_START_INDEX && reverb > NineConstantExtDsp.NINE_DSP_REVERB_STANDARD;
        bandValue = nineDspBandSettings.getUserBandValue(reverb);
        initEqualizerSurfaceView();
        initEqualizerSurfaceViewEdit();
        refreshBandSeekBarStatus();
        btnLoudness.setText(loudnessList[nineDspBandSettings.getLoudness()]);
        btnSurround.setChecked(nineDspBandSettings.getSurround() == 1);
        btnEdit.setEnabled(canEdit);
        triggerButton.setText(dataList[reverb]);
        tvEditName.setText(dataList[reverb]);
        nineDspBandSettings.setCurrentCustomReverbName(dataList[reverb]);
        ivEditAbleDrawer.setVisibility(canEdit ? View.VISIBLE : View.GONE);
        loudnessHandler = new Handler();
        runnable = new Runnable() {
            @Override
            public void run() {
                llLoudnessPop.setVisibility(View.INVISIBLE);
                ifBlur = new boolean[]{false, false, true, false, false, false, false, false}; // 等响按钮背景改变，需高斯模糊处理
                drawableIds[2] = collapsedSelectedBackground;
                FastBlurUtils.applyGlassEffectOptimized(ifBlur, views, blurRadii, sampleSizes, ifWindow, cornerRadii, drawableIds, reverbBackgroundDrawables[nineDspBandSettings.getReverb()], triggerButton, mContext);
            }
        };
        reverbHandler = new Handler();
        runnableReverb = new Runnable() {
            @Override
            public void run() {
                customPopupWindow.dismiss();
            }
        };
    }

    public void initEqualizerSurfaceView() {
        int bandTotal = EqUtils.getBandTotal();
        mEqualizerSurface = (EqualizerSurface) mainView.findViewById(SkinUtils.getId(R.id.es_freq));
        try {
            // 获取指定类的声明类数组
            Class<?>[] declaredClasses = Class.forName("com.auto.hequalizer.EqualizerSurface").getDeclaredClasses();

            // 用于存储最终找到的类
            Class<?> cls = null;
            // 遍历声明类数组，查找名为 RConfig 的类
            for (Class<?> clazz : declaredClasses) {
                if (clazz.getSimpleName().equals("RConfig")) {
                    cls = clazz;
                    break;
                }
            }
            if (cls != null) {
                // 获取 RConfig 类中的 mControlBarShadowColor 字段
                Field declaredField = cls.getDeclaredField("mControlBarShadowColor");
                // 设置该字段为可访问
                declaredField.setAccessible(true);
                // 将 mControlBarShadowColor 字段的值设置为 0
                declaredField.set(null, 0);
                // 获取该字段的值
                Object obj = declaredField.get(null);
                // 打印修改后的属性值
                Log.d(TAG, "修改后的属性值为: " + obj);
            }
        } catch (ClassNotFoundException | NoSuchFieldException | IllegalAccessException e) {
            e.printStackTrace();
        }
        mEqualizerSurface.invalidate();
        if (mEqualizerSurface != null) {
            if (EqUtils.isRtL(getContext())) {
                mEqualizerSurface.setScaleX(-1.0f);
            } else {
                mEqualizerSurface.setScaleX(1.0f);
            }
        }
        equalizerEnabled();
        Log.d(TAG, "mEqualizerSurface.getBackground() before" + mEqualizerSurface.getBackground().getAlpha());
        mEqualizerSurface.initConfig("background_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("gridlines_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("gain_text_size", 0);
        mEqualizerSurface.initConfig("gain_text_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("bar_width", 0);
        mEqualizerSurface.initConfig("bar_text_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("bar_text_shadow_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("bar_text_size", 0);
        mEqualizerSurface.initConfig("bar_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("bar_shadow_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("heq_bar_knob_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("heq_bar_knob_shadow_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("heq_bar_shader_color_1", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("heq_bar_shader_color_2", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("freq_curve_color_1", SkinUtils.getColor(R.color.nine_curve_line_color));
        mEqualizerSurface.initConfig("freq_curve_color_2", SkinUtils.getColor(R.color.nine_curve_line_color));
        mEqualizerSurface.initConfig("freq_curve_width_1", (int) SkinUtils.getDimension(R.dimen.x4));
        mEqualizerSurface.initConfig("freq_curve_width_2", (int) SkinUtils.getDimension(R.dimen.x4));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_1", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_2", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_3", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_4", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("freq_curve_bg_shader_color_5", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("shell_Color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurface.initConfig("padding_top", (int) SkinUtils.getDimension(R.dimen.y20));
        mEqualizerSurface.initConfig("padding_bottom", (int) SkinUtils.getDimension(R.dimen.y20));
        Log.d(TAG, "mEqualizerSurface.getBackground() after" + mEqualizerSurface.getBackground().getAlpha());
        // 配置增益区间
        mEqualizerSurface.setGainRange(EqUtils.getDspGainMax() / 2 * -1, EqUtils.getDspGainMax() / 2);
        // 配置中心频段个数
        mEqualizerSurface.setCenterFreqBands(bandTotal);
        // [测试数据]

        for (int i = 0; i < bandTotal; i++) {
            mEqualizerSurface.setCenterFreqBandsValue(i, freqValue[i][2]);
        }

        // 限制图表的增益值为整数
        mEqualizerSurface.setGainRound(true);
        // 配置曲线的光滑度, 也就是曲率, 取整范围: 0.0 ~ 0.3
        mEqualizerSurface.setCurveSmoothness(0.18f);
        // 配置均衡器曲线的显示模式
        mEqualizerSurface.setEqualizerUIMode(UIMode.BEZIER_EQUIDISTANT);
        // 初始化配置
        mEqualizerSurface.initialize();
        for (int mUiBand = 0; mUiBand < bandValue[0].length; mUiBand++) {
            mEqualizerSurface.setBand(mUiBand, bandValue[0][mUiBand], false);
        }
        mEqualizerSurface.setZOrderOnTop(true);
    }

    public void initEqualizerSurfaceViewEdit() {
        int bandTotal = EqUtils.getBandTotal();
        mEqualizerSurface = mainView.findViewById(SkinUtils.getId(R.id.es_freq));
        mEqualizerSurfaceEdit = mainView.findViewById(SkinUtils.getId(R.id.es_freq_edit));

        if (mEqualizerSurfaceEdit != null) {
            if (EqUtils.isRtL(getContext())) {
                mEqualizerSurfaceEdit.setScaleX(-1);
            } else {
                mEqualizerSurfaceEdit.setScaleX(1);
            }
        }

        // 可编辑区域-------------------------
        // 判定配置能拖动
        equalizerEnabled();
        Log.d(TAG, "mEqualizerSurfaceEdit.getBackground() before" + mEqualizerSurfaceEdit.getBackground());
        mEqualizerSurfaceEdit.initConfig("background_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("gridlines_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("gain_text_size", (int) 0);
        mEqualizerSurfaceEdit.initConfig("gain_text_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("bar_width", (int) 0);
        mEqualizerSurfaceEdit.initConfig("bar_text_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("bar_text_shadow_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("bar_text_size", (int) 0);
        mEqualizerSurfaceEdit.initConfig("bar_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("bar_shadow_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("heq_bar_knob_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("heq_bar_knob_shadow_color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("heq_bar_shader_color_1", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("heq_bar_shader_color_2", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("freq_curve_color_1", SkinUtils.getColor(R.color.nine_curve_line_color));
        mEqualizerSurfaceEdit.initConfig("freq_curve_color_2", SkinUtils.getColor(R.color.nine_curve_line_color));
        mEqualizerSurfaceEdit.initConfig("freq_curve_width_1", (int) SkinUtils.getDimension(R.dimen.x4));
        mEqualizerSurfaceEdit.initConfig("freq_curve_width_2", (int) SkinUtils.getDimension(R.dimen.x4));
        mEqualizerSurfaceEdit.initConfig("freq_curve_bg_shader_color_1", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("freq_curve_bg_shader_color_2", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("freq_curve_bg_shader_color_3", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("freq_curve_bg_shader_color_4", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("freq_curve_bg_shader_color_5", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("shell_Color", SkinUtils.getColor(R.color.color_transparent));
        mEqualizerSurfaceEdit.initConfig("padding_top", (int) SkinUtils.getDimension(R.dimen.y35));
        mEqualizerSurfaceEdit.initConfig("padding_bottom", (int) SkinUtils.getDimension(R.dimen.y35));
        Log.d(TAG, "mEqualizerSurfaceEdit.getBackground() after" + mEqualizerSurfaceEdit.getBackground().getAlpha());

        // 配置增益区间
        mEqualizerSurfaceEdit.setGainRange(EqUtils.getDspGainMax() / 2 * -1, EqUtils.getDspGainMax() / 2);
        // 配置中心频段个数
        mEqualizerSurfaceEdit.setCenterFreqBands(bandTotal);
        // [测试数据]

        for (int i = 0; i < bandTotal; i++) {
            mEqualizerSurfaceEdit.setCenterFreqBandsValue(i, freqValue[i][2]);
        }

        // 限制图表的增益值为整数
        mEqualizerSurfaceEdit.setGainRound(true);
        // 配置曲线的光滑度, 也就是曲率, 取整范围: 0.0 ~ 0.3
        mEqualizerSurfaceEdit.setCurveSmoothness(0.18f);
        // 配置均衡器曲线的显示模式
        mEqualizerSurfaceEdit.setEqualizerUIMode(UIMode.BEZIER_EQUIDISTANT);
        // 初始化配置
        mEqualizerSurfaceEdit.initialize();
        for (int mUiBand = 0; mUiBand < bandValue[0].length; mUiBand++) {
            mEqualizerSurfaceEdit.setBand(mUiBand, bandValue[0][mUiBand], false);
        }

        // 监听回调用来设置 DSP
        mEqualizerSurfaceEdit.setCenterFreqChangedListener(new OnCenterFreqChangedListener() {
            @Override
            public void OnCenterFreqChanged(int index, double gain, boolean touch) {
                String szGain = (gain < 0 ? "" : "+") + (float) (Math.round(gain * 10) / 10.0f);
                Log.d(TAG, "center freq change: ["
                        + new DecimalFormat("00").format(index) + " & " + szGain + "]"
                        + " - [" + touch + "]");
                if (touch) {
                    int[] _gainValue = bandValue[0];
                    int[] _qValue = bandValue[1];
                    int _bandIndex = index;
                    _gainValue[_bandIndex] = (int) gain;

                    refreshSingleBandSeekBarStatus(index);

                    //需要获取最新的处理数据，然后再针对相应位置进行数据处理；
                    int _Gain = nineDspBandSettings.getDealBandValue(_gainValue, index);
                    int _Q = nineDspBandSettings.getDealBandValue(_qValue, index);
                    int _Index = nineDspBandSettings.getDealBandIndex(_gainValue, _bandIndex);
                    nineDspBandSettings.nativeBand(_Index, _Gain, _Q);
                }

            }
        }, 100);
        mEqualizerSurfaceEdit.setZOrderOnTop(true);
    }

    public void equalizerEnabled() {
        if (mEqualizerSurface != null) {
            mEqualizerSurface.setEnabled(false);
        }
        if (mEqualizerSurfaceEdit != null) {
            mEqualizerSurfaceEdit.setEnabled(true);
        }
    }

    private void refreshSingleBandSeekBarStatus(int index) {
        int reverb = nineDspBandSettings.getReverb();
        int[] _gainValue = bandValue[0];
        int[] _qValue = bandValue[1];
        boolean enable = reverb > NineConstantExtDsp.NINE_DSP_REVERB_STANDARD && reverb < NineConstantExtDsp.NINE_DSP_REVERB_PREVIEW_START_INDEX;

        View child = llSeekBar.getChildAt(index);
        if (child instanceof NineBandSeekBar) {
            NineBandSeekBar bandSeekBarView = (NineBandSeekBar) child;
            bandSeekBarView.setProgress(_gainValue[index], false);
            bandSeekBarView.setQValue(_qValue[index]);
            bandSeekBarView.setSeekBarStatus(enable);
            if (enable) { // 只有自定义的才能做修改操作
                bandSeekBarView.setOnSeekBarChangeListener(this);
                bandSeekBarView.setFragmentManager(getFragmentManager());
            }
        }
    }

    private void refreshBandSeekBarStatus() {
        int reverb = nineDspBandSettings.getReverb();
        int[][] bandValue = nineDspBandSettings.getUserBandValue(reverb);
        int[] _gainValue = bandValue[0];
        int[] _qValue = bandValue[1];

        for (int i = 0; i < llSeekBar.getChildCount(); i++) {
            View child = llSeekBar.getChildAt(i);
            if (child instanceof NineBandSeekBar) {
                NineBandSeekBar bandSeekBarView = (NineBandSeekBar) child;
                bandSeekBarView.setProgress(_gainValue[i], false);
                bandSeekBarView.setQValue(_qValue[i]);
                if (reverb > NineConstantExtDsp.NINE_DSP_REVERB_STANDARD && reverb < NineConstantExtDsp.NINE_DSP_REVERB_PREVIEW_START_INDEX) { // 只有自定义的才能做修改操作
                    bandSeekBarView.setOnSeekBarChangeListener(this);
                    bandSeekBarView.setFragmentManager(getFragmentManager());
                }
            }
        }
    }


    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            int[] _gainValue = bandValue[0];
            int[] _qValue = bandValue[1];
            int _bandIndex = Integer.parseInt((String) seekBar.getTag());
            _gainValue[_bandIndex] = progress;
            refreshEqualizerSurfaceViewEdit();
            //需要获取最新的处理数据，然后再针对相应位置进行数据处理；
            int _Gain = nineDspBandSettings.getDealBandValue(_gainValue, _bandIndex);
            int _Q = nineDspBandSettings.getDealBandValue(_qValue, _bandIndex);
            int _Index = nineDspBandSettings.getDealBandIndex(_gainValue, _bandIndex);
            nineDspBandSettings.nativeBand(_Index, _Gain, _Q);
        }
        Log.d(TAG, "onProgressChanged, fromUser " + fromUser + ", progress = " + progress + " getProgress: " + seekBar.getProgress());
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == SkinUtils.getId(R.id.iv_edit)) {
            bandValueBack = new int[bandValue.length][bandValue[0].length];
            for (int i = 0; i < bandValue.length; i++) {
                for (int j = 0; j < bandValue[i].length; j++) {
                    bandValueBack[i][j] = bandValue[i][j];
                }
            }
            clMain.setVisibility(View.GONE);
            clEdit.setVisibility(View.VISIBLE);
            touchView.setVisibility(View.VISIBLE);
            tvEditName.setText(dataList[nineDspBandSettings.getReverb()]);
            nineDspBandSettings.setCurrentCustomReverbName(dataList[nineDspBandSettings.getReverb()]);
            isEditing = true;
            return;
        }
        if (view.getId() == SkinUtils.getId(R.id.tv_eq_edit_exit)) {
            judgeShowExitDialog(1);
            return;
        }
        if (view.getId() == SkinUtils.getId(R.id.tv_eq_edit_apply)) {
            applyBandEdit();
            return;
        }
        if (view.getId() == SkinUtils.getId(R.id.tv_eq_edit_reset)) {
            nineDspBandSettings.resetUserBand(bandValue);
            nineDspBandSettings.saveBandValue(nineDspBandSettings.getUserBandValue(nineDspBandSettings.getReverb()));
            refreshData();
            return;
        }
        if (view.getId() == SkinUtils.getId(R.id.btn_loudness)) {
            llLoudnessPop.setVisibility(View.VISIBLE);
            ifBlur = new boolean[]{false, false, true, false, false, false, false, false}; // 等响按钮背景改变，需高斯模糊处理
            drawableIds[2] = expandedSelectedBackground;
            FastBlurUtils.applyGlassEffectOptimized(ifBlur, views, blurRadii, sampleSizes, ifWindow, cornerRadii, drawableIds, reverbBackgroundDrawables[nineDspBandSettings.getReverb()], triggerButton, mContext);
            int selectedIndex = nineDspBandSettings.getLoudness();
            tvOff.setSelected(selectedIndex == 0);
            tvLow.setSelected(selectedIndex == 1);
            tvMedium.setSelected(selectedIndex == 2);
            tvHigh.setSelected(selectedIndex == 3);
            loudnessHandler.removeCallbacks(runnable);
            loudnessHandler.postDelayed(runnable, 3000L);
            return;
        }
        if (view.getId() == SkinUtils.getId(R.id.cl_nine_eq_main)) {
            loudnessHandler.removeCallbacks(runnable);
            btnLoudness.setText(loudnessList[nineDspBandSettings.getLoudness()]);
            llLoudnessPop.setVisibility(View.INVISIBLE);
            ifBlur = new boolean[]{false, false, true, false, false, false, false, false}; // 等响按钮背景改变，需高斯模糊处理
            drawableIds[2] = collapsedSelectedBackground;
            FastBlurUtils.applyGlassEffectOptimized(ifBlur, views, blurRadii, sampleSizes, ifWindow, cornerRadii, drawableIds, reverbBackgroundDrawables[nineDspBandSettings.getReverb()], triggerButton, mContext);
        }
    }

    private void setSelectedLoudness(int i) {
        tvOff.setSelected(i == 0);
        tvLow.setSelected(i == 1);
        tvMedium.setSelected(i == 2);
        tvHigh.setSelected(i == 3);
        btnLoudness.setText(loudnessList[i]);
        nineDspBandSettings.saveLoudness(i);
        nineDspBandSettings.nativeLoudness(i);
        loudnessHandler.removeCallbacks(runnable);
        loudnessHandler.postDelayed(runnable, 0L);
    }

    @Override
    public void onCheckedChanged(CompoundButton compoundButton, boolean z) {
        nineDspBandSettings.saveSurround(z ? 1 : 0);
        if (compoundButton.isPressed()) {
            nineDspBandSettings.nativeSurround(z ? 1 : 0);
        }
    }

    @Override
    public void onVerticalScrollDetected() {
        if (isEditing) {
            return;
        }
        boolean editEnable = nineDspBandSettings.getReverb() > NineConstantExtDsp.NINE_DSP_REVERB_STANDARD && nineDspBandSettings.getReverb() < NineConstantExtDsp.NINE_DSP_REVERB_PREVIEW_START_INDEX;
        Log.d(TAG, "nVerticalScrollDetected editEnable: " + editEnable);
        if (editEnable) {
            bandValueBack = new int[bandValue.length][bandValue[0].length];
            for (int i = 0; i < bandValue.length; i++) {
                for (int j = 0; j < bandValue[i].length; j++) {
                    bandValueBack[i][j] = bandValue[i][j];
                }
            }
            clMain.setVisibility(View.GONE);
            clEdit.setVisibility(View.VISIBLE);
            touchView.setVisibility(View.VISIBLE);
            tvEditName.setText(dataList[nineDspBandSettings.getReverb()]);
            nineDspBandSettings.setCurrentCustomReverbName(dataList[nineDspBandSettings.getReverb()]);
            isEditing = true;
        }
    }


    @Override
    public void onDestroy() {
        super.onDestroy();
        loudnessHandler.removeCallbacksAndMessages(null);
        reverbHandler.removeCallbacksAndMessages(null);
        nineDspBandSettings.shutdownExecutorService();
        handler.removeCallbacksAndMessages(null);
    }

    private boolean getIsDataChange() {
        for (int i = 0; i < bandValue.length; i++) {
            int[] subArray1 = bandValue[i];
            int[] subArray2 = bandValueBack[i];
            for (int j = 0; j < subArray1.length; j++) {
                // 比较元素是否相等
                if (subArray1[j] != subArray2[j]) {
                    return true;
                }
            }
        }
        return false;
    }

    public void applyBandEdit() {
        nineDspBandSettings.saveBandValue(bandValue);
        refreshData();
        clMain.setVisibility(View.VISIBLE);
        clEdit.setVisibility(View.GONE);
        touchView.setVisibility(View.GONE);
        isEditing = false;
    }

    public void recycleOldData() {
        Log.d(TAG, "recycleOldData, bandValue: " + (bandValue != null) + " bandValueBack: " + (bandValueBack != null));
        if (bandValueBack != null) {
            // 使用副本回退
            for (int i = 0; i < bandValue.length; i++) {
                for (int j = 0; j < bandValue[i].length; j++) {
                    bandValue[i][j] = bandValueBack[i][j];
                }
            }
        }
        isEditing = false;
        nineDspBandSettings.saveBandValue(bandValue);
        nineDspBandSettings.nativeUserReverbType(); // 恢复本地数据后要再发一次数据给系统
        refreshData();
        clMain.setVisibility(View.VISIBLE);
        clEdit.setVisibility(View.GONE);
        touchView.setVisibility(View.GONE);
    }


    public void judgeShowExitDialog(int i) {
        if (!getIsDataChange()) {
            isEditing = false;
            clMain.setVisibility(View.VISIBLE);
            clEdit.setVisibility(View.GONE);
            touchView.setVisibility(View.GONE);
        } else {
            dialog.setMode(i);
            dialog.setUserName(nineDspBandSettings.getCurrentCustomReverbName());
            dialog.setOnDialogListener(new NineDspUserModeExitDialog.OnDialogListener() {
                @Override
                public void isOk(int i2) {
                    applyBandEdit();
                    Log.d(TAG, "isOK");
                }

                @Override
                public void isCancel(int i2) {
                    recycleOldData();
                    Log.d(TAG, "isCancel");
                }
            });
            dialog.show(getParentFragmentManager(), "");
            dialog.isShow = true;
        }
    }

    // 整体背景切换，高斯模糊效果处理
    private void refreshReverbBg() {
        // 设置主背景图
        if (getActivity() instanceof MainActivity) {
            ((MainActivity) getActivity()).setAppBackground(SkinUtils.getId(R.id.rb_nine_eq));
        }
        ifBlur = new boolean[]{true, true, true, true, true, true, true, true};
        FastBlurUtils.applyGlassEffectOptimized(ifBlur, views, blurRadii, sampleSizes, ifWindow, cornerRadii, drawableIds, reverbBackgroundDrawables[nineDspBandSettings.getReverb()], triggerButton, mContext);
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            int i = v.getId();
            if (i == SkinUtils.getId(R.id.tv_loudness_off)) {
                setSelectedLoudness(0);
            } else if (i == SkinUtils.getId(R.id.tv_loudness_low)) {
                setSelectedLoudness(1);
            } else if (i == SkinUtils.getId(R.id.tv_loudness_medium)) {
                setSelectedLoudness(2);
            } else if (i == SkinUtils.getId(R.id.tv_loudness_high)) {
                setSelectedLoudness(3);
            }
        }
        return false;
    }
}