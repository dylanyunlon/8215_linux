package com.hcn.autoradio.ui;

import static android.carsource.McuConstant.K_EQ;
import static android.carsource.McuConstant.K_MUTE;
import static android.media.AudioManager.AUDIOFOCUS_REQUEST_FAILED;
import static com.hcn.autoradio.data.RadioData.BAND_AM_1;
import static com.hcn.autoradio.data.RadioData.BAND_FM_1;
import static com.hcn.autoradio.data.RadioData.BAND_FM_2;
import static com.hcn.autoradio.data.RadioData.BAND_FM_3;
import static com.hcn.autoradio.data.RadioData.BAND_SIZE;
import static com.hcn.autoradio.data.RadioData.E_THEME_GOD;
import static com.hcn.autoradio.data.RadioData.E_THEME_SUB;

import android.Configures.HConfig;
import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.carsource.McuManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.AnimationDrawable;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.sourceservice.ExtAudioMuxer;
import android.text.InputFilter;
import android.text.TextUtils;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.ToggleButton;

import androidx.annotation.NonNull;
import androidx.core.view.GravityCompat;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.hcn.auto_compat.app.Wallpaper;
import com.hcn.autoradio.EDigitFreq;
import com.hcn.autoradio.FMApplication;
import com.hcn.autoradio.FMDragControl;
import com.hcn.autoradio.FMResource;
import com.hcn.autoradio.IDragControlEvent;
import com.hcn.autoradio.IFMCallBack;
import com.hcn.autoradio.IPresetCallBack;
import com.hcn.autoradio.R;
import com.hcn.autoradio.RadioDigitFreq;
import com.hcn.autoradio.RadioIcon;
import com.hcn.autoradio.RadioMain;
import com.hcn.autoradio.ScreenSpec;
import com.hcn.autoradio.audio.RadioAudioManager;
import com.hcn.autoradio.data.FMDataControl;
import com.hcn.autoradio.data.RadioData;
import com.hcn.autoradio.skin.SkinID;
import com.hcn.autoradio.skin.SkinUtils;
import com.hcn.autoradio.skin.ThemeID;
import com.hcn.autoradio.skin.ThemeUtilsEx;
import com.hcn.autoradio.util.FastBlurUtils;
import com.hcn.autoradio.util.LimitedQueue;
import com.hcn.autoradio.util.LogoUtils;
import com.hcn.autoradio.util.RadioUtils;
import com.hcn.autoradio.view.CollectionAdapter;
import com.hcn.autoradio.view.CollectionListAdapter;
import com.hcn.autoradio.view.CustomDrawerLayout;
import com.hcn.autoradio.view.DragContainer;
import com.hcn.autoradio.view.FMFreeLayout;
import com.hcn.autoradio.view.FMPresetView;
import com.hcn.autoradio.view.FMSeekBarListener;
import com.hcn.autoradio.view.FMSeekBarView;
import com.hcn.autoradio.view.KeyBoardDialog;
import com.hcn.autoradio.view.PresetPaperAdapter;
import com.hcn.autoradio.view.PtyAdapter;
import com.hcn.autoradio.view.RoundKnobSeekBar;
import com.hcn.autoradio.view.RoundKnobSeekBarView;
import com.hcn.autoradio.view.ViewPager;
import com.hcn.common.lang.Listenable;
import com.hcn.common.utils.HImageUtils;

import java.util.List;
import java.util.Locale;
import java.util.Objects;

/**
 * 描述：后装UI
 *
 * @author simon
 * @date 2023/1/10 9:45
 */
public class RadioHz extends RadioBaseUI implements IPresetCallBack, View.OnClickListener,
        PresetPaperAdapter.OnPresetPaperAdapterListener {

    private static final String TAG = RadioHz.class.getSimpleName();
    private static final int ALERT_WINDOW_PERMISSION_CODE = 100;
    private static final String VOLUME_CHANGED_ACTION = "android.media.VOLUME_CHANGED_ACTION";
    private static final String EXTRA_VOLUME_STREAM_TYPE = "android.media.EXTRA_VOLUME_STREAM_TYPE";
    private static final String EXTRA_VOLUME_STREAM_VALUE =
            "android.media.EXTRA_VOLUME_STREAM_VALUE";
    // mute changed
    private static final String STREAM_MUTE_CHANGED_ACTION =
            "android.media.STREAM_MUTE_CHANGED_ACTION";
    private static final String EXTRA_STREAM_VOLUME_MUTED =
            "android.media.EXTRA_STREAM_VOLUME_MUTED";
    public static final String SETTINGS_PACKAGE_NAME = "com.android.settings";
    public static final String SETTINGS_ACTIVITY_NAME = "com.android.settings.Settings";
    public static final String EQ_INSIDE = "radio_is_display_eq";
    private FMSeekBarView mFMSeekBar = null;
    private AutoFMSeekBarListener mFMSeekBarListener = null;
    private boolean isHaveIntentExtra = false;

    // TextView ShowInfo (Hide)
    private TextView mTextBand = null;
    private TextView[] mDigitFreqArrary;
    private TextView mTextUnit = null;

    // ImageView ShowInfo
    private ImageView mImageBand = null;
    private ImageView[] mDigitFreqImageArrary;
    private ImageView mImageUnit = null;
    private RadioIcon mRadioIcon = null;
    private TextView mFreqName = null;

    private LinearLayout mDigitFreqLayout = null;

    private FMPresetView[] mPresetItem = null;
    private boolean mPresetViewEnableClick = true;
    private int mCurrCheckedIndex = -1;
    private final PresetOnTouchListener mPresetOnTouchListener = new PresetOnTouchListener();
    private final PresetLongClickListener mPresetOnLongClickListener = new PresetLongClickListener();

    private ViewPager mPresetViewPaper = null;
    private ViewPagerOnPageChangeListener mPageChangeListener = null;
    private PresetPaperAdapter mPaperAdapter = null;

    private View btnLoc = null;
    private ImageView btnMute = null;
    private View btnEq;

    //Collection list
    private View mCollectionBtn = null;
    private ListView mCollectListView = null;
    private CollectionListAdapter mCollectionListAdapter = null;

    private RadioButton[] mScreenFousItem = null;
    private View[] mBandFocusItem = null;//FM1 FM2 FM3 AM1
    private View btnBand;

    private int mCurrUnitStep = 205; // (mCurrFreq - mMinFreq) / mFMStepValue
    //For auto Test
    private int mAutoTestNum = 0;
    private long mBeginTime = 0;

    private FMDataControl mFMDCC = null;
    private UpdateRadioUIListener mUpdateUIListener = null;

    private final OnBottomClickListener mOnBottomClickListener = new OnBottomClickListener();
    private final OnButtLongClickListener mOnButtLongClickListener = new OnButtLongClickListener();

    private OnRdsClickListener mOnRdsClickListener = null;

    private OnKeyBoardClickListener mOnKeyboardClickListener = null;

    private View mXmlLayoutView = null;
    private View mRootLayoutView = null;
    private FMFreeLayout mControlPanel = null;
    private FMDragControl mFmDragControl = null;
    private DragContainer mFmDragCellCan = null;
    private IFMCallBack mIfmCallBack = null;
    private IDragControlEvent mDragControlEvent = null;

    private FMApplication mFmApp = null;

    private KeyBoardDialog mKeyBoard = null;
    // add by dsh Begin RDS
    private String mStrPS = "";//用于比较PS是否有变化
    private PopupWindow popupWindow;
    private PopupWindow keyboardWindow;

    private View mKeyboardPopupView = null;
    private View mBtnKey1,mBtnKey2,mBtnKey3,mBtnKey4,mBtnKey5,mBtnKey6,mBtnKey7,mBtnKey8,mBtnKey9,mBtnKey0,mBtnKeyDel,mBtnKeyDot,mBtnKeyEnter;

    private TextView mTextInput;
    private View mRdsPopupView = null;
    private Context mContext = null;
    private TextView tvPTY, tvPTYMain, tvRDS_PS, tvRDS_RT;
    private TextView tvEdit_rds_ps; //有自定义名称时优先显示自定义名称，没有时显示RDS的PS信息
    private CheckBox cbxAF, cbxTA, cbxTP;
    private CheckBox cbxAF_ani, cbxTA_ani, cbxTP_ani;
    private View rdsImageButton;
    private ToggleButton tglBtn_ta, tglBtn_af, tglBtn_ct;
    private AnimationDrawable RDS_TA_iconTransition, RDS_TP_iconTransition, RDS_AF_iconTransition;

    //mcc400-mnc021特殊定制
    private PopupWindow settingWindow;
    private ToggleButton settingToggleBtnTA, settingToggleBtnAF, settingToggleBtnLOC;
    private ToggleButton buttonTglTA,buttonTglAF;
    private View rdsStatusLayout, settingLocLayout, settingAfLayout, settingTaLayout;
    private OnSettingClickListener mSettingClickListener = null;
    private RoundKnobSeekBarListener roundKnobSeekBarListener = null;
    private RoundKnobSeekBarView roundKnobSeekBarView;
    private CheckBox cbxMainAF, cbxMainTA, cbxMainLoc;
    private TextView tvPtyTitle, tvTpStatus;

    private static final int RDS_AF_ON = 110;
    private static final int RDS_AF_OFF = 111;
    private static final int RDS_AF_DETECTED = 112;
    private static final int RDS_AF_SIGNAL_DISAPPEAR = 113;

    private static final int RDS_TA_ON = 120;
    private static final int RDS_TA_OFF = 121;
    private static final int RDS_TA_DETECTED = 122;
    private static final int RDS_TA_SIGNAL_DISAPPEAR = 123;
    private static final int INIT_DRAG_CELL_POSITION = 124;

    public RadioButton[] mPtyButts = new RadioButton[32];
    private static final int[] PTY_BUTTON_ID = {R.id.id_pty_none, R.id.id_pty_news,
            R.id.id_pty_affairs, R.id.id_pty_info,
            R.id.id_pty_sport, R.id.id_pty_educate,
            R.id.id_pty_drama, R.id.id_pty_culture,
            R.id.id_pty_science, R.id.id_pty_varied,
            R.id.id_pty_popm, R.id.id_pty_rockm,
            R.id.id_pty_easym, R.id.id_pty_lightm,
            R.id.id_pty_classics, R.id.id_pty_otherm,
            R.id.id_pty_weather, R.id.id_pty_finance,
            R.id.id_pty_children, R.id.id_pty_social,
            R.id.id_pty_religion, R.id.id_pty_phone,
            R.id.id_pty_travel, R.id.id_pty_leisure,
            R.id.id_pty_jazz, R.id.id_pty_country,
            R.id.id_pty_nation, R.id.id_pty_oldies,
            R.id.id_pty_folk, R.id.id_pty_document,
            R.id.id_pty_test, R.id.id_pty_alarm,};

    private static final int[] PTY_TYPE_NAME =
            {R.string.pty_type_none, R.string.pty_type_news, R.string.pty_type_affairs,
                    R.string.pty_type_info, R.string.pty_type_sport, R.string.pty_type_education,
                    R.string.pty_type_drama, R.string.pty_type_culture, R.string.pty_type_science,
                    R.string.pty_type_varied, R.string.pty_type_popm, R.string.pty_type_rockm,
                    R.string.pty_type_easym, R.string.pty_type_lightm, R.string.pty_type_classics,
                    R.string.pty_type_otherm, R.string.pty_type_weather, R.string.pty_type_finance,
                    R.string.pty_type_children, R.string.pty_type_social,
                    R.string.pty_type_religion, R.string.pty_type_phonein,
                    R.string.pty_type_travel, R.string.pty_type_leisure,
                    R.string.pty_type_jazz, R.string.pty_type_country, R.string.pty_type_nationm,
                    R.string.pty_type_oldies, R.string.pty_type_folk, R.string.pty_type_document,
                    R.string.pty_type_test, R.string.pty_type_alarm};

    // add by dsh End RDS
    private LimitedQueue<Integer> keyCodeQueue = new LimitedQueue<>(2); // 按键事件的队列
    private static final int MSG_WHAT_GOTO_FREQ = 0;
    private static final int MSG_DELAYED_GOTO_FREQ = 1000; // xxx ms后响应按键事件
    private static final int MSG_UPDATE_COLLECT_LIST = 1;
    private static final int MSG_UPDATE_FREQ_LIST_LOGO = 2;

    private AnimationDrawable mRadioIconAnim;
    private View mainInfoView; //根布局下属一级布局
    private LinearLayout llRadioMainInfo;
    private int radioMainWidth;
    private ImageView ivSideSlide;
    private Drawable layoutDrawable;
    private Bitmap extendBitmap;
    private Bitmap compressBitmap;
    private RadioMain mRadioMain = null;
    /*** 壁纸设置相关 */
    private LinearLayout mLayoutWallpaper = null;
    private Button mWallpaperButton = null;
    private PopupWindow mWallpaperDialog = null;
    private RecyclerView mWallpaperRecyclerView = null;
    private WallpaperAdapter mWallpaperAdapter = null;
    private static final String SIDEBAR_STYLE = "sidebar_Style";
    private View buttFavoriteList;
    private ListView radioCollectionList;
    private TextView favouritesEmpty;
    private LinearLayout llRdsIcon;
    private View buttAmFm;
    private View rdsPty;
    private ImageButton ivRds;
    private int N91_TOP; //n91定制UI顶部按钮是否显示
    private PopupWindow mCollectionDialog = null;
    private CollectionAdapter mCollectionAdapter = null;
    private PopupWindow mPtyDialog;
    private PtyAdapter mPtyAdapter = null;
    private ListView radioPtyList;
    private ImageButton btnReg;
    //白天黑夜模式 0为白天，1为黑夜，2自动
    public static final String auto_setting_day_night_mode= "setting_day_night_mode_choose";
    private Window window;
    private View btnRadioSetting;
    private PopupWindow radioSettingWin;
    private int isRssiChange;
    private View mRadioSettingView = null;
    private OnRadioSettingClickListener mOnRadioSettingClickListener = null;
    private View tvDefault,btnAmDown,btnAmUp,btnFmDown,btnFmUp;
    private TextView tvAmSize,tvFmSize;
    private ToggleButton sensitivitySwitch;

    /*** 壁纸设置相关 */

    public RadioHz(RadioMain radioMain) {
        this.mRadioMain = radioMain;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate:");
        mFmApp = (FMApplication) mRadioMain.getApplication();
        mFmApp.addActivity(mRadioMain);
        // data center control
        mFMDCC = FMDataControl.getInstance();
        mFMDCC.openDataService();

        mRadioMain.requestWindowFeature(Window.FEATURE_NO_TITLE);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            window = mRadioMain.getWindow();
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            window.getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.setStatusBarColor(Color.TRANSPARENT);
        }
        if (SkinUtils.getCurrentSkinID().equals(SkinID.SKIN_N91)) {
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
        }
        Configuration config = mRadioMain.getResources().getConfiguration();
        ScreenSpec.setScreenStatus(config);
        initIntent(mRadioMain.getIntent());
        initCurrentFreq();
        initRadioLayout();
        initRadioMainCtrl();
        initMemberVariable();
        onVolumeChanged(RadioAudioManager.getInstance().getStreamVolume());
        initRdsFunctionStatus();
        initRegStatus();
        initBroadcast();
        initWallpaper();
    }

    private void initView(){
        mRadioIcon = null;
        mIfmCallBack = null;
        initCurrentFreq();
        initRadioLayout();
        initRadioMainCtrl();
        initMemberVariable();
        onVolumeChanged(RadioAudioManager.getInstance().getStreamVolume());
        initRdsFunctionStatus();
        initRegStatus();
        initBroadcast();
        initWallpaper();
        refresh();
        if (mTextBand != null) {
            updateDigitFreqText(mFMDCC.getFormatFreq(mFMDCC.currentFreq(), false));
        } else if (mImageBand != null) {
            updateDigitFreq(mFMDCC.currentFreq());
        }
    }

    private void initWallpaper() {
        mWallpaperButton = mRadioMain.findViewById(SkinUtils.getId(R.id.btn_set_wallpaper));
        if (mWallpaperButton != null) {
            mWallpaperButton.setOnClickListener(this);
            if (RadioUtils.supportWallpaperCustomized()) {
                Wallpaper.instance().initialize();
                Wallpaper.instance().register(mWallpaperListener);
                buildWallpaperDialog();
                mWallpaperButton.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        List<Wallpaper.Info> list = Wallpaper.instance().getInfo();
                        if (Objects.isNull(list) || list.isEmpty()) {
                            //没有壁纸图片时，隐藏该项配置
                            mWallpaperButton.setVisibility(View.GONE);
                        } else {
                            mWallpaperButton.setVisibility(View.VISIBLE);
                        }
                    }
                }, 1000);
            }
        }
    }

    @SuppressLint("HandlerLeak")
    private final Handler processRdsUiHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            // TODO Auto-generated method stub
            super.handleMessage(msg);
            switch (msg.what) {
                case RDS_AF_ON:
                    if (null != cbxAF) {
                        cbxAF.setVisibility(View.GONE);
                    }
                    if (null != cbxAF_ani) {
                        cbxAF_ani.setVisibility(View.VISIBLE);
                    }
                    if (null != RDS_AF_iconTransition) {
                        RDS_AF_iconTransition.start();
                    }
                    if (null != cbxMainAF) {
                        cbxMainAF.setChecked(true);
                    }
                    break;
                case RDS_AF_OFF:
                    if (null != cbxAF) {
                        cbxAF.setVisibility(View.VISIBLE);
                    }
                    if (null != cbxAF_ani) {
                        cbxAF_ani.setVisibility(View.GONE);
                    }
                    if (null != RDS_AF_iconTransition) {
                        RDS_AF_iconTransition.stop();
                    }
                    if (null != cbxMainAF) {
                        cbxMainAF.setChecked(false);
                    }
                    break;
                case RDS_TA_ON:
                    if (null != cbxTA) {
                        cbxTA.setVisibility(View.GONE);
                    }
                    if (null != cbxTA_ani) {
                        cbxTA_ani.setVisibility(View.VISIBLE);
                    }
                    if (null != RDS_TA_iconTransition) {
                        RDS_TA_iconTransition.start();
                    }
                    if (null != cbxMainTA) {
                        cbxMainTA.setChecked(true);
                    }
                    break;
                case RDS_TA_OFF:
                    if (null != cbxTA) {
                        cbxTA.setVisibility(View.VISIBLE);
                    }
                    if (null != cbxTA_ani) {
                        cbxTA_ani.setVisibility(View.GONE);
                    }
                    if (null != cbxTP) {
                        cbxTP.setVisibility(View.VISIBLE);
                    }
                    if (null != RDS_TA_iconTransition) {
                        RDS_TA_iconTransition.stop();
                    }
                    if (null != cbxMainTA) {
                        cbxMainTA.setChecked(false);
                    }
                    break;
                case RDS_AF_DETECTED:
                    if (null != cbxAF) {
                        cbxAF.setVisibility(View.VISIBLE);
                    }
                    if (null != RDS_AF_iconTransition) {
                        RDS_AF_iconTransition.stop();
                    }
                    if (null != cbxAF_ani) {
                        cbxAF_ani.setVisibility(View.GONE);
                    }
                    break;
                case RDS_TA_DETECTED:
                    if (null != cbxTA) {
                        cbxTA.setVisibility(View.VISIBLE);
                    }
                    if (null != RDS_TA_iconTransition) {
                        RDS_TA_iconTransition.stop();
                    }
                    if (null != cbxTA_ani) {
                        cbxTA_ani.setVisibility(View.GONE);
                    }
                    break;
                case RDS_AF_SIGNAL_DISAPPEAR:
                    if (null != cbxAF) {
                        cbxAF.setVisibility(View.GONE);
                    }
                    if (null != cbxAF_ani) {
                        cbxAF_ani.setVisibility(View.VISIBLE);
                    }
                    if (null != RDS_AF_iconTransition) {
                        RDS_AF_iconTransition.start();
                    }
                    break;
                case RDS_TA_SIGNAL_DISAPPEAR:
                    if (null != cbxTA) {
                        cbxTA.setVisibility(View.GONE);
                    }
                    if (null != cbxTA_ani) {
                        cbxTA_ani.setVisibility(View.VISIBLE);
                    }
                    if (null != RDS_TA_iconTransition) {
                        RDS_TA_iconTransition.start();
                    }
                    break;
                case INIT_DRAG_CELL_POSITION:
                    if (!initDragCellPosition()) {
                        processRdsUiHandler.sendEmptyMessageDelayed(INIT_DRAG_CELL_POSITION,
                                500);
                    }
                    break;
                default:
                    break;
            }
        }
    };

    private void initCurrentFreq() {
        if (isHaveIntentExtra) {
            mFMDCC.setFreq(mFMDCC.mCurrentBand, mFMDCC.mCurrentFreq, -1);
        } else {
            mFMDCC.initCurrentFreq();
        }
    }

    private void initRdsFunctionStatus() {
        mFMDCC.mIsTA = mFMDCC.readRadioInfo("TA_Enable", 0x00) == 0x01;
        mFMDCC.mIsAF = mFMDCC.readRadioInfo("AF_Enable", 0x00) == 0x01;
        mFMDCC.setTA(mFMDCC.mIsTA);
        mFMDCC.setAF(mFMDCC.mIsAF);
        if (SkinUtils.getCurrentSkinID().equals(SkinID.SKIN_N91) && buttonTglTA != null && buttonTglAF != null ) {
            buttonTglTA.setChecked(mFMDCC.mIsTA);
            buttonTglAF.setChecked(mFMDCC.mIsAF);
        }
        if (mFMDCC.mIsSupportRDS && ScreenSpec.getScreenStatus() < ScreenSpec.HALF_SCREEN && mFMDCC.isFMBand()) {
            showRdsDetected(true);
        } else {
            HideRDS();
        }
    }

    private void initRegStatus(){
        mFMDCC.mIsReg = mFMDCC.readRadioInfo("Reg_Enable", 0x00) == 0x01;
        Log.d(TAG, "initRegStatus: " + mFMDCC.mIsReg);
        mFMDCC.setReg(mFMDCC.mIsReg);
        if (btnReg != null) {
            btnReg.setSelected(mFMDCC.mIsReg);
        }
    }

    /**
     * @description: 壁纸设置Adapter
     * @since: 2024/1/17 16:18
     * @param:
     * @return:
     */
    private class WallpaperAdapter extends RecyclerView.Adapter<WallpaperAdapter.WallpaperViewHolder> {
        private List<Wallpaper.Info> mData;
        private WallpaperItemClickListener mItemClickListener;

        @Override
        public WallpaperViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(mContext).inflate(SkinUtils.getId(R.layout.wallpaper_listitem), null);
            return new WallpaperViewHolder(view);
        }

        @Override
        public void onBindViewHolder(WallpaperViewHolder holder, int position) {
            //优先缩略图，没有缩略图显示大图
            if (!TextUtils.isEmpty(mData.get(position).thumbnailPath)) {
                Bitmap bitmap = HImageUtils.getBitmap(mData.get(position).thumbnailPath);
                if (Objects.nonNull(bitmap)) {
                    holder.ivWallPaper.setImageBitmap(bitmap);
                    holder.ivWallPaper.setOnClickListener(v -> {
                        mItemClickListener.onClick(v, mData.get(position), position);
                    });

                    //高亮显示选中壁纸
                    if (holder.ivWallpaperBg != null) {
                        if (RadioUtils.getWallpaperSavePath().equals(mData.get(position).wallpaperPath)) {
                            holder.ivWallpaperBg.setVisibility(View.VISIBLE);
                        } else {
                            holder.ivWallpaperBg.setVisibility(View.GONE);
                        }
                    }
                }
            }
        }

        public void setOnItemClickListener(WallpaperItemClickListener listener) {
            this.mItemClickListener = listener;
        }

        public void setData(List<Wallpaper.Info> data) {
            this.mData = data;
        }

        @Override
        public int getItemCount() {
            return mData == null ? 0 : mData.size();
        }

        private class WallpaperViewHolder extends RecyclerView.ViewHolder {
            private ImageView ivWallPaper;
            private ImageView ivWallpaperBg;

            public WallpaperViewHolder(View itemView) {
                super(itemView);
                this.ivWallPaper = itemView.findViewById(SkinUtils.getId(R.id.iv_wallpaper));
                this.ivWallpaperBg = itemView.findViewById(SkinUtils.getId(R.id.iv_wallpaper_bg));
            }
        }
    }

    /**
     * @description: 构建壁纸设置弹窗
     * @since: 2024/1/17 12:21
     * @param:
     * @return:
     */
    private void buildWallpaperDialog() {

        View view = LayoutInflater.from(mContext).inflate(SkinUtils.getId(R.layout.wallpaper_dialog), null);
        if (view != null) {
            mWallpaperDialog = new PopupWindow(view, ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);
            mWallpaperDialog.setOutsideTouchable(true);
            mWallpaperDialog.setFocusable(true);
            mWallpaperDialog.setAnimationStyle(R.style.PopupAnimation);

            mWallpaperRecyclerView = view.findViewById(SkinUtils.getId(R.id.wallpaper_recyclerview));

            GridLayoutManager glManager = new GridLayoutManager(mContext, SkinUtils.getInteger(R.integer.wallpaper_grid_layout_numRows),
                    SkinUtils.getInteger(R.integer.wallpaper_recyclerview_orientation), false);
            mWallpaperRecyclerView.setLayoutManager(glManager);

            mWallpaperAdapter = new WallpaperAdapter();
            mWallpaperAdapter.setOnItemClickListener(mWallpaperItemClickListener);
            mWallpaperRecyclerView.setAdapter(mWallpaperAdapter);
        }
    }


    private void showWallpaperDialog() {
        if (mWallpaperDialog != null) {
            Wallpaper.instance().initialize();
            mWallpaperDialog.showAtLocation(mXmlLayoutView, Gravity.CENTER, 0, 0);
        }
    }

    private void hideWallpaperDialog() {
        if (mWallpaperDialog != null) {
            mWallpaperDialog.dismiss();
        }
    }

    /*** @Des: 壁纸recyclerview Item的点击事件接口*/
    public interface WallpaperItemClickListener {
        void onClick(View view, Wallpaper.Info itemData, int position);
    }

    public WallpaperItemClickListener mWallpaperItemClickListener = new WallpaperItemClickListener() {
        @Override
        public void onClick(View view, Wallpaper.Info itemData, int position) {
            if (Objects.isNull(itemData)) {
                return;
            }

            // 设置页面背景图片
            String path = itemData.wallpaperPath;
            if (TextUtils.isEmpty(path)) {
                return;
            }
            Bitmap bitmap = HImageUtils.getBitmap(path);
            if (!Objects.isNull(bitmap)) {
                //将壁纸路径保存到sp中
                RadioUtils.setWallpaperSavePath(path);
                //刷新RecyclerView列表
                mWallpaperAdapter.notifyDataSetChanged();
                //更新背景图
                Drawable wallPaper = updateWallpaper();
                if (null != wallPaper) {
                    mRootLayoutView.setBackground(wallPaper);
                }
            }
        }
    };

    private final Listenable<String> mWallpaperListener = (s, o) -> {
        // 壁纸状态检查
        if (Objects.equals(s, Wallpaper.ST_COMPLETED)) {
            Log.d(TAG, "Wallpaper.ST_COMPLETED");
            // 壁纸扫描完成
            // 扫描 /apd/appWallpaper/ 目录下是否有壁纸文件
            List<Wallpaper.Info> list = Wallpaper.instance().getInfo();
            if (list == null || list.isEmpty()) {
                Log.d(TAG, "Wallpaper is empty");
                // 没有壁纸文件
                if (mLayoutWallpaper != null) {
                    mLayoutWallpaper.setVisibility(View.GONE);
                }
                hideWallpaperDialog();
                return;
            } else {
                // 有壁纸文件
                // 可以在这里设置壁纸数据到显示 UI 列表中
                Log.d(TAG, "Wallpaper count = " + list.size());
                if (mLayoutWallpaper != null) {
                    mLayoutWallpaper.setVisibility(View.VISIBLE);
                }
                mWallpaperAdapter.setData(list);
                //数据刷新
                mWallpaperAdapter.notifyDataSetChanged();
            }
        }
    };

    public Drawable updateWallpaper() {
        Drawable wallPaper = null;
        if (RadioUtils.supportWallpaperCustomized()) {
            //加载用户设置的壁纸  /apd/appWallpaper/路径的壁纸
            String wallpaperPath = RadioUtils.getWallpaperSavePath();
            if (!TextUtils.isEmpty(wallpaperPath)) {
                Bitmap bitmap = HImageUtils.getBitmap(wallpaperPath);
                if (!Objects.isNull(bitmap)) {
                    wallPaper = new BitmapDrawable(mContext.getResources(), bitmap);
                }
            }
        }
        if (Objects.isNull(wallPaper)) {
            wallPaper = ThemeUtilsEx.getAppShareBackground();
        }
        return wallPaper;
    }

    private void initBroadcast() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction("com.hcn.action.show_settings");
        intentFilter.addAction(VOLUME_CHANGED_ACTION); //volume
        intentFilter.addAction(STREAM_MUTE_CHANGED_ACTION);//mute
        mRadioMain.registerReceiver(mBroadcastReceiver, intentFilter);
    }

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if ("com.hcn.action.show_settings".equals(action)) {
                String packageName = intent.getStringExtra("PACKAGE");
                if (packageName.equals(mRadioMain.getPackageName())) {
                    ShowRDSPopupWindow();
                }
            } else if (VOLUME_CHANGED_ACTION.equals(action)) {
                int streamType = intent.getIntExtra(EXTRA_VOLUME_STREAM_TYPE, -1);
                int streamValue = intent.getIntExtra(EXTRA_VOLUME_STREAM_VALUE, -1);

                if (streamType == AudioManager.STREAM_MUSIC) {
                    onVolumeChanged(streamValue);
                }
            } else if (STREAM_MUTE_CHANGED_ACTION.equals(action)) {
                int streamType = intent.getIntExtra(EXTRA_VOLUME_STREAM_TYPE, -1);
                boolean volumeMuted = intent.getBooleanExtra(EXTRA_STREAM_VOLUME_MUTED, false);

                if (streamType == AudioManager.STREAM_MUSIC) {
                    onVolumeChanged(volumeMuted ? 0
                            : RadioAudioManager.getInstance().getStreamVolume());
                }
            }
        }
    };

    private void initIntent(Intent intent) {
        if (intent == null) {
            return;
        }
        String getIntentStr = intent.getStringExtra("opration");
        //Voice ctrl
        int freqFromVoice = intent.getIntExtra("com.hcn.autoradio.toitem", 0x00);
        int cmdFromVoice = intent.getIntExtra("com.hc.fmradio.event.asps", 0x00);
        if (null != getIntentStr) {
            if ("scan".equals(getIntentStr)) {
                onButtScanEvent();
            } else if ("next".equals(getIntentStr)) {
                onButtSeekDownEvent();
            } else if ("previous".equals(getIntentStr)) {
                onButtSeekUpEvent();
            } else if ("fm".equals(getIntentStr)) {
                if (mFMDCC.getFMPlugService() == null) {
                    if (mFMDCC.currentBand() >= BAND_AM_1) {
                        isHaveIntentExtra = true;
                        mFMDCC.mCurrentBand = BAND_FM_1;
                        mFMDCC.mCurrentFreq = FMDataControl.mRadioParameters.FmMin;
                    }
                } else {
                    if (mFMDCC.currentBand() >= BAND_AM_1) {
                        onButtBandFMEvent();
                    }
                }
            } else if ("am".equals(getIntentStr)) {
                if (mFMDCC.getFMPlugService() == null) {
                    if (mFMDCC.currentBand() != BAND_AM_1) {
                        isHaveIntentExtra = true;
                        mFMDCC.mCurrentBand = BAND_AM_1;
                        mFMDCC.mCurrentFreq = FMDataControl.mRadioParameters.AmMin;
                    }
                } else {
                    if (mFMDCC.currentBand() != BAND_AM_1) {
                        onButtBandAMEvent();
                    }
                }
            }
        } else if (freqFromVoice != 0x00) {
            isHaveIntentExtra = true;
            if (mFMDCC.currentBand() >= BAND_AM_1) {
                mFMDCC.mCurrentBand = BAND_AM_1;
                mFMDCC.mCurrentFreq = freqFromVoice * 100;
                mFMDCC.setFreq(mFMDCC.mCurrentBand, mFMDCC.mCurrentFreq, -1);
            } else {
                mFMDCC.mCurrentFreq = freqFromVoice * 100;
                mFMDCC.setFreq(mFMDCC.mCurrentFreq, -1);
            }
        } else if (cmdFromVoice != 0x00) {
            onButtASEvent();
        }

    }

    @Override
    public void onNewIntent(Intent intent) {
        Log.v(TAG, "onNewIntent");
        initIntent(intent);
    }

    private void initRadioLayout() {
        Log.d(TAG, "initRadioLayout: begin");
        if (SkinUtils.useSkinPackage()) {
            mContext = SkinUtils.getContext();
        } else {
            mContext = mRadioMain.getBaseContext();
        }
        mControlPanel = new FMFreeLayout(mContext);
        mXmlLayoutView = LayoutInflater.from(mContext).inflate(SkinUtils.getId(R.layout.radio_main), mControlPanel);

        mRootLayoutView = mXmlLayoutView.findViewById(SkinUtils.getId(R.id.radio_main));
        if (RadioUtils.supportWallpaperCustomized()) {
            Drawable wallPaper = updateWallpaper();
            if (wallPaper != null) {
                mRootLayoutView.setBackground(wallPaper);
            }
        } else if (mRootLayoutView != null) {
            Drawable wallPaper = ThemeUtilsEx.getAppShareBackground();
            if (wallPaper != null) {
                mRootLayoutView.setBackground(wallPaper);
            }
            layoutDrawable = mRootLayoutView.getBackground();
        }
        mRadioMain.setContentView(mControlPanel);
        Log.d(TAG, "initRadioLayout: end");
    }

    /**
     * za01 初始化DrawLayout相关
     */
    public void initDrawLayout() {
        mainInfoView = mRadioMain.findViewById(SkinUtils.getId("dl_radio_main"));
        if (mainInfoView instanceof CustomDrawerLayout) {
            CustomDrawerLayout drawLayout = (CustomDrawerLayout) mainInfoView;
            drawLayout.addDrawerListener(new DrawerListenerImpl());
            drawLayout.setScrimColor(Color.TRANSPARENT);
            drawLayout.setRightDragEdgeSize(60);
            drawLayout.setLeftDragEdgeSize(60);
            ivSideSlide = mRadioMain.findViewById(SkinUtils.getId("iv_side_slide"));
            if (ivSideSlide != null) {
                ivSideSlide.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if (drawLayout.isDrawerOpen(GravityCompat.END)) {
                            drawLayout.closeDrawers();
                        } else {
                            drawLayout.openDrawer(GravityCompat.END);
                        }
                    }
                });
            }
            llRadioMainInfo = mRadioMain.findViewById(SkinUtils.getId("ll_radio_main_info"));
            if (llRadioMainInfo != null) {
                radioMainWidth = llRadioMainInfo.getLayoutParams().width;
                Log.d(TAG, "initDrawLayout: radioMainWidth=" + radioMainWidth);
            }
        }
    }

    private AdapterView.OnItemClickListener
            mCollectionListClickListener = new AdapterView.OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> adapterView, View arg1, int arg2, long arg3) {
            Log.v(TAG, " onItemClick, arg2 = " + arg2 + " arg3= " + arg3);
            if (mFMDCC != null) {
                String freq = (String) adapterView.getItemAtPosition(arg2);
                mFMDCC.gotoFreq(freq);
            }
        }
    };

    private final class DrawerListenerImpl implements DrawerLayout.DrawerListener {
        @Override
        public void onDrawerSlide(@NonNull View view, float arg1) {
            //当前滑动的菜单
            int menuWidth = view.getMeasuredWidth();
            //计算向左移动的距离
            float moveX = arg1 * menuWidth;

            //设置主视图布局大小
            if (llRadioMainInfo != null) {
                ViewGroup.LayoutParams layoutParams = llRadioMainInfo.getLayoutParams();
                layoutParams.width = radioMainWidth - (int) moveX;
                llRadioMainInfo.setLayoutParams(layoutParams);
            }
        }

        @Override
        public void onDrawerOpened(@NonNull View view) {
            if (mainInfoView != null) {
                mainInfoView.setClickable(true);
            }
            if (ivSideSlide != null) {
                ivSideSlide.setClickable(false);
            }
            mFmDragControl.setFirstInitPresetCenterFixed(true);
            initDragCellPosition();
        }

        @Override
        public void onDrawerClosed(@NonNull View view) {
            if (ivSideSlide != null) {
                ivSideSlide.setClickable(true);
            }
            initDragCellPosition();
        }

        @Override
        public void onDrawerStateChanged(int i) {

        }
    }

    private void initMemberVariable() {
        if (null == mIfmCallBack) {
            mIfmCallBack = new FMCallBack();
            mFmDragControl = new FMDragControl(mContext, this.mControlPanel, mIfmCallBack, this);

            mDragControlEvent = new FMDragControlEvent();
            mFmDragControl.setDragControlEvent(mDragControlEvent);
            mFmDragCellCan = mFmDragControl.initDigitFreqCell();
        }

        if (null != mFMDCC) {
            mUpdateUIListener = new UpdateRadioUIListener();
            mFMDCC.registerDataChangeListener(TAG, mUpdateUIListener);
            if (mFMSeekBar != null) {
                mFMSeekBar.resetFMSeekbarData(getSeekbarViewUID());
            }
            if (roundKnobSeekBarView != null) {
                roundKnobSeekBarView.setRoundKnobScrollValue(mFMDCC.isFMBand());
            }
        }
    }

    public boolean initDragCellPosition() {
        int[] digitFreqLocation = new int[2];
        mDigitFreqLayout.getLocationInWindow(digitFreqLocation);
        if (digitFreqLocation[0] != 0x00) {
            int centerX = digitFreqLocation[0] + (int) (mDigitFreqLayout.getWidth() * 0.5);
            int centerY = digitFreqLocation[1] + (int) (mDigitFreqLayout.getHeight() * 0.5);
            mFmDragControl.initDigitFreqCellPos(centerX, centerY, mDigitFreqLayout.getWidth(),
                    mDigitFreqLayout.getHeight());
            return true;
        }
        return false;
    }

    private final class FMDragControlEvent implements IDragControlEvent {

        @Override
        public void FMStoreStationEvent(int storeIndex) {
            if (null == mFMDCC || null == mPresetViewPaper) {
                return;
            }
            int nScreen = mPresetViewPaper.getCurrentItem();

            if (nScreen >= 0 && nScreen < mFMDCC.BAND_STATION_TOTAL / RadioData.PAGE_STATION_NUM) {
                storeIndex += nScreen * RadioData.PAGE_STATION_NUM;
            }

            mFMDCC.savePreset(mFMDCC.currentFreq(), storeIndex);
        }
    }

    private int getSeekbarViewUID() {
        int uid = FMResource.REGION_CHINA_FM;

        if (mFMDCC != null) {
            if (mFMDCC.isFMBand()) {
                switch (mFMDCC.getRegion()) {
                    case RadioData.REGION_Europe:
                        uid = FMResource.REGION_EUROPE_FM;
                        break;
                    case RadioData.REGION_JAPAN:
                        uid = FMResource.REGION_JAPAN_FM;
                        break;
                    case RadioData.REGION_Latin:
                        uid = FMResource.REGION_LATIN_FM;
                        break;
                    case RadioData.REGION_OIRT:
                        uid = FMResource.REGION_OIRT_FM;
                        break;
                    case RadioData.REGION_USA:
                        uid = FMResource.REGION_USA_FM;
                        break;
                    case RadioData.REGION_Latin2:
                        uid = FMResource.REGION_LATIN2_FM;
                        break;
                    default:
                        uid = FMResource.REGION_CHINA_FM;
                        break;
                }
            } else {
                switch (mFMDCC.getRegion()) {
                    case RadioData.REGION_Europe:
                        uid = FMResource.REGION_EUROPE_AM;
                        break;
                    case RadioData.REGION_JAPAN:
                        uid = FMResource.REGION_JAPAN_AM;
                        break;
                    case RadioData.REGION_Latin:
                        uid = FMResource.REGION_LATIN_AM;
                        break;
                    case RadioData.REGION_OIRT:
                        uid = FMResource.REGION_OIRT_AM;
                        break;
                    case RadioData.REGION_USA:
                        uid = FMResource.REGION_USA_AM;
                        break;
                    case RadioData.REGION_Latin2:
                        uid = FMResource.REGION_LATIN2_AM;
                        break;
                    default:
                        uid = FMResource.REGION_CHINA_AM;
                        break;
                }
            }
        }

        return uid;
    }

    private void updatePresetFreq(int index, int freq) {
        if (index < 0 || index >= mFMDCC.BAND_STATION_TOTAL) {
            return;
        }
        String strFreq = "87.50";
        String PS = "";
        if (FMDataControl.CONFIG_PRESET_EDIT_NAME) {
            if (mFMDCC.isFMBand()) {
                PS = mFMDCC.readRdsPs(String.valueOf(freq), "");
            }
        }

        if (null != mPresetItem[index]) {
            if (!mRadioMain.isInMultiWindowMode()) {
                if (PS.length() > 0) {
                    //PS有效
                    mPresetItem[index].setFreqUnit("");
                    mPresetItem[index].setTextSize(26);
                    mPresetItem[index].setText(PS);
                } else {
                    //PS无效
                    if (SkinUtils.useSkinPackage()) {
                        setPresetFreqTextForSkin(index, freq);
                    } else {
                        setPresetFreqTextForMcc(index, freq);
                    }
                }
            } else {
                //分屏模式
                if (ScreenSpec.getScreenStatus() != ScreenSpec.FULL_SCREEN) {
                    mPresetItem[index].setTextSize(
                            mContext.getResources().getDimension(SkinUtils.getId(R.dimen.PresetViewFreqSmallTextSize)));
                } else {
                    mPresetItem[index].setTextSize(
                            mContext.getResources().getDimension(SkinUtils.getId(R.dimen.PresetViewFreqTextSize)));
                }
                //分屏模式下空间小，不显示单位
                mPresetItem[index].setFreqUnit("");
                strFreq = mFMDCC.getFormatFreq(freq, false);
                mPresetItem[index].setText(strFreq);
            }

            if (mFMDCC.currentIndx() != index) {
                mPresetItem[index].setChecked(false);
            } else {
                mPresetItem[index].setChecked(true);
            }
        }
    }

    private void setPresetFreqTextForSkin(int index, int freq) {
        if (null == mPresetItem[index]) {
            return;
        }
        //后续如有特殊UI定制频点和单位的时候再根据皮肤包ID来区分
        mPresetItem[index].setFreqUnit(mFMDCC.getFreqUnit(freq));
        String strFreq = mFMDCC.getFormatFreq(freq, false);
        mPresetItem[index].setText(strFreq);
    }

    private void setPresetFreqTextForMcc(int index, int freq) {
        if (null == mPresetItem[index]) {
            return;
        }
        if (E_THEME_GOD == ThemeID.E_THEME_ID_400
                || E_THEME_GOD == ThemeID.E_THEME_ID_209
                || E_THEME_GOD == ThemeID.E_THEME_ID_404
                || E_THEME_GOD == ThemeID.E_THEME_ID_408
                || E_THEME_GOD == ThemeID.E_THEME_ID_409) {
            mPresetItem[index].setFreqUnit(mFMDCC.getFreqUnit(freq));
        }

        String strFreq = "87.50";
        if (E_THEME_GOD == ThemeID.E_THEME_ID_203 || E_THEME_GOD == ThemeID.E_THEME_ID_205
                || E_THEME_GOD == ThemeID.E_THEME_ID_401 || E_THEME_GOD == ThemeID.E_THEME_ID_403) {
            strFreq = mFMDCC.getFormatFreq(freq, true);
        } else {
            strFreq = mFMDCC.getFormatFreq(freq, false);
        }

        mPresetItem[index].setText(strFreq);
    }

    // FMDataContrl Notify UI Update Data
    private final class UpdateRadioUIListener implements FMDataControl.UpdateDataListener {

        private void checkFMSeekbarView() {
            if (mFMDCC != null) {
                if (null != mFMSeekBar) {
                    mFMSeekBar.resetFMSeekbarData(getSeekbarViewUID());
                }
                if (roundKnobSeekBarView != null) {
                    roundKnobSeekBarView.setRoundKnobScrollValue(mFMDCC.isFMBand());
                }
            }
        }

        // update seekbar view position
        private void updateFreqSeekBar() {
            if (null == mFMDCC) {
                return;
            }

            if (null != mFMSeekBar) {

                if (mFMDCC.getScanType() >= RadioData.SEEK_ALL
                        && mFMDCC.getScanType() < RadioData.PRESET_PLAY) {
                    mFMSeekBar.setPos(mCurrUnitStep);
                } else {
                    mFMSeekBar.setNum(mCurrUnitStep);
                }
            }
        }

        // update band stereo local-dx
        private void updateStatusLabel() {
            if (null == mFMDCC) {
                return;
            }

            // FM or AM
            if (null != mTextBand) {
                mTextBand.setText(mFMDCC.BandToString(mFMDCC.currentBand()));
            } else if (null != mImageBand) {
                mImageBand.setImageResource(SkinUtils.getId(mFMDCC.BandToImage(mFMDCC.currentBand())));
            }

            if (null != mBandFocusItem && null != mBandFocusItem[mFMDCC.currentBand()]) {
                ((RadioButton) mBandFocusItem[mFMDCC.currentBand()]).setChecked(true);
            }

            if (!SkinUtils.useSkinPackage() && (E_THEME_GOD == ThemeID.E_THEME_ID_200 ||
                    E_THEME_GOD == ThemeID.E_THEME_ID_201)) { //xt52
                if (mFMDCC.isLocal()) {
                    ((Button) btnLoc).setText(SkinUtils.getString(R.string.LOC));
                } else {
                    ((Button) btnLoc).setText(SkinUtils.getString(R.string.DX));
                }
            }

            if (null != mRadioIcon) {
                // stereo
                mRadioIcon.setCheckBox(RadioIcon.ICON_STEREO, mFMDCC.isStereo());
                // local-dx
                mRadioIcon.setCheckBox(RadioIcon.ICON_LOCDX, mFMDCC.isLocal());

                // Scan or seek
                mRadioIcon.setCheckBox(RadioIcon.ICON_SCAN,
                        (mFMDCC.getScanType() == RadioData.SEEK_PLAY
                                || mFMDCC.getScanType() == RadioData.SEEK_DOWN
                                || mFMDCC.getScanType() == RadioData.SEEK_UP));

                // AS
                mRadioIcon.setCheckBox(RadioIcon.ICON_AS,
                        mFMDCC.getScanType() == RadioData.SEEK_ALL);

                // PS
                mRadioIcon.setCheckBox(RadioIcon.ICON_PS,
                        mFMDCC.getScanType() == RadioData.PRESET_PLAY);
            }

            //mcc400-mnc021特殊定制
            if (null != cbxMainLoc) {
                cbxMainLoc.setChecked(mFMDCC.isLocal());
            }
        }

        // update presetview text info
        private void updateFreqList() {
            if (null == mFMDCC) {
                return;
            }
            boolean bIndexChange = false;
            int index = mFMDCC.currentIndx();
            mRadioIcon.setCheckBoxPxx(index + 1);
            if (index != mCurrCheckedIndex) {
                bIndexChange = true;
                mCurrCheckedIndex = index;
            }
            int[] preset = mFMDCC.readPresetList(mFMDCC.currentBand());
            for (int i = 0; i < mFMDCC.BAND_STATION_TOTAL; i++) {
                if (FMDataControl.CONFIG_PRESET_FREQ_REPLACE_PS) {
                    updatePresetIndex(i, preset[i]);
                }
                updatePresetFreq(i, preset[i]);
            }

            try {
                int screenIdx = index / RadioData.PAGE_STATION_NUM;
                if (screenIdx >= 0x00 && screenIdx != mPresetViewPaper.getCurrentItem()) {
                    if (mFMDCC.getScanType() == RadioData.SEEK_PLAY) {
                        mPresetViewPaper.setCurrentItem(screenIdx);
                    } else if (bIndexChange && mFMDCC.getScanType() != RadioData.SEEK_ALL) {
                        mPresetViewPaper.setCurrentItem(screenIdx);
                    }
                }
                mPaperAdapter.notifyDataSetChanged();
            } catch (Exception e) {

            }
        }

        @Override
        public void updateRadioUIElement(int nType) {
            switch (nType) {
                case UPDATE_DATA_INFO:
                    updateDataInfo();
                    break;
                case UPDATE_DATA_RANGE:
                    updateFreqSeekBar();
                    updateRoundKnobSeekBar();
                    break;
                case UPDATE_DATA_FREQLIST:
                    Log.d(TAG, "UPDATE_DATA_FREQLIST");
                    updateFreqList();
                    updateFreqListLogo();
                    break;
                case UPDATE_DATA_RDS_INFO: {
                    Log.d(TAG, "UPDATE_DATA_RDS_INFO");
                    if (null != tvPTYMain) {
                        if (0 != mFMDCC.mPtyType) {
                            tvPTYMain.setText(SkinUtils.getString(PTY_TYPE_NAME[mFMDCC.mPtyType]));
                        } else {
                            if (isMcc400Mnc021() || isMcc400Mnc030() || isMcc400Mnc039()) {
                                tvPTYMain.setText(SkinUtils.getString(PTY_TYPE_NAME[0])); // None
                            } else {
                                tvPTYMain.setText(""); // None
                            }
                        }
                    }
                    tvRDS_RT.setText(mFMDCC.mRdsRT);

                    //PS有变化时
                    if (null != mFMDCC.mRdsPS && !mStrPS.equals(mFMDCC.mRdsPS)) {
                        mStrPS = mFMDCC.mRdsPS;
                        if (FMDataControl.CONFIG_PS_FREQ_EXCHANGE) {
                            updateDigitFreq(mFMDCC.currentFreq());
                        } else {
                            updateFreqName();
                        }

                        //收到RDS电台后自动保存PS名称到预存列表上
                        if (FMDataControl.CONFIG_PRESET_FREQ_REPLACE_PS && FMDataControl.CONFIG_PRESET_EDIT_NAME && mFMDCC.isFMBand()) {
                            if (!TextUtils.isEmpty(mStrPS)) {
                                mFMDCC.writeRdsPs(String.valueOf(mFMDCC.currentFreq()), mStrPS);
                                updateFreqList();
                            }
                        }
                    }

                    if ((mFMDCC.mRdsInfo & 4) != 0 && mFMDCC.mIsAF) {
                        if (null != cbxAF) {
                            cbxAF.setChecked(true);
                        }
                        processRdsUiHandler.sendEmptyMessage(RDS_AF_DETECTED);
                    } else {
                        if (null != cbxAF) {
                            cbxAF.setChecked(false);
                        }
                        if (mFMDCC.mIsAF) {
                            processRdsUiHandler.sendEmptyMessage(RDS_AF_SIGNAL_DISAPPEAR);
                        }
                    }

                    if (((mFMDCC.mRdsInfo) & (2)) != 0 && mFMDCC.mIsTA) {
                        if (null != cbxTA) {
                            cbxTA.setChecked(true);
                        }
                        processRdsUiHandler.sendEmptyMessage(RDS_TA_DETECTED);
                    } else {
                        if (null != cbxTA) {
                            cbxTA.setChecked(false);
                        }
                        if (mFMDCC.mIsTA) {
                            processRdsUiHandler.sendEmptyMessage(RDS_TA_SIGNAL_DISAPPEAR);
                        }
                    }

                    if (((mFMDCC.mRdsInfo) & (1)) != 0) {
                        if (null != cbxTP) {
                            cbxTP.setChecked(true);
                        }
                        if (null != tvTpStatus) {
                            tvTpStatus.setVisibility(View.VISIBLE);
                        }
                    } else {
                        if (null != cbxTP) {
                            cbxTP.setChecked(false);
                        }
                        if (null != tvTpStatus) {
                            tvTpStatus.setVisibility(View.INVISIBLE);
                        }
                    }
                }
                break;
                case UPDATE_DATA_CLOSE_TA:
                    if (null != tglBtn_ta) {
                        tglBtn_ta.setChecked(false);
                    }
                    if (null != settingToggleBtnTA) {
                        settingToggleBtnTA.setChecked(false);
                    }
                    processRdsUiHandler.sendEmptyMessage(RDS_TA_OFF);
                    break;
                case EVENT_FREQ_CHANGE:
                    mHandler.removeMessages(MSG_UPDATE_COLLECT_LIST);
                    mHandler.sendEmptyMessage(MSG_UPDATE_COLLECT_LIST);
                    break;
                case EVENT_BAND_CHANGE:
                    mHandler.removeMessages(MSG_UPDATE_COLLECT_LIST);
                    mHandler.sendEmptyMessage(MSG_UPDATE_COLLECT_LIST);
                    mHandler.removeMessages(MSG_UPDATE_FREQ_LIST_LOGO);
                    mHandler.sendEmptyMessage(MSG_UPDATE_FREQ_LIST_LOGO);
                default:
                    break;
            }
        }

        // UPDATE_DATA_INFO
        private void updateDataInfo() {
            if (null == mFMDCC) {
                return;
            }
            if (mFMDCC.mIsSupportRDS) {
                if (mFMDCC.isFMBand()) {
                    // FM mode show RDS icons
                    if (ScreenSpec.getScreenStatus() < ScreenSpec.HALF_SCREEN) {
                        ShowRDS();
                    } else {
                        showRdsDetected(false);
                    }
                } else {
                    // AM mode hide RDS icons
                    HideRDS();
                    if (popupWindow != null && popupWindow.isShowing()) {
                        dismissPopupWindow();
                    }
                }
            }
            updateDigitFreq(mFMDCC.currentFreq());

            if (mFMDCC.isFMBand()) {
                mCurrUnitStep = (mFMDCC.mCurrentFreq - FMDataControl.mRadioParameters.FmMin)
                        / FMDataControl.mRadioParameters.FmStep;
            } else {
                mCurrUnitStep = (mFMDCC.mCurrentFreq - FMDataControl.mRadioParameters.AmMin)
                        / FMDataControl.mRadioParameters.AmStep;
            }

            checkFMSeekbarView();
            updateFreqSeekBar();
            // status
            updateStatusLabel();

            updateFreqList();
            //mcc400-mnc021特殊定制
            updateRoundKnobSeekBar();
            //恢复所有Band按键状态[20240219]
            enableAllBandCtrl(true);
        }
    }

    // update digit freq view content
    private void updateDigitFreq(int nFreq) {
        if (FMDataControl.CONFIG_PS_FREQ_EXCHANGE && mFMDCC.isFMBand()
                && !TextUtils.isEmpty(mFMDCC.mRdsPS)) {
            if (null != mImageUnit) {
                mImageBand.setVisibility(View.GONE);
                mImageUnit.setVisibility(View.GONE);
            } else if (null != mTextUnit) {
                mTextBand.setVisibility(View.GONE);
                mTextUnit.setVisibility(View.GONE);
            }
            if (null != mDigitFreqLayout) {
                mDigitFreqLayout.setVisibility(View.GONE);
            }
            if (null != mFreqName) {
                mFreqName.setText(mFMDCC.mRdsPS);
                mFreqName.setVisibility(View.VISIBLE);
            }

            if (null != tvRDS_PS) {
                String freq = mFMDCC.getFormatFreq(nFreq, true);
                tvRDS_PS.setText(
                        String.format("%s %s", mFMDCC.BandToString(mFMDCC.currentBand()), freq));
            }
        } else {
            if (null != mImageUnit) {
                mImageBand.setVisibility(View.VISIBLE);
                mImageUnit.setVisibility(View.VISIBLE);
                mDigitFreqLayout.setVisibility(View.VISIBLE);
            } else if (null != mTextUnit) {
                mDigitFreqLayout.setVisibility(View.VISIBLE);
            }
            if (null != mFreqName) {
                mFreqName.setVisibility(View.GONE);
            }
            updateFreqName();

            String strFreq = mFMDCC.getFormatFreq(nFreq, false);
            if (null != mTextUnit) {
                updateDigitFreqText(strFreq);
            } else if (null != mImageUnit) {
                updateDigitFreqImage(strFreq);
            }
        }
    }

    private void updateDigitFreqText(String szFreq) {
        int nLength = szFreq.length();

        for (int i = 0; i < EDigitFreq.DIGIT_FREQ.getValue(); i++) {

            if (nLength > 0) {
                nLength--;

                mDigitFreqArrary[i].setText(String.valueOf(szFreq.charAt(nLength)));
            } else {
                mDigitFreqArrary[i].setText("");
            }
            updateDigitFreqTextSize(i);
        }
        if (null != mFMDCC) {
            mTextUnit.setText(mFMDCC.getFreqUnit(mFMDCC.currentFreq()));
        }
    }

    private void updateDigitFreqTextSize(int index) {
        int screenStatus = ScreenSpec.getScreenStatus();
        float textSize = 0.0F;
        if (screenStatus == ScreenSpec.TWO_THIRD_SCREEN) {
            textSize = mContext.getResources().getDimension(SkinUtils.getId(R.dimen.digit_freq_middle_text_size));
            mDigitFreqArrary[index].setTextSize(textSize);
        } else if (screenStatus > ScreenSpec.TWO_THIRD_SCREEN && screenStatus <= ScreenSpec.ONE_THIRD_SCREEN) {
            textSize = mContext.getResources().getDimension(SkinUtils.getId(R.dimen.digit_freq_small_text_size));
            mDigitFreqArrary[index].setTextSize(textSize);
        }
    }

    private void updateDigitFreqImage(String szFreq) {
        if (mFMDCC == null) {
            return;
        }
        int nLength = szFreq.length();

        for (int i = 0; i < EDigitFreq.DIGIT_FREQ.getValue(); i++) {

            if (nLength > 0) {
                nLength--;

                int resId = RadioDigitFreq.DigitFreqImg(szFreq.charAt(nLength));

                if (EDigitFreq.DOT.getValue() == i && !mFMDCC.isFMBand()) {
                    mDigitFreqImageArrary[i++].setVisibility(View.GONE);
                }

                mDigitFreqImageArrary[i].setImageResource(SkinUtils.getId(resId));
                mDigitFreqImageArrary[i].setVisibility(View.VISIBLE);

            } else {
                mDigitFreqImageArrary[i].setVisibility(View.GONE);
            }
        }
        mImageUnit.setImageResource(SkinUtils.getId(mFMDCC.UnitToImage()));
    }

    private final class FMCallBack implements IFMCallBack {

        @Override
        public int getCurrentBand() {
            return mFMDCC.mCurrentBand;
        }

        @Override
        public int getCurrentFreq() {
            return mFMDCC.mCurrentFreq;
        }
    }

    private void initRadioMainCtrl() {
        if (null == mRadioIcon) {
            mRadioIcon = new RadioIcon();
            mRadioIcon.initRadioIcon(mRadioMain);
        }
        initDigitFreqCtrl();
        initFMSeekBar();
        initPresetItem();
        initMiddleCtrl();
        initBottomCtrl();
        initRdsCtrl();
        initCustomUI();
    }

    private void initDigitFreqCtrl() {
        mDigitFreqLayout = mRadioMain.findViewById(SkinUtils.getId(R.id.digital_frequency));
        View show_info_text = mRadioMain.findViewById(SkinUtils.getId(R.id.text_band_info));
        if (show_info_text instanceof TextView) {
            mTextBand = (TextView) show_info_text;

            mDigitFreqArrary = new TextView[EDigitFreq.DIGIT_FREQ.getValue()];
            mDigitFreqArrary[EDigitFreq.PERCENTILE.getValue()] = mRadioMain.findViewById(SkinUtils.getId(R.id.number_F));
            mDigitFreqArrary[EDigitFreq.TENTH.getValue()] = mRadioMain.findViewById(SkinUtils.getId(R.id.number_E));
            mDigitFreqArrary[EDigitFreq.DOT.getValue()] = mRadioMain.findViewById(SkinUtils.getId(R.id.number_D));
            mDigitFreqArrary[EDigitFreq.UNITS.getValue()] = mRadioMain.findViewById(SkinUtils.getId(R.id.number_C));
            mDigitFreqArrary[EDigitFreq.DECADE.getValue()] = mRadioMain.findViewById(SkinUtils.getId(R.id.number_B));
            mDigitFreqArrary[EDigitFreq.HUNDRED.getValue()] = mRadioMain.findViewById(SkinUtils.getId(R.id.number_A));

            mTextUnit = mRadioMain.findViewById(SkinUtils.getId(R.id.radio_info_hz));
            mFreqName = mRadioMain.findViewById(SkinUtils.getId(R.id.freq_name));
        } else if (show_info_text instanceof ImageView) {
            mImageBand = (ImageView) show_info_text;

            mDigitFreqImageArrary = new ImageView[EDigitFreq.DIGIT_FREQ.getValue()];
            mDigitFreqImageArrary[EDigitFreq.PERCENTILE.getValue()] = mRadioMain.findViewById(SkinUtils.getId(R.id.number_F));
            mDigitFreqImageArrary[EDigitFreq.TENTH.getValue()] = mRadioMain.findViewById(SkinUtils.getId(R.id.number_E));
            mDigitFreqImageArrary[EDigitFreq.DOT.getValue()] = mRadioMain.findViewById(SkinUtils.getId(R.id.number_D));
            mDigitFreqImageArrary[EDigitFreq.UNITS.getValue()] = mRadioMain.findViewById(SkinUtils.getId(R.id.number_C));
            mDigitFreqImageArrary[EDigitFreq.DECADE.getValue()] = mRadioMain.findViewById(SkinUtils.getId(R.id.number_B));
            mDigitFreqImageArrary[EDigitFreq.HUNDRED.getValue()] = mRadioMain.findViewById(SkinUtils.getId(R.id.number_A));

            mImageUnit = mRadioMain.findViewById(SkinUtils.getId(R.id.radio_info_hz));
            mFreqName = mRadioMain.findViewById(SkinUtils.getId(R.id.freq_name));
        }
    }

    private void initFMSeekBar() {
        // SeekBar
        mFMSeekBarListener = new AutoFMSeekBarListener();
        mFMSeekBar = mRadioMain.findViewById(SkinUtils.getId(R.id.id_china_seekbar_fm));
        if (null != mFMSeekBar) {
            mFMSeekBar.setFMSeekBarListener(mFMSeekBarListener);
        }

        //RoundKnobSeek
        roundKnobSeekBarListener = new RoundKnobSeekBarListener();
        roundKnobSeekBarView = mRadioMain.findViewById(SkinUtils.getId(R.id.id_round_knob_seekbar_fm));
        if (null != roundKnobSeekBarView) {
            roundKnobSeekBarView.setOnProgressChangeListener(roundKnobSeekBarListener);
        }
    }

    private void initPresetItem() {
        mScreenFousItem = new RadioButton[3];
        mScreenFousItem[0] = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_screen1));
        mScreenFousItem[1] = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_screen2));
        mScreenFousItem[2] = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_screen3));
        if (mScreenFousItem[0] != null) {
            mScreenFousItem[0].setChecked(true);
            mScreenFousItem[0].setOnClickListener(this);
        }
        if (mScreenFousItem[1] != null) {
            mScreenFousItem[1].setOnClickListener(this);
        }
        if (mScreenFousItem[2] != null) {
            mScreenFousItem[2].setOnClickListener(this);
        }

        mPresetItem = new FMPresetView[mFMDCC.BAND_STATION_TOTAL];
        mPresetViewPaper = mRadioMain.findViewById(SkinUtils.getId(R.id.PresetViewPaper));
        mPaperAdapter = new PresetPaperAdapter(mContext);
        mPaperAdapter.setOnPresetPaperAdapterListener(this);
        mPageChangeListener = new ViewPagerOnPageChangeListener();
        if (mPresetViewPaper != null) {
            mPresetViewPaper.addOnPageChangeListener(mPageChangeListener);
            mPresetViewPaper.setAdapter(mPaperAdapter);
        }
    }

    private void initMiddleCtrl() {
        View btnPrev = mRadioMain.findViewById(SkinUtils.getId(R.id.btn_seek_up));
        if (null != btnPrev) {
            btnPrev.setOnClickListener(mOnBottomClickListener);
            btnPrev.setOnLongClickListener(mOnButtLongClickListener);
        }

        View btnNext = mRadioMain.findViewById(SkinUtils.getId(R.id.btn_seek_down));
        if (null != btnNext) {
            btnNext.setOnClickListener(mOnBottomClickListener);
            btnNext.setOnLongClickListener(mOnButtLongClickListener);
        }

        Button btnAutoTest = mRadioMain.findViewById(SkinUtils.getId(R.id.auto_test));
        if (null != btnAutoTest) {
            btnAutoTest.setOnClickListener(mOnBottomClickListener);
        }

        View btnPresetDown = mRadioMain.findViewById(SkinUtils.getId(R.id.btn_preset_down));
        if (btnPresetDown != null) {
            btnPresetDown.setOnClickListener(mOnBottomClickListener);
            btnPresetDown.setOnLongClickListener(mOnButtLongClickListener);
        }

        View btnPresetUp = mRadioMain.findViewById(SkinUtils.getId(R.id.btn_preset_up));
        if (btnPresetUp != null) {
            btnPresetUp.setOnClickListener(mOnBottomClickListener);
            btnPresetUp.setOnLongClickListener(mOnButtLongClickListener);
        }
    }

    private void initBottomCtrl() {
        btnBand = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_band));
        if (null != btnBand) {
            btnBand.setOnClickListener(mOnBottomClickListener);
        }

        View btnFmBand = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_fm_band));
        if (null != btnFmBand) {
            btnFmBand.setOnClickListener(mOnBottomClickListener);
        }

        View btnAmBand = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_am_band));
        if (null != btnAmBand) {
            btnAmBand.setOnClickListener(mOnBottomClickListener);
            if (RadioUtils.getRadioModel().equalsIgnoreCase(RadioUtils.RADIO_INSIDE)) {
                btnAmBand.setVisibility(View.GONE);//AM
            }
        }

        View btnScan = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_scan));
        if (null != btnScan) {
            btnScan.setOnClickListener(mOnBottomClickListener);
            btnScan.setOnLongClickListener(mOnButtLongClickListener);
        }

        View btnScan_ru = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_scan_ru));
        if (null != btnScan_ru) {
            btnScan_ru.setOnLongClickListener(mOnButtLongClickListener);
        }

        View btnAsPs = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_asps));
        if (null != btnAsPs) {
            btnAsPs.setOnClickListener(mOnBottomClickListener);
            btnAsPs.setOnLongClickListener(mOnButtLongClickListener);
        }

        View btnEditName = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_edit_name));
        if (null != btnEditName) {
            btnEditName.setOnClickListener(mOnBottomClickListener);
        }

        btnMute = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_mute));
        if (null != btnMute) {
            btnMute.setOnClickListener(mOnBottomClickListener);
        }

        btnLoc = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_locdx));
        if (null != btnLoc) {
            btnLoc.setOnClickListener(mOnBottomClickListener);
        }

        View btnKeypad = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_keypad));
        if (btnKeypad != null) {
            btnKeypad.setOnClickListener(mOnBottomClickListener);
        }

        btnEq = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_eq));
        isDisplayEQ();

        View btnSetting = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_setting));
        if (null != btnSetting) {
            btnSetting.setOnClickListener(mOnBottomClickListener);
        }
        View btnAs = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_as));
        if (null != btnAs) {
            btnAs.setOnClickListener(mOnBottomClickListener);
        }
        View btnSettings = mRadioMain.findViewById(SkinUtils.getId(R.id.btn_setting));
        if (null != btnSettings) {
            btnSettings.setOnClickListener(mOnBottomClickListener);
        }
        mCollectionBtn = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_collect));
        if (null != mCollectionBtn) {
            mCollectionBtn.setOnClickListener(mOnBottomClickListener);
        }
        buttFavoriteList = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_favorite_list));
        if (null != buttFavoriteList) {
            buttFavoriteList.setOnClickListener(mOnBottomClickListener);
        }
        buttAmFm = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_am_fm));
        if (null != buttAmFm) {
            if (mFMDCC.isFMBand()) {
                buttAmFm.setBackgroundResource(SkinUtils.getId(R.drawable.radio_fm1));
            } else {
                buttAmFm.setBackgroundResource(SkinUtils.getId(R.drawable.radio_am1));
            }
            buttAmFm.setOnClickListener(mOnBottomClickListener);
        }
        btnRadioSetting = mRadioMain.findViewById(SkinUtils.getId(R.id.btn_radio_setting));
        if(null != btnRadioSetting) {
            btnRadioSetting.setOnClickListener(mOnBottomClickListener);
        }
    }

    private void isDisplayEQ() {
        if (btnEq != null) {
            btnEq.setOnClickListener(mOnBottomClickListener);
            if (!ExtAudioMuxer.ExtAudioAvailable || 0 == Settings.System.getInt
                    (mContext.getContentResolver(), EQ_INSIDE, 1)) {
                btnEq.setVisibility(View.GONE);
            } else if (1 == Settings.System.getInt
                    (mContext.getContentResolver(), EQ_INSIDE, 1)) {
                btnEq.setVisibility(View.VISIBLE);
            }
        }
    }

    /**
     * 初始化特殊定制的UI
     */
    private void initCustomUI() {
        if (SkinUtils.useSkinPackage()) {
            initSkinCustomUI();
        } else {
            initMccCustomUI();
        }
    }

    /**
     * 初始化皮肤包特殊定制的UI
     */
    private void initSkinCustomUI() {
        switch (SkinUtils.getCurrentSkinID()) {
            case SkinID.SKIN_ZA01:
            case SkinID.SKIN_ZA03:
            case SkinID.SKIN_ZA04:
            case SkinID.SKIN_ZA05:
            case SkinID.SKIN_ZA33:
            case SkinID.SKIN_SA82:
            case SkinID.SKIN_SA85:
            case SkinID.SKIN_SA87:
            case SkinID.SKIN_GB01:
            case SkinID.SKIN_GB03:
                initDrawLayout();
                break;
            case SkinID.SKIN_DZ16:
            case SkinID.SKIN_XT366:
                mainInfoView = mRadioMain.findViewById(SkinUtils.getId("ll_radio_main"));
                break;
            case SkinID.SKIN_N91:
                SharedPreferences per = mContext.getSharedPreferences("date",Context.MODE_PRIVATE);
                int Style = per.getInt("n91_top_hidden", Integer.parseInt("0"));
                ivRds = mRadioMain.findViewById(SkinUtils.getId(R.id.iv_rds));
                llRdsIcon = mRadioMain.findViewById(SkinUtils.getId(R.id.ll_rds_icon));
                if (null != ivRds) {
                    ivRds.setOnClickListener(this);
                    if (Style == 1 && llRdsIcon != null ) {
                        ivRds.setSelected(false);
                        llRdsIcon.setVisibility(View.GONE);
                    } else if (Style == 0) {
                        ivRds.setSelected(true);
                        llRdsIcon.setVisibility(View.VISIBLE);
                    }
                    if (mFMDCC.isFMBand() && llRdsIcon != null) {
                        ivRds.setEnabled(true);
                    } else {
                        ivRds.setEnabled(false);
                        if (llRdsIcon.getVisibility() == View.VISIBLE) {
                            llRdsIcon.setVisibility(View.GONE);
                        }
                    }
                }
                break;
            case SkinID.SKIN_SA133:
                initDrawLayout();
                bandSwitch();
                break;
            case SkinID.SKIN_NONE:
            default:
                break;
        }
    }

    /**
     * 初始化原MCC特殊定制的UI
     */
    private void initMccCustomUI() {
        switch (E_THEME_GOD) {
            case ThemeID.E_THEME_ID_153:
                bandSwitch();
                break;
            case ThemeID.E_THEME_ID_200:
            case ThemeID.E_THEME_ID_201:
                Button mBottomNext = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_bottom_next));
                if (null != mBottomNext) {
                    mBottomNext.setOnClickListener(mOnBottomClickListener);
                }
                Button mBottomPrev = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_bottom_prev));
                if (null != mBottomPrev) {
                    mBottomPrev.setOnClickListener(mOnBottomClickListener);
                }
                Button mBottomBack = mRadioMain.findViewById(SkinUtils.getId(R.id.butt_back));
                if (null != mBottomBack) {
                    mBottomBack.setOnClickListener(mOnBottomClickListener);
                }
                break;
            case ThemeID.E_THEME_ID_205:
                ImageView imageView = mRadioMain.findViewById(SkinUtils.getId(R.id.radio_icon_anim));
                imageView.setBackgroundResource(SkinUtils.getId(R.drawable.radio_icon_anim));
                mRadioIconAnim = (AnimationDrawable) imageView.getBackground();
                mRadioIconAnim.setOneShot(false);
                break;
            case ThemeID.E_THEME_ID_206:
                for (int pos = 0; pos < mFMDCC.BAND_STATION_TOTAL; pos++) {
                    String name = String.format(Locale.ENGLISH, "preset_%02d", pos + 1);
                    int resID = mRadioMain.getResources().getIdentifier(name, "id", mRadioMain.getPackageName());
                    mPresetItem[pos] = mRadioMain.findViewById(SkinUtils.getId(resID));
                    if (mPresetItem[pos] != null) {
                        mPresetItem[pos].setOnTouchListener(mPresetOnTouchListener);
                        mPresetItem[pos].setOnLongClickListener(mPresetOnLongClickListener);
                    }
                }
                break;
            case ThemeID.E_THEME_ID_400:
                if (RadioData.E_THEME_SUB == 28) {
                    bandSwitch();
                }
                break;
            case ThemeID.E_THEME_ID_401:
                if (ScreenSpec.getScreenStatus() == ScreenSpec.FULL_SCREEN) {
                    mDigitFreqArrary[EDigitFreq.PERCENTILE.getValue()].setTextSize(110);
                    mDigitFreqArrary[EDigitFreq.TENTH.getValue()].setTextSize(110);
                    mDigitFreqArrary[EDigitFreq.DOT.getValue()].setTextSize(110);
                    mDigitFreqArrary[EDigitFreq.UNITS.getValue()].setTextSize(110);
                    mDigitFreqArrary[EDigitFreq.DECADE.getValue()].setTextSize(110);
                    mDigitFreqArrary[EDigitFreq.HUNDRED.getValue()].setTextSize(110);
                } else if (ScreenSpec.getScreenStatus() == ScreenSpec.HALF_SCREEN) {
                    mTextBand.setVisibility(View.GONE);
                    mTextUnit.setVisibility(View.GONE);
                    mPresetViewPaper.setVisibility(View.GONE);
                }
                break;
            case ThemeID.E_THEME_ID_405:
                Log.d(TAG, "initMccCustomUI:  E_THEME_SUB = " + E_THEME_SUB);
                //mcc405-mnc001
                if (E_THEME_SUB == 1) {
                    mCollectListView = (ListView) mRadioMain.findViewById(R.id.radio_freq_collection_list);
                    mCollectionListAdapter = new CollectionListAdapter(mContext, null);
                    mCollectListView.setAdapter(mCollectionListAdapter);
                    mCollectListView.setOnItemClickListener(mCollectionListClickListener);

                    View[] freqListSelect = new View[2];
                    freqListSelect[0] = mRadioMain.findViewById(SkinUtils.getId(R.id.rb_radio_freq_preset));
                    freqListSelect[1] = mRadioMain.findViewById(SkinUtils.getId(R.id.rb_radio_freq_favorite));
                    if (freqListSelect[0] != null) {
                        freqListSelect[0].setOnClickListener(mOnBottomClickListener);
                        ((RadioButton) freqListSelect[0]).setChecked(true);
                    }
                    if (freqListSelect[1] != null) {
                        freqListSelect[1].setOnClickListener(mOnBottomClickListener);
                        ((RadioButton) freqListSelect[1]).setChecked(false);
                    }
                }
            default:
                break;
        }
    }

    private void bandSwitch() {
        mBandFocusItem = new View[4];
        mBandFocusItem[0] = mRadioMain.findViewById(SkinUtils.getId(R.id.band_info_fm1));
        mBandFocusItem[1] = mRadioMain.findViewById(SkinUtils.getId(R.id.band_info_fm2));
        mBandFocusItem[2] = mRadioMain.findViewById(SkinUtils.getId(R.id.band_info_fm3));
        mBandFocusItem[3] = mRadioMain.findViewById(SkinUtils.getId(R.id.band_info_am1));
        mBandFocusItem[0].setOnClickListener(mOnBottomClickListener);
        mBandFocusItem[1].setOnClickListener(mOnBottomClickListener);
        mBandFocusItem[2].setOnClickListener(mOnBottomClickListener);
        mBandFocusItem[3].setOnClickListener(mOnBottomClickListener);
        if (RadioUtils.getRadioModel().equalsIgnoreCase(RadioUtils.RADIO_INSIDE)) {
            mBandFocusItem[3].setVisibility(View.GONE); //AM1
        }
    }

    /**
     * 初始化RDS 信息显示的UI
     */
    private void initRdsCtrl() {
        // add by dsh Begin RDS
        cbxAF = mRadioMain.findViewById(SkinUtils.getId(R.id.checkbox_rds_af));
        cbxAF_ani = mRadioMain.findViewById(SkinUtils.getId(R.id.checkbox_rds_af_ani));
        if (null != cbxAF_ani) {
            RDS_AF_iconTransition = (AnimationDrawable) cbxAF_ani.getBackground();
        }

        cbxTA = mRadioMain.findViewById(SkinUtils.getId(R.id.checkbox_rds_ta));

        cbxTA_ani = mRadioMain.findViewById(SkinUtils.getId(R.id.checkbox_rds_ta_ani));
        if (null != cbxTA_ani) {
            RDS_TA_iconTransition = (AnimationDrawable) cbxTA_ani.getBackground();
        }

        if (SkinUtils.getCurrentSkinID().equals(SkinID.SKIN_N91)) {
            cbxTA.setVisibility(View.GONE);
            cbxAF.setVisibility(View.GONE);
        }
        cbxTP = mRadioMain.findViewById(SkinUtils.getId(R.id.checkbox_rds_tp));
        cbxTP_ani = mRadioMain.findViewById(SkinUtils.getId(R.id.checkbox_rds_tp_ani));
        if (null != cbxTP_ani) {
            RDS_TP_iconTransition = (AnimationDrawable) cbxTP_ani.getBackground();
        }

        tvPTYMain = mRadioMain.findViewById(SkinUtils.getId(R.id.rds_pty));
        tvRDS_PS = mRadioMain.findViewById(SkinUtils.getId(R.id.rds_ps));
        if (FMDataControl.CONFIG_TEXTVIEW_PS_EDIT_NAME) {
            tvEdit_rds_ps = mRadioMain.findViewById(SkinUtils.getId(R.id.tv_edit_rds_ps));
        }
        tvRDS_RT = mRadioMain.findViewById(SkinUtils.getId(R.id.rds_rt_info));

        rdsImageButton = mRadioMain.findViewById(SkinUtils.getId(R.id.rds_imgbtn));
        if (rdsImageButton != null) {
            rdsImageButton.setOnClickListener(this);
        }

        //mcc400-mnc021特殊定制
        rdsStatusLayout = mRadioMain.findViewById(SkinUtils.getId(R.id.ll_rds_status));
        tvPtyTitle = mRadioMain.findViewById(SkinUtils.getId(R.id.rds_pty_title));
        tvTpStatus = mRadioMain.findViewById(SkinUtils.getId(R.id.rds_tp_status));
        cbxMainAF = mRadioMain.findViewById(SkinUtils.getId(R.id.checkbox_af));
        if (null != cbxMainAF) {
            cbxMainAF.setOnClickListener(new OnSettingClickListener());
        }
        cbxMainTA = mRadioMain.findViewById(SkinUtils.getId(R.id.checkbox_ta));
        if (null != cbxMainTA) {
            cbxMainTA.setOnClickListener(new OnSettingClickListener());
        }
        cbxMainLoc = mRadioMain.findViewById(SkinUtils.getId(R.id.checkbox_loc));
        if (null != cbxMainLoc) {
            cbxMainLoc.setOnClickListener(new OnSettingClickListener());
        }
        //n91特殊定制
        rdsPty = mRadioMain.findViewById(SkinUtils.getId(R.id.rds_pty_list));
        if (null != rdsPty) {
            rdsPty.setOnClickListener(this);
        }
        buttonTglTA = mRadioMain.findViewById(SkinUtils.getId(R.id.button_tglTA));
        if (null != buttonTglTA) {
            buttonTglTA.setOnClickListener(this);
        }

        buttonTglAF = mRadioMain.findViewById(SkinUtils.getId(R.id.button_tglAF));
        if (null != buttonTglAF) {
            buttonTglAF.setOnClickListener(this);
        }
        btnReg = mRadioMain.findViewById(SkinUtils.getId(R.id.btn_reg));
        if (btnReg != null) {
            btnReg.setOnClickListener(this);
        }
    }

    /**
     * 用于处理RDS打开的情况界面标识显示
     *
     * @param bShow
     */
    public void showRdsDetected(boolean bShow) {
        if (mFMDCC.mIsAF) {
            processRdsUiHandler.sendEmptyMessage(RDS_AF_ON);

            if (((mFMDCC.mRdsInfo) & (4)) != 0) {
                if (null != cbxAF) {
                    cbxAF.setChecked(true);
                }
                processRdsUiHandler.sendEmptyMessage(RDS_AF_DETECTED);
            }
        } else {
            if (bShow) {
                processRdsUiHandler.sendEmptyMessage(RDS_AF_OFF);
            }
        }

        if (mFMDCC.mIsTA) {
            processRdsUiHandler.sendEmptyMessage(RDS_TA_ON);

            if (((mFMDCC.mRdsInfo) & (2)) != 0) {
                if (null != cbxTA) {
                    cbxTA.setChecked(true);
                }
                processRdsUiHandler.sendEmptyMessage(RDS_TA_DETECTED);
            }
        } else {
            if (bShow) {
                processRdsUiHandler.sendEmptyMessage(RDS_TA_OFF);
            }
        }
    }

    public void HideRDS() {
        if (null != cbxAF) {
            cbxAF.setVisibility(View.GONE);
        }
        if (null != cbxTA) {
            cbxTA.setVisibility(View.GONE);
        }
        if (null != cbxTP) {
            cbxTP.setVisibility(View.GONE);
        }
        if (null != tvPTYMain) {
            tvPTYMain.setVisibility(View.GONE);
        }
        if (null != cbxAF_ani) {
            cbxAF_ani.setVisibility(View.GONE);
        }
        if (null != cbxTA_ani) {
            cbxTA_ani.setVisibility(View.GONE);
        }

        if (null != tvRDS_PS) {
            tvRDS_PS.setVisibility(View.INVISIBLE);
        }
        if (null != tvRDS_RT) {
            tvRDS_RT.setVisibility(View.INVISIBLE);
        }

        if (null != rdsImageButton) {
            if (E_THEME_GOD == ThemeID.E_THEME_ID_153) {
                rdsImageButton.setVisibility(View.GONE);
            } else {
                rdsImageButton.setVisibility(View.INVISIBLE);
            }
        }
        //mcc400-mnc021特殊定制
        if (null != rdsStatusLayout) {
            rdsStatusLayout.setVisibility(View.INVISIBLE);
        }
        if (null != tvPtyTitle) {
            tvPtyTitle.setVisibility(View.INVISIBLE);
        }
        if (null != tvTpStatus) {
            tvTpStatus.setVisibility(View.INVISIBLE);
        }
    }

    public void ShowRDS() {
        if (null != cbxAF) {
            cbxAF.setVisibility(View.VISIBLE);
        }
        if (null != cbxTA) {
            cbxTA.setVisibility(View.VISIBLE);
        }
        if (null != cbxTP) {
            cbxTP.setVisibility(View.VISIBLE);
        }
        if (null != tvPTYMain) {
            tvPTYMain.setVisibility(View.VISIBLE);
        }
        if (null != cbxAF_ani) {
            cbxAF_ani.setVisibility(View.VISIBLE);
        }
        if (null != cbxTA_ani) {
            cbxTA_ani.setVisibility(View.VISIBLE);
        }
        if (null != tvRDS_PS) {
            tvRDS_PS.setVisibility(View.VISIBLE);
        }
        //modify by fanguoqing
        boolean bRdsRT = "true".equals(RadioUtils.getProp("persist.sys.radio.rds.rt", "true"));
        if (null != tvRDS_RT) {
            tvRDS_RT.setVisibility(bRdsRT ? View.VISIBLE : View.INVISIBLE);
        }

        if (null != rdsImageButton) {
            rdsImageButton.setVisibility(View.VISIBLE);
        }

        //mcc400-mnc021特殊定制
        if (null != rdsStatusLayout) {
            rdsStatusLayout.setVisibility(View.VISIBLE);
        }
        if (null != tvPtyTitle) {
            tvPtyTitle.setVisibility(View.VISIBLE);
        }
    }


    private void ShowKeyBoardPopupWindow(){
        if (mKeyboardPopupView == null){
            mKeyboardPopupView = LayoutInflater.from(mContext).inflate(SkinUtils.getId(R.layout.dialog_keyboard), null);
            keyboardWindow = new PopupWindow(mKeyboardPopupView, ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);
            mTextInput = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.input));
            mBtnKey1 = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.key_1));
            mBtnKey2 = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.key_2));
            mBtnKey3 = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.key_3));
            mBtnKey4 = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.key_4));
            mBtnKey5 = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.key_5));
            mBtnKey6 = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.key_6));
            mBtnKey7 = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.key_7));
            mBtnKey8 = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.key_8));
            mBtnKey9 = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.key_9));
            mBtnKey0 = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.key_0));
            mBtnKeyDel = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.key_del));
            mBtnKeyDot = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.key_dot));
            mBtnKeyEnter = mKeyboardPopupView.findViewById(SkinUtils.getId(R.id.key_enter));
            mOnKeyboardClickListener = new OnKeyBoardClickListener();

            if (mBtnKey1 != null) {
                mBtnKey1.setOnClickListener(mOnKeyboardClickListener);
            }
            if (mBtnKey2 != null) {
                mBtnKey2.setOnClickListener(mOnKeyboardClickListener);
            }
            if (mBtnKey3 != null) {
                mBtnKey3.setOnClickListener(mOnKeyboardClickListener);
            }
            if (mBtnKey4 != null) {
                mBtnKey4.setOnClickListener(mOnKeyboardClickListener);
            }
            if (mBtnKey5 != null) {
                mBtnKey5.setOnClickListener(mOnKeyboardClickListener);
            }
            if (mBtnKey6 != null) {
                mBtnKey6.setOnClickListener(mOnKeyboardClickListener);
            }
            if (mBtnKey7 != null) {
                mBtnKey7.setOnClickListener(mOnKeyboardClickListener);
            }
            if (mBtnKey8 != null) {
                mBtnKey8.setOnClickListener(mOnKeyboardClickListener);
            }
            if (mBtnKey9 != null) {
                mBtnKey9.setOnClickListener(mOnKeyboardClickListener);
            }
            if (mBtnKey0 != null) {
                mBtnKey0.setOnClickListener(mOnKeyboardClickListener);
            }
            if (mBtnKeyDel != null) {
                mBtnKeyDel.setOnClickListener(mOnKeyboardClickListener);
            }
            if (mBtnKeyDot != null) {
                mBtnKeyDot.setOnClickListener(mOnKeyboardClickListener);
            }
            if (mBtnKeyEnter != null) {
                mBtnKeyEnter.setOnClickListener(mOnKeyboardClickListener);
            }
        }
        mTextInput.setText("");
        updateKeyboardState(0);
        keyboardWindow.showAtLocation(mXmlLayoutView, Gravity.RIGHT, 0, 30);
    }

    private void ShowRDSPopupWindow() {
        if (popupWindow == null) {
            mRdsPopupView = LayoutInflater.from(mContext).inflate(SkinUtils.getId(R.layout.rds_popuwindow), null);
            popupWindow = new PopupWindow(mRdsPopupView, ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);

            mOnRdsClickListener = new OnRdsClickListener();

            tglBtn_ta = mRdsPopupView.findViewById(SkinUtils.getId(R.id.tglTA));
            if (null != tglBtn_ta) {
                tglBtn_ta.setOnClickListener(mOnRdsClickListener);
            }

            tglBtn_af = mRdsPopupView.findViewById(SkinUtils.getId(R.id.tglAF));
            if (null != tglBtn_af) {
                tglBtn_af.setOnClickListener(mOnRdsClickListener);
            }

            tglBtn_ct = mRdsPopupView.findViewById(SkinUtils.getId(R.id.tglCT));
            tvPTY = mRdsPopupView.findViewById(SkinUtils.getId(R.id.tv_pty));

            ImageButton dismissBtn = mRdsPopupView.findViewById(SkinUtils.getId(R.id.dismis_imgbtn));
            if (null != dismissBtn) {
                dismissBtn.setOnClickListener(mOnRdsClickListener);
            }

            initPtyTypesBtn(mRdsPopupView);
        }

        //za01 dz16弹框前虚化背景
        setBlurBackground();
        if (null != tvPTY) {
            tvPTY.setText(SkinUtils.getString(PTY_TYPE_NAME[mFMDCC.mPtyType]));
        }
        if (null != mPtyButts[mFMDCC.mPtyType]) {
            mPtyButts[mFMDCC.mPtyType].setChecked(true);
        }
        if (null != tglBtn_af) {
            tglBtn_af.setChecked(mFMDCC.mIsAF);
        }
        if (null != tglBtn_ta) {
            tglBtn_ta.setChecked(mFMDCC.mIsTA);
        }

        popupWindow.setOutsideTouchable(false);
        popupWindow.setFocusable(false);
        popupWindow.setAnimationStyle(R.style.PopupAnimation);
        //mcc400-mnc021特殊定制
        if (isMcc400Mnc021() || isMcc400Mnc030() || isMcc400Mnc039()) {
            popupWindow.showAtLocation(mXmlLayoutView, Gravity.TOP | Gravity.START, 0, 0);
        } else if (SkinUtils.getCurrentSkinID().equals(SkinID.SKIN_ZA37)) {
            popupWindow.showAtLocation(mXmlLayoutView, Gravity.TOP, 0, 145);
        } else if (SkinUtils.getCurrentSkinID().equals(SkinID.SKIN_XT510)) {
            popupWindow.showAtLocation(mXmlLayoutView, Gravity.TOP, 0, 145);
             if(null != mXmlLayoutView ) {
                 float originalAlpha = mXmlLayoutView.getAlpha();
                 mXmlLayoutView.setAlpha(0.4f);
                 popupWindow.setOnDismissListener(new PopupWindow.OnDismissListener() {
                     @Override
                     public void onDismiss() {
                         // 恢复原始透明度
                         mXmlLayoutView.setAlpha(originalAlpha);
                     }
                 });
             }
        } else if (SkinUtils.getCurrentSkinID().equals(SkinID.SKIN_XT554)) {
            popupWindow.showAtLocation(mXmlLayoutView, Gravity.CENTER, 0, -42);
        } else if (isMcc400Mnc040()) {
            Log.d(TAG, "ShowRDSPopupWindow: 11111111");
            popupWindow.showAtLocation(mXmlLayoutView, Gravity.CENTER, 0, -10);
        } else if (isRK01()) {
            popupWindow.showAtLocation(mXmlLayoutView, Gravity.CENTER, 0, -30);
        } else {
            popupWindow.showAtLocation(mXmlLayoutView, Gravity.CENTER, 0, 30);
        }
    }

    /**
     * 背景虚化处理:截取DrawLayout展出与收缩图片来存储变量
     */
    public void setBlurBackground() {
        if (mainInfoView == null || mRootLayoutView == null) {
            return;
        }
        if (RadioUtils.supportWallpaperCustomized()) {
            Drawable drawable = updateWallpaper();
            if (drawable != null) {
                mRootLayoutView.setBackground(drawable);
            }
        } else if (mainInfoView instanceof CustomDrawerLayout) {
            if (((CustomDrawerLayout) mainInfoView).isDrawerOpen(GravityCompat.END)) {
                if (compressBitmap == null) {
                    compressBitmap = FastBlurUtils.getBlurBackgroundDrawer(mRadioMain);
                }
                mRootLayoutView.setBackground(new BitmapDrawable(mContext.getResources(), compressBitmap));
            } else {
                if (extendBitmap == null) {
                    extendBitmap = FastBlurUtils.getBlurBackgroundDrawer(mRadioMain);
                }
                mRootLayoutView.setBackground(new BitmapDrawable(mContext.getResources(), extendBitmap));
            }
        } else {
            if (extendBitmap == null) {
                extendBitmap = FastBlurUtils.getBlurBackgroundDrawer(mRadioMain);
            }
            mRootLayoutView.setBackground(new BitmapDrawable(mContext.getResources(), extendBitmap));
        }

        mainInfoView.setVisibility(View.INVISIBLE);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        /*判断当前Pop的状态，是否需要拦截点击事件*/
        if (popupWindow != null && popupWindow.isShowing()) {
            dismissPopupWindow();
            return false;
        }
        if (settingWindow != null && settingWindow.isShowing()) {
            dismissSettingWindow();
            return false;
        }
        if (keyboardWindow != null && keyboardWindow.isShowing()) {
            dismissKeyBoardgWindow();
            return false;
        }

        return true;
    }

    private void initPtyTypesBtn(View view) {
        for (int i = 0; i < mPtyButts.length; i++) {
            mPtyButts[i] = view.findViewById(SkinUtils.getId(PTY_BUTTON_ID[i]));
            String text;
            if (isMcc400Mnc021() || isMcc400Mnc030() || isMcc400Mnc039()) {
                text = String.format(Locale.ENGLISH, "%s", SkinUtils.getString(PTY_TYPE_NAME[i]));
            } else {
                text = String.format(Locale.ENGLISH, "%d %s", i, SkinUtils.getString(PTY_TYPE_NAME[i]));
            }
            if (null != mPtyButts[i]) {
                mPtyButts[i].setText(text);
                mPtyButts[i].setOnClickListener(mOnRdsClickListener);
            }
        }
    }

    public void setClickPresetItem(int nPresetIndex) {
        if (null == mFMDCC) {
            return;
        }
        Log.d(TAG, "setClickPresetItem: nPresetIndex=" + nPresetIndex);
        if (nPresetIndex >= 0 && nPresetIndex < mFMDCC.BAND_STATION_TOTAL) {
            int[] presets = mFMDCC.readPresetList(mFMDCC.currentBand());
            mFMDCC.setFreq(presets[nPresetIndex], nPresetIndex);
        }
    }

    public void onLongClickPresetItem(int nPresetIndex) {
        if (null == mFMDCC) {
            return;
        }
        if (SkinUtils.useSkinPackage()) {
            onLongClickPresetItemForSkin(nPresetIndex);
        } else {
            onLongClickPresetItemForMcc(nPresetIndex);
        }
    }

    private void onLongClickPresetItemForSkin(int Index) {
        if (Index >= 0 && Index < mFMDCC.BAND_STATION_TOTAL) {
            mFmDragControl.saveFavoriteFreq(Index % RadioData.PAGE_STATION_NUM);
        }
    }

    private void onLongClickPresetItemForMcc(int Index) {
        if (Index >= 0 && Index < mFMDCC.BAND_STATION_TOTAL) {
            if (mFMDCC.getScanType() > 0 || E_THEME_GOD == ThemeID.E_THEME_ID_206) {
                mFMDCC.savePreset(mFMDCC.currentFreq(), Index);
            } else {
                mFmDragControl.saveFavoriteFreq(Index % RadioData.PAGE_STATION_NUM);
            }
        }
    }

    /**
     * 更新预设频点的按钮背景为对应电台图片
     */
    private void updateFreqListLogo() {
        if (null == mFMDCC) {
            return;
        }
        if (FMDataControl.CONFIG_PRESET_FREQ_REPLACE_PS && LogoUtils.isSupportArea()) {
            int[] preset = mFMDCC.readPresetList(mFMDCC.currentBand());
            for (int i = 0; i < mFMDCC.BAND_STATION_TOTAL; i++) {
                setLogo(i, preset[i]);
            }
        }
    }

    /**
     * 设置某个预设频点的按钮背景为电台图片
     *
     * @param index
     * @param freq
     */
    private void setLogo(int index, int freq) {
        if (null == mPresetItem[index]) {
            return;
        }
        if (FMDataControl.CONFIG_PRESET_FREQ_REPLACE_PS && LogoUtils.isSupportArea()) {
            Log.d(TAG, "setLogo: index=" + index + " freq=" + freq);
            if (LogoUtils.isSupportAreaFreq(freq)) {
                mPresetItem[index].setBackground(LogoUtils.getLogo(freq));
            } else {
                mPresetItem[index].setBackgroundResource(SkinUtils.getId(R.drawable.preset_xml_selector));
            }
        }
    }

    /**
     * [mcc153 ]动态禁用band按钮，避免点击太快出现波段和频点不对应[20240219]
     * @param band
     */
    private void enableBandCtrl(int band) {
        if (mBandFocusItem != null) {
            for (int i = 0; i < BAND_SIZE; i++) {
                if (mBandFocusItem[i] != null) {
                    mBandFocusItem[i].setEnabled(band == i);
                }
            }
        }
    }

    /**
     * 动动态禁用band按钮，避免点击太快出现波段和频点不对应[20240219]
     * @param enable
     */
    private void enableAllBandCtrl(boolean enable) {
        if (mBandFocusItem != null) {
            for (int i = 0; i < BAND_SIZE; i++) {
                if (mBandFocusItem[i] != null) {
                    mBandFocusItem[i].setEnabled(enable);
                }
            }
        }

        if (btnBand != null) {
            btnBand.setEnabled(enable);
        }
    }

    private final class OnBottomClickListener implements View.OnClickListener {
        @SuppressLint("NonConstantResourceId")
        @Override
        public void onClick(View v) {
            // TODO Auto-generated method stub
            // int btnId = v.getId();
            int btnId = SkinUtils.getViewId(v);
            switch (btnId) {
                case R.id.butt_band:
                    enableAllBandCtrl(false);
                    onButtBandEvent();
                    break;
                case R.id.butt_fm_band:
                    onButtBandFMEvent();
                    break;
                case R.id.butt_am_band:
                    onButtBandAMEvent();
                    break;
                case R.id.band_info_fm1:
                    if (mFMDCC.mCurrentBand != BAND_FM_1) {
                        enableBandCtrl(BAND_FM_1);
                        mFMDCC.Band(BAND_FM_1);
                    }
                    break;
                case R.id.band_info_fm2:
                    if (mFMDCC.mCurrentBand != BAND_FM_2) {
                        enableBandCtrl(BAND_FM_2);
                        mFMDCC.Band(BAND_FM_2);
                    }
                    break;
                case R.id.band_info_fm3:
                    if (mFMDCC.mCurrentBand != BAND_FM_3) {
                        enableBandCtrl(BAND_FM_3);
                        mFMDCC.Band(BAND_FM_3);
                    }
                    break;
                case R.id.band_info_am1:
                    if (mFMDCC.mCurrentBand != BAND_AM_1) {
                        enableBandCtrl(BAND_AM_1);
                        mFMDCC.Band(BAND_AM_1);
                    }
                    break;
                case R.id.butt_scan:
                    onButtScanEvent();
                    break;
                case R.id.butt_asps:
                    if (E_THEME_GOD == ThemeID.E_THEME_ID_200
                            || E_THEME_GOD == ThemeID.E_THEME_ID_201) {
                        onButtASEvent();
                    } else {
                        onButtPSEvent();
                    }
                    break;
                case R.id.butt_eq:
                    onButtEqEvent();
                    break;
                case R.id.butt_locdx:
                    onButtLocDxEvent();
                    break;
                case R.id.btn_seek_up:
                    onButtManulUpEvent();
                    break;
                case R.id.btn_seek_down:
                    onButtManulDownEvent();
                    break;
                case R.id.butt_keypad:
                    if(SkinUtils.getCurrentSkinID().equals(SkinID.SKIN_SA133) || SkinUtils.getCurrentSkinID().equals(SkinID.SKIN_SA155)){
                        ShowKeyBoardPopupWindow();
                        break;
                    }
                    if (mKeyBoard == null) {
                        mKeyBoard = new KeyBoardDialog(mRadioMain, R.layout.dialog_keyboard, mFMDCC);
                    }
                    if (E_THEME_GOD == ThemeID.E_THEME_ID_153) {
                        mKeyBoard.setAnimResId(R.style.KeyboardAnimation);
                        mKeyBoard.setGravity(Gravity.CENTER_VERTICAL | Gravity.RIGHT);
                    }
                    mKeyBoard.show();
                    break;
                case R.id.auto_test:
                    mAutoTestNum += 0x01;
                    if (mAutoTestNum == 0x01) {
                        mBeginTime = System.currentTimeMillis();
                    }
                    if (mAutoTestNum == 0x05) {
                        if (System.currentTimeMillis() - mBeginTime < 15000) {
                            Intent intent = new Intent(Intent.ACTION_MAIN);
                            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                            ComponentName componentName = new ComponentName("com.hcn.changedapp",
                                    "com.hcn.changedapp.TestActivity");
                            intent.setComponent(componentName);
                            mRadioMain.startActivity(intent);
                        }
                        mAutoTestNum = 0;
                        mBeginTime = 0;
                    }
                    break;
                case R.id.butt_bottom_next:
                    onButtSeekDownEvent();
                    break;
                case R.id.butt_bottom_prev:
                    onButtSeekUpEvent();
                    break;
                case R.id.butt_back:
                    mRadioMain.finish();
                    break;
                case R.id.btn_preset_down:
                    if (mFMDCC != null) {
                        mFMDCC.presetDown();
                    }
                    break;
                case R.id.btn_preset_up:
                    if (mFMDCC != null) {
                        mFMDCC.presetUp();
                    }
                    break;
                case R.id.butt_mute:
                    onButtMuteEvent();
                    break;
                case R.id.butt_edit_name:
                    if (FMDataControl.CONFIG_TEXTVIEW_PS_EDIT_NAME) {
                        showEditPSDialog(mRadioMain, String.valueOf(mFMDCC.currentFreq()));
                    }
                    break;
                case R.id.butt_collect:
                    dealCollectFreq();
                    break;
                case R.id.rb_radio_freq_preset:
                    showView(true, mPresetViewPaper);
                    showView(false, mCollectListView);
                    break;
                case R.id.rb_radio_freq_favorite:
                    showView(false, mPresetViewPaper);
                    showView(true, mCollectListView);
                    UpdateUICollectState();
                    break;
                case R.id.butt_setting:
                    ShowSettingWindow();
                    break;
                case R.id.butt_as:
                    onButtASEvent();
                    break;
                case R.id.btn_setting:
                    RadioUtils.onButtSettingEvent(mContext, SETTINGS_PACKAGE_NAME, SETTINGS_ACTIVITY_NAME);
                    break;
                case R.id.butt_favorite_list:
                    initCollectionDialog();
                    showCollectionWindow();
                    updateCollectList1();
                    break;
                case R.id.butt_am_fm:
                    if (mFMDCC.isFMBand()) {
                        onButtBandAMEvent();
                        buttAmFm.postDelayed(new Runnable() {
                            @Override
                            public void run() {
                                buttAmFm.setBackgroundResource(SkinUtils.getId(R.drawable.radio_am1));
                                ivRds.setEnabled(false);
                                if (llRdsIcon.getVisibility() == View.VISIBLE) {
                                    llRdsIcon.setVisibility(View.GONE);
                                }
                            }
                        },800);
                    } else {
                        onButtBandFMEvent();
                        buttAmFm.postDelayed(new Runnable() {
                            @Override
                            public void run() {
                                buttAmFm.setBackgroundResource(SkinUtils.getId(R.drawable.radio_fm1));
                                ivRds.setEnabled(true);
                                if (llRdsIcon.getVisibility() == View.GONE) {
                                    llRdsIcon.setVisibility(View.VISIBLE);
                                }
                            }
                        },800);
                    }
                    break;
                case R.id.btn_radio_setting:
                    if (ThemeUtilsEx.isSm6225()) {
                        ShowRadioSettingWindow();
                    }
                    break;
                default:
                    break;
            }
        }
    }

    private void ShowRadioSettingWindow(){
        if (radioSettingWin == null) {
            mRadioSettingView = LayoutInflater.from(mContext).inflate(SkinUtils.getId(R.layout.radio_setting_popuwindow), null);
            radioSettingWin = new PopupWindow(mRadioSettingView, ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT);
            radioSettingWin.setClippingEnabled(false);
            mOnRadioSettingClickListener = new OnRadioSettingClickListener();

            tvDefault = mRadioSettingView.findViewById(SkinUtils.getId(R.id.tv_default));
            if (null != tvDefault) {
                tvDefault.setOnClickListener(mOnRadioSettingClickListener);
            }
            btnAmDown = mRadioSettingView.findViewById(SkinUtils.getId(R.id.btn_am_down));
            if (null != btnAmDown) {
                btnAmDown.setOnClickListener(mOnRadioSettingClickListener);
            }
            tvAmSize = mRadioSettingView.findViewById(SkinUtils.getId(R.id.tv_am_size));
            if (null != tvAmSize) {
                String amSensitivityUp = String.valueOf(Settings.System.getInt(mContext.getContentResolver(), HConfig.radio_am_rssi,0));
                tvAmSize.setText(amSensitivityUp);
            }
            btnAmUp = mRadioSettingView.findViewById(SkinUtils.getId(R.id.btn_am_up));
            if (null != btnAmUp) {
                btnAmUp.setOnClickListener(mOnRadioSettingClickListener);
            }
            btnFmDown = mRadioSettingView.findViewById(SkinUtils.getId(R.id.btn_fm_down));
            if (null != btnFmDown) {
                btnFmDown.setOnClickListener(mOnRadioSettingClickListener);
            }
            btnFmUp = mRadioSettingView.findViewById(SkinUtils.getId(R.id.btn_fm_up));
            if (null != btnFmUp) {
                btnFmUp.setOnClickListener(mOnRadioSettingClickListener);
            }
            tvFmSize = mRadioSettingView.findViewById(SkinUtils.getId(R.id.tv_fm_size));
            if (null != tvFmSize) {
                String fmSensitivity = String.valueOf(Settings.System.getInt(mContext.getContentResolver(), HConfig.radio_fm_rssi,0));
                tvFmSize.setText(fmSensitivity);
            }
            sensitivitySwitch = mRadioSettingView.findViewById(SkinUtils.getId(R.id.sensitivity_switch));
            if (null != sensitivitySwitch) {
                sensitivitySwitch.setOnClickListener(mOnRadioSettingClickListener);
                SharedPreferences per = mContext.getSharedPreferences("date",Context.MODE_PRIVATE);
                int Style = per.getInt("is_change_rssi", Integer.parseInt("0"));
                if (Style == 1) {
                    sensitivitySwitch.setChecked(true);
                    isRssiChange = 1;
                } else {
                    isRssiChange = 0;
                    sensitivitySwitch.setChecked(false);
                }
            }
        }
        radioSettingWin.showAtLocation(mXmlLayoutView, Gravity.CENTER, 0, 0);
    }

    private final class OnRadioSettingClickListener implements View.OnClickListener {
        @SuppressLint("NonConstantResourceId")
        @Override
        public void onClick(View v) {
            int viewId = SkinUtils.getViewId(v);
            switch (viewId) {
                case R.id.tv_default:
                    if (isRssiChange == 1) {
                        int amSensitivity = 30;//AM一键默认值
                        int fmSensitivity = 20;//FM一键默认值
                        mFMDCC.setFMRssiThreshold(fmSensitivity);
                        mFMDCC.setAMRssiThreshold(amSensitivity);
                        tvAmSize.setText(String.valueOf(amSensitivity));
                        tvFmSize.setText(String.valueOf(fmSensitivity));
                    }
                    break;
                case R.id.btn_am_down:
                    if (isRssiChange == 1) {
                        int amSensitivity = Settings.System.getInt(mContext.getContentResolver(), HConfig.radio_am_rssi,0);
                        amSensitivity --;
                        if (amSensitivity <= 60 && 20 <= amSensitivity) {
                            tvAmSize.setText(String.valueOf(amSensitivity));
                            mFMDCC.setAMRssiThreshold(amSensitivity);
                        }
                    }
                    break;
                case R.id.btn_am_up:
                    if (isRssiChange == 1) {
                        int amSensitivityUp = Settings.System.getInt(mContext.getContentResolver(), HConfig.radio_am_rssi,0);
                        amSensitivityUp ++;
                        if (amSensitivityUp <= 60 && 20 <= amSensitivityUp) {
                            tvAmSize.setText(String.valueOf(amSensitivityUp));
                            mFMDCC.setAMRssiThreshold(amSensitivityUp);
                        }
                    }
                    break;
                case R.id.btn_fm_down:
                    if (isRssiChange == 1) {
                        int fmSensitivity = Settings.System.getInt(mContext.getContentResolver(), HConfig.radio_fm_rssi,0);
                        fmSensitivity --;
                        if (fmSensitivity <= 50 && 10 <= fmSensitivity) {
                            tvFmSize.setText(String.valueOf(fmSensitivity));
                            mFMDCC.setFMRssiThreshold(fmSensitivity);
                        }
                    }
                    break;
                case R.id.btn_fm_up:
                    if (isRssiChange == 1) {
                        int fmSensitivityUp = Settings.System.getInt(mContext.getContentResolver(), HConfig.radio_fm_rssi,0);
                        fmSensitivityUp ++;
                        if (fmSensitivityUp <= 50 && 10 <= fmSensitivityUp) {
                            tvFmSize.setText(String.valueOf(fmSensitivityUp));
                            mFMDCC.setFMRssiThreshold(fmSensitivityUp);
                        }
                    }
                    break;
                case R.id.sensitivity_switch:
                    SharedPreferences.Editor editor =  mContext.getSharedPreferences("date",Context.MODE_PRIVATE).edit();
                    if (sensitivitySwitch.isChecked()) {
                        isRssiChange = 1;
                    } else {
                        isRssiChange = 0;
                    }
                    editor.putInt("is_change_rssi",isRssiChange );
                    editor.apply();
                    break;
                default:
                    break;
            }
        }
    }
    private void initCollectionDialog() {
        View view = LayoutInflater.from(mContext).inflate(SkinUtils.getId(R.layout.collection_dialog), null);
        if (view != null) {
            mCollectionDialog = new PopupWindow(view, ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);
            mCollectionDialog.setOutsideTouchable(true);
            mCollectionDialog.setFocusable(true);
            mCollectionDialog.setAnimationStyle(R.style.PopupAnimation);
            radioCollectionList = view.findViewById(SkinUtils.getId(R.id.radio_collection_list));
            favouritesEmpty = view.findViewById(SkinUtils.getId(R.id.tv_favourites_empty));
            mCollectionAdapter = new CollectionAdapter(mContext,null);
            List<String> collectList = mFMDCC.getCollectedFreqList(mFMDCC.currentBand());
            if (collectList.isEmpty() && favouritesEmpty != null && radioCollectionList != null) {
                favouritesEmpty.setVisibility(View.VISIBLE);
                radioCollectionList.setVisibility(View.GONE);
            } else {
                favouritesEmpty.setVisibility(View.GONE);
                radioCollectionList.setVisibility(View.VISIBLE);
            }
            radioCollectionList.setAdapter(mCollectionAdapter);
            radioCollectionList.setOnItemClickListener(mCollectionListClickListener);
        }
    }

    private void showCollectionWindow() {

        if (mCollectionDialog != null) {
            mCollectionDialog.showAtLocation(mXmlLayoutView, Gravity.RIGHT, 0, -30);
        }
    }
    private void onButtMuteEvent() {
        McuManager.getsInstance().injectKeyEventTimeout(K_MUTE, 50);
    }

    // add by fanguoqing
    public void onButtBandFMEvent() {
        if (mFMDCC != null) {
            if (mFMDCC.currentBand() >= BAND_FM_3) {
                mFMDCC.Band(BAND_FM_1);
            } else {
                mFMDCC.Band((mFMDCC.currentBand() + 1));
            }
        }
    }

    public void onButtBandAMEvent() {
        if (mFMDCC != null) {
            mFMDCC.Band(BAND_AM_1);
        }
    }

    private final class OnButtLongClickListener implements View.OnLongClickListener {
        @SuppressLint("NonConstantResourceId")
        @Override
        public boolean onLongClick(View v) {
            //int btnId = v.getId();
            int btnId = SkinUtils.getViewId(v);
            switch (btnId) {
                case R.id.butt_asps:
                    if (E_THEME_GOD == ThemeID.E_THEME_ID_200
                            || E_THEME_GOD == ThemeID.E_THEME_ID_201) {
                        onButtPSEvent();
                    } else {
                        onButtASEvent();
                    }
                    break;
                case R.id.btn_seek_up:
                case R.id.btn_preset_up:
                    onButtSeekUpEvent();
                    break;
                case R.id.btn_seek_down:
                case R.id.btn_preset_down:
                    onButtSeekDownEvent();
                    break;
                case R.id.butt_scan:
                    if (mFMDCC.isOverseasVersion()) {

                    } else {
                        onButtASEvent();
                    }
                    break;
                case R.id.butt_scan_ru://俄罗斯客户订制
                    onButtScanEvent();
                    break;
                default:
                    break;
            }

            return true;
        }

    }

    public void onButtBandEvent() {
        if (mFMDCC != null) {
            mFMDCC.Band((mFMDCC.currentBand() + 1) % BAND_SIZE);
        }
    }

    public void onButtASEvent() {
        if (mFMDCC != null) {
            mFMDCC.AS();
        }
    }

    public void onButtPSEvent() {
        if (mFMDCC != null) {
            mFMDCC.PS();
        }
    }

    public void onButtEqEvent() {
        McuManager.getsInstance().injectKeyEventTimeout(K_EQ, 50);
    }

    public void onButtLocDxEvent() {
        if (mFMDCC != null) {
            mFMDCC.Local();
        }
    }

    public void onButtSeekDownEvent() {
        if (mFMDCC != null) {
            mFMDCC.seekDown();
        }
    }

    public void onButtSeekUpEvent() {
        if (mFMDCC != null) {
            mFMDCC.seekUp();
        }
    }

    public void onButtManulUpEvent() {
        if (mFMDCC != null) {
            mFMDCC.stepUp();
        }
    }

    public void onButtManulDownEvent() {
        if (mFMDCC != null) {
            mFMDCC.stepDown();
        }
    }

    public void onButtScanEvent() {
        if (mFMDCC != null) {
            mFMDCC.scan();
        }
    }

    @Override
    public void onStart() {

    }

    @Override
    public void onResume() {
        Log.d(TAG, "onResume");
        int dayNight = Settings.System.getInt(mContext.getContentResolver(), auto_setting_day_night_mode, 3);
        if (SkinUtils.getCurrentSkinID().equals(SkinID.SKIN_N91)) {
            if (dayNight == 0) {
                switchModeClosePop();
                initView();
                window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
            } else if (dayNight == 1) {
                switchModeClosePop();
                initView();
                window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_VISIBLE);
            }
        }
        sdk23Permission();
        isDisplayEQ();
        int result = RadioAudioManager.getInstance().requestAudioFocus(AudioManager.AUDIOFOCUS_GAIN,
                AudioAttributes.USAGE_MEDIA, AudioAttributes.CONTENT_TYPE_MUSIC);
        if (AUDIOFOCUS_REQUEST_FAILED != result) {
            RadioAudioManager.getInstance().registerMediaButtonEvent();
        }
        mAutoTestNum = 0;
        refresh();
    }

    private void refresh() {
        if (null != mFMSeekBar) {
            mFMSeekBar.resetFMSeekbarData(getSeekbarViewUID());
            mFMSeekBar.monitorThreadStart();
        }

        mUpdateUIListener.updateDataInfo();

        if (processRdsUiHandler != null) {
            processRdsUiHandler.removeMessages(INIT_DRAG_CELL_POSITION);
            processRdsUiHandler.sendEmptyMessage(INIT_DRAG_CELL_POSITION);
        }

        if (mRadioIconAnim != null) {
            if (mRadioIconAnim.isRunning()) {
                mRadioIconAnim.stop();
            }
            mRadioIconAnim.start();
        }

        if (null != roundKnobSeekBarView) {
            roundKnobSeekBarView.setRoundKnobScrollValue(mFMDCC.isFMBand());
        }
    }

    @Override
    public void onRestart() {

    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause");
        if (null != mFMSeekBar) {
            mFMSeekBar.monitorThreadStop();
        }
    }

    @Override
    public void onStop() {
        Log.d(TAG, "onStop");
        if (processRdsUiHandler != null) {
            processRdsUiHandler.removeMessages(INIT_DRAG_CELL_POSITION);
        }
        if (mHandler != null) {
            mHandler.removeMessages(MSG_WHAT_GOTO_FREQ);
            mHandler.removeMessages(MSG_UPDATE_COLLECT_LIST);
            mHandler.removeMessages(MSG_UPDATE_FREQ_LIST_LOGO);
        }
        if (keyCodeQueue != null) {
            keyCodeQueue.clear();
        }
        if (mRadioIconAnim != null && mRadioIconAnim.isRunning()) {
            mRadioIconAnim.stop();
        }
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        mRadioMain.unregisterReceiver(mBroadcastReceiver);

        // remove from list
        mFmApp.removeActivity(mRadioMain);
        if (mPresetViewPaper != null) {
            mPresetViewPaper.removeOnPageChangeListener(mPageChangeListener);
        }
        // close update callback
        if (null != mFMDCC) {
            mFMDCC.unRegisterDataChangeListener(TAG);
        }

        if (null != mFMSeekBar) {
            mFMSeekBar.recycle();
        }

        // modify by Xiao Kunyu 修复按返回键时，会闪一下主桌面壁纸的问题
        // 原因：Activity 没有走正常的退出动画
        // 修复方式：等 Activity 正常销毁流程完毕后，再停止服务，等服务销毁流程完毕后，再退出进程
        if (mFMDCC != null && mFMDCC.isExitOnBackKey() && !mRadioMain.isInMultiWindowMode() && mRadioMain.isFinishing()) {
            mFMDCC.closeDataService();
        }
    }

    @Override
    public void onConfigurationChanged() {

    }

    @Override
    public void onBackPressed() {
        if (null != mFMDCC && mFMDCC.isExitOnBackKey()) {
            //返回时发送广播供主界面使用
            Intent intent = new Intent("com.hcn.autoradio");
            intent.putExtra("key" ,"onBackPressed");
            mContext.sendBroadcast(intent);
            mRadioMain.finish();
        }
    }


    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (radioSettingWin != null && radioSettingWin.isShowing()) {
            radioSettingWin.dismiss();
            return true;
        }
        if (keyCode == KeyEvent.KEYCODE_0 || keyCode == KeyEvent.KEYCODE_1
                || keyCode == KeyEvent.KEYCODE_2 || keyCode == KeyEvent.KEYCODE_3
                || keyCode == KeyEvent.KEYCODE_4 || keyCode == KeyEvent.KEYCODE_5
                || keyCode == KeyEvent.KEYCODE_6 || keyCode == KeyEvent.KEYCODE_7
                || keyCode == KeyEvent.KEYCODE_8 || keyCode == KeyEvent.KEYCODE_9) {
            if (null == mFMDCC) {
                return false;
            }

            // 每次接收到数字按键，都丢入队列中，延时处理
            keyCodeQueue.add(keyCode);
            mHandler.removeMessages(MSG_WHAT_GOTO_FREQ);
            mHandler.sendEmptyMessageDelayed(MSG_WHAT_GOTO_FREQ, MSG_DELAYED_GOTO_FREQ);
            return true;
        } else {
            return false;
        }
    }

    @SuppressLint("HandlerLeak")
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            switch (msg.what) {
                case MSG_WHAT_GOTO_FREQ:
                    int nPresetIndex = -1;
                    if (keyCodeQueue.size() == 1) {
                        nPresetIndex = (keyCodeQueue.getFirst() - KeyEvent.KEYCODE_1);
                    } else if (keyCodeQueue.size() == 2) {
                        nPresetIndex = (keyCodeQueue.getFirst() - KeyEvent.KEYCODE_0)
                                * 10
                                + (keyCodeQueue.getLast() - KeyEvent.KEYCODE_1);
                    }
                    if (nPresetIndex >= 0 && nPresetIndex < mFMDCC.BAND_STATION_TOTAL) {
                        int[] presets = mFMDCC.readPresetList(mFMDCC.currentBand());
                        mFMDCC.setFreq(presets[nPresetIndex], nPresetIndex);
                    }

                    keyCodeQueue.clear();
                    break;
                case MSG_UPDATE_COLLECT_LIST:
                    UpdateUICollectState();
                    break;
                case MSG_UPDATE_FREQ_LIST_LOGO:
                    updateFreqListLogo();
                    break;
                default:
                    break;
            }
        }
    };

    private final class ViewPagerOnPageChangeListener implements ViewPager.OnPageChangeListener {
        @Override
        public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {

        }

        @Override
        public void onPageSelected(int position) {
            if (null != mScreenFousItem[position]) {
                mScreenFousItem[position].setChecked(true);
            }
        }

        @Override
        public void onPageScrollStateChanged(int state) {

        }
    }

    @Override
    public void setPresetViewEnabled(boolean bEnabled) {
        mPresetViewEnableClick = bEnabled;
    }

    @Override
    public View[] getCurrentPagePresets() {
        View[] currentPagePresets = new View[RadioData.PAGE_STATION_NUM];
        int startPos = mPresetViewPaper.getCurrentItem() * RadioData.PAGE_STATION_NUM;
        System.arraycopy(mPresetItem, startPos, currentPagePresets, 0, RadioData.PAGE_STATION_NUM);
        return currentPagePresets;
    }

    @Override
    public void setPresetViewAimed(int nIndex, boolean bAimed) {

        if (nIndex > -1 && nIndex < RadioData.PAGE_STATION_NUM) {

            if (null != mPresetViewPaper) {
                int nScreen = mPresetViewPaper.getCurrentItem();

                if (nScreen > -1
                        && nScreen < mFMDCC.BAND_STATION_TOTAL / RadioData.PAGE_STATION_NUM) {
                    nIndex += nScreen * RadioData.PAGE_STATION_NUM;
                    if (null != mPresetItem[nIndex]) {
                        mPresetItem[nIndex].setAimed(bAimed);
                    }
                }
            }
        }
    }

    @SuppressLint("ClickableViewAccessibility")
    @Override
    public void onInitPresetPaper(View view, int position) {
        int[] preset = mFMDCC.readPresetList(mFMDCC.currentBand());
        for (int i = 0; i < RadioData.PAGE_STATION_NUM; i++) {
            int pos = i + position * RadioData.PAGE_STATION_NUM;
            String name = String.format(Locale.ENGLISH, "preset_%02d", i + 1);
            int resID = mRadioMain.getResources().getIdentifier(name, "id", mRadioMain.getPackageName());
            mPresetItem[pos] = view.findViewById(SkinUtils.getId(resID));
            if (mPresetItem[pos] != null) {
                mPresetItem[pos].setOnTouchListener(mPresetOnTouchListener);
                mPresetItem[pos].setOnLongClickListener(mPresetOnLongClickListener);
                updatePresetIndex(pos, preset[pos]);
                setLogo(pos, preset[pos]);
                updatePresetFreq(pos, preset[pos]);
            }
        }
    }

    /**
     * 皮肤包预存频点列表序号
     *
     * @param index
     * @param freq
     */
    private void updatePresetIndex(int index, int freq) {
        if (SkinUtils.useSkinPackage()) {
            updatePresetIndexForSkin(index, freq);
        } else {
            updatePresetIndexForMcc(index, freq);
        }
    }

    /**
     * 皮肤包预存频点列表序号定制
     *
     * @param index
     * @param freq
     */
    private void updatePresetIndexForSkin(int index, int freq) {
        if (null == mPresetItem[index]) {
            return;
        }

        switch (SkinUtils.getCurrentSkinID()) {
            case SkinID.SKIN_SA82:
            case SkinID.SKIN_SA85:
            case SkinID.SKIN_SA87:
            case SkinID.SKIN_ZA09:
            case SkinID.SKIN_ZA10:
            case SkinID.SKIN_ZA12:
            case SkinID.SKIN_ZA36:
            case SkinID.SKIN_ZA37:
            case SkinID.SKIN_ZA39:
            case SkinID.SKIN_SA133:
            case SkinID.SKIN_XT510:
                mPresetItem[index].setIndex(String.valueOf(index + 1));
                break;
            case SkinID.SKIN_DZ17:
                if (isHavePsName(freq)) {
                    mPresetItem[index].setIndex("");
                } else {
                    mPresetItem[index].setIndex(String.format(Locale.ENGLISH, "P%d", index + 1));
                }
                break;
            case SkinID.SKIN_XT144:
                if (isHavePsName(freq)) {
                    mPresetItem[index].setFreqUnit("");
                } else {
                    mPresetItem[index].setIndex(String.format(Locale.ENGLISH, "P%d", index + 1));
                }
                break;
            case SkinID.SKIN_NONE:
            default:
                mPresetItem[index].setIndex(String.format(Locale.ENGLISH, "P%d", index + 1));
                break;
        }
    }

    /**
     * 原MCC UI预存频点列表序号定制
     *
     * @param index
     * @param freq
     */
    private void updatePresetIndexForMcc(int index, int freq) {
        if (null == mPresetItem[index]) {
            return;
        }

        switch (E_THEME_GOD) {
            case ThemeID.E_THEME_ID_153:
            case ThemeID.E_THEME_ID_205:
            case ThemeID.E_THEME_ID_401:
            case ThemeID.E_THEME_ID_405:
                mPresetItem[index].setIndex(String.valueOf(index + 1));
                break;
            case ThemeID.E_THEME_ID_400:
                if (E_THEME_SUB == 23 || E_THEME_SUB == 25 || E_THEME_SUB == 28) {
                    mPresetItem[index].setIndex(String.valueOf(index + 1));
                } else {
                    mPresetItem[index].setIndex(String.format(Locale.ENGLISH, "P%d", index + 1));
                }
                break;
            case ThemeID.E_THEME_ID_200:
            case ThemeID.E_THEME_ID_201:
            case ThemeID.E_THEME_ID_204:
                mPresetItem[index].setIndex("");
                break;
            default:
                //序号加上频点：摩洛哥客户特殊要求20230717
                if (FMDataControl.CONFIG_PRESET_FREQ_REPLACE_PS) {
                    String strFreq = mFMDCC.getFormatFreq(freq, false);
                    if (isHavePsName(freq)) {
                        mPresetItem[index].setIndex(String.format(Locale.ENGLISH, "P%d  %s", index + 1, strFreq));
                    } else {
                        mPresetItem[index].setIndex(String.format(Locale.ENGLISH, "P%d", index + 1));
                    }
                } else {
                    mPresetItem[index].setIndex(String.format(Locale.ENGLISH, "P%d", index + 1));
                }
                break;
        }
    }

    /**
     * 存在PS名称
     *
     * @return
     */
    private boolean isHavePsName(int freq) {
        String strPS = "";
        if (FMDataControl.CONFIG_PRESET_EDIT_NAME) {
            if (mFMDCC.isFMBand()) {
                strPS = mFMDCC.readRdsPs(String.valueOf(freq), "");
            }
        }
        Log.d(TAG, "isHavePsName: strPS=" + strPS);
        return strPS.length() > 0;
    }

    // FMSeekBarView
    public final class AutoFMSeekBarListener implements FMSeekBarListener {

        @Override
        public void onMotionBegin() {

        }

        @Override
        public void onMotionChanged(int uId, int nIndex, boolean bUpdate) {

        }

        @Override
        public void onMotionFinished() {

        }

        @Override
        public void onSetValue(int nSource, Object object, boolean bUpdate) {
            if (object instanceof Integer) {
                int nCurrValue = (Integer) object;
                if (null != mFMDCC) {
                    if (mFMDCC.isFMBand()) {
                        nCurrValue = nCurrValue * FMDataControl.mRadioParameters.FmStep
                                + FMDataControl.mRadioParameters.FmMin;
                    } else {
                        nCurrValue = nCurrValue * FMDataControl.mRadioParameters.AmStep
                                + FMDataControl.mRadioParameters.AmMin;
                    }
                    mFMDCC.setFreq(nCurrValue);
                    Log.i(TAG, "onSetValue--->" + nCurrValue);
                }
            }
        }

        @Override
        public void onScrollThumbCenterLocation(int cx, int cy) {

            if (null != mFmDragControl) {
                if (null != mFMSeekBar) {
                    int[] pos = new int[2];
                    mFMSeekBar.getLocationOnScreen(pos);
                    mFmDragControl.setCurrFreqScaleCenterPos(pos[0] + cx, pos[1] + cy);
                } else {
                    mFmDragControl.setCurrFreqScaleCenterPos(cx, cy);
                }
            }
        }
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        // TODO Auto-generated method stub
        int viewId = SkinUtils.getViewId(v);
        switch (viewId) {
            case R.id.rds_imgbtn:
                ShowRDSPopupWindow();
                break;
            case R.id.btn_set_wallpaper:
                showWallpaperDialog();
                break;
            case R.id.butt_screen1:
                if (mPresetViewPaper != null) {
                    mPresetViewPaper.setCurrentItem(0);
                }
                break;
            case R.id.butt_screen2:
                if (mPresetViewPaper != null) {
                    mPresetViewPaper.setCurrentItem(1);
                }
                break;
            case R.id.butt_screen3:
                if (mPresetViewPaper != null) {
                    mPresetViewPaper.setCurrentItem(2);
                }
                break;
            case R.id.iv_rds:
                if (llRdsIcon.getVisibility() == View.GONE) {
                    N91_TOP = 0;
                    ivRds.setSelected(true);
                    llRdsIcon.setVisibility(View.VISIBLE);
                } else if (llRdsIcon.getVisibility() == View.VISIBLE) {
                    N91_TOP = 1;
                    ivRds.setSelected(false);
                    llRdsIcon.setVisibility(View.GONE);
                }
                SharedPreferences.Editor editor =  mContext.getSharedPreferences("date",Context.MODE_PRIVATE).edit();
                editor.putInt("n91_top_hidden",N91_TOP );
                editor.apply();
                break;
            case R.id.rds_pty_list:
                initPtyDialog();
                showPtyPopupWindow();
                break;
            case R.id.button_tglTA:
                if (buttonTglTA == null) {
                    return;
                }
                if (buttonTglTA.isChecked()) {
                    mFMDCC.setTA(true);
                    buttonTglTA.setChecked(true);
                } else {
                    mFMDCC.setTA(false);
                    buttonTglTA.setChecked(false);
                }
                break;
            case R.id.button_tglAF:
                if (buttonTglAF == null) {
                    return;
                }
                if (buttonTglAF.isChecked()) {
                    mFMDCC.setAF(true);
                    buttonTglAF.setChecked(true);
                } else {
                    mFMDCC.setAF(false);
                    buttonTglAF.setChecked(false);
                }
                break;
            case R.id.btn_reg:
                if (btnReg == null) {
                    return;
                }
                if (btnReg.isSelected()) {
                    mFMDCC.setReg(false);
                    btnReg.setSelected(false);
                } else {
                    mFMDCC.setReg(true);
                    btnReg.setSelected(true);
                }
                break;
            default:
                break;
        }

    }

    private void showPtyPopupWindow() {
        if (mPtyDialog != null) {
            mPtyDialog.showAtLocation(mXmlLayoutView, Gravity.RIGHT, 0, -30);
        }

    }

    private void initPtyDialog() {
        View view = LayoutInflater.from(mContext).inflate(SkinUtils.getId(R.layout.pty_dialog), null);
        if (view != null) {
            mPtyDialog = new PopupWindow(view, ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);
            mPtyDialog.setOutsideTouchable(true);
            mPtyDialog.setFocusable(true);
            mPtyDialog.setAnimationStyle(R.style.PopupAnimation);
            radioPtyList = view.findViewById(SkinUtils.getId(R.id.radio_pty_list));
            mPtyAdapter = new PtyAdapter(mContext,null);
            radioPtyList.setAdapter(mPtyAdapter);
            radioPtyList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                @Override
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                    mFMDCC.setPTY(position);
                    mPtyDialog.dismiss();
                }
            });
        }
    }

    private final class OnRdsClickListener implements View.OnClickListener {
        @SuppressLint("NonConstantResourceId")
        @Override
        public void onClick(View v) {
            int viewId = SkinUtils.getViewId(v);
            switch (viewId) {
                case R.id.dismis_imgbtn:
                    dismissPopupWindow();
                    break;
                case R.id.tglTA:
                    if (tglBtn_ta == null || mFMDCC == null) {
                        return;
                    }
                    if (tglBtn_ta.isChecked()) {
                        mFMDCC.setTA(true);
                        tglBtn_ta.setChecked(true);
                        processRdsUiHandler.sendEmptyMessage(RDS_TA_ON);
                    } else {
                        mFMDCC.setTA(false);
                        tglBtn_ta.setChecked(false);
                        processRdsUiHandler.sendEmptyMessage(RDS_TA_OFF);
                    }
                    break;

                case R.id.tglAF:
                    if (tglBtn_af == null || mFMDCC == null) {
                        return;
                    }
                    if (tglBtn_af.isChecked()) {
                        mFMDCC.setAF(true);
                        tglBtn_af.setChecked(true);
                        processRdsUiHandler.sendEmptyMessage(RDS_AF_ON);
                    } else {
                        mFMDCC.setAF(false);
                        tglBtn_af.setChecked(false);
                        processRdsUiHandler.sendEmptyMessage(RDS_AF_OFF);
                    }
                    break;

                case R.id.id_pty_none:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_none));
                    }
                    if (null != tvPTYMain) {
                        if (isMcc400Mnc021() || isMcc400Mnc030() || isMcc400Mnc039()) {
                            tvPTYMain.setText(SkinUtils.getString(R.string.pty_type_none)); // None
                        } else {
                            tvPTYMain.setText(""); // None
                        }
                    }
                    mFMDCC.setPTY(0);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_news:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_news));
                    }
                    mFMDCC.setPTY(1);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_affairs:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_affairs));
                    }
                    mFMDCC.setPTY(2);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_info:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_info));
                    }
                    mFMDCC.setPTY(3);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_sport:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_sport));
                    }
                    mFMDCC.setPTY(4);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_educate:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_education));
                    }
                    mFMDCC.setPTY(5);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_drama:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_drama));
                    }
                    mFMDCC.setPTY(6);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_culture:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_culture));
                    }
                    mFMDCC.setPTY(7);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_science:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_science));
                    }
                    mFMDCC.setPTY(8);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_varied:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_varied));
                    }
                    mFMDCC.setPTY(9);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_popm:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_popm));
                    }
                    mFMDCC.setPTY(10);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_rockm:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_rockm));
                    }
                    mFMDCC.setPTY(11);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_easym:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_easym));
                    }
                    mFMDCC.setPTY(12);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_lightm:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_lightm));
                    }
                    mFMDCC.setPTY(13);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_classics:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_classics));
                    }
                    mFMDCC.setPTY(14);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_otherm:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_otherm));
                    }
                    mFMDCC.setPTY(15);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_weather:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_weather));
                    }
                    mFMDCC.setPTY(16);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_finance:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_finance));
                    }
                    mFMDCC.setPTY(17);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_children:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_children));
                    }
                    mFMDCC.setPTY(18);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_social:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_social));
                    }
                    mFMDCC.setPTY(19);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_religion:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_religion));
                    }
                    mFMDCC.setPTY(20);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_phone:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_phonein));
                    }
                    mFMDCC.setPTY(21);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_travel:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_travel));
                    }
                    mFMDCC.setPTY(22);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_leisure:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_leisure));
                    }
                    mFMDCC.setPTY(23);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_jazz:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_jazz));
                    }
                    mFMDCC.setPTY(24);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_country:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_country));
                    }
                    mFMDCC.setPTY(25);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_nation:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_nationm));
                    }
                    mFMDCC.setPTY(26);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_oldies:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_oldies));
                    }
                    mFMDCC.setPTY(27);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_folk:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_folk));
                    }
                    mFMDCC.setPTY(28);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_document:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_document));
                    }
                    mFMDCC.setPTY(29);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_test:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_test));
                    }
                    mFMDCC.setPTY(30);
                    dismissPopupWindow();
                    break;

                case R.id.id_pty_alarm:
                    if (null != tvPTY) {
                        tvPTY.setText(SkinUtils.getString(R.string.pty_type_alarm));
                    }
                    mFMDCC.setPTY(31);
                    dismissPopupWindow();
                    break;

                default:
                    break;
            }
        }
    }

    private final class OnKeyBoardClickListener implements View.OnClickListener {
        public void onClick(View v) {
            // int btnId = v.getId();
            int btnId = SkinUtils.getViewId(v);
            switch (btnId) {
                case R.id.key_1:
                    onButtKeyEvent("1");
                    break;
                case R.id.key_2:
                    onButtKeyEvent("2");
                    break;
                case R.id.key_3:
                    onButtKeyEvent("3");
                    break;
                case R.id.key_4:
                    onButtKeyEvent("4");
                    break;
                case R.id.key_5:
                    onButtKeyEvent("5");
                    break;
                case R.id.key_6:
                    onButtKeyEvent("6");
                    break;
                case R.id.key_7:
                    onButtKeyEvent("7");
                    break;
                case R.id.key_8:
                    onButtKeyEvent("8");
                    break;
                case R.id.key_9:
                    onButtKeyEvent("9");
                    break;
                case R.id.key_0:
                    onButtKeyEvent("0");
                    break;
                case R.id.key_del:
                    onButtKeyEvent("del");
                    break;
                case R.id.key_dot:
                    onButtKeyEvent("dot");
                    break;
                case R.id.key_enter:
                    onButtKeyEvent("enter");
                    break;
                default:
                    break;
            }
        }
    }
    private void onButtKeyEvent(String key) {

        if (mTextInput != null) {
            String strInput = mTextInput.getText().toString();

            if (key.equals("1")) {
                strInput += "1";
            } else if (key.equals("2")) {
                strInput += "2";
            } else if (key.equals("3")) {
                strInput += "3";
            } else if (key.equals("4")) {
                strInput += "4";
            } else if (key.equals("5")) {
                strInput += "5";
            } else if (key.equals("6")) {
                strInput += "6";
            } else if (key.equals("7")) {
                strInput += "7";
            } else if (key.equals("8")) {
                strInput += "8";
            } else if (key.equals("9")) {
                strInput += "9";
            } else if (key.equals("0")) {
                strInput += "0";
            } else if (key.equals("dot")) {
                strInput += ".";
            } else if (key.equals("del")) {
                if (strInput.length() > 0) {
                    Log.i(TAG, "input length:" + strInput.length());
                    strInput = (strInput.length() - 1) > 0 ? strInput.substring(0,
                            strInput.length() - 1) : "";
                }
            } else if (key.equals("enter")) {
                Log.i(TAG, "enter input:" + strInput);
                if (strInput != null && !strInput.equals("")) {
                    if (mFMDCC.isFMBand()) {
                        float item = Float.parseFloat(strInput);
                        int nFreq = (int) (item * 1000);
                        mFMDCC.setFreq(nFreq);
                    } else {
                        int nFreq = Integer.parseInt(strInput);
                        mFMDCC.setFreq(nFreq);
                    }
                }
                keyboardWindow.dismiss();
            }

            Log.i(TAG, "update input:" + strInput + " length:" + strInput.length());
            mTextInput.setText(strInput);

            updateKeyboardState(strInput != null ? strInput.length() : 0);
        }
    }

    private void updateKeyboardState(int pos) {
        int nMinFreq, nMaxFreq;

        if (mFMDCC.isFMBand()) {
            nMinFreq = FMDataControl.mRadioParameters.FmMin / 10;
            nMaxFreq = FMDataControl.mRadioParameters.FmMax / 10;
        } else {
            nMinFreq = FMDataControl.mRadioParameters.AmMin;
            nMaxFreq = FMDataControl.mRadioParameters.AmMax;
        }
        Log.i(TAG, "updateKeyboardState:(" + nMinFreq + "," + nMaxFreq + ")" + pos);

        mBtnKey1.setEnabled(false);
        mBtnKey2.setEnabled(false);
        mBtnKey3.setEnabled(false);
        mBtnKey4.setEnabled(false);
        mBtnKey5.setEnabled(false);
        mBtnKey6.setEnabled(false);
        mBtnKey7.setEnabled(false);
        mBtnKey8.setEnabled(false);
        mBtnKey9.setEnabled(false);
        mBtnKey0.setEnabled(false);
        mBtnKeyDot.setEnabled(false);

        String strMin = String.valueOf(nMinFreq);
        String strMax = String.valueOf(nMaxFreq);

        String strInput = mTextInput.getText().toString();

        if (strInput != null) {
            strInput = strInput.replace(".", "");
        }

        if (pos == 0) {
            String strMinNum = String.valueOf(nMinFreq).substring(pos, pos + 1);
            int numMin = Integer.parseInt(strMinNum);
            String strMaxNum = String.valueOf(nMaxFreq).substring(pos, pos + 1);
            int numMax = Integer.parseInt(strMaxNum);
            for (int i = numMin; i <= 9; i++) {
                setKeyEnable(i);
            }
            setKeyEnable(numMax);
        } else {
            String strFirstNum = String.valueOf(strInput).substring(0, 1);
            int numFirst = Integer.parseInt(strFirstNum);

            int nStepPos = 0;
            int nMaxInputLength = 0;
            int nStepValue = 0;

            if (mFMDCC.isFMBand()) {
                if (FMDataControl.mRadioParameters.FmStep / 10 == 5) {
                    nStepPos = (numFirst == 1) ? 4 : 3;
                } else {
                    nStepPos = (numFirst == 1) ? 3 : 2;
                }
                nMaxInputLength = (numFirst == 1) ? 5 : 4;
                nStepValue = FMDataControl.mRadioParameters.FmStep / 10;
            } else {
                if (FMDataControl.mRadioParameters.AmStep == 9) {
                    nStepPos = (numFirst == 1) ? 3 : 2;
                } else {
                    nStepPos = (numFirst == 1) ? 2 : 1;
                }
                nMaxInputLength = (numFirst == 1) ? 4 : 3;
                nStepValue = FMDataControl.mRadioParameters.AmStep;
            }

            if (mFMDCC.isFMBand() && ((pos == 2 && numFirst != 1) || (pos == 3 && numFirst == 1))) {
                mBtnKeyDot.setEnabled(true);
            } else {
                if (mFMDCC.isFMBand()) {
                    if ((numFirst == 1 && pos >= 4) || (numFirst != 1 && pos >= 3)) {
                        pos = pos - 1;
                    }
                }
                int nInputLength = strInput.length();

                if (numFirst == 1 && strMax.startsWith(strInput) && pos < nStepPos) {
                    if (nInputLength >= nMaxInputLength) {
                        Log.e(TAG, "Max Input");
                        return;
                    }
                    String strMaxNum = String.valueOf(nMaxFreq).substring(nInputLength,
                            nInputLength + 1);
                    int numMax = Integer.parseInt(strMaxNum);
                    Log.i(TAG, "can set Max to:" + numMax);
                    for (int i = 0; i <= numMax; i++) {
                        setKeyEnable(i);
                    }
                } else if (strMin.startsWith(strInput) && pos < nStepPos) {
                    if (nInputLength >= nMaxInputLength) {
                        Log.e(TAG, "Max Input");
                        return;
                    }
                    String strMaxNum = String.valueOf(nMinFreq).substring(nInputLength,
                            nInputLength + 1);
                    int numMax = Integer.parseInt(strMaxNum);
                    Log.i(TAG, "can set Min from:" + numMax);
                    for (int i = numMax; i <= 9; i++) {
                        setKeyEnable(i);
                    }
                } else {
                    Log.i(TAG, "pos:" + pos + " nStepValue:" + nStepValue + " nMaxInputLength:"
                            + nMaxInputLength);
                    if (pos > nStepPos) {
                        if (nStepValue >= 10) {
                            if (pos < nMaxInputLength) {
                                setKeyEnable(0);
                            }
                        }
                    } else if (pos == nStepPos) {
                        for (int i = 0; i <= 9; i++) {
                            String num = strInput + i;
                            if (nStepValue >= 10) {
                                num += "0";
                            }
                            int value = Integer.parseInt(num);
                            if (value < nMinFreq || value > nMaxFreq) {
                                continue;
                            }
                            if ((value - nMinFreq) % nStepValue == 0) {
                                setKeyEnable(i);
                            }
                        }

                    } else {
                        for (int i = 0; i <= 9; i++) {
                            setKeyEnable(i);
                        }
                    }
                }
            }
        }
    }

    private void setKeyEnable(int key) {
        switch (key) {
            case 0:
                mBtnKey0.setEnabled(true);
                break;
            case 1:
                mBtnKey1.setEnabled(true);
                break;
            case 2:
                mBtnKey2.setEnabled(true);
                break;
            case 3:
                mBtnKey3.setEnabled(true);
                break;
            case 4:
                mBtnKey4.setEnabled(true);
                break;
            case 5:
                mBtnKey5.setEnabled(true);
                break;
            case 6:
                mBtnKey6.setEnabled(true);
                break;
            case 7:
                mBtnKey7.setEnabled(true);
                break;
            case 8:
                mBtnKey8.setEnabled(true);
                break;
            case 9:
                mBtnKey9.setEnabled(true);
                break;
            default:
                break;
        }
    }

    public void dismissKeyBoardgWindow() {
        if (keyboardWindow != null && keyboardWindow.isShowing()) {
            keyboardWindow.dismiss();
        }
    }


    public void dismissPopupWindow() {
        if (popupWindow != null && popupWindow.isShowing()) {

            /**
             * za01 dz16 UI的控件
             */
            if (mainInfoView != null) {
                mainInfoView.setVisibility(View.VISIBLE);
            }

            if (mRootLayoutView != null && !RadioUtils.supportWallpaperCustomized()) {
                mRootLayoutView.setBackground(layoutDrawable);
            }

            popupWindow.dismiss();
        }
    }

    @SuppressLint("NonConstantResourceId")
    public void onPresetDoubleClick(View view) {
        // TODO Auto-generated method stub
        if (FMDataControl.CONFIG_PRESET_EDIT_NAME) {
            int PresetIndex = 0;
            if (view != null) {
                int viewId = SkinUtils.getViewId(view);
                switch (viewId) {
                    case R.id.preset_01:
                        PresetIndex =
                                mPresetViewPaper.getCurrentItem() * RadioData.PAGE_STATION_NUM;
                        break;
                    case R.id.preset_02:
                        PresetIndex =
                                1 + mPresetViewPaper.getCurrentItem() * RadioData.PAGE_STATION_NUM;
                        break;
                    case R.id.preset_03:
                        PresetIndex =
                                2 + mPresetViewPaper.getCurrentItem() * RadioData.PAGE_STATION_NUM;
                        break;
                    case R.id.preset_04:
                        PresetIndex =
                                3 + mPresetViewPaper.getCurrentItem() * RadioData.PAGE_STATION_NUM;
                        break;
                    case R.id.preset_05:
                        PresetIndex =
                                4 + mPresetViewPaper.getCurrentItem() * RadioData.PAGE_STATION_NUM;
                        break;
                    case R.id.preset_06:
                        PresetIndex =
                                5 + mPresetViewPaper.getCurrentItem() * RadioData.PAGE_STATION_NUM;
                        break;
                    default:
                        break;
                }
                int[] presets = mFMDCC.readPresetList(mFMDCC.currentBand());
                showEditPSDialog(mRadioMain, String.valueOf(presets[PresetIndex]));
            }
        }
    }

    @SuppressLint("NonConstantResourceId")
    public void onPresetClick(View view) {
        if (view != null && mPresetViewEnableClick) {
            int prevPresetCount;
            if (null == mPresetViewPaper) {
                prevPresetCount = 0;
            } else {
                prevPresetCount = mPresetViewPaper.getCurrentItem() * RadioData.PAGE_STATION_NUM;
            }
            int viewId = SkinUtils.getViewId(view);
            switch (viewId) {
                case R.id.preset_01:
                    setClickPresetItem(prevPresetCount);
                    break;
                case R.id.preset_02:
                    setClickPresetItem(1 + prevPresetCount);
                    break;
                case R.id.preset_03:
                    setClickPresetItem(2 + prevPresetCount);
                    break;
                case R.id.preset_04:
                    setClickPresetItem(3 + prevPresetCount);
                    break;
                case R.id.preset_05:
                    setClickPresetItem(4 + prevPresetCount);
                    break;
                case R.id.preset_06:
                    setClickPresetItem(5 + prevPresetCount);
                    break;
                case R.id.preset_07:
                    setClickPresetItem(6 + prevPresetCount);
                    break;
                case R.id.preset_08:
                    setClickPresetItem(7 + prevPresetCount);
                    break;
                default:
                    break;
            }
        }
    }

    class PresetOnTouchListener implements View.OnTouchListener {

        private final int DOUBLE_TAP_TIMEOUT = 200;
        private int mPreviousViewId;
        private MotionEvent mPreviousUpEvent;

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            // TODO Auto-generated method stub
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                Log.d(TAG, "onTouch: MotionEvent.ACTION_DOWN");
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                if (mPreviousUpEvent != null && mPreviousViewId == SkinUtils.getViewId(v)
                        && isConsideredDoubleTap(mPreviousUpEvent, event)) {
                    Log.d(TAG, "onTouch: onPresetDoubleClick");
                    onPresetDoubleClick(v);
                } else {
                    Log.d(TAG, "onTouch: onPresetClick");
                    onPresetClick(v);
                }
                mPreviousUpEvent = MotionEvent.obtain(event);
                mPreviousViewId = SkinUtils.getViewId(v);
            }
            return false;
        }

        private boolean isConsideredDoubleTap(MotionEvent firstUp, MotionEvent secondup) {
            if (secondup.getEventTime() - firstUp.getEventTime() > DOUBLE_TAP_TIMEOUT) {
                return false;
            }
            int deltaX = (int) firstUp.getX() - (int) secondup.getX();
            int deltaY = (int) firstUp.getY() - (int) secondup.getY();
            return deltaX * deltaX + deltaY * deltaY < 10000;
        }
    }


    class PresetLongClickListener implements View.OnLongClickListener {
        @SuppressLint("NonConstantResourceId")
        @Override
        public boolean onLongClick(View v) {
            if (v != null) {
                int prevPresetCount;
                if (null == mPresetViewPaper) {
                    prevPresetCount = 0;
                } else {
                    prevPresetCount =
                            mPresetViewPaper.getCurrentItem() * RadioData.PAGE_STATION_NUM;
                }
                int viewId = SkinUtils.getViewId(v);
                switch (viewId) {
                    case R.id.preset_01:
                        onLongClickPresetItem(prevPresetCount);
                        break;
                    case R.id.preset_02:
                        onLongClickPresetItem(1 + prevPresetCount);
                        break;
                    case R.id.preset_03:
                        onLongClickPresetItem(2 + prevPresetCount);
                        break;
                    case R.id.preset_04:
                        onLongClickPresetItem(3 + prevPresetCount);
                        break;
                    case R.id.preset_05:
                        onLongClickPresetItem(4 + prevPresetCount);
                        break;
                    case R.id.preset_06:
                        onLongClickPresetItem(5 + prevPresetCount);
                        break;
                    case R.id.preset_07:
                        onLongClickPresetItem(6 + prevPresetCount);
                        break;
                    case R.id.preset_08:
                        onLongClickPresetItem(7 + prevPresetCount);
                        break;
                    default:
                        break;
                }
            }
            return false;
        }
    }

    public void sdk23Permission() {
        if (Build.VERSION.SDK_INT >= 23) {
            if (!Settings.canDrawOverlays(mRadioMain)) {
                Intent intent = new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION,
                        Uri.parse("package:" + mRadioMain.getPackageName()));
                mRadioMain.startActivityForResult(intent, ALERT_WINDOW_PERMISSION_CODE);
            }
        }
    }

    /**
     * 用户返回
     */
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (Build.VERSION.SDK_INT >= 23 && requestCode == ALERT_WINDOW_PERMISSION_CODE) {
            if (!Settings.canDrawOverlays(mRadioMain)) {
                mFMDCC.closeDataService();
                mRadioMain.finish();
            }
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        Log.d(TAG, "onConfigurationChanged");
        int dayNight = Settings.System.getInt(mContext.getContentResolver(), auto_setting_day_night_mode,3);

        if (dayNight == 2 && SkinUtils.getCurrentSkinID().equals(SkinID.SKIN_N91)) {
            if (RadioUtils.isNightMode(newConfig)) {
                window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_VISIBLE);
            } else {
                window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
            }
            switchModeClosePop();
        }
       initView();
    }

    /*切换白天黑夜关闭弹框*/
    private void switchModeClosePop(){
        if (mCollectionDialog != null && mCollectionDialog.isShowing()) {
            mCollectionDialog.dismiss();
        }
        if (mPtyDialog != null && mPtyDialog.isShowing()) {
            mPtyDialog.dismiss();
        }
        dismissPopupWindow();

    }

    @Override
    public void onMultiWindowModeChanged(boolean isInMultiWindowMode, Configuration newConfig) {
        Log.d(TAG, "onMultiWindowModeChanged: " + isInMultiWindowMode);
    }

    private void onVolumeChanged(int streamVolume) {
        if (btnMute != null && streamVolume >= 0) {
            if (streamVolume == 0) {
                btnMute.setImageResource(SkinUtils.getId(R.drawable.butt_xml_mute));
            } else {
                btnMute.setImageResource(SkinUtils.getId(R.drawable.butt_xml_volume));
            }
        }
    }

    private void showEditPSDialog(Context context, final String freq) {
        final EditText editText = new EditText(context);
        editText.setSingleLine(true);
        editText.setText(mFMDCC.readRdsPs(freq, ""));
        editText.setSelection(editText.getText().toString().length());
        editText.setFilters(new InputFilter[]{new InputFilter.LengthFilter(10)});
        editText.setImeOptions(EditorInfo.IME_ACTION_DONE);
        editText.setInputType(EditorInfo.TYPE_CLASS_TEXT);

        final AlertDialog.Builder builder = new AlertDialog.Builder(context, 3);
        builder.setView(editText);
        builder.setPositiveButton(android.R.string.ok,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        String string = editText.getText().toString();
                        if ("".equals(string)) {
                            mFMDCC.deleteRdsPs(freq);
                            mFMDCC.deleteRdsPs(freq + "edit");
                        } else {
                            mFMDCC.writeRdsPs(freq + "edit", string);
                        }
                        if (FMDataControl.CONFIG_PRESET_EDIT_NAME) {
                            mUpdateUIListener.updateFreqList();
                        }
                        if (FMDataControl.CONFIG_TEXTVIEW_PS_EDIT_NAME) {
                            updateFreqName();
                        }
                    }
                });
        builder.setNegativeButton(android.R.string.cancel,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.cancel();
                    }
                });
        builder.create().show();
    }

    void updateFreqName() {
        if (null != tvRDS_PS) {
            tvRDS_PS.setText(mFMDCC.mRdsPS);
        }
        if (FMDataControl.CONFIG_TEXTVIEW_PS_EDIT_NAME) {
            String editName = mFMDCC.readRdsPs(String.valueOf(mFMDCC.currentFreq()), "");
            if (null != tvEdit_rds_ps) {
                if (TextUtils.isEmpty(editName) || SkinUtils.getCurrentSkinID().equals(SkinID.SKIN_XT144)) {
                    tvEdit_rds_ps.setText(mFMDCC.mRdsPS);
                } else {
                    tvEdit_rds_ps.setText(editName);
                }
            }
        }
    }

    /**
     * 试图显示切换调用
     *
     * @param isShow
     * @param view
     */
    private void showView(boolean isShow, View view) {
        if (view != null) {
            if (isShow) {
                if (view.getVisibility() != View.VISIBLE) {
                    view.setVisibility(View.VISIBLE);
                }
            } else {
                if (view.getVisibility() != View.GONE) {
                    view.setVisibility(View.GONE);
                }
            }
        } else {
            Log.d(TAG, "showView view is null");
        }
    }

    /**
     * 收藏列表显示与隐藏
     *
     * @param show
     */
    private void showCollectList(boolean show) {
        if (mCollectListView != null) {
            mCollectListView.setVisibility(show ? View.VISIBLE : View.GONE);
        }
    }

    /**
     * 电台收藏处理函数
     */
    private void dealCollectFreq() {
        if (mFMDCC != null) {
            String mCurBandFreq = mFMDCC.getCurBandFreq();
            boolean isCollected = mFMDCC.isCollectedFreq(mFMDCC.currentBand(), mCurBandFreq);

            boolean isAS = mFMDCC.isAS();
            boolean isScan = mFMDCC.isScan();
            boolean isSeek = mFMDCC.isSeek();
            if (!isCollected) {
                if (isAS || isScan || isSeek) {
                    Log.d(TAG, "dealCollect return just isAS=" + isAS + " isScan=" + isScan + " isSeek=" + isSeek);
                    return;
                }
                Log.d(TAG, "dealCollect  mCurBandFreq = " + mCurBandFreq);
                if (mFMDCC.collectFreq(mFMDCC.currentBand(), mCurBandFreq)) {
                    UpdateUICollectState();
                }
            } else {
                Log.d(TAG, "dealCollect uncollected  mCurBandFreq=" + mCurBandFreq);
                if (mFMDCC.deleteCollectFreq(mFMDCC.currentBand(), mCurBandFreq)) {
                    UpdateUICollectState();
                }
            }
        }
    }

    /**
     * 更新收藏相关控件状态
     */
    private void UpdateUICollectState() {
        if (SkinUtils.getCurrentSkinID().equals(SkinID.SKIN_N91)) {
            updateCollectList1();
        } else {
            updateCollectList();
        }
        updateCollectBtn();
    }

    /**
     * 更新收藏按钮状态
     */
    private void updateCollectBtn() {
        if (mCollectionBtn != null && mFMDCC != null) {
            String mCurBandFreq = mFMDCC.getCurBandFreq();
            boolean isCollected = mFMDCC.isCollectedFreq(mFMDCC.currentBand(), mCurBandFreq);
            Log.d(TAG, "updateCollectBtn isCollected=" + isCollected + " mCurBandFreq=" + mCurBandFreq);
            if (E_THEME_GOD == 405 && E_THEME_SUB == 1) {
                if (!isCollected) {
                    ((ImageView) mCollectionBtn).setImageResource(R.drawable.radio_collect_n);
                } else {
                    ((ImageView) mCollectionBtn).setImageResource(R.drawable.radio_collect_p);
                }
            } else {
                if (!isCollected) {
                    ((ImageButton) mCollectionBtn).setSelected(false);
                } else {
                    ((ImageButton) mCollectionBtn).setSelected(true);
                }
            }

        }
    }

    /**
     * 更新收藏列表显示
     */
    private void updateCollectList() {
        if (mCollectListView != null && mCollectListView.getVisibility() == View.VISIBLE &&
                mFMDCC != null && mCollectionListAdapter != null) {
            List<String> collectList = mFMDCC.getCollectedFreqList(mFMDCC.currentBand());
            if (collectList != null) {
                Log.d(TAG, "collectList size = " + collectList.size());
                mCollectionListAdapter.updateListData(collectList);
            }
            updateSelectState();
        }
    }

    /**
     * 更新收藏列表选中状态
     */
    private void updateSelectState() {
        if (mFMDCC != null && mCollectionListAdapter != null) {
            mCollectionListAdapter.updateCurFreq(mFMDCC.getCurBandFreq());
        }
    }

    private void updateCollectList1() {

        if ((radioCollectionList != null && mFMDCC != null && mCollectionAdapter != null)) {
            Log.d(TAG, "updateCollectList1: ");
            List<String> collectList = mFMDCC.getCollectedFreqList(mFMDCC.currentBand());
            if (collectList != null) {
                Log.d(TAG, "collectList size = " + collectList.size());
                mCollectionAdapter.updateListData(collectList);
            }
        }
        updateSelectState1();
    }

    private void updateSelectState1() {
        if (mFMDCC != null && mCollectionAdapter != null) {
            Log.d(TAG, "updateSelectState1: ");
            mCollectionAdapter.updateCurFreq(mFMDCC.getCurBandFreq());
        }
    }

    /**
     * mcc400-mnc021特殊定制
     */
    private boolean isMcc400Mnc021() {
        return !SkinUtils.useSkinPackage() && E_THEME_GOD == 400 && E_THEME_SUB == 21;
    }

    private boolean isMcc400Mnc030() {
        return !SkinUtils.useSkinPackage() && E_THEME_GOD == 400 && E_THEME_SUB == 30;
    }

    private boolean isMcc400Mnc039() {
        return !SkinUtils.useSkinPackage() && E_THEME_GOD == 400 && E_THEME_SUB == 39;
    }

    private boolean isMcc400Mnc040() {
        return !SkinUtils.useSkinPackage() && E_THEME_GOD == 400 && E_THEME_SUB == 40;
    }

    private boolean isRK01(){
        return SkinUtils.getCurrentSkinID().equals(SkinID.SKIN_RK01);
    }

    /**
     * mcc400-mnc021特殊定制:RDS设置弹框
     */
    private void ShowSettingWindow() {
        if (null == mFMDCC) {
            return;
        }
        if (settingWindow == null) {
            View settingPopupView = LayoutInflater.from(mContext).inflate(SkinUtils.getId(R.layout.setting_popuwindow), null);
            settingWindow = new PopupWindow(settingPopupView, ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);

            mSettingClickListener = new OnSettingClickListener();

            settingToggleBtnTA = settingPopupView.findViewById(SkinUtils.getId(R.id.setting_tglTA));
            if (null != settingToggleBtnTA) {
                settingToggleBtnTA.setOnClickListener(mSettingClickListener);
            }

            settingToggleBtnAF = settingPopupView.findViewById(SkinUtils.getId(R.id.setting_tglAF));
            if (null != settingToggleBtnAF) {
                settingToggleBtnAF.setOnClickListener(mSettingClickListener);
            }

            settingToggleBtnLOC = settingPopupView.findViewById(SkinUtils.getId(R.id.setting_tglLOC));
            if (null != settingToggleBtnLOC) {
                settingToggleBtnLOC.setOnClickListener(mSettingClickListener);
            }

            settingLocLayout = settingPopupView.findViewById(SkinUtils.getId(R.id.ll_setting_loc));
            settingTaLayout = settingPopupView.findViewById(SkinUtils.getId(R.id.ll_setting_ta));
            settingAfLayout = settingPopupView.findViewById(SkinUtils.getId(R.id.ll_setting_af));
        }

        if (null != settingToggleBtnAF) {
            settingToggleBtnAF.setChecked(mFMDCC.mIsAF);
        }

        if (null != settingToggleBtnTA) {
            settingToggleBtnTA.setChecked(mFMDCC.mIsTA);
        }

        if (null != settingToggleBtnLOC) {
            settingToggleBtnLOC.setChecked(mFMDCC.isLocal());
        }

        if (mFMDCC.mIsSupportRDS && mFMDCC.isFMBand()) {
            if (settingTaLayout != null) {
                settingTaLayout.setVisibility(View.VISIBLE);
            }
            if (settingAfLayout != null) {
                settingAfLayout.setVisibility(View.VISIBLE);
            }
        } else {
            if (settingTaLayout != null) {
                settingTaLayout.setVisibility(View.INVISIBLE);
            }
            if (settingAfLayout != null) {
                settingAfLayout.setVisibility(View.INVISIBLE);
            }
        }

        settingWindow.setOutsideTouchable(false);
        settingWindow.setFocusable(false);
        settingWindow.setAnimationStyle(R.style.PopupAnimation);
        settingWindow.showAtLocation(mXmlLayoutView, Gravity.CENTER, 0, 0);
    }

    /**
     * 隐藏设置弹框
     */
    public void dismissSettingWindow() {
        if (settingWindow != null && settingWindow.isShowing()) {
            settingWindow.dismiss();
        }
    }

    /**
     * 设置弹框的点击事件
     */
    private final class OnSettingClickListener implements View.OnClickListener {
        @SuppressLint("NonConstantResourceId")
        @Override
        public void onClick(View v) {
            int viewId = SkinUtils.getViewId(v);
            switch (viewId) {
                case R.id.setting_tglTA:
                    if (settingToggleBtnTA == null || cbxMainTA == null) {
                        return;
                    }
                    if (settingToggleBtnTA.isChecked()) {
                        mFMDCC.setTA(true);
                        settingToggleBtnTA.setChecked(true);
                        cbxMainTA.setChecked(true);
                    } else {
                        mFMDCC.setTA(false);
                        settingToggleBtnTA.setChecked(false);
                        cbxMainTA.setChecked(false);
                    }
                    break;
                case R.id.setting_tglAF:
                    if (settingToggleBtnAF == null || cbxMainAF == null) {
                        return;
                    }
                    if (settingToggleBtnAF.isChecked()) {
                        mFMDCC.setAF(true);
                        settingToggleBtnAF.setChecked(true);
                        cbxMainAF.setChecked(true);
                    } else {
                        mFMDCC.setAF(false);
                        settingToggleBtnAF.setChecked(false);
                        cbxMainAF.setChecked(false);
                    }
                    break;
                case R.id.setting_tglLOC:
                    if (settingToggleBtnLOC == null) {
                        return;
                    }
                    if (settingToggleBtnLOC.isChecked()) {
                        mFMDCC.setLocal(true);
                        settingToggleBtnLOC.setChecked(true);
                    } else {
                        mFMDCC.setLocal(false);
                        settingToggleBtnLOC.setChecked(false);
                    }
                    break;
                case R.id.checkbox_af:
                    if (cbxMainAF == null) {
                        return;
                    }
                    if (cbxMainAF.isChecked()) {
                        mFMDCC.setAF(true);
                        cbxMainAF.setChecked(true);
                    } else {
                        mFMDCC.setAF(false);
                        cbxMainAF.setChecked(false);
                    }
                    break;
                case R.id.checkbox_ta:
                    if (cbxMainAF == null) {
                        return;
                    }
                    if (cbxMainTA.isChecked()) {
                        mFMDCC.setTA(true);
                        cbxMainTA.setChecked(true);
                    } else {
                        mFMDCC.setTA(false);
                        cbxMainTA.setChecked(false);
                    }
                    break;
                case R.id.checkbox_loc:
                    if (cbxMainLoc == null) {
                        return;
                    }
                    if (cbxMainLoc.isChecked()) {
                        mFMDCC.setLocal(true);
                        cbxMainLoc.setChecked(true);
                    } else {
                        mFMDCC.setLocal(false);
                        cbxMainLoc.setChecked(false);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    // RoundKnobSeekBarView
    public final class RoundKnobSeekBarListener implements RoundKnobSeekBar.OnProgressChangeListener {

        @Override
        public void onProgressChanged(RoundKnobSeekBar seekBar, float progress, boolean fromUser) {
            int nCurrValue = (int) progress;
            Log.d(TAG, "onProgressChanged progress = " + nCurrValue);
            if (null != mFMDCC) {
                if (mFMDCC.isFMBand()) {
                    nCurrValue = nCurrValue * FMDataControl.mRadioParameters.FmStep
                            + FMDataControl.mRadioParameters.FmMin;
                } else {
                    nCurrValue = nCurrValue * FMDataControl.mRadioParameters.AmStep
                            + FMDataControl.mRadioParameters.AmMin;
                }
                Log.i(TAG, "updateDigitFreq--->" + nCurrValue);
                updateDigitFreq(nCurrValue);
            }
        }

        @Override
        public void onStartTrackingTouch(RoundKnobSeekBar seekBar) {

        }

        @Override
        public void onStopTrackingTouch(RoundKnobSeekBar seekBar) {
            int nCurrValue = (int) seekBar.getProgress();
            Log.d(TAG, "onStopTrackingTouch progress = " + nCurrValue);
            if (null != mFMDCC) {
                if (mFMDCC.isFMBand()) {
                    nCurrValue = nCurrValue * FMDataControl.mRadioParameters.FmStep
                            + FMDataControl.mRadioParameters.FmMin;
                } else {
                    nCurrValue = nCurrValue * FMDataControl.mRadioParameters.AmStep
                            + FMDataControl.mRadioParameters.AmMin;
                }
                mFMDCC.setFreq(nCurrValue);
                Log.i(TAG, "onSetValue--->" + nCurrValue);
            }
        }
    }

    /**
     * 更新旋转进度
     */
    private void updateRoundKnobSeekBar() {
        if (null != roundKnobSeekBarView) {
            roundKnobSeekBarView.setProgress(mCurrUnitStep);
        }
    }
}
