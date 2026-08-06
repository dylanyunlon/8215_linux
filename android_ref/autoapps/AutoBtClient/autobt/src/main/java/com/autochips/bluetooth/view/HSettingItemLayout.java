package com.autochips.bluetooth.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.widget.LinearLayout;

import androidx.annotation.Nullable;

import java.util.ArrayList;
import java.util.List;

/**
 * 辅助处理Item类型的聚焦显示。
 * <p>
 * T5右侧的视图显示，如是SettingItemView可以辅助处理，其他的要自行处理。
 */
public class HSettingItemLayout extends LinearLayout {
    private List<HSettingsItemView> mSettingViewList;
    private List<View> mOtherViewList;
    private HSettingsItemView.OnItemViewExpandChangedListener mExpandChangedListener;
    private HSettingsItemView.OnItemViewCheckedChangedListener mCheckedChangedListener;
    //
    private ClickListener mClickListener;

    public HSettingItemLayout(Context context) {
        this(context, null);
    }

    public HSettingItemLayout(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public HSettingItemLayout(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        setOrientation(VERTICAL);
        addListener();
    }

    public void setOnItemViewCheckedChangedListener(HSettingsItemView.OnItemViewCheckedChangedListener listener) {
        mCheckedChangedListener = listener;
        addListener();
    }

    public void setOnItemViewExpandChangedListener(HSettingsItemView.OnItemViewExpandChangedListener listener) {
        mExpandChangedListener = listener;
    }

    private void addListener() {

        int count = getChildCount();
        if (count > 0) {
            SettingItemExpandListener mExpandListener = null;
            SettingItemSwitchListener mSwitchListener = null;
            if (null == mSwitchListener) {
                mSwitchListener = new SettingItemSwitchListener();
            }
            if (null == mExpandListener) {
                mExpandListener = new SettingItemExpandListener();
            }
            if (null == mClickListener) {
                mClickListener = new ClickListener();
            }
            if (null == mSettingViewList) {
                mSettingViewList = new ArrayList<>();
            }
            if (null == mOtherViewList) {
                mOtherViewList = new ArrayList<>();
            }
            mSettingViewList.clear();
            mOtherViewList.clear();
            for (int i = 0; i < count; i++) {
                View v = getChildAt(i);
                if (v instanceof HSettingsItemView) {
                    ((HSettingsItemView) v).setOnItemViewCheckedChangedListener(mSwitchListener);
                    ((HSettingsItemView) v).setOnItemViewExpandChangedListener(mExpandListener);
//                    if (i == 0) {
//                        ((HSettingsItemView) v).openExpandView();
//                    }
                    mSettingViewList.add((HSettingsItemView) v);
                } else {
                    mOtherViewList.add(v);
                    v.setOnClickListener(mClickListener);
                }
            }
        }
    }

    /**
     *
     */
    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        super.onLayout(changed, l, t, r, b);
        addListener();
    }

    /**
     * 修改VIEW的聚焦
     */
    private void resetViewFocus(View v) {
        for (HSettingsItemView view : mSettingViewList) {
            if (view.getId() != v.getId()) {
                view.closeExpandView();
            }
        }
        //
        for (View view : mOtherViewList) {
            if (view.getId() != v.getId()) {
                view.setSelected(false);
            }
        }
    }


    private class SettingItemSwitchListener implements HSettingsItemView.OnItemViewCheckedChangedListener {
        @Override
        public void onItemViewCheckedChanged(View v, boolean isChecked) {
            if (mCheckedChangedListener != null) {
                mCheckedChangedListener.onItemViewCheckedChanged(v, isChecked);
            }
            resetViewFocus(v);
        }
    }

    private class SettingItemExpandListener implements HSettingsItemView.OnItemViewExpandChangedListener {
        @Override
        public void onItemViewExpand(View v, View expandView, boolean isExpand, boolean isFormUser) {
            if (isFormUser) {//手点有效
                if (mExpandChangedListener != null) {
                    mExpandChangedListener.onItemViewExpand(v, expandView, isExpand, true);
                }
                resetViewFocus(v);
            }
        }
    }

    private class ClickListener implements OnClickListener {
        @Override
        public void onClick(View v) {
            resetViewFocus(v);
            v.setSelected(true);
        }
    }
}
