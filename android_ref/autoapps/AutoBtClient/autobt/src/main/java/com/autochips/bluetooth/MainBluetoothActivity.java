package com.autochips.bluetooth;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;

import com.autochips.bluetooth.fragment.BaseFragment;
import com.autochips.bluetooth.fragment.ContactsFragment;
import com.autochips.bluetooth.fragment.DialFragment;
import com.autochips.bluetooth.fragment.RecordFragment;
import com.autochips.bluetooth.fragment.SettingsFragment;
import com.autochips.bluetooth.manager.HStateBroadReceiver;
import com.autochips.bluetooth.util.Constants;
import com.autochips.bluetooth.util.HUtils;

import java.util.List;


public class MainBluetoothActivity extends AppCompatActivity implements RadioGroup.OnCheckedChangeListener
        , HStateBroadReceiver.BtStateCallback {
    private final String TAG = "MainBluetoothActivity";
    //final
    //通话时传递的参数。
    private final String EXTRA_PAGE_INDEX = "extra_page_index";
    //来源
    private final String EXTRA_SOURCE = "reason";
    //方控按键
    private final String EXTRA_SOURCE_SWC = "swc_key_bt";
    //
    private final int MSG_HIDE_ROOT = 1;
    private final int MSG_SHOW_TIP = 2;

    private View mRootView;
    private RadioGroup mRg;
    private Fragment mCurrentFragment = null;
    private int mLastId = Constants.ID_FRAGMENT_UNDEFINE;
    /**
     * 第一次按下拨号按键起来。
     */
    private boolean bCallEvent = false;
    //
    private Handler handler = new Handler() {
        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_HIDE_ROOT:
                    Log.d(TAG, "handler 1");
                    //适配电话弹框切换到界面，先隐藏下，然后
                    updateMenu(BaseApplication.getInstance().getBluetoothAdapter().isBluetoothConnected());
                    if (mRootView != null && mRootView.getVisibility() == View.VISIBLE) {
                        Log.d(TAG, "handler 1  INVISIBLE");
                        mRootView.setVisibility(View.INVISIBLE);
                    }
                    break;
                case MSG_SHOW_TIP:
                    if(!BaseApplication.getInstance().getBluetoothAdapter().isBluetoothConnected()) {
                        HUtils.showTips(MainBluetoothActivity.this);
                    }
                    break;
                default:
                    break;
            }
        }
    };
    /**
     * 状态监听
     */
    @Override
    public void callbackConnect() {
        updateMenu(true);
    }

    @Override
    public void callbackDisconnect() {
        updateMenu(false);
    }

    @Override
    public void callbackChange() {

    }

    @Override
    public void callbackPower() {
        boolean connect = BaseApplication.getInstance().getBluetoothAdapter().isBluetoothConnected();
        /**
         * bug：如ACC起来按拨号按键，先进界面没连接，跳不到拨号界面，再连接上后会跳到联系人，所以表现首次按方控拨号无效。
         */
        if(connect){
            if(bCallEvent){
                //dial页不会一直读数据，先进record页会一直读，等读完根据对应状态dial也取数据就行
                updateMenu(true,Constants.ID_FRAGMENT_RECORD);
                updateMenu(true,Constants.ID_FRAGMENT_DIAL);
                bCallEvent = false;
            }else{
                updateMenu(true);
            }
        }else {
            updateMenu(false);
        }
    }

    @Override
    public String callbackTag() {
        return MainBluetoothActivity.class.getSimpleName();
    }

    /**
     *
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        //requestWindowFeature(Window.FEATURE_NO_TITLE);
        super.onCreate(savedInstanceState);
        //getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN
        //        ,WindowManager.LayoutParams.FLAG_FULLSCREEN);
        //getSupportActionBar().hide();
        setContentView(R.layout.activity_main);

        requestPermission();
        registerReceivers();

        initView();
    }

    @Override
    protected void onStart() {
        super.onStart();
        //
        if (mRootView != null && mRootView.getVisibility() == View.VISIBLE) {
            Log.d(TAG, "onStart set   INVISIBLE");
            mRootView.setVisibility(View.INVISIBLE);
        }
        Log.d(TAG, "onStart");
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        Log.d(TAG, "onNewIntent");

        setIntent(intent);
    }

    @Override
    protected void onResume() {
        super.onResume();
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
        if (handler.hasMessages(1)) {
            Log.d(TAG, "remove handler 1");
            handler.removeMessages(1);
        }
        boolean isConnect = BaseApplication.getInstance().getBluetoothAdapter().isBluetoothConnected();
        boolean inCall = BaseApplication.getInstance().getBluetoothAdapter().isInCall();
        Log.d(TAG, "onResume : " +  isConnect + " , " + inCall);

        int page_index = -1;
        String source = "";
        if (getIntent() != null) {
            Bundle bundle = getIntent().getExtras();
            if (bundle != null) {
                page_index = bundle.getInt(EXTRA_PAGE_INDEX, -1);
                source = bundle.getString(EXTRA_SOURCE,"");
                bCallEvent = false;
                if(page_index == -1 && !TextUtils.isEmpty(source) && source.equals(EXTRA_SOURCE_SWC)){
                    page_index = Constants.ID_FRAGMENT_DIAL;
                    bCallEvent = true;
                }
            }
        }
        Log.d(TAG, "onResume : page_index:" + page_index + " , source : " + source);

        if (isConnect) {
            if (page_index > Constants.ID_FRAGMENT_SETTING || page_index < Constants.ID_FRAGMENT_CONTACT) {
                //默认进联系人
                page_index = Constants.ID_FRAGMENT_CONTACT;
            }
            //过掉添加通话时的广播，这个是发给service来刷新通话框的，在拨号界面不能显示大框。
            if (page_index != Constants.ID_FRAGMENT_DIAL) {
                sendBroadcast(new Intent(Constants.ACTION_BT_UI_CHANGE));
            }
            showFragment(page_index);
            updateMenu(true, page_index);
        } else {
            //在ui没起来时，点顶部栏进这里有问题
            sendBroadcast(new Intent(Constants.ACTION_BT_UI_CHANGE));
            showFragment(Constants.ID_FRAGMENT_SETTING);
            updateMenu(isConnect);
            //没连接提示
            if(page_index == Constants.ID_FRAGMENT_DIAL){
                if(!handler.hasMessages(MSG_SHOW_TIP)){
                    handler.sendEmptyMessageDelayed(MSG_SHOW_TIP,1000);
                }
            }
        }

        new Handler().postDelayed(() -> {
            if (mRootView.getVisibility() != View.VISIBLE) {
                mRootView.setVisibility(View.VISIBLE);
            }
        }, inCall ? 500 : 0);

        //通知
        if(mCurrentFragment != null && mCurrentFragment instanceof BaseFragment){
            Log.d(TAG, "onResume : updateNotify");
            ((BaseFragment)mCurrentFragment).updateNotify();
        }
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        Log.d(TAG, "onRestart");
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.d(TAG, "onConfigurationChanged");
    }

    @Override
    protected void onPause() {
        handler.sendEmptyMessageDelayed(MSG_HIDE_ROOT, 50);
        super.onPause();
        Log.d(TAG, "onPause");
        bCallEvent = false;
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.d(TAG, "onStop");
        sendBroadcast(new Intent(Constants.ACTION_BT_UI_CHANGE));
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    //---------------------------------------------private
    private void requestPermission() {
        //6.0之后非系统app需要
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            return;
        }
        String[] permissions = {Manifest.permission.WRITE_CONTACTS
                , Manifest.permission.READ_CONTACTS, Manifest.permission.READ_CALL_LOG
                , Manifest.permission.WRITE_CALL_LOG, Manifest.permission.BLUETOOTH_CONNECT
                , Manifest.permission.BLUETOOTH};
        for (String permission : permissions) {
            if (checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(permissions, 666);
                break;
            }
        }
    }

    /**
     * 注册监听
     */
    private void registerReceivers() {
        BaseApplication.getInstance().getBluetoothAdapter().setLocalCallback(this);
    }

    /**
     * 初始化view
     */
    private void initView() {
        mRootView = findViewById(R.id.id_root_view);
        mRg = findViewById(R.id.id_main_menu_rg);
        mRg.setOnCheckedChangeListener(this);
    }

    /**
     * 更新菜单
     *
     * @param connect
     */
    private void updateMenu(boolean connect) {
        updateMenu(connect, -1);
    }

    private void updateMenu(boolean connect, int index) {
        Log.d(TAG, "updateMenu# connect : " + connect + " , " + index);
        if (mRg.getChildCount() == 0) {
            return;
        }
        for (int i = 0; i < mRg.getChildCount() - 1; i++) {
            mRg.getChildAt(i).setEnabled(connect);
        }
        if (connect && index > -1) {
            Log.d(TAG, "updateMenu# connect : " + mRg.getChildAt(index).getId());
            ((RadioButton) mRg.getChildAt(index)).setChecked(true);
        } else {
            int position = connect ? Constants.ID_FRAGMENT_CONTACT : Constants.ID_FRAGMENT_SETTING;
            if (position > mRg.getChildCount()) {
                position = 0;
            }
            ((RadioButton) mRg.getChildAt(position)).setChecked(true);
        }
    }

    /**
     * fragment 切换
     *
     * @param id
     */
    private void showFragment(int id) {
        Log.d(TAG, "showFragment: id = " + id);
        Fragment fragment = null;
        Class cls = null;
        if (mLastId == id) {
            Log.d(TAG, "showFragment: has show " + id);
            return;
        }
        mLastId = id;
        switch (id) {
            case Constants.ID_FRAGMENT_SETTING:
                cls = SettingsFragment.class;
                break;
            case Constants.ID_FRAGMENT_DIAL:
                cls = DialFragment.class;
                break;
            case Constants.ID_FRAGMENT_RECORD:
                cls = RecordFragment.class;
                break;
            case Constants.ID_FRAGMENT_CONTACT:
                cls = ContactsFragment.class;
                break;
            default:
                break;
        }

        if (cls == null) {
            return;
        }
        FragmentManager fm = getSupportFragmentManager();
        FragmentTransaction ft = fm.beginTransaction();
        //特别切换语言后需要，显示前先隐藏之前的
        List<Fragment> list = fm.getFragments();
        for (Fragment f : list) {
            if (!f.isHidden()) {
                ft.hide(f);
            }
        }
        //重复隐藏，依据tag查找
        String tag = cls.getSimpleName();
        Log.d(TAG, "showFragment: tag = " + tag);
        fragment = fm.findFragmentByTag(tag);
        if (mCurrentFragment != null) {
            ft.hide(mCurrentFragment);
        }
        if (fragment != null) {
            Log.d(TAG, "showFragment: isAdded = " + fragment.isAdded());
        }
        //显示当前的
        if (fragment != null && fragment.isAdded()) {
            ft.show(fragment);
        } else {
            try {
                fragment = (Fragment) cls.newInstance();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (InstantiationException e) {
                e.printStackTrace();
            }
            ft.add(R.id.id_container, fragment, tag);
        }
        mCurrentFragment = fragment;
        ft.commitAllowingStateLoss();
    }

    //--------------------------------------------listener
    @Override
    public void onCheckedChanged(RadioGroup radioGroup, int i) {
        switch (i) {
            case R.id.id_main_menu_rb1:
                showFragment(Constants.ID_FRAGMENT_CONTACT);
                break;
            case R.id.id_main_menu_rb2:
                showFragment(Constants.ID_FRAGMENT_RECORD);
                break;
            case R.id.id_main_menu_rb3:
                showFragment(Constants.ID_FRAGMENT_DIAL);
                break;
            case R.id.id_main_menu_rb4:
                showFragment(Constants.ID_FRAGMENT_SETTING);
                break;
            default:
                break;
        }
    }


    //----------------------------------------

}