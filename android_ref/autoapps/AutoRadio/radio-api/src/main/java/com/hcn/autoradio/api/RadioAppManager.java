package com.hcn.autoradio.api;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import androidx.annotation.NonNull;

import com.hcn.autoradio.IRadioCallBack;
import com.hcn.autoradio.IRadioServiceAPI;

import java.util.ArrayList;
import java.util.List;


/**
 * 外部调用收音代理
 *
 * @author simon
 */
public class RadioAppManager {
    private static final String TAG = "RadioAppManager";

    private Context mContext = null;
    private boolean mIsBindService = false;
    private Intent mServiceIntent = null;
    private IRadioServiceAPI mFMPlugService = null;

    public static RadioAppManager sInstance = null;

    private List<IConnectionListener> mListeners = new ArrayList<>();

    public static final int BAND_FM_1 = 0;
    public static final int BAND_FM_2 = 1;
    public static final int BAND_FM_3 = 2;
    public static final int BAND_AM_1 = 3;
    public static final int BAND_SIZE = 4;

    public static RadioAppManager getInstance() {
        if (sInstance == null) {
            sInstance = new RadioAppManager();
        }
        return sInstance;
    }


    public RadioAppManager() {
        Log.i(TAG, "RadioAppManager Create");
    }

    public void init(@NonNull Context context) {
        Log.i(TAG, "init");
        if (null == mContext) {
            mContext = context.getApplicationContext();
        }

        if (!mIsBindService) {
            mServiceIntent = new Intent("com.hcn.radio.FM_PLUG_SERVICE");
            mServiceIntent.setComponent(new ComponentName(
                    "com.hcn.autoradio",
                    "com.hcn.autoradio.service.FMPlugService"));
            mContext.bindService(mServiceIntent, mServiceConnection, Context.BIND_AUTO_CREATE);
        }
    }

    public void unInit() {
        if (null == mContext) {
            return;
        }
        Log.i(TAG, "unInit");
        if (mIsBindService) {
            if (null != mServiceConnection) {
                mContext.unbindService(mServiceConnection);
            }
        }
        mIsBindService = false;
        mContext = null;
    }

    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "onServiceConnected");
            mFMPlugService = IRadioServiceAPI.Stub.asInterface(service);
            mIsBindService = true;
            callConnectListener(true);
            try {
                mFMPlugService.requestPlayAudio();
                service.linkToDeath(mDeathRecipient, 0);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "onServiceDisconnected");
            mFMPlugService = null;
            mIsBindService = false;
            callConnectListener(false);
        }
    };

    public void addConnectListener(IConnectionListener listener) {
        if (null != listener) {
            mListeners.remove(listener);
            mListeners.add(listener);
        }
    }

    public void removeConnectListener(IConnectionListener listener) {
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

    private final IBinder.DeathRecipient mDeathRecipient = new IBinder.DeathRecipient() {
        @Override
        public void binderDied() {
            Log.v(TAG, " binderDied() remote service died,  mFMPlugService = " + mFMPlugService);
            if (mFMPlugService != null) {
                mFMPlugService.asBinder().unlinkToDeath(mDeathRecipient, 0);
                mFMPlugService = null;
            }
            mIsBindService = false;
        }
    };

    public boolean isBindSuccess() {
        return mIsBindService;
    }

    public void registerRadioClientBinder(IBinder clientBinder) {
        if (mIsBindService) {
            if (mFMPlugService != null) {
                try {
                    mFMPlugService.registerRadioClientBinder(clientBinder);
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
    }

    public void unRegisterRadioClientBinder() {
        if (mIsBindService) {
            if (mFMPlugService != null) {
                try {
                    mFMPlugService.unRegisterRadioClientBinder();
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
    }

    public void registerRadioCallback(IRadioCallBack callback) {
        if (mIsBindService) {
            if (mFMPlugService != null) {
                try {
                    mFMPlugService.registerRadioCallback(callback);
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
    }

    public void unRegisterRadioCallback(IRadioCallBack callback) {
        if (mIsBindService) {
            if (mFMPlugService != null) {
                try {
                    mFMPlugService.unRegisterRadioCallback(callback);
                } catch (RemoteException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
    }

    /**
     * FM/AM切换波段
     */
    public void onBandEvent() {
        if (mFMPlugService != null) {
            try {
                mFMPlugService.onBandEvent();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.v(TAG, " onBandEvent failed -->FMService null");
        }
    }

    /**
     * 自动搜索存台
     */
    public void onASEvent() {
        if (mFMPlugService != null) {
            try {
                mFMPlugService.onASEvent();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.v(TAG, " onASEvent failed -->FMService null");
        }
    }

    /**
     * 浏览存储电台
     */
    public void onPSEvent() {
        if (mFMPlugService != null) {
            try {
                mFMPlugService.onPSEvent();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.v(TAG, " onPSEvent failed -->FMService null");
        }
    }

    /**
     * 远近程切换
     */
    public void onLocDxEvent() {
        if (mFMPlugService != null) {
            try {
                mFMPlugService.onLocDxEvent();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.v(TAG, " onLocDxEvent failed -->FMService null");
        }
    }

    /**
     * 向下收搜有效台
     */
    public void onSeekDownEvent() {
        if (mFMPlugService != null) {
            try {
                mFMPlugService.onSeekDownEvent();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.v(TAG, " onSeekDownEvent failed -->FMService null");
        }
    }

    /**
     * 向上收搜有效台
     */
    public void onSeekUpEvent() {
        if (mFMPlugService != null) {
            try {
                mFMPlugService.onSeekUpEvent();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.v(TAG, " onSeekUpEvent failed -->FMService null");
        }
    }

    /**
     * 步进一个单位
     */
    public void onManualUpEvent() {
        if (mFMPlugService != null) {
            try {
                mFMPlugService.onManualUpEvent();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.v(TAG, " onManualUpEvent failed -->FMService null");
        }
    }

    /**
     * 步进一个单位
     */
    public void onManualDownEvent() {
        if (mFMPlugService != null) {
            try {
                mFMPlugService.onManualDownEvent();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.v(TAG, " onManualDownEvent failed -->FMService null");
        }
    }

    /**
     * 全域收搜有效电台
     */
    public void onScanEvent() {
        if (mFMPlugService != null) {
            try {
                mFMPlugService.onScanEvent();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.v(TAG, " onScanEvent failed -->FMService null");
        }
    }

    /**
     * 切到对应频点
     *
     * @param freq
     */
    public void gotoFreq(int freq) {
        if (mFMPlugService != null) {
            try {
                mFMPlugService.gotoFreq(freq);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.v(TAG, " gotoFreq failed -->FMService null");
        }
    }

    /**
     * 切到对应频点
     *
     * @param freq
     */
    public void gotoFreq(String freq) {
        if (mFMPlugService != null) {
            try {
                mFMPlugService.gotoFreq2(freq);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.v(TAG, "gotoFreq(String) failed -->FMService null");
        }
    }

    /**
     * 切到预存台
     *
     * @param index
     */
    public void gotoFreqIndex(int index) {
        if (mFMPlugService != null) {
            try {
                mFMPlugService.gotoFreqIndex(index);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.v(TAG, " gotoFreqIndex failed -->FMService  null");
        }
    }

    /**
     * 获取当前FM/AM波段
     *
     * @return
     */
    public int getCurrentBand() {
        int ret = BAND_FM_1;
        if (mFMPlugService != null) {
            try {
                ret = mFMPlugService.getCurrentBand();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, "getCurrentBand failed -->FMService  null");
        }

        return ret;
    }

    /**
     * 获取当前电台
     *
     * @return
     */
    public int getCurrentFreq() {
        int ret = 0;
        if (mFMPlugService != null) {
            try {
                ret = mFMPlugService.getCurrentFreq();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, "getCurrentFreq failed-->FMService is null");
        }

        return ret;
    }

    /**
     * 是否处于自动搜索存台中
     *
     * @return
     */
    public boolean IsAS() {
        boolean ret = false;
        if (mFMPlugService != null) {
            try {
                ret = mFMPlugService.IsPS();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, " IsAS failed -->FMService null");
        }

        return ret;
    }

    /**
     * 是否处于电台浏览中
     *
     * @return
     */
    public boolean IsPS() {
        boolean ret = false;
        if (mFMPlugService != null) {
            try {
                ret = mFMPlugService.IsPS();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, " isPS failed -->FMService null");
        }

        return ret;
    }

    /**
     * 是否在进行搜台
     *
     * @return
     */
    public boolean IsScan() {
        boolean ret = false;
        if (mFMPlugService != null) {
            try {
                ret = mFMPlugService.IsScan();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, " isScan failed -->FMService  null");
        }

        return ret;
    }

    /**
     * 是否在进行上下一个有效台收搜
     *
     * @return
     */
    public boolean IsSeek() {
        boolean ret = false;
        if (mFMPlugService != null) {
            try {
                ret = mFMPlugService.IsSeek();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, "IsSeek failed -->FMService null");
        }

        return ret;
    }

    /**
     * 是否是stereo状态（立体声状态）
     *
     * @return
     */
    public boolean IsStereo() {
        boolean ret = false;
        if (mFMPlugService != null) {
            try {
                ret = mFMPlugService.IsStereo();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, "IsStereo failed -->FMService null");
        }

        return ret;
    }

    /**
     * 是否远近程
     *
     * @return
     */
    public boolean IsDxLocal() {
        boolean ret = false;
        if (mFMPlugService != null) {
            try {
                ret = mFMPlugService.IsDxLocal();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, "IsDxLocal failed -->FMService null");
        }

        return ret;
    }
}
