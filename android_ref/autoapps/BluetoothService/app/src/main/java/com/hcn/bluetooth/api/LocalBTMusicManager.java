package com.hcn.bluetooth.api;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import com.hcn.bluetooth.service.IBluetoothMusicService;
import com.hcn.bluetooth.service.IMusicCallback;

import java.util.ArrayList;
import java.util.List;

public class LocalBTMusicManager {
    public static final String TAG = "LocalBTMusicManager";

    /**
     * Requires android.Manifest.permission#BLUETOOTH permission to receive. contains 3 extras
     * BluetoothProfile.EXTRA_STATE BluetoothProfile.EXTRA_PREVIOUS_STATE
     * BluetoothDevice.EXTRA_DEVICE
     */
    public static final String ACTION_CONNECTION_STATE_CHANGED =
            "android.bluetooth.a2dp-sink.profile.action.CONNECTION_STATE_CHANGED";

    /**
     * Requires android.Manifest.permission#BLUETOOTH permission to receive. contains 2 extras
     * PlaybackState pbb = intent.getParcelableExtra(EXTRA_PLAYBACK); MediaMetadata mmd =
     * intent.getParcelableExtra(EXTRA_METADATA);
     */
    public static final String ACTION_TRACK_EVENT =
            "android.bluetooth.avrcp-controller.profile.action.TRACK_EVENT";
    public static final String EXTRA_METADATA =
            "android.bluetooth.avrcp-controller.profile.extra.METADATA";
    public static final String EXTRA_PLAYBACK =
            "android.bluetooth.avrcp-controller.profile.extra.PLAYBACK";


    /**
     * used by send_Avrcp_Cmd方法
     */
    public static final int CMD_AVRCP_PLAY = 0;
    public static final int CMD_AVRCP_PAUSE = 1;
    public static final int CMD_AVRCP_PLAY_PAUSE = 2;
    public static final int CMD_AVRCP_STOP = 3;
    public static final int CMD_AVRCP_NEXT = 4;
    public static final int CMD_AVRCP_PREV = 5;
    public static final int CMD_AVRCP_FAST_FORWARD = 6;
    public static final int CMD_AVRCP_REWIND = 7;

    private static LocalBTMusicManager sInstance;
    private Context mContext;
    private boolean isBound = false;
    private IBluetoothMusicService mService = null;
    private List<ConnectionListener> mListeners = new ArrayList<>();

    public static LocalBTMusicManager getInstance() {
        if (sInstance == null) {
            sInstance = new LocalBTMusicManager();
        }
        return sInstance;
    }

    private LocalBTMusicManager() {
        Log.i(TAG, "LocalBluetoothMusicManager Create");
    }

    public LocalBTMusicManager init(Context context) {
        Log.i(TAG, "init");
        if (null == mContext) {
            mContext = context.getApplicationContext();
        }
        if (isBound == false) {
            Intent intent = new Intent(
                    "com.hcn.bluetooth.service.BluetoothMusicService");
            intent.setClassName("com.hcn.bluetoothservice",
                    "com.hcn.bluetooth.service.BluetoothMusicService");
            mContext.bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
        }
        return sInstance;
    }

    public void unInit() {
        Log.i(TAG, "unInit");
        if (isBound) {
            mContext.unbindService(mConnection);
        }
        isBound = false;
        mContext = null;
    }

    public boolean isReady() {
        return isBound;
    }

    public void addConnectListener(ConnectionListener listener) {
        if (null != listener) {
            mListeners.remove(listener);
            mListeners.add(listener);
        }
    }

    public void removeConnectListener(ConnectionListener listener) {
        if (null != listener) {
            mListeners.remove(listener);
        }
    }

    public void callConnectListener(boolean connect) {
        for (int i = 0; i < mListeners.size(); i++) {
            if (connect) {
                mListeners.get(i).onServiceConnected();
            } else {
                mListeners.get(i).onServiceDisconnected();
            }
        }
    }

    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "onServiceDisconnected");
            isBound = false;
            mService = null;
            callConnectListener(false);
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "onServiceConnected");
            mService = IBluetoothMusicService.Stub.asInterface(service);
            isBound = true;
            callConnectListener(true);
        }
    };

    public boolean isA2dpConnected() {
        Log.i(TAG, "isA2dpConnected");
        if (isBound) {
            try {
                return mService.isA2dpConnected();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    public boolean isAvrcpConnected() {
        Log.i(TAG, "isAvrcpConnected");
        if (isBound) {
            try {
                return mService.isAvrcpConnected();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    public boolean isA2dpPlaying() {
        Log.i(TAG, "isA2dpPlaying");
        if (isBound) {
            try {
                return mService.isA2dpPlaying();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    public BluetoothDeviceInfo getConnectDevice() {
        if (isBound) {
            try {
                return mService.getConnectDevice();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return null;
    }

    public void send_Avrcp_Cmd(int avrcp_cmd) {
        Log.i(TAG, "send_Avrcp_Cmd cmd=" + avrcp_cmd);
        if (isBound) {
            try {
                mService.send_Avrcp_Cmd(avrcp_cmd);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public void requestA2dp() {
        Log.i(TAG, "requestA2dp");
        if (isBound) {
            try {
                mService.requestA2dp();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public void releaseA2dp() {
        Log.i(TAG, "releaseA2dp");
        if (isBound) {
            try {
                mService.releaseA2dp();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public void regMusicClientBinder(IBinder cBinder) {
        if (isBound) {
            if (mService != null) {
                try {
                    mService.regMusicClientBinder(cBinder);
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }

    }

    public void unRegMusicClientBinder() throws RemoteException {
        if (isBound) {
            if (mService != null) {
                try {
                    mService.unRegMusicClientBinder();
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
    }

    public void registerBTMusicCallback(IMusicCallback callback) {
        if (isBound) {
            if (mService != null) {
                try {
                    mService.registerBTMusicCallback(callback);
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }

    }

    public void unRegisterBTMusicCallback(IMusicCallback callback) {
        if (isBound) {
            if (mService != null) {
                try {
                    mService.unregisterBTMusicCallback(callback);
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
    }

    public String[] getID3Info(){
        if (isBound) {
            if (mService != null) {
                try {
                    return mService.getID3Info();
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
        return null;
    }
}
