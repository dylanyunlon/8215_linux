package com.hcn.bluetooth.bean;

public class CallInfo {
    /**
     * 电话号码
     */
    private String mCallNumber;
    /**
     * 通话状态
     */
    private int mCallState;
    /**
     * 通话时间，从接听开始计时，单位秒
     */
    private int mCallTime;

    public CallInfo(String callNumber, int callState) {
        this(callNumber,callState,0);
    }

    public CallInfo(String callNumber, int callState, int callTime) {
        mCallNumber = callNumber;
        mCallState = callState;
        mCallTime = callTime;
    }

    public String getCallNumber() {
        return mCallNumber;
    }

    public void setCallNumber(String callNumber) {
        mCallNumber = callNumber;
    }

    public int getCallState() {
        return mCallState;
    }

    public void setCallState(int callState) {
        mCallState = callState;
    }

    public int getCallTime() {
        return mCallTime;
    }

    public void setCallTime(int callTime) {
        mCallTime = callTime;
    }

    /**
     * 通话时间增加一秒
     */
    public void CallTimeAdd(){
        mCallTime+=1;
    }
}
