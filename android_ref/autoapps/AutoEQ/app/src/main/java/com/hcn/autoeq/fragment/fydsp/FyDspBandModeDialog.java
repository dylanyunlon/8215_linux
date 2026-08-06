package com.hcn.autoeq.fragment.fydsp;

import android.app.Dialog;
import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;

import com.hcn.autoeq.R;
import com.hcn.autoeq.bean.EventMessage;
import com.hcn.autoeq.bean.FyDspBandMode;
import com.hcn.autoeq.data.FyDspBandSettings;

import org.greenrobot.eventbus.EventBus;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class FyDspBandModeDialog extends DialogFragment implements View.OnClickListener {
    private static final String TAG = FyDspBandModeDialog.class.getSimpleName();

    private View mainView;

    private List<Button> bandModeList = new ArrayList<>();

    private Context context;
    private FyDspBandSettings fyDspBandSettings;

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        this.context = context;
        fyDspBandSettings = FyDspBandSettings.getInstance(this.context);
    }

    public FyDspBandModeDialog() {
    }

    public static FyDspBandModeDialog newInstance() {
        FyDspBandModeDialog fragment = new FyDspBandModeDialog();
        Bundle args = new Bundle();
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setStyle(DialogFragment.STYLE_NO_TITLE, android.R.style.Theme_Dialog);
        if (getArguments() != null) {

        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        // 设置dialog背景透明，才可以显示圆角背景图
        getDialog().getWindow().setBackgroundDrawableResource(android.R.color.transparent);
        mainView = inflater.inflate(R.layout.fydsp_fragment_band_mode, container, false);

        Dialog dialog = getDialog();
        if (dialog != null) {
            Window window = dialog.getWindow();
            if (window != null) {
                // 设置 dialog 全屏
                window.getDecorView().setPadding(0, 0, 0, 0);
                WindowManager.LayoutParams lp = window.getAttributes();
                lp.width = WindowManager.LayoutParams.MATCH_PARENT;
                lp.height = WindowManager.LayoutParams.MATCH_PARENT;
                window.setAttributes(lp);

                // 状态栏透明
                window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
                window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
                window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
                window.setStatusBarColor(Color.TRANSPARENT);
            }
        }

        return mainView;
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        Button btnBandModeEdit = mainView.findViewById(R.id.btn_band_mode_edit);
        btnBandModeEdit.setOnClickListener(this);

        Button btnBandModeUser = mainView.findViewById(R.id.btn_band_mode_custom);
        Button btnBandModeStandard = mainView.findViewById(R.id.btn_band_mode_standard);
        Button btnBandModeNews = mainView.findViewById(R.id.btn_band_mode_news);
        Button btnBandModeJazz = mainView.findViewById(R.id.btn_band_mode_jazz);
        Button btnBandModeCity = mainView.findViewById(R.id.btn_band_mode_city);
        Button btnBandModePop = mainView.findViewById(R.id.btn_band_mode_pop);
        Button btnBandModeElectronic = mainView.findViewById(R.id.btn_band_mode_electronic);
        Button btnBandModeClassics = mainView.findViewById(R.id.btn_band_mode_classics);
        Button btnBandModeMovie = mainView.findViewById(R.id.btn_band_mode_movie);
        Button btnBandModeRock = mainView.findViewById(R.id.btn_band_mode_rock);
        Button btnBandModeTechno = mainView.findViewById(R.id.btn_band_mode_techno);

        bandModeList = Arrays.asList(btnBandModeUser, btnBandModeStandard, btnBandModeNews, btnBandModeJazz, btnBandModeCity, btnBandModePop
                , btnBandModeElectronic, btnBandModeClassics, btnBandModeMovie, btnBandModeRock, btnBandModeTechno);
        bandModeList.forEach(btn -> btn.setOnClickListener(this));

        // 当前模式高亮选中（Button 的 tag 必须和 FyDspBandMode 枚举一致）
        String bandMode = fyDspBandSettings.getBandMode().name();
        bandModeList.stream().filter(btn -> btn.getTag().toString().equals(bandMode)).findFirst().get().setSelected(true);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_band_mode_edit:
                dismiss();
                break;
            default:
                // 获取当前选中的按钮
                final String lastBandMode = (String) bandModeList.stream().filter(btn -> btn.isSelected()).findFirst().get().getTag();
                // 当前点击的按钮
                final String currentBandMode = (String) v.getTag();

                // 有变化时才发送事件
                if (currentBandMode != null && !currentBandMode.equals(lastBandMode)) {
                    // 把当前选中的模式传出去
                    EventBus.getDefault().post(new EventMessage(EventMessage.MSG_BAND_MODE_CHANGED, FyDspBandMode.valueOf(currentBandMode)));
                }
                // 其他所有按钮取消高亮
                bandModeList.forEach(btn -> btn.setSelected(false));
                // 当前点击的按钮高亮
                v.setSelected(true);
                break;
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        dismiss();
    }
}
