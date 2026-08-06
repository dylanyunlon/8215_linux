// IRadioServiceAPI.aidl
package com.hcn.autoradio;
import com.hcn.autoradio.IRadioCallBack;

// Declare any non-default types here with import statements
interface IRadioServiceAPI {
    void registerRadioClientBinder(IBinder clientBinder);
    void unRegisterRadioClientBinder();
    void registerRadioCallback(IRadioCallBack callback);
    void unRegisterRadioCallback(IRadioCallBack callback);
    void onBandEvent();
    void onASEvent();
    void onPSEvent();
    void onLocDxEvent();
    void onSeekDownEvent();
    void onSeekUpEvent();
    void onManualUpEvent() ;
    void onManualDownEvent();
    void onScanEvent() ;
    void gotoFreq(int freq);
    void gotoFreq2(String freq);
    void gotoFreqIndex(int index);
    int getCurrentBand();
    int getCurrentFreq();
    boolean IsAS();
    boolean IsPS();
    boolean IsScan();
    boolean IsSeek();
    boolean IsStereo();
    boolean IsDxLocal();
    boolean requestPlayAudio() ;
}