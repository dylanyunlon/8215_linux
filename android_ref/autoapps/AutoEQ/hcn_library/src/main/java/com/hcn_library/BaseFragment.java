package com.hcn_library;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.skin.support.app.SkinCompatFragment;
import com.hcn_library.bean.EventMessage;
import com.hcn_library.util.SkinUtils;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

/**
 * 基类Fragment
 * DSP：DSPFragment
 * ASP：ASPFragment
 * Balance：BalanceFragment
 * Fragment都要继承该类
 */
public abstract class BaseFragment extends SkinCompatFragment {
    private static final String TAG = "BaseFragment";
    protected Context mContext;
    protected boolean mIsLayoutIdExit;

    /**
     * 当该类被系统创建的时候回调
     *
     * @param savedInstanceState
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = getActivity();
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        initView();
        initData();
        if (!EventBus.getDefault().isRegistered(this)) {
            EventBus.getDefault().register(this);
        }
    }

    //抽象类，由子类实现，实现不同的效果
    public abstract void initView();

    /**
     * 当子类初始化界面数据使用
     */
    public void initData() {
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (EventBus.getDefault().isRegistered(this)) {
            EventBus.getDefault().unregister(this);
        }
    }

    @Subscribe()
    public void onEvent(EventMessage eventMessage) {
    }

    public void onBindViewData() {

    }

    @Override
    public int getLayoutRes() {
        return 0;
    }

    public boolean checkLayoutExists(int layoutId) {
        mIsLayoutIdExit = true;
        int id = SkinUtils.getId(layoutId);
        if (id == 0) {
            Log.e(TAG, "getLayoutRes: layout not found.");
            mIsLayoutIdExit = false;
        }
        return mIsLayoutIdExit;
    }

}
