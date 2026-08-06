package com.hcn.autoeq.nine;

import android.content.Context;
import android.graphics.Rect;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.fragment.app.DialogFragment;

import com.hcn.autoeq.R;
import com.hcn_library.data.NineDspBandSettings;
import com.hcn_library.util.NineConstantExtDsp;

import java.util.Locale;

public class NineDspQValueFragment extends DialogFragment implements View.OnClickListener, NineConstantExtDsp {
    private static final String TAG = NineDspQValueFragment.class.getSimpleName();

    private Context context;
    private NineDspBandSettings nineDspBandSettings;
    private final int SEEK_BAR_MIN = 500; // q值调节最小为0.5
    private final int SEEK_BAR_MAX = 6000; // q值调节最大为6
    private SeekBar seekBar;
    private TextView tvQValue;
    private int position, gainValue, currentQValue;
    private Callback callback;

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        this.context = context;
        nineDspBandSettings = NineDspBandSettings.getInstance(this.context);
    }

    public NineDspQValueFragment() {
    }

    /**
     * 把当前调节的数据传入到对话框中
     *
     * @param position      第几段索引
     * @param currentQValue 当前调节的q值
     * @return
     */
    public static NineDspQValueFragment newInstance(int position, int gainValue, int currentQValue) {
        NineDspQValueFragment fragment = new NineDspQValueFragment();
        Bundle args = new Bundle();
        args.putInt("position", position);
        args.putInt("gainValue", gainValue);
        args.putInt("currentQValue", currentQValue);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getArguments() != null) {
            position = getArguments().getInt("position");
            gainValue = getArguments().getInt("gainValue");
            currentQValue = getArguments().getInt("currentQValue");
        }
    }

    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        // 设置dialog背景透明，才可以显示圆角背景图
        getDialog().getWindow().setBackgroundDrawableResource(android.R.color.transparent);
        View view = inflater.inflate(R.layout.nine_dsp_fragment_qvalue, container, false);
//        hcn.view.setOnClickListener(this); // 触摸任何地方都关闭窗口

        seekBar = view.findViewById(R.id.sb_qvalue);
        //防止点击父布局时，导致seekbar也响应点击事件，资源文件显示被点击状态，出现未响应的错觉
        view.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                // 获取点击位置
                int x = (int) event.getX();
                int y = (int) event.getY();

                // 检查点击是否发生在 SeekBar 内部
                Rect seekBarRect = new Rect();
                seekBar.getGlobalVisibleRect(seekBarRect);
                if (!seekBarRect.contains(x, y)) {
                    return true; // 消费点击事件，不在 SeekBar 上
                }

                return false; // 不消费点击事件，让 SeekBar 处理
            }
        });
        // FIXME, 换肤方式，在xml里设置 padding=0, offset=0，滑块依然会超出，通过代码设置后正常
        seekBar.setPadding((int) getResources().getDimension(R.dimen.x0), (int) getResources().getDimension(R.dimen.x0)
                , (int) getResources().getDimension(R.dimen.x0), (int) getResources().getDimension(R.dimen.x0));
        seekBar.setThumbOffset((int) getResources().getDimension(R.dimen.x0));
        seekBar.setMax(SEEK_BAR_MAX);
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (!fromUser) return;
                // 不能小于最低的范围值
                if (progress < SEEK_BAR_MIN) {
                    progress = SEEK_BAR_MIN;
                }
                tvQValue.setText(String.format(Locale.getDefault(), "%.1f", progress / 1000f).replace(".0", ""));
                nineDspBandSettings.nativeBand(position, gainValue, progress);
                if (callback != null) {
                    callback.onQValueChanged(position, progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
        this.tvQValue = view.findViewById(R.id.tv_q_value);
        return view;
    }

    @Override
    public void onResume() {
        super.onResume();
        seekBar.setProgress(currentQValue);
        tvQValue.setText(String.format(Locale.getDefault(), "%.1f", currentQValue / 1000f).replace(".0", ""));
    }

    @Override
    public void onClick(View v) {
        dismiss();
    }

    public interface Callback {
        void onQValueChanged(int position, int qValue);
    }
}
