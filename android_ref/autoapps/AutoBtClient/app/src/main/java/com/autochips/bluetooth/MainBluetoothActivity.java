package com.autochips.bluetooth;

import static com.hcn.auto.theme.utils.Utils.getSystemProperty;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageView;

import androidx.fragment.app.Fragment;
import androidx.viewpager.widget.ViewPager;

import com.autochips.bluetooth.adapter.EnableScrollViewPager;
import com.autochips.bluetooth.fragment.DialFragment;
import com.autochips.bluetooth.fragment.PBFragment;
import com.autochips.bluetooth.fragment.PairedFragment;
import com.autochips.bluetooth.fragment.PairedFragmentEx;
import com.autochips.bluetooth.fragment.RecordFragment;
import com.autochips.bluetooth.fragment.SetupFragment;
import com.autochips.bluetooth.fragment.SetupFragmentEx;
import com.autochips.bluetooth.skin.FastBlurUtils;
import com.autochips.bluetooth.skin.SkinID;
import com.autochips.bluetooth.skin.SkinUtils;
import com.autochips.bluetooth.skin.ThemeUtilsEx;
import com.autochips.bluetooth.utils.Utility;
import com.autochips.bluetooth.utils.WallpaperUtil;
import com.autochips.bluetooth.viewpager.FragmentViewPaperAdapter;
import com.autochips.bluetooth.viewpager.transforms.DepthPageTransformer;
import com.hcn.auto.utils.HImageUtils;
import com.hcn.bluetooth.api.ConnectionListener;
import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;
import com.hcn.bluetooth.api.Utils;
import com.hcn.skin.support.app.SkinCompatActivity;
import com.hcn.skin.support.resources.SkinCompatResources;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;


public class MainBluetoothActivity extends SkinCompatActivity implements
        ViewPager.OnPageChangeListener, View.OnClickListener, IFragmentCallback {

    private static final String TAG = "MainBluetoothActivity";
    //同行者语音关闭蓝牙指令
    public static final String VOICE_EVENT_BT_EXIT = "com.bt.extra.exit";

    private EnableScrollViewPager mViewPager;

    public static final String NEED_STATUS_BAR_CHANGE = "need_status_bar_change";
    public static final int NO_STATUS_BAR_CHANGE = 0;
    public static final int STATUS_BAR_CHANGE = 1;

    /**
     * setupFragment 是否显示蓝牙网络
     * <p> fix 30261 临时处理方案，后续调整白天黑夜框架可移除
     */
    public static boolean showLayoutNetwork = false;

    /**
     * 当前选中的view
     */
    private View mSelectedView;

    private View mBtnDial;
    private View mBtnContract;
    private View mBtnRecord;
    private View mBtnSetting;
    private View mBtnPaired;

    public static final int PAGE_ONE = 0;
    public static final int PAGE_TWO = 1;
    public static final int PAGE_THREE = 2;
    public static final int PAGE_FOUR = 3;
    public static final int PAGE_FIVE = 4;

    private List<Fragment> fragments = new ArrayList<Fragment>();
    private static final int MSG_UPDATE_CTRL = 0;
    private Handler mHandler = null;
    private FragmentViewPaperAdapter mFragmentViewPaperAdapter;
    private Fragment dialFragment;
    private Fragment recordFragment;
    private LocalBluetoothAdapterManager mAdapterManager;
    private boolean isConnected = false;
    private ViewGroup mainLayout = null;
    private ViewGroup llBottomMenu = null;

    /** 当高斯模糊时用于还原背景 */
    private Drawable resetDrawable = null;

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(LocalBluetoothAdapterManager.ACTION_CONNECTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,
                        BluetoothProfile.STATE_DISCONNECTED);
                if (state == BluetoothProfile.STATE_CONNECTED) {
                    isConnected = true;
                    updateView();
                } else if (state == BluetoothProfile.STATE_DISCONNECTED) {
                    isConnected = false;
                    updateView();
                }
            } else if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE,
                        BluetoothAdapter.STATE_OFF);
                if (state == BluetoothAdapter.STATE_OFF) {
                    isConnected = false;
                    updateView();
                } else if (state == BluetoothAdapter.STATE_ON) {

                }
            } else if (action.equals(VOICE_EVENT_BT_EXIT)) {
                finish();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            Window window = getWindow();
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            window.getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.setStatusBarColor(Color.TRANSPARENT);
        }
        Log.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        mHandler = new MyHandler(this);
        setContentView(R.layout.activity_main);

        init();
        initView();
        selectPage(PAGE_FOUR);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            Window window = getWindow();
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            boolean isNight = isNight(getBaseContext().getResources().getConfiguration());
            //获取皮肤包里面的具体资源情况：判定该皮肤包是否需要状态栏进行背景颜色适配，0：不需要；1需要；
            boolean needStatusBarChange = needStatusBarChange();
            if (needStatusBarChange) {
                if (isNight) {
                    getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_VISIBLE);
                } else {
                    getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
                }
            } else {
                getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
            }
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.setStatusBarColor(Color.TRANSPARENT);
        }

    }

    private static class MyHandler extends Handler {
        private WeakReference<MainBluetoothActivity> mWeakReference;

        public MyHandler(MainBluetoothActivity activity) {
            mWeakReference = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            int what = msg.what;
            switch (what) {
                case MSG_UPDATE_CTRL:
                    MainBluetoothActivity activity = mWeakReference.get();
                    if (activity != null) {
                        activity.updateView();
                    }
                    break;
                default:
                    break;
            }
            super.handleMessage(msg);
        }
    }

    protected void init() {
        mAdapterManager = LocalBluetoothAdapterManager.getInstance().init(this);
        mAdapterManager.addConnectListener(mAdapterListener);
        dialFragment = new DialFragment();
        recordFragment = new RecordFragment();
        PBFragment phonebookFragment = new PBFragment();
        phonebookFragment.setFragmentCallback(this);
        Fragment setupFragment = getSetupFragment();
        Fragment pairedFragment = getPairedFragment();

        fragments.clear();
        fragments.add(dialFragment);
        fragments.add(phonebookFragment);
        fragments.add(recordFragment);
        fragments.add(pairedFragment);
        fragments.add(setupFragment);
    }

    /**
     * 根据配置文件参数生成对应的 pairedFragment
     *
     * @return Fragment
     */
    private Fragment getPairedFragment() {
        Fragment pairedFragment = null;
        if (SkinUtils.getInteger(R.integer.paired_fragment_type) == 1) {
            pairedFragment = new PairedFragmentEx();
            ((PairedFragmentEx) pairedFragment).setFragmentCallback(this);
        } else {
            pairedFragment = new PairedFragment();
        }
        return pairedFragment;
    }

    /**
     * 根据配置文件参数生成对应的 pairedFragment
     *
     * @return Fragment
     */
    private Fragment getSetupFragment() {
        Fragment setupFragment = null;
        if (SkinUtils.getInteger(R.integer.setup_fragment_type) == 1) {
            setupFragment = new SetupFragmentEx();
            ((SetupFragmentEx) setupFragment).setFragmentCallback(this);
        } else {
            setupFragment = new SetupFragment();
        }
        return setupFragment;
    }

    private void updateView() {
        if (null == mViewPager) {
            return;
        }
        if (!isConnected) {
            int currentPage = mViewPager.getCurrentItem();
            if (currentPage >= PAGE_ONE && currentPage <= PAGE_THREE) {
                selectPage(PAGE_FOUR);
            }
        }
        mViewPager.setScanScroll(isConnected);
    }

    /**
     * 通过配置更新壁纸
     * @param newConfig 新配置
     * @param isInit 是否初始化
     */
    private void updateWallpaperWithChangeMode(Configuration newConfig, boolean isInit){
        int flag = newConfig.uiMode & Configuration.UI_MODE_NIGHT_MASK;
        if (flag == Configuration.UI_MODE_NIGHT_YES || flag == Configuration.UI_MODE_NIGHT_NO) {
            updateWallpaper();
        }
        Log.i(TAG, "updateWallpaperWithChangeMode: " + isInit);
    }


    /**
     * 更新壁纸
     */
    public void updateWallpaper() {
        Drawable wallPaper = null;

        //优先加载用户设置的壁纸
        if (Utility.supportWallpaperCustomized()) {
            //加载用户设置的壁纸  /apd/appWallpaper/路径的壁纸
            String wallpaperPath = WallpaperUtil.getInstance(getApplicationContext())
                    .getShowWallpaperPath(getResources().getConfiguration());
            if (!TextUtils.isEmpty(wallpaperPath) && Utility.isValidPath(wallpaperPath)) {
                Bitmap bitmap = HImageUtils.getBitmap(wallpaperPath);
                if (!Objects.isNull(bitmap)) {
                    wallPaper = new BitmapDrawable(getResources(), bitmap);
                }
            }
        }
        if (Objects.isNull(wallPaper)) {
            wallPaper = ThemeUtilsEx.getAppShareBackground();
        }
        if (null != mainLayout) {
            if (null != wallPaper) {
                mainLayout.setBackground(wallPaper);
            } else if (resetDrawable != null) {
                // 没有配置默认壁纸时用最开始的背景还原
                mainLayout.setBackground(resetDrawable);
            }
        }

    }

    private void initView() {
        mainLayout = findViewById(SkinUtils.getId(R.id.layout_main));
        llBottomMenu = findViewById(SkinUtils.getId(R.id.bottom_menu));
        mBtnDial = findViewById(SkinUtils.getId(R.id.btn_telephone));
        mBtnContract = findViewById(SkinUtils.getId(R.id.btn_phonebook));
        mBtnRecord = findViewById(SkinUtils.getId(R.id.btn_record));
        mBtnSetting = findViewById(SkinUtils.getId(R.id.btn_setup));
        mBtnPaired = findViewById(SkinUtils.getId(R.id.btn_pared));

        mViewPager = findViewById(SkinUtils.getId(R.id.viewPager));
        mViewPager.setOffscreenPageLimit(5);

        mBtnDial.setOnClickListener(this);
        mBtnContract.setOnClickListener(this);
        mBtnRecord.setOnClickListener(this);
        mBtnPaired.setOnClickListener(this);
        mBtnSetting.setOnClickListener(this);

        mFragmentViewPaperAdapter = new FragmentViewPaperAdapter(getSupportFragmentManager(),
                fragments);
        mViewPager.setAdapter(mFragmentViewPaperAdapter);
        mViewPager.setPageTransformer(true, new DepthPageTransformer());
        mViewPager.addOnPageChangeListener(this);
    }

    private void setDrawablesForDayNightMode() {
        setButtonDrawable(R.id.btn_telephone, R.drawable.tab_dialpad);
        setButtonDrawable(R.id.btn_phonebook, R.drawable.tab_contacts);
        setButtonDrawable(R.id.btn_record, R.drawable.tab_history);
        setButtonDrawable(R.id.btn_setup, R.drawable.tab_settings);
        setButtonDrawable(R.id.btn_pared, R.drawable.tab_paired);

        if (mainLayout != null && (getSystemProperty("persist.sys.bluetooth.skins", "").equals("gb03")
                                || getSystemProperty("persist.sys.bluetooth.skins", "").equals("rk02"))) {
            resetDrawable = SkinUtils.getDrawable(R.drawable.bg);
            mainLayout.setBackground(resetDrawable);
        }

        if (llBottomMenu != null) {
            llBottomMenu.setBackground(SkinUtils.getDrawable(R.drawable.bg_dock));
        }
    }

    private void setButtonDrawable(int buttonId, int drawableId) {
        ImageView imageView = findViewById(SkinUtils.getId(buttonId));
        if (imageView != null) {
            imageView.setImageResource(SkinUtils.getId(drawableId));
        } else {
            Log.w("Warning", "ImageView with id " + buttonId + " is null.");
        }
    }

    private void selectPage(int position) {
        mViewPager.setCurrentItem(position, false);
    }

    @Override
    public void onPageScrolled(int position, float positionOffset,
                               int positionOffsetPixels) {

    }

    @Override
    public void onPageSelected(int position) {
        switch (position) {
            case PAGE_ONE:
                setViewSelected(mBtnDial);
                break;
            case PAGE_TWO:
                setViewSelected(mBtnContract);
                break;
            case PAGE_THREE:
                setViewSelected(mBtnRecord);
                break;
            case PAGE_FOUR:
                setViewSelected(mBtnPaired);
                break;
            case PAGE_FIVE:
                setViewSelected(mBtnSetting);
                break;
            default:
                break;
        }
    }

    @Override
    public void onPageScrollStateChanged(int state) {
    }

    @Override
    public void onClick(View view) {
        int viewId = SkinUtils.getViewId(view);
        switch (viewId) {
            case R.id.btn_telephone:
                if (isConnected) {
                    selectPage(PAGE_ONE);
                    setViewSelected(view);
                } else {
                    Utils.showToast(this, R.string.str_connect_hf);
                }
                break;
            case R.id.btn_phonebook:
                if (isConnected) {
                    selectPage(PAGE_TWO);
                    setViewSelected(view);
                } else {
                    Utils.showToast(this, R.string.str_connect_hf);
                }
                break;
            case R.id.btn_record:
                if (isConnected) {
                    selectPage(PAGE_THREE);
                    setViewSelected(view);
                } else {
                    Utils.showToast(this, R.string.str_connect_hf);
                }
                break;
            case R.id.btn_pared:
                selectPage(PAGE_FOUR);
                setViewSelected(view);
                break;
            case R.id.btn_setup:
                selectPage(PAGE_FIVE);
                setViewSelected(view);
                break;
            default:
                break;
        }
    }

    /**
     * 取消当前view(mSelectedView)的选中状态，将参数传入的view设置为选中状态
     *
     * @param view
     */
    public void setViewSelected(View view) {
        if (null != mSelectedView) {
            mSelectedView.setSelected(false);
        }
        view.setSelected(true);
        mSelectedView = view;
    }

    @SuppressLint("MissingSuperCall")
    @Override
    protected void onSaveInstanceState(Bundle outState) {
        // TODO Auto-generated method stub
        //super.onSaveInstanceState(outState);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        // TODO Auto-generated method stub
        super.onConfigurationChanged(newConfig);
        if (SkinUtils.getInteger(R.integer.support_day_night_mode) == 1) {
            int currentItem = mViewPager.getCurrentItem();
            init();
            initView();
            setDrawablesForDayNightMode();
            selectPage(currentItem);
        }
        updateWallpaperWithChangeMode(newConfig, false);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            Window window = getWindow();
            window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            //获取皮肤包里面的具体资源情况：判定该皮肤包是否需要状态栏进行背景颜色适配，0：不需要；1需要；
            boolean needStatusBarChange = needStatusBarChange();
            if (needStatusBarChange) {
                if (isNight(newConfig)) {
                    getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_VISIBLE);
                } else {
                    getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
                }
            } else {
                getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
            }
            window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            window.setStatusBarColor(Color.TRANSPARENT);
        }
    }

    public boolean isNight(Configuration newConfig) {
        return (newConfig.uiMode & Configuration.UI_MODE_NIGHT_MASK) == Configuration.UI_MODE_NIGHT_YES;
    }

    public boolean needStatusBarChange() {
        int values = NO_STATUS_BAR_CHANGE;
        values = SkinCompatResources.getInstance().getInteger(NEED_STATUS_BAR_CHANGE);
        return STATUS_BAR_CHANGE == values;
    }


    @Override
    public void onAttachedToWindow() {
        super.onAttachedToWindow();
        updateWallpaperWithChangeMode(getResources().getConfiguration(), true);

        // 记录最开始的背景---高斯模糊效果会重置背景
        if (mainLayout != null) {
            resetDrawable = mainLayout.getBackground();
        }
    }


    @Override
    protected void onUserLeaveHint() {
        super.onUserLeaveHint();
        Log.e(TAG, "onUserLeaveHint");
    }

    @Override
    protected void onStart() {
        super.onStart();
        IntentFilter filter = new IntentFilter();
        filter.addAction(LocalBluetoothAdapterManager.ACTION_CONNECTION_STATE_CHANGED);
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        filter.addAction(VOICE_EVENT_BT_EXIT);
        registerReceiver(mReceiver, filter);
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        Log.e(TAG, "onResume()");
        super.onResume();
        isConnected = mAdapterManager.isBluetoothConnected();
        mHandler.removeMessages(MSG_UPDATE_CTRL);
        mHandler.sendEmptyMessage(MSG_UPDATE_CTRL);
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        Log.e(TAG, "onPause()");
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (null != mReceiver) {
            unregisterReceiver(mReceiver);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (null != mViewPager) {
            mViewPager.removeOnPageChangeListener(this);
        }
        mAdapterManager.removeConnectListener(mAdapterListener);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_0:
            case KeyEvent.KEYCODE_1:
            case KeyEvent.KEYCODE_2:
            case KeyEvent.KEYCODE_3:
            case KeyEvent.KEYCODE_4:
            case KeyEvent.KEYCODE_5:
            case KeyEvent.KEYCODE_6:
            case KeyEvent.KEYCODE_7:
            case KeyEvent.KEYCODE_8:
            case KeyEvent.KEYCODE_9:
                if (mViewPager.getCurrentItem() == PAGE_ONE) {
                    return ((DialFragment) dialFragment).onKeyDown(keyCode, event);
                }
                break;
            default:
                break;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_0:
            case KeyEvent.KEYCODE_1:
            case KeyEvent.KEYCODE_2:
            case KeyEvent.KEYCODE_3:
            case KeyEvent.KEYCODE_4:
            case KeyEvent.KEYCODE_5:
            case KeyEvent.KEYCODE_6:
            case KeyEvent.KEYCODE_7:
            case KeyEvent.KEYCODE_8:
            case KeyEvent.KEYCODE_9:
                if (mViewPager.getCurrentItem() == PAGE_ONE) {
                    return ((DialFragment) dialFragment).onKeyUp(keyCode, event);
                }
                break;
            default:
                break;
        }
        return super.onKeyUp(keyCode, event);
    }

    private ConnectionListener mAdapterListener = new ConnectionListener() {
        @Override
        public void onServiceConnected() {
            Log.d(TAG, "onServiceConnected");
            isConnected = mAdapterManager.isBluetoothConnected();
            mHandler.removeMessages(MSG_UPDATE_CTRL);
            mHandler.sendEmptyMessage(MSG_UPDATE_CTRL);
        }

        @Override
        public void onServiceDisconnected() {

        }
    };

    @Override
    public boolean updateBackground(boolean update) {
        Log.d(TAG, "updateBackground: " + update);
        if (update) {
            if (null != mainLayout) {
                mainLayout.setBackground(new BitmapDrawable(getResources(), FastBlurUtils.getBlurBackgroundDrawer(MainBluetoothActivity.this)));
            }
            if (null != mViewPager) {
                mViewPager.setVisibility(View.INVISIBLE);
            }
            if (null != llBottomMenu) {
                llBottomMenu.setVisibility(View.INVISIBLE);
            }
        } else {
            if (null != mainLayout) {
                updateWallpaper();
            }
            if (null != mViewPager) {
                mViewPager.setVisibility(View.VISIBLE);
            }
            if (null != llBottomMenu) {
                llBottomMenu.setVisibility(View.VISIBLE);
            }
        }
        return false;
    }

    @Override
    public boolean clearRecordList() {
        if (recordFragment != null){
            ((RecordFragment)recordFragment).clearCallLog();
        }
        return false;
    }
}
