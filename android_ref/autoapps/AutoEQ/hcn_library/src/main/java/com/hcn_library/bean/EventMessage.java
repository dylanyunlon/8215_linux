package com.hcn_library.bean;

import android.content.Context;

public class EventMessage {

    private static final String TAG = EventMessage.class.getSimpleName();

    // 全局事件，任何界面有任何操作，都通知取消 user 模式
    public static final String MSG_STICKY_ANY_CHANGED = "msg_any_changed"; // 有任何操作事件
    public static final String MSG_STICKY_USER_MODE_CHANGED = "msg_user_mode_changed";

    public static final String MSG_BAND_MODE_CHANGED = "msg_band_mode_changed"; // 模式切换事件
    public static final String MSG_BAND_FREQ_CHANGED = "msg_band_freq_changed"; // 频点修改事件
    public static final String MSG_BAND_Q_VALUE_CHANGED = "msg_band_qvalue_changed"; // q值修改事件

    public static final String MSG_BALANCE_CHANGE_TO_DELAY = "msg_balance_change_to_delay"; // 切换成延时事件
    public static final String MSG_DELAY_CHANGE_TO_BALANCE = "msg_delay_change_to_balance"; // 切换成声场事件

    // 需要使用粘性事件（接收界面没初始化时收不到事件，待接收界面初始化后也能收到事件）
    public static final String MSG_STICKY_OUTPUT_MODE_CHANGED = "msg_output_mode_changed"; // 输出模式切换事件

    /**
     * 在 USER 模式下有任何变化时，把数据复制到自定义模式
     * @param context
     * @param caller  调试用，记录调用者的类名方法名
     */
    public static void anyChanged(Context context, String caller) {
//        Log.d(TAG, "anyChanged call from : " + caller);
//        FyDspBandSettings fyDspBandSettings = FyDspBandSettings.getInstance(context);
//        String userModeFrom = fyDspBandSettings.getUserMode();
//        if (!StringUtils.isTrimEmpty(userModeFrom)) { // 在 user 模式下发生改变
//            // 把所有的数据都复制一份到自定义模式数据中
//            FyDspBandSettings.getInstance(context).reload(userModeFrom, "");
//            FyDspBalanceSettings.getInstance(context).reload(userModeFrom, "");
//            FyDspDelaySettings.getInstance(context).reload(userModeFrom, "");
//            FyDspSurroundSettings.getInstance(context).reload(userModeFrom, "");
//            FyDspAttenuateSettings.getInstance(context).reload(userModeFrom, "");
//            FyDspHLPFSettings.getInstance(context).reload(userModeFrom, "");
//        }
//
//        EventBus.getDefault().postSticky(new EventMessage(MSG_STICKY_ANY_CHANGED));
    }

    private String message;
    private Object data;

    public EventMessage(String message) {
        this.message = message;
    }

    public EventMessage(String message, Object data) {
        this.message = message;
        this.data = data;
    }

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

    public Object getData() {
        return data;
    }

    public void setData(Object data) {
        this.data = data;
    }
}
