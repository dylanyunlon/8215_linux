package com.hcn.bluetooth.api;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import com.hcn.bluetooth.service.IBluetoothHfpclientService;

public final class LocalBluetoothHfpclientManager {

    public static final String TAG = "LocalHfpClientManager";

    private static LocalBluetoothHfpclientManager sInstance;

    private Context mContext = null;
    private boolean isBound = false;
    private IBluetoothHfpclientService mService = null;

    public static synchronized LocalBluetoothHfpclientManager getInstance() {
        if (sInstance == null) {
            sInstance = new LocalBluetoothHfpclientManager();
        }
        return sInstance;
    }

    private LocalBluetoothHfpclientManager() {
        Log.i(TAG, "LocalBluetoothHfpclientManager Create");
    }

    public void init(Context context) {
        Log.i(TAG, "init");
        if (null == context) {
            throw new IllegalArgumentException(TAG
                    + "init(Context context): conext is null");
        }
        if (null != mContext) {
            Log.e(TAG, "mContext is not null " + mContext.toString());
        }
        mContext = context.getApplicationContext();
        // mContext.bindService(new Intent(context,
        // BluetoothHfpclientService.class),mConnection,Context.BIND_AUTO_CREATE);
        Intent intent = new Intent(
                "com.hcn.bluetooth.service.BluetoothHfpclientService");
        intent.setClassName("com.hcn.bluetoothservice",
                "com.hcn.bluetooth.service.BluetoothHfpclientService");
        mContext.bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }



    public boolean isReady() {
        return isBound;
    }

    public void unInit() {
        Log.i(TAG, "unInit");
        if (isBound) {
            mContext.unbindService(mConnection);
        }
        isBound = false;
        mContext = null;
        mService = null;
        sInstance = null;
    }

    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "onServiceDisconnected");
            isBound = false;
            mService = null;
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "onServiceConnected");
            mService = IBluetoothHfpclientService.Stub.asInterface(service);
            isBound = true;
        }
    };

    public synchronized boolean isAudioConnected() {
        Log.i(TAG, "isAudioConnected");
        if (isBound) {
            try {
                return mService.isAudioConnected();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return false;
    }

    public synchronized void dial(String number) {
        Log.i(TAG, "dial:" + number + " isBound:" + isBound);
        if (null != mService) {
            try {
                mService.dial(number);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, "mService == null");
        }
    }

    public synchronized void sendDTMF(byte code) {
        Log.i(TAG, "sendDTMF:" + code);
        if (isBound) {
            try {
                mService.sendDTMF(code);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public synchronized void acceptCall(int flag) {
        Log.i(TAG, "pickup");
        if (isBound) {
            try {
                mService.acceptCall(flag);
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public synchronized void hangup() {
        Log.i(TAG, "hangup");
        if (isBound) {
            try {
                mService.hangup();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public synchronized void switchAudio() {
        Log.i(TAG, "switchAudio");
        if (isBound) {
            try {
                mService.switchAudio();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    public synchronized String getLastCall() {
        Log.i(TAG, "getLastCall");
        if (isBound) {
            try {
                return mService.getLastCall();
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return "";
    }
}
