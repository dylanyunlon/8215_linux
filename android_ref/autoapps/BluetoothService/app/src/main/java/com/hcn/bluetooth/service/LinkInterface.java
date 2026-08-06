package com.hcn.bluetooth.service;

interface LinkInterface {
   String LINK_BROAD_NAME = "com.hcn.link";

   String EXTRA_CONNECT_STATE = "status";
   String DATA_STATUS_CONNECTED = "CONNECTED";
   String DATA_STATUS_DISCONNECTED = "DISCONNECTED";

   String EXTRA_TYPE = "type";
   String DATA_TYPE_CARPLAY = "carplay";
   String DATA_TYPE_AUTO = "androidauto";
   String DATA_TYPE_AIRPLAY = "airplay";
   String DATA_TYPE_MIRROR = "androidMirror";
   String DATA_TYPE_HICAR = "hicar";
   String DATA_TYPE_CARLIFE = "carlife";
   String DATA_TYPE_DLNA = "dlna";

   String EXTRA_BLUETOOTH = "bluetooth";
   String DATA_BLUETOOTH_STOP_A2DP = "stopA2dp";
   String DATA_BLUETOOTH_RESUME_A2DP = "resumeA2dp";
   String DATA_BLUETOOTH_CLOSE_BT = "closeBt";
   String DATA_BLUETOOTH_OPEN_BT = "openBt";
   String DATA_BLUETOOTH_NULL = "NULL";


}
