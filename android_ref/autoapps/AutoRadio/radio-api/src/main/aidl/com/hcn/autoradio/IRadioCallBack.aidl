// IRadioCallBack.aidl
package com.hcn.autoradio;

interface IRadioCallBack {
 /**
     * 收音事件回调接口
     * <p> oneway 表示是异步调用；
     *
     * @param event 事件类型
     * @param arg0 附加参数 1
     */
    oneway void onEvent(int event, String arg0);
}