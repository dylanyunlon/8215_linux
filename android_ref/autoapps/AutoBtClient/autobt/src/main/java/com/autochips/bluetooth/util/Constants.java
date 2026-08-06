package com.autochips.bluetooth.util;

public interface Constants {
    public static final int ID_FRAGMENT_UNDEFINE = -1;
    public static final int ID_FRAGMENT_CONTACT = 0;
    public static final int ID_FRAGMENT_RECORD = 1;
    public static final int ID_FRAGMENT_DIAL = 2;
    public static final int ID_FRAGMENT_SETTING = 3;


    public static final String BT_POWER_PROP = "persist.sys.BT_Status";
    public static final String BT_NAME_PROP = "persist.sys.BTName";

    public static final String DEVICE_SERIAL = "ro.boot.serialno";
    //public static final String DEVICE_MACHINECODE = "persist.sys.machinebarcode";
    public static final String DEVICE_MACHINECODE = "no.such.thing";

    //通知service刷新通话框
    public final String ACTION_BT_UI_CHANGE = "ACTION_UPDATE_BT_CALL_VIEW";


}
