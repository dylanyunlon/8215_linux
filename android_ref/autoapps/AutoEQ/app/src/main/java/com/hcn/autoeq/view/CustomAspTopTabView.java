package com.hcn.autoeq.view;

import static com.hcn.autoeq.util.ConstantCscAsp.EXT_CSC_ASP_REVERB_SIZE;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.constraintlayout.widget.ConstraintLayout;

import com.hcn.autoeq.R;
import com.hcn.autoeq.adapter.CscAspGalleryAdapter;
import com.hcn.autoeq.data.CscAspEqualizerChartSettings;
import com.hcn.autoeq.util.ConstantCscAsp;
import com.hcn.autoeq.util.EqUtils;
import com.hcn.autoeq.util.ScalePageTransformer;
import com.hcn.autoeq.util.SkinUtils;
import com.hcn.common.misc.LogUtils;
import com.hcn.skin.support.resources.SkinCompatResources;
import com.hcn.skin2.Skin2;

import java.util.ArrayList;
import java.util.List;

/**
 * 专门定制的asp顶部导航栏
 * 起用原因：在某一布局里，多界面使用，并且共用一套资源和全局数据；
 */
public class CustomAspTopTabView extends ConstraintLayout implements View.OnClickListener {

    private static final String TAG = CustomAspTopTabView.class.getSimpleName();

    private Context mContext;

    /**
     * 音量-按钮
     */
    private DrawableCenterRadioButton btnVolumeTabView;

    private int volumeStatus = 0;

    /**
     * 音响模式悬浮框-popupWindow
     */
    private CscAspPopupWindow mVolumePopWindow;
    /**
     * 用户自定义模式图标
     */
    private Drawable userVolumeIconClose;
    private Drawable userVolumeIconOpen;


    /**
     * 音频场景模式-按钮
     */
    private DrawableCenterRadioButton btnAspBandReverbMode;
    private LinearLayout llAspBandReverb;
    /**
     * 场景模式回廊-自定义view(继承是ViewPage)
     */
    CscAspGalleryViewPager gvpAspBandReverb;
    /**
     * 场景模式回廊适配器
     */
    CscAspGalleryAdapter gvpAspBandReverbAdapter;


    /**
     * 音频情景用户自定义模式-按钮
     */
    private DrawableCenterRadioButton btnAspUserMode;
    /**
     * 用户自定义模式悬浮框-popupWindow
     */
    private CscAspPopupWindow mUserReverbPopWindow;

    private int cscAspReverbMode = 0;

    /**
     * 用户自定义模式图标
     */
    Drawable userReverbIconClose;
    Drawable userReverbIconOpen;

    /**
     * asp顶部导航栏组
     */
    RadioGroup rgHcnAspBandTopTab;


    /**
     * 音频效果设定类
     */
    private CscAspEqualizerChartSettings cscAspEqualizerChartSettings;

    private OnCscAspTopTabViewListener onCscAspTopTabViewListener;

    public CustomAspTopTabView(@NonNull Context context) {
        super(context);
        mContext = context;
        initView();
    }

    public CustomAspTopTabView(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
        initView();
    }


    public void initView() {
        Log.i(TAG, "initView: "+mContext);
        cscAspEqualizerChartSettings = CscAspEqualizerChartSettings.getInstance(mContext);
        View view = LayoutInflater.from(mContext).inflate(SkinUtils.getId(R.layout.csc_asp_top_tab_view), this);
        SkinCompatResources.getInstance().checkIfNeedBuildLayoutParams(view, SkinUtils.getId(R.layout.csc_asp_top_tab_view));

        rgHcnAspBandTopTab = findViewById(SkinUtils.getId(R.id.rg_csc_asp_band_top_tab));
        //响度
        initAspVolumePopupWindow();
        //响度按钮
        initAspVolumeView();

        //音频场景模式
        initReverbModeView();

        //音频用户自定义弹窗
        initUserModePopupWindow();
        //音频自定义模式
        initUserModeView();
    }


    @Override
    protected void onVisibilityChanged(@NonNull View changedView, int visibility) {
        super.onVisibilityChanged(changedView, visibility);
        initViewStatus();

    }

    /**
     * 音频默认重置
     */
    private void initAspVolumePopupWindow() {
        ConstraintLayout popup_view = (ConstraintLayout) LayoutInflater.from(SkinCompatResources.getInstance().getSkinResId(R.layout.csc_asp_band_volume_popup, "layout") != 0
                        ? SkinUtils.getContext() : mContext)
                .inflate(SkinUtils.getId(R.layout.csc_asp_band_volume_popup), null);
        mVolumePopWindow = new CscAspPopupWindow(popup_view, LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, true);
        RadioGroup rgAspBandVolume = popup_view.findViewById(SkinUtils.getId(R.id.rg_asp_band_volume));
        RadioButton btn1 = popup_view.findViewById(SkinUtils.getId(R.id.rb_csc_asp_volume_off));
        RadioButton btn2 = popup_view.findViewById(SkinUtils.getId(R.id.rb_csc_asp_volume_low));
        RadioButton btn3 = popup_view.findViewById(SkinUtils.getId(R.id.rb_csc_asp_volume_middle));
        RadioButton btn4 = popup_view.findViewById(SkinUtils.getId(R.id.rb_csc_asp_volume_high));
        btn1.setOnClickListener(this);
        btn2.setOnClickListener(this);
        btn3.setOnClickListener(this);
        btn4.setOnClickListener(this);
        mVolumePopWindow.setOnDspPopupListener(new CscAspPopupWindow.OnCscAspopupListener() {
            @Override
            public void UpdatePopupContent() {
                rgAspBandVolume.clearCheck();
                int loudness = cscAspEqualizerChartSettings.getCscAspLoudness();
                switch (loudness) {
                    case ConstantCscAsp.CSC_ASP_DEFAULT_LOUDNESS:
                        rgAspBandVolume.check(SkinUtils.getId(R.id.rb_csc_asp_volume_off));
                        break;
                    case ConstantCscAsp.CSC_ASP_LOW_LOUDNESS:
                        rgAspBandVolume.check(SkinUtils.getId(R.id.rb_csc_asp_volume_low));
                        break;
                    case ConstantCscAsp.CSC_ASP_MIDDLE_LOUDNESS:
                        rgAspBandVolume.check(SkinUtils.getId(R.id.rb_csc_asp_volume_middle));
                        break;
                    case ConstantCscAsp.CSC_ASP_HIGH_LOUDNESS:
                        rgAspBandVolume.check(SkinUtils.getId(R.id.rb_csc_asp_volume_high));
                        break;
                    default:
                        rgAspBandVolume.check(SkinUtils.getId(R.id.rb_csc_asp_volume_off));
                        break;
                }
            }

            @Override
            public void openOrCloseListener(boolean isOpenStatus) {
                refreshBtnVolumeStatus(isOpenStatus);
            }
        });

    }

    private void initAspVolumeView() {
        btnVolumeTabView = findViewById(SkinUtils.getId(R.id.btn_csc_asp_volume));
        if(btnVolumeTabView != null){
            btnVolumeTabView.setSelected(true);
            btnVolumeTabView.setOnClickListener(this);
        }

        userVolumeIconOpen = SkinUtils.getDrawable(R.drawable.csc_asp_icon_open);
        userVolumeIconClose = SkinUtils.getDrawable(R.drawable.csc_asp_icon_close);
        refreshBtnVolumeStatus(false);
    }

    /**
     * 音频场景模式
     */
    @SuppressLint("ClickableViewAccessibility")
    private void initReverbModeView() {
        //场景模式回廊
        llAspBandReverb = findViewById(SkinUtils.getId(R.id.ll_asp_band_reverb));
        llAspBandReverb.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                return gvpAspBandReverb.dispatchTouchEvent(event);
            }
        });
        //场景模式按钮
        btnAspBandReverbMode = findViewById(SkinUtils.getId(R.id.btn_csc_asp_reverb_mode));
        btnAspBandReverbMode.setOnClickListener(this);
        if (EqUtils.HEQ_CSC_ASP_HIDE_SCENE) {
            btnAspBandReverbMode.setVisibility(View.INVISIBLE);
        }

        //情景回廊
        gvpAspBandReverb = findViewById(SkinUtils.getId(R.id.gvp_asp_band_reverb));
        // 设置每项的间距
        gvpAspBandReverb.setPageMargin(5);
        // 设置缩放和移动动画
        gvpAspBandReverb.setPageTransformer(true, new ScalePageTransformer());

        /*
         设置需要缓存的数量，最好>=集合数量
         多缓存一倍，可以防止快速滑动时，来不及刷新导致界面异常的问题
          */
        String[] reverbList =  SkinUtils.getStringArray(R.array.csc_asp_band_reverb);
        List<Drawable> drawableList = getReverbResource();
        if(reverbList != null){
            gvpAspBandReverb.setOffscreenPageLimit(reverbList.length * 2);
        }else{
            LogUtils.vTag(TAG,"get revert list fail!");
        }


        //情景回廊适配器
        gvpAspBandReverbAdapter = new CscAspGalleryAdapter(mContext, drawableList, reverbList);
        gvpAspBandReverb.setOnItemClickListener(new CscAspGalleryViewPager.OnItemClickListener() {
            @Override
            public void onItemClick(View view, int position) {
                int mode = position % (reverbList.length);
                chooseReverbMode(mode);
                llAspBandReverb.setVisibility(View.GONE);
            }

            @Override
            public void onItemInvalidClick() {
                highGallery();
            }

            @Override
            public void showViewPager() {
                int initPosition = (Short.MAX_VALUE / 2 - 1) - (Short.MAX_VALUE / 2 - 1) % reverbList.length + cscAspEqualizerChartSettings.getReverb();
                gvpAspBandReverb.setCurrentItem(initPosition, false);
            }

            @Override
            public void closeViewPager() {
                highGallery();
            }
        });
        gvpAspBandReverb.setAdapter(gvpAspBandReverbAdapter);

        int reverb = cscAspEqualizerChartSettings.getReverb();
        int initPosition = (Short.MAX_VALUE / 2 - 1) - (Short.MAX_VALUE / 2 - 1) % reverbList.length + cscAspEqualizerChartSettings.getReverb();
        if (reverb < EXT_CSC_ASP_REVERB_SIZE) {
            cscAspEqualizerChartSettings.saveReverb(initPosition % reverbList.length);
        }
        gvpAspBandReverb.setCurrentItem(initPosition); // 默认选中中间（必须在 setAdapter 之后）
    }

    /**
     * 音频用户自定义弹窗
     */
    public void initUserModePopupWindow() {
        View popup_view = LayoutInflater.from(SkinCompatResources.getInstance().getSkinResId(R.layout.csc_asp_band_user_reverb_popup, "layout") != 0
                        ? SkinUtils.getContext() : mContext)
                .inflate(SkinUtils.getId(R.layout.csc_asp_band_user_reverb_popup), null);
        mUserReverbPopWindow = new CscAspPopupWindow(popup_view, LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, true);
        RadioGroup rgBand_user = popup_view.findViewById(SkinUtils.getId(R.id.rg_asp_band_user));
        RadioButton btn1 = popup_view.findViewById(SkinUtils.getId(R.id.rb_csc_asp_band_user0));
        RadioButton btn2 = popup_view.findViewById(SkinUtils.getId(R.id.rb_csc_asp_band_user1));
        ImageButton btn1Reset = popup_view.findViewById(SkinUtils.getId(R.id.rb_csc_asp_icon_user0_reset));
        ImageButton btn2Reset = popup_view.findViewById(SkinUtils.getId(R.id.rb_csc_asp_icon_user1_reset));
        btn1Reset.setOnClickListener(this);
        btn2Reset.setOnClickListener(this);
        btn1.setOnClickListener(this);
        btn2.setOnClickListener(this);

        mUserReverbPopWindow.setOnDspPopupListener(new CscAspPopupWindow.OnCscAspopupListener() {
            @Override
            public void UpdatePopupContent() {
                int reverb = cscAspEqualizerChartSettings.getReverb();
                switch (reverb) {
                    case ConstantCscAsp.EXT_CSC_ASP_REVERB_USER0:
                        rgBand_user.check(SkinUtils.getId(R.id.rb_csc_asp_band_user0));
                        break;
                    case ConstantCscAsp.EXT_CSC_ASP_REVERB_USER1:
                        rgBand_user.check(SkinUtils.getId(R.id.rb_csc_asp_band_user1));
                        break;
                    default:
                        rgBand_user.clearCheck();
                        break;
                }
            }

            @Override
            public void openOrCloseListener(boolean isOpenStatus) {
                refreshBtnUserIconStatus(isOpenStatus);
                if (!isOpenStatus) {
                    initViewStatus();
                }
            }
        });
    }

    //

    /**
     * 音频用户自定义模式
     */
    private void initUserModeView() {
        btnAspUserMode = findViewById(SkinUtils.getId(R.id.btn_csc_asp_user_reverb_mode));
        if(btnAspUserMode != null){
            btnAspUserMode.setSelected(true);
            btnAspUserMode.setOnClickListener(this);
        }
        userReverbIconOpen = SkinUtils.getDrawable(R.drawable.csc_asp_icon_open);
        userReverbIconClose = SkinUtils.getDrawable(R.drawable.csc_asp_icon_close);
        refreshBtnUserIconStatus(false);
    }


    /**
     * 获取本地存储的场景模式图片资源
     */
    public List<Drawable> getReverbResource() {
        List<Drawable> drawableList = new ArrayList<>();
        drawableList.add(SkinUtils.getDrawable(R.drawable.csc_asp_com_hcn_eq_type_soft_n));
        drawableList.add(SkinUtils.getDrawable(R.drawable.csc_asp_com_hcn_eq_type_jazz_n));
        drawableList.add(SkinUtils.getDrawable(R.drawable.csc_asp_com_hcn_eq_type_pop_n));
        drawableList.add(SkinUtils.getDrawable(R.drawable.csc_asp_com_hcn_eq_type_electronic_n));
        drawableList.add(SkinUtils.getDrawable(R.drawable.csc_asp_com_hcn_eq_type_classic_n));
        drawableList.add(SkinUtils.getDrawable(R.drawable.csc_asp_com_hcn_eq_type_voice_n));
        drawableList.add(SkinUtils.getDrawable(R.drawable.csc_asp_com_hcn_eq_type_rock_n));
        return drawableList;
    }

    /**
     * 刷新响度标志图标
     */
    public void refreshBtnVolumeStatus(boolean isOpenStatus) {
        if (userVolumeIconClose == null) {
            userVolumeIconClose = SkinUtils.getDrawable(R.drawable.csc_asp_icon_close);
        }
        if (userVolumeIconOpen == null) {
            userVolumeIconOpen = SkinUtils.getDrawable(R.drawable.csc_asp_icon_open);
        }
        btnVolumeTabView.setCompoundDrawablesWithIntrinsicBounds(null, null, isOpenStatus ? userVolumeIconOpen : userVolumeIconClose, null);
    }


    /**
     * 刷新自定义模式标志图标
     */
    public void refreshBtnUserIconStatus(boolean isOpenStatus) {
        if (userReverbIconClose == null) {
            userReverbIconClose = SkinUtils.getDrawable(R.drawable.csc_asp_icon_close);
        }
        if (userReverbIconOpen == null) {
            userReverbIconOpen = SkinUtils.getDrawable(R.drawable.csc_asp_icon_open);
        }
        btnAspUserMode.setCompoundDrawablesWithIntrinsicBounds(null, null, isOpenStatus ? userReverbIconOpen : userReverbIconClose, null);
    }


    @Override
    public void onClick(View view) {
        int id = view.getId();
        if (id == SkinUtils.getId(R.id.btn_csc_asp_volume)) {
            if (mVolumePopWindow != null) {
                mVolumePopWindow.showAsDropDown(view.findViewById(SkinUtils.getId(R.id.btn_csc_asp_volume))
                        , SkinUtils.getInteger(R.integer.tab_btn_csc_asp_volume_x)
                        , SkinUtils.getInteger(R.integer.tab_btn_csc_asp_volume_y),
                        Gravity.BOTTOM);
            }
            initViewStatus();
        } else if (id == SkinUtils.getId(R.id.rb_csc_asp_volume_off)) {
            volumeStatus = ConstantCscAsp.CSC_ASP_DEFAULT_LOUDNESS;
            chooseLoudness();
            mVolumePopWindow.close(mVolumePopWindow);
        } else if (id == SkinUtils.getId(R.id.rb_csc_asp_volume_low)) {
            volumeStatus = ConstantCscAsp.CSC_ASP_LOW_LOUDNESS;
            chooseLoudness();
            mVolumePopWindow.close(mVolumePopWindow);
        } else if (id == SkinUtils.getId(R.id.rb_csc_asp_volume_middle)) {
            volumeStatus = ConstantCscAsp.CSC_ASP_MIDDLE_LOUDNESS;
            chooseLoudness();
            mVolumePopWindow.close(mVolumePopWindow);
        } else if (id == SkinUtils.getId(R.id.rb_csc_asp_volume_high)) {
            volumeStatus = ConstantCscAsp.CSC_ASP_HIGH_LOUDNESS;
            chooseLoudness();
            mVolumePopWindow.close(mVolumePopWindow);
        } else if (id == SkinUtils.getId(R.id.btn_csc_asp_reverb_mode)) {
            showGallery();
        } else if (id == SkinUtils.getId(R.id.btn_csc_asp_user_reverb_mode)) {//显示popWindow
            if (mUserReverbPopWindow != null) {
                mUserReverbPopWindow.showAsDropDown(view.findViewById(SkinUtils.getId(R.id.btn_csc_asp_user_reverb_mode))
                        , SkinUtils.getInteger(R.integer.tab_band_user_reverb_mode_x)
                        , SkinUtils.getInteger(R.integer.tab_band_user_reverb_mode_y),
                        Gravity.BOTTOM);
            }
        } else if (id == SkinUtils.getId(R.id.rb_csc_asp_band_user0)) {
            cscAspReverbMode = ConstantCscAsp.EXT_CSC_ASP_REVERB_USER0;
            UserReverbPopWindowChooseReverbMode(cscAspReverbMode);
            mUserReverbPopWindow.close(mUserReverbPopWindow);
        } else if (id == SkinUtils.getId(R.id.rb_csc_asp_band_user1)) {
            cscAspReverbMode = ConstantCscAsp.EXT_CSC_ASP_REVERB_USER1;
            UserReverbPopWindowChooseReverbMode(cscAspReverbMode);
            mUserReverbPopWindow.close(mUserReverbPopWindow);
        } else if (id == SkinUtils.getId(R.id.rb_csc_asp_icon_user0_reset)) {
            int mode = cscAspEqualizerChartSettings.getReverb();
            if (mode != ConstantCscAsp.EXT_CSC_ASP_REVERB_USER0) {
                return;
            }
            //显示重置弹窗
            onCscAspTopTabViewListener.showUserResetDialog(ConstantCscAsp.EXT_CSC_ASP_REVERB_USER0);
        } else if (id == SkinUtils.getId(R.id.rb_csc_asp_icon_user1_reset)) {
            int mode = cscAspEqualizerChartSettings.getReverb();
            if (mode != ConstantCscAsp.EXT_CSC_ASP_REVERB_USER1) {
                return;
            }
            //显示重置弹窗
            onCscAspTopTabViewListener.showUserResetDialog(ConstantCscAsp.EXT_CSC_ASP_REVERB_USER1);
        }

    }

    /**
     * 隐藏画廊
     */
    public void highGallery() {
        llAspBandReverb.setVisibility(View.GONE);
        initViewStatus();
    }

    /**
     * 展示画廊
     */
    public void showGallery() {     //获取效果
        llAspBandReverb.setVisibility(View.VISIBLE);

    }

    public void initViewStatus() {
        //判断场景模式
        int reverb = cscAspEqualizerChartSettings.getReverb();
        //获取数据，刷新View
        refreshBtnTopReverbActivityStatus(reverb);
        refreshBtnReverbModeStatus(reverb);
        refreshBtnUserStatus(reverb);
        refreshBtnLoudnessStatus();
    }

    /**
     * 刷新顶部音频效果按钮激活状态
     */
    private void refreshBtnTopReverbActivityStatus(int reverb) {
        rgHcnAspBandTopTab.clearCheck();
        switch (reverb) {
            case ConstantCscAsp.EXT_CSC_ASP_REVERB_NEWS:
            case ConstantCscAsp.EXT_CSC_ASP_REVERB_JAZZ:
            case ConstantCscAsp.EXT_CSC_ASP_REVERB_CITY:
            case ConstantCscAsp.EXT_CSC_ASP_REVERB_POP:
            case ConstantCscAsp.EXT_CSC_ASP_REVERB_CLASSIZ:
            case ConstantCscAsp.EXT_CSC_ASP_REVERB_MOVIE:
            case ConstantCscAsp.EXT_DSP_REVERB_ROCK:
                rgHcnAspBandTopTab.check(SkinUtils.getId(R.id.btn_csc_asp_reverb_mode));
                break;
            case ConstantCscAsp.EXT_CSC_ASP_REVERB_USER0:
            case ConstantCscAsp.EXT_CSC_ASP_REVERB_USER1:
                rgHcnAspBandTopTab.check(SkinUtils.getId(R.id.btn_csc_asp_user_reverb_mode));
                break;
            default:
                break;
        }
    }

    /**
     * 刷新场景模式状态
     */
    private void refreshBtnReverbModeStatus(int reverb) {
        if (reverb < EXT_CSC_ASP_REVERB_SIZE) {
            String[] reverbList = SkinUtils.getStringArray(R.array.csc_asp_band_reverb);
            if(reverbList != null){
                String reverbMode = SkinUtils.getText(R.string.extdsp_band_reverb_mode_title) + ":"
                        + reverbList[reverb];
                btnAspBandReverbMode.setText(reverbMode);
            }else {
                LogUtils.vTag(TAG,"get revert list fail!");
            }
        } else {
            btnAspBandReverbMode.setText(SkinUtils.getText(R.string.tab_btn_csc_asp_reverb_mode));
        }
    }

    /**
     * 刷新用户自定义场景模式状态
     */
    private void refreshBtnUserStatus(int reverb) {
        switch (reverb) {
            case ConstantCscAsp.EXT_CSC_ASP_REVERB_USER0:
                btnAspUserMode.setText(SkinUtils.getText(R.string.tab_rb_csc_asp_band_user0));
                break;
            case ConstantCscAsp.EXT_CSC_ASP_REVERB_USER1:
                btnAspUserMode.setText(SkinUtils.getText(R.string.tab_rb_csc_asp_band_user1));
                break;
            default:
                btnAspUserMode.setText(SkinUtils.getText(R.string.tab_rb_csc_asp_band_user0));
                break;
        }
    }

    /**
     * 刷新响度状态
     */
    private void refreshBtnLoudnessStatus() {
        int loudness = cscAspEqualizerChartSettings.getCscAspLoudness();
        switch (loudness) {
            case ConstantCscAsp.CSC_ASP_DEFAULT_LOUDNESS:
                btnVolumeTabView.setText(SkinUtils.getText(R.string.tab_rb_csc_asp_volume_off));
                break;
            case ConstantCscAsp.CSC_ASP_LOW_LOUDNESS:
                btnVolumeTabView.setText(SkinUtils.getText(R.string.tab_rb_csc_asp_volume_low));
                break;
            case ConstantCscAsp.CSC_ASP_MIDDLE_LOUDNESS:
                btnVolumeTabView.setText(SkinUtils.getText(R.string.tab_rb_csc_asp_volume_middle));
                break;
            case ConstantCscAsp.CSC_ASP_HIGH_LOUDNESS:
                btnVolumeTabView.setText(SkinUtils.getText(R.string.tab_rb_csc_asp_volume_high));
                break;
            default:
                btnVolumeTabView.setText(SkinUtils.getText(R.string.tab_rb_csc_asp_volume_off));
                break;
        }
    }

    /**
     * 选择响度
     */
    public void chooseLoudness() {
        cscAspEqualizerChartSettings.saveCscAspLoudness(volumeStatus);
        initViewStatus();
    }

    /**
     * 选择模式
     */
    public void chooseReverbMode(int mode) {
        cscAspEqualizerChartSettings.saveReverb(mode);
        cscAspEqualizerChartSettings.nativeBand(mode);
        initViewStatus();
        onCscAspTopTabViewListener.updateModeContent();
    }

    public void UserReverbPopWindowChooseReverbMode(int mode) {
        cscAspEqualizerChartSettings.saveReverb(mode);
        cscAspEqualizerChartSettings.nativeBand(mode);
        initViewStatus();
        onCscAspTopTabViewListener.updateModeContent();
        mUserReverbPopWindow.UpdatePopupWindow();
    }
    public void setOnDspPopupListener(OnCscAspTopTabViewListener listener) {
        this.onCscAspTopTabViewListener = listener;
    }

    /**
     * tab栏监听器
     */
    public interface OnCscAspTopTabViewListener {
        //更新mode内容
        void updateModeContent();

        void showUserResetDialog(int mode);
    }
}
