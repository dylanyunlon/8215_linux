package com.hcn.autoeq.adapter;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CompoundButton;
import android.widget.ToggleButton;

import com.hcn.autoeq.R;
import com.hcn_library.util.SkinUtils;

import java.util.Arrays;

/**
 * 九品高低通斜率面板适配器，使用 gridview 来显示和管理斜率选项。
 */
public class NineFilterGirdAdapter extends BaseAdapter {
    // 存储斜率选项的字符串数组
    private String[] slopeOptionsArray;
    // 用于监听 ToggleButton 状态改变的监听器
    private ToggleButtonStateChangeListener stateChangeListener;
    // 存储上下文信息
    private Context context;
    // 存储每个选项的选中状态
    public boolean[] selectedStates;
    // 网格的列数
    private int columnCount = 3;
    // 网格的行数
    private int rowCount = 2;
    // 开关是否启用
    private boolean isEnabled = false;

    /**
     * 此接口用于监听 ToggleButton 的状态改变，并将相应的信息传递给外部。
     */
    public interface ToggleButtonStateChangeListener {
        /**
         * 当 ToggleButton 的状态发生改变时调用此方法。
         *
         * @param position  发生状态改变的 ToggleButton 的位置
         * @param isChecked 状态是否为选中
         * @param isPressed 是否正在被按下
         */
        void onToggleButtonStateChanged(int position, boolean isChecked, boolean isPressed);
    }

    @Override
    public long getItemId(int id) {
        // 返回当前项的 id，这里简单地返回传入的 id
        return id;
    }

    /**
     * 设置 ToggleButton 状态改变的监听器。
     *
     * @param toggleButtonStateChangeListener 监听器实例
     */
    public void setToggleButtonStateChangeListener(ToggleButtonStateChangeListener toggleButtonStateChangeListener) {
        stateChangeListener = toggleButtonStateChangeListener;
    }

    /**
     * 构造函数，初始化适配器所需的上下文和斜率选项数据。
     *
     * @param context        上下文对象
     * @param slopeOptions   斜率选项的字符串数组
     */
    public NineFilterGirdAdapter(Context context, String[] slopeOptions) {
        this.context = context;
        slopeOptionsArray = slopeOptions;
        selectedStates = new boolean[slopeOptions.length];
    }

    @Override
    public int getCount() {
        // 返回斜率选项的数量
        return slopeOptionsArray.length;
    }

    @Override
    public Object getItem(int i) {
        // 根据位置返回相应的斜率选项
        return slopeOptionsArray[i];
    }

    // 绑定布局，每次更新时刷新每个按钮的状态，包括是否可用和是否选中
    @Override
    public View getView(final int position, View convertView, ViewGroup viewGroup) {
        ViewHolder holder;
        // 如果 convertView 为 null，则创建新的视图并设置 ViewHolder
        if (convertView == null) {
            convertView= LayoutInflater.from(SkinUtils.getContext()).inflate(SkinUtils.getId(R.layout.nine_filter_grid_item_layout), null, false);
            holder = new ViewHolder();
            holder.toggleButton = (ToggleButton) convertView.findViewById(R.id.btn_in_grid_item);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }
        ToggleButton toggleButton = holder.toggleButton;
        // 设置 ToggleButton 是否启用
        toggleButton.setEnabled(isEnabled);
        toggleButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    // 当按钮按下时，调用监听器的方法
                    if (stateChangeListener!= null) {
                        stateChangeListener.onToggleButtonStateChanged(position, true, true);
                    }
                    Log.d("NineFilterGirdAdapter", "onTouch is pressed " + toggleButton.isPressed() + " position: " + position + " is checked: " + toggleButton.isChecked());
                }
                return false;
            }
        });
        // 根据选中状态设置 ToggleButton 的选中状态
        if (selectedStates[position]) {
            toggleButton.setChecked(true);
        } else {
            toggleButton.setChecked(false);
        }
        // 设置 ToggleButton 的文本内容
        toggleButton.setText(slopeOptionsArray[position]);
        toggleButton.setTextOff(slopeOptionsArray[position]);
        toggleButton.setTextOn(slopeOptionsArray[position]);
        return convertView;
    }

    // 更新选中项，先将所有选项设为未选中，再将指定位置设为选中，并通知适配器更新
    public void setSelected(int position) {
        Log.d("setSelected", "position: " + position);
        isEnabled = true;
        // 将所有选中状态置为 false
        Arrays.fill(selectedStates, false);
        selectedStates[position] = true;
        notifyDataSetChanged();
    }

    // 将所有选项设为未选中，关闭开关，并通知适配器更新
    public void setAllUnselected() {
        isEnabled = false;
        Arrays.fill(selectedStates, false);
        notifyDataSetChanged();
    }

    /**
     * 获取选中项的位置，如果没有选中项则返回 0。
     *
     * @return 选中项的位置，如果没有选中项则返回 0
     */
    public int getSelectedPosition() {
        for (int i = 0; i < selectedStates.length; i++) {
            if (selectedStates[i]) {
                return i;
            }
        }
        return 0;
    }

    // ViewHolder 模式，避免重复调用 findViewById
    private static class ViewHolder {
        ToggleButton toggleButton;
    }
}