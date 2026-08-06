package com.hcn.autoeq.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.hcn.autoeq.R;
import com.hcn_library.util.SkinUtils;

/**
 * 九品 band 混响模式调节面板适配器，使用 gridview
 */
public class NineEQGridViewAdapter extends BaseAdapter {
    // 存储上下文，用于获取资源和服务等操作
    private Context context;
    // 存储要显示的数据列表，混响模式数组
    private String[] dataList;
    // 存储每个数据项的选中状态，使用 Boolean 包装类方便进行 null 检查
    private Boolean[] itemSelectedStates;

    @Override
    public long getItemId(int position) {
        // 返回当前项的位置作为唯一标识符
        return position;
    }

    /**
     * 构造函数，初始化适配器所需的上下文和数据列表
     *
     * @param context  上下文对象
     * @param dataList 要显示的数据列表
     */
    public NineEQGridViewAdapter(Context context, String[] dataList) {
        this.context = context;
        this.dataList = dataList;
        // 根据数据列表的长度初始化选中状态列表，初始都为未选中（null）
        itemSelectedStates = new Boolean[dataList.length];
    }

    @Override
    public int getCount() {
        // 返回数据列表的长度，即要显示的项数
        return dataList.length;
    }

    @Override
    public Object getItem(int position) {
        // 根据位置返回相应的数据项
        return dataList[position];
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder viewHolder;
        // 如果 convertView 为 null，创建新的视图并设置 ViewHolder
        if (convertView == null) {
            // 使用 SkinUtils 从相应布局文件中加载视图，避免直接使用资源 ID
            convertView = LayoutInflater.from(SkinUtils.getContext()).inflate(SkinUtils.getId(R.layout.nine_dsp_eq_grid_item), (ViewGroup) null, false);
            viewHolder = new ViewHolder();
            // 查找并存储 TextView 组件
            viewHolder.textView = (TextView) convertView.findViewById(SkinUtils.getId(R.id.item_text));
            // 查找并存储分割线视图组件
            viewHolder.divider = convertView.findViewById(SkinUtils.getId(R.id.dividerBottom));
            convertView.setTag(viewHolder);
        } else {
            // 从 convertView 的 tag 中获取已有的 ViewHolder
            viewHolder = (ViewHolder) convertView.getTag();
        }
        // 设置 TextView 的文本内容
        viewHolder.textView.setText(dataList[position]);
        // 根据选中状态设置 TextView 的选中状态
        viewHolder.textView.setSelected(itemSelectedStates[position]!= null && itemSelectedStates[position].booleanValue());
        // 根据位置显示或隐藏分割线
        if (position == 2 || position == 3) {
            viewHolder.divider.setVisibility(View.VISIBLE);
        } else {
            viewHolder.divider.setVisibility(View.GONE);
        }
        return convertView;
    }

    private static class ViewHolder {
        // 分割线视图
        View divider;
        // 显示文本的 TextView
        TextView textView;

        private ViewHolder() {
        }
    }

    /**
     * 设置选中的项，将其他项置为未选中，并更新适配器
     *
     * @param selectedPosition 要选中的项的位置
     */
    public void setSelectedItem(int selectedPosition) {
        // 遍历状态列表，将所有项置为未选中
        for (int i = 0; i < dataList.length; i++) {
            itemSelectedStates[i] = false;
        }
        // 将指定位置的项置为选中
        itemSelectedStates[selectedPosition] = true;
        // 通知适配器数据发生变化，以便更新视图
        notifyDataSetChanged();
    }
}