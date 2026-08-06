package com.autochips.bluetooth.fragment;

import static com.hcn.bluetooth.api.LocalBluetoothAdapterManager.THIRD_PART_ZJ_CARPLAY_MODE;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.autochips.bluetooth.IFragmentCallback;
import com.autochips.bluetooth.MainBluetoothActivity;
import com.autochips.bluetooth.MyApplication;
import com.autochips.bluetooth.R;
import com.autochips.bluetooth.skin.SkinUtils;
import com.autochips.bluetooth.utils.Utility;
import com.autochips.bluetooth.utils.WallpaperUtil;
import com.hcn.auto.app.Wallpaper;
import com.hcn.auto.app.base.Listenable;
import com.hcn.auto.utils.HImageUtils;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;
import com.hcn.bluetooth.api.LocalBluetoothAdapterManager;
import com.hcn.bluetooth.api.Utils;
import com.hcn.skin.support.app.SkinCompatFragment;

import java.util.List;
import java.util.Objects;

/**
 * 描述：蓝牙设置界面 ZA01使用
 *
 * @author simon
 * @date 2023/3/28 14:43
 */
public class SetupFragmentEx extends SkinCompatFragment implements View.OnClickListener {
    public static final String TAG = "SetupFragmentEx";
    private LocalBluetoothAdapterManager mLocalAdapterManager = null;
    private View mViewRoot;
    private Context mContext;
    /**
     * UI 控件变量
     */
    private Button mAutoConnectButton = null;
    private Button mAutoAnswerButton = null;
    private Button mPowerSwitchButton = null;
    private Button mTipsButton = null;
    private Button mNetWorkButton = null;
    private Button mTipsCloseButton = null;
    private ViewGroup mLayoutNetwork;
    private ViewGroup mLayoutSetting;
    private LinearLayout layoutPinCode = null;

    private PopupWindow mEditBTNameDialog = null;
    private EditText mEditDeviceName = null;

    private PopupWindow mEditBTPinDialog = null;
    private EditText mEditPin = null;
    /**
     * 默认蓝牙名称和默认Pin码
     */
    public String mPinCode = "0000";
    public String mDevName = "CarBT";
    private static final int MIN_PIN_CODE_LENGTH = 4;
    private static final int MAX_PIN_CODE_LENGTH = 16;
    /**
     * 自动接听状态
     */
    private boolean mIsAutoAnswer = false;

    /**
     * 自动连接状态
     */
    private boolean mIsAutoConnect = false;

    /**
     * 通知Activity更新背景
     */
    private IFragmentCallback fragmentCallback = null;


    /*** 壁纸设置相关 */
    private LinearLayout mLayoutWallpaper = null;
    private Button mWallpaperButton = null;
    private PopupWindow mWallpaperDialog = null;
    private RecyclerView mWallpaperRecyclerView = null;
    private WallpaperAdapter mWallpaperAdapter = null;
    /*** 壁纸设置相关 */

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mLocalAdapterManager = MyApplication.getInstance().getAdapterManager();
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        mViewRoot = super.onCreateView(inflater, container, savedInstanceState);
        initView();
        registerReceiver();
        initWallpaper();
        return mViewRoot;
    }

    private void registerReceiver() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        Objects.requireNonNull(getContext()).registerReceiver(mReceiver, filter);
    }

    /**
     * 初始化控件
     */
    private void initView() {
        if (mLocalAdapterManager != null) {
            mDevName = mLocalAdapterManager.getBTName();
        }

        if(Utils.getSystemProperty(SkinUtils.BT_NAME_PROP, null) != null){
            mDevName = Utils.getSystemProperty(SkinUtils.BT_NAME_PROP, null);
        }

        //需要加接口，确认是否支持
        if (mLocalAdapterManager != null) {
            mPinCode = mLocalAdapterManager.getBTPincode();
        }

        TextView editNameView = mViewRoot.findViewById(SkinUtils.getId(R.id.tv_edit_name));
        if (editNameView != null) {
            editNameView.setText(mDevName);
        }
        TextView editPinView = mViewRoot.findViewById(SkinUtils.getId(R.id.tv_edit_pin));
        if (editPinView != null) {
            editPinView.setText(mPinCode);
        }
        Button editNameButton = mViewRoot.findViewById(SkinUtils.getId(R.id.btn_edit_name_bt));
        if (editNameButton != null) {
            editNameButton.setOnClickListener(this);
        }
        Button editPinButton = mViewRoot.findViewById(SkinUtils.getId(R.id.btn_edit_pin_bt));
        if (editPinButton != null) {
            editPinButton.setOnClickListener(this);
        }

        mAutoConnectButton = mViewRoot.findViewById(SkinUtils.getId(R.id.switch_autoconnect));
        if (mAutoConnectButton != null) {
            mAutoConnectButton.setOnClickListener(this);
        }

        mAutoAnswerButton = mViewRoot.findViewById(SkinUtils.getId(R.id.switch_autoanswer));
        if (mAutoAnswerButton != null) {
            mAutoAnswerButton.setOnClickListener(this);
        }

        mNetWorkButton = mViewRoot.findViewById(SkinUtils.getId(R.id.network));
        if (mNetWorkButton != null) {
            mNetWorkButton.setOnClickListener(this);
        }

        mTipsButton = mViewRoot.findViewById(SkinUtils.getId(R.id.network_tips));
        if (mTipsButton != null) {
            mTipsButton.setOnClickListener(this);
        }

        mTipsCloseButton = mViewRoot.findViewById(SkinUtils.getId(R.id.tips_close));
        if (mTipsCloseButton != null) {
            mTipsCloseButton.setOnClickListener(this);
        }

        //壁纸相关
        mLayoutWallpaper = mViewRoot.findViewById(SkinUtils.getId(R.id.layout_wallpaper));
        if (mLayoutWallpaper != null){
            if (Utility.supportWallpaperCustomized()){
                mWallpaperButton = mViewRoot.findViewById(SkinUtils.getId(R.id.btn_set_wallpaper));
                if (mWallpaperButton != null){
                    mWallpaperButton.setOnClickListener(this);
                }
                buildWallpaperDialog();
                // 获取壁纸数据
                List<Wallpaper.Info> list = Wallpaper.instance().getInfo();
                if (mLayoutWallpaper != null){
                    if (Objects.isNull(list) || list.isEmpty()){
                        //没有壁纸图片时，隐藏该项配置
                        mLayoutWallpaper.setVisibility(View.GONE);
                    }else{
                        mLayoutWallpaper.setVisibility(View.VISIBLE);
                    }
                }
            }else{
                mLayoutWallpaper.setVisibility(View.GONE);
            }
        }


        mLayoutNetwork = mViewRoot.findViewById(SkinUtils.getId(R.id.network_layout));
        mLayoutSetting = mViewRoot.findViewById(SkinUtils.getId(R.id.setting_layout));

        mPowerSwitchButton = mViewRoot.findViewById(SkinUtils.getId(R.id.switch_power));
        mPowerSwitchButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View arg0) {
                if (mLocalAdapterManager != null) {
                    if (THIRD_PART_ZJ_CARPLAY_MODE == mLocalAdapterManager.getThirdPartAPPMode()) {
                        Utils.showToast(mContext, SkinUtils.getString(R.string.str_zlink_mode));
                        return;
                    }
                    //先断开连接
                    BluetoothDeviceInfo device = mLocalAdapterManager.getConnectDevice();
                    if (null != device) {
                        mLocalAdapterManager.disconnectDevice(device.getDeviceAddr());
                    }
                    mLocalAdapterManager.resetBT();
                }
            }
        });

        //persist.sys.bt.disablessp 该属性为true 才支持手机端输入pin码配对模式
        String disable_ssp = Utils.getSystemProperty("persist.sys.bt.disablessp", "false");
        layoutPinCode = mViewRoot.findViewById(SkinUtils.getId(R.id.layout_pin));
        if (layoutPinCode != null && "true".equals(disable_ssp)) {
            layoutPinCode.setVisibility(View.VISIBLE);
        }

        // 临时方案-后续调整框架移除
        if (MainBluetoothActivity.showLayoutNetwork) {
            onActionTipsStarted();
        } else {
            onActionTipsFinished();
        }
    }

    /**
     * 初始化一次壁纸数据(保证唯一注册)
     * 在 initView 中 B+/- 会导致重复注册
     * 从而导致 mWallpaperListener 使用 mWallpaperAdapter 空指针异常
     */
    private void initWallpaper() {
        if (Utility.supportWallpaperCustomized()){
            Wallpaper.instance().initialize();
            Wallpaper.instance().register(mWallpaperListener);
        }
    }


    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        if (SkinUtils.useSkinPackage()) {
            mContext = SkinUtils.getContext();
        } else {
            mContext = context;
        }
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onResume() {
        super.onResume();
        checkBluetoothState();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onStop() {
        super.onStop();
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        Objects.requireNonNull(getContext()).unregisterReceiver(mReceiver);
        Wallpaper.instance().unregister(mWallpaperListener);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onDetach() {
        super.onDetach();
    }

    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        if (!mLocalAdapterManager.isBluetoothEnable()) {
            Utils.showToast(mContext, SkinUtils.getId(R.string.bluetooth_please_power_on_toast));
            return;
        }

        // 每次操作后，都重新计时
        //BtUtils.checkAutoConnectSetting(BtUtils.AUTO_CONNECT_TRY_DELAY);
        int viewId = SkinUtils.getViewId(v);
        switch (viewId) {
            case R.id.btn_edit_name_bt:
                showEditNameDialog();
                break;
            case R.id.btn_edit_pin_bt:
                showEditPinDialog();
                break;
            case R.id.switch_autoconnect:
                mIsAutoConnect = !mLocalAdapterManager.isBluetoothAutoConnect();
                if (mIsAutoConnect) {
                    mAutoConnectButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_on));
                } else {
                    mAutoConnectButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_off));
                }
                mLocalAdapterManager.setBluetoothAutoConnect(mIsAutoConnect);
                break;

            case R.id.switch_autoanswer:
                mIsAutoAnswer = !mLocalAdapterManager.isBluetoothAutoAnswer();
                if (mIsAutoAnswer) {
                    mAutoAnswerButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_on));
                } else {
                    mAutoAnswerButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_off));
                }
                mLocalAdapterManager.setBluetoothAutoAnswer(mIsAutoAnswer);
                break;

            case R.id.network_tips:
                onActionTipsStarted();
                // 临时方案
                MainBluetoothActivity.showLayoutNetwork = true;
                break;
            case R.id.network:
                Log.d(TAG, "network click");
                boolean isAutoConnectNetwork = !mLocalAdapterManager.isAutoConnectedNetwork();
                mLocalAdapterManager.setAutoConnectedNetwork(isAutoConnectNetwork);

                if (isAutoConnectNetwork) {
                    mNetWorkButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_on));
                } else {
                    mNetWorkButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_off));
                }
                break;
            case R.id.tips_close:
                onActionTipsFinished();
                // 临时方案
                MainBluetoothActivity.showLayoutNetwork = false;
                break;
            case R.id.bt_name_confirm:
                confirmModifyBTName();
                break;
            case R.id.bt_name_cancel:
                hideEditNameDialog();
                break;
            case R.id.pin_code_confirm:
                confirmModifyPinCode();
                break;
            case R.id.pin_code_cancel:
                hideEditPinDialog();
                break;
            case R.id.btn_set_wallpaper:
                showWallpaperDialog();
                break;
            default:
                break;
        }
    }

    @Override
    public void onBindViewData() {

    }

    @Override
    public int getLayoutRes() {
        return R.layout.layout_bt_setting;
    }

    public void setFragmentCallback(IFragmentCallback callback) {
        fragmentCallback = callback;
    }

    /**
     * 更新蓝牙状态
     */
    private void checkBluetoothState() {
        if (mLocalAdapterManager == null) {
            Log.e(TAG, "checkBluetoothState: mLocalAdapterManager is null");
            return;
        }

        if (mLocalAdapterManager.isBluetoothEnable()) {
            if (mPowerSwitchButton != null) {
                mPowerSwitchButton.setEnabled(true);
                mPowerSwitchButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_on));
            }
        } else {
            if (mPowerSwitchButton != null) {
                mPowerSwitchButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_off));
            }
        }

        if (mLocalAdapterManager.isBluetoothAutoAnswer()) {
            if (mAutoAnswerButton != null) {
                mAutoAnswerButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_on));
            }
        } else {
            if (mAutoAnswerButton != null) {
                mAutoAnswerButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_off));
            }
        }

        if (mLocalAdapterManager.isBluetoothAutoConnect()) {
            if (mAutoConnectButton != null) {
                mAutoConnectButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_on));
            }
        } else {
            if (mAutoConnectButton != null) {
                mAutoConnectButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_off));
            }
        }

        if (mLocalAdapterManager.isAutoConnectedNetwork()) {
            if (mNetWorkButton != null) {
                mNetWorkButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_on));
            }
        } else {
            if (mNetWorkButton != null) {
                mNetWorkButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_off));
            }
        }
    }

    private void onActionTipsStarted() {
        Log.d(TAG, "onActionTipsStarted");

        if (mLayoutNetwork != null) {
            mLayoutNetwork.setVisibility(View.VISIBLE);
        }
        if (mLayoutSetting != null) {
            mLayoutSetting.setVisibility(View.INVISIBLE);
        }
    }

    private void onActionTipsFinished() {
        Log.d(TAG, "onActionTipsFinished");

        if (mLayoutNetwork != null) {
            mLayoutNetwork.setVisibility(View.GONE);
        }
        if (mLayoutSetting != null) {
            mLayoutSetting.setVisibility(View.VISIBLE);
        }
    }

    /**
     * 显示修改蓝牙名称弹框
     */
    private void showEditNameDialog() {
        if (null == mContext) {
            Log.e(TAG, "showEditNameDialog: mContext is null");
            return;
        }
        if (null == getActivity() || getActivity().isFinishing()) {
            Log.e(TAG, "Activity is finish");
            return;
        }

        View view = SkinUtils.inflate(R.layout.bt_name_change_dialog);
        if (view != null) {
            mEditBTNameDialog = new PopupWindow(view, ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);
            mEditBTNameDialog.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
            mEditBTNameDialog.setOutsideTouchable(true);
            mEditBTNameDialog.setFocusable(true);
            mEditBTNameDialog.setAnimationStyle(R.style.PopupAnimation);
            mEditBTNameDialog.setOnDismissListener(new PopupWindow.OnDismissListener() {
                @Override
                public void onDismiss() {
                    if (fragmentCallback != null) {
                        fragmentCallback.updateBackground(false);
                    }
                }
            });

            mEditDeviceName = view.findViewById(SkinUtils.getId(R.id.edit_bt_name));
            Button btnConfirm = view.findViewById(SkinUtils.getId(R.id.bt_name_confirm));
            if (btnConfirm != null) {
                btnConfirm.setOnClickListener(SetupFragmentEx.this);
            }
            Button btnCancel = view.findViewById(SkinUtils.getId(R.id.bt_name_cancel));
            if (btnCancel != null) {
                btnCancel.setOnClickListener(SetupFragmentEx.this);
            }

            if (mLocalAdapterManager != null && mEditDeviceName != null) {
                if(Utils.getSystemProperty(SkinUtils.BT_NAME_PROP, null) != null){
                    mDevName = Utils.getSystemProperty(SkinUtils.BT_NAME_PROP, null);
                }else{
                    mDevName = mLocalAdapterManager.getBTName();
                }
                mEditDeviceName.setText(mDevName);
            }

            if (fragmentCallback != null) {
                fragmentCallback.updateBackground(true);
            }
            mEditBTNameDialog.showAtLocation(mViewRoot, Gravity.CENTER, 0, 0);
        }
    }

    /**
     * 隐藏蓝牙名称弹窗
     */
    private void hideEditNameDialog() {
        if (mEditBTNameDialog != null) {
            mEditBTNameDialog.dismiss();
        }
        if (fragmentCallback != null) {
            fragmentCallback.updateBackground(false);
        }
    }

    /**
     * 确认修改蓝牙名称
     */
    private void confirmModifyBTName() {
        if (mEditDeviceName != null) {
            String newDeviceName = mEditDeviceName.getText().toString().trim();
            if (newDeviceName.length() > 20) {
                newDeviceName = newDeviceName.substring(0, 19);
            }

            if (!newDeviceName.isEmpty()) {
                //需要增加接口
                if (mLocalAdapterManager != null) {
                    mLocalAdapterManager.setBTName(newDeviceName);
                }
                TextView editNameView = mViewRoot.findViewById(SkinUtils.getId(R.id.tv_edit_name));
                if (editNameView != null) {
                    editNameView.setText(newDeviceName);
                }
                hideEditNameDialog();
            } else {
                Utils.showToast(mContext, SkinUtils.getId(R.string.bluetooth_device_info_toast));
            }
        }
    }

    private void showEditPinDialog() {
        if (null == mContext) {
            Log.e(TAG, "buildEditPinDialog: mContext is null");
            return;
        }
        if (null == getActivity() || getActivity().isFinishing()) {
            Log.e(TAG, "Activity is finishing");
            return;
        }
        View view = SkinUtils.inflate(R.layout.pin_code_change_dialog);
        if (view != null) {
            mEditBTPinDialog = new PopupWindow(view, ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);
            mEditBTPinDialog.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
            mEditBTPinDialog.setOutsideTouchable(true);
            mEditBTPinDialog.setFocusable(true);
            mEditBTPinDialog.setAnimationStyle(R.style.PopupAnimation);
            mEditBTPinDialog.setOnDismissListener(new PopupWindow.OnDismissListener() {
                @Override
                public void onDismiss() {
                    if (fragmentCallback != null) {
                        fragmentCallback.updateBackground(false);
                    }
                }
            });

            mEditPin = view.findViewById(SkinUtils.getId(R.id.edit_pin_code));
            Button btnConfirm = view.findViewById(SkinUtils.getId(R.id.pin_code_confirm));
            if (btnConfirm != null) {
                btnConfirm.setOnClickListener(SetupFragmentEx.this);
            }
            Button btnCancel = view.findViewById(SkinUtils.getId(R.id.pin_code_cancel));
            if (btnCancel != null) {
                btnCancel.setOnClickListener(SetupFragmentEx.this);
            }

            if (mLocalAdapterManager != null && mEditPin != null) {
                mPinCode = mLocalAdapterManager.getBTPincode();
                mEditPin.setText(mPinCode);
            }

            if (fragmentCallback != null) {
                fragmentCallback.updateBackground(true);
            }

            mEditBTPinDialog.showAtLocation(mViewRoot, Gravity.CENTER, 0, 0);
        }
    }

    /**
     * 隐藏蓝牙PinCode修改弹窗
     */
    private void hideEditPinDialog() {
        if (mEditBTPinDialog != null) {
            mEditBTPinDialog.dismiss();
        }
        if (fragmentCallback != null) {
            fragmentCallback.updateBackground(false);
        }
    }

    /**
     * 确认修改PinCode
     */
    private void confirmModifyPinCode() {
        if (mEditPin != null) {
            String newPinCode = mEditPin.getText().toString().trim();

            if (newPinCode.length() >= MIN_PIN_CODE_LENGTH
                    && newPinCode.length() <= MAX_PIN_CODE_LENGTH) {
                TextView editPinView = mViewRoot.findViewById(SkinUtils.getId(R.id.tv_edit_pin));
                if (editPinView != null) {
                    editPinView.setText(newPinCode);
                }
                mPinCode = newPinCode;
                hideEditPinDialog();
                if (mLocalAdapterManager != null) {
                    mLocalAdapterManager.setBTPincode(mPinCode);
                }
            } else {
                Utils.showToast(mContext, SkinUtils.getId(R.string.bluetooth_device_info_toast));
            }
        }
    }

    /**
     * @description: 构建壁纸设置弹窗
     * @author: guohonglan
     * @since: 2024/1/17 12:21
     * @param:
     * @return:
     */
    private void buildWallpaperDialog() {
        if (null == mContext) {
            Log.e(TAG, "showSetWallpaperDialog: mContext is null");
            return;
        }
        if (null == getActivity() || getActivity().isFinishing()) {
            Log.e(TAG, "Activity is finish");
            return;
        }

        View view = SkinUtils.inflate(R.layout.wallpaper_dialog);
        if (view != null) {
            mWallpaperDialog = new PopupWindow(view, ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT);
            mWallpaperDialog.setOutsideTouchable(true);
            mWallpaperDialog.setFocusable(true);
            mWallpaperDialog.setAnimationStyle(R.style.PopupAnimation);
            mWallpaperDialog.setOnDismissListener(new PopupWindow.OnDismissListener() {
                @Override
                public void onDismiss() {
                    if (fragmentCallback != null) {
                        fragmentCallback.updateBackground(false);
                    }
                }
            });

            mWallpaperRecyclerView = view.findViewById(SkinUtils.getId(R.id.wallpaper_recyclerview));

            GridLayoutManager glManager = new GridLayoutManager(mContext, SkinUtils.getInteger(R.integer.wallpaper_grid_layout_numRows),
                    SkinUtils.getInteger(R.integer.wallpaper_recyclerview_orientation), false);
            mWallpaperRecyclerView.setLayoutManager(glManager);

            mWallpaperAdapter = new WallpaperAdapter();
            mWallpaperAdapter.setOnItemClickListener(mWallpaperItemClickListener);
            mWallpaperRecyclerView.setAdapter(mWallpaperAdapter);
        }
    }

    /**
     * @description: 显示壁纸设置弹窗
     * @author: guohonglan
     * @since: 2024/1/22 12:13
     * @param:
     * @return:
     */
    private void showWallpaperDialog(){
        if (mWallpaperDialog != null) {
            Wallpaper.instance().initialize();
            mWallpaperDialog.showAtLocation(mViewRoot, Gravity.CENTER, 0, 0);
        }
    }

    /**
     * @description: 隐藏壁纸设置弹窗
     * @author: guohonglan
     * @since: 2024/1/17 12:22
     * @param:
     * @return:
     */
    private void hideWallpaperDialog() {
        if (mWallpaperDialog != null) {
            mWallpaperDialog.dismiss();
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        hideWallpaperDialog();
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE,
                        BluetoothAdapter.STATE_OFF);
                if (state == BluetoothAdapter.STATE_OFF) {
                    Log.d(TAG, "onReceive: ACTION_STATE_CHANGED STATE_OFF");
                    if (mPowerSwitchButton != null) {
                        mPowerSwitchButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_off));
                    }
                } else if (state == BluetoothAdapter.STATE_ON) {
                    Log.d(TAG, "onReceive: ACTION_STATE_CHANGED STATE_ON");
                    if (mPowerSwitchButton != null) {
                        mPowerSwitchButton.setBackgroundResource(SkinUtils.getId(R.drawable.set_tumbler_on));
                    }
                }
            }
        }
    };
    /**
     * @description: 壁纸监听器，监听扫描文件完成事件
     * @author: guohonglan
     * @since: 2024/1/17 15:32
     * @param:
     * @return:
     */
    private final Listenable<String> mWallpaperListener = (s, o) -> {
        // 壁纸状态检查
        if (Objects.equals(s, Wallpaper.ST_COMPLETED)) {
            Log.d(TAG, "Wallpaper.ST_COMPLETED");
            // 壁纸扫描完成
            // 扫描 /apd/appWallpaper/ 目录下是否有壁纸文件
            List<Wallpaper.Info> list = WallpaperUtil.getInstance(mContext).getFilterWallpapers();
            if (list == null || list.isEmpty()) {
                Log.d(TAG, "Wallpaper is empty");
                // 没有壁纸文件
                if (mLayoutWallpaper != null){
                    mLayoutWallpaper.setVisibility(View.GONE);
                }
                hideWallpaperDialog();
                return;
            } else {
                // 有壁纸文件
                // 可以在这里设置壁纸数据到显示 UI 列表中
                Log.d(TAG, "Wallpaper count = " + list.size());
                if (mLayoutWallpaper != null){
                    mLayoutWallpaper.setVisibility(View.VISIBLE);
                }
                mWallpaperAdapter.setData(list);
                //数据刷新
                mWallpaperAdapter.notifyDataSetChanged();
            }
        }
    };

    /**
     * @description: 壁纸设置Adapter
     * @author: guohonglan
     * @since: 2024/1/17 16:18
     * @param:
     * @return:
     */
    private class WallpaperAdapter extends RecyclerView.Adapter<WallpaperAdapter.WallpaperViewHolder>{
        private List<Wallpaper.Info> mData;
        private WallpaperItemClickListener mItemClickListener;

        @Override
        public WallpaperViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View view = SkinUtils.inflate(R.layout.wallpaper_listitem);
            return new WallpaperViewHolder(view);
        }

        @Override
        public void onBindViewHolder(WallpaperViewHolder holder, int position) {
            Log.d(TAG, "wallpaper onBindViewHolder");
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
                        String lastShow = WallpaperUtil.getInstance(mContext)
                                .getShowWallpaperPath(mContext.getResources().getConfiguration());
                        if (mData.get(position).wallpaperPath.equals(lastShow)) {
                            holder.ivWallpaperBg.setVisibility(View.VISIBLE);
                        } else {
                            holder.ivWallpaperBg.setVisibility(View.GONE);
                        }
                    }
                }
            }
        }

        public void setOnItemClickListener(WallpaperItemClickListener listener){
            this.mItemClickListener = listener;
        }
        public void setData(List<Wallpaper.Info> data){
            this.mData = data;
        }

        @Override
        public int getItemCount() {
            return mData == null ? 0 : mData.size();
        }

        private class WallpaperViewHolder extends RecyclerView.ViewHolder{
            private ImageView ivWallPaper;
            private ImageView ivWallpaperBg;
            public WallpaperViewHolder(View itemView) {
                super(itemView);
                this.ivWallPaper = itemView.findViewById(SkinUtils.getId(R.id.iv_wallpaper));
                this.ivWallpaperBg = itemView.findViewById(SkinUtils.getId(R.id.iv_wallpaper_bg));
            }
        }
    }

    /*** @Des: 壁纸recyclerview Item的点击事件接口*/
    public interface WallpaperItemClickListener{
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
                //将壁纸路径保存到Settings配置中
                WallpaperUtil.getInstance(mContext).saveWallpaperData(path);
                //刷新RecyclerView列表
                mWallpaperAdapter.notifyDataSetChanged();
                //与Activity通信，刷新背景图
                if(fragmentCallback != null){
                    fragmentCallback.updateBackground(false);
                }
                //发送广播,通知BTMusic刷新背景图
                Intent intent = new Intent("android.intent.action.ACTION_WALLPAPER_CHANGED");
                LocalBroadcastManager.getInstance(mContext).sendBroadcast(intent);
            }
        }
    };
}
