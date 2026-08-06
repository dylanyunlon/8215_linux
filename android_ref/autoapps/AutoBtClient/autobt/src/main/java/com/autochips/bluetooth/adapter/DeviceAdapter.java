package com.autochips.bluetooth.adapter;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.TextView;

import com.autochips.bluetooth.R;
import com.autochips.bluetooth.util.SkinUtils;
import com.hcn.bluetooth.api.BluetoothDeviceInfo;

import java.util.ArrayList;
import java.util.List;

/**
 *  BOND_BONDED = 12;
 *  BOND_BONDING = 11;
 *  BOND_NONE = 10;
 */
public class DeviceAdapter extends BaseAdapter {

    public interface OnDeviceClickListener{
        void onClickConnect(BluetoothDeviceInfo device);
        void onClickDelete(BluetoothDeviceInfo device);
    }

    private List<BluetoothDeviceInfo> mData = new ArrayList<>();
    private Context mContext = null;
    private OnDeviceClickListener listen;
    private boolean bIsShowIcon = false;
    private String mNormalStr = "";
    private String mConnectStr = "";

    /**
     * 连接设备的地址
     */
    private BluetoothDeviceInfo mDevice = null;

    public void setOnDeviceClickListener(OnDeviceClickListener l){
        listen = l;
    }

    public DeviceAdapter(Context context, List<BluetoothDeviceInfo> data) {
        mContext = context;
        setData(data);
    }

    public DeviceAdapter(Context context, List<BluetoothDeviceInfo> data,String[] state) {
        mContext = context;
        setData(data);
    }

    /**
     *
     * @param context
     * @param data
     * @param normal 未连接的状态显示Bond none 和 Disconnect
     * @param focus 连接的状态显示   Bonded    和 Connected
     *              搜索列表是 Bond none  ，Bonded
     *              配对列表是 Disconnect ，Connected
     */
    public DeviceAdapter(Context context, List<BluetoothDeviceInfo> data
           , String normal,String focus,OnDeviceClickListener l) {
        this(context,data,normal,focus,l,false);
    }

    public DeviceAdapter(Context context, List<BluetoothDeviceInfo> data
            , String normal,String focus,OnDeviceClickListener l,boolean hasIcon) {
        mContext = context;
        setData(data);
        mNormalStr = normal;
        mConnectStr = focus;
        listen = l;
        bIsShowIcon = hasIcon;
    }


    public void setConnectDevice(BluetoothDeviceInfo info){
        mDevice = info;
        notifyDataSetChanged();
    }

    public void setData(List<BluetoothDeviceInfo> data){
        if(mData == null){
            mData = new ArrayList<>();
        }
        if(data != null){
            mData.clear();
            mData.addAll(data);
        }
    }

    public void setOnClickListener(OnDeviceClickListener l){
        listen = l;
    }

    @Override
    public int getCount() {
        return mData != null ? mData.size() : 0;
    }

    @Override
    public Object getItem(int i) {
        return mData != null ? mData.get(i) : null;
    }

    @Override
    public long getItemId(int i) {
        return 0;
    }

    @Override
    public View getView(int position, View view, ViewGroup viewGroup) {
        DeviceHold hold = null;
        if (view == null) {
            view = LayoutInflater.from(mContext).inflate(SkinUtils.getId(R.layout.adapter_device), null);
            hold = new DeviceHold();
            hold.button = view.findViewById(R.id.id_device_adapter_button);
            hold.title = view.findViewById(R.id.id_device_adapter_title);
            hold.state = view.findViewById(R.id.id_device_adapter_state);
            hold.button.setVisibility(bIsShowIcon ? View.VISIBLE : View.INVISIBLE);
            view.setTag(hold);
        } else {
            hold = (DeviceHold) view.getTag();
        }
        BluetoothDeviceInfo current = mData.get(position);
        if(current.getDeviceStatus() == BluetoothDeviceInfo.DeviceStatus.DEVICE_STATUS_CONNECTED
                && mDevice != null
                && current.getDeviceAddr().equals(mDevice.getDeviceAddr())){
            hold.state.setText(mConnectStr);
            hold.title.setTextColor(SkinUtils.getColor(R.color.color_adapter_title_focus));
            hold.state.setTextColor(SkinUtils.getColor(R.color.color_adapter_title_focus));
        }else{
            hold.state.setText(mNormalStr);
            hold.title.setTextColor(SkinUtils.getColorStateList(R.color.colors_menu_title));
            hold.state.setTextColor(SkinUtils.getColorStateList(R.color.colors_menu_title));
        }
        hold.title.setText(current.getDeviceName());
        hold.button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(listen != null && mData != null && position < mData.size()){
                    listen.onClickDelete(mData.get(position));
                }
            }
        });
        view.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(listen != null && mData != null && position < mData.size()){
                    listen.onClickConnect(mData.get(position));
                }
            }
        });

        return view;
    }

    class DeviceHold {
        private TextView title;
        private TextView state;
        private View button;
    }

    private String getString(BluetoothDeviceInfo info){
        //BluetoothDevice.BOND_BONDED = 12;
        //初次直接从服务端取到的设备如是连接状态那么state == 1 (DEVICE_STATUS_CONNECTED)
        if(info.getDeviceStatus() == BluetoothDeviceInfo.DeviceStatus.DEVICE_STATUS_CONNECTED
                && mDevice != null
                && info.getDeviceAddr().equals(mDevice.getDeviceAddr())){
            return mConnectStr;
        }else{
            return mNormalStr;
        }
    }
}
