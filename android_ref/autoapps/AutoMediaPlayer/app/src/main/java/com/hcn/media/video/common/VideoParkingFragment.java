package com.hcn.media.video.common;

import android.os.Bundle;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.media.base.fragment.MediaFragment;
import com.hcn.media.R3;

/**
 * 安全提示信息页面
 * @author 86158
 */
public class VideoParkingFragment extends MediaFragment {
    private boolean mInitView = false;

    /**
     * 安全警告信息提示文本控件
     * <p> 例如：行车中，不允许观看视频。
     */
    private TextView mTvParking = null;

    public VideoParkingFragment() {
        super(null);
    }

    @Override
    public void initFragment() {
    }

    @Override
    public void doCallbackEvent(int eventId) {
    }

    @Override
    public int getLayoutRes() {
        return R.layout.fragment_videoparking;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container,
            @Nullable Bundle savedInstanceState) {
        // 兼容 Android-skin-support 支持
        View view = super.onCreateView(inflater, container, savedInstanceState);

        assert view != null;
        initView(view);
        initFragment();
        mInitView = true;
        return view;
    }

    private void initView(@NonNull View layout) {
        mTvParking = layout.findViewById(xId(R.id.tvParking));
    }

    @Override
    public void onResume() {
        super.onResume();

        updateFragmentInfo();
    }

    @Override
    protected void onPostboxMediaEvent(int eventId, Object wParam, Object lParam) {
        // TODO: 预留接口
    }

    /**
     * 更新页面信息
     * <pre>
     *    这里元素单一，主要是为了刷新页面相关的信息；
     *    由于扩展皮肤包中不会包含全部多国语言，所以文本类信息我们采用手动更新设置；
     * </pre>
     */
    private void updateFragmentInfo() {
        if (mTvParking != null) {
            mTvParking.setText(getString(R3.string.tip_parking_info));
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }
}
